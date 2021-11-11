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
	
	framecontent received_fc = receive_frame(fd);
	if(received_fc.data != NULL){
		free(received_fc.data);
	}

	framecontent fc = create_non_information_frame(CTL_UA);
	emitter(fd, &fc);

	while(true){
		received_fc = receive_frame(fd);
		if(received_fc.data != NULL){
			free(received_fc.data);
		}
		fc = create_non_information_frame(CTL_RR);
		emitter(fd, &fc);
	}
	
	disconnect_serial(fd, &oldtio);
	return 0;
}