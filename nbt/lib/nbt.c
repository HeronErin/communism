// McCarthy's                            ,----, 
//                ,--.                 ,/   .`| 
//              ,--.'|    ,---,.     ,`   .'  : 
//          ,--,:  : |  ,'  .'  \  ;    ;     / 
//       ,`--.'`|  ' :,---.' .' |.'___,/    ,'  
//       |   :  :  | ||   |  |: ||    :     |   
//       :   |   \ | ::   :  :  /;    |.';  ;   
//       |   : '  '; |:   |    ; `----'  |  |   
//       '   ' ;.    ;|   :     \    '   :  ;   
//       |   | | \   ||   |   . |    |   |  '   
//       '   : |  ; .''   :  '; |    '   :  |   
//       |   | '`--'  |   |  | ;     ;   |.'    
//       '   : |      |   :   /      '---'      
//       ;   |.'      |   | ,'                  
//       '---'        `----'      Library.
// Not the best NBT implementation, but it works.

// Original Mojang spec: https://web.archive.org/web/20110723210920/http://www.minecraft.net/docs/NBT.txt
// Community documentation: https://wiki.vg/NBT


#include "nbt.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <endian.h>
#include <string.h>
#include "zlib.h"

// Heavily modified fromhttps://stackoverflow.com/questions/49622938/gzip-compression-using-zlib-into-buffer
int gzip_compress(const Bytef *source, uLong sourceLen, Bytef **dest, uLong *destLen) {
    // Allocate memory for the destination buffer
    uLong bound = compressBound(sourceLen);
    *dest = (Bytef *)malloc(bound);

    // Initialize the z_stream structure
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = (Bytef *)source;
    stream.avail_in = sourceLen;
    stream.next_out = *dest;
    stream.avail_out = bound;

    // Initialize deflate with gzip header
    int res = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (res != Z_OK) {
        free(*dest);
        return res;
    }

    // Compress the data
    res = deflate(&stream, Z_FINISH);
    if (res != Z_STREAM_END) {
        deflateEnd(&stream);
        free(*dest);
        return res == Z_OK ? Z_BUF_ERROR : res;
    }

    // Finalize the compression
    res = deflateEnd(&stream);
    if (res != Z_OK) {
        free(*dest);
        return res;
    }

    *destLen = stream.total_out;
    return Z_OK;
}

int gzip_decompress(const Bytef *source, uLong sourceLen, Bytef **dest, uLong *destLen) {
    // Estimate the size of the decompressed data (adjust if necessary)
    uLong decompressedLen = *destLen;
    *dest = (Bytef *)malloc(decompressedLen);

    // Initialize the z_stream structure
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = (Bytef *)source;
    stream.avail_in = sourceLen;
    stream.next_out = *dest;
    stream.avail_out = decompressedLen;

    // Initialize inflate with gzip header detection
    int res = inflateInit2(&stream, 16 + MAX_WBITS);
    if (res != Z_OK) {
        free(*dest);
        return res;
    }

    // Decompress the data
    res = inflate(&stream, Z_NO_FLUSH);
    if (res != Z_STREAM_END) {
        inflateEnd(&stream);
        free(*dest);
        return res == Z_OK ? Z_BUF_ERROR : res;
    }

    // Finalize the decompression
    res = inflateEnd(&stream);
    if (res != Z_OK) {
        free(*dest);
        return res;
    }

    *destLen = stream.total_out;
    return Z_OK;
}

const char* TAG_TYPE_NAMES[13] = {
    "TAG_End",
    "TAG_Byte",
    "TAG_Short",
    "TAG_Int",
    "TAG_Long",
    "TAG_Float",
    "TAG_Double",
    "TAG_Byte_Array",
    "TAG_String",
    "TAG_List",
    "TAG_Compound",
    "TAG_Int_Array",
    "TAG_Long_Array"
};

