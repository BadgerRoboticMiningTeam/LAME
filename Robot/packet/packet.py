import struct

# opcodes #
SWITCH_DRIVE_MODE = 0x10
SWITCH_DRIVE_MODE_ACK = 0x11
QUERY_HEARTBEAT = 0x12
REPORT_HEARTBEAT = 0x13
REPORT_SPEEDS = 0x14
SET_SPEEDS = 0x15
REPORT_BIN_POS = 0x16
SET_BIN_POS = 0x17

# drive modes #
DIRECT = 1
REMOTE = 2
AI = 3

# packet metadata #
HEADER_BYTE = 0xBAAD
END_BYTE = 0x7F
HEADER_SIZE = 5
HEADER_FORMAT_STR = '!HBB{}B'

# payload objects #
class WheelSpeeds(object):
    def __init__(self, front_left, back_left, front_right, back_right):
        self.front_left = front_left
        self.back_left = back_left
        self.front_right = front_right
        self.back_right = back_right

    def to_list(self):
        return [
            self.front_left,
            self.back_left,
            self.front_right,
            self.back_right
        ]

    def __str__(self):
        return "Speeds: {} {} {} {}".format(*self.to_list())

    def __repr__(self):
        return self.__str__()

def _serialize_packet(opcode, payload_fmt_str, payload):
    struct_fmt_str = HEADER_FORMAT_STR.format(payload_fmt_str)
    packer = struct.Struct(struct_fmt_str)
    return packer.pack(
        HEADER_BYTE,
        opcode,
        struct.calcsize(payload_fmt_str) + HEADER_SIZE,
        *payload,
        END_BYTE
    )

def peek_packet_header(buffer):
    if len(buffer) < HEADER_SIZE:
        return False

    if (buffer[0] << 8 | buffer[1]) != HEADER_BYTE:
        return False

    if buffer[-1] != END_BYTE:
        return False

    return buffer[2]

def _deserialize_packet(payload_fmt_str, buffer):
    struct_fmt_str = HEADER_FORMAT_STR.format(payload_fmt_str)
    if struct.calcsize(struct_fmt_str) != len(buffer):
        print("[Parse] Expected {}, got {}.".format(struct.calcsize(struct_fmt_str), len(buffer)))
        return False
    data = struct.unpack(struct_fmt_str, buffer)

    # check metadata
    if data[0] != HEADER_BYTE:
        return False

    if data[-1] != END_BYTE:
        return False

    if data[2] != len(buffer):
        return False

    # header OK, return tuple of (opcode, payload)
    return (data[1], data[3:-1])

def _serialize_empty_packet(opcode):
    return _serialize_packet(opcode, '', [])

def _deserialize_empty_packet(opcode, buffer):
    data = _deserialize_packet('', buffer)
    if not data:
        return False

    if data[0] != opcode:
        return False

    if len(data[1]) != 0:
        return False

    return True

def get_query_heartbeat():
    return _serialize_empty_packet(QUERY_HEARTBEAT)

def get_report_heartbeat():
    return _serialize_empty_packet(REPORT_HEARTBEAT)

def get_switch_drive_mode(drive_mode):
    return _serialize_packet(SWITCH_DRIVE_MODE, 'B', [drive_mode])

def get_switch_drive_mode_ack(drive_mode):
    return _serialize_packet(SWITCH_DRIVE_MODE_ACK, 'B', [drive_mode])

def get_set_speeds(speeds):
    return _serialize_packet(SET_SPEEDS, 'BBBB', speeds.to_list())

def get_set_wheel_speeds(speeds):
    return _serialize_packet(SET_SPEEDS, 'BBBB', speeds.to_list())

def parse_query_heartbeat(buffer):
    return _deserialize_empty_packet(QUERY_HEARTBEAT, buffer)

def parse_set_wheel_speeds(buffer):
    data = _deserialize_packet('BBBB', buffer)
    if not data:
        return False

    if data[0] != SET_SPEEDS:
        return False

    return WheelSpeeds(*data[1])

def parse_switch_drive_mode(buffer):
    data = _deserialize_packet('B', buffer)
    if not data:
        return False

    if data[0] != SWITCH_DRIVE_MODE:
        return False

    return data[1][0]
