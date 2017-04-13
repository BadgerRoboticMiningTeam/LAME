#include <iostream>
#include <memory>
#include <cstdlib>
#include "BLER.hpp"

using namespace LAME;


int main(int argc, char **argv)
{
    std::string serial_port = "";
	int port = 10000;
	int ai_remote_port = 11000;

	// parse command line arguments
	for (int i = 1; i < argc; i++)
	{
		if (strcmp("-p", argv[i]) == 0)
		{
			if (i + 1 > argc)
				break;
			
			port = atoi(argv[i + 1]);
			i++;
		}
		else if (strcmp("-s", argv[i]) == 0)
		{
			if (i + 1 > argc)
				break;

			serial_port = std::string(argv[i + 1]);
			i++;
		}
		else if (strcmp("-ai", argv[i]) == 0)
		{
			if (i + 1 > argc)
				break;

			ai_remote_port = atoi(argv[i + 1]);
			i++;
		}
	}

	if (serial_port == "")
	{
		std::cout << "Serial port not specified!" << std::endl;
		return 1;
	}

	std::unique_ptr<BLER> bler(new BLER(port, ai_remote_port, serial_port));
	if (!bler->Run())
	{
		std::cout << "Failed to start!" << std::endl;
		return 1;
	}

	std::cout << "Successfully initialized." << std::endl;
    char c;
    std::cin >> c;
//    while (true)
//        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    return 0;
}
