#ifndef __INTERFACE_
#define __INTERFACE_


#define MAX_INFO_SIZE 512
#define MAX_SIZE (MAX_INFO_SIZE*2 + 2)

#define INFO_FRAME_FAILURE_RATE 0 // Percentage

typedef struct {
    int fileDescriptor; // Descritor correspondente à porta série
    int status; // TRANSMITTER | RECEIVER
} applicationLayer;

typedef enum{
    EMITTER,
    RECEIVER
} flag_t;


/**
 * @brief Start serial port with respective flag
 * 
 * @param port serial port to be started
 * @param flag indicator if it is emitter or receiver
 * @return int -1 if failure; file descriptor if success
 */
int llopen(int port, flag_t flag);

/**
 * @brief Create frame and send it
 * 
 * @param fd file descriptor
 * @param buffer buffer with packet
 * @param length length of packet
 * @return int -1 if failure
 */
int llwrite(int fd, uint8_t * buffer, int length);

/**
 * @brief Receive frame and read it
 * 
 * @param fd file descriptor
 * @param buffer buffer with packet
 * @return int -1 if failure; packet length if success
 */
int llread(int fd, uint8_t * buffer);

/**
 * @brief Finish transmission and close serial port
 * 
 * @param fd file descriptor
 * @return int -1 if failure; 0 if success
 */
int llclose(int fd);

#endif