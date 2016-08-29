#pragma once

#ifdef _WIN32
    #include <windows.h>
    typedef HANDLE SerialPortHandle; 
#else
    #include <stdlib.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <termios.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <string.h>

    typedef int SerialPortHandle;
#endif

#define INVALID_SERIAL_PORT_HANDLE ((SerialPortHandle) (-1))

namespace LAME
{
    class SerialPort
    {
    public:
        SerialPort();
        SerialPort(const char *port, int baud);
        ~SerialPort();
        bool Open();
        void Close();
        bool PortIsOpen() const;
        int WriteData(const void *data, int size_bytes);
        int ReadData(void *data, int max_size_bytes);
        
    private:
        SerialPortHandle handle;
        bool opened;
        const char *port;
        int baud;
    };
}