#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BACKLOG 10
#define REQ_BUF 4096

static void write_500(int client_fd) {
    const char *body = "internal error\n";
    char resp[256];
    int n = snprintf(resp, sizeof(resp),
                     "HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     "%s",
                     strlen(body), body);
    (void)write(client_fd, resp, n);
}

int start_server(uint16_t port, request_handler_t handler, void *user_data) {
    int server_fd = -1, opt = 1;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt"); close(server_fd); return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    printf("Server listening on port %u...\n", (unsigned)port);

    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }

        char *buf = (char*)calloc(1, REQ_BUF);
        if (!buf) { write_500(client_fd); close(client_fd); continue; }

        ssize_t r = read(client_fd, buf, REQ_BUF - 1);
        if (r <= 0) { free(buf); close(client_fd); continue; }
        buf[r] = '\0';

        if (handler) handler(client_fd, buf, user_data);
        else write_500(client_fd);

        free(buf);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
