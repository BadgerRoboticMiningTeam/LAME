#include "ServiceMaster.hpp"
#include "Service.hpp"
#include <cstdint>

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 256;

ServiceMaster::ServiceMaster(int port) : socket(port)
{
    this->isRunning = false;
}

ServiceMaster::~ServiceMaster()
{
}

void ServiceMaster::AddService(std::shared_ptr<Service> s)
{
    this->services.push_back(s);
    if (this->isRunning)
        s->ExecuteOnTime();
}

void ServiceMaster::RegisterEndpoint(struct sockaddr *addr)
{
    this->dest = addr;
}

void ServiceMaster::SendPacket(Packet *pkt)
{
    if (pkt == nullptr || this->dest == nullptr)
        return;

    auto data = pkt->Serialize();
    uint8_t *buffer = &data[0];
    this->socket.Write(buffer, data.size(), this->dest);
}

void ServiceMaster::Run()
{
    uint8_t read_buffer[READ_BUFFER_SIZE];

    if (!socket.Open())
        return;

    // run the services
    for (auto& s : this->services)
        if (s->GetSleepInterval() != Service::RUN_ON_PACKET_RECEIVE)
            s->ExecuteOnTime();

    memset(read_buffer, 0, READ_BUFFER_SIZE);

    while (true)
    {
        int bytes_read = socket.Read(read_buffer, READ_BUFFER_SIZE, nullptr);
        if (bytes_read < 0)
            continue;

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
    }
}
