#include "../../packets.h"


const PacketConstructor HanshakeC2S = {0, 5, {
    {FID_PROTOCOL_VERSION, DT_VARINT},
    {FID_STRING_SIZE, DT_VARINT},
    {FID_SERVER_ADDRESS, DT_STRING, 255, FID_STRING_SIZE},
    {FID_SERVER_PORT, DT_USHORT},
    {FID_NEXT_STATE, DT_VARINT}, // 1 for Status, 2 for Login, 3 for Transfer.
}};


const PacketConstructor StatusRequestC2S = {0, 0};
const PacketConstructor PingRequestC2S = {1, 1, {
    {FID_PAYLOAD, DT_LONG}
}};
