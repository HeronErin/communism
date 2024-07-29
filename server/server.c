#include "server.h"
#include "descripterMap.h"
#include <stdlib.h>

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>

bool continueRunning = true;

int bindServer(const uint16_t port, const int maxConnections, void* worldState, TcpServerOutline outline){
    ssize_t valread;
    struct sockaddr_in address;
    fd_set fdset;
    int server_fd;

    int newClient;
    int maxfd;
    int selectCode;
    int i;
    TCP_ACTION action;

    int retVal = 0;
    int* clientSockets = calloc(1, sizeof(int) * maxConnections);
    DescriptorMap* playerStates = makeDescriptorMap(maxConnections);

    // Create the socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    socklen_t addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, maxConnections) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    #define HANDLE_TCP_ACTION_OF_CLIENT_AT_I(FD) switch (action) { \
            case TCP_ACT_DISCONNECT_CLIENT:\
                outline.HandleClosedConnection(&worldState, lookupElement(playerStates, FD), FD);\
                removeElement(playerStates, FD);\
                close(FD);\
            case TCP_ACT_REMOVE_CLIENT:\
                clientSockets[i] = 0;\
                break;\
            case TCP_ACT_KILL_SERVER:\
                goto CLEANUP;\
            case TCP_ACT_NOTHING:\
            default:\
                break;\
        }
    while (continueRunning){
        FD_ZERO(&fdset);
        FD_SET(server_fd, &fdset);
        maxfd = server_fd;
        for (i = 0; i < maxConnections; i++){
            int c = clientSockets[i];
            if (0==c) continue;
            FD_SET(c, &fdset);   
            if (c > maxfd)
                maxfd = c;
        }
        fd_set exceptfds;
        memcpy(&exceptfds, &fdset, sizeof(fd_set));
        
        selectCode = select(maxfd + 1, &fdset, NULL, &exceptfds, NULL);

        if (FD_ISSET(server_fd, &fdset)){
            if ((newClient = accept(server_fd,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
                perror("Error accepting new socket...");
            
            for (i = 0; i < maxConnections; i++){
                int c = clientSockets[i];
                if (0!=c) continue;
                clientSockets[i] = newClient;
                break;
            }
            void* playerState = NULL;
            action = outline.HandleNewConnection(&worldState, &playerState, newClient);
            setElement(playerStates, newClient, playerState);
            HANDLE_TCP_ACTION_OF_CLIENT_AT_I(newClient);
        }
        for (i = 0; i < maxConnections; i++){
            int c = clientSockets[i];
            if (0==c) continue;
            if (!FD_ISSET(c, &fdset)) continue;
            action = outline.HandleNewMessage(&worldState, lookupElement(playerStates, c), newClient);
            HANDLE_TCP_ACTION_OF_CLIENT_AT_I(c);
        }
    }

    CLEANUP:
    for (i = 0; i < maxConnections; i++){
        int c = clientSockets[i];
        if (0==c) continue;
        // Callbacks job to free player state (if any)
        outline.HandleClosedConnection(&worldState, lookupElement(playerStates, c), c);
        removeElement(playerStates, c);
        close(c);
    }
    close(server_fd);

    freeMap(playerStates);
    free(clientSockets);
    return retVal;
}

TCP_ACTION NO_CALLBACK(void* worldState, void** playerSate, int fd){
    return TCP_ACT_NOTHING;
}
TCP_ACTION CALLBACK_REMOVE(void* worldState, void** playerSate, int fd){
    return TCP_ACT_REMOVE_CLIENT;
}
TCP_ACTION CALLBACK_DISCONNECT(void* worldState, void** playerSate, int fd){
    return TCP_ACT_DISCONNECT_CLIENT;
}