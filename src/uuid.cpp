#include "uuid.h"
#include "utils/log.h"

// ALL uuids are 128bit UUIDs. See https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-54/out/en/host/service-discovery-protocol--sdp--specification.html#UUID-ef710684-4c7e-6793-4350-4a190ea9a7a4
static const DataBuffer BASE_UUID = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

UUID::UUID()
	: type(UUIDType::UUID128),
	  uuid(BASE_UUID)
{
}

UUID::UUID(uint16_t uuid16)
	: type(UUIDType::UUID16),
	  uuid(BASE_UUID)
{
	// 3rd and 4th bytes represent UUID16
	uuid[2] = uuid16 >> 8;
	uuid[3] = uuid16 & 0xFF;
}

UUID::UUID(DataBuffer uuid128)
	: type(UUIDType::UUID128)
{
	if(uuid128.size() != 16)
	{
		LOG_ERROR("Buffer size is not 16 (not a UUID128)");
		throw InternalError;
	}

	uuid = uuid128;
}

bool UUID::operator==(const UUID& other) const
{
	return type == other.type && uuid == other.uuid;
}

bool UUID::isUUID16() const
{
	return type == UUIDType::UUID16;
}

bool UUID::isUUID128() const
{
	return type == UUIDType::UUID128;
}

uint16_t UUID::getUUID16() const
{
	if(!isUUID16())
	{
		LOG_ERROR("UUID16 requested, but UUID is of %d type", static_cast<int>(type));
		throw InternalError;
	}

	uint16_t ret = (uuid[2] << 8) + uuid[3];

	return ret;
}

DataBuffer UUID::getUUID128() const
{
	if(!isUUID128())
	{
		LOG_ERROR("UUID128 requested, but UUID is of %d type", static_cast<int>(type));
		throw InternalError;
	}

	return uuid;
}

namespace uuids
{
	UUID DatabaseHash(0x2B2A);
	UUID ServiceChanged(0x2A05);

	UUID PrimaryService(0x2800);
	UUID SecondaryService(0x2801);
	UUID Include(0x2802);
	UUID Characteristic(0x2803);
} /* namespace uuids */