#include <termio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FRAME_RESEND_TIMEOUT 3
#define MAX_EMIT_ATTEMPTS 3

#define FLAG 0x7e
#define ADDRESS1 0x03  // TODO: Change name
#define ADDRESS2 0x01

// Control Commands
#define CTL_SET 0x03
#define CTL_UA 0x07
#define CTL_DISC 0X0B
#define CTL_RR 0x05
#define CTL_REJ 0x01

// Representation of a frame, after flags have been removed
typedef struct {
	uint8_t address;
	uint8_t control;
	char *data;
	size_t data_len;
} framecontent;

#define DEFAULT_FC {0, 0, NULL, 0};

#define INFORMATION_FRAME_CONTROL_BYTE(S) (S << 6)

int setup_serial(struct termios *oldtio, char *serial_device);
int disconnect_serial(int fd, struct termios *oldtio);

int verifyargv(int argc, char **argv);

int emitter(int fd, framecontent *fc);
int send_bytes(int fd, char *frame, size_t frame_size);
int frame_to_bytes(char *buffer, size_t buffer_size, framecontent *fc);

void sig_handler(int signum);
void setup_sigalarm();
int emit_until_response(int fd, framecontent *fc, uint8_t expected_response);

framecontent create_non_information_frame(uint8_t control);

framecontent create_information_frame(char *data, size_t data_len, int S);

int emitter_information(int fd, char *data, uint8_t data_len, int S);
