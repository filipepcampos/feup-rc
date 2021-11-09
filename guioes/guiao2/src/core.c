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


int create_frame(char *buffer, size_t buffer_size, framecontent *fc) {
	if (buffer_size < 5) {
		return -1;
	}
	buffer[0] = FLAG;
	buffer[1] = fc->address;
	buffer[2] = fc->control;
	buffer[3] = (fc->address) ^ (fc->control);
	buffer[4] = FLAG;
	return 0;
}

int send_frame(int fd, char *frame, size_t frame_size) {
	int res = write(fd, frame, frame_size);
	if (res == -1) {
		perror("write");
		exit(-1);
	} 
	printf("%d bytes written\n", res);
	return 0;
}

int emitter(int fd, uint8_t control_byte) {
	framecontent fc = DEFAULT_FC; //TODO Change name
	fc.address = ADDRESS1;  // TODO: Confirmar
	fc.control = control_byte;
	char buffer[5];
	create_frame(buffer, 5, &fc);
	send_frame(fd, buffer, 5);
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

void emit_until_response(int fd, uint8_t control_byte, uint8_t expected_response){
	emitter(fd, control_byte);
	alarm(FRAME_RESEND_TIMEOUT);
	while(true){
		framecontent fc = receive_frame(fd);
		if(fc.control == expected_response){
			break;
		}
		if(RESEND_FRAME){
			RESEND_FRAME = false;
			emitter(fd, control_byte);
			alarm(FRAME_RESEND_TIMEOUT);
		}
	}
}