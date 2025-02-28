#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"


// ========= [Serial port configuration] ========= //
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
	newtio.c_cc[VTIME] = DEFAULT_VTIME; /* inter-uint8_tacter timer unused (in deciseconds) */
	newtio.c_cc[VMIN] = DEFAULT_VMIN;  /* blocking read until 5 uint8_ts received */

	if (tcflush(fd, TCIOFLUSH) == -1) { // Clear the data that might be present in the fd
		perror("tcflush");
		exit(-1);
	} 
	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {  // TCSANOW -> set attr takes place immediately
		perror("tcsetattr");
		exit(-1);
	}
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

// ========= [Alarm setup] ========= //
volatile bool ALARM_ACTIVATED = false;

void sig_handler(int signum){
	ALARM_ACTIVATED = true;
}

void setup_sigalarm(){
	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGALRM, &a, NULL);
}