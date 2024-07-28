#pragma once
#include "stdint.h"
#include <stddef.h>

typedef enum {
    T_End = 0,
    T_Byte,
    T_Short,
    T_Int,
    T_Long,
    T_float,
    T_Double,
    T_Byte_Array,
    T_String,
    T_List,
    T_Compound,

    // Not in original spec
    T_Int_Array,
    T_Long_Array
} TAG_TYPE;

extern const char* TAG_TYPE_NAMES[13];




struct _NbtTag {
    TAG_TYPE type;
    union{
        int8_t byteValue;
        int16_t shortValue;
        int32_t intValue;
        int64_t longValue;

        float floatValue;
        double doubleValue;
        
        struct{
            // Compounds can't be known at parse time how large they are,
            // so it make sense to use this style of array.
            int compoundLength;
            struct _CompoundElement** compoundElements;
        };
        struct{
            // Lists are length prefixed, so they get one contiguous buffer.

            int listSize;
            TAG_TYPE listContentType;
            struct _NbtTag* nbtListTags;
        };
        struct{
            int intArrayLen;
            int32_t* intArray;
        };
        struct{
            int longArrayLen;
            int64_t* longArray;
        };
        struct{
            int byteArraySize;
            const uint8_t* byteArrayBuffer;
        };
        struct{
            int stringSize;
            const uint8_t* stringBuffer;
        };
    };
};
typedef struct _NbtTag NbtTag;

struct _CompoundElement {
    char* name;
    NbtTag tag;
};
typedef struct _CompoundElement CompoundElement;


typedef NbtTag* (*NbtDecodeFunction)(uint8_t* bin, size_t len);
typedef int (*NbtEncodeFunction)(NbtTag* tag, uint8_t** outBin, size_t* length);

NbtTag* parseBinary(uint8_t* bin, size_t len);
NbtTag* parseZlibBinary(uint8_t* bin, size_t len);
NbtTag* parseGzipBinary(uint8_t* bin, size_t len);

void freeNbt(NbtTag* tag);
void printNbt(NbtTag* root);


int writeBinary(NbtTag* tag, uint8_t** outBin, size_t* length);
int writeZlibBinary(NbtTag* tag, uint8_t** outBin, size_t* length);
int writeGzipBinary(NbtTag* tag, uint8_t** outBin, size_t* length);