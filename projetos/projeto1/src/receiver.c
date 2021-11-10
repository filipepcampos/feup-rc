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

	framecontent fc = create_non_information_frame(CTL_UA);
	emitter(fd, &fc);

	
	fc = create_non_information_frame(CTL_RR);
	while(true){
		printf("Attempting to read\n");
		receive_frame(fd);
		emitter(fd, &fc);
	}
	
	disconnect_serial(fd, &oldtio);
	return 0;
}