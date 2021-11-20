#include "core.h"
#include "datalink_receiver.h"
#include "util.h"
#include "config.h"
#include "logger.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>



// ========= [Frame creation] ========= //

framecontent create_non_information_frame(uint8_t control){
  framecontent fc = DEFAULT_FC;
  fc.control = control;
  fc.address = ADDRESS1; // TODO: This may be changed
  return fc;
}

framecontent create_information_frame(unsigned char *data, size_t data_len, int S){
	uint8_t bcc = calculate_bcc(data, data_len);
	data[data_len] = bcc;
	size_t stuffed_bytes_size = byte_stuffing(data, data_len+1);

	framecontent fc = DEFAULT_FC;
	fc.control = CREATE_INFO_FRAME_CTL_BYTE(S);
	fc.address = ADDRESS1; // TODO:
	fc.data = data;
	fc.data_len = stuffed_bytes_size;
	return fc;
}

// ========= [Byte stuffing] ========= //

size_t byte_stuffing(unsigned char *data, size_t data_len) {
	unsigned char aux_buffer[BUFFER_SIZE];
	strncpy((char *) aux_buffer, (char *) data, data_len); // TODO: This is not a good idea, maybe use memcpy instead
	
	int k = 0; // TODO: Change name
	for(int i = 0; i < data_len; ++i){
		if (aux_buffer[i] == FLAG) {
			data[k++] = ESCAPE;
			data[k++] = FLAG ^ 0x20;
		}
		else if (aux_buffer[i] == ESCAPE) {
			data[k++] = ESCAPE;
			data[k++] = ESCAPE ^ 0x20;
		} else {
			data[k++] = aux_buffer[i];
		}
	}
	return k;
}

size_t byte_destuffing(unsigned char* buffer, size_t buf_size) {	
	int size_dif = 0;
	int current = 0;
	for (int i = 0; i < buf_size; ++i){
		if( buffer[i] == ESCAPE ){
			i++;
			size_dif++;
			buffer[current] = buffer[i] ^ 0x20;
		} else {
			buffer[current] = buffer[i];
		}
		current++;
	}
	return buf_size - size_dif;
}