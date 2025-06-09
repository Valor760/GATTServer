#include "gattserver.h"
#include "utils/log.h"
#include "att_utils.h"

#include <cstring>
#include <algorithm>

// FIXME: Mutex can't be in GATT Service otherwise no operator= available. Need to think
static std::mutex charLock;

/*
<GATT_SERVER handle="0000" uuid="579cabf3-bfe1-4c27-84c1-4955b6d5a102">
      <SERVICE handle="0001" uuid="180F" primary="true" enable="true">
         <CHARACTERISTIC handle="0011" uuid="2901" is_descriptor="true" value="76657269666F6E652062617474657279206C6576656C" enable="true" />
         <CHARACTERISTIC handle="0012" uuid="2A19" value="20" read="true" notify="true" authentication_signed_writes="true" enable="true" />
         <CHARACTERISTIC handle="0013" uuid="2901" is_descriptor="true" value="76657269666F6E65206261747465727920737461747573" enable="true" />
         <CHARACTERISTIC handle="0014" uuid="2A1A" value="64" broadcast="true" read="true" authentication_signed_writes="true" enable="true" />
      </SERVICE>
      <SERVICE handle="0002" uuid="579cabf4-bfe1-4c27-84c1-4955b6d5a102" primary="true" enable="true">
         <CHARACTERISTIC handle="0021" uuid="2901" is_descriptor="true" value="707269766174655F64617461" enable="true" />
         <CHARACTERISTIC handle="0022" uuid="579cabf5-bfe1-4c27-84c1-4955b6d5a102" value="7465737420737472696E67" authentication_signed_reads="true" authentication_signed_writes="true" enable="true" />
         <CHARACTERISTIC handle="0023" uuid="2901" is_descriptor="true" value="7075626C69635F646174615F6E6F74696679" enable="true" />
         <CHARACTERISTIC handle="0024" uuid="579cabf7-bfe1-4c27-84c1-4955b6d5a102" value="20" read="true" notify="true" write="true" enable="true" />
         <CHARACTERISTIC handle="0025" uuid="2901" is_descriptor="true" value="7075626C69635F646174615F696E646963617465" enable="true" />
         <CHARACTERISTIC handle="0026" uuid="579cabf8-bfe1-4c27-84c1-4955b6d5a102" value="02" read="true" indicate="true" write="true" enable="true" />
      </SERVICE>
   </GATT_SERVER>
*/

void GATTServer::createTestServer()
{
	// *******************************
	// *         SERVICES            *
	// *******************************
	{
		Attribute svc1 = {
			.handle = 0x0010,
			.type = 0xFFFF,
			.read = true,
			.write = false,
			.notify = false,
			.indicate = false,
		};
		createService(svc1);
	}

	{
		Attribute svc2 = {
			.handle = 0x0020,
			.type = 0xFFFF,
			.read = true,
			.write = false,
			.notify = false,
			.indicate = false,
		};
		createService(svc2);
	}

	// *******************************
	// *       CHARACTERISTICS       *
	// *******************************

	{
		Attribute char1 = {
			.handle = 0x0011,
			.type = 0xFFFF,
			.read = true,
			.write = false,
		};
		createCharacteristic(0x0010, char1, {'H', 'e', 'l', 'l', 'o', '!'});
	}

	{
		Attribute char2 = {
			.handle = 0x0012,
			.type = 0xFFFF,
			.read = false,
			.write = true,
		};
		createCharacteristic(0x0010, char2);
	}

	{
		Attribute char3 = {
			.handle = 0x0021,
			.type = 0xFFFF,
			.read = true,
			.write = true,
		};
		createCharacteristic(0x0020, char3);
	}

	{
		Attribute char4 = {
			.handle = 0x0028,
			.type = 0xFFFF,
			.read = true,
			.write = true,
			.notify = true,
		};
		createCharacteristic(0x0020, char4);
	}


	// handle = 0x0001;
	// {
	// 	uint8_t tmp[] = {0x57, 0x9c, 0xab, 0xf3, 0xbf, 0xe1, 0x4c, 0x27, 0x84, 0xc1, 0x49, 0x55, 0xb6, 0xd5, 0xa1, 0x02};
	// 	memcpy(uuid, tmp, sizeof(uuid));
	// }

	// GATTService svc;
	// svc.handle = 0x0002;
	// {
	// 	// TODO: By protocol it is allowed to set UUID as uint16, but maybe for ease we can deprecate it and allow only uuid128?
	// 	// OR at least convert it to uuid128
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0F};
	// 	memcpy(svc.uuid, tmp, sizeof(svc.uuid));
	// }
	// svc.primary = true;

	// {
	// 	GATTCharacteristic charstic;
	// 	charstic.handle = 0x0011;
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x20};
	// 	memcpy(charstic.uuid, tmp, sizeof(charstic.uuid));
	// 	charstic.value = {'H', 'e', 'l', 'l', 'o'};
	// 	charstic.read = true;
	// 	charstic.notify = true;
	// 	svc.characteristics.push_back(charstic);
	// }

	// {
	// 	GATTCharacteristic charstic;
	// 	charstic.handle = 0x0012;
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x21};
	// 	memcpy(charstic.uuid, tmp, sizeof(charstic.uuid));
	// 	charstic.value = {'W', 'o', 'r', 'l', 'd'};
	// 	charstic.write = true;
	// 	charstic.read = true;
	// 	charstic.indicate = true;
	// 	svc.characteristics.push_back(charstic);
	// }

	// services.push_back(svc);
}

