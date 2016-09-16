#include "Service.hpp"
#include <functional>

using namespace LAME;
using System::Threading::TimerCallback;

void ExecuteTimerCallback(System::Object^ service)
{
    static_cast<Service^>(service)->Execute();
}

Service::Service(System::Int32 interval, System::Boolean active)
{
    this->SleepInterval = interval;
    this->IsActive = active;
}

Service::~Service()
{

}

void Service::ExecuteOnTime()
{
    this->execute_timer = gcnew Timer(gcnew TimerCallback(&ExecuteTimerCallback), this, 0, this->SleepInterval);
}

void Service::Execute()
{
}

System::Boolean Service::HandlePacket(array<System::Byte>^ buffer, System::Int32 length)
{
    return false;
}
