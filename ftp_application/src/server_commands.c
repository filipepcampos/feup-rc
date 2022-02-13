#include "server_commands.h"
#include <stdlib.h>
#include <stdio.h>

// Send pasv mode command to server
int activate_passive_mode(int fd){
    size_t bytes = dprintf(fd, "pasv\n");
    if(bytes < 0) {
        perror("dprintf()");
        exit(-1);
    }
}

// Retrieve file with url_path
int retrieve_file(int fd, char *url_path){
    size_t bytes = dprintf(fd, "retr %s\n", url_path);
    if(bytes <= 0){
        perror("dprintf()");
        exit(-1);
    }
    return 0;
}

// Send username command to server
int send_user(int fd, ftp_information *ftp_info){
    size_t bytes;
    if(ftp_info->anonymous){
        bytes = dprintf(fd, "USER anonymous\n");
    } else {
        bytes = dprintf(fd, "USER %s\n", ftp_info->user);
    } 
    if(bytes < 0){
        perror("dprintf()");
        exit(-1);
    }  
    return 0;
}

// Send password command to server
int send_password(int fd, ftp_information *ftp_info){
    size_t bytes;
    if(ftp_info->anonymous){
        bytes = dprintf(fd, "PASS anonymous\n");
    } else {
        bytes = dprintf(fd, "PASS %s\n", ftp_info->password);
    } 
    if(bytes < 0){
        perror("dprintf()");
        exit(-1);
    }  
    return 0;
}