//
// Created by kai on 21/01/23.
//
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "net.h"

server_t* server;

static void handle_exit() {
    printf("\nCTRL+C pressed, exiting...");
    close(server->socket);
    exit(0);
}

int ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


static void handle_sigio() {
    struct sockaddr_storage their_addr;
    unsigned int size;
    int sock = accept(server->socket, (struct sockaddr *) &their_addr, &size);

    char* str = (char*)calloc(65536, sizeof(char));

    recv(sock, str, 65536, 0);

    char* tok = strtok(str, "\n");

    char method[4];
    char fname[256];
    char version[4];

    sscanf(tok, "%s %s HTTP/%s", method, fname, version);

    printf("Got %s %s HTTP/%s", method, fname, version);

    char* response = (char*)calloc(65536, sizeof(char));

    if (strcmp(fname, "/") == 0) {
        strcpy(fname, "index.html");
    }

    if (!ends_with(fname, ".html")) {
        strcat(fname, ".html");
    }

    FILE* file = fopen(&fname[1], "r");

    if (file == NULL) {
        strcpy(response, "HTTP/1.1 500 Internal Server Error");
    } else {
        char data[65536];
        fread(data, sizeof(*data), 65536, file);

        sprintf(response,
                "HTTP/1.1 200 OK\n"
                "Content-Length: %lu\n"
                "Content-Type: text/html\n"
                "Connection: close\n"
                "\n"
                "%s\n",
                strlen(data), data);
    }

    send(sock, response, strlen(response), 0);
    close(sock);

    free(str);
    free(response);
}

static void handle_signal(int signal) {
    switch (signal) {
        case SIGIO:
            handle_sigio();
            break;
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
            handle_exit();
            break;
        default:
            break;
    }
}

bool net_init(char* ip, int port) {
    server = (server_t*)calloc(1, sizeof(server_t));

    struct sigaction new_action;

    new_action.sa_handler = handle_signal;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaddset(&new_action.sa_mask, SIGIO);
    sigaddset(&new_action.sa_mask, SIGINT);
    sigaddset(&new_action.sa_mask, SIGTERM);
    sigaddset(&new_action.sa_mask, SIGKILL);

    sigaction(SIGIO, &new_action, NULL);
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGKILL, &new_action, NULL);

    server->port = port;
    server->socket = socket(AF_INET, SOCK_STREAM, 0);

    fcntl(server->socket, F_SETFL, O_ASYNC | O_NONBLOCK);
    fcntl(server->socket,F_SETOWN, getpid());

    int opt = 1;

    setsockopt(server->socket, SOL_SOCKET,
               SO_REUSEADDR | SO_REUSEPORT, &opt,
               sizeof(int));

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    bind(server->socket, (struct sockaddr *) &addr, sizeof(addr));
    listen(server->socket, 32);

    return true;
}