int populateFromPayload(NbtTag* tagToPopulate, uint8_t type, uint8_t** binRef, uint8_t* extent){
    uint8_t* bin = *binRef;
    tagToPopulate->type = type;
    uint16_t datalen;
    size_t reserveElementSize;

    switch (type){
        case T_End:
            break;
        case T_Byte_Array:
            if (bin+4 > extent){errno = ERANGE; return -1;}
            datalen = be32toh(*(uint32_t*)bin);
            bin+=4;
            
            if (bin+datalen > extent){errno = ERANGE; return -1;}
            
            char* toData = malloc(datalen+1);
            memcpy(toData, bin, datalen);
            toData[datalen] = 0;
            
            bin += datalen;
            tagToPopulate->byteArraySize = datalen;
            tagToPopulate->byteArrayBuffer = toData;
            break;
        case T_String:
            if (bin+2 > extent){errno = ERANGE; return -1;}
            datalen = be16toh(*(uint16_t*)bin);
            bin+=2;
            
            if (bin+datalen > extent){errno = ERANGE; return -1;}
            
            char* name = malloc(datalen+1);
            memcpy(name, bin, datalen);
            name[datalen] = 0;
            
            bin += datalen;
            tagToPopulate->stringSize = datalen;
            tagToPopulate->stringBuffer = name;
            break;
        case T_Byte:
            if (bin >= extent){errno = ERANGE; return -1;}
            tagToPopulate->byteValue = *(bin++);
            break;
        case T_Short:
            if (bin + 2 > extent){errno = ERANGE; return -1;}
            tagToPopulate->shortValue = be16toh(*(int16_t*)bin);
            bin+=2;
            break;
        case T_Int:
            if (bin + 4 > extent){errno = ERANGE; return -1;}
            tagToPopulate->intValue = be32toh(*(int32_t*)bin);
            bin+=4;
            break;
        case T_Long:
            if (bin + 8 > extent){errno = ERANGE; return -1;}
            tagToPopulate->longValue = be64toh(*(int64_t*)bin);
            bin+=8;
            break;
        case T_float:
            if (bin + sizeof(float) > extent){errno = ERANGE; return -1;}
            int floatRaw = be32toh(*(int*)bin);  // We need to convert endianness as an int, otherwise things get messed up. 
            tagToPopulate->floatValue =  *(float*)(&floatRaw);
            bin+=sizeof(float);
            break;
        case T_Double:
            if (bin + sizeof(double) > extent){errno = ERANGE; return -1;}
            long doubleRaw = be64toh(*(long*)bin); // Ditto
            tagToPopulate->doubleValue = *(double*)&doubleRaw;
            bin+=sizeof(double);
            break;
        case T_Int_Array:
            if (bin + 4 > extent){errno = ERANGE; return -1;}
            int32_t intListLen = be32toh(*(int32_t*)bin);
            bin+=4;

            tagToPopulate->intArrayLen = intListLen;
            if (bin + 4 * intListLen > extent){errno = ERANGE; return -1;}
            int32_t* intArray = malloc(4 * intListLen);
            tagToPopulate->intArray = intArray;

            for (int i = 0; i < intListLen; i++){
                intArray[i] = be32toh(*(int32_t*)bin);
                bin += 4;
            }
            break;
        case T_Long_Array:
            if (bin + 4 > extent){errno = ERANGE; return -1;}
            int32_t longListLen = be32toh(*(int32_t*)bin);
            bin+=4;

            tagToPopulate->longArrayLen = longListLen;
            if (bin + 8 * longListLen > extent){errno = ERANGE; return -1;}
            int64_t* longArray = malloc(8 * longListLen);
            tagToPopulate->longArray = longArray;

            for (int i = 0; i < longListLen; i++){
                longArray[i] = be64toh(*(int64_t*)bin);
                bin += 8;
            }
            break;
        case T_List:
            if (bin + 1 /*byte*/ + 4 /*int*/ > extent){errno = ERANGE; return -1;}
            uint8_t type = *(bin++);
            tagToPopulate->listContentType = type;
            
            tagToPopulate->listSize = be32toh(*(int32_t*)bin);
            bin += 4;
            int size = tagToPopulate->listSize;
            
            NbtTag* tags = malloc(sizeof(NbtTag) * size);
            tagToPopulate->nbtListTags = tags;

            for (int i = 0; i < size; i++){
                if (0 != populateFromPayload(&tags[i], type, &bin, extent))
                    return -1;
            }

            break;
        case T_Compound:
            reserveElementSize = 128;
            CompoundElement** elements = malloc(reserveElementSize * sizeof(void*));
            size_t elementIndex = 0;
            while(1){
                if (bin >= extent-1) break; // Assume that if we reached the end of the buffer, the compound is over. 


                // TODO: THIS SEGFAULTS!!!
                uint8_t type =  *(bin++);
                if (type == T_End) break;
                if (bin+2 > extent){errno = ERANGE; return -1;}
                CompoundElement* element = malloc(sizeof(CompoundElement));
                if (elementIndex >= reserveElementSize-1){
                    reserveElementSize*=2;
                    elements = realloc(elements, reserveElementSize * sizeof(void*));
                }
                elements[elementIndex++] = element;

                uint16_t strlen = be16toh(*(uint16_t*)bin);
                bin+=2;

                if (bin+strlen > extent){errno = ERANGE; return -1;}
                char* name = malloc(strlen+1);
                memcpy(name, bin, strlen);
                name[strlen] = 0;
                
                element->name = name;
                bin+=strlen;

                if (0 != populateFromPayload(&element->tag, type, &bin, extent))
                    return -1;
            }
            elements[elementIndex] = 0;
            tagToPopulate->compoundLength = elementIndex;
            tagToPopulate->compoundElements = elements;
    }
    *binRef = bin;
    return 0;
}
int writePayload(NbtTag* tag, uint8_t** binRef, size_t* index, size_t* reserve){
    int i, size;
    uint8_t* bin = *binRef;
    #define extentFor(SIZE) \
        while ((SIZE) > *reserve){\
            *reserve *= 2;\
            bin = realloc(bin, *reserve);\
            if (bin == NULL) return -1;\
        }
    
    switch (tag->type){
        case T_End:
            break;
        case T_Byte:
            extentFor(*index + 1);
            bin[(*index)++] = tag->byteValue;
            break;
        case T_Short:
            extentFor(*index + 2);
            *(short*)&bin[*index] = htobe16(tag->shortValue);
            *index += 2;
            break;
        case T_Int:
            extentFor(*index + 4);
            *(int*)&bin[*index] = htobe32(tag->intValue);
            *index += 4;
            break;
        case T_Long:
            extentFor(*index + 8);
            *(long*)&bin[*index] = htobe64(tag->longValue);
            *index += 8;
            break;
        case T_float:
            extentFor(*index + 4);
            int floatAsInt = htobe32(*(int*)&tag->floatValue);
            *(int*)&bin[*index] = floatAsInt;
            *index += 4;
            break;
        case T_Double:
            extentFor(*index + 8);
            long doubleAsLong = htobe64(*(long*)&tag->floatValue);
            *(long*)&bin[*index] = doubleAsLong;
            *index += 8;
            break;
        case T_Byte_Array:
            extentFor(*index + 4 + tag->byteArraySize);
            *(int*)&bin[*index] = htobe32(tag->byteArraySize);
            *index += 4;
            memcpy(bin + *index, tag->byteArrayBuffer, tag->byteArraySize);
            *index += tag->byteArraySize;
            break;
        case T_String:
            extentFor(*index + 2 + tag->stringSize);
            *(short*)&bin[*index] = htobe16(tag->stringSize);
            *index += 2;
            memcpy(bin + *index, tag->stringBuffer, tag->stringSize);
            *index += tag->stringSize;
            break;
        case T_List:
            extentFor(*index + 1 + 4);
            size = tag->listSize;

            bin[(*index)++] = tag->listContentType;
            *(int*)&bin[*index] = htobe32(tag->listSize);
            *index+=4;
            for (i = 0; i < size; i++){
                if (0 != writePayload(&tag->nbtListTags[i], &bin, index, reserve))
                    return -1;
            }
            break;
        case T_Compound:
            size = tag->compoundLength;
            for (i = 0; i < size; i++){
                CompoundElement* element = tag->compoundElements[i];
                size_t nameLen = strlen(element->name);
                extentFor(*index + nameLen + 2 + 1);
                bin[(*index)++] = element->tag.type;
                *(short*)&bin[*index] = htobe16(nameLen);
                *index += 2;
                memcpy(&bin[*index], element->name, nameLen);
                *index += nameLen;
                if (0 != writePayload(&element->tag, &bin, index, reserve))
                    return -1;
            }
            extentFor(*index + 1);
            bin[(*index)++] = 0;
            break;
        case T_Int_Array:
            extentFor(*index + 4);
            *(int*)&bin[*index] = htobe32(tag->intArrayLen);
            *index += 4;
            size = tag->intArrayLen;
            extentFor(*index + 4 * size);

            for (i = 0; i < size; i++){
                *(int*)&bin[*index] = htobe32(tag->intArray[i]);
                *index += 4;
            }

            break;
        case T_Long_Array:
            extentFor(*index + 4);
            *(int*)&bin[*index] = htobe32(tag->longArrayLen);
            *index += 4;
            size = tag->intArrayLen;
            extentFor(*index + 8 * size);

            for (i = 0; i < size; i++){
                *(long*)&bin[*index] = htobe64(tag->longArray[i]);
                *index += 8;
            }

            break;
    }
    *binRef = bin;
    return 0;
}

