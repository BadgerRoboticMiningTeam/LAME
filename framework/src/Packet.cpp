#include "Packet.hpp"

using namespace LAME;

Packet::Packet(uint8_t opcode, uint8_t packet_size)
{
    this->opcode = opcode;
    this->size = packet_size;
}

Packet::~Packet()
{
}

std::vector<uint8_t> Packet::Serialize()
{
    std::vector<uint8_t> data(this->size);
    std::vector<uint8_t> payload = this->SerializePayload();
    data.insert(data.end(), payload.begin(), payload.end());

    return data;
}

std::vector<uint8_t> Packet::SerializePayload()
{
    return std::vector<uint8_t>();
}

PacketParseStatus Packet::Parse(const uint8_t *buffer, int length)
{
    // TODO
    return PacketParseStatus::SUCCESS;
}

uint8_t Packet::GetOpcode() const
{
    return this->opcode;
}

uint8_t Packet::GetSize() const
{
    return this->size;
}