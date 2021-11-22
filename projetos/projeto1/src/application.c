#include "application.h"
#include "interface.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    int output_fd = llopen(1, EMITTER); // todo remove hard coded port
    int input_fd = open(argv[2], 2); // TODO This may stupid

    // --- Create control packet --- //
    control_packet ctl_packet;
    ctl_packet.control = CTL_BYTE_START;

    struct stat statbuf;
    if(fstat(input_fd, &statbuf) < 0){
        return -1;
    }
    uint64_t size_value = statbuf.st_size;
    uint64_t total_packets = size_value / ((uint64_t) MAX_PACKET_DATA_SIZE);
    total_packets += (size_value % ((uint64_t) MAX_PACKET_DATA_SIZE)) == 0 ? 0 : 1;
    if(control_packet_fill_parameter(&ctl_packet, 0, SIZE, 8, (uint8_t *) &size_value) < 0){
        return 1;
    }
    if(control_packet_fill_parameter(&ctl_packet, 1, NAME, strlen(argv[2])+1, argv[2]) < 0){  // TODO: Could strlen be dangerous?
        return 1;
    }
    write_control_packet(output_fd, &ctl_packet);
    // ---


    int read_res = 0;
    uint8_t sequence = 0;
    data_packet dt_packet; // TODO: name could be better
    while((read_res = read(input_fd, &(dt_packet.data), MAX_PACKET_DATA_SIZE)) > 0){
		dt_packet.sequence_number = sequence++;
        dt_packet.L2 = read_res / 256;
        dt_packet.L1 = read_res % 256;
        printf("Write DATA packet [%d/%ld]\n", sequence, total_packets);
        write_data_packet(output_fd, &dt_packet);
	}

    ctl_packet.control = CTL_BYTE_END;
    write_control_packet(output_fd, &ctl_packet);
    llclose(output_fd);
    close(input_fd);
    return 0;
}

int receiver(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
    int input_fd = llopen(0, RECEIVER); // TODO: remove hardcoded port

    int output_fd;
    if(argc == 3){
        output_fd = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    }

	uint8_t buffer[MAX_PACKET_SIZE]; 
    int read_res = 0;
    int sequence = 0;
    uint64_t total_packets = 0;

    bool started = false;
    while((read_res = llread(input_fd, buffer)) > 0){
        int buf_pos = 0;
        uint8_t control_byte = buffer[buf_pos++];

        if(control_byte == CTL_BYTE_START || control_byte == CTL_BYTE_END){
            if(!started){
                printf("Received START packet\n");
            } else {
                printf("Received END packet\n");
            }

            for(int i = 0; i < CONTROL_PACKET_PARAMETER_COUNT; ++i){
                uint8_t type = buffer[buf_pos++];
                uint8_t len = buffer[buf_pos++];
                uint8_t value[len];
                memcpy(value, buffer + buf_pos, len);
                if(type == NAME && argc == 2){
                    output_fd = open(value, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                } 
                if(len == 8 && type == SIZE){
                    uint64_t size_value;
                    memcpy(&size_value, value, 8);
                    printf("  parameter T=%d L=%d V=%ld\n", type, len, size_value);
                    total_packets = size_value / ((uint64_t) MAX_PACKET_DATA_SIZE);
                    total_packets += (size_value % ((uint64_t) MAX_PACKET_DATA_SIZE)) == 0 ? 0 : 1;
                } else {
                    printf("  parameter T=%d L=%d V=%s\n", type, len, value);
                }
                buf_pos += len;
            }
            if(started){
                break;
            }
            started = true;
        } else if (control_byte == CTL_BYTE_DATA){
            uint8_t packet_sequence_number = buffer[buf_pos++];
            if(sequence != packet_sequence_number){
                return 1;
            }
            sequence++;
            uint8_t L2 = buffer[buf_pos++];
            uint8_t L1 = buffer[buf_pos++];

            write(output_fd, buffer+buf_pos, (L2*256)+L1);
            printf("Received DATA packet [%d/%ld]\n", sequence, total_packets);
        } else {
            printf("error\n");
            return 1;
        }
    }
    llclose(input_fd);
    close(output_fd);
    return 0;
}

void print_usage(char *name){
    printf("Usage:\n");
    printf("    %s emitter input_filename\n", name);
    printf("    %s receiver\n", name);
    printf("    %s receiver output_filename\n", name);
}

int main(int argc, char *argv[]){
    if(argc < 2){
        print_usage(argv[0]);
        return 1;
    }

    if(strcmp(argv[1],"emitter") == 0){
        if(argc != 3){
            print_usage(argv[0]);
            return 1;
        }
        emitter(argc, argv);
        return 0;
    }
    if(strcmp(argv[1], "receiver") == 0){
        if(argc > 3){
            print_usage(argv[0]);
        }
        receiver(argc, argv);
        return 0;
    }
    return 1;
}