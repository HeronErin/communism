#include "../../packets.h"


const PacketConstructor StatusResponseS2C  = {0, 2, {
    {FID_STRING_SIZE, DT_VARINT},
    {FID_JSON, DT_JSON_TEXT, 0, FID_STRING_SIZE}
}};
const PacketConstructor PingResponseS2C  = {1, 1, {
    {FID_PAYLOAD, DT_LONG}
}};

const PacketConstructor DisconnectS2C  = {0, 1, {
     {FID_JSON, DT_JSON_TEXT}
}};
const PacketConstructor EncryptionRequestS2C  = {1, 6, {
     {FID_SERVER_ID, DT_STRING, 20},
     {FID_PUBLIC_KEY_LENGTH, DT_VARINT},
     {FID_PUBLIC_KEY, DT_BYTE_ARRAY, 0, FID_PUBLIC_KEY_LENGTH},
     {FID_VERIFY_TOKEN_LENGTH, DT_VARINT},
     {FID_VERIFY_TOKEN, DT_BYTE_ARRAY, 0, FID_VERIFY_TOKEN_LENGTH},
     {FID_SHOULD_AUTH, DT_BOOL}
}};

static const FieldConstructor _optionalSignature = {FID_SIGNATURE, DT_STRING, 32767 };
static const FieldConstructor _propertiesList[] = {
        {FID_NAME, DT_STRING, 32767},
        {FID_VALUE, DT_STRING, 32767},
        {FID_IS_SIGNED, DT_BOOL},
        {FID_SIGNATURE, DT_OPTIONAL, 0, FID_IS_SIGNED, 1, &_optionalSignature}
}
;
const PacketConstructor LoginSuccessS2C = {2, 5, {
    {FID_UUID, DT_UUID},
    {FID_USERNAME, DT_STRING, 16},
    {FID_PROP_NUMBER, DT_VARINT},
    {FID_PROPERTIES, DT_ARRAY, 0, 0, sizeof(_propertiesList) / sizeof(_propertiesList[0]), _propertiesList},
    {FID_STRICT_ERROR_HANDLING, DT_BOOL}
}};

const PacketConstructor SetCompressionS2C = {2, 1, {
    {FID_THRESHOLD, DT_VARINT}
}};