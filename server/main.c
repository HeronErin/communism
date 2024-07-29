#include <stdio.h>
#include "server.h"
#include <unistd.h>
const char resp[] = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\nContent-Type: text/plain; charset=utf-8\r\n\r\nHello World!";

TCP_ACTION incomingHttp(void* worldState, void** playerSate, int fd){
    char data[1024*10] = {0};
    // printf("Hello new \n");
    read(fd, &data, 1024*10);
    write(fd, resp, sizeof(resp));

    return TCP_ACT_DISCONNECT_CLIENT;
}

const TcpServerOutline  HTTP = {

    incomingHttp,
    NO_CALLBACK,
    CALLBACK_REMOVE,
    CALLBACK_REMOVE
};

int main(){
    return bindServer(1234, 5, NULL, HTTP);
}