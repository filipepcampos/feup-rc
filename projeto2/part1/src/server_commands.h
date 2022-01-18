#ifndef __server_commands__
#define __server_commands__

#include "url_parser.h"

int activate_passive_mode(int fd);
int retrieve_file(int fd, char *url_path);
int send_user(int fd, ftp_information *ftp_info);
int send_password(int fd, ftp_information *ftp_info);

#endif