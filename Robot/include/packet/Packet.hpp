#pragma once

#include <cstdint>
#include <vector>

namespace LAME
{
    enum class PacketParseStatus
    {
        SUCCESS,
        BAD_BUFFER,
        BAD_HEADER,
        BAD_OPCODE,
        BAD_END_BYTE,
        BAD_SIZE,
        BAD_CRC,
        BAD_PAYLOAD
    };

    enum class PacketOpcode : uint8_t
    {
        Drive = 0x90,

        SwitchToAIDrive = 0x01,
        SwitchToRemoteDrive = 0x02,
        EStop = 0x03,
        ClearEStop = 0x04,

        QueryDriveMode = 0x10,
        QueryBinWeight = 0x11,
        QueryLocation = 0x12,
        QueryCameraImage = 0x13,
        QueryHeartbeat = 0x14,

        ReportDriveMode = 0x40,
        ReportBinWeight = 0x41,
        ReportLocation = 0x42,
        ReportHeartbeat = 0x44
    };

    class Packet
    {
    public:
        static const uint16_t LAME_HEADER = 0xBEEF;
        static const uint8_t  LAME_END = 0x7F;

        Packet(PacketOpcode opcode);
        virtual ~Packet();
        std::vector<uint8_t> Serialize();
        PacketParseStatus Parse(const uint8_t *buffer, int length);
        PacketOpcode GetOpcode() const;

    protected:
        virtual std::vector<uint8_t> SerializePayload();
        virtual PacketParseStatus ParsePayload(const uint8_t *payload, int payload_length);

        PacketOpcode opcode;
    };
}