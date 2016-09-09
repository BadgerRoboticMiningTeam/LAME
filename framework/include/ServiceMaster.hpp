#pragma once

#include "Service.hpp"
#include "UdpSocket.hpp"
#include "Timer.hpp"
#include <vector>
#include <memory>


namespace LAME
{
    class ServiceMaster
    {
    public:
        ServiceMaster(int port);
        ~ServiceMaster();
        void AddService(std::shared_ptr<Service> s);
        void Run();

    private:
        UdpSocket socket;
        std::vector<std::shared_ptr<Service>> services;
        std::chrono::milliseconds timer_interval;
        bool isRunning;
    };
}