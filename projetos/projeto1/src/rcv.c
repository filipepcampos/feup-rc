#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "core.h"
#include "rcv.h"


receiver_state statemachine_flag(uint8_t byte) {
	return (byte == ADDRESS1 || byte == ADDRESS2) ? A_RCV : START;  // TODO: Maybe verify this
}

bool valid_ctl_byte(uint8_t byte) {
	if (byte == CTL_SET || byte == CTL_UA) { //TODO: Add all control 
		return true;
	}
	return false;
}

receiver_state statemachine_addressrcv(uint8_t byte) {
	return valid_ctl_byte(byte) ? C_RCV : START;
}

receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc) {
	if (((fc->control) ^ (fc->address)) == byte) {
		return BCC_OK;
	}
	return START;
}

framecontent receive_frame(int fd) {
	char buffer[255];
	receiver_state state = START;
	framecontent fc = DEFAULT_FC;

	while (state != STOP) {              /* loop for input */
		int res = read(fd, buffer, 1); /* returns after 5 chars have been input */
		if (res == -1) {
			if(errno == EINTR){
				fc.control = 0; // TODO: Make sure there's no other Command with this value, and that this is properly documented
				return fc;  // TODO: There is a risk of data loss doing this, although it's kinda small
			}
			perror("read");
			exit(-1);
		}
		uint8_t byte = buffer[0];
		if(byte == FLAG){
			if(state == BCC_OK){
				state = STOP;
				break;
			}
			state = FLAG_RCV;
		} else {
			switch (state) {
				case FLAG_RCV: state = statemachine_flag(byte); 
					fc.address = byte; break;
				case A_RCV:	state = statemachine_addressrcv(byte); 
					fc.control = byte; break;
				case C_RCV:	state = statemachine_cRcv(byte, &fc); break;
					default: state = START;
			}
		}
	}
	printf("Read successful. (Address=%x and Control=%x)\n", fc.address, fc.control);
	return fc;
}