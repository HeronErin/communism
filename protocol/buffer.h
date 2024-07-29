#pragma once
#include "stdint.h"
#include <stddef.h>
#include <stdlib.h>


typedef struct {
    size_t reserved;
    size_t size;
    size_t index;
    uint8_t data[];
} BUFF; 

BUFF* makeBuff(size_t size, size_t index);
BUFF* remakeBuff(BUFF* buff, size_t size, size_t index);

BUFF* quickBuff(size_t size, const uint8_t* data);
int extendFor(BUFF** buff, size_t newIndex);
int writeByte(BUFF** buff, uint8_t b);

#define MK_BUFF(XXXX) quickBuff(sizeof(XXXX), (XXXX))
