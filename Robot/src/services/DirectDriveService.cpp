#include "DirectDriveService.hpp"
#include <stdexcept>

using namespace LAME;

constexpr int DIRECT_DRIVE_INTERVAL = 100;

DirectDriveService::DirectDriveService(ServiceMaster * master) : 
    Service(master, DIRECT_DRIVE_INTERVAL, false),
    js(new Xbox360Service(1))
{
    if (!js->Initialize() || !js->Start())
        throw std::exception("Xbox service failed to initialize in DirectDriveService!");
}

void DirectDriveService::Execute()
{
    
}
