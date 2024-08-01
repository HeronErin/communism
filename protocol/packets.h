#pragma once
#include "buffer.h"
#include "datatypes.h"

typedef enum {
    FID_UNDEFINED,

    FID_JSON,
    FID_PAYLOAD,
    FID_PROTOCOL_VERSION,
    FID_SERVER_ADDRESS,
    FID_SERVER_PORT,
    FID_NEXT_STATE,
    FID_SERVER_ID,
    FID_PUBLIC_KEY_LENGTH,
    FID_PUBLIC_KEY,
    FID_VERIFY_TOKEN_LENGTH,
    FID_VERIFY_TOKEN,
    FID_SHOULD_AUTH,
    FID_UUID,
    FID_NAME,
    FID_VALUE,
    FID_IS_SIGNED,
    FID_SIGNATURE,
    FID_USERNAME,
    FID_PROP_NUMBER,
    FID_PROPERTIES,
    FID_STRICT_ERROR_HANDLING,
    FID_THRESHOLD,
    FID_STRING_SIZE

} PACKET_FIELD_ID;


struct _FieldConstructor {
    PACKET_FIELD_ID id;
    NetworkDatatype type;

    int sizeInt;  // Used for strings of (N) max size
    PACKET_FIELD_ID dependsOn; // Used for optional values, and array sizes
    

    unsigned int length;
    const struct _FieldConstructor* content;
      
};
typedef struct _FieldConstructor FieldConstructor;


typedef struct{
    int id;
    int size;
    const FieldConstructor fields[];
} PacketConstructor;



struct _PacketFieldData {
    PACKET_FIELD_ID id;
    NetworkDatatype type;
    
    union {
        int8_t boolData;
        int8_t byteData;
        uint8_t ubyteData;

        int16_t shortData;
        uint16_t ushortData;

        int32_t intData;
        uint32_t uintData;

        int32_t varIntData;
        int64_t varLongData;

        int64_t longData;
        uint64_t ulongData;

        float floatData;
        double doubleData;

        


        UUID uuidData;
        struct _PacketFieldData* optionalFieldData; // Null if not present
        struct {
            union{
                int stringLength;
                int identifierLength;
            };
            union{
                unsigned char* stringData;
                unsigned char* identifierData;
            };
        };        
        struct {
            unsigned int byteArrayLength;
            char* byteArrayData;
        };
        struct {
            unsigned int arrayLength;
            struct _PacketFieldData* arrayContents;
        };

    };
};
typedef struct _PacketFieldData PacketFieldData;
PacketFieldData* searchFieldsForId(PACKET_FIELD_ID field, PacketFieldData* data, int length);
PacketFieldData* makeFieldDataPackageFor(const FieldConstructor* constructor, int size);
PacketFieldData* decodePacket(const FieldConstructor* constructor, int size, BUFF* buff);
int encodePacket(const FieldConstructor *constructor, int size, PacketFieldData* fieldData, BUFF** buff);
int sendPacketRaw(const PacketConstructor *constructor, PacketFieldData* fieldData, int fd);

// Packet defs:

// Clientbound:
extern const PacketConstructor StatusResponseS2C;
extern const PacketConstructor PingResponseS2C;
extern const PacketConstructor DisconnectS2C;
extern const PacketConstructor EncryptionRequestS2C;
extern const PacketConstructor LoginSuccessS2C;
extern const PacketConstructor SetCompressionS2C;

// Serverbound
extern const PacketConstructor HanshakeC2S;
extern const PacketConstructor StatusRequestC2S;
extern const PacketConstructor PingRequestC2S;