#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define FLAG 0x7e
#define ADDRESS1 0x03 // TODO: Change name
#define ADDRESS2 0x01
#define CTL_SET 0x03
#define CTL_UA  0x07


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

typedef struct {
    uint8_t address;
    uint8_t control;
} framecontent;

int create_frame(char *buffer, size_t buffer_size, framecontent *fc){
    if(buffer_size < 5){
        return -1;
    }
    buffer[0] = FLAG;
    buffer[1] = fc->address;
    buffer[2] = fc->control;
    buffer[3] = (fc->address) ^ (fc->control);
    buffer[4] = FLAG;
}

int send_frame(int fd, char *frame, int frame_size){
    int res = write(fd, frame, frame_size); // TODO: Maybe we could add some error handling
    printf("%d bytes written\n", res);
    return 0;
}

int receive_frame(int fd, char *buffer, int buffer_size){
    bool STOP = false;
    while (STOP==false) {       /* loop for input */
        int res = read(fd,buffer,buffer_size);   /* returns after 5 chars have been input */
        printf("> %s %d bytes read\n", buffer, res);
        if (buffer[res-1]=='\0') STOP=true;
    }
    return 0;
}

int emitter(int fd){
    framecontent fc; //TODO Change name
    fc.address = ADDRESS1; // TODO: Confirmar
    fc.control = CTL_SET;
    char buffer[6];
    buffer[5] = '\0'; // TODO: Remove
    create_frame(buffer, 5, &fc);
    send_frame(fd, buffer, 6);
}

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} receiver_state;

receiver_state statemachine_start(uint8_t byte){
    printf("START\n");
    if(byte == FLAG){
        return FLAG_RCV;
    }
    return START;
}

receiver_state statemachine_flag(uint8_t byte){
    printf("FLAG\n");
    if(byte == FLAG){
        return FLAG_RCV;
    }
    else if(byte == ADDRESS1 || byte == ADDRESS2){ // TODO: Maybe verify this
        return A_RCV;
    }
    return START;
}

bool valid_ctl_address(uint8_t byte){
    printf("CTL\n");
    if(byte == CTL_SET || byte == CTL_UA){
        return true;
    }
    return false;
}

receiver_state statemachine_addressrcv(uint8_t byte){
    printf("ADDR\n");
    if(byte == FLAG){
        return FLAG_RCV;
    }
    if(valid_ctl_address(byte)){
        return C_RCV;
    }
    return START;
}

receiver_state statemachine_cRcv(uint8_t byte, framecontent *fc){
    printf("Control RCV\n");
    if(byte == FLAG){
        return FLAG_RCV;
    }
    if((fc->control) ^ (fc->address) == byte){
        return BCC_OK;
    }
    return START;
}

receiver_state statemachine_bcc(uint8_t byte){
    printf("BCC\n");
    if(byte == FLAG){
        return STOP;
    }
    return START;
}


int receiver(int fd){
    char buffer[255];
    receiver_state state = START;
    framecontent fc;

    while (state != STOP) {       /* loop for input */
        int res = read(fd,buffer,255);   /* returns after 5 chars have been input */
        printf("DEBUG> %s %d bytes read\n", buffer, res);
        for(int i = 0; i < res; ++i){
            uint8_t byte = buffer[i];
            switch(state){
                case START: state = statemachine_start(byte); break;
                case FLAG_RCV: state = statemachine_flag(byte); break;
                case A_RCV: 
                    state = statemachine_addressrcv(byte);
                    fc.address = byte;
                    break;
                case C_RCV: 
                    state = statemachine_cRcv(byte, &fc); 
                    fc.control = byte;
                    break;
                case BCC_OK: state = statemachine_bcc(byte); break;
            }
        }
    }
    printf("Stopping now\n");
    return 0;
}



int verifyargv(int argc, char** argv){
  return (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0));
}

int main(int argc, char** argv)
{
  int fd, res;
  struct termios oldtio,newtio;
  char outbuf[255], inbuf[255];
  int i, sum = 0, speed = 0;
  
  if (verifyargv(argc, argv)) {
    printf("Usage:\t%s SerialPort\n\tex: %s /dev/ttyS10\n", argv[0], argv[0]);
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio)); // Set contents of newtio to zero.
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;  /*
      BAUDRATE => B38400, I don't understand this #TODO
      CS8 => Character size mask
      CLOCAL => Ignore modem control lines
      CREAD => Enable receiver
  */
  newtio.c_iflag = IGNPAR; // Ignore framing errors and parity errors
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;
  newtio.c_cc[VTIME]    = 5;   /* inter-character timer unused (in deciseconds) */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH); // Clear the data that might be present in the fd

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) { // TCSANOW -> set attr takes place immediately
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  receiver(fd);

/*
  if(read(fd, inbuf, 255) == res && strcmp(inbuf, outbuf) == 0){
    printf("Success\n");
  }
  else {
    perror("failure");
    exit(-1);
  }
  */

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) { // Restaurar settings antigas do fd
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
