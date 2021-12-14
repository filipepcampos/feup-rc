#ifndef __LOGGER__
#define __LOGGER__

#include "common.h"

/**
 * @brief Print the control byte
 * 
 * @param byte Control byte
 */
void log_control_byte(uint8_t byte);

/**
 * @brief Print address byte
 * 
 * @param byte Address byte
 */
void log_address_byte(uint8_t byte);

/**
 * @brief Print frame emition info
 * 
 * @param fc Frame emitted
 */
void log_emission(framecontent *fc);

/**
 * @brief Print frame receival info
 * 
 * @param fc Frame received
 */
void log_receival(framecontent *fc);

#endif