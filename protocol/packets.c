#include "packets.h"
#include "buffer.h"
#include "datatypes.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

PacketFieldData* searchFieldsForId(PACKET_FIELD_ID field, PacketFieldData* data, int length){
    for (int i = 0; i < length; i++){
        if (field == data[i].id)
            return &data[i];
    }
    return NULL;
}

PacketFieldData* makeFieldDataPackageFor(const FieldConstructor* constructor, int size){
    PacketFieldData* ret = calloc(size, sizeof(PacketFieldData));
    for (int i = 0; i < size; i++){
        ret[i].id = constructor[i].id;
        ret[i].type = constructor[i].type;
        switch (ret[i].type) {
            case DT_OPTIONAL:
                ret[i].optionalFieldData = makeFieldDataPackageFor(constructor[i].content, constructor[i].length);
                break;
            default: break;
        }
    }

    return ret;
}

// Makes sure string lengths are proper in the fieldData
// Also makes sure all field types are the same
static int _bakeLengths(const FieldConstructor *constructor, int size, PacketFieldData* fieldData){
    for (int i = 0; i < size; i++){
        PacketFieldData* field = &fieldData[i];
        if (field->type != constructor[i].type || field->id != constructor[i].id){
            errno = EINVAL;
            return 1;
        }

        PacketFieldData* dep;
        switch(field->type){
            case DT_STRING:
            case DT_IDENTIFIER:
            case DT_TEXT_COMPONENT:
            case DT_JSON_TEXT:
                dep = searchFieldsForId(constructor[i].dependsOn, fieldData, i);
                if (dep == NULL){
                    errno = EBADR;
                    
                    return 1;
                }
                field->stringLength = strlen(field->stringData);
                switch (dep->type) {
                    case DT_VARINT:
                    case DT_INT:
                    case DT_UINT:
                        dep->varIntData = field->stringLength;
                        break;
                    case DT_LONG:
                    case DT_ULONG:
                    case DT_VARLONG:
                        dep->varLongData = field->stringLength;
                        break;
                    case DT_SHORT:
                    case DT_USHORT:
                        dep->shortData = field->stringLength;;
                        break;
                    default:
                        perror("Unsupported dep type\n");
                        return 1;
                }
            default:
                break;
        }
    }

    return 0;
}

PacketFieldData* decodePacket(const FieldConstructor* constructor, int size, BUFF* buff){
    PacketFieldData* fieldReturnData = calloc(size, sizeof(PacketFieldData));
    for (int i = 0; i < size; i++){
        PacketFieldData data;
        FieldConstructor fieldCons = constructor[i];
        data.type = fieldCons.type;
        data.id = fieldCons.id;
        PacketFieldData* dep;

        switch(fieldCons.type){
            case DT_VARINT:
                if (0 != decodeVarInt(buff, &data.varIntData)) return NULL;
                break;
            case DT_VARLONG:
                if (0 != decodeVarLong(buff, &data.varLongData)) return NULL;
                break;
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
            case DT_OPTIONAL:
                dep = searchFieldsForId(fieldCons.dependsOn, fieldReturnData, i);
                if (dep == NULL || dep->type != DT_BOOL){
                    errno = EBADR;
                    return NULL;
                }
                data.optionalFieldData = NULL;
                if (!dep->boolData) break;
                data.optionalFieldData = decodePacket(fieldCons.content, fieldCons.length, buff);
                if (data.optionalFieldData == NULL) return NULL;
                break;
            case DT_ARRAY:
                dep = searchFieldsForId(fieldCons.dependsOn, fieldReturnData, i);
                if (dep == NULL){
                    errno = EBADR;
                    return NULL;
                }
                data.arrayLength = dep->varIntData;
                for (int i = 0; i < data.arrayLength; i++){
                    PacketFieldData* p =  decodePacket(fieldCons.content, fieldCons.length, buff);
                    if (p == NULL) return NULL;
                    data.arrayContents[i] = *p;
                    free(p);
                }
                break;
            case DT_STRING:
            case DT_IDENTIFIER:
            case DT_TEXT_COMPONENT:
            case DT_JSON_TEXT:
            case DT_BYTE_ARRAY:
                dep = searchFieldsForId(fieldCons.dependsOn, fieldReturnData, i);
                if (dep == NULL){
                    errno = EBADR;
                    perror("1");
                    return NULL;
                }
                int size = dep->varIntData;
                
                if (0 != decodeStringPreSized(buff, &data.stringData, size, fieldCons.length))
                    return NULL;
                break;
            default:
                printf("Unimplemented network datatype: %d\n", fieldCons.type);
                exit(1);
        }
        fieldReturnData[i] = data;
        continue;
    }

    return fieldReturnData;
}

int encodePacket(const FieldConstructor *constructor, int size, PacketFieldData* fieldData, BUFF** buff){
    if (0 != _bakeLengths(constructor, size, fieldData)) return 1;
    for (int i = 0; i < size; i++){
        PacketFieldData* dep;
        PacketFieldData data = fieldData[i];
        switch (data.type){
            case DT_VARINT:
                if (0 != encodeVarInt(buff, data.varIntData)) return 1;
                break;
            case DT_VARLONG:
                if (0 != encodeVarLong(buff, data.varLongData)) return 1;
                break;
            case DT_BOOL:
            case DT_BYTE:
            case DT_UBYTE:
                if (0 != encodeByteUnsigned(buff, data.ubyteData)) return 1;
                break;
            case DT_SHORT:
            case DT_USHORT:
                if (0 != encodeShortUnsigned(buff, data.ushortData)) return 1;
                break;
            case DT_INT:
            case DT_UINT:
                if (0 != encodeIntUnsigned(buff, data.uintData)) return 1;
                break;
            case DT_LONG:
            case DT_ULONG:
                if (0 != encodeLongUnsigned(buff, data.ulongData)) return 1;
                break;
            case DT_STRING:
            case DT_IDENTIFIER:
            case DT_TEXT_COMPONENT:
            case DT_JSON_TEXT:
            case DT_BYTE_ARRAY:
                // printf("Wow: %llu %d\n",(size_t) fieldData[i].stringData, fieldData[i].stringLength);
                if (0 != encodeStringPreSized(buff, fieldData[i].stringData, fieldData[i].stringLength, constructor[i].sizeInt))
                    return 1;
                break;
            case DT_OPTIONAL:
                dep = searchFieldsForId(constructor[i].dependsOn, fieldData, i);
                if (dep == NULL || dep->type != DT_BOOL) return 1;
                if (!dep->boolData) break;
                if (0 != encodePacket(constructor[i].content, constructor[i].sizeInt, fieldData[i].optionalFieldData, buff))
                    return 1;
                break;
            case DT_ARRAY:
                for (int i = 0; data.arrayLength; i++){
                    if (0 != encodePacket(constructor[i].content, constructor[i].sizeInt, &data.arrayContents[i], buff))
                        return 1;
                }
                break;
            default:
                printf("Unimplemented network datatype: %d\n", data.type);
                exit(1);
        }

    }
}
int sendPacketRaw(const PacketConstructor *constructor, PacketFieldData* fieldData, int fd){
    BUFF* out = makeBuff(0, 0);

    if (0 != encodeVarInt(&out, constructor->id))
        return 1;
    if (0 != encodePacket(constructor->fields, constructor->size, fieldData, &out))
        return 1;
    if (0 != encodeVarIntToFd(out->size, fd))
        return 1;
    if (-1 == write(fd, &out->data, out->size)){
        return 1;
    }

    free(out);
    return 0;
}