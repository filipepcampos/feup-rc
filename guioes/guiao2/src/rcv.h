typedef enum {
	START,
	FLAG_RCV,
	A_RCV,
	C_RCV,
	BCC_OK,
	STOP
} receiver_state;

receiver_state statemachine_flag(uint8_t byte);
bool valid_ctl_byte(uint8_t byte);
receiver_state statemachine_addressrcv(uint8_t byte);
receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc);
framecontent receive_frame(int fd);