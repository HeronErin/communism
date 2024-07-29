#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"

typedef enum {
    // Size: 1 byte
    DT_BOOL,

    DT_BYTE,
    DT_UBYTE,

    // Size: 2 bytes
    DT_SHORT,
    DT_USHORT,

    DT_INT,    // 4 bytes

    DT_LONG,   // 8 bytes
    DT_FLOAT,  // 4 bytes
    DT_DOUBLE, // 8 bytes
    
    DT_VARINT,  // 1 <= n <= 5 bytes
    DT_VARLONG, // 1 <= n <= 10 bytes

    DT_STRING,     // Varint prefixed utf-8 string
    DT_IDENTIFIER, // Just a string 

    DT_TEXT_COMPONENT,
    DT_JSON_TEXT,

    DT_ENTITY_METADATA,
    DT_SLOT,
    DT_NBT,
    
    DT_POSITION,     // 8 bytes
    DT_ANGLE,        // 1 byte (1/256 of a turn, smart)
    DT_UUID,         // 16 bytes

    DT_BITSET,       // Length prefixed
    DT_BITSET_FIXED, // Of fixed size

    // Arrays
    DT_ARRAY,        // Containing felids 
    DT_BYTE_ARRAY,   // Length prefixed

} NetworkDatatype;

typedef struct{
    uint64_t leastSignificant;
    uint64_t mostSignificant;
} UUID;

#define STRING_LEN(NNN) (NNN) * 3 + 3


int decodeVarIntFromFd(int fd, int* res);
int decodeVarInt(BUFF *buff, int32_t *value);
int decodeVarIntUnsigned(BUFF *buff, uint32_t *value);
int decodeVarLong(BUFF *buff, int64_t *value);
int decodeVarLongUnsigned(BUFF *buff, uint64_t *value);


int encodeVarIntUnsigned(BUFF** buff, uint32_t value);
int encodeVarLongUnsigned(BUFF** buff, uint64_t value);
int encodeVarInt(BUFF** buff, int32_t value);
int encodeVarLong(BUFF** buff, int32_t value);

int decodeByte(BUFF* buff, int8_t* val);
int decodeByteUnsigned(BUFF* buff, uint8_t* val);

#define encodeByte writeByte
#define encodeByteUnsigned writeByte

int decodeShortUnsigned(BUFF* buff, uint16_t* ret);
int decodeShort(BUFF* buff, int16_t* ret);
int encodeShortUnsigned(BUFF** buff, uint16_t val);
int encodeShort(BUFF** buff, int16_t val);


int decodeIntUnsigned(BUFF* buff, uint32_t* ret);
int decodeInt(BUFF* buff, int32_t* ret);
int encodeIntUnsigned(BUFF** buff, uint32_t val);
int encodeInt(BUFF** buff, int32_t val);

int decodeLongUnsigned(BUFF* buff, uint64_t* ret);
int decodeLong(BUFF* buff, uint64_t* ret);
int encodeLongUnsigned(BUFF** buff, uint64_t val);
int encodeLong(BUFF** buff, int64_t val);

int decodeUUID(BUFF* buff, UUID* uuid);
int encodeUUID(BUFF** buff, UUID uuid);

int decodeFixedString(BUFF* buff, uint8_t* result, size_t maxSize);
int decodeString(BUFF* buff, uint8_t** result, size_t knownMaxOrDefault);
int encodeString(BUFF** buff, const uint8_t* string, size_t knownMaxOrDefault);
