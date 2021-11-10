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

	emit_until_response(fd, CTL_SET, CTL_UA);

	int fd2 = open("./file", 2); // TODO This may stupid
	char buffer[5];
	printf("Reading\n");
	while(read(fd2, &buffer, 5) == 5){
		printf("Emitting info\n");
		emit_information_until_response(fd, buffer, 5, CTL_RR);
		printf("Finished emit\n");
	}

	disconnect_serial(fd, &oldtio);
	return 0;
}