#include "Drive.hpp"

using namespace FrameworkSharp;

Drive::Drive() : Packet()
{
    this->pkt = new Framework::Drive();
}

Drive::Drive(array<System::Byte>^ wheel_speeds) : Packet()
{
    std::array<uint8_t, Drive::NUMBER_WHEELS> native_speeds;
    for (int i = 0; i < Drive::NUMBER_WHEELS; i++)
        native_speeds[i] = wheel_speeds[i];

    this->pkt = new Framework::Drive(native_speeds);
}

array<System::Byte>^ Drive::GetWheelSpeeds()
{
    array<System::Byte>^ speeds = gcnew array<System::Byte>(Drive::NUMBER_WHEELS);
    auto native_speeds = static_cast<Framework::Drive *>(this->pkt)->GetWheelSpeeds();

    for (int i = 0; i < Drive::NUMBER_WHEELS; i++)
        speeds[i] = native_speeds[i];

    return speeds;
}

void Drive::SetWheelSpeeds(array<System::Byte>^ wheel_speeds)
{
    std::array<uint8_t, Drive::NUMBER_WHEELS> native_speeds;
    for (int i = 0; i < Drive::NUMBER_WHEELS; i++)
        native_speeds[i] = wheel_speeds[i];

    static_cast<Framework::Drive *>(this->pkt)->SetWheelSpeeds(native_speeds);
}