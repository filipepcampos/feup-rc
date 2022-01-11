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

// ftp://[<user>:<password>@]<host>/<url-path>
#define FTP_PORT 21
#define REPLY_COMMAND_OK 200
#define REPLY_SERVICE_READY 220
#define REPLY_REQUIRE_PASSWORD 331
#define REPLY_ENTERING_PASV 227
#define REPLY_LOGGED_IN 230
#define REPLY_FILE_OK 150
#define REPLY_CLOSING_DATA 226

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

int get_reply_code(char *line){
    if(strlen(line) >= 3){
        char replyCode[3];
        memcpy(replyCode, line, 3);
        return atoi(replyCode);
    }
    return -1;    
}

int readFTP(int fd, queue *reply_queue){
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

int activate_passive_mode(int fd){
    size_t bytes = dprintf(fd, "pasv\n");
    if (bytes <= 0) {
        perror("dprintf()");
        exit(-1);
    }
}

connection_info parse_pasv_ip(char *message){
    int h1,h2,h3,h4;
    int p1,p2;
    sscanf(message, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &h1, &h2, &h3, &h4, &p1, &p2);

    connection_info info;
    info.port = p1*256 + p2;
    info.address = (h4 << 24) | (h3 << 16) | (h2 << 8)| h1;
    return info;
}

int start_client_connection(char *message){
    connection_info client_connection_info = parse_pasv_ip(message);
    int clientfd = init_socket(&client_connection_info.address, client_connection_info.port);
    return clientfd;
}

int main(int argc, char **argv) {
    if(argc != 2){
        printf("Insuficient arguments\n");
        print_usage(argv[0]);
        return 1;
    }
    ftp_information ftp_info;
    if(parse_url(argv[1], &ftp_info) != 0){
        printf("Invalid URL\n");
        print_usage(argv[0]);
        return 1;
    }

    struct hostent *host;
    if ((host = gethostbyname(ftp_info.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    print_parameters(&ftp_info, host);
    int serverfd = init_socket((in_addr_t *) host->h_addr, FTP_PORT);

    // Log in to server
    size_t bytes;
    if(ftp_info.anonymous){
        bytes = dprintf(serverfd, "USER anonymous\nPASS anonymous\n");
    } else {
        bytes = dprintf(serverfd, "USER %s\nPASS %s\n", ftp_info.user, ftp_info.password);
    }    
    
    int clientfd;
    bool received_data = false;
    queue reply_queue = create_queue(16); // TODO: Change size?
    while(!received_data && readFTP(serverfd, &reply_queue) >= 0){
        while(!is_empty(&reply_queue)){
            ftp_reply reply = dequeue(&reply_queue);
            printf("%s\n", reply.message);
            switch(reply.code){
                case REPLY_LOGGED_IN: activate_passive_mode(serverfd); break;
                case REPLY_ENTERING_PASV:
                    clientfd = start_client_connection(reply.message);
                    dprintf(serverfd, "retr %s\n", ftp_info.url_path);
                    break;
                case REPLY_FILE_OK: received_data = true; break;
            }
            free(reply.message);
        }
    }

    printf("---------------------------\n");

    int outputfd = open("output", O_WRONLY | O_CREAT, 0666); // TODO: Maybe change ?
    if(outputfd < 0){
        perror("open()");
        exit(-1);
    }

    char buffer[1024];
    size_t bytes_read;
    while((bytes_read = read(clientfd, buffer, 1024)) > 0){
        buffer[bytes_read] = 0;
        printf("Read %ld bytes\n", bytes_read);
        printf("%s", buffer);
        write(outputfd, buffer, bytes_read);
    }

    if (close(serverfd)<0 || close(clientfd)<0 || close(outputfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}


