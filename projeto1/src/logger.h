#ifndef __LOGGER__
#define __LOGGER__

#include "common.h"

void log_address_byte(uint8_t byte);
void log_control_byte(uint8_t byte);
void log_emission(framecontent *fc);
void log_receival(framecontent *fc);

#endif