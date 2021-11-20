#include "util.h"

uint8_t calculate_bcc(unsigned char *data, size_t data_len){
    if(data_len <= 0){
		return 0; // TODO: Add proper error return
	}
	uint8_t result = 0;
	for(size_t i = 0; i < data_len; ++i){
		result ^= data[i];
	}
	return result;
}