#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#include "url_parser.h"
#include "queue.h"
#include "server_commands.h"

// ftp://[<user>:<password>@]<host>/<url-path>
#define FTP_PORT 21
#define REPLY_FILE_OK 150
#define REPLY_COMMAND_OK 200
#define REPLY_SERVICE_READY 220
#define REPLY_CLOSING_DATA 226
#define REPLY_ENTERING_PASV 227
#define REPLY_LOGGED_IN 230
#define REPLY_REQUIRE_PASSWORD 331
#define REPLY_ANONYMOUS_ONLY 530

typedef struct {
    in_addr_t address;
    int port;
} connection_info;

int print_usage(char *program_name){
    printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", program_name);
    return 0;    
}

int print_parameters(ftp_information *ftp_info, struct hostent *host){
    printf("\n-----------------------------------\n");
    printf("Parameters:\n");
    if(ftp_info->anonymous){
        printf("  host : %s\n  url_path : %s\n", ftp_info->host, ftp_info->url_path);
    } else {
        printf("  user : %s\n  password : %s\n  host : %s\n  url_path : %s\n", ftp_info->user, ftp_info->password, ftp_info->host, ftp_info->url_path);
    }
    printf("  host_name  : %s\n", host->h_name);
    printf("  ip_address : %s\n", inet_ntoa(*((struct in_addr *) host->h_addr)));
    printf("-----------------------------------\n\n");
    return 0;
}

// Open socket with given ip address / port and return it's file descriptor
int init_socket(in_addr_t *ip_addr, unsigned int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    server_addr.sin_addr.s_addr = *ip_addr;    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }
    return sockfd;
}

// Parse line and get it's reply code
int get_reply_code(char *line){
    if(strlen(line) >= 3){
        char replyCode[3];
        memcpy(replyCode, line, 3);
        return atoi(replyCode);
    }
    return -1;    
}

// Read FTP server replies
int readFTP(int fd, reply_queue *reply_queue){
    size_t read_size = 0;
    char buf[1024]; // TODO: Verify size
    read_size = read(fd, &buf, 1024);
    if(read_size > 0){
        buf[read_size] = 0;
        char *line = strtok(buf, "\n");
        while(line != NULL){
            ftp_reply reply;
            reply.message = malloc((sizeof(char))*strlen(line));
            strcpy(reply.message, line);
            
            reply.code = get_reply_code(line);
            enqueue(reply_queue, reply);
            line = strtok(NULL, "\n");
        }
        return 0;
    }
    return -1;
}

// Read data from the given file descriptor into a file
int read_data_to_file(int fd, char *filename){
    int outputfd = open(filename, O_WRONLY | O_CREAT, 0666); // TODO: Maybe change ?
    if(outputfd < 0){
        perror("open()");
        exit(-1);
    }

    char buffer[4098];
    size_t bytes_read;
    while((bytes_read = read(fd, buffer, 4098)) > 0){
        buffer[bytes_read] = 0;
        write(outputfd, buffer, bytes_read);
    }
}

// Parse entering passive mode response and return ip address + port 
connection_info parse_pasv_ip(char *message){
    int h1,h2,h3,h4;
    int p1,p2;
    sscanf(message, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &h1, &h2, &h3, &h4, &p1, &p2);

    connection_info info;
    info.port = p1*256 + p2;
    info.address = (h4 << 24) | (h3 << 16) | (h2 << 8)| h1;
    return info;
}

// Run client connection and output to file
int run_client(connection_info *connection_info){
    int fd = init_socket(&connection_info->address, connection_info->port);
    
    if(read_data_to_file(fd, "output") < 0){
        exit(-1);
    }

    if(close(fd) < 0){
        perror("close()");
        exit(-1);
    }
    exit(0);
}

// Create a fork and run the client
int fork_and_run_client(connection_info *connection_info){
    if(fork() == 0){
        return run_client(connection_info);
    }
    return 0;
}

// Setup data connection by sending appropriate command to server and starting the client
int open_data_connection(char *reply_message, int fd, char *url_path){
    retrieve_file(fd, url_path);
    connection_info client_connection_info = parse_pasv_ip(reply_message);
    fork_and_run_client(&client_connection_info);
    return 0;
}

// Check reply and do appropriate action
int handle_reply(ftp_reply reply, int fd, ftp_information *ftp_info){
    printf("%s\n", reply.message);
    bool connection_finished = false;
    switch(reply.code){
        case REPLY_REQUIRE_PASSWORD:
            send_password(fd, ftp_info);
            break;
        case REPLY_LOGGED_IN: 
            activate_passive_mode(fd); 
            break;
        case REPLY_ENTERING_PASV:
            open_data_connection(reply.message, fd, ftp_info->url_path);
            break;
        case REPLY_ANONYMOUS_ONLY:
        case REPLY_CLOSING_DATA:
            connection_finished = true;
            break;
    }
    free(reply.message);
    return connection_finished;
}

// Run server connection
int run_server(int fd, ftp_information *ftp_info){
    send_user(fd, ftp_info); 
    
    bool connection_finished = false;
    reply_queue reply_queue = create_queue(16);
    while(!connection_finished && readFTP(fd, &reply_queue) >= 0){
        while(!is_empty(&reply_queue)){
            ftp_reply reply = dequeue(&reply_queue);
            connection_finished = handle_reply(reply, fd, ftp_info);
        }
    }
    return 0;
}

int download(ftp_information *ftp_info){
    struct hostent *host;
    if ((host = gethostbyname(ftp_info->host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    print_parameters(ftp_info, host);
    int serverfd = init_socket((in_addr_t *) host->h_addr, FTP_PORT);

    if(run_server(serverfd, ftp_info) < 0){
        return -1;
    }

    if (close(serverfd) < 0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 2){
        printf("Insuficient arguments\n");
        print_usage(argv[0]);
        return -1;
    }
    ftp_information ftp_info;
    if(parse_url(argv[1], &ftp_info) != 0){
        printf("Invalid URL\n");
        print_usage(argv[0]);
        return -1;
    }

    return download(&ftp_info);
}

