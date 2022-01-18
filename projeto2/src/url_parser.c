#include "url_parser.h"

#include <stdlib.h>
#include <string.h>

// Check if url starts with 'ftp://', returns string without prefix if true, null otherwise.
char *check_protocol(char *url){
    if(strncmp(url, "ftp://", 6) != 0){
        return NULL;
    }
    url += 6;
    return url;
}

// Parse password
char *parse_password(char *url, ftp_information *ftp){
    size_t password_size = strcspn(url, "@");
    if(password_size == strlen(url)){
        return NULL;
    } else {
        ftp->password = malloc(sizeof(char)*password_size);
        memcpy(ftp->password, url, password_size);
        url += password_size+1;
        return url;
    }
}

// Parse username and password, removing them from the string if found
char *parse_login(char *url, ftp_information *ftp){
    size_t username_size = strcspn(url, ":");
    if(strlen(url) == username_size){ // There's no ':' character, therefore there's no username.
        ftp->anonymous = true;
    } else {
        ftp->anonymous = false;
        ftp->user = malloc(sizeof(char)*username_size);
        memcpy(ftp->user, url, username_size);
        url += username_size + 1;
        url = parse_password(url, ftp);
        
        if(url == NULL){ // Username defined but no password error
            free(ftp->user);
            return NULL;
        }
    }
    return url;
}

char *parse_host(char *url, ftp_information *ftp){
    size_t host_size = strcspn(url, "/");
    if(host_size == strlen(url)){
        return NULL;
    } else {
        ftp->host = malloc(sizeof(char)* host_size);
        memcpy(ftp->host, url, host_size);
        url += host_size;
    }
    return url;
}

int parse_url(char *url, ftp_information *ftp){
    if((url = check_protocol(url)) == NULL){
        return -1;
    }

    if((url = parse_login(url, ftp)) == NULL){
        return -1;
    }
    
    if((url = parse_host(url, ftp)) == NULL){
        if(!ftp->anonymous){
            free(ftp->user);
            free(ftp->password);
        }
        return -1;
    }
    ftp->url_path = url;
    return 0;
}