#pragma once

#include "types.h"
#include "uuid.h"

#include <vector>
#include <cstdint>
#include <mutex>
#include <map>

class Attribute
{
public:
	uint16_t handle;
	UUID type;

	bool read;
	bool write;
	// bool writeNoResponse;
	bool notify;
	bool indicate;
	bool authReads;
	bool authWrites;
};

class GATTCharacteristic : public Attribute
{
public:
	bool isDescriptor;
	DataBuffer value;
};

class GATTService : public Attribute
{
	std::map<uint16_t, GATTCharacteristic> characteristics;

public:
	bool primary;

	void addCharacteristic(GATTCharacteristic chstic);

};

class GATTServer
{
	std::mutex serviceLock;
	std::map<uint16_t, GATTService> services;

public:
	GATTServer() = default;

	void createTestServer(); // TODO: Remove and replace with proper set/get functions

	// Attribute struct should be fully configured
	// TODO: Currently only primary services are supportedm which are in the top level of profile
	void createService(const Attribute& cfg, bool isPrimary = true);
	void createCharacteristic(uint16_t svcHandle, const Attribute& cfg, const DataBuffer& value = {}, bool isDescriptor = false);
};
