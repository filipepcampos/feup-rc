#include <termio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FLAG 0x7e
#define ADDRESS1 0x03  // TODO: Change name
#define ADDRESS2 0x01
#define CTL_SET 0x03
#define CTL_UA 0x07

typedef struct {
	uint8_t address;
	uint8_t control;
} framecontent;


typedef enum {
	START,
	FLAG_RCV,
	A_RCV,
	C_RCV,
	BCC_OK,
	STOP
} receiver_state;

int setup_serial(struct termios *oldtio, char *serial_device);
int disconnect_serial(int fd, struct termios *oldtio);

int verifyargv(int argc, char **argv);

int receiver(int fd);
receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc);
receiver_state statemachine_addressrcv(uint8_t byte);
bool valid_ctl_byte(uint8_t byte);
receiver_state statemachine_flag(uint8_t byte);

int emitter(int fd);
int send_frame(int fd, char *frame, size_t frame_size);
int create_frame(char *buffer, size_t buffer_size, framecontent *fc);