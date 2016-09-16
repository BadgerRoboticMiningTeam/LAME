#pragma once

#include "Packet.hpp"

namespace LAME
{
    public ref class Drive : public Packet
    {
    public:
        static const System::Int32 NUMBER_WHEELS = 4;
        Drive();
        Drive(array<System::Byte>^ wheel_speeds);
        array<System::Byte>^ GetWheelSpeeds();
        void SetWheelSpeeds(array<System::Byte>^ wheel_speeds);

    protected:
        std::vector<uint8_t> SerializePayload() override;

    private:
        array<System::Byte>^ wheelSpeeds;
    };
}