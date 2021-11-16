#include "core.h"
#include "rcv.h"
#include "util.h"
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

void print_control_byte(uint8_t byte){
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
void print_address_byte(uint8_t byte){
	switch(byte){
		case ADDRESS1 : printf("ADRESS1"); break;
		case ADDRESS2 : printf("ADDRESS2"); break;
		default: printf("INVALID"); break;
	}
}
void log_emission(framecontent *fc){
	printf(" emit: CTL=");
	print_control_byte(fc->control);
	printf(" ADR=");
	print_address_byte(fc->address);
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
	printf(" receive: CTL=");
	print_control_byte(fc->control);
	printf(" ADR=");
	print_address_byte(fc->address);
	if(IS_INFO_CONTROL_BYTE(fc->control)){
		printf(" INFO=");
		for(int i = 0;  i < fc->data_len; ++i){
			printf(" %x ", fc->data[i]);
		}
		printf(" S=%d", GET_INFO_FRAME_CTL_BIT(fc->control));
	}
	printf("\n");
}

int frame_to_bytes(char *buffer, size_t buffer_size, framecontent *fc) {
	if (buffer_size < 5) {
		return -1;
	}
	buffer[0] = FLAG;
	buffer[1] = fc->address;
	buffer[2] = fc->control;
	buffer[3] = (fc->address) ^ (fc->control);
	int i = 4;
	if(fc->data_len > 0){
		strncpy(buffer+4, fc->data, fc->data_len);
		uint8_t bcc = calculate_bcc(fc->data, fc->data_len);
		i += fc->data_len;
		buffer[i++] = bcc;
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

int emitter(int fd, framecontent *fc) {
	size_t buffer_size = 5 + (fc->data_len > 0 ? 1 : 0) + fc->data_len;
	char buffer[buffer_size];
	frame_to_bytes(buffer, buffer_size, fc);
	send_bytes(fd, buffer, buffer_size);
	log_emission(fc);
	return 0;
}

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
	fc.data = data;
	fc.data_len = data_len;
	return fc;
}

int setup_serial(struct termios *oldtio, char *serial_device) {
	/*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
	int fd = open(serial_device, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(serial_device);
		exit(-1);
	}
	if (tcgetattr(fd, oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	struct termios newtio;
	bzero(&newtio, sizeof(newtio)); // Set contents of newtio to zero.
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; /*
      BAUDRATE => B38400
      CS8 => Character size mask
      CLOCAL => Ignore modem control lines
      CREAD => Enable receiver
  */
	newtio.c_iflag = IGNPAR;  // Ignore framing errors and parity errors
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 5; /* inter-character timer unused (in deciseconds) */
	newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

	if (tcflush(fd, TCIOFLUSH) == -1) { // Clear the data that might be present in the fd
		perror("tcflush");
		exit(-1);
	} 

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {  // TCSANOW -> set attr takes place immediately
		perror("tcsetattr");
		exit(-1);
	}
	printf("New termios structure set\n");

	return fd;
}

int disconnect_serial(int fd, struct termios *oldtio) {
	if (tcsetattr(fd, TCSANOW, oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	if (close(fd) == -1) {
		perror("close");
		exit(-1);
	}
	return 0;
}

int verifyargv(int argc, char **argv) {
	return (argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) && (strcmp("/dev/ttyS1", argv[1]) != 0));
}

volatile bool RESEND_FRAME = false;

void sig_handler(int signum){
	RESEND_FRAME = true;
}

void setup_sigalarm(){
	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGALRM, &a, NULL);
}

int emit_until_response(int fd, framecontent *fc, uint8_t expected_response){ // TODO: Check this in the proper places
	emitter(fd, fc);
	int attempts = MAX_EMIT_ATTEMPTS - 1;
	alarm(FRAME_RESEND_TIMEOUT);
	while(attempts > 0){
		framecontent response_fc = receive_frame(fd);
		if(response_fc.control == expected_response){
			break;
		}
		if(RESEND_FRAME){
			RESEND_FRAME = false;
			emitter(fd, fc);
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