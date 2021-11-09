#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <signal.h>
#include "core.h"
#include "rcv.h"

volatile bool RESEND_FRAME = false;

void sig_handler(int signum){
	RESEND_FRAME = true;
}

int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGALRM, &a, NULL);

    struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);

	emitter(fd, CTL_SET);
	alarm(FRAME_RESEND_TIMEOUT);
	while(true){
		framecontent fc = receive_frame(fd);
		if(fc.control != 0){
			if(fc.control == CTL_UA){
				break;
			}
		}
		if(RESEND_FRAME){
			RESEND_FRAME = false;
			emitter(fd, CTL_SET);
			alarm(FRAME_RESEND_TIMEOUT);
		}
	}

	disconnect_serial(fd, &oldtio);
	return 0;
}