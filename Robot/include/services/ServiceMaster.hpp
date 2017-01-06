#pragma once

#include "Service.hpp"
#include "UdpSocket.hpp"
#include "Timer.hpp"
#include "Packet.hpp"
#include <vector>
#include <memory>

namespace LAME
{
    class Service;

    class ServiceMaster
    {
    public:
        ServiceMaster(int port);
        ~ServiceMaster();
        void AddService(std::shared_ptr<Service> s);
        void RegisterEndpoint(struct sockaddr *dest);
        void Run();
        void SendPacket(Packet *pkt);

    private:
        UdpSocket socket;
        std::vector<std::shared_ptr<Service>> services;
        std::chrono::milliseconds timer_interval;
        bool isRunning;
        struct sockaddr *dest;
    };
}