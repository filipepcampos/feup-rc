#ifndef __INTERFACE_
#define __INTERFACE_

#include <stdbool.h>
#include <termios.h>
#include "common.h"

#define MAX_WRITE_SIZE (MAX_INFO_SIZE)
#define MAX_OPEN_SERIAL_PORTS 8 // Maximum allowed serial ports open at any given time

typedef enum{
    EMITTER,
    RECEIVER
} flag_t;

typedef struct {
    int fd; // File descriptor associated with the serial port
    int S; // Sequence number of the transmission
    flag_t status; // EMITTER | Receiver
    struct termios oldtio; // Old settings to be restored after closing.
    bool open; // States if the serial_interface is still in use or not
} serial_interface;

/**
 * @brief Connect serial port with respective flag
 * 
 * @param port Serial port to be started
 * @param flag Indicator if it is emitter or receiver
 * @return int -1 if failure; file descriptor if success
 */
int llopen(int port, flag_t flag);

/**
 * @brief Create frame and send it
 * 
 * @param fd File descriptor
 * @param buffer Buffer with packet
 * @param length Length of packet
 * @return int -1 if failure, frame size if success
 */
int llwrite(int fd, uint8_t * buffer, int length);

/**
 * @brief Receive frame and read it
 * 
 * @param fd File descriptor
 * @param buffer Buffer with packet
 * @return int -1 if failure, packet length if success
 */
int llread(int fd, uint8_t * buffer);

/**
 * @brief Finish transmission and disconnect serial port
 * 
 * @param fd File descriptor
 * @return int -1 if failure, 0 if success
 */
int llclose(int fd);

#endif