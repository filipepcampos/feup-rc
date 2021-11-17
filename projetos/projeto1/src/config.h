#ifndef __CONFIG__
#define __CONFIG__

#define BAUDRATE B38400

int verifyargv(int argc, char **argv);

int setup_serial(struct termios *oldtio, char *serial_device);
int disconnect_serial(int fd, struct termios *oldtio);

extern volatile bool RESEND_FRAME;
void sig_handler(int signum);
void setup_sigalarm();

#endif