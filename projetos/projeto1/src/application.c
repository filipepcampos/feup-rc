#include "application.h"
#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


int write_control_packet(int fd, control_packet *packet){
    uint8_t buffer[MAX_PACKET_SIZE];
    buffer[0] = packet->control;

    int buf_pos = 1;
    for(int i = 0; i < CONTROL_PACKET_PARAMETER_COUNT; ++i){
        control_parameter parameter = (packet->parameters)[i];
        if(buf_pos + parameter.length > MAX_PACKET_SIZE){
            return 1;
        }
        buffer[buf_pos++] = parameter.type;
        buffer[buf_pos++] = parameter.length;
        memcpy(buffer+buf_pos, parameter.value, parameter.length);
        buf_pos += parameter.length;
    }
    llwrite(fd, buffer, buf_pos);
    return 0;
}

int write_data_packet(int fd, data_packet *packet){
    uint8_t buffer[MAX_PACKET_SIZE];
    buffer[0] = CTL_BYTE_DATA;
    buffer[1] = packet->sequence_number;
    buffer[2] = packet->L2;
    buffer[3] = packet->L1;
    size_t len = (packet->L2)*256 + (packet->L1);
    memcpy(buffer + 4, packet->data, len);
    llwrite(fd, buffer, len + 4);
    return 0;
}

int control_packet_fill_parameter(control_packet *packet, size_t parameter_index, parameter_type type, size_t length, uint8_t *value){
    if(parameter_index > CONTROL_PACKET_PARAMETER_COUNT){
        return -1;
    }
    packet->parameters[parameter_index].type = type;
    packet->parameters[parameter_index].length = length;
    packet->parameters[parameter_index].value = value;
    return 0;
}

int emitter(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
    int fd = llopen(1, EMITTER); // todo remove hard coded port
    int fd2 = open(argv[1], 2); // TODO This may stupid

    // --- Create control packet --- //
    control_packet ctl_packet;
    ctl_packet.control = CTL_BYTE_START;
    uint8_t size_value = 1;
    if(control_packet_fill_parameter(&ctl_packet, 0, SIZE, 1, &size_value) < 0){
        return 1;
    }
    if(control_packet_fill_parameter(&ctl_packet, 1, NAME, strlen(argv[1])+1, argv[1]) < 0){  // TODO: Could strlen be dangerous?
        return 1;
    }
    printf("ctl_param[1] (%d, %s)", ctl_packet.parameters[1].length, ctl_packet.parameters[1].value);
    write_control_packet(fd, &ctl_packet);
    // ---


    int read_res = 0;
    uint8_t sequence = 0;
    data_packet dt_packet; // TODO: name could be better
    while((read_res = read(fd2, &(dt_packet.data), MAX_PACKET_DATA_SIZE)) > 0){
		dt_packet.sequence_number = sequence++;
        dt_packet.L2 = read_res / 256;
        dt_packet.L1 = read_res % 256;
        printf("Write sequence=%d\n", sequence);
        write_data_packet(fd, &dt_packet);
	}

    ctl_packet.control = CTL_BYTE_END;
    write_control_packet(fd, &ctl_packet);
    llclose(fd);
    return 0;
}

int receiver(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
    int fd = llopen(0, RECEIVER); // TODO: remove hardcoded port
    int fd2 = open(argv[1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // TODO This may stupid

	uint8_t buffer[MAX_PACKET_SIZE]; 
    int read_res = 0;
    int sequence = 0;

    while((read_res = llread(fd, buffer)) > 0){
        int buf_pos = 0;
        uint8_t control_byte = buffer[buf_pos++];
        printf("Got control_byte=%02x (sequence=%d)\n", control_byte, sequence);

        if(control_byte == CTL_BYTE_START || control_byte == CTL_BYTE_END){
            printf("start/end\n");
            for(int i = 0; i < CONTROL_PACKET_PARAMETER_COUNT; ++i){
                uint8_t type = buffer[buf_pos++];
                uint8_t len = buffer[buf_pos++];
                uint8_t value[len];
                memcpy(value, buffer + buf_pos, len);
                printf("Got parameter T=%d L=%d V=%s\n", type, len, value);
                buf_pos += len;
            }
        } else if (control_byte == CTL_BYTE_DATA){
            uint8_t packet_sequence_number = buffer[buf_pos++];
            if(sequence != packet_sequence_number){
                return 1;
            }
            sequence++;
            uint8_t L2 = buffer[buf_pos++];
            uint8_t L1 = buffer[buf_pos++];

            write(fd2, buffer+buf_pos, (L2*256)+L1);
            printf("data\n");
        } else {
            printf("error\n");
            return 1;
        }
    }
    llclose(fd);
    return 0;
}

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("./%s filename emitter/receiver\n", argv[0]);
        return 1;
    }
    if(strcmp(argv[2],"emitter") == 0){
        emitter(argc, argv);
        return 0;
    }
    if(strcmp(argv[2], "receiver") == 0){
        receiver(argc, argv);
        return 0;
    }
    printf("./%s filename emitter/receiver\n", argv[0]);
    return 1;
}