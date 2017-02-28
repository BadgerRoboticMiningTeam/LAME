using System;


namespace BaseStation
{
    /**
        SwitchService (1,2,3)
        SwitchServiceOK (1,2,3)
        QueryHeartbeat X
        ReportHeartbeat X
        ReportSpeed (1,2,3,4)
        SetSpeed (1,2,3,4)
        SetBin (1)
        ReportBin (1)
        QueryRobotLocation X
        ReportRobotLocation (2)
     */

     // packet payload structs //
     struct SwitchDriveModePayload
     {
        int DriveMode;
     }

     struct SpeedPayload
     {
        int LeftBack, LeftFront;
        int RightBack, RightFront;
     }

     struct BinPosPayload
     {
        int BinPos;
     }

    static class Packet
    {
        // packet opcodes //
        public const int SWITCH_DRIVE_MODE = 0x10;
        public const int SWITCH_DRIVE_MODE_ACK = 0x11;
        public const int QUERY_HEARTBEAT = 0x12;
        public const int REPORT_HEARTBEAT = 0x13;
        public const int REPORT_SPEEDS = 0x14;
        public const int SET_SPEEDS = 0x15;
        public const int REPORT_BIN_POS = 0x16;
        public const int SET_BIN_POS = 0x17;
        
        // packet constants //
        const int MIN_PKT_SIZE = 5; // metadata, no payload
        const ushort PKT_HEADER = 0xBAAD;
        const byte PKT_END = 0x7F;

        // packet organization //
        // [header(2), op(1), total size(1), data(n), end(1)] //

        public static bool ParseHeader(byte[] buffer, int length, ref int opcode)
        {
            if (length < MIN_PKT_SIZE || buffer.Length < MIN_PKT_SIZE)
                return false;

            ushort buffer_hdr = (ushort) ((buffer[0] << 8) | buffer[1]);
            int buf_opcode = buffer[1];
            int buf_size = buffer[2];
            if (buf_size > buffer.Length)
                return false;

            byte buf_end = buffer[buf_size - 1];
            if (buf_end != PKT_END)
                return false;

            opcode = buf_opcode;
            return true;
        }

        public static byte[] GetQueryHeartbeatPacket()
        {
            return Serialize(QUERY_HEARTBEAT, new byte[0]);
        }

        public static byte[] GetReportHeartbeatPacket()
        {
            return Serialize(REPORT_HEARTBEAT, new byte[0]);
        }

        public static byte[] GetSwitchDriveModePacket(int mode)
        {
            return Serialize(SWITCH_DRIVE_MODE, new byte[] { (byte) mode });
        }

        static byte[] Serialize(int opcode, byte[] payload)
        {
            byte[] buffer = new byte[payload.Length + MIN_PKT_SIZE];
            buffer[0] = PKT_HEADER >> 8;
            buffer[1] = PKT_HEADER & 0xFF;
            buffer[2] = (byte) opcode;
            buffer[3] = (byte) buffer.Length;
            Array.Copy(payload, 0, buffer, 4, payload.Length);
            buffer[buffer.Length - 1] = PKT_END;
            return buffer;
        }
    }
}
