#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

typedef void (*request_handler_t)(int client_fd, const char *req, void *user_data);

int start_server(uint16_t port, request_handler_t handler, void *user_data);

#endif