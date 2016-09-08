#pragma once


namespace LAME
{
    class Service
    {
    public:
        constexpr int RUN_ON_PACKET_RECEIVE = -1;

        Service(int sleep_interval, bool active);
        ~Service();
        virtual void Execute();
        virtual bool HandlePacket(const uint8_t *buffer, int length);
        int GetSleepInterval() const;
        void SetSleepInterval(int sleep_interval);
        bool IsActive() const;
        void SetActive(bool active);

    protected:
        int sleepInterval;
        bool isActive;
    };
}