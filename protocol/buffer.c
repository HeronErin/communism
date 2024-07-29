#include "buffer.h"
#include "stdint.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

BUFF* makeBuff(size_t size, size_t index){
    BUFF* buff = (BUFF*) malloc(size + sizeof(BUFF));
    buff->index = index;
    buff->size = size;
    buff->reserved = size;
    return buff;
}
BUFF* remakeBuff(BUFF* buff, size_t size, size_t index){
    if (buff->reserved > size){
        buff->size = size;
        return buff;
    }
    buff = (BUFF*) realloc(buff, size + sizeof(BUFF));
    buff->index = index;
    buff->size = size;
    buff->reserved = size;
    return buff;
}


int extendFor(BUFF** buff, size_t newIndex) {
    if (buff == NULL || *buff == NULL) {
        errno = EINVAL;
        return -1;
    }

    BUFF* bf = *buff;
    if (newIndex >= bf->reserved) {
        size_t newReserved = newIndex * 2;
        BUFF* newBuff = (BUFF*) realloc(bf, newReserved + sizeof(BUFF));
        
        if (newBuff == NULL) {
            errno = ENOMEM;
            return -1;
        }

        newBuff->reserved = newReserved;
        newBuff->size = newIndex;
        bf = newBuff;
        *buff = bf;
    }
    if (newIndex > bf->size)
        bf->size = newIndex;

    return 0;
}
int writeByte(BUFF** buff, uint8_t b){
    if (0 != extendFor(buff, (*buff)->index + 1))
        return -1;
    BUFF* bf = *buff;
    bf->data[bf->index++] = b;
    if (bf->index > bf->size)
        bf->size = bf->index;
    return 0;
}

BUFF* quickBuff(size_t size, const uint8_t* data){
    BUFF* buff = makeBuff(size, 0);
    memcpy(buff->data, data, size);
    return buff;
}
