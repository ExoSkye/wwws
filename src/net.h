#ifndef WWWS_NET_H
#define WWWS_NET_H

#include <stdbool.h>

typedef struct server_t {
    int port;
    int socket;
} server_t;

extern server_t* server;

bool net_init(char* ip, int port);

#endif //WWWS_NET_H
