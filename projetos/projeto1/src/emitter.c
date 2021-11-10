#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "core.h"
#include "rcv.h"

int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	setup_sigalarm();

	struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);

	emit_until_response(fd, CTL_SET, CTL_UA);

	disconnect_serial(fd, &oldtio);
	return 0;
}