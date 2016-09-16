#include "ServiceMaster.hpp"
#include "Service.hpp"
#include <cstdint>

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 256;

ServiceMaster::ServiceMaster(int port)
{
    this->isRunning = false;
    this->socket = new UdpSocket(port);
    this->services = gcnew List<Service^>();
}

ServiceMaster::~ServiceMaster()
{
    if (this->socket)
    {
        this->socket->Close();
        delete this->socket;
    }
}

void ServiceMaster::AddService(Service^ s)
{
    this->services->Add(s);
    if (this->isRunning)
        s->ExecuteOnTime();
}

void ServiceMaster::Run()
{
    uint8_t raw_read_buffer[READ_BUFFER_SIZE];

    if (!socket->Open())
        return;

    // run the services
    for (int i = 0; i < this->services->Count; i++)
    {
        Service^ s = this->services[i];
        if (s->SleepInterval == Service::RUN_ON_PACKET_RECEIVE)
            s->ExecuteOnTime();
    }

    memset(raw_read_buffer, 0, READ_BUFFER_SIZE);

    while (true)
    {
        int bytes_read = socket->Read(raw_read_buffer, READ_BUFFER_SIZE, nullptr);
        if (bytes_read < 0)
            continue;

        array<System::Byte>^ managed_buffer = gcnew array<System::Byte>(bytes_read);
        for (int i = 0; i < bytes_read; i++)
            managed_buffer[i] = raw_read_buffer[i];

        for (int i = 0; i < this->services->Count; i++)
        {
            Service^ s = this->services[i];
            if (!s->IsActive)
                continue;

            if (!s->HandlePacket(managed_buffer, bytes_read))
                continue;

            if (s->SleepInterval == Service::RUN_ON_PACKET_RECEIVE)
                s->ExecuteOnTime();

            // packet issued to service - continue reading
            break;
        }
    }
}
