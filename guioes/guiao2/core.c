#include "core.h"
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

int emitter(int fd) {
	framecontent fc;        //TODO Change name
	fc.address = ADDRESS1;  // TODO: Confirmar
	fc.control = CTL_SET;
	char buffer[5];
	create_frame(buffer, 5, &fc);
	send_frame(fd, buffer, 5);
	return 0;
}

receiver_state statemachine_flag(uint8_t byte) {
	return (byte == ADDRESS1 || byte == ADDRESS2) ? A_RCV : START;  // TODO: Maybe verify this
}

bool valid_ctl_byte(uint8_t byte) {
	if (byte == CTL_SET || byte == CTL_UA) { //TODO: Add all control 
		return true;
	}
	return false;
}

receiver_state statemachine_addressrcv(uint8_t byte) {
	return valid_ctl_byte(byte) ? C_RCV : START;
}

receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc) {
	if (((fc->control) ^ (fc->address)) == byte) {
		return BCC_OK;
	}
	return START;
}

int receiver(int fd) {
	char buffer[255];
	receiver_state state = START;
	framecontent fc;

	while (state != STOP) {              /* loop for input */
		int res = read(fd, buffer, 255); /* returns after 5 chars have been input */
		if (res == -1) {
			perror("read");
			exit(-1);
		}
		printf("DEBUG> %s %d bytes read\n", buffer, res);
		for (int i = 0; i < res; ++i) {
			uint8_t byte = buffer[i];
			if(byte == FLAG){
				if(state == BCC_OK){
					state = STOP;
					break;
				}
				state = FLAG_RCV;
			} else {
				switch (state) {
					case FLAG_RCV: state = statemachine_flag(byte); 
						fc.address = byte; break;
					case A_RCV:	state = statemachine_addressrcv(byte); 
						fc.control = byte; break;
					case C_RCV:	state = statemachine_cRcv(byte, &fc); break;
					default: state = START;
				}
			}
		}
	}
	printf("Read successful. (Address=%x and Control=%x)\n", fc.address, fc.control);
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
