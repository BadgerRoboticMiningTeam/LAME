#pragma once

#include <vector>
#include <memory>
#include <thread>
#include "Packet.h"
#include "Serial.hpp"
#include "UdpSocket.hpp"
#include "joystick.h"


namespace LAME
{
    enum class DriveMode
    {
        Direct,
        Remote, 
        AI
    };

    class BLER
    {
    public:
		BLER(int base_port, int ai_port, std::string& serial_port);
        ~BLER();

        void SendPacketSerial(const uint8_t *buffer, int length);
		void SendPacketUdp(const uint8_t *buffer, int length, struct sockaddr *addr);
        bool Run();
        

    private:
        void ReceivePacketsFromSerial();
        void ReceivePacketsFromUdp();
        void Execute();

        bool isRunning;
		struct sockaddr_in baseStationAddr;
        struct sockaddr_in aiAddr;
        DriveMode currentMode;

        std::unique_ptr<SerialPort> serialPort;
        std::unique_ptr<UdpSocket> socket;
        std::unique_ptr<JoystickLibrary::Xbox360Service> js;

        std::thread serialReadThread;
        std::thread udpReadThread;
        std::thread executeThread;

		// packet payload structs //
		DrivePayload latestDrivePayload;
		time_t lastDrivePayloadReceivedTime;
    };
}