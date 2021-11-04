#include <stdio.h>
#include <stdlib.h>
#include "core.h"

int main(int argc, char *argv[]){
    if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

    struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);
	receiver(fd);

	disconnect_serial(fd, &oldtio);
	return 0;
}