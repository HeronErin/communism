#include "lib/nbt.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


// This file it to test if NBT can read then reconstruct an NBT file.
// The actual NBT library is in lib/ (see MakeFile) and is intended
// to be statically linked.


struct Test{
    const char* name;
    NbtDecodeFunction decodeFunc;
};

const struct Test tests[] = {
    {"hello_world.nbt", &parseBinary},
    {"int_test.nbt", &parseBinary},
    {"bigtest.nbt", &parseGzipBinary}
};

int main(){
    struct Test test;
    FILE* fp;
    long fileSize;
    uint8_t* fileBuff;
    NbtTag* tagFromFile;
    NbtTag* tagFromReEncode;

    int r;
    uint8_t* tempDecodeBuff;
    size_t tempDecodeSize;

    uint8_t* tempReEncodeBuff;
    size_t tempReEncodeSize;
    for (int i = 0; i < sizeof(tests) / sizeof(struct Test); i++){
        test = tests[i];

        // Open file and get length
        fp = fopen(test.name, "r");
        if (fp == NULL){
            printf("[NBT Tests] File not found: %s\n", test.name);
            exit(-1);
        }
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        // Read file into memory
        fileBuff = malloc(fileSize);
        fread(fileBuff, fileSize, 1, fp);

        tagFromFile = test.decodeFunc(fileBuff, fileSize);
        if (tagFromFile == NULL){
            printf("[NBT Tests] Can't decode file: %s\n", test.name);
            exit(-1);
        }


        // Decode testing
        if (0 != (r=writeBinary(tagFromFile, &tempDecodeBuff, &tempDecodeSize))){
            printf("[NBT Tests] Can't encode file with writeBinary() with code %d: %s\n", r, test.name);
            exit(-1);
        }
        if ((NbtDecodeFunction) &writeBinary == test.decodeFunc
                && 0 != memcmp(tempDecodeBuff, fileBuff, tempDecodeSize)){
            printf("[NBT Tests] : writeBinary() can't perfectly with binary match re-encode file: %s\n", test.name);
            exit(-1);
        }
        tagFromReEncode = parseBinary(tempDecodeBuff, tempDecodeSize);
        if (tagFromReEncode == NULL){
            printf("[NBT Tests] Can't decode file with parseBinary() during re-encode: %s\n", test.name);
            exit(-1);
        }
        if (0 != (r=writeBinary(tagFromReEncode, &tempReEncodeBuff, &tempReEncodeSize))){
            printf("[NBT Tests] Can't encode file with writeBinary() with code %d after re-encode: %s\n", r, test.name);
            exit(-1);
        }
        if (0 != memcmp(tempReEncodeBuff, tempDecodeBuff, tempReEncodeSize)){
            printf("[NBT Tests] During final file rencode, using function writeBinary(), binary was not identical! %s\n", test.name);
            exit(-1);
        }
        printNbt(tagFromFile);
        
        freeNbt(tagFromReEncode);
        freeNbt(tagFromFile);
        free(fileBuff);
        free(tempDecodeBuff);
        free(tempReEncodeBuff);
        fclose(fp);
    }

    return 0;
}