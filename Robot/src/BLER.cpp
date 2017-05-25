#include "BLER.hpp"
#include "Packet.h"
#include "TcpClient.hpp"
#include <cstdint>
#include <iostream>
#include <cstdio>

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 64;
constexpr int AI_TIMEOUT_INTERVAL = 5; // AI dead timeout 
constexpr int XBOX_DEADZONE = 20;
constexpr int BASE_IMAGE_PORT = 11000;
constexpr int BAUD_RATE = 9600;

std::vector<int> params = {
    CV_IMWRITE_JPEG_QUALITY,
    5
};


BLER::BLER(int base_port, int ai_port, std::string& serial_port) : 
    socket(new UdpSocket(base_port)),
    serialPort(new SerialPort(serial_port, BAUD_RATE))
{
    // init serial port //
    serialPort->Open();

    std::memset(&this->aiAddr, 0, sizeof(struct sockaddr_in));
    this->aiAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    this->aiAddr.sin_family = AF_INET;
    this->aiAddr.sin_port = htons(ai_port);

    // init other fields //
    this->lastDrivePayloadReceivedTime = time(nullptr);
    this->lastAIHeartbeatReceivedTime = time(nullptr);
    this->isRunning = false;
    this->aiConnected = false;
    this->currentMode = DriveMode::Direct;
    memset(&latestDrivePayload, 0, sizeof(DrivePayload));
}

BLER::~BLER()
{
    this->isRunning = false;

    // close serialport, udpsocket
    serialPort->Close();
    socket->Close();

    // join on all threads
    if (aiHeartbeatThread.joinable())
        aiHeartbeatThread.join();

    if (serialReadThread.joinable())
        serialReadThread.join();

    if (udpReadThread.joinable())
        udpReadThread.join();

    if (executeThread.joinable())
        executeThread.join();
}

void BLER::SendPacketSerial(const uint8_t *buffer, int length)
{
    this->serialPort->Write(buffer, length);
}

void BLER::SendPacketUdp(const uint8_t *buffer, int length, struct sockaddr *addr)
{
    if (addr == nullptr)
        return;
/*
    printf("buffer: ");
    for (int i = 0; i < length; i++)
        printf(" %x ", buffer[i]);
    printf("\n");    
*/
    this->socket->Write(buffer, length, addr);
}

void BLER::ReceivePacketsFromSerial()
{
    uint8_t read_buffer[READ_BUFFER_SIZE];
    memset(read_buffer, 0, READ_BUFFER_SIZE);

    while (this->isRunning)
    {
        int bytes_read = serialPort->Read(read_buffer, READ_BUFFER_SIZE);
        if (bytes_read < 0)
            continue;

        // TODO: HANDLE SENSOR PACKETS HERE

        memset(&read_buffer, 0, READ_BUFFER_SIZE);
    }
}


