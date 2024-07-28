#pragma once

#include <stdbool.h>
#include <stdint.h>

// Actions a callback can command a server to take
// after responding to a client event
typedef enum{
    TCP_ACT_NOTHING,
    TCP_ACT_DISCONNECT_CLIENT,
    TCP_ACT_REMOVE_CLIENT,
    TCP_ACT_KILL_SERVER
} TCP_ACTION;

// This socket provides a LOW level control over what is done
// for specific sockets. This is a const object for what functions
// are handling what part of the connection process.
typedef struct {
    TCP_ACTION (*HandleNewConnection)(void* worldState, void** playerSate, int fd);
    TCP_ACTION (*HandleNewMessage)(void* worldState, void** playerSate, int fd);
    TCP_ACTION (*HandleClosedConnection)(void* worldState, void** playerSate, int fd);
    TCP_ACTION (*HandlePlayerConnectionError)(void* worldState, void** playerSate, int fd);
} TcpServerOutline;


extern bool continueRunning;
int bindServer(const uint16_t port, const int maxConnections, void* worldState, TcpServerOutline outline);

TCP_ACTION NO_CALLBACK(void* worldState, void** playerSate, int fd);
TCP_ACTION CALLBACK_REMOVE(void* worldState, void** playerSate, int fd);
TCP_ACTION CALLBACK_DISCONNECT(void* worldState, void** playerSate, int fd);