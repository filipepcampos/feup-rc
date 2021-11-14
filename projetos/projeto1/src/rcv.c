#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "core.h"
#include "rcv.h"
#include "util.h"


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
	char *buffer = malloc ((sizeof (char)) * 255); // TODO: Verify, also make sure there's no memory leak
	size_t buffer_pos = 0;
	bool has_info = false;
	uint8_t current_byte;
	receiver_state state = START;
	
	framecontent fc = DEFAULT_FC;

	while (state != STOP) {              /* loop for input */
		int res = read(fd, &current_byte, 1); /* returns after 5 chars have been input */
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
				if(buffer[buffer_pos-1] == calculate_bcc(buffer, buffer_pos-1) ){  // TODO: Make sure this is totally safe
					printf("  received INFO: ");
					for(int i = 0; i < buffer_pos; ++i){
						printf(" %x ", buffer[i]);
					}
					printf("\n");
					fc.data = buffer;
					fc.data_len = buffer_pos - 1; // TODO: Make sure this is totally safe
					state = STOP;
					has_info = true;
					break;
				}
				buffer_pos = 0;
				state = START;  // TODO: Is this the correct behaviour?				
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
	if(!has_info){
		free(buffer);
	}
	return fc;
}