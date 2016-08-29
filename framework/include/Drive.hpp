#pragma once

#include "Packet.hpp"
#include <array>

namespace LAME
{
    class Drive : public Packet
    {
    public:
        static const int NUMBER_WHEELS = 4;
        Drive();
        Drive(std::array<uint8_t, NUMBER_WHEELS> wheel_speeds);

    protected:
        std::vector<uint8_t> SerializePayload();

    private:
        std::array<uint8_t, NUMBER_WHEELS> wheelSpeeds;
    };
}