
#include <stdio.h>
#include "server.h"
#include "datatypes.h"
#include "buffer.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


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
            PacketFieldData* addr = searchFieldsForId(FID_NEXT_STATE, p, HanshakeC2S.size);
            st = addr->varIntData;
        }
    }else if(st == 1){
        if (type == 0){
            BUFF* out = makeBuff(0, 0);
            PacketFieldData* data = makeFieldDataPackageFor(StatusResponseS2C.fields, StatusResponseS2C.size);
            data[1].stringData = "{ \"version\": { \"name\": \"1.19.4\", \"protocol\": 762 }, \"players\": { \"max\": 100, \"online\": 5, \"sample\": [ { \"name\": \"thinkofdeath\", \"id\": \"4566e69f-c907-48ee-8d71-d7ba5aa00d20\" } ] }, \"description\": { \"text\": \"Hello, world!\" }, \"favicon\": \"data:image/png;base64,<data>\", \"enforcesSecureChat\": false }";
            printf("Sending\n");
            if (0!=sendPacketRaw(&StatusResponseS2C, data, fd))
                perror("Shite");
            else
                printf("");
            free(out);
        }
    }else{
        printf("State: %lu\n", st);
    }
    *(size_t*)playerSate = st;
    free(packet);
    if (errno != 0){
        perror("Error with player packet");
        errno = 0;
        return TCP_ACT_DISCONNECT_CLIENT;
    }
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