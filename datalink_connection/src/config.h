#ifndef __CONFIG__
#define __CONFIG__

#define BAUDRATE B38400
#define DEFAULT_VTIME 5
#define DEFAULT_VMIN 5

#include <termios.h>
#include <stdbool.h>

extern volatile bool ALARM_ACTIVATED;

/**
 * @brief Set the up serial port
 * 
 * @param oldtio Old serial port configuration
 * @param serial_device Serial port device
 * @return int -1 if error, file descriptor if success
 */
int setup_serial(struct termios *oldtio, char *serial_device);

/**
 * @brief Disconnect serial port
 * 
 * @param fd File descriptor
 * @param oldtio Old serial port configuration
 * @return int 0 if success, -1 if failure
 */
int disconnect_serial(int fd, struct termios *oldtio);

/**
 * @brief Activate alarm
 * 
 * @param signum 
 */
void sig_handler(int signum);

/**
 * @brief Set up the alarm
 * 
 */
void setup_sigalarm();

#endif