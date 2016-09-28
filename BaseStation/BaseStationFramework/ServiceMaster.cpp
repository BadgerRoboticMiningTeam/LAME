#include "ServiceMaster.hpp"
#include "Service.hpp"
#include <cstdint>

using namespace FrameworkSharp;

ServiceMaster::ServiceMaster(int port)
{
    this->isRunning = false;
    this->services = gcnew List<Service^>();
    this->socket = gcnew UdpClient(port);
}

ServiceMaster::~ServiceMaster()
{
}

void ServiceMaster::AddService(Service^ s)
{
    this->services->Add(s);
    if (this->isRunning)
        s->ExecuteOnTime();
}

void ServiceMaster::SendPacket(Packet^ p)
{
    if (p == nullptr)
        return;

    auto data = p->Serialize();
    if (data == nullptr)
        return;

    this->socket->Send(data, data->Length);
}

void ServiceMaster::RegisterEndpoint(IPEndPoint^ addr)
{
    this->dest = addr;
}

void ServiceMaster::Run()
{
    // run the services
    for (int i = 0; i < this->services->Count; i++)
    {
        Service^ s = this->services[i];
        if (s->SleepInterval == Service::RUN_ON_PACKET_RECEIVE)
            s->ExecuteOnTime();
    }

    while (true)
    {
        auto src = gcnew System::Net::IPEndPoint(System::Net::IPAddress::Any, 0);
        auto data = this->socket->Receive(src);
        if (data->Length <= 0)
            continue;

        for (int i = 0; i < this->services->Count; i++)
        {
            Service^ s = this->services[i];
            if (!s->IsActive)
                continue;

            if (!s->HandlePacket(data, data->Length))
                continue;

            if (s->SleepInterval == Service::RUN_ON_PACKET_RECEIVE)
                s->ExecuteOnTime();

            // packet issued to service - continue reading
            break;
        }
    }
}
