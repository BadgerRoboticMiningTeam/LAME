CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -pedantic -g
INCLUDE_FLAGS = -Iinclude -I../JoystickLibrary/cpp/include -Iapriltags `pkg-config --cflags opencv libevdev libudev`
LIB_FLAGS = -lpthread $(APRILTAG_DIR)/libapriltag.a -L. -lJoystickLibrary `pkg-config --libs opencv libevdev libudev` 
APRILTAG_DIR = apriltags/

all:
	$(MAKE) -C $(APRILTAG_DIR)
	@$(CXX) src/*.cpp $(CXXFLAGS) $(INCLUDE_FLAGS) $(LIB_FLAGS) -o LAME

clean:
	rm -f LAME
