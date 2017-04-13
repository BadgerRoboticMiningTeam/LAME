#include "BLER.hpp"
#include "Packet.h"
#include <cstdint>
#include <iostream>
#include <cstdio>

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 64;
constexpr int AI_TIMEOUT_INTERVAL = 5; // AI dead timeout 


BLER::BLER(int base_port, int ai_port, std::string& serial_port) : 
	socket(new UdpSocket(base_port)),
	serialPort(new SerialPort(serial_port, 115200))
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

    printf("buffer: ");
    for (int i = 0; i < length; i++)
        printf(" %x ", buffer[i]);
    printf("\n");    

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
				lastDrivePayloadReceivedTime = time(nullptr);
				this->currentMode = DriveMode::Remote;
                int bytes_written = CreateSwitchModeAckPacket(pkt, 16);
                this->SendPacketUdp(pkt, bytes_written, (struct sockaddr *) &addr);
                std::cout << "Switched to Remote mode" << std::endl;
				break;
            }

			case DIRECT_SWITCH_OPCODE:
				this->currentMode = DriveMode::Direct;
				break;

			case QUERY_CAMERA_IMAGE_OPCODE:
				// TODO: read camera
				// TODO: send to appropriate location
				break;

			case DRIVE_OPCODE:
				break;

			default:
				continue;
		}

        memset(read_buffer, 0, READ_BUFFER_SIZE);
    }
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
				this->SendPacketSerial(buffer, bytes_written);
				break;
			}

            case DriveMode::AI:
                // TODO:
                break;
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

bool BLER::Run()
{
    if (!socket->Open()) //|| !serialPort->Open())
        return false;

    // initialize joystick //
	if (!js.Initialize())
		return false;

    this->isRunning = true;

    // spin off threads //
	serialReadThread = std::thread(&BLER::ReceivePacketsFromSerial, this);
	udpReadThread = std::thread(&BLER::ReceivePacketsFromUdp, this);
	executeThread = std::thread(&BLER::Execute, this);
    aiHeartbeatThread = std::thread(&BLER::QueryHeartbeatAI, this);

    // init successful //
    return true;
}
