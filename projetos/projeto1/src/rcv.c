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
	
	if (byte == CTL_SET || byte == CTL_UA || byte == CTL_RR || byte == CTL_DISC || byte == CTL_REJ|| ((byte & 0xBF) == 0)) { //TODO: Add all control, change 0xBF
		return true;
	}
	return false;
}

receiver_state statemachine_addressrcv(uint8_t byte) {
	return valid_ctl_byte(byte) ? C_RCV : START;
}

receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc) {
	if (((fc->control) ^ (fc->address)) == byte) {
		if( ((fc->control) & 0xBF) == 0) { // TODO: Change 0xBF to define
			return INFO;
		}
		return BCC_OK;
	}
	return START;
}

framecontent receive_frame(int fd) {
	char *buffer = malloc ((sizeof (char)) * 255); // TODO: Verify
	size_t buffer_pos = 0;
	uint8_t current_byte;
	receiver_state state = START;
	
	framecontent fc = DEFAULT_FC;

	while (state != STOP) {              /* loop for input */
		printf("	debug: going to read a byte\n");
		int res = read(fd, &current_byte, 1); /* returns after 5 chars have been input */
		printf("	got %x\n", current_byte);
		switch(state){
			case START : printf("START\n"); break;
			case FLAG_RCV : printf("FLAGRCV\n"); break;
			case A_RCV : printf("A_RCV\n"); break;
			case C_RCV : printf("C_RCV\n"); break;
			case BCC_OK : printf("BCC_OK\n"); break;
			case INFO : printf("INFO\n"); break;
			case STOP : printf("STOP\n"); break;
		}
		if (res == -1) {
			if(errno == EINTR){
				fc.control = 0; // TODO: Make sure there's no other Command with this value, and that this is properly documented
				return fc;  // TODO: There is a risk of data loss doing this, although it's kinda small
			}
			perror("read");
			exit(-1);
		}
		if(current_byte == FLAG){
			if(state == BCC_OK){
				state = STOP;
				break;
			}
			if(state == INFO){
				// TODO: verificar BCC
				fc.data = buffer;
				fc.data_len = buffer_pos;
				state = STOP;
				break;
			}
			state = FLAG_RCV;
		} else {
			switch (state) {
				case FLAG_RCV: state = statemachine_flag(current_byte); 
					fc.address = current_byte; break;
				case A_RCV:	state = statemachine_addressrcv(current_byte); 
					fc.control = current_byte; break;
				case C_RCV:	state = statemachine_cRcv(current_byte, &fc); break;
					default: state = START;
				case INFO: buffer[buffer_pos++] = current_byte; break;
			}
		}
	}
	printf("Read successful. (Address=%x and Control=%x)\n", fc.address, fc.control);
	return fc;
}