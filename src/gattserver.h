#pragma once

#include "types.h"
#include "uuid.h"

#include <vector>
#include <cstdint>
#include <mutex>
#include <map>
#include <variant>
#include <memory>

using AttHandle = uint16_t;

class AttributeData
{
public:
	// AttHandle handle; // TODO: Since we store everything in the ordered map - should we store handle of the attribute inside class?
	UUID type;

	bool read;
	bool write;
	// bool writeNoResponse;
	bool notify;
	bool indicate;
	bool authReads;
	bool authWrites;
};

class Attribute : public AttributeData
{
protected:
	enum class GattType
	{
		SERVICE,
		CHARACTERISTIC,
		UNKNOWN,
	};
	GattType gattType;

public:
	Attribute(GattType gattType)
		: gattType(gattType) {}

	bool isService() const { return gattType == GattType::SERVICE; }
	bool isCharstic() const { return gattType == GattType::CHARACTERISTIC; }
};

class GATTCharacteristic : public Attribute
{
public:
	AttHandle parentHandle; // TODO: Should we need store child handles in service too?
	bool isDescriptor;
	DataBuffer value;

	GATTCharacteristic()
		: Attribute(GattType::CHARACTERISTIC) {}
};

class GATTService : public Attribute
{
public:
	bool primary;

	GATTService()
		: Attribute(GattType::SERVICE) {}
};

using AttHandlePair = std::pair<AttHandle, std::unique_ptr<Attribute>&>;

class GATTServer
{
	// Each part of gatt server (characteristic or service) is an Attribute
	std::mutex attLock;
	std::map<AttHandle, std::unique_ptr<Attribute>> attributes;

	void handleCheckBeforeCreation(AttHandle handle);

	AttHandlePair findFirstAttrWithinRange(uint16_t startHandle, uint16_t endHandle);

public:
	GATTServer() = default;

	void createTestServer(); // TODO: Remove and replace with proper set/get functions

	// Attribute struct should be fully configured
	// TODO: Currently only primary services are supportedm which are in the top level of profile
	void createService(AttHandle handle, const AttributeData& cfg, bool isPrimary = true);
	void createCharacteristic(AttHandle handle, AttHandle svcHandle, const AttributeData& cfg, const DataBuffer& value = {}, bool isDescriptor = false);

	DataBuffer readPrimaryServices(uint16_t startHandle, uint16_t endHandle);
};
