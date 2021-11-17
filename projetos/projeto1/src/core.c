#include "core.h"
#include "rcv.h"
#include "util.h"
#include "config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

// ========= [Logging functions] ========= //

void log_control_byte(uint8_t byte){
	switch(byte){
		case CTL_SET : printf("SET"); return;
		case CTL_UA : printf("UA"); return;
		case CTL_DISC : printf("DISC"); return;
		case CTL_RR : printf("RR"); return;
		case CTL_REJ : printf("REJ"); return;
	}
	if(IS_INFO_CONTROL_BYTE(byte)){
		printf("INFO");
		return;
	}
	printf("INVALID");
	return;
}
void log_address_byte(uint8_t byte){
	switch(byte){
		case ADDRESS1 : printf("ADRESS1"); break;
		case ADDRESS2 : printf("ADDRESS2"); break;
		default: printf("INVALID"); break;
	}
}
void log_emission(framecontent *fc){
    if(VERBOSE == false){
        return;
    }
	printf(" emit: CTL=");
	log_control_byte(fc->control);
	printf(" ADR=");
	log_address_byte(fc->address);
	if(IS_INFO_CONTROL_BYTE(fc->control)){
		printf(" INFO=");
		for(int i = 0;  i < fc->data_len; ++i){
			printf(" %x ", fc->data[i]);
		}
		printf(" S=%d", GET_INFO_FRAME_CTL_BIT(fc->control));
	}
	printf("\n");
}
void log_receival(framecontent *fc){
    if(VERBOSE == false){
        return;
    }
	printf(" receive: CTL=");
	log_control_byte(fc->control);
	printf(" ADR=");
	log_address_byte(fc->address);
	if(IS_INFO_CONTROL_BYTE(fc->control)){
		printf(" INFO=");
		for(int i = 0;  i < fc->data_len; ++i){
			printf(" %x ", fc->data[i]);
		}
		printf(" S=%d", GET_INFO_FRAME_CTL_BIT(fc->control));
	}
	printf("\n");
}


// ========= [Emission functions] ========= //

int frame_to_bytes(char *buffer, size_t buffer_size, framecontent *fc) {
	if (buffer_size < 5) {
		return -1;
	}
	buffer[0] = FLAG;
	buffer[1] = fc->address; // Address
	buffer[2] = fc->control; // Control
	buffer[3] = (fc->address) ^ (fc->control); // BCC
	int i = 4;
	if(fc->data_len > 0){
		strncpy(buffer+4, fc->data, fc->data_len);
		i += fc->data_len;	
	}
	buffer[i] = FLAG;
	return 0;
}

int send_bytes(int fd, char *buffer, size_t buffer_size) {
	int res = write(fd, buffer, buffer_size);
	if (res == -1) {
		perror("write");
		exit(-1);
	}
	return 0;
}

int emit_frame(int fd, framecontent *fc) {
	size_t buffer_size = 5 + fc->data_len;
	char buffer[buffer_size]; // TODO: How does this even work?
	frame_to_bytes(buffer, buffer_size, fc);
	send_bytes(fd, buffer, buffer_size);
	log_emission(fc);
	return 0;
}

int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response){ // TODO: Check this in the proper places
	char buffer[BUFFER_SIZE]; // TODO This is weird
	emit_frame(fd, fc);
	int attempts = MAX_EMIT_ATTEMPTS - 1;
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

// ========= [Frame creation] ========= //

framecontent create_non_information_frame(uint8_t control){
  framecontent fc = DEFAULT_FC;
  fc.control = control;
  fc.address = ADDRESS1; // TODO: This may be changed
  return fc;
}

framecontent create_information_frame(char *data, size_t data_len, int S){
	framecontent fc = DEFAULT_FC;
	fc.control = CREATE_INFO_FRAME_CTL_BYTE(S);
	fc.address = ADDRESS1; // TODO:

	uint8_t bcc = calculate_bcc(data, data_len);
	size_t stuffed_bytes_size = byte_stuffing(data, data_len);
	fc.data = data;
	fc.data_len = stuffed_bytes_size + 1;
	fc.data[fc.data_len-1] = bcc;
	return fc;
}

// ========= [Byte stuffing] ========= //

size_t byte_stuffing(char *data, size_t data_len) {
	char aux_buffer[BUFFER_SIZE];
	strncpy(aux_buffer, data, data_len);
	
	int k = 0; // TODO: Change name
	for(int i = 0; i < data_len; ++i){
		if (aux_buffer[i] == FLAG) {
			data[k++] = ESCAPE;
			data[k++] = FLAG ^ 0x20;
		}
		else if (aux_buffer[i] == ESCAPE) {
			data[k++] = ESCAPE;
			data[k++] = ESCAPE ^ 0x20;
		} else {
			data[k++] = aux_buffer[i];
		}
	}
	return k;
}

size_t byte_destuffing(char* buffer, size_t buf_size) {	
	int size_dif = 0;
	int current = 0;
	for (int i = 0; i < buf_size; ++i){
		if( buffer[i] == ESCAPE ){
			i++;
			size_dif++;
			buffer[current] = buffer[i] ^ 0x20;
		} else {
			buffer[current] = buffer[i];
		}
		current++;
	}
	return buf_size - size_dif;
}