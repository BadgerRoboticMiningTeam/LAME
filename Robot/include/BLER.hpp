#pragma once

#include "Packet.h"
#include "Serial.hpp"
#include "UdpSocket.hpp"
#include "AprilTagsLocator.hpp"
#include <Xbox360Service.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using JoystickLibrary::Xbox360Service;


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
            void QueryHeartbeatAI();
            void CameraReadThread();

            bool isRunning;
            bool aiConnected;
            struct sockaddr_in baseStationAddr;
            struct sockaddr_in aiAddr;
            DriveMode currentMode;

            std::unique_ptr<SerialPort> serialPort;
            std::unique_ptr<UdpSocket> socket;
            Xbox360Service& js = Xbox360Service::GetInstance();

            std::thread serialReadThread;
            std::thread udpReadThread;
            std::thread executeThread;
            std::thread aiHeartbeatThread;
            std::thread cameraReadThread;
            std::mutex cameraMutex;

            cv::VideoCapture camera;
            cv::Mat latestCameraFrame;

            // packet payload structs //
            DrivePayload latestDrivePayload;
            time_t lastDrivePayloadReceivedTime;
            time_t lastAIHeartbeatReceivedTime;
    };
}
