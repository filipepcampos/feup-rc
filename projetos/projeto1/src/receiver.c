#include <stdio.h>
#include <stdlib.h>
#include "core.h"
#include "rcv.h"

int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);
	
	receive_frame(fd);
	emitter(fd, CTL_UA);
	
	disconnect_serial(fd, &oldtio);
	return 0;
}