#ifndef __APPLICATION_
#define __APPLICATION_

#include <stdint.h>
#include <stddef.h>

#define MAX_PACKET_SIZE 512 /*Maximum packet size allowed by llwrite*/
#define MAX_PACKET_DATA_SIZE MAX_PACKET_SIZE - 4 /*Section of MAX_PACKET_SIZE that's available for data*/
#define CONTROL_PACKET_PARAMETER_COUNT 2 /* Number of parameters sent in control packet */
#define MAX_CONTROL_PARAMETER_SIZE ((MAX_PACKET_SIZE-1)/CONTROL_PACKET_PARAMETER_COUNT)

/* Control bytes sent in control packet */
#define CTL_BYTE_DATA 1
#define CTL_BYTE_START 2
#define CTL_BYTE_END 3

typedef union {
    struct {
        uint8_t control;
        uint8_t sequence_number;
        uint8_t L2;
        uint8_t L1;
        uint8_t data[MAX_PACKET_SIZE - 4]; // TODO: this -4 looks awful tbh
    };
    uint8_t bytes[MAX_PACKET_SIZE - 1];
} data_packet;

typedef enum {
    SIZE,
    NAME
} parameter_type; /* Control packet parameters type */

/* TLV Control packet parameter */
typedef union {
    struct {
        uint8_t type;
        uint8_t length;
        uint8_t value[MAX_CONTROL_PARAMETER_SIZE-2];
    };
    uint8_t bytes[MAX_CONTROL_PARAMETER_SIZE];
} control_parameter;

typedef struct {
    uint8_t control;
    control_parameter parameters[CONTROL_PACKET_PARAMETER_COUNT];
} control_packet;

/**
 * @brief Print program usage
 * 
 * @param name - program name
 */
void print_usage(char *name);

/**
 * @brief Run receiver
 * 
 * @param argc 
 * @param argv 
 * @param port_number serial port number [0,999] 
 * @return int status
 */
int receiver(int argc, char *argv[], int port_number);

/**
 * @brief Read packets from input_fd and write them onto output_fd
 * 
 * @param input_fd 
 * @param output_fd 
 * @param argc 
 * @return int 
 */
int receiver_read_to_file(int input_fd, int output_fd, int argc);

/**
 * @brief Process data packets, writting them into output_fd
 * 
 * @param buffer 
 * @param sequence 
 * @param output_fd 
 * @return int 
 */
int process_data_packet(uint8_t *buffer, uint8_t *sequence, int output_fd);

/**
 * @brief Process the SIZE parameter sent in control packet
 * 
 * @param value 
 * @return uint64_t total_packets necessary to send the complete file
 */
uint64_t process_size_parameter(uint8_t *value);

/**
 * @brief Write a control packet using llwrite
 * 
 * @param fd 
 * @param packet 
 * @return int 
 */
int write_control_packet(int fd, control_packet *packet);

/**
 * @brief Write a data packet using llwrite
 * 
 * @param fd 
 * @param packet 
 * @return int status
 */
int write_data_packet(int fd, data_packet *packet);

/**
 * @brief Fill a parameter in control packet
 * 
 * @param packet target control packet
 * @param parameter_index index of chosen parameter
 * @param type 
 * @param length 
 * @param value 
 * @return int status
 */
int control_packet_fill_parameter(control_packet *packet, size_t parameter_index, parameter_type type, size_t length, uint8_t *value);

/**
 * @brief Run emitter
 * 
 * @param argc 
 * @param argv 
 * @param port_number serial port number [0,999] 
 * @return int 
 */
int emitter(int argc, char *argv[], int port_number);

/**
 * @brief Create a complete control packet
 * 
 * @param ctl_packet 
 * @param fd 
 * @param filename 
 * @return uint64_t 
 */
uint64_t create_control_packet(control_packet *ctl_packet, int fd, char *filename);

#endif