#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include <string.h>
#include "core.h"
#include "datalink_receiver.h"
#include "logger.h"


receiver_state statemachine_flag_received(uint8_t byte) {
	return (byte == ADDRESS1 || byte == ADDRESS2) ? A_RCV : START;
}

bool is_valid_control_byte(uint8_t byte) {
	if (byte == CTL_SET || byte == CTL_UA || byte == CTL_DISC 
			|| (APPLY_RESPONSE_CTL_MASK(byte) == CTL_RR) 
			|| (APPLY_RESPONSE_CTL_MASK(byte) == CTL_REJ)
			|| IS_INFO_CONTROL_BYTE(byte)
		) { 
		return true;
	}
	return false;
}

receiver_state statemachine_address_received(uint8_t byte) {
	return is_valid_control_byte(byte) ? C_RCV : START;
}

receiver_state statemachine_control_received(uint8_t byte, framecontent *fc) {
	if (((fc->control) ^ (fc->address)) == byte) {
		if(IS_INFO_CONTROL_BYTE(fc->control)) {
			return INFO;
		}
		return BCC_OK;
	}
	return START;
}

framecontent receive_frame(int fd) {
	framecontent fc;
	size_t buffer_pos = 0;
	uint8_t current_byte;
	receiver_state state = START;
	
	while (state != STOP) {
		int res = read(fd, &current_byte, 1);
		if (res == -1) {
			if(errno == EINTR){ // Read was interrupted by an alarm.
				fc.control = CTL_INVALID_FRAME;
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
				size_t destuffed_size = byte_destuffing(fc.data, buffer_pos);
				uint8_t bcc = fc.data[destuffed_size-1]; // The last byte of the buffer is the BCC. We can't distinguish it from the data until we hit a flag.
				if(bcc == calculate_bcc(fc.data, destuffed_size-1)){
					fc.data_len = destuffed_size-1;
					break;
				} 
				fc.data_len = 0;
				// If an error occurs, in data (wrong BCC) the data is discarded.
			}
			state = FLAG_RCV;
		} else {
			switch (state) {
				case FLAG_RCV: state = statemachine_flag_received(current_byte); 
					fc.address = current_byte; break;
				case A_RCV:	state = statemachine_address_received(current_byte); 
					fc.control = current_byte; break;
				case C_RCV:	state = statemachine_control_received(current_byte, &fc); break;
				case INFO: 
					if(buffer_pos < BUFFER_SIZE){
						fc.data[buffer_pos++] = current_byte; break;
					} else {
						state = START;
						buffer_pos = 0;
						break;
					}
				default: state = START;
			}
		}
	}

	int random_value = rand() % 101;
	if(random_value < FER){
		if(random_value < FER_HEADER){
			printf("[Efficiency Analysis]: Simulating an error in the header\n");
			return receive_frame(fd); // Return to START as if an BCC error occured.
		} else {
			printf("[Efficiency Analysis]: Simulating an error in the data\n");
			fc.data_len = 0;
		}
	}
	usleep(T_PROP);
	log_receival(&fc);
	return fc;
}