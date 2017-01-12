#include "ServiceMaster.hpp"
#include "Service.hpp"
#include <cstdint>
#include "AIDriveService.hpp"
#include "RemoteDriveService.hpp"
#include "DirectDriveService.hpp"

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 256;


struct ServiceMaster::ServiceMasterImpl
{
    std::unique_ptr<AIDriveService> ai;
    std::unique_ptr<DirectDriveService> directDrive;
    std::unique_ptr<RemoteDriveService> remoteDrive;

    // XXX: Initialize the services here
    ServiceMasterImpl(ServiceMaster *sm) :
        directDrive(new DirectDriveService(sm))
    {
    }
};


ServiceMaster::ServiceMaster(int port) : socket(port)
{
    this->isRunning = false;
    this->services = new ServiceMasterImpl(this);
}

ServiceMaster::~ServiceMaster()
{
    if (this->services)
        delete this->services;
}

void ServiceMaster::SendPacket(const Packet& pkt)
{
    if (this->dest == nullptr)
        return;

    auto& data = pkt.Serialize();
    uint8_t *buffer = &data[0];
    this->socket.Write(buffer, data.size(), this->dest);
}

void ServiceMaster::Run()
{
    uint8_t read_buffer[READ_BUFFER_SIZE];

    if (!socket.Open())
        return;

    //TODO  run the services

    memset(read_buffer, 0, READ_BUFFER_SIZE);

    while (true)
    {
        int bytes_read = socket.Read(read_buffer, READ_BUFFER_SIZE, nullptr);
        if (bytes_read < 0)
            continue;

        // TODO: HANDLE ACCESS CONTROL PACKETS HERE
        // TODO: ROUTE PACKETS HERE

        /*
        for (auto& s : this->services)
        {
            if (!s->IsActive())
                continue;

            if (!s->HandlePacket(read_buffer, bytes_read))
                continue;

            if (s->GetSleepInterval() == Service::RUN_ON_PACKET_RECEIVE)
                s->ExecuteOnTime();

            // packet issued to service - continue reading
            break;
        }
        */
    }
}
