using System;


namespace BaseStation.Packet
{
    public enum PacketOpcode : byte
    {
        Drive = 0x99,
        SwitchToDirectDrive = 0x90,
        SwitchToRemoteDrive = 0x91,
        SwitchToAIDrive = 0x92,
        SwitchDriveModeAck = 0x93,

        EStop = 0xAA,
        ClearEStop = 0xAB,
        
        AI_Init = 0x02,
        QueryLocation = 0x44,
        ReportLocation = 0x45,
        SetLocation = 0x46,

        QueryEncoder = 0x50,
        ReportEncoder = 0x47,

        QueryHeartbeat = 0x14,
        ReportHeartbeat = 0x44,

        QueryCameraImage = 0x13
    }

    public enum PacketParseError
    {
        OK, BadHeader, BadOpcode, BadEndByte,
        BadCRC, BadPayload, BadBuffer
    }

    public abstract class Packet
    {
        const byte LAME_HEADER = 0xAB;
        const byte LAME_END = 0x7F;
        const byte PACKET_HEADER_SIZE = 3;   // header, opcode, payload size,
        const byte PACKET_METADATA_SIZE = 4; // header size + end byte

        public static bool GetOpcodeFromBuffer(byte[] buffer, ref PacketOpcode op)
        {
            if (buffer == null || buffer.Length < PACKET_METADATA_SIZE)
                return false;

            byte buffer_header = buffer[0];
            byte buffer_opcode = buffer[1];
            byte payload_size = buffer[2];

            if (buffer_header != LAME_HEADER)
                return false;

            if (buffer[PACKET_HEADER_SIZE + payload_size - 1] != LAME_END)
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

            buffer[0] = LAME_HEADER;
            buffer[1] = (byte) Opcode;
            buffer[2] = (byte) payload.Length;
            Array.Copy(payload, 0, buffer, PACKET_HEADER_SIZE, payload.Length);
            buffer[buffer.Length - 1] = LAME_END;
            return buffer;
        }

        public PacketParseError Deserialize(byte[] buffer)
        {
            if (buffer == null || buffer.Length < PACKET_METADATA_SIZE)
                return PacketParseError.BadBuffer;

            byte buffer_header = buffer[0];
            byte buffer_opcode = buffer[1];
            byte payload_size = buffer[2];

            byte[] payload = new byte[payload_size];
            Array.Copy(buffer, PACKET_HEADER_SIZE, payload, 0, payload_size);
            
            if (buffer_header != LAME_HEADER)
                return PacketParseError.BadHeader;

            if (buffer[1] != (byte) Opcode)
                return PacketParseError.BadOpcode;

            if (buffer[PACKET_HEADER_SIZE + payload_size] != LAME_END)
                return PacketParseError.BadEndByte;

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
