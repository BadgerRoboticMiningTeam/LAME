#include "ServiceMaster.hpp"
#include "Service.hpp"
#include <cstdint>
#include <cstring>
#include <climits>

using namespace LAME;

constexpr int READ_BUFFER_SIZE = 256;

ServiceMaster::ServiceMaster(int port) : socket(port), timer_interval(0)
{
}

ServiceMaster::~ServiceMaster()
{
}

void ServiceMaster::AddService(Service s)
{
    // dynamic adding of services not allowed
    if (this->executor_timer.IsRunning())
        return;

    this->services.push_back(s);
}

static void ExecuteServices()
{

}

void ServiceMaster::Run()
{
    uint8_t read_buffer[READ_BUFFER_SIZE];

    if (!socket.Open())
        return;

    // determine interval to run
    this->timer_interval = std::chrono::milliseconds(INT_MAX);
    for (const Service& s : this->services)
    {
        auto service_interval = s.GetSleepInterval();
        if (service_interval != Service::RUN_ON_PACKET_RECEIVE && service_interval < this->timer_interval)
            timer_interval = service_interval;
    }


    // set up timer
    this->executor_timer.SetPeriod(this->timer_interval);
    this->executor_timer.SetCallback(&ExecuteServices);
    this->executor_timer.Start();

    memset(read_buffer, 0, READ_BUFFER_SIZE);

    while (true)
    {
        int bytes_read = socket.Read(read_buffer, READ_BUFFER_SIZE, nullptr);
        if (bytes_read < 0)
            continue;

        for (Service& s : this->services)
        {
            if (!s.IsActive())
                continue;

            if (!s.HandlePacket(read_buffer, bytes_read))
                continue;

            if (s.GetSleepInterval() == Service::RUN_ON_PACKET_RECEIVE)
                s.Execute();

            // packet issued to service - continue reading
            break;
        }
    }
}
