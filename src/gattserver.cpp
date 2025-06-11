#include "gattserver.h"
#include "utils/log.h"
#include "att_utils.h"

#include <cstring>
#include <algorithm>

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

static uint8_t calculateCharProperties(const AttributeData& data)
{
	uint8_t ret = 0;

	if(data.read)
	{
		ret |= CHARPROP_Read;
	}

	// if(data.writeNoResponse)
	// {
	// 	ret |= CHARPROP_WriteWithoutResponse;
	// }

	if(data.write)
	{
		ret |= CHARPROP_Write;
	}

	if(data.notify)
	{
		ret |= CHARPROP_Notify;
	}

	if(data.indicate)
	{
		ret |= CHARPROP_Indicate;
	}

	if(data.authWrites)
	{
		ret |= CHARPROP_AuthenticatedSignedWrites;
	}

	LOG_DEBUG("Calculated characteristic properties: 0x%02X", ret);
	return ret;
}

void GATTServer::createTestServer()
{
	// *******************************
	// *         SERVICES            *
	// *******************************
	{
		AttributeData svc1 = {
			// .handle = 0x0010,
			.type = 0xFFFF,
			.read = true,
			.write = false,
			.notify = false,
			.indicate = false,
		};
		createService(0x0010, svc1);
	}

	{
		AttributeData svc2 = {
			// .handle = 0x0020,
			.type = 0xFFFF,
			.read = true,
			.write = false,
			.notify = false,
			.indicate = false,
		};
		createService(0x0020, svc2);
	}

	// *******************************
	// *       CHARACTERISTICS       *
	// *******************************

	{
		AttributeData char1 = {
			.type = 0xFFFF,
			.read = true,
			.write = false,
		};
		createCharacteristic(0x0011, 0x0010, char1, {'H', 'e', 'l', 'l', 'o', '!'});
	}

	{
		AttributeData char2 = {
			.type = 0xFFFF,
			.read = false,
			.write = true,
		};
		createCharacteristic(0x0012, 0x0010, char2);
	}

	{
		AttributeData char3 = {
			.type = 0xFFFF,
			.read = true,
			// .write = true,
			.write = false,
		};
		createCharacteristic(0x0021, 0x0020, char3);
	}

	{
		// TODO: Uncomment when fixed notify and stuff
		// AttributeData char4 = {
		// 	// .handle = 0x0028,
		// 	.type = 0xFFFF,
		// 	.read = true,
		// 	.write = true,
		// 	.notify = true,
		// };
		// createCharacteristic(0x0028, 0x0020, char4);
	}
}

void GATTServer::handleCheckBeforeCreation(AttHandle handle)
{
	std::lock_guard lg(attLock);

	if(attributes.contains(handle))
	{
		LOG_ERROR("Attribute with %04X handle already exists!", handle);
		throw -1;
	}

	if(handle == 0x0000)
	{
		LOG_ERROR("Invalid handle %04X", handle);
		throw -1;
	}
}

void GATTServer::createService(AttHandle handle, const AttributeData& cfg, bool isPrimary)
{
	handleCheckBeforeCreation(handle);

	std::lock_guard lg(attLock);

	std::unique_ptr<GATTService> svc = std::make_unique<GATTService>();
	if(!svc)
	{
		LOG_ERROR("Failed to allocate service memory");
		throw -1;
	}

	// svc.handle = cfg.handle;
	svc->type = cfg.type;
	svc->read = cfg.read;
	svc->write = cfg.write;
	svc->notify = cfg.notify;
	svc->indicate = cfg.indicate;
	svc->authReads = cfg.authReads;
	svc->authWrites = cfg.authWrites;
	svc->primary = isPrimary;

	LOG_DEBUG("Adding Service with %04X handle", handle);

	attributes[handle] = std::move(svc);
}

