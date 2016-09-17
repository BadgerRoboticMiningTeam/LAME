#pragma once

#include <cstdint>
#include <vector>

namespace Framework
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
        Drive = 0x90
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