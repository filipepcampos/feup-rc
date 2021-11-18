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

void sig_handler(int signum);
void setup_sigalarm();

framecontent create_non_information_frame(uint8_t control);

framecontent create_information_frame(char *data, size_t data_len, int S);

size_t byte_stuffing(char *data, size_t data_len);
size_t byte_destuffing(char* buffer, size_t buf_size);

#endif