#ifndef __APPLICATION_
#define __APPLICATION_

#include <stdint.h>

#define MAX_PACKET_SIZE 512
#define MAX_PACKET_DATA_SIZE MAX_PACKET_SIZE - 4
#define CONTROL_PACKET_PARAMETER_COUNT 2
#define CTL_BYTE_DATA 1
#define CTL_BYTE_START 2
#define CTL_BYTE_END 3


typedef struct {
    uint8_t sequence_number;
    uint8_t L2;
    uint8_t L1;
    uint8_t data[MAX_PACKET_SIZE - 4]; // TODO: this -4 looks awful tbh
} data_packet;

typedef enum {
    SIZE,
    NAME
} parameter_type;

typedef struct {
    parameter_type type;
    uint8_t length;
    uint8_t *value;
} control_parameter;

typedef struct {
    uint8_t control;
    control_parameter parameters[CONTROL_PACKET_PARAMETER_COUNT];
} control_packet;

#endif