void GATTServer::createService(const Attribute& cfg, bool isPrimary)
{
	if(cfg.handle == 0x0000)
	{
		LOG_ERROR("Invalid service handle %04X", cfg.handle);
		throw -1;
	}

	{
		auto it = services.find(cfg.handle);
		if(it != services.end())
		{
			LOG_ERROR("Service with %04X handle already exists!", cfg.handle);
			throw -1;
		}
	}
	// TODO: Check no other attribute (service and characteristics) doesn't exist with the same handle
	// Need to check characteristics too

	GATTService svc;
	svc.handle = cfg.handle;
	svc.type = cfg.type;
	svc.read = cfg.read;
	svc.write = cfg.write;
	svc.notify = cfg.notify;
	svc.indicate = cfg.indicate;
	svc.authReads = cfg.authReads;
	svc.authWrites = cfg.authWrites;
	svc.primary = isPrimary;

	std::lock_guard lg(serviceLock);
	services[cfg.handle] = svc;
}

void GATTServer::createCharacteristic(uint16_t svcHandle, const Attribute& cfg, const DataBuffer& value, bool isDescriptor)
{
	if(cfg.handle == 0x0000)
	{
		LOG_ERROR("Invalid characteristic handle %04X", cfg.handle);
		throw -1;
	}

	std::lock_guard lg(serviceLock);
	auto it = services.find(svcHandle);
	if(it == services.end())
	{
		LOG_ERROR("Couldn't find service with %04X handle", svcHandle);
		throw -1;
	}

	GATTCharacteristic charstic;
	charstic.handle = cfg.handle;
	charstic.type = cfg.type;
	charstic.read = cfg.read;
	charstic.write = cfg.write;
	charstic.notify = cfg.notify;
	charstic.indicate = cfg.indicate;
	charstic.authReads = cfg.authReads;
	charstic.authWrites = cfg.authWrites;
	charstic.isDescriptor = isDescriptor;
	charstic.value = value;

	it->second.addCharacteristic(charstic);
}

DataBuffer GATTServer::readPrimaryServices(uint16_t startHandle, uint16_t endHandle)
{
	std::lock_guard lg(serviceLock);
	auto& svc = findServiceWithinRange(startHandle, endHandle);

	LOG_DEBUG("Service found (hdl: %04X)", svc.handle);

	// TODO: Let's return services 1 by 1. Not very effective, but easy! Otherwise need to calculate how much
	// space each service takes
	// NOTE: For services it is easy, since they don't have data field, but pain for characteristics!
	DataBuffer buf;
	appendMsgData(buf, (uint8_t) (4 + 16));
	appendMsgData(buf, svc.handle);
	appendMsgData(buf, svc.handle);
	appendMsgData(buf, svc.type.getUUID128());

	return buf;
}

GATTService& GATTServer::findServiceWithinRange(uint16_t startHandle, uint16_t endHandle)
{
	// std::lock_guard lg(serviceLock);
	for(auto& [hdl, svc] : services)
	{
		if(hdl >= startHandle && hdl <= endHandle)
		{
			return svc;
		}
	}

	LOG_DEBUG("Service within %04X and %04X handle range not found", startHandle, endHandle);
	throw HandleError(AttErrorCodes::AttributeNotFound, startHandle);
}

void GATTService::addCharacteristic(GATTCharacteristic chstic)
{
	std::lock_guard lg(charLock);
	characteristics[chstic.handle] = chstic;
}