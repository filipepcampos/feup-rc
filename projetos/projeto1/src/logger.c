#include "logger.h"
#include <stdbool.h>
#include <stdio.h>

void log_control_byte(uint8_t byte){
	switch(byte){
		case CTL_SET : printf("SET"); return;
		case CTL_UA : printf("UA"); return;
		case CTL_DISC : printf("DISC"); return;
	}
	if(IS_INFO_CONTROL_BYTE(byte)){
		printf("INFO");
		return;
	}
	else if(APPLY_RESPONSE_CTL_MASK(byte) == _CTL_RR){
		printf("RR");
		return;
	}
	else if(APPLY_RESPONSE_CTL_MASK(byte) == _CTL_REJ) {
		printf("REJ");
		return;
	}
	printf("INVALID");
	return;
}
void log_address_byte(uint8_t byte){
	switch(byte){
		case ADDRESS1 : printf("ADRESS1"); break;
		case ADDRESS2 : printf("ADDRESS2"); break;
		default: printf("INVALID"); break;
	}
}
void log_emission(framecontent *fc){
    if(VERBOSE == false){
        return;
    }
	printf(" emit: CTL=");
	log_control_byte(fc->control);
	printf(" ADR=");
	log_address_byte(fc->address);
	if(IS_INFO_CONTROL_BYTE(fc->control)){
		printf(" INFO=\n");
		for(int i = 0;  i < fc->data_len; ++i){
			printf(" %02x ", fc->data[i]);
		}
		printf("\n");
		printf(" S=%d", GET_INFO_FRAME_CTL_BIT(fc->control));
	}
	printf("\n\n");
}
void log_receival(framecontent *fc){
    if(VERBOSE == false){
        return;
    }
	printf(" receive: CTL=");
	log_control_byte(fc->control);
	printf(" ADR=");
	log_address_byte(fc->address);
	if(IS_INFO_CONTROL_BYTE(fc->control)){
		printf(" INFO=\n");
		for(int i = 0;  i < fc->data_len; ++i){
			printf(" %02x ", fc->data[i]);
		}
		printf("\n");
		printf(" S=%d", GET_INFO_FRAME_CTL_BIT(fc->control));
	}
	printf("\n\n");
}