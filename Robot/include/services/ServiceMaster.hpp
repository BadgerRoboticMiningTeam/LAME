#pragma once

#include "Service.hpp"
#include "UdpSocket.hpp"
#include "Timer.hpp"
#include "Packet.hpp"
#include "Serial.hpp"
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
        void Run();
        void SendPacket(const Packet& pkt);
        

    private:
        std::unique_ptr<SerialPort> serialPort;

        UdpSocket socket;
        bool isRunning;
        struct sockaddr *dest;

        struct ServiceMasterImpl;
        struct ServiceMasterImpl *services;
    };
}