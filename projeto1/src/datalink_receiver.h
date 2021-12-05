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

/**
 * @brief Process byte when in FLAG_RCV state
 * 
 * @param byte 
 * @return receiver_state Next state
 */
receiver_state statemachine_flag_received(uint8_t byte);

/**
 * @brief Verify if the given byte is an valid control byte
 * 
 * @param byte 
 * @return bool
 */
bool is_valid_control_byte(uint8_t byte);

/**
 * @brief Process byte when in A_RCV state
 * 
 * @param byte 
 * @return receiver_state Next state
 */
receiver_state statemachine_address_received(uint8_t byte);

/**
 * @brief Process byte when in C_RCV state
 * 
 * @param byte 
 * @param fc 
 * @return receiver_state Next state
 */
receiver_state statemachine_control_received(uint8_t byte, framecontent *fc);

/**
 * @brief Receive a frame from fd 
 * 
 * @param fd 
 * @return framecontent FC filled with the received frame.
 */
framecontent receive_frame(int fd);

#endif