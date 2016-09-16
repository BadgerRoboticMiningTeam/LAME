#include "Packet.hpp"
#include "crc8.h"

using namespace LAME;

constexpr uint8_t PACKET_HEADER_SIZE = sizeof(Packet::LAME_HEADER) +
                                        sizeof(uint8_t) + // opcode
                                        sizeof(uint8_t) + // payload size
                                        sizeof(uint8_t);  // payload crc

constexpr uint8_t MIN_PACKET_SIZE = PACKET_HEADER_SIZE + sizeof(Packet::LAME_END);
constexpr const char *PACKET_HEADER_FORMAT_STR = "sccc";


Packet::Packet(PacketOpcode opcode)
{
    this->opcode = opcode;
}

Packet::~Packet()
{
}

array<System::Byte>^ Packet::Serialize()
{
    std::vector<uint8_t> data;
    std::vector<uint8_t> payload = this->SerializePayload();
    uint8_t crc_val = compute_crc8(payload.data(), payload.size());

    // push header, opcode, size, payload crc, payload, and end byte, in that order
    data.push_back(static_cast<uint8_t>(LAME_HEADER >> 8));
    data.push_back(static_cast<uint8_t>(LAME_HEADER & 0xFF));
    data.push_back(static_cast<uint8_t>(this->opcode));
    data.push_back(static_cast<uint8_t>(payload.size()));
    data.push_back(crc_val);
    data.insert(data.end(), payload.begin(), payload.end());
    data.push_back(static_cast<uint8_t>(LAME_END));

    array<System::Byte>^ managed_buffer = gcnew array<System::Byte>(data.size());
    for (unsigned i = 0; i < data.size(); i++)
        managed_buffer[i] = data[i];

    return managed_buffer;
}

std::vector<uint8_t> Packet::SerializePayload()
{
    return std::vector<uint8_t>();
}

constexpr int START_BYTE_INDEX = 0;
constexpr int OPCODE_INDEX = 2;
constexpr int PAYLOAD_SIZE_INDEX = 3;
constexpr int CRC_INDEX = 4;

PacketParseStatus Packet::Parse(array<System::Byte>^ buffer, System::Int32 length)
{
    uint16_t buffer_header;
    uint8_t buffer_opcode;
    uint8_t buffer_size;
    uint8_t buffer_crc;

    if (!buffer || length < 0 || length < MIN_PACKET_SIZE)
        return PacketParseStatus::BAD_BUFFER;

    pin_ptr<uint8_t> raw_buffer = &buffer[0];


    buffer_header = buffer[START_BYTE_INDEX] << 8 | buffer[START_BYTE_INDEX + 1];
    buffer_opcode = buffer[OPCODE_INDEX];
    buffer_size = buffer[PAYLOAD_SIZE_INDEX];
    buffer_crc = buffer[CRC_INDEX];

    // verify header and opcode
    if (buffer_header != Packet::LAME_HEADER)
        return PacketParseStatus::BAD_HEADER;

    if (buffer_opcode != static_cast<uint8_t>(this->opcode))
        return PacketParseStatus::BAD_OPCODE;

    // verify end byte
    if (buffer[PACKET_HEADER_SIZE + buffer_size] != Packet::LAME_END)
        return PacketParseStatus::BAD_END_BYTE;

    // verify payload crc
    // we only want to analyze the payload - so we take off the header + 1
    // which is just the min packet size!
    uint8_t computed_crc = compute_crc8(raw_buffer + PACKET_HEADER_SIZE, length - MIN_PACKET_SIZE);
    if (computed_crc != buffer_crc)
        return PacketParseStatus::BAD_CRC;

    array<System::Byte>^ payload = gcnew array<System::Byte>(length - MIN_PACKET_SIZE);
    System::Array::Copy(buffer, PACKET_HEADER_SIZE, payload, 0, length - MIN_PACKET_SIZE);
    return this->ParsePayload(payload, payload->Length);
}

PacketParseStatus Packet::ParsePayload(array<System::Byte>^payload, System::Int32 payload_length)
{
    return PacketParseStatus::SUCCESS;
}

PacketOpcode Packet::GetOpcode()
{
    return this->opcode;
}
