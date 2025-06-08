#pragma once

#include "types.h"

class UUID
{
	enum class UUIDType
	{
		UUID16,
		UUID32,
		UUID128
	};

	UUIDType type;
	DataBuffer uuid;

public:
	UUID();
	UUID(uint16_t uuid16);
	UUID(DataBuffer uuid128);

	bool operator==(const UUID& other) const;

	bool isUUID16() const;
	bool isUUID128() const;

	uint16_t getUUID16() const;
	DataBuffer getUUID128() const;
};