from twisted.internet.protocol import DatagramProtocol
from packet import packet
from ai import ai
import sys


class LAME(DatagramProtocol):
    def __init__(self, launchpad_addr):
        self.packet_handlers = {
            packet.SWITCH_DRIVE_MODE: self._switch_mode
        }
        self.dest = None
        self.drive_mode = packet.DIRECT
        self.ai = AI()

    def datagramReceived(self, data, addr):
        if not self.dest:
            self.dest = addr

        # process packet
        opcode = packet.peek_packet_header(data)
        if not opcode:
            print("Received {}, but is not valid packet!".format(data))
            return

        if opcode not in self.packet_handlers:
            print("Received unknown packet with opcode {0}".format(opcode))
            return

        return self.packet_handlers[opcode](data)

    def send_packet(self, buffer):
        if not self.dest:
            return False
        self.transport.write(buffer, self.dest)
        return True

    # packet handlers #
    def _switch_mode(self, data):
        mode = packet.parse_switch_drive_mode(data)
        if not mode:
            return False
        self.drive_mode = mode

        # send ack
        ack_data = packet.get_switch_drive_mode_ack(mode)
        self.send_packet(ack_data)
        return True
