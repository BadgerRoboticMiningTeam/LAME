
namespace BaseStation.Packet
{
    public class Drive : Packet
    {
        const int MAX_SPEED = 100;
        const int MIN_SPEED = -100;

        int left;
        int right;

        public Drive() : this(0, 0)
        {
        }

        public Drive(int left, int right)
        {
            Left = left;
            Right = right;
        }

        public int Left
        {
            get { return left; }
            set { left = Util.Clamp(value, MIN_SPEED, MAX_SPEED); }
        }

        public int Right
        {
            get { return right; }
            set { right = Util.Clamp(value, MIN_SPEED, MAX_SPEED); }
        }

        public override PacketOpcode Opcode
        {
            get { return PacketOpcode.Drive; }
        }

        protected override PacketParseError DeserializePayload(byte[] payload)
        {
            if (payload.Length != 2)
                return PacketParseError.BadPayload;

            Left = (sbyte) payload[0];
            Right = (sbyte) payload[1];
            return PacketParseError.OK;
        }

        protected override byte[] SerializePayload()
        {
            byte[] payload = new byte[2];
            payload[0] = (byte) left;
            payload[1] = (byte) right;

            return payload;
        }
    }
}
