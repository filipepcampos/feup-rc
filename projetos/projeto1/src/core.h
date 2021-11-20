#ifndef __CORE__
#define __CORE__

#include <termio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "common.h"

framecontent create_non_information_frame(uint8_t control);

framecontent create_information_frame(unsigned char *data, size_t data_len, int S);

size_t byte_stuffing(unsigned char *data, size_t data_len);
size_t byte_destuffing(unsigned char* buffer, size_t buf_size);

#endif