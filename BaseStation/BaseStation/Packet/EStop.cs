
namespace BaseStation.Packet
{
    public class EStop : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get
            {
                return PacketOpcode.EStop;
            }
        }
    }

    public class ClearEStop : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get
            {
                return PacketOpcode.ClearEStop;
            }
        }
    }
}
