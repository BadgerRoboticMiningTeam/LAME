#ifndef _PACKET_H
#define _PACKET_H

#ifdef _cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PKT_HEADER							0xAB // TODO: fix me
#define PKT_END								0x7F // TODO: fix me
#define PKT_MIN_SIZE						4
#define PKT_HDR_INDEX						0
#define PKT_OP_INDEX						1
#define PKT_SIZE_INDEX						2
#define PKT_PAYLOAD_START_INDEX				3

// opcodes //
#define DRIVE_OPCODE						0x90
#define DIRECT_SWITCH_OPCODE				0x99
#define AI_SWITCH_OPCODE					0x01
#define REMOTE_SWITCH_OPCODE				0x02
#define ESTOP_OPCODE						0x03
#define CLEAR_ESTOP_OPCODE					0x04
#define QUERY_DRIVE_MODE_OPCODE				0x10
#define QUERY_LOCATION_OPCODE				0x12
#define QUERY_CAMERA_IMAGE_OPCODE			0x13
#define QUERY_HEARTBEAT_OPCODE				0x14
#define REPORT_DRIVE_MODE_OPCODE			0x40
#define REPORT_LOCATION_OPCODE				0x42
#define REPORT_HEARTBEAT_OPCODE				0x44
#define SWITCH_MODE_OPCODE                  0x60


// payload structs //
struct DrivePayload
{
	int8_t left;
	int8_t right;
};


// functions //

/**
 * Does simplistic verification of the packet. Returns 1 on success, 0 otherwise.
 * Also (optionally) gives back the opcode and pointer to payload. The pointer to payload
 * is set to NULL if there is no payload.
 */
static int ReadPacketHeader(uint8_t *buffer, uint8_t length, uint8_t *opcode, uint8_t **payload_ptr)
{
	if (!buffer || length < PKT_MIN_SIZE)
		return 0;

	if (buffer[PKT_HDR_INDEX] != PKT_HEADER)
		return 0;

	if (buffer[length - 1] != PKT_END)
		return 0;

	if (opcode)
		*opcode = buffer[PKT_OP_INDEX];

	if (payload_ptr)
		*payload_ptr = (length == PKT_MIN_SIZE) ? NULL : buffer + PKT_PAYLOAD_START_INDEX;
	return 1;
}

// serialize functions //
static int CreateNoPayloadPacket(uint8_t *buffer, uint8_t length, uint8_t opcode)
{
	if (length < PKT_MIN_SIZE)
		return 0;
	buffer[PKT_HDR_INDEX] = PKT_HEADER;
	buffer[PKT_OP_INDEX] = opcode;
	buffer[PKT_SIZE_INDEX] = PKT_MIN_SIZE;
	buffer[PKT_PAYLOAD_START_INDEX] = PKT_END;
	return PKT_MIN_SIZE;
}

static int CreateQueryHeartbeatPacket(uint8_t *buffer, uint8_t length)
{
	return CreateNoPayloadPacket(buffer, length, QUERY_HEARTBEAT_OPCODE);
}

static int CreateReportHeartbeatPacket(uint8_t *buffer, uint8_t length)
{
	return CreateNoPayloadPacket(buffer, length, REPORT_HEARTBEAT_OPCODE);
}

static int CreateSwitchModeAckPacket(uint8_t *buffer, uint8_t length)
{
    return CreateNoPayloadPacket(buffer, length, SWITCH_MODE_OPCODE);
}

static int CreateDrivePacket(uint8_t *buffer, uint8_t length, struct DrivePayload payload)
{
	if (length < PKT_MIN_SIZE + 2)
		return 0;
	buffer[PKT_HDR_INDEX] = PKT_HDR_INDEX;
	buffer[PKT_OP_INDEX] = PKT_OP_INDEX;
	buffer[PKT_SIZE_INDEX] = PKT_MIN_SIZE;
	buffer[PKT_PAYLOAD_START_INDEX] = payload.left;
	buffer[PKT_PAYLOAD_START_INDEX + 1] = payload.right;
	buffer[PKT_PAYLOAD_START_INDEX + 2] = PKT_END;
	return PKT_MIN_SIZE + 2;
}

// deserialize functions - only for packets that have payloads //
// yes, there is an assumption that you won't feed a bad buffer //

static void ParseDrivePayload(uint8_t *payload, DrivePayload *drive)
{
	if (!payload || !drive)
		return;

	drive->left = payload[0];
	drive->right = payload[1];
}


#ifdef _cplusplus
}
#endif
#endif
