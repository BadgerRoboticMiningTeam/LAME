#include "PacketHandler.h"
#include <cstring>

using namespace BaseStation;


PacketHandler::PacketHandler()
{
}

array<System::Byte>^ BaseStation::PacketHandler::GetQueryHeartbeatPacket()
{
    return this->GetNoPayloadPacket(&CreateQueryHeartbeatPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetReportHeartbeatPacket()
{
    return this->GetNoPayloadPacket(&CreateReportHeartbeatPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetRemoteSwitchPacket()
{
    return this->GetNoPayloadPacket(&CreateRemoteSwitchPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetAiInitPacket()
{
    return this->GetNoPayloadPacket(&CreateAiInitPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetQueryLocationPacket()
{
    return this->GetNoPayloadPacket(&CreateQueryLocationPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetQueryEncoderPacket()
{
    return this->GetNoPayloadPacket(&CreateQueryEncoderPacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetQueryCameraImagePacket()
{
    return this->GetNoPayloadPacket(&CreateQueryCameraImagePacket);
}

array<System::Byte>^ BaseStation::PacketHandler::GetDrivePacket(Drive^ drive)
{
    uint8_t buffer[128];
    DrivePayload payload;

    memset(buffer, 0, 128);
    memset(&payload, 0, sizeof(DrivePayload));

    payload.left = drive->left;
    payload.right = drive->right;
    payload.actuator = drive->actuator;
    payload.scooper = drive->scooper;
    int bytes_written = CreateDrivePacket(buffer, 128, payload);

    array<System::Byte>^ managed_buffer = gcnew array<System::Byte>(bytes_written);
    for (int i = 0; i < bytes_written; i++)
        managed_buffer[i] = buffer[i];
    return managed_buffer;
}

Opcode BaseStation::PacketHandler::GetPacketOpcode(array<System::Byte>^ buffer, System::Int32 length)
{
    pin_ptr<uint8_t> pinned = &buffer[0];
    uint8_t* src = pinned;

    uint8_t opcode = 0;
    ReadPacketHeader(src, length, &opcode, nullptr);
    return (Opcode)opcode;
}

System::Boolean BaseStation::PacketHandler::ParseReportHeartbeatPacket(array<System::Byte>^ buffer, System::Int32 length)
{
    return this->ParseNoPayloadPacket(buffer, length, REPORT_HEARTBEAT_OPCODE);
}

System::Boolean BaseStation::PacketHandler::ParseReportLocationPacket(array<System::Byte>^ buffer, System::Int32 length, Location ^% loc)
{
    pin_ptr<uint8_t> pinned = &buffer[0];
    uint8_t *src = pinned;

    uint8_t parsed_opcode = 0;
    uint8_t *payload = nullptr;

    bool parse_success = ReadPacketHeader(src, (uint8_t)length, &parsed_opcode, &payload) == 1;
    if (!parse_success)
        return false;
    if (!payload)
        return false;
    if (parsed_opcode != REPORT_LOCATION_OPCODE)
        return false;

    LocationPayload loc_payload;
    memset(&loc_payload, 0, sizeof(LocationPayload));
    ParseLocationPayload(payload, &loc_payload);
    
    loc = gcnew Location;
    loc->x = loc_payload.x;
    loc->y = loc_payload.y;
    loc->heading = loc_payload.heading;
    return true;
}

System::Boolean BaseStation::PacketHandler::ParseReportEncoderPacket(array<System::Byte>^ buffer, System::Int32 length, Encoder ^% enc)
{
    pin_ptr<uint8_t> pinned = &buffer[0];
    uint8_t *src = pinned;

    uint8_t parsed_opcode = 0;
    uint8_t *payload = nullptr;

    bool parse_success = ReadPacketHeader(src, (uint8_t)length, &parsed_opcode, &payload) == 1;
    if (!parse_success)
        return false;
    if (!payload)
        return false;
    if (parsed_opcode != REPORT_ENCODER_OPCODE)
        return false;

    EncoderPayload enc_payload;
    memset(&enc_payload, 0, sizeof(EncoderPayload));
    ParseEncoderPayload(payload, &enc_payload);

    enc = gcnew Encoder;
    enc->back_left = enc_payload.back_left;
    enc->back_right = enc_payload.back_right;
    enc->front_left = enc_payload.front_left;
    enc->front_right = enc_payload.front_right;
    enc->left_actuator = enc_payload.left_actuator;
    enc->right_actuator = enc_payload.right_actuator;
    return true;
}

array<System::Byte>^ BaseStation::PacketHandler::GetNoPayloadPacket(NoPayloadPacketFunc func)
{
    uint8_t buffer[128];
    memset(buffer, 0, 128);

    int bytes_written = func(buffer, 128);
    array<System::Byte>^ managed_buffer = gcnew array<System::Byte>(bytes_written);
    for (int i = 0; i < bytes_written; i++)
        managed_buffer[i] = buffer[i];
    return managed_buffer;
}

System::Boolean BaseStation::PacketHandler::ParseNoPayloadPacket(array<System::Byte>^ buffer, int length, int opcode)
{
    pin_ptr<uint8_t> pinned = &buffer[0];
    uint8_t* src = pinned;

    uint8_t parsed_opcode = 0;
    bool parse_success = ReadPacketHeader(src, (uint8_t) length, &parsed_opcode, nullptr) == 1;
    if (!parse_success)
        return false;

    return (parsed_opcode == opcode);
}
