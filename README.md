# LAME
Stands for "Lunar Autonomous Mining Entity". But really, it's just an excuse to call our software LAME 😊. 


## Overview
The higher level control software for BLER is split into 2 parts: the BaseStation and Robot code.
The base station code (GUI) is written in C# with its own implementation of the packet structure.
The robot, its own as well. It's easier to write it twice than attempt a shim layer, especially if it's for
a small thing like buffer packing.

#### Packet Structure
The robot and base station communicate via packets. The structure is given below. 

| Byte Index  |     0 - 1     |       2     |       3       |      4      |  5 ... n - 1  |      n       |
| ----------- | ------------- | ----------- | ------------- | ----------- | ------------- | ------------ |
| Description | Header (0xBEEF) | Opcode | Payload size | Payload CRC | Payload | End bytes (0x7F) |

#### Services
Services are independent pieces of code that, at runtime, can be selected to run, either at regular intervals or when a packet is received.
In order to mediate which services are running, each service is registered to a ServiceMaster. A client can send control packets activate/deactivating services.


### Base Station
The base station code is a C# WPF application. It is responsible for moderating the status of the robot and switching drive modes if necessary.

### Robot
The robot code is a native C++ application. It supports (or will support!) 3 drive modes (tethered/direct, remote, and AI). 
It is also responsible for sending back images of the field upon base station request, and more.


## Building

### Robot
You must have JoystickLibrary (https://github.com/WisconsinRobotics/JoystickLibrary) cloned in the top level.
The static C++ library must be copied into the Robot/ folder.

```
cd Robot/
mkdir build
cd build
cmake .. 
cmake --build . 
```

### Base Station
You will need the C# version of the aforementioned JoystickLibrary, and copy the compiled dll into the BaseStation folder.
Then open the solution and build within Visual Studio.