int writeBinary(NbtTag* tag, uint8_t** outBin, size_t* length){
    int r;
    size_t reserve = 512;
    *outBin = malloc(reserve);
    *length = 0;

    if (0 != ( r = writePayload(tag, outBin, length, &reserve) ) )
        return r;
    *length -= 1; // Otherwise root node will produce an unneeded null byte, we can just decrease the length to discard it.
    return 0;
}


void printNbt_(NbtTag* element, char* knownName, int knownIndex, int tabCount){
    if (element == NULL) return;
    int size, i;
    int tabCountTemp = tabCount;
    while(tabCountTemp--) putchar('\t');

    if (knownName) printf("%s(\"%s\"", TAG_TYPE_NAMES[element->type], knownName);
    else if (knownIndex >= 0) printf("%s(%d", TAG_TYPE_NAMES[element->type], knownIndex);
    else printf("%s(", TAG_TYPE_NAMES[element->type]);
    switch (element->type) {
        case T_End:
            printf(")\n");
            break;
        case T_Byte_Array:
            printf(") [Containing %d bytes]\n", element->byteArraySize);
            break;
        case T_Int_Array:
            printf(") [Containing %d ints]\n", element->intArrayLen);
            for (i = 0; i < element->intArrayLen; i++){
                tabCountTemp = tabCount;
                while(tabCountTemp--) putchar('\t');
                printf("\t-%d\n", element->intArray[i]);
            }
            break;
        case T_Long_Array:
            printf(") [Containing %d longs]\n", element->longArrayLen);
            for (i = 0; i < element->longArrayLen; i++){
                tabCountTemp = tabCount;
                while(tabCountTemp--) putchar('\t');
                printf("\t* %ld\n", element->longArray[i]);
            }
            break;
        case T_String:
            printf(") [Containing %d bytes]: %s\n", element->stringSize, element->stringBuffer);
            break;
        case T_Byte:
            printf("): %d\n", element->byteValue);
            break;
        case T_Short:
            printf("): %d\n", element->shortValue);
            break;
        case T_Int:
            printf("): %d\n", element->intValue);
            break;
        case T_Long:
            printf("): %ld\n", element->longValue);
            break;
        case T_float:
            printf("): %f\n", element->floatValue);
            break;
        case T_Double:
            printf("): %f\n", element->doubleValue);
            break;
        case T_List:
            size = element->listSize;
            NbtTag* listElement = element->nbtListTags;
            printf("): %d entries of type %s\n", size, TAG_TYPE_NAMES[element->listContentType]);
            for (i = 0; i < size; i++){
                printNbt_(&listElement[i], NULL, i, tabCount + 1);
            }
            break;
        case T_Compound:
            size = element->compoundLength;
            CompoundElement** elements = element->compoundElements;
            printf("): %d entries\n", size);
            for (i = 0; i < size; i++){
                CompoundElement* element = elements[i];
                if (element != NULL)
                printNbt_(&element->tag, element->name, -1, tabCount + 1);
            }
            break;
    }
}

