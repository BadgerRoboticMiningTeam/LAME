#pragma once

#include "Timer.hpp"
#include <chrono>

namespace Framework
{
    class Service
    {
    public:
        const static std::chrono::milliseconds RUN_ON_PACKET_RECEIVE;

        Service(std::chrono::milliseconds interval, bool active);
        ~Service();
        
        void ExecuteOnTime();
        virtual bool HandlePacket(const uint8_t *buffer, int length);
        std::chrono::milliseconds GetSleepInterval() const;
        void SetSleepInterval(std::chrono::milliseconds sleep_interval);
        bool IsActive() const;
        void SetActive(bool active);

    protected:
        virtual void Execute();

        Timer execute_timer;
        std::chrono::milliseconds sleepInterval;
        bool isActive;
    };
}