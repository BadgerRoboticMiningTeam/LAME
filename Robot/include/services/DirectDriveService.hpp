#pragma once

#include "Service.hpp"
#include "joystick.h"

using JoystickLibrary::Xbox360Service;

namespace LAME
{
    class DirectDriveService : public Service
    {
    public:
        DirectDriveService(ServiceMaster *master);

    protected:
        void Execute();

    private:
        std::unique_ptr<Xbox360Service> js;
    };
}