void BLER::ReceivePacketsFromUdp()
{
    uint8_t read_buffer[READ_BUFFER_SIZE];
    memset(read_buffer, 0, READ_BUFFER_SIZE);

    while (this->isRunning)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));

        int bytes_read = socket->Read(read_buffer, READ_BUFFER_SIZE, (struct sockaddr *) &addr);
        if (bytes_read < 0)
            continue;

        bool is_from_ai = addr.sin_family == AF_INET && 
            addr.sin_addr.s_addr == htonl(INADDR_LOOPBACK) && 
            addr.sin_port == this->aiAddr.sin_port;

        if (!is_from_ai)
        {
            baseStationAddr = addr;
        }

        // parse packets //
        uint8_t opcode = 0;
        uint8_t *payload = nullptr;
        if (!ReadPacketHeader(read_buffer, (uint8_t)bytes_read, &opcode, &payload))
            continue;

        switch (opcode)
        {
            case QUERY_HEARTBEAT_OPCODE:
                {
                    uint8_t pkt[16];
                    memset(pkt, 0, 16);
                    int bytes_written = CreateReportHeartbeatPacket(pkt, 16);
                    std::cout << "Sent ReportHeartbeat" << std::endl;
                    this->SendPacketUdp(pkt, bytes_written, (struct sockaddr *) &addr);

                    if (is_from_ai)
                    {
                        if (!aiConnected)
                            aiConnected = true;
                        this->lastAIHeartbeatReceivedTime = time(nullptr);
                    }
                    break;
                }

            case AI_SWITCH_OPCODE:
                {
                    uint8_t pkt[16];
                    memset(pkt, 0, 16);
                    this->currentMode = DriveMode::AI;
                    int bytes_written = CreateSwitchModeAckPacket(pkt, 16);
                    this->SendPacketUdp(pkt, bytes_written, (struct sockaddr *) &addr);
                    std::cout << "Switched to AI mode" << std::endl;

                    if (!aiConnected)
                    {
                        std::cout << "AI is not connected!" << std::endl;
                    }
                    else
                    {
                        // begin mining cycle
                        memset(pkt, 0, 16);
                        bytes_written = CreateAiInitPacket(pkt, 16);
                        this->SendPacketUdp(pkt, bytes_written, (struct sockaddr *) &addr);
                    }
                    break;
                }

            case REMOTE_SWITCH_OPCODE:
                {
                    uint8_t pkt[16];
                    memset(pkt, 0, 16);
                    lastDrivePayloadReceivedTime = time(nullptr);
                    this->currentMode = DriveMode::Remote;
                    int bytes_written = CreateSwitchModeAckPacket(pkt, 16);
                    this->SendPacketUdp(pkt, bytes_written, (struct sockaddr *) &baseStationAddr);
                    std::cout << "Switched to Remote mode" << std::endl;
                    break;
                }

            case DIRECT_SWITCH_OPCODE:
                this->currentMode = DriveMode::Direct;
                break;

            case QUERY_CAMERA_IMAGE_OPCODE:
                this->SendCameraImage(0);
                break;

            case QUERY_CAMERA1_IMAGE_OPCODE:
                this->SendCameraImage(1);
                break;

            case SET_CAMERA_QUALITY_OPCODE:
                CameraQualityPayload localCamPayload;
                ParseQualityPayload(payload, &localCamPayload);
                params[1] = localCamPayload.quality > 100 ? 100 : localCamPayload.quality;
                std::cout << "Setting quality to " << params[1] << std::endl;
                break;

            case DRIVE_OPCODE:
                DrivePayload localPayload;
                ParseDrivePayload(payload, &localPayload);
                this->latestDrivePayload = localPayload;
                this->lastDrivePayloadReceivedTime = time(nullptr);
                break;

            case ENABLE_ENCODER_OPCODE:
                std::cout << "Enabling encoder-assisted drive!" << std::endl;
                this->SendPacketSerial(read_buffer, bytes_read);
                break;

            case DISABLE_ENCODER_OPCODE:
                std::cout << "Disabling encoder-assisted drive!" << std::endl;
                this->SendPacketSerial(read_buffer, bytes_read);
                break;

            default:
                continue;
        }

        memset(read_buffer, 0, READ_BUFFER_SIZE);
    }
}

void BLER::SendCameraImage(int id)
{
    cv::Mat img, filtered;
    std::vector<uchar> raw_compressed_data;
    struct sockaddr_in img_addr;

    // read a frame //
    cameraMutex.lock();
    if (id == 0)
        img = this->latestCameraFrame;
    else
        img = this->latestCamera1Frame;
    cameraMutex.unlock();

    if (!img.data)
        return;

    // convert to grayscale, then compress it //
    cv::cvtColor(img, filtered, cv::COLOR_BGR2GRAY);
    if (!cv::imencode(".jpg", filtered, raw_compressed_data, params))
    {
        std::cout << "Failed to encode camera data!" << std::endl;
        return;
    }

    // send to base station //
    std::unique_ptr<TcpClient> tcpClient(new TcpClient());
    if (!tcpClient->Open())
    {
        std::cout << "Failed to open TCP port for image send!" << std::endl;
        return;
    }

    // set up image port address //
    img_addr = this->baseStationAddr;
    img_addr.sin_port = htons(BASE_IMAGE_PORT);
    if (!tcpClient->Connect(img_addr))
    {
        std::cout << "Failed to connect to base station image port?" << std::endl;
        return;
    }

    // send header //
    auto size = raw_compressed_data.size();
    uint8_t firstPkt[] = { 0xAA, 0, 0, 0, 0, (uint8_t)id, 0x7F };
    firstPkt[1] = (uint8_t)(size >> 24);
    firstPkt[2] = (uint8_t)(size >> 16);
    firstPkt[3] = (uint8_t)(size >> 8);
    firstPkt[4] = (uint8_t)size;
    tcpClient->Write(firstPkt, 7);

    // send data //
    tcpClient->Write(&raw_compressed_data[0], static_cast<unsigned int>(size));
    std::cout << "Sent an image back to base station!" << std::endl;
}

