#include "Packet.hpp"
#include "bprintf.h"
#include "crc8.h"
#include <cerrno>

using namespace LAME;

constexpr uint8_t PACKET_HEADER_SIZE = sizeof(Packet::LAME_HEADER)  +
                                       sizeof(uint8_t)              + // opcode
                                       sizeof(uint8_t)              + // payload size
                                       sizeof(uint8_t);               // payload crc

constexpr uint8_t MIN_PACKET_SIZE = PACKET_HEADER_SIZE + sizeof(Packet::LAME_END);
constexpr const char *PACKET_HEADER_FORMAT_STR = "sccc";


Packet::Packet(PacketOpcode opcode)
{
    this->opcode = opcode;
}

Packet::~Packet()
{
}

std::vector<uint8_t> Packet::Serialize()
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
    data.push_back(LAME_END);

    return data;
}

std::vector<uint8_t> Packet::SerializePayload()
{
    return std::vector<uint8_t>();
}

PacketParseStatus Packet::Parse(const uint8_t *buffer, int length)
{
    uint16_t buffer_header;
    uint8_t buffer_opcode;
    uint8_t buffer_size;
    uint8_t buffer_crc;

    if (!buffer || length < 0 || length < MIN_PACKET_SIZE)
        return PacketParseStatus::BAD_BUFFER;

    int bytes_scanned = bscanf(
        buffer, 
        PACKET_HEADER_SIZE, 
        PACKET_HEADER_FORMAT_STR,
        Big,
        &buffer_header,
        &buffer_opcode,
        &buffer_size,
        &buffer_crc
    );

    if (bytes_scanned < 0)
        return PacketParseStatus::BAD_HEADER;

    // verify header and opcode
    if (buffer_header != LAME_HEADER)
        return PacketParseStatus::BAD_HEADER;

    if (buffer_opcode != static_cast<uint8_t>(this->opcode))
        return PacketParseStatus::BAD_OPCODE;

    // verify end byte
    if (buffer[PACKET_HEADER_SIZE + buffer_size] != LAME_END)
        return PacketParseStatus::BAD_END_BYTE;

    // verify payload crc
    // we only want to analyze the payload - so we take off the header + 1
    // which is just the min packet size!
    uint8_t computed_crc = compute_crc8(buffer + PACKET_HEADER_SIZE, length - MIN_PACKET_SIZE);
    if (computed_crc != buffer_crc)
        return PacketParseStatus::BAD_CRC;

    return this->ParsePayload(buffer + PACKET_HEADER_SIZE, length - MIN_PACKET_SIZE);
}

PacketParseStatus Packet::ParsePayload(const uint8_t *payload, int payload_length)
{
    return PacketParseStatus::SUCCESS;
}

PacketOpcode Packet::GetOpcode() const
{
    return this->opcode;
}
