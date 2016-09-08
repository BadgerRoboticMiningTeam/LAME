#pragma once

#include "Service.hpp"
#include "UdpSocket.hpp"
#include "Timer.hpp"
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
        std::chrono::milliseconds timer_interval;
        Timer executor_timer;
    };
}