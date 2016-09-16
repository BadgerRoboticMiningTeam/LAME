#pragma once

using System::Threading::Timer;

namespace LAME
{
    public ref class Service
    {
    public:
        const static System::Int32 RUN_ON_PACKET_RECEIVE = -1;
        Service(System::Int32 interval, System::Boolean active);
        ~Service();
        void ExecuteOnTime();
        virtual void Execute();
        virtual System::Boolean HandlePacket(array<System::Byte>^ buffer, System::Int32 length);

        System::Boolean IsActive;
        System::Int32 SleepInterval;

    protected:
        Timer^ execute_timer;
    };
}

