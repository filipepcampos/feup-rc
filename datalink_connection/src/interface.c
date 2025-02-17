#include "config.h"
#include "datalink_emitter.h"
#include "datalink_receiver.h"
#include "core.h"

#include "interface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int used_serial_ports = 0;
static serial_interface serial_ports[MAX_OPEN_SERIAL_PORTS];

int first_available_serial_index(){
    if(used_serial_ports < MAX_OPEN_SERIAL_PORTS){
        serial_ports[used_serial_ports].open = true;
        return used_serial_ports++;
    }
    for(int i = 0; i < used_serial_ports; ++i){
        if(serial_ports[i].open == false){ // Reuse serial ports that are not in use
            serial_ports[i].open = true;
            return i;
        }
    }
    return -1;
}

int find_serial_by_fd(int fd){
    for(int i = 0; i < used_serial_ports; ++i){
        if(serial_ports[i].fd == fd){
            return i;
        }
    }
    return -1;
}

int llopen(int port, flag_t flag) {
    if(port < 0 || port > 999){
        return -1;
    }
    int serial_index = first_available_serial_index();
    if(serial_index < 0){ // Too many serial ports already open
        return -1;
    }
    serial_interface *serial = &(serial_ports[serial_index]);
    serial->status = flag;
    setup_sigalarm();

    char device_name[10 + 3];
    snprintf(device_name, 10 + 3, "/dev/ttyS%d", port);
	serial->fd = setup_serial(&(serial->oldtio), device_name);

    if (flag == EMITTER) {
        serial->S = 0;
        framecontent fc = create_non_information_frame(CTL_SET, ADDRESS1);
        if(emit_frame_until_response(serial->fd, &fc, CTL_UA) != 0){
            printf("Maximum emit attempts reached\n");
            return -1;
	    }
    }
    else if (flag == RECEIVER) {
        serial->S = 1;
        bool success = false;
        alarm(READ_TIMEOUT);
        for(int i = 0; i < MAX_EMIT_ATTEMPTS; ++i){
            framecontent received_fc = receive_frame(serial->fd);
            if(received_fc.control == CTL_SET){
                success = true;
                break;
            }
            if(ALARM_ACTIVATED){
                ALARM_ACTIVATED = false;
                return -1;
            }    
        }
        alarm(0);
        ALARM_ACTIVATED = false;
        if(success){
            framecontent fc = create_non_information_frame(CTL_UA, ADDRESS1);
	        emit_frame(serial->fd, &fc);
        } else {
            return -1;
        }
    } else {
        perror("Invalid flag");
        return -1;
    }
    return serial->fd;   
}

int llwrite(int fd, uint8_t * input_buffer, int length) {
    int serial_index = find_serial_by_fd(fd);
    if(serial_index < 0){
        return -1;
    }
    serial_interface *serial = &(serial_ports[serial_index]);

    if(serial->status != EMITTER || length > MAX_INFO_SIZE){
        return -1;
    }
    framecontent fc = create_information_frame(input_buffer, length, serial->S,  ADDRESS1);
    if(fc.data_len == 0){ // Datalen > MAX_INFO_SIZE.
        return -1;
    }
    if(emit_frame_until_response(fd, &fc, CREATE_RR_FRAME_CTL_BYTE(serial->S)) != 0){
        printf("Maximum emit attempts reached\n");
        return -1;
    }
    serial->S = 1 - (serial->S);
    return INFO_FRAME_SIZE_WITHOUT_DATA + fc.data_len;
}

int llread(int fd, uint8_t * output_buffer) {
    int serial_index = find_serial_by_fd(fd);
    if(serial_index < 0){
        return -1;
    }
    serial_interface *serial = &(serial_ports[serial_index]);

    if (serial->status != RECEIVER) {
        return -1;
    }

    framecontent received_fc;
    bool received = false;
    alarm(READ_TIMEOUT);
    while(!received){
        received_fc = receive_frame(fd);
        if(IS_INFO_CONTROL_BYTE(received_fc.control)){
            uint8_t control = 0;

            bool is_new_frame = serial->S != GET_INFO_FRAME_CTL_BIT(received_fc.control);
            if(received_fc.data_len > 0){
                if(is_new_frame){
                    serial->S = 1 - (serial->S);
                } else {
                    received_fc.data_len = 0;
                }
                control = CREATE_RR_FRAME_CTL_BYTE(serial->S);
                received = true;
            } else {
                printf("An error has occurred, responding with: ");
                if(is_new_frame){
                    printf("REJ\n");
                    control = CREATE_REJ_FRAME_CTL_BYTE(1-(serial->S));
                } else {
                    printf("RR\n");
                    control = CREATE_RR_FRAME_CTL_BYTE(serial->S);
                    received_fc.data_len = 0;
                    received = true;
                }
            }
            framecontent response_fc = create_non_information_frame(control, ADDRESS1);
            emit_frame(fd, &response_fc);
        }
        if(ALARM_ACTIVATED){
            ALARM_ACTIVATED = false;
            return -1;
        }
        // If the frame doesn't have an control_byte an error might have occurred
        // The receiver should try to read again
    }
    alarm(0);
    ALARM_ACTIVATED = false;
    memcpy(output_buffer, received_fc.data, received_fc.data_len);
    return received_fc.data_len;
}

int llclose(int fd){
    int serial_index = find_serial_by_fd(fd);
    if(serial_index < 0){
        return -1;
    }
    serial_interface *serial = &(serial_ports[serial_index]);
    serial->open = false;

    if(serial->status == EMITTER){
        framecontent fc = create_non_information_frame(CTL_DISC, ADDRESS1);
        emit_frame_until_response(fd, &fc, CTL_DISC);
        fc = create_non_information_frame(CTL_UA, ADDRESS2);
        emit_frame(fd, &fc);
    } else if (serial->status == RECEIVER){
        alarm(READ_TIMEOUT);
        framecontent received_fc = receive_frame(fd);
        if(received_fc.control == CTL_DISC){
            framecontent fc = create_non_information_frame(CTL_DISC, ADDRESS2);
            emit_frame_until_response(fd, &fc, CTL_UA);
        } else {
            alarm(0);
            ALARM_ACTIVATED = false;
            return -1;
        }
        alarm(0);
        ALARM_ACTIVATED = false;
    } else {
        return -1;
    }
	disconnect_serial(fd, &(serial->oldtio));
    return 0;
}