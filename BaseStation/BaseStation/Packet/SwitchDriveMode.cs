
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
}
