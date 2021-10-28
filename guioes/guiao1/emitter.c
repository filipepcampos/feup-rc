/*Non-Canonical Input Processing*/
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int verifyargv(int argc, char** argv){
  return (argc < 2) || ((strcmp("/dev/ttyS10", argv[1])!=0) && (strcmp("/dev/ttyS11", argv[1])!=0));
}

int main(int argc, char** argv)
{
  int fd, res;
  struct termios oldtio,newtio;
  char buf[255];
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
  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused (in deciseconds) */
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

  fgets(buf, 255, stdin); // Ler até buffer_size - 1 chars
  int datalen = strlen(buf);
  buf[datalen+1] = '\0';
  res = write(fd, buf, datalen+1);
  printf("%d bytes written\n", res);

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) { // Restaurar settings antigas do fd
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
