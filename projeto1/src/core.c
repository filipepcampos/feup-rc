#include "core.h"
#include "datalink_receiver.h"
#include "config.h"
#include "logger.h"

#include <string.h>

// ========= [Frame creation] ========= //
framecontent create_non_information_frame(uint8_t control, uint8_t address){
  framecontent fc = DEFAULT_FC;
  fc.control = control;
  fc.address = address;
  return fc;
}

framecontent create_information_frame(uint8_t *data, size_t data_len, int S, uint8_t address){
	uint8_t bcc = calculate_bcc(data, data_len);
	data[data_len] = bcc;
	size_t stuffed_bytes_size = byte_stuffing(data, data_len+1);

	framecontent fc = DEFAULT_FC;
	fc.control = CREATE_INFO_FRAME_CTL_BYTE(S);
	fc.address = address;
	fc.data = data;
	fc.data_len = stuffed_bytes_size;
	return fc;
}

// ========= [Byte stuffing] ========= //

size_t byte_stuffing(uint8_t *data, size_t data_len) {
	uint8_t aux_buffer[BUFFER_SIZE];
	if(data_len > BUFFER_SIZE){
		return -1;
	}
	memcpy(aux_buffer, data, data_len);
	
	int k = 0;
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

size_t byte_destuffing(uint8_t* buffer, size_t buf_size) {	
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

// ========= [BCC] ========= //
uint8_t calculate_bcc(uint8_t *data, size_t data_len){
	uint8_t result = 0;
	for(size_t i = 0; i < data_len; ++i){
		result ^= data[i];
	}
	return result;
}