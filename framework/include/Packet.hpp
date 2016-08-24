#pragma once

#include <cstdint>
#include <vector>

namespace LAME
{
    enum class PacketParseStatus
    {
        SUCCESS,
        BAD_HEADER,
        BAD_END_BYTE,
        BAD_SIZE,
        CRC_MISMATCH,
        BAD_PAYLOAD
    };

    class Packet
    {
    public:
        static const uint16_t LAME_HEADER = 0xBEEF;
        static const uint8_t  LAME_END = 0x7F;

        Packet(uint8_t opcode, uint8_t packet_size);
        virtual ~Packet();
        std::vector<uint8_t> Serialize();
        PacketParseStatus Parse(const uint8_t *buffer, int length);

        uint8_t GetOpcode() const;
        uint8_t GetSize() const;

    protected:
        virtual std::vector<uint8_t> SerializePayload();
        PacketParseStatus ParsePayload(const uint8_t *payload, int payload_length);

        uint8_t opcode;
        uint8_t size; 
    };
}