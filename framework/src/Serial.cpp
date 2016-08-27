#include "Serial.hpp"
#include <stdexcept>

using namespace LAME;


SerialPort::SerialPort()
{
    this->port = nullptr;
    this->baud = 0;
    this->opened = false;
}


SerialPort::SerialPort(const char *port, int baud)
{
    this->port = port;
    this->baud = baud;
    this->opened = false;
}

SerialPort::~SerialPort()
{
    this->Close();
}


bool SerialPort::PortIsOpen() const
{
    return this->opened;
}

#ifdef _WIN32
bool SerialPort::Open()
{
    DCB dcb;
    COMMTIMEOUTS commTO;
    
    if (!this->port)
        return false;

    handle = CreateFile(
        port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH,
        0
    );
        
    if (handle == INVALID_HANDLE_VALUE)
        return false;
    
    memset(&dcb, 0, sizeof(DCB));
    if (!GetCommState(handle, &dcb))
    {
        CloseHandle(handle);
        return false;
    }

    dcb.BaudRate = baud;
    dcb.fParity = FALSE;
    dcb.StopBits = ONESTOPBIT;
    dcb.ByteSize = 8;
    dcb.DCBlength = sizeof(dcb);
    if (!SetCommState(handle, &dcb))
    {
        CloseHandle(handle);
        return false;
    }
    
    memset(&commTO, 0, sizeof(COMMTIMEOUTS));
    commTO.ReadIntervalTimeout = MAXDWORD;
    if (!SetCommTimeouts(handle, &commTO))
    {
        CloseHandle(handle);
        return INVALID_HANDLE_VALUE;
    }

    this->opened = true;
    return true;
}

void SerialPort::Close()
{
    if (handle != INVALID_SERIAL_PORT_HANDLE)
        CloseHandle(handle);
}

int SerialPort::WriteData(const void *data, int size_bytes)
{
    DWORD bytesWritten;
    
    if (handle == INVALID_HANDLE_VALUE)
        return -1;
    
    if (!WriteFile(handle, data, size_bytes, &bytesWritten, nullptr))
        return -1;
    
    return bytesWritten;
}

int SerialPort::ReadData(void *data, int max_size_bytes)
{
    DWORD bytesRead;
    
    if (handle == INVALID_HANDLE_VALUE)
        return -1;
    
    if (!ReadFile(handle, data, max_size_bytes, &bytesRead, nullptr))
        return -1;
    
    return bytesRead;
}

#else
bool SerialPort::Open()
{
    struct termios serialPortProperties;

    if (!this->port)
        return false;

    handle = open(port, O_RDWR | O_NOCTTY);
    if (handle < 0)
        return false;
        
    memset(&serialPortProperties, 0, sizeof(struct termios));
    tcgetattr(handle, &serialPortProperties);
    cfsetospeed(&serialPortProperties, baud);
    cfsetispeed(&serialPortProperties, baud);

    serialPortProperties.c_cflag = (serialPortProperties.c_cflag & ~CSIZE) | CS8;
    serialPortProperties.c_cflag &= ~(PARENB | PARODD);
    serialPortProperties.c_cflag &= ~CSTOPB;
    tcflush(handle, TCIFLUSH);
    tcsetattr(handle, TCSANOW, &serialPortProperties);

    this->opened = true;  
    return true;
}

void SerialPort::Close()
{
    if (handle > 0)
        close(handle);
}

int SerialPort::WriteData(const void *data, int size_bytes)
{
    if (handle < 0)
        return -1;
        
    return write(handle, data, size_bytes);
}

int SerialPort::ReadData(void *data, int max_size_bytes)
{
    if (handle < 0)
        return -1;
        
    return read(handle, data, max_size_bytes);
}

#endif