#include <stdbool.h>

typedef struct {
    bool anonymous;
    char *user;
    char *password;
    char *host;
    char *url_path;
} ftp_information;

char *check_protocol(char *url);

char *parse_password(char *url, ftp_information *ftp);

char *parse_login(char *url, ftp_information *ftp);

char *parse_host(char *url, ftp_information *ftp);

int parse_url(char *str, ftp_information *ftp);