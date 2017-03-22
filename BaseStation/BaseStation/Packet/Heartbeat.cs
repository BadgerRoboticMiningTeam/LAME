
namespace BaseStation.Packet
{
    public class QueryHeartbeat : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.QueryHeartbeat; }
        }
    }

    public class ReportHeartbeat : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.ReportHeartbeat; }
        }
    }
}
