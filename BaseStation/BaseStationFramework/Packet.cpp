#include "Packet.hpp"
#include "crc8.h"

using namespace FrameworkSharp;


Packet::Packet()
{
}

Packet::~Packet()
{
    if (pkt)
        delete pkt;
}

array<System::Byte>^ Packet::Serialize()
{
    auto data = this->pkt->Serialize();

    array<System::Byte>^ managed_buffer = gcnew array<System::Byte>(data.size());
    for (unsigned i = 0; i < data.size(); i++)
        managed_buffer[i] = data[i];

    return managed_buffer;
}

PacketParseStatus Packet::Parse(array<System::Byte>^ buffer, System::Int32 length)
{
    pin_ptr<uint8_t> raw_buffer = &buffer[0];
    return (PacketParseStatus) (this->pkt->Parse(raw_buffer, length));
}

PacketOpcode Packet::GetOpcode()
{
    return (PacketOpcode) this->pkt->GetOpcode();
}
