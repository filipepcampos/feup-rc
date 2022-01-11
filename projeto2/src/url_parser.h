#include <stdbool.h>

typedef struct {
    bool anonymous;
    char *user;
    char *password;
    char *host;
    char *url_path;
} ftp_information;

int parse_url(char *str, ftp_information *ftp);