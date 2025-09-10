#include <stdio.h>
#include <string.h>
#include <unistd.h>  

#include "config.h"
#include "sensors.h"
#include "server.h"

static void dump_sensor_to_fd(int fd, const sensor_cfg_t *s) {
    dprintf(fd, "id=%s, unit=%s, min=%.2f, max=%.2f, period_ms=%u\n",
            s->id ? s->id : "",
            s->unit ? s->unit : "",
            s->min, s->max, s->period_ms);
}

static int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static void router(int client_fd, const char *req, void *user_data) {
    const car_sensors_t *cfg = (const car_sensors_t*)user_data;

    if (starts_with(req, "GET /health")) {
        dprintf(client_fd,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Connection: close\r\n"
                "\r\n"
                "ok\n");
        return;
    }

    if (starts_with(req, "GET /sensors")) {
        
        dprintf(client_fd,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Connection: close\r\n"
                "\r\n");

        
        dprintf(client_fd, "Loaded %d sensors:\n", cfg->count);
        for (int i = 0; i < cfg->count; i++) {
            dump_sensor_to_fd(client_fd, &cfg->s[i]);
        }
        return;
    }

    dprintf(client_fd,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "not found\n");
}

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "config/sensors.yaml";

    car_sensors_t cfg = {0};
    if (load_config(path, &cfg) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return 1;
    }

    return start_server(8080, router, &cfg);
}
