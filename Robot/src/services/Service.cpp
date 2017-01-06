#include "Service.hpp"
#include <functional>

using namespace LAME;

Service::Service(ServiceMaster* master) : Service(master, RUN_ON_PACKET_RECEIVE, false)
{
}

Service::Service(ServiceMaster* master, int interval, bool active)
{
    this->serviceMaster = master;
    this->sleepInterval = interval;
    this->isActive = active;
}

Service::~Service()
{
    this->execute_timer.Stop();
}

void Service::Execute()
{
}

bool Service::HandlePacket(const uint8_t *buffer, int length)
{
    return false;
}

int Service::GetSleepInterval() const
{
    return this->sleepInterval;
}

void Service::SetSleepInterval(int sleep_interval)
{
    this->sleepInterval = sleep_interval;
}

bool Service::IsActive() const
{
    return this->isActive;
}

void Service::SetActive(bool active)
{
    this->isActive = active;
}

void Service::ExecuteOnTime()
{
    if (this->sleepInterval == RUN_ON_PACKET_RECEIVE)
        return;

    execute_timer.SetCallback(std::bind(&Service::Execute, this));
    execute_timer.SetPeriod(std::chrono::milliseconds(this->sleepInterval));
    execute_timer.SetPeriodic(true);
    execute_timer.Start();
}


