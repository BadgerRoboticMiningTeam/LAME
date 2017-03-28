using System;

namespace BaseStation.Packet
{
    public class QueryEncoderPacket : NoPayloadPacket
    {
        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.QueryEncoder; }
        }
    }

    public class ReportEncoderPacket : Packet
    {
        public int LeftFront
        {
            get; set;
        }

        public int RightFront
        {
            get; set;
        }

        public int LeftBack
        {
            get; set;
        }

        public int RightBack
        {
            get; set;
        }

        public int LeftActuator
        {
            get; set;
        }

        public int RightActuator
        {
            get; set;
        }

        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.ReportEncoder; }
        }

        protected override PacketParseError DeserializePayload(byte[] payload)
        {
            if (payload.Length != 12)
                return PacketParseError.BadPayload;

            LeftFront = (payload[0] << 8) | payload[1];
            RightFront = (payload[2] << 8) | payload[3];
            LeftBack = (payload[4] << 8) | payload[5];
            RightBack = (payload[6] << 8) | payload[7];
            LeftActuator = (payload[8] << 8) | payload[9];
            RightActuator = (payload[10] << 8) | payload[11];
            return PacketParseError.OK;
        }

        protected override byte[] SerializePayload()
        {
            throw new NotImplementedException();
        }
    }
}
