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
	if (buffer_size < 5) {
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
	uint8_t buffer[buffer_size]; // TODO: How does this even work?
	frame_to_bytes(buffer, buffer_size, fc);
	send_bytes(fd, buffer, buffer_size);
	log_emission(fc);
	return 0;
}

int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response){ // TODO: Check this in the proper places
	uint8_t buffer[BUFFER_SIZE]; // TODO This is weird
	emit_frame(fd, fc);
	int attempts = MAX_EMIT_ATTEMPTS;
	alarm(FRAME_RESEND_TIMEOUT);
	while(attempts > 0){
		framecontent response_fc = receive_frame(fd, buffer, BUFFER_SIZE);
		if(response_fc.control == expected_response){
			break;
		}
		if(RESEND_FRAME){
			RESEND_FRAME = false;
			emit_frame(fd, fc);
			alarm(FRAME_RESEND_TIMEOUT);
		}
		attempts--;
	}
	alarm(0);
	if(attempts == 0){
		return 1;
	}
	return 0;
}