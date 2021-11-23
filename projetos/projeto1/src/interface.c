#include "config.h"
#include "common.h"
#include "datalink_emitter.h"
#include "datalink_receiver.h"
#include "core.h"

#include "interface.h"
#include <stdio.h>

static flag_t status;
static int S = 0;
static struct termios oldtio;

int llopen(int port, flag_t flag) {
    if(port < 0 || port > 999){
        return -1;
    }
    status = flag;
    setup_sigalarm();

    char device_name[10 + 3];
    snprintf(device_name, 10 + 3, "/dev/ttyS%d", port);
	int fd = setup_serial(&oldtio, device_name);

    if (flag == EMITTER) {
        S = 0;
        framecontent fc = create_non_information_frame(CTL_SET, ADDRESS1);
        if(emit_frame_until_response(fd, &fc, CTL_UA) != 0){
            printf("Maximum emit attempts reached\n");
            return -1;
	    }
    }
    else if (flag == RECEIVER) {
        S = 1;
        uint8_t buffer[BUFFER_SIZE];
        framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
        if(received_fc.control != CTL_SET){
            return -1;
        }
	    framecontent fc = create_non_information_frame(CTL_UA, ADDRESS1);
	    emit_frame(fd, &fc);
    } else {
        perror("Invalid flag");
        return -1;
    }
    return fd;   
}

int llwrite(int fd, uint8_t * buffer, int length) {
    if(status != EMITTER || length > MAX_INFO_SIZE){
        return -1;
    }
    framecontent fc = create_information_frame(buffer, length, S,  ADDRESS1);
    if(emit_frame_until_response(fd, &fc, CREATE_RR_FRAME_CTL_BYTE(S)) != 0){
        printf("Maximum emit attempts reached\n");
        return -1;
    }
    S = 1 - S;
    return INFO_FRAME_SIZE_WITHOUT_DATA + fc.data_len;
}

int llread(int fd, uint8_t * buffer) {
    printf("debug: llread\n");
    if (status != RECEIVER) {
        return -1;
    }

    framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
    bool received = false;
    while(!received){
        if(IS_INFO_CONTROL_BYTE(received_fc.control)){
            uint8_t control = 0;

            bool is_new_frame = S != GET_INFO_FRAME_CTL_BIT(received_fc.control);
            if(received_fc.data_len > 0){
                if(is_new_frame){
                    S = 1 - S;
                } else {
                    received_fc.data_len = -1;
                }
                control = CREATE_RR_FRAME_CTL_BYTE(S);
                received = true;
            } else {
                if(is_new_frame){
                    control = CREATE_REJ_FRAME_CTL_BYTE(1-S);
                } else {
                    control = CREATE_RR_FRAME_CTL_BYTE(S);
                    received_fc.data_len = -1;
                    received = true;
                }
            }
            framecontent response_fc = create_non_information_frame(control, ADDRESS1);
            emit_frame(fd, &response_fc);
        } else {
            return -1;
        }
    }
    return received_fc.data_len;
}

int llclose(int fd){
    if(status == EMITTER){
        framecontent fc = create_non_information_frame(CTL_DISC, ADDRESS1);
        emit_frame_until_response(fd, &fc, CTL_DISC);
        fc = create_non_information_frame(CTL_UA, ADDRESS2);
        emit_frame(fd, &fc);
    } else if (status == RECEIVER){
        uint8_t buffer[BUFFER_SIZE];
        framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
        if(received_fc.control == CTL_DISC){
            framecontent fc = create_non_information_frame(CTL_DISC, ADDRESS2);
            emit_frame_until_response(fd, &fc, CTL_UA);
        } else {
            return -1;
        }
    } else {
        return -1;
    }
	disconnect_serial(fd, &oldtio);
    return 0;
}