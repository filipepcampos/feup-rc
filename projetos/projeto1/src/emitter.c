#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "core.h"
#include "rcv.h"


#include <fcntl.h> // TODO maybe remove


int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	setup_sigalarm();

	struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);

	framecontent fc = create_non_information_frame(CTL_SET);

	if(emit_frame_until_response(fd, &fc, CTL_UA) != 0){
		printf("Maximum emit attempts reached\n");
		exit(1);
	}

	int fd2 = open("./file", 2); // TODO This may stupid
	char buffer[BUFFER_SIZE]; 
	int S = 0;
	int read_res = 0;
	while((read_res = read(fd2, &buffer, INFO_FRAME_SIZE)) > 0){
		fc = create_information_frame(buffer, read_res, S);
		if(emit_frame_until_response(fd, &fc, CTL_RR) != 0){
			printf("Maximum emit attempts reached\n");
			exit(1);
		}
		S = 1 - S;
	}
	fc = create_non_information_frame(CTL_DISC);
	emit_frame_until_response(fd, &fc, CTL_DISC);
	fc = create_non_information_frame(CTL_UA);
	emit_frame(fd, &fc);
	disconnect_serial(fd, &oldtio);
	return 0;
}