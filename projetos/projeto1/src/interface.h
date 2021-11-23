#ifndef __INTERFACE_
#define __INTERFACE_


#define MAX_INFO_SIZE 512
#define MAX_SIZE (MAX_INFO_SIZE*2 + 2)

typedef struct {
    int fileDescriptor; // Descritor correspondente à porta série
    int status; // TRANSMITTER | RECEIVER
} applicationLayer;

typedef enum{
    EMITTER,
    RECEIVER
} flag_t;

int llopen(int port, flag_t flag);
int llwrite(int fd, uint8_t * buffer, int length);
int llread(int fd, uint8_t * buffer);
int llclose(int fd);

#endif