void GATTServer::createCharacteristic(AttHandle handle, AttHandle svcHandle, const AttributeData& cfg, const DataBuffer& value, bool isDescriptor)
{
	handleCheckBeforeCreation(handle);

	std::lock_guard lg(attLock);

	if(!attributes.contains(svcHandle))
	{
		LOG_ERROR("No parent Service with %04X handle is found!", svcHandle);
		throw -1;
	}

	std::unique_ptr<GATTCharacteristic> charstic = std::make_unique<GATTCharacteristic>();
	if(!charstic)
	{
		LOG_ERROR("Failed to allocate characteristic memory");
		throw -1;
	}

	charstic->parentHandle = svcHandle;
	charstic->type = cfg.type;
	charstic->read = cfg.read;
	charstic->write = cfg.write;
	charstic->notify = cfg.notify;
	charstic->indicate = cfg.indicate;
	charstic->authReads = cfg.authReads;
	charstic->authWrites = cfg.authWrites;
	charstic->isDescriptor = isDescriptor;
	charstic->value = value;

	LOG_DEBUG("Adding Characteristic with %04X handle (parent service: %04X)", handle, svcHandle);

	attributes[handle] = std::move(charstic);
}

DataBuffer GATTServer::readPrimaryServices(AttHandle startHandle, AttHandle endHandle)
{
	std::lock_guard lg(attLock);
	auto attPair = findFirstAttrWithinRange(startHandle, endHandle);

	AttHandle handle = attPair.first;
	std::unique_ptr<Attribute>& attr = attPair.second;
	LOG_DEBUG("Attribute found (hdl: %04X)", handle);

	DataBuffer buf;
	if(attr->isService())
	{
		GATTService& svc = static_cast<GATTService&>(*attr);
		AttHandle parentHandle = handle;
		AttHandle nextHandle = handle + 1;

		while(true)
		{
			try
			{
				auto nextPair = findFirstAttrWithinRange(nextHandle, endHandle);
				if(nextPair.second->isCharstic())
				{
					GATTCharacteristic& chstic = static_cast<GATTCharacteristic&>(*nextPair.second);
					if(chstic.parentHandle != parentHandle)
					{
						// Parent handle is not the same as the service we are currently looking at
						// TODO: Check if this is even valid from GATT protocol!
						break;
					}

					nextHandle = nextPair.first + 1;
				}
				else
				{
					// We are interested only in characteristics
					break;
				}
			}
			catch(...)
			{
				break;
			}
		}

		// We are going to return only 1 service per request
		// TODO: Do return several in the future
		appendMsgData(buf, (uint8_t) (4 + 16));
		appendMsgData(buf, parentHandle);
		appendMsgData(buf, (AttHandle)(nextHandle - 1));
		appendMsgData(buf, svc.type.getUUID128());
	}
	else
	{
		// After testing with Android it seems that it is not supported for characteristic to be "out of order"
		// What it means - assume the following structure of attributes:
		// Service(0010)
		// -- Char (0011, parent 0010)
		// Service(0020)
		// -- Char(0021, parent 0020)
		// -- Char(0030, parent 0010)
		// The characteristics parent should always correspond to the service right on top of them!
		LOG_ERROR("Found attribute that is not a Service!");
		throw InternalError;
	}

	return buf;
}

DataBuffer GATTServer::readCharacteristics(AttHandle startHandle, AttHandle endHandle)
{
	DataBuffer buf;
	AttHandle dupStartHandle = startHandle;
	while(dupStartHandle <= endHandle && dupStartHandle != 0x0000)
	{
		AttHandlePair attPair = findFirstAttrWithinRange(dupStartHandle, endHandle);
		if(!attPair.second->isCharstic())
		{
			dupStartHandle = attPair.first + 1;
			continue;
		}

		LOG_DEBUG("Found characteristic with %04X handle", attPair.first);
		appendMsgData(buf, (uint8_t) (sizeof(AttHandle) + sizeof(uint8_t) + sizeof(AttHandle) + 2));
		appendMsgData(buf, attPair.first);
		appendMsgData(buf, calculateCharProperties(*attPair.second));
		appendMsgData(buf, attPair.first); // TODO: Is correct if char handle is the same as value handle?
		appendMsgData(buf, attPair.second->type.getUUID16());
		break; // TODO: For now return only 1 characteristic per request
	}

	if(buf.empty())
	{
		throw HandleError(AttErrorCodes::AttributeNotFound, startHandle);
	}

	return buf;
}

AttHandlePair GATTServer::findFirstAttrWithinRange(AttHandle startHandle, AttHandle endHandle)
{
	// std::lock_guard lg(serviceLock);
	for(auto& [hdl, svc] : attributes)
	{
		if(hdl >= startHandle && hdl <= endHandle)
		{
			return {hdl, svc};
		}
	}

	LOG_DEBUG("Service within %04X and %04X handle range not found", startHandle, endHandle);
	throw HandleError(AttErrorCodes::AttributeNotFound, startHandle);
}
