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
		// CHARVALUE,
	};
	GattType gattType;

public:
	Attribute(GattType gattType)
		: gattType(gattType) {}

	bool isService() const { return gattType == GattType::SERVICE; }
	bool isCharstic() const { return gattType == GattType::CHARACTERISTIC; }
	// bool isCharsticValue() const { return gattType == GattType::CHARVALUE; }
};

class GATTCharacteristic : public Attribute
{
public:
	AttHandle parentHandle;
	bool isDescriptor;
	DataBuffer value;

	GATTCharacteristic()
		: Attribute(GattType::CHARACTERISTIC) {}
};

class GATTWriteQueue
{
public:
	AttHandle handle;
	uint16_t offset;
	DataBuffer data;
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
	std::vector<GATTWriteQueue> writeQueue;

	void handleCheckBeforeCreation(AttHandle handle);

	AttHandlePair findFirstAttrWithinRange(uint16_t startHandle, uint16_t endHandle);

public:
	GATTServer() = default;

	void createTestServer(); // TODO: Remove and replace with proper set/get functions

	// Attribute struct should be fully configured
	// TODO: Currently only primary services are supportedm which are in the top level of profile
	void createService(AttHandle handle, const AttributeData& cfg, bool isPrimary = true);
	void createCharacteristic(AttHandle handle, AttHandle svcHandle, const AttributeData& cfg, const DataBuffer& value = {}, bool isDescriptor = false);

	DataBuffer readPrimaryServices(AttHandle startHandle, AttHandle endHandle);
	DataBuffer readCharacteristics(AttHandle startHandle, AttHandle endHandle);

	DataBuffer readCharData(AttHandle handle);
	DataBuffer readCharBlobData(AttHandle handle, uint16_t offset);

	void writeCharData(AttHandle handle, const DataBuffer& data);
	void prepareWriteCharData(AttHandle handle, uint16_t offset, const DataBuffer& data);
	void executeWriteCharData(bool cancel);

	// Returns response either Indicate or Notify, empty for nothing
	DataBuffer localUpdateCharData(AttHandle handle, const DataBuffer& data);
};
