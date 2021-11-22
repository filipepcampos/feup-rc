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
    printf("DEBUG: Opening port=%d with flag=%d\n", port, flag);
    if(port < 0 || port > 999){
        return -1;
    }
    status = flag;

    setup_sigalarm();

    char device_name[10 + 3];
    snprintf(device_name, 10 + 3, "/dev/ttyS%d", port);

	int fd = setup_serial(&oldtio, device_name);

    if (flag == EMITTER) {
        framecontent fc = create_non_information_frame(CTL_SET);
        if(emit_frame_until_response(fd, &fc, CTL_UA) != 0){
            printf("Maximum emit attempts reached\n");
            return -1;
	    }
    }
    else if (flag == RECEIVER) {
        uint8_t buffer[BUFFER_SIZE];
        framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
        if(received_fc.control != CTL_SET){
            return -1; // todo: check this
        }
	    framecontent fc = create_non_information_frame(CTL_UA);
	    emit_frame(fd, &fc);
    } else {
        perror("Invalid flag");
        return -1;
    }
    return fd;   
}

int llwrite(int fd, uint8_t * buffer, int length) {
    if(status != EMITTER){
        return -1;
    }
    framecontent fc = create_information_frame(buffer, length, S);
    if(emit_frame_until_response(fd, &fc, CTL_RR) != 0){
        printf("Maximum emit attempts reached\n");
        return -1;
    }
    S = 1 - S;
    return 5 + fc.data_len; // TODO: This looks a bit ugly
}

int llread(int fd, uint8_t * buffer) {
    if (status != RECEIVER) {
        return -1;
    }
    framecontent sent_fc = DEFAULT_FC;
    framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
    bool received = false;
    while(!received){ // TODO: review this whole loop
        if(IS_INFO_CONTROL_BYTE(received_fc.control)){
            bool is_new_frame = S != GET_INFO_FRAME_CTL_BIT(received_fc.control);
            if(received_fc.data_len > 0){
                if(is_new_frame){
                    S = 1 - S;
                } else {
                    sent_fc.data_len = -1;
                }
                sent_fc.control = CTL_RR;
                received = true;
            } else {
                if(is_new_frame){
                    sent_fc.control = CTL_REJ;
                } else {
                    sent_fc.control = CTL_RR;
                    sent_fc.data_len = -1;
                    received = true;
                }
            }
        }
        emit_frame(fd, &sent_fc);
    }
    return received_fc.data_len;
}

int llclose(int fd){
    framecontent fc = create_non_information_frame(CTL_DISC);
    if(status == EMITTER){
        emit_frame_until_response(fd, &fc, CTL_DISC);
        fc = create_non_information_frame(CTL_UA);
        emit_frame(fd, &fc);
    } else if (status == RECEIVER){
        uint8_t buffer[BUFFER_SIZE];
        framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
        if(received_fc.control == CTL_DISC){
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