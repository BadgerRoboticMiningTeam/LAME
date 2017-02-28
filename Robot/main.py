from twisted.internet import reactor
from LAME import LAME
import sys

LAUNCHPAD_I2C_ADDR = 0x01
LAME_PORT = 10000

# notes to remember
# i2c library
# joystick library (pygame, evdev)
# pkt implementation
# ai implementation 
# camera streaming

def main():
    reactor.listenUDP(LAME_PORT, LAME(LAUNCHPAD_I2C_ADDR))
    print("LAME is starting...")
    reactor.run()

if __name__ == "__main__":
    main()
