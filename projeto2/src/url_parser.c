#include "url_parser.h"

#include <stdlib.h>
#include <string.h>

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