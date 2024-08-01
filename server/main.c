
#include <stdio.h>
#include "server.h"
#include "datatypes.h"
#include "buffer.h"
#include <unistd.h>
#include <stdlib.h>


#include "packets.h"

TCP_ACTION bounce(void* worldState, void** playerSate, int fd){
    *(size_t*)playerSate = 0;
    return TCP_ACT_NOTHING;
}
TCP_ACTION packet(void* worldState, void** playerSate, int fd){
    size_t st = *(size_t*)playerSate;
    uint32_t size, type; 
    decodeVarIntFromFd(fd, &size);
    printf("Packet of %d size\n", size);
    if (size > 32000){
        printf("Packet too large, disconnect\n");
        return TCP_ACT_DISCONNECT_CLIENT;
    }
    uint8_t* packet = malloc(size);
    read(fd, packet, size);
    BUFF* b = quickBuff(size, packet);
    decodeVarInt(b, &type);

    printf("Packet of %d type in mode %lu\n", type, st);
    if (st == 0){
        if (type == 0){
            PacketFieldData* p = decodePacket(HanshakeC2S.fields, HanshakeC2S.size, b);
            if (p == NULL) return TCP_ACT_DISCONNECT_CLIENT;
            PacketFieldData* addr = searchFieldsForId(FID_SERVER_ADDRESS, p, HanshakeC2S.size);
            printf("Addr: %llu: %s\n", (size_t)addr, addr->stringData);

        }
    }else{
        printf("State: %lu\n", st);
    }
    free(packet);
    return TCP_ACT_NOTHING;
}

const TcpServerOutline  HTTP = {

    bounce,
    packet,
    CALLBACK_REMOVE,
    CALLBACK_REMOVE
};

int main(){
    return bindServer(1234, 5, NULL, HTTP);
}