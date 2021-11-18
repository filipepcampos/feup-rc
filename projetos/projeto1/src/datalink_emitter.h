#ifndef __DATALINKEMITTER__
#define __DATALINKEMITTER__

#include "common.h"

int frame_to_bytes(char *buffer, size_t buffer_size, framecontent *fc);
int send_bytes(int fd, char *frame, size_t frame_size);
int emit_frame(int fd, framecontent *fc);
int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response);

#endif