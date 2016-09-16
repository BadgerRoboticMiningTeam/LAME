#pragma once

#include "Service.hpp"
#include "UdpSocket.hpp"

using System::Threading::Thread;
using System::Collections::Generic::List;

namespace LAME
{
    public ref class ServiceMaster
    {
    public:
        ServiceMaster(int port);
        ~ServiceMaster();
        void AddService(Service^ s);
        void Run();

    private:
        UdpSocket *socket;
        List<Service^>^ services;
        System::Boolean isRunning;
    };
}

