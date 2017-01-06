using System;


namespace BaseStation.Packet
{
    public enum PacketOpcode : byte
    {
        Drive = 0x90,

        SwitchToAIDrive = 0x01,
        SwitchToRemoteDrive = 0x02,
        EStop = 0x03,
        ClearEStop = 0x04,

        QueryDriveMode = 0x10,
        QueryBinWeight = 0x11,
        QueryLocation = 0x12,
        QueryCameraImage = 0x13,
        QueryHeartbeat = 0x14,

        ReportDriveMode = 0x40,
        ReportBinWeight = 0x41,
        ReportLocation = 0x42,
        ReportHeartbeat = 0x44
    }

    public enum PacketParseError
    {
        OK, BadHeader, BadOpcode, BadEndByte,
        BadCRC, BadPayload, BadBuffer
    }

    public abstract class Packet
    {
        const ushort LAME_HEADER = 0xBEEF;
        const byte LAME_END = 0x7F;
        const byte PACKET_HEADER_SIZE = sizeof(ushort) + 3 * sizeof(byte);   // header, opcode, payload size, payload crc
        const byte PACKET_METADATA_SIZE = PACKET_HEADER_SIZE + sizeof(byte); // header size + end byte

        public static bool GetOpcodeFromBuffer(byte[] buffer, ref PacketOpcode op)
        {
            if (buffer == null || buffer.Length < PACKET_METADATA_SIZE)
                return false;

            ushort buffer_header = (ushort)(buffer[0] << 8 | buffer[1]);
            byte buffer_opcode = buffer[2];
            byte buffer_size = buffer[3];

            if (buffer_header != LAME_HEADER)
                return false;

            if (buffer[PACKET_HEADER_SIZE + buffer_size] != LAME_END)
                return false;

            if (!Enum.IsDefined(typeof(PacketOpcode), buffer_opcode))
                return false;

            op = (PacketOpcode) buffer_opcode;
            return true;
        }

        public abstract PacketOpcode Opcode
        {
            get;
        }

        public byte[] Serialize()
        {
            byte[] payload = SerializePayload();
            byte[] buffer = new byte[PACKET_METADATA_SIZE + payload.Length];

            buffer[0] = LAME_HEADER >> 8;
            buffer[1] = LAME_HEADER & 0xFF;
            buffer[2] = (byte) Opcode;
            buffer[3] = (byte) payload.Length;
            buffer[4] = Util.ComputeCRC(payload);
            Array.Copy(payload, 0, buffer, PACKET_HEADER_SIZE, payload.Length);
            buffer[buffer.Length - 1] = LAME_END;

            return buffer;
        }

        public PacketParseError Deserialize(byte[] buffer)
        {
            if (buffer == null || buffer.Length < PACKET_METADATA_SIZE)
                return PacketParseError.BadBuffer;

            ushort buffer_header = (ushort) (buffer[0] << 8 | buffer[1]);
            byte buffer_opcode = buffer[2];
            byte buffer_size = buffer[3];
            byte buffer_crc = buffer[4];

            byte[] payload = new byte[buffer_size];
            Array.Copy(buffer, PACKET_HEADER_SIZE, payload, 0, buffer_size);

            byte computed_crc = Util.ComputeCRC(payload);

            if (buffer_header != LAME_HEADER)
                return PacketParseError.BadHeader;

            if (buffer[2] != (byte) Opcode)
                return PacketParseError.BadOpcode;

            if (buffer[PACKET_HEADER_SIZE + buffer_size] != LAME_END)
                return PacketParseError.BadEndByte;

            if (computed_crc != buffer_crc)
                return PacketParseError.BadCRC;

            return DeserializePayload(payload);
        }

        protected abstract byte[] SerializePayload();
        protected abstract PacketParseError DeserializePayload(byte[] payload);
    }

    public abstract class NoPayloadPacket : Packet
    {
        protected override PacketParseError DeserializePayload(byte[] payload)
        {
            if (payload == null || payload.Length != 0)
                return PacketParseError.BadPayload;
            return PacketParseError.OK;
        }

        protected override byte[] SerializePayload()
        {
            return new byte[0];
        }
    }
}