void printNbt(NbtTag* root){
    printNbt_(root, NULL, -1, 0);
}


NbtTag* parseBinary(uint8_t* bin, size_t len){
    NbtTag* root = malloc(sizeof(NbtTag));
    if (0 != populateFromPayload(root, T_Compound, &bin, bin + len))
        return NULL;
    return root;
}

NbtTag* parseZlibBinary(uint8_t* bin, size_t len){
    size_t compressLen = compressBound(len);
    uint8_t* compressed = (uint8_t*)malloc(compressLen);

    int r = uncompress(compressed, &compressLen, bin, len);
    if (r != 0) return NULL;
    NbtTag* nbt = parseBinary(compressed, compressLen);
    free(compressed);
    return nbt;
}
NbtTag* parseGzipBinary(uint8_t* bin, size_t len){
    size_t compressLen = 1024*1024*5;
    uint8_t* compressed = NULL;

    int r = gzip_decompress(bin, len, &compressed, &compressLen);
    if (r != 0) return NULL;
    NbtTag* nbt = parseBinary(compressed, compressLen);
    // free(compressed);
    return nbt;
}

int writeGzipBinary(NbtTag* tag, uint8_t** outBin, size_t* length){
    uint8_t* tempBuff;
    size_t tempLen;
    int r;

    if (0 != writeBinary(tag, &tempBuff, &tempLen))
        return -1;
    if (0 != (r = gzip_compress(tempBuff, tempLen, outBin, length)))
        return r;
    free(tempBuff);
    return 0;
}
int writeZlibBinary(NbtTag* tag, uint8_t** outBin, size_t* length){
    uint8_t* tempBuff;
    size_t tempLen;
    int r;

    if (0 != writeBinary(tag, &tempBuff, &tempLen))
        return -1;
    *length = compressBound(tempLen);
    *outBin = malloc(*length);
    if (0 != (r=compress(*outBin, length, tempBuff, tempLen) ))
        return r;
    free(tempBuff);
    return 0;
}

void _freeNbt(NbtTag* tag){
    int i;
    size_t size;
    switch (tag->type) {
        // These don't have other things we need to care about
        case T_Short:
        case T_Int:
        case T_Long:
        case T_float:
        case T_Double:
        case T_Byte:
        case T_End:
            break;
        case T_Byte_Array:
            free((void*) tag->byteArrayBuffer);
            break;
        case T_String:
            free((void*) tag->stringBuffer);
            break;

        case T_List:
            free(tag->nbtListTags);
            break;
        case T_Compound:
            size = tag->compoundLength;
            
            CompoundElement** elements = tag->compoundElements;
            for (i = 0; i < size; i++){
                CompoundElement* element = elements[i];
                if (element != NULL) free(element);
            }
            free(elements);
            break;
        case T_Int_Array:
            free(tag->intArray);
            break;
        case T_Long_Array:
            free(tag->longArray);
            break;
    }
}
void freeNbt(NbtTag* tag){
    _freeNbt(tag);
    free(tag);
}

