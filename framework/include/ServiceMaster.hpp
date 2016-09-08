#pragma once
#include "Service.hpp"
#include "UdpSocket.hpp"
#include <vector>

namespace LAME
{
    class ServiceMaster
    {
    public:
        ServiceMaster(int port);
        ~ServiceMaster();
        void AddService(Service s);
        void Run();
    private:
        UdpSocket socket;
        std::vector<Service> services;
        int timer_interval;
    };
}