#pragma once

#include <cstdint>
#include <vector>

namespace LAME
{
    public enum class PacketParseStatus : System::Byte
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

    public enum class PacketOpcode : System::Byte
    {
        Drive = 0x90
    };

    public ref class Packet
    {
    public:
        static const System::Int16 LAME_HEADER = 0xBEEF;
        static const System::Byte  LAME_END = 0x7F;

        Packet(PacketOpcode opcode);
        virtual ~Packet();
        array<System::Byte>^ Serialize();
        PacketParseStatus Parse(array<System::Byte>^ buffer, System::Int32 length);
        PacketOpcode GetOpcode();

    protected:
        virtual std::vector<uint8_t> SerializePayload();
        virtual PacketParseStatus ParsePayload(array<System::Byte>^payload, System::Int32 payload_length);

        PacketOpcode opcode;
    };
}