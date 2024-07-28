#pragma once


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
    TCP_ACTION (*HandleClosedConnection)(void* worldState, void** playerSate, int fd);
    TCP_ACTION (*HandlePlayerConnectionError)(void* worldState, void** playerSate, int fd);
} TcpServerOutline;


int bindServer(void* worldState, TcpServerOutline outline);