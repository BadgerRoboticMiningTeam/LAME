#include "Service.hpp"
#include <functional>

using namespace Framework;


Service::Service(std::chrono::milliseconds interval, bool active)
{
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

std::chrono::milliseconds Service::GetSleepInterval() const
{
    return this->sleepInterval;
}

void Service::SetSleepInterval(std::chrono::milliseconds sleep_interval)
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
    execute_timer.SetCallback(std::bind(&Service::Execute, this));
    execute_timer.SetPeriod(this->sleepInterval);
    execute_timer.SetPeriodic(true);
    execute_timer.Start();
}


