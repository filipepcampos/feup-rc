#ifndef __CORE__
#define __CORE__

#include "common.h"

framecontent create_non_information_frame(uint8_t control, uint8_t address);

framecontent create_information_frame(uint8_t *data, size_t data_len, int S, uint8_t address);

size_t byte_stuffing(uint8_t *data, size_t data_len);
size_t byte_destuffing(uint8_t* buffer, size_t buf_size);

uint8_t calculate_bcc(uint8_t *data, size_t data_len);

#endif