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

	emit_until_response(fd, &fc, CTL_UA);

	int fd2 = open("./file", 2); // TODO This may stupid
	char buffer[5];
	printf("Reading\n");
	while(read(fd2, &buffer, 5) == 5){
		printf("Emitting info\n");
		fc = create_information_frame(buffer, 5, 1);
		emit_until_response(fd, &fc, CTL_RR);
		printf("Finished emit\n");
	}

	disconnect_serial(fd, &oldtio);
	return 0;
}