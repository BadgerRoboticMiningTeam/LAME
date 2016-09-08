#include "Drive.hpp"

using namespace LAME;

Drive::Drive() : Packet(PacketOpcode::Drive)
{
    for (int i = 0; i < NUMBER_WHEELS; i++)
        this->wheelSpeeds[i] = 0;
}

Drive::Drive(std::array<uint8_t, NUMBER_WHEELS> wheel_speeds) : Packet(PacketOpcode::Drive)
{
    this->wheelSpeeds = wheel_speeds;   
}

std::vector<uint8_t> Drive::SerializePayload()
{
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), this->wheelSpeeds.begin(), this->wheelSpeeds.end());
    return payload;
}

std::array<uint8_t, Drive::NUMBER_WHEELS> Drive::GetWheelSpeeds()
{
    return this->wheelSpeeds;
}

void Drive::SetWheelSpeeds(std::array<uint8_t, NUMBER_WHEELS> wheel_speeds)
{
    this->wheelSpeeds = wheel_speeds;
}