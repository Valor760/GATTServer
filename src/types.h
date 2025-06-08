#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#define ATT_ERROR_RSP 0x01
#define ATT_READ_BY_TYPE_REQ 0x08
#define ATT_READ_BY_TYPE_RSP 0x09
#define ATT_READ_BY_GROUP_TYPE_REQ 0x10

enum class AttErrorCodes : uint8_t
{
	InvalidHandle                = 0x01, // The attribute handle given was not valid on this server.
	ReadNotPermitted             = 0x02, // The attribute cannot be read.
	WriteNotPermitted            = 0x03, // The attribute cannot be written.
	InvalidPDU                   = 0x04, // The attribute PDU was invalid.
	InsufficientAuthentication   = 0x05, // The attribute requires authentication before it can be read or written.
	RequestNotSupported          = 0x06, // ATT Server does not support the request received from the client.
	InvalidOffset                = 0x07, // Offset specified was past the end of the attribute.
	InsufficientAuthorization    = 0x08, // The attribute requires authorization before it can be read or written.
	PrepareQueueFull             = 0x09, // Too many prepare writes have been queued.
	AttributeNotFound            = 0x0A, // No attribute found within the given attribute handle range.
	AttributeNotLong             = 0x0B, // The attribute cannot be read using the ATT_READ_BLOB_REQ PDU.
	EncryptionKeyTooShort        = 0x0C, // The Encryption Key Size used for encrypting this link is too short.
	InvalidAttributeValueLength  = 0x0D, // The attribute value length is invalid for the operation.
	UnlikelyError                = 0x0E, // The attribute request that was requested has encountered an error that was unlikely, and therefore could not be completed as requested. (Internal error)
	InsufficientEncryption       = 0x0F, // The attribute requires encryption before it can be read or written.
	UnsupportedGroupType         = 0x10, // The attribute type is not a supported grouping attribute as defined by a higher layer specification.
	InsufficientResources        = 0x11, // Insufficient Resources to complete the request.
	DatabaseOutOfSync            = 0x12, // The server requests the client to rediscover the database.
	ValueNotAllowed              = 0x13, // The attribute parameter value was not allowed.
};

class AttError
{
public:
	AttErrorCodes error;
	uint8_t opcode;
	uint16_t handle;

	AttError(AttErrorCodes error, uint8_t opcode, uint16_t handle = 0x0000)
		: error(error), opcode(opcode), handle(handle)
	{}
};

using HandleError = std::pair<AttErrorCodes, uint16_t>;
// TODO: How to make it pretty?
#define InternalError HandleError(AttErrorCodes::UnlikelyError, 0x0000)

using DataBuffer = std::vector<uint8_t>;
