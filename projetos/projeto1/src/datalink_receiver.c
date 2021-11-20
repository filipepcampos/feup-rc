#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "core.h"
#include "datalink_receiver.h"
#include "util.h"
#include "logger.h"


receiver_state statemachine_flag_received(uint8_t byte) {
	return (byte == ADDRESS1 || byte == ADDRESS2) ? A_RCV : START;  // TODO: Maybe verify this
}

bool is_valid_control_byte(uint8_t byte) {
	if (byte == CTL_SET || byte == CTL_UA || 
			byte == CTL_RR || byte == CTL_DISC || 
			byte == CTL_REJ|| IS_INFO_CONTROL_BYTE(byte)
		) { 
		return true;
	}
	return false;
}

receiver_state statemachine_addressrcv(uint8_t byte) {
	return is_valid_control_byte(byte) ? C_RCV : START;
}

receiver_state statemachine_control_received(uint8_t byte, framecontent *fc) {
	if (((fc->control) ^ (fc->address)) == byte) {
		if( ((fc->control) & 0xBF) == 0) { // TODO: Change 0xBF to define
			return INFO;
		}
		return BCC_OK;
	}
	return START;
}

framecontent receive_frame(int fd, uint8_t *buffer, size_t size) {
	size_t buffer_pos = 0;
	uint8_t current_byte;
	receiver_state state = START;
	
	framecontent fc = DEFAULT_FC;

	while (state != STOP) {
		int res = read(fd, &current_byte, 1);
		if (res == -1) {
			if(errno == EINTR){ // Read was interrupted by an alarm.
				fc.control = 0; // TODO: Make sure there's no other Command with this value, and that this is properly documented
				return fc;
			}
			perror("read");
			exit(-1);
		}
		if(current_byte == FLAG){
			if(state == BCC_OK){ // Marks the end of an non-information frame
				state = STOP;
				break;
			}
			if(state == INFO){
				state = STOP;
				size_t destuffed_size = byte_destuffing(buffer, buffer_pos);
				uint8_t bcc = buffer[destuffed_size-1]; // The last byte of the buffer is the BCC. We can't distinguish it from the data until we hit a flag.
				if(bcc == calculate_bcc(buffer, destuffed_size-1)){
					fc.data = buffer;
					fc.data_len = destuffed_size-1;
					break;
				} 
				// If an error occurs, in data (wrong BCC) the data is discarded.
				// TODO: Document this behaviour
			}
			state = FLAG_RCV;
		} else {
			switch (state) {
				case FLAG_RCV: state = statemachine_flag_received(current_byte); 
					fc.address = current_byte; break;
				case A_RCV:	state = statemachine_addressrcv(current_byte); 
					fc.control = current_byte; break;
				case C_RCV:	state = statemachine_control_received(current_byte, &fc); break;
					default: state = START;
				case INFO: buffer[buffer_pos++] = current_byte; break;
			}
		}
	}
	log_receival(&fc);
	return fc;
}