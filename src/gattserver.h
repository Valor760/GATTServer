#pragma once

#include <vector>
#include <cstdint>

class GATTServer;
class GATTService;

class GATTCharacteristic
{
	uint16_t handle;
	uint8_t uuid[16]; // TODO: Use UUID class for this

	bool enabled; // TODO: See the comment 'enabled' in GATTService. The same situation is here
	bool isDescriptor;
	bool read;
	bool write;
	bool writeNoResponse;
	bool notify;
	bool indicate;
	bool authReads;
	bool authWrites;

	std::vector<uint8_t> value;

public:
	GATTCharacteristic() = default;

	friend GATTService;
	friend GATTServer;
};

class GATTService
{
	uint16_t handle;
	uint8_t uuid[16]; // TODO: Use UUID class for this

	bool enabled; // TODO: Even though XML has this field, I don't see it being changed in BSA (like if not enabled becomes enabled). Probably we can omit this and if in XML this is set as false - skip the whole service
	bool primary;

	std::vector<GATTCharacteristic> characteristics;

public:
	GATTService() = default;

	friend GATTServer;
};

class GATTServer
{
	uint8_t uuid[16]; // TODO: Use UUID class for this
	uint16_t handle;
	std::vector<GATTService> services;

public:
	GATTServer() = default;

	void createTestServer(); // TODO: Remove and replace with proper set/get functions
};