#pragma once

#include <cstdint>
#include <vector>
#include "../../../framework/include/Packet.hpp"
#include "../../../framework/include/Drive.hpp"

namespace FrameworkSharp
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
        Packet();
        virtual ~Packet();
        array<System::Byte>^ Serialize();
        PacketParseStatus Parse(array<System::Byte>^ buffer, System::Int32 length);
        PacketOpcode GetOpcode();

    protected:
         Framework::Packet *pkt;
    };
}