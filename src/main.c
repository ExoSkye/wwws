#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "net.h"

int main() {
    net_init("127.0.0.1", 8080);

    while(1) pause();
}
