#ifndef __DATALINKRECEIVER__
#define __DATALINKRECEIVER__

#include <stdbool.h>

typedef enum {
	START,
	FLAG_RCV,
	A_RCV,
	C_RCV,
	BCC_OK,
	INFO,
	STOP
} receiver_state;

receiver_state statemachine_flag_received(uint8_t byte);
bool is_valid_control_byte(uint8_t byte);
receiver_state statemachine_addressrcv(uint8_t byte);
receiver_state statemachine_control_received(uint8_t byte, framecontent *fc);
framecontent receive_frame(int fd, char *buffer, size_t size);

#endif