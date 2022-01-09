#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <stdbool.h>

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
    bool anonymous;
    char *user;
    char *password;
    char *host;
    char *url_path;
} ftp_information;

int parse_url(char *str, ftp_information *ftp){
    if(strncmp(str, "ftp://", 6) != 0){
        return 1;
    }
    str += 6;

    ftp->anonymous = true;
    size_t username_size = strcspn(str, ":");
    ftp->user = malloc(sizeof(char)*username_size);
    memcpy(ftp->user, str, username_size);

    if(strlen(str) == username_size){
        free(ftp->user);
    } else {
        str += username_size+1;
        size_t password_size = strcspn(str, "@");
        ftp->password = malloc(sizeof(char)*password_size);
        memcpy(ftp->password, str, password_size);
        str += password_size+1;
        if(password_size == strlen(str)){
            free(ftp->user);
            free(ftp->password);
            return 1;
        }
    }
    size_t host_size = strcspn(str, "/");
    ftp->host = malloc(sizeof(char)* host_size);
    memcpy(ftp->host, str, host_size);
    if(host_size == strlen(str)){
        if(!ftp->anonymous){
            free(ftp->user);
            free(ftp->password);
        }
        free(ftp->host);
        return 1;
    }
    str += host_size;
    ftp->url_path = str;
    return 0;
}

int print_usage(char *program_name){
    printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", program_name);
    return 0;    
}

int print_parameters(ftp_information *ftp_info, struct hostent *host){
    printf("\n-----------------------------------\nParameters:\n");
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
    
    size_t read_size = 0;
    char buf[1024]; // TODO: Verify size
    while((read_size = read(serverfd, &buf, 1024)) > 0){
        buf[read_size] = 0;
        printf("%s", buf);
        if(strstr(buf, "230") != NULL){
            break;
        }
    }

    bytes = dprintf(serverfd, "pasv\n");
    if (bytes <= 0) {
        perror("dprintf()");
        exit(-1);
    }

    if((read_size = read(serverfd, &buf, 1024)) > 0){
        buf[read_size] = 0;
        int h1,h2,h3,h4;
        int p1,p2;
        sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &h1, &h2, &h3, &h4, &p1, &p2);
        printf("%s", buf);
        printf("Got %d.%d.%d.%d %d\n", h1,h2,h3,h4,p1*256+p2);
    } else {
        perror("read()");
        exit(-1);
    }

    if (close(serverfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}


