#pragma once
#using <System.dll>

#include "Service.hpp"
#include "Packet.hpp"
#include "../../framework/include/UdpSocket.hpp"

using System::Threading::Thread;
using System::Collections::Generic::List;
using System::Net::IPEndPoint;
using System::Net::Sockets::UdpClient;

namespace FrameworkSharp
{
    public ref class ServiceMaster
    {
    public:
        ServiceMaster(int port);
        ~ServiceMaster();
        void AddService(Service^ s);
        void Run();
        void SendPacket(Packet^ p);
        void RegisterEndpoint(IPEndPoint^ dest);

    private:
        List<Service^>^ services;
        System::Boolean isRunning;
        UdpClient^ socket;
        IPEndPoint^ dest;

    };
}

