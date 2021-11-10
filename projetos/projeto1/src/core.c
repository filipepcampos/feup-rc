#include "core.h"
#include "rcv.h"
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


uint8_t calculate_bcc(char *data, size_t data_len){
	if(data_len <= 0){
		return 0; // TODO: Add proper error return
	}
	uint8_t result = 0;
	for(size_t i = 0; i < data_len; ++i){
		result ^= data[i];
	}
	return result;
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
	printf("%d bytes written\n", res);
	return 0;
}

int emitter(int fd, framecontent *fc) {
	size_t buffer_size = 5 + (fc->data_len > 0 ? 1 : 0) + fc->data_len;
	char buffer[buffer_size];
	frame_to_bytes(buffer, buffer_size, fc);
	printf("DEBUG: sending the following message:");
	for(int i = 0; i < buffer_size; ++i){
		printf(" %x", buffer[i]);
	}
	printf("\n");
	send_bytes(fd, buffer, buffer_size);
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
	fc.control = INFORMATION_FRAME_CONTROL_BYTE(S);
	fc.address = ADDRESS1; // TODO:
	fc.data = data;
	fc.data_len = data_len;
	return fc;
}

int emitter_information(int fd, char *data, uint8_t data_len, int S) {
	framecontent fc = DEFAULT_FC;
	fc.address = ADDRESS1;
	fc.control = INFORMATION_FRAME_CONTROL_BYTE(S);
	fc.data = data;
	fc.data_len = data_len;
	size_t buffer_size = 6 + data_len;
	char *buffer = malloc ((sizeof (char)) * buffer_size);
	frame_to_bytes(buffer, buffer_size, &fc);
	send_bytes(fd, buffer, buffer_size);
	return 0;
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
      BAUDRATE => B38400, I don't understand this #TODO
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

void emit_until_response(int fd, framecontent *fc, uint8_t expected_response){
	emitter(fd, fc);
	alarm(FRAME_RESEND_TIMEOUT);
	while(true){
		framecontent response_fc = receive_frame(fd);
		if(response_fc.control == expected_response){
			break;
		}
		if(RESEND_FRAME){
			RESEND_FRAME = false;
			emitter(fd, fc);
			alarm(FRAME_RESEND_TIMEOUT);
		}
	}
	alarm(0);
}