void BLER::Execute()
{
    while (this->isRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        uint8_t buffer[64];
        memset(buffer, 0, 64);

        switch (this->currentMode)
        {
            case DriveMode::Direct:
                {
                    std::vector<int> jsIDs;
                    DrivePayload payload;
                    int left, right;

                    jsIDs = js.GetIDs();
                    if (jsIDs.size() < 1)
                        continue;

                    int js_id = jsIDs[0];
                    payload.left = js.GetLeftY(js_id, left) ? left : 0;
                    payload.right = js.GetRightY(js_id, right) ? right : 0;

                    if (payload.left > -XBOX_DEADZONE && payload.left < XBOX_DEADZONE)
                        payload.left = 0;
                    if (payload.right > -XBOX_DEADZONE && payload.right < XBOX_DEADZONE)
                        payload.right = 0;

                    int bytes_written = CreateDrivePacket(buffer, 64, payload);
                    this->SendPacketSerial(buffer, bytes_written);
                    break;
                }

            case DriveMode::Remote:
                {
                    DrivePayload localPayload = this->latestDrivePayload;

                    // timeout: 2 seconds
                    if (difftime(time(nullptr), this->lastDrivePayloadReceivedTime) > 2)
                    {
                        localPayload.left = 0;
                        localPayload.right = 0;
                    }

                    int bytes_written = CreateDrivePacket(buffer, 64, localPayload);
                    printf("L: %d R: %d Ac: %d V: %d S: %d \n", localPayload.left, localPayload.right, localPayload.actuator, localPayload.vibrator, localPayload.scooper);
                    this->SendPacketSerial(buffer, bytes_written);
                    break;
                }

            case DriveMode::AI:
                break;

            default:
                continue;
        }
    }
} 

void BLER::QueryHeartbeatAI()
{
    uint8_t queryBuffer[16];
    uint8_t remoteBuffer[16];

    memset(queryBuffer, 0, 16);
    memset(remoteBuffer, 0, 16);

    int queryHeartbeatWritten = CreateQueryHeartbeatPacket(queryBuffer, 16);
    int remoteWritten = CreateRemoteSwitchPacket(remoteBuffer, 16);

    while (this->isRunning)
    {
        if (this->currentMode != DriveMode::AI)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // AI is probably dead, revert to remote control
        if (difftime(time(nullptr), this->lastAIHeartbeatReceivedTime) > AI_TIMEOUT_INTERVAL)
        {
            this->currentMode = DriveMode::Remote;

            // notify base station
            this->SendPacketUdp(remoteBuffer, remoteWritten, (struct sockaddr *) &this->baseStationAddr);
        }

        this->SendPacketUdp(queryBuffer, queryHeartbeatWritten, (struct sockaddr *) &this->aiAddr);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void BLER::CameraReadThread()
{
    while (this->isRunning)
    {
        cv::Mat img, img1;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (camera.isOpened())
            camera >> img; 
        if (camera1.isOpened())
            camera1 >> img1;

        if (!img.data && !img1.data)
            continue;

        cameraMutex.lock();
        if (img.data)
            this->latestCameraFrame = img;
        if (img1.data)
            this->latestCamera1Frame = img1;
        cameraMutex.unlock();
    }
}

void BLER::CameraSendThread()
{
    while (this->isRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        this->SendCameraImage(0);
        this->SendCameraImage(1);
    }
}

bool BLER::Run()
{
    if (!socket->Open())
    {
        std::cout << "Failed to open socket!" << std::endl;
        return false;
    }

    if (!serialPort->Open())
    {
        std::cout << "Failed to open serial port!" << std::endl;
        return false;
    }

    // initialize joystick //
    if (!js.Initialize())
    {
        std::cout << "Failed to initialize joysticks!" << std::endl;
        return false;
    }

    // initialize cameras //
    if (!camera.open(0))
    {
        std::cout << "Failed to open camera 0!" << std::endl;
        //return false;
    }

    if (!camera1.open(1))
    {
        std::cout << "Failed to open camera 1!" << std::endl;
        //return false;
    }

    this->isRunning = true;

    // spin off threads //
    serialReadThread = std::thread(&BLER::ReceivePacketsFromSerial, this);
    udpReadThread = std::thread(&BLER::ReceivePacketsFromUdp, this);
    executeThread = std::thread(&BLER::Execute, this);
    cameraReadThread = std::thread(&BLER::CameraReadThread, this);
    cameraSendThread = std::thread(&BLER::CameraSendThread, this);

    // init successful //
    return true;
}
