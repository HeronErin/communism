#include <stdint.h>
#include "../buffer.h"
#include "../datatypes.h"


// PACKET_MODE = 0 for generating headers
// PACKET_MODE = 1 for generating free functions
// PACKET_MODE = 2 for generating encode functions
// PACKET_MODE = 3 for generating decode functions


#undef GLUE_HELPER
#define GLUE_HELPER(X, Y) X##Y

#undef GLUE
#define GLUE(X, Y) GLUE_HELPER(X, Y)

// Reset already defined macros


#undef BEGIN_PACKET
#undef END_PACKET

#undef D_VarInt
#undef D_VarUInt

#undef D_Byte
#undef D_UByte

#undef D_Short
#undef D_UShort

#undef D_Int
#undef D_UInt

#undef D_Long
#undef D_ULong

#undef D_Fixed_String
#undef D_Dyn_String

#undef D_UUID
#undef D_OPTIONAL_BY_BOOL
#undef D_BOOL


// Bools are just ints
#define D_BOOL(NAME) D_Byte(NAME)


// Headers / structs
#if PACKET_MODE == 0
    #define BEGIN_PACKET \
        typedef struct {
    #define END_PACKET \
        } P_NAME;\
        void GLUE(free, P_NAME)(P_NAME* element); \
        int GLUE(encode, P_NAME)(BUFF** buff, P_NAME* element); \
        int GLUE(decode, P_NAME)(BUFF* buff, P_NAME** element);

    #define D_VarInt(NAME) int32_t NAME
    #define D_VarUInt(NAME) uint32_t NAME

    #define D_Byte(NAME) int8_t NAME
    #define D_UByte(NAME) uint8_t NAME

    #define D_Short(NAME) int16_t NAME
    #define D_UShort(NAME) uint16_t NAME

    #define D_Int(NAME) int32_t NAME
    #define D_UInt(NAME) uint32_t NAME

    #define D_Long(NAME) int64_t NAME
    #define D_ULong(NAME) uint64_t NAME

    #define D_Fixed_String(NAME, SIZE) char NAME[SIZE]
    #define D_Dyn_String(NAME) char* NAME

    #define D_UUID(NAME) UUID NAME

    #define D_OPTIONAL_BY_BOOL(NAME, BOOL_ELEMENT, ENCODE_FUNC, DECODE_FUNC, FREE_FUNC) void* NAME
// Free functions
#elif PACKET_MODE == 1
    #define BEGIN_PACKET void GLUE(free, P_NAME)(P_NAME* element){
    #define END_PACKET free(element); }

    #define D_VarInt(NAME);
    #define D_VarUInt(NAME);

    #define D_Byte(NAME);
    #define D_UByte(NAME);

    #define D_Short(NAME);
    #define D_UShort(NAME);

    #define D_Int(NAME);
    #define D_UInt(NAME);

    #define D_Long(NAME);
    #define D_ULong(NAME);

    #define D_Fixed_String(NAME, SIZE);
    #define D_Dyn_String(NAME) free(element-> NAME)

    #define D_UUID(NAME);

    #define D_OPTIONAL_BY_BOOL(NAME, BOOL_ELEMENT, ENCODE_FUNC, DECODE_FUNC, FREE_FUNC) FREE_FUNC(element-> NAME)
// encode functions
#elif PACKET_MODE == 2
    #define BEGIN_PACKET int GLUE(encode, P_NAME)(BUFF** buff, P_NAME* element){ \
        int tmp
    #define END_PACKET return 0; }

    #define _DEF_CALL(X)  if (0 != (tmp = X)) return tmp
    #define _DEF_ENCODE(FUNC, OUT) _DEF_CALL(FUNC(buff, element-> OUT))

    #define D_VarInt(NAME) _DEF_ENCODE(encodeVarInt, NAME)
    #define D_VarUInt(NAME) _DEF_ENCODE(encodeVarIntUnsigned, NAME)

    #define D_Byte(NAME) _DEF_ENCODE(encodeByte, NAME)
    #define D_UByte(NAME) _DEF_ENCODE(encodeByteUnsigned, NAME)

    #define D_Short(NAME) _DEF_ENCODE(encodeShort, NAME)
    #define D_UShort(NAME) _DEF_ENCODE(encodeShortUnsigned, NAME)

    #define D_Int(NAME) _DEF_ENCODE(encodeInt, NAME)
    #define D_UInt(NAME) _DEF_ENCODE(encodeIntUnsigned, NAME)

    #define D_Long(NAME) _DEF_ENCODE(encodeLong, NAME)
    #define D_ULong(NAME) _DEF_ENCODE(encodeLongUnsigned, NAME)

    #define D_Fixed_String(NAME, SIZE)  _DEF_CALL(encodeString(buff, element-> NAME, SIZE))
    #define D_Dyn_String(NAME) _DEF_CALL(encodeString(buff, element-> NAME, 0))

    #define D_UUID(NAME) _DEF_ENCODE(encodeUUID, NAME)

    #define D_OPTIONAL_BY_BOOL(NAME, BOOL_ELEMENT, ENCODE_FUNC, DECODE_FUNC, FREE_FUNC) \
        if (element-> BOOL_ELEMENT) \
            _DEF_CALL(ENCODE_FUNC(buff, element-> NAME))
#endif

