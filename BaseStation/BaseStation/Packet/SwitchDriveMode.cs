
using System;

namespace BaseStation.Packet
{
    public class SwitchToAIDrive : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.SwitchToAIDrive; }
        }
    }

    public class SwitchToRemoteDrive : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.SwitchToRemoteDrive; }
        }
    }

    public class SwitchToDirectDrive : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.SwitchToDirectDrive; }
        }
    }

    public class SwitchDriveModeAck : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.SwitchDriveModeAck; }
        }
    }
}
