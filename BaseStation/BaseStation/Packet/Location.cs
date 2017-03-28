
namespace BaseStation.Packet
{
    public class QueryLocation : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.QueryLocation; }
        }
    }

    public class ReportLocation : Packet
    {
        public int X
        {
            get; set;
        }

        public int Y
        {
            get; set;
        }

        public int Heading
        {
            get; set;
        }

        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.ReportLocation; }
        }

        protected override PacketParseError DeserializePayload(byte[] payload)
        {
            if (payload.Length != 6)
                return PacketParseError.BadPayload;

            X = (payload[0] << 8) | payload[1];
            Y = (payload[2] << 8) | payload[3];
            Heading = (payload[4] << 8) | payload[5];
            return PacketParseError.OK;
        }

        protected override byte[] SerializePayload()
        {
            byte[] payload = new byte[6];
            payload[0] = (byte)(X >> 8);
            payload[1] = (byte)(X & 0xFF);
            payload[2] = (byte)(Y >> 8);
            payload[3] = (byte)(Y & 0xFF);
            payload[4] = (byte)(Heading >> 8);
            payload[5] = (byte)(Heading & 0xFF);
            return payload;
        }
    }

    public class SetLocation : ReportLocation
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.SetLocation; }
        }
    }
}
