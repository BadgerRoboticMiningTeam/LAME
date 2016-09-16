#include "Drive.hpp"

using namespace LAME;

Drive::Drive() : Packet(PacketOpcode::Drive)
{
    this->wheelSpeeds = gcnew array<System::Byte>(NUMBER_WHEELS);
    for (int i = 0; i < NUMBER_WHEELS; i++)
        this->wheelSpeeds[i] = 0;
}

Drive::Drive(array<System::Byte>^ wheel_speeds) : Packet(PacketOpcode::Drive)
{
    this->wheelSpeeds = wheel_speeds;
}

std::vector<uint8_t> Drive::SerializePayload()
{
    std::vector<uint8_t> payload;
    for (int i = 0; i < NUMBER_WHEELS; i++)
    {
        uint8_t spd = this->wheelSpeeds[i];
        payload.push_back(spd);
    }

    return payload;
}

array<System::Byte>^ Drive::GetWheelSpeeds()
{
    return static_cast<array<System::Byte>^>(this->wheelSpeeds->Clone());
}

void Drive::SetWheelSpeeds(array<System::Byte>^ wheel_speeds)
{
    this->wheelSpeeds = wheel_speeds;
}