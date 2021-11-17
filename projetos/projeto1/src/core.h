#ifndef __CORE__
#define __CORE__

#include <termio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "common.h"

int setup_serial(struct termios *oldtio, char *serial_device);
int disconnect_serial(int fd, struct termios *oldtio);

int verifyargv(int argc, char **argv);

int emit_frame(int fd, framecontent *fc);
int send_bytes(int fd, char *frame, size_t frame_size);
int frame_to_bytes(char *buffer, size_t buffer_size, framecontent *fc);

void sig_handler(int signum);
void setup_sigalarm();
int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response);

framecontent create_non_information_frame(uint8_t control);

framecontent create_information_frame(char *data, size_t data_len, int S);

size_t byte_stuffing(char *data, size_t data_len);
size_t byte_destuffing(char* buffer, size_t buf_size);

#endif