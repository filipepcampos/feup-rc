#ifndef __COMMON__
#define __COMMON__

#include <stdint.h>
#include <stddef.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define VERBOSE true

#define INFO_FRAME_SIZE 512
#define BUFFER_SIZE 1026

#define FRAME_RESEND_TIMEOUT 3
#define MAX_EMIT_ATTEMPTS 3

#define FLAG 0x7e
#define ESCAPE 0x7d
#define ADDRESS1 0x03
#define ADDRESS2 0x01

// Control Commands
#define CTL_SET 0x03
#define CTL_UA 0x07
#define CTL_DISC 0X0B
#define CTL_RR 0x05
#define CTL_REJ 0x01

// Representation of a frame, after flags have been removed
typedef struct {
	uint8_t address;
	uint8_t control;
	uint8_t *data;
	size_t data_len;
} framecontent;

#define DEFAULT_FC {0, 0, NULL, 0};

#define CREATE_INFO_FRAME_CTL_BYTE(S) (S << 6)
#define GET_INFO_FRAME_CTL_BIT(b) (b >> 6)
#define IS_INFO_CONTROL_BYTE(b) ((b & 0xBF) == 0)
#define INFO_FRAME_SIZE_WITHOUT_DATA 5

#define CREATE_RR_FRAME_CTL_BYTE(R) ((R << 7) | CTL_RR)
#define CREATE_REJ_FRAME_CTL_BYTE(R) ((R << 7) | CTL_REJ)
#define GET_RESPONSE_FRAME_CTL_BIT(b) (b >> 7)
#define RESPONSE_CTL_MASK 0b00000111
#define APPLY_RESPONSE_CTL_MASK(b) (b & RESPONSE_CTL_MASK)


#endif