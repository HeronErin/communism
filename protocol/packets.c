#include "packets.h"
#include "buffer.h"
#include "datatypes.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

static PacketFieldData* searchFieldsForId(PACKET_FIELD_ID field, PacketFieldData* data, int length){
    for (int i = 0; i < length; i++){
        if (field == data[i].id)
            return &data[i];
    }
    return NULL;
}

PacketFieldData* decodePacket(const PacketConstructor* constructor, BUFF* buff){
    PacketFieldData* fieldReturnData = calloc(constructor->size, sizeof(PacketFieldData));
    for (int i = 0; i < constructor->size; i++){
        PacketFieldData data;
        FieldConstructor fieldCons = constructor->fields[i];
        data.type = fieldCons.type;
        
        PacketFieldData* dep;

        switch(fieldCons.type){
            case DT_BOOL:
            case DT_BYTE:
            case DT_UBYTE:
                if (0 != decodeByteUnsigned(buff, &data.ubyteData)) return NULL;
                break;
            case DT_SHORT:
            case DT_USHORT:
                if (0 != decodeShortUnsigned(buff, &data.ushortData)) return NULL;
                break;
            case DT_INT:
            case DT_UINT:
                if (0 != decodeIntUnsigned(buff, &data.uintData)) return NULL;
                break;
            case DT_LONG:
            case DT_ULONG:
                if (0 != decodeLongUnsigned(buff, &data.ulongData)) return NULL;
                break;
            case DT_STRING:
            case DT_IDENTIFIER:
            case DT_TEXT_COMPONENT:
            case DT_JSON_TEXT:
            case DT_BYTE_ARRAY:
                dep = searchFieldsForId(fieldCons.dependsOn, fieldReturnData, i);
                if (dep == NULL){
                    errno = EBADR;
                    return NULL;
                }
                int size = dep->varIntData;
                int knownMaxOrDefault = 32767;
                if (fieldCons.length != 0)
                    knownMaxOrDefault = fieldCons.length;

                if (size > knownMaxOrDefault || buff->index + size >= buff->size){
                    perror(size > knownMaxOrDefault ? 
                        "Attempted to read string that is too large\n" 
                    : "Attempted to read string past end of buffer\n"
                    );
                        errno = EOVERFLOW;
                        return NULL;
                }
                data.byteArrayLength = size;
                data.stringData = malloc(size);
                memcpy(data.stringData, buff->data + buff->index, size);
                buff->index += size;
                break;
            default:
                printf("Unimplemented network datatype: %d\n", fieldCons.type);
                exit(1);
        }
        fieldReturnData[i] = data;
        continue;
    }
}