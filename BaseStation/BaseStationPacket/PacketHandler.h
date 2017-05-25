#pragma once

#include <Packet.h>

typedef int(*NoPayloadPacketFunc)(uint8_t *, uint8_t);

namespace BaseStation
{
    public enum class Opcode : int
    {
        DRIVE = DRIVE_OPCODE,
        DIRECT_SWITCH = DIRECT_SWITCH_OPCODE,
        AI_SWITCH = AI_SWITCH_OPCODE,
        REMOTE_SWITCH = REMOTE_SWITCH_OPCODE,
        SWITCH_MODE_ACK = SWITCH_MODE_ACK_OPCODE,
        AI_INIT = AI_INIT_OPCODE,
        QUERY_LOCATION = QUERY_LOCATION_OPCODE,
        REPORT_LOCATION = REPORT_LOCATION_OPCODE,
        SET_TARGET_LOCATION = SET_TARGET_LOCATION_OPCODE,
        QUERY_ENCODER = QUERY_ENCODER_OPCODE,
        REPORT_ENCODER = REPORT_ENCODER_OPCODE,
        ENABLE_ENCODER = ENABLE_ENCODER_OPCODE,
        DISABLE_ENCODER = DISABLE_ENCODER_OPCODE,
        QUERY_CAMERA_IMAGE = QUERY_CAMERA_IMAGE_OPCODE,
        QUERY_HEARTBEAT = QUERY_HEARTBEAT_OPCODE,
        REPORT_HEARTBEAT = REPORT_HEARTBEAT_OPCODE,
        ESTOP = ESTOP_OPCODE,
        CLEAR_ESTOP = CLEAR_ESTOP_OPCODE,
        SET_CAMERA_QUALITY = SET_CAMERA_QUALITY_OPCODE
    };

    public ref struct Drive
    {
        System::Int32 left;
        System::Int32 right;
        System::Int32 actuator;
        System::Int32 scooper;
        System::Int32 vibrator;

        virtual System::Boolean Equals(System::Object^ obj) override
        {
            if (obj->GetType() != this->GetType())
                return false;
            try
            {
                Drive^ other = safe_cast<Drive^>(obj);
                return left == other->left &&
                    right == other->right &&
                    actuator == other->actuator &&
                    scooper == other->scooper &&
                    vibrator == other->vibrator;
            }
            catch (System::InvalidCastException^)
            {
                return false;
            }
        }
    };

    public ref struct Location
    {
        System::Int32 x;
        System::Int32 y;
        System::Int32 heading; // capped between +/- 180
    };

    public ref struct Encoder
    {
        System::Int32 front_left;
        System::Int32 front_right;
        System::Int32 back_left;
        System::Int32 back_right;
        System::Int32 left_actuator;
        System::Int32 right_actuator;
    };

    public ref struct CameraQuality
    {
        System::Int32 quality;
    };

    public ref class PacketHandler
    {
    public:
        PacketHandler();
        array<System::Byte>^ GetQueryHeartbeatPacket();
        array<System::Byte>^ GetReportHeartbeatPacket();
        array<System::Byte>^ GetRemoteSwitchPacket();
        array<System::Byte>^ GetAiInitPacket();
        array<System::Byte>^ GetQueryLocationPacket();
        array<System::Byte>^ GetQueryEncoderPacket();
        array<System::Byte>^ GetQueryCameraImagePacket();
        array<System::Byte>^ GetQueryCamera1ImagePacket();
        array<System::Byte>^ GetDrivePacket(Drive^ drive);
        array<System::Byte>^ GetEnableEncoderPacket();
        array<System::Byte>^ GetDisableEncoderPacket();
        array<System::Byte>^ GetSetCameraQualityPacket(CameraQuality^ cq);

        Opcode GetPacketOpcode(array<System::Byte>^ buffer, System::Int32 length);
        System::Boolean ParseReportHeartbeatPacket(array<System::Byte>^ buffer, System::Int32 length);
        System::Boolean ParseReportLocationPacket(array<System::Byte>^ buffer, System::Int32 length, Location^% loc);
        System::Boolean ParseReportEncoderPacket(array<System::Byte>^ buffer, System::Int32 length, Encoder^% enc);

    private:
        array<System::Byte>^ GetNoPayloadPacket(NoPayloadPacketFunc func);
        System::Boolean ParseNoPayloadPacket(array<System::Byte>^ buffer, int length, int opcode);
    };
}


