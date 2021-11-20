#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "core.h"
#include "datalink_receiver.h"
#include "config.h"
#include "datalink_emitter.h"

int main(int argc, char *argv[]){
	if (verifyargv(argc, argv)) {
		printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS0\n", argv[0], argv[0]);
		exit(1);
	}

	setup_sigalarm();

	struct termios oldtio;
	int fd = setup_serial(&oldtio, argv[1]);
	
	uint8_t buffer[BUFFER_SIZE];
	framecontent received_fc = receive_frame(fd, buffer, BUFFER_SIZE);

	framecontent fc = create_non_information_frame(CTL_UA);
	emit_frame(fd, &fc);

	int fd2 = open("./out", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // TODO This may stupid

	int S = 1;
	while(true){
		received_fc = receive_frame(fd, buffer, BUFFER_SIZE);
		if(IS_INFO_CONTROL_BYTE(received_fc.control)){
			bool is_new_frame = S != GET_INFO_FRAME_CTL_BIT(received_fc.control);
			if(received_fc.data_len > 0){
				if(is_new_frame){
					write(fd2, received_fc.data, received_fc.data_len);
					S = 1 - S;
				}
				fc.control = CTL_RR;
			} else {
				if(is_new_frame){
					fc.control = CTL_REJ;
				} else {
					fc.control = CTL_RR;
				}
			}
		}
		if(received_fc.control == CTL_DISC){
			fc.control = CTL_DISC;
			emit_frame_until_response(fd, &fc, CTL_UA);
			break;
		}
		emit_frame(fd, &fc);
	}
	close(fd2);
	
	disconnect_serial(fd, &oldtio);
	return 0;
}