#pragma once
#include "buffer.h"


// Types used for sub fields 
typedef void FREE_FUNCTION(void* field);
typedef int ENCODE_FUNCTION(BUFF** buff, void* field);
typedef int DECODE_FUNCTION(BUFF* buff, void** field);

#define PACKET_MODE 0
#include "packets/_packetList.h"
