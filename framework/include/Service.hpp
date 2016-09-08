#pragma once

#include <chrono>

namespace LAME
{
    class Service
    {
    public:
        const static std::chrono::milliseconds RUN_ON_PACKET_RECEIVE;

        Service(std::chrono::milliseconds interval, bool active);
        ~Service();
        virtual void Execute();
        virtual bool HandlePacket(const uint8_t *buffer, int length);
        std::chrono::milliseconds GetSleepInterval() const;
        void SetSleepInterval(std::chrono::milliseconds sleep_interval);
        bool IsActive() const;
        void SetActive(bool active);

    protected:
        std::chrono::system_clock::time_point lastTimeRun;
        std::chrono::milliseconds sleepInterval;
        bool isActive;
    };
}