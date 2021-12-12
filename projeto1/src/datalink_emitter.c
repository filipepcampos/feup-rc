#include "datalink_emitter.h"
#include "datalink_receiver.h"
#include "config.h"
#include "logger.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int frame_to_bytes(uint8_t *buffer, size_t buffer_size, framecontent *fc) {
	if (buffer_size < 5 + fc->data_len) {
		return -1;
	}
	buffer[0] = FLAG;
	buffer[1] = fc->address; // Address
	buffer[2] = fc->control; // Control
	buffer[3] = (fc->address) ^ (fc->control); // BCC
	int i = 4;
	if(fc->data_len > 0){
		memcpy(buffer+4, fc->data, fc->data_len);
		i += fc->data_len;	
	}
	buffer[i] = FLAG;
	return 0;
}

int send_bytes(int fd, uint8_t *buffer, size_t buffer_size) {
	int res = write(fd, buffer, buffer_size);
	if (res == -1) {
		perror("write");
		exit(-1);
	}
	return 0;
}

int emit_frame(int fd, framecontent *fc) {
	size_t buffer_size = 5 + fc->data_len;
	uint8_t buffer[buffer_size];
	if(frame_to_bytes(buffer, buffer_size, fc) < 0){
		return -1;
	}
	if(send_bytes(fd, buffer, buffer_size) < 0){
		return -1;
	}
	log_emission(fc);
	return 0;
}

int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response){
	if(emit_frame(fd, fc) < 0){
		return -1;
	}
	int attempts = MAX_EMIT_ATTEMPTS;
	alarm(FRAME_RESEND_TIMEOUT);
	while(attempts > 0){
		framecontent response_fc = receive_frame(fd);
		if(response_fc.control == expected_response){
			break;
		} else { // Either receive_frame was interrupted by alarm or control byte is invalid (e.g REJ), therefore requiring reemission.
			ALARM_ACTIVATED = false;
			alarm(0);
			printf("Resending frame, attempt %d/%d\n", MAX_EMIT_ATTEMPTS-attempts+1, MAX_EMIT_ATTEMPTS);
			if(emit_frame(fd, fc) < 0){
				return -1;
			}
			alarm(FRAME_RESEND_TIMEOUT);
			attempts--;
		}
	}
	alarm(0);
	ALARM_ACTIVATED = false;
	if(attempts == 0){
		return 1;
	}
	return 0;
}