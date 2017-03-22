CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -pedantic -g
INCLUDE_FLAGS = -Iinclude -I../JoystickLibrary/cpp/include `pkg-config --cflags opencv libevdev`
LIB_FLAGS = -lpthread -L. -lJoystickLibrary `pkg-config --libs opencv libevdev` 

all:
	@$(CXX) src/*.cpp $(CXXFLAGS) $(INCLUDE_FLAGS) $(LIB_FLAGS) -o LAME

clean:
	rm -f LAME
