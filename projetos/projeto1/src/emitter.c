#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

	if(emit_until_response(fd, &fc, CTL_UA) != 0){
		printf("Maximum emit attempts reached\n");
		exit(1);
	}

	int fd2 = open("./file", 2); // TODO This may stupid
	char buffer[5];
	while(read(fd2, &buffer, 5) == 5){
		fc = create_information_frame(buffer, 5, 1);
		if(emit_until_response(fd, &fc, CTL_RR) != 0){
			printf("Maximum emit attempts reached\n");
			exit(1);
		}
	}
	fc = create_non_information_frame(CTL_DISC);
	emit_until_response(fd, &fc, CTL_DISC);
	fc = create_non_information_frame(CTL_UA);
	emitter(fd, &fc);
	disconnect_serial(fd, &oldtio);
	return 0;
}