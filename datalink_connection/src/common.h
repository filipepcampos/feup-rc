#ifndef __COMMON__
#define __COMMON__

#include <stdint.h>
#include <stddef.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define VERBOSE false /* Controls if logger functions are enabled or disabled */

/* For efficiency analysis purposes only, should be set to 0 for real use */
#ifndef FER
#define FER 0 // In percentage. Used to test FER,should be set to 0 for 'real' use.
#endif
#define FER_HEADER (FER/2) // Chance of the error occuring in the header
#define FER_DATA (FER - FER_HEADER) // Chance of the error occuring in the data
// FER = FER_HEADER + FER_DATA
#ifndef T_PROP
#define T_PROP 0 // In microseconds
#endif

#ifndef MAX_INFO_SIZE
#define MAX_INFO_SIZE 512
#endif
#define BUFFER_SIZE (MAX_INFO_SIZE*2 + 2) /*Necessary buffer size to accommodate INFO_FRAME_SIZE, 
							in the edge case where all data is flags (double the size)
							+ 2 bytes for the BCC (if it also needs to be escaped) */

#define FRAME_RESEND_TIMEOUT 3 /* Timeout between frame resends */
#define MAX_EMIT_ATTEMPTS 3 /* Maximum amounts of frame resends before giving up*/
#define READ_TIMEOUT (FRAME_RESEND_TIMEOUT * (MAX_EMIT_ATTEMPTS+1))

#define FLAG 0x7e /*Frame flag byte*/
#define ESCAPE 0x7d /*Frame escape byte*/
#define ADDRESS1 0x03 /*Frame address byte*/
#define ADDRESS2 0x01 /*Frame address byte*/

// Control Commands
#define CTL_SET 0x03
#define CTL_UA 0x07
#define CTL_DISC 0X0B
#define CTL_RR 0x05
#define CTL_REJ 0x01
#define CTL_INVALID_FRAME 0xFF

// Representation of a frame, after flags have been removed
typedef struct {
	uint8_t address;
	uint8_t control;
	uint8_t data[BUFFER_SIZE];
	size_t data_len;
} framecontent;

#define CREATE_INFO_FRAME_CTL_BYTE(S) (S << 6) /* Create INFO Control byte with the chosen S value */
#define GET_INFO_FRAME_CTL_BIT(b) (b >> 6) /*Get the sequence bit from a INFO control byte*/
#define IS_INFO_CONTROL_BYTE(b) ((b & 0xBF) == 0) /*Check if a control byte is an INFO control byte*/
#define INFO_FRAME_SIZE_WITHOUT_DATA 5 /*Size of a information frame without data (FLAG,CTL,ADDR,BYTE,FLAG)*/

#define CREATE_RR_FRAME_CTL_BYTE(R) (((R) << 7) | CTL_RR) /* Create RR Control byte with chosen R value */
#define CREATE_REJ_FRAME_CTL_BYTE(R) (((R) << 7) | CTL_REJ) /* Create REJ Control byte with chosen R value */
#define GET_RESPONSE_FRAME_CTL_BIT(b) (b >> 7) /* Get the R bit from a RR/REJ Control byte */
#define RESPONSE_CTL_MASK 0b00000111 /*Mask that removes the R bit from RR/REJ*/
#define APPLY_RESPONSE_CTL_MASK(b) (b & RESPONSE_CTL_MASK) /*Short function to apply the previous mask*/


#endif
