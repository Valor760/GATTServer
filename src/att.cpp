#include "att.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "types.h"
#include "att_utils.h"
#include "uuid.h"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include <vector>
#include <array>
#include <thread>
#include <utility>

/*
 * The PSM 31 is used for getting attributes over BR/EDR.
 * After the pairing is done iOS sends connection request with PSM 31,
 * as the NEO device advertises with UUID128_GENERIC_ATTRIBUTE_PROFILE.
 */

static ssize_t readData(int fd, std::vector<uint8_t>& ret)
{
	ret.clear();
	ret.reserve(4096);

	uint8_t data[4096];

	ssize_t r = read(fd, data, sizeof(data));
	if(r <= 0)
	{
		LOG_DEBUG("Failed to read [errno %d: %m]", errno);
	}
	else
	{
		ret.insert(ret.begin(), data, data + r);
	}

	return r;
}

static bool writeData(int fd, const std::vector<uint8_t>& data)
{
	HEXDUMP_DEBUG("Writing response", data.data(), data.size());

	ssize_t r = write(fd, data.data(), data.size());
	if(r <= 0)
	{
		LOG_ERROR("Failed to write response [errno %d: %m]", errno);
		return false;
	}

	return true;
}

ATTBind::ATTBind(uint16_t psm, uint16_t cid, uint8_t addrType)
{
	struct sockaddr_l2 addr = {0, 0, {0}, 0, 0};
	bdaddr_t any            = {0, 0, 0, 0, 0, 0};
	addr.l2_family = AF_BLUETOOTH; /* Addressing family, always AF_BLUETOOTH */
	bacpy(&addr.l2_bdaddr, &any);  /* Bluetooth address of local bluetooth adapter */
	addr.l2_psm = psm;
	addr.l2_cid = cid;
	addr.l2_bdaddr_type = addrType; // TODO: Use enum/define

	if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		LOG_ERROR("Failed to bind ATT (psm: %hu cid: %hu) [errno %d: %m]", psm, cid, errno);
		throw -1;
	}

	listen(sock, 5);
}

int ATTBind::getFD() const
{
	return sock;
}

ATTServer::ATTServer()
	: bredrHandle(htobs(ATT_PSM), htobs(0), 0),
	  bleHandle(htobs(0), htobs(ATT_CID), 1)
{
	LOG_DEBUG("Start Bluetooth ATT server...");

	gatt.createTestServer(); // TODO: Remove
}

void ATTServer::finish()
{
}

void ATTServer::run()
{
	while(true)
	{
		std::vector<pollfd> pfds = {{bleHandle.getFD(), POLLIN, 0}, {bredrHandle.getFD(), POLLIN, 0}};

		LOG_DEBUG("Wait for ATT connect");

		if(poll(pfds.data(), pfds.size(), -1) <= 0 || (!(pfds[0].revents & POLLIN) && !(pfds[1].revents & POLLIN)))
		{
			LOG_DEBUG("L2CAP signalled to close");
			return;
		}

		if(pfds[0].revents & POLLIN)
		{
			LOG_DEBUG("Accepting BLE ATT connection");
			acceptAttConnection(bleHandle.getFD());
		}

		if(pfds[1].revents & POLLIN)
		{
			LOG_DEBUG("Accepting BR/EDR ATT connection");
			acceptAttConnection(bredrHandle.getFD());
		}

		// TODO: Interrupt pipe
	}
}

void ATTServer::acceptAttConnection(int serverFD)
{
	struct sockaddr_l2 addr = {};
	unsigned int opt        = sizeof(addr);

	int clientFD = accept(serverFD, (struct sockaddr *) &addr, &opt);
	if(clientFD >= 0)
	{
		std::thread th(&ATTServer::handleAttConnection, this, clientFD, addr);
		th.detach();
	}
	else
	{
		LOG_ERROR("Failed to accept client [errno %d: %m]", errno);
	}
}

void ATTServer::handleAttConnection(int clientFD, struct sockaddr_l2 l2addr)
{
	const bool isBLE = l2addr.l2_bdaddr_type != 0; // TODO: Enum/define

	LOG_DEBUG("Handle %s ATT connection from %s", isBLE ? "BLE" : "BR/EDR", addrToStr(l2addr.l2_bdaddr).c_str());

	while(true)
	{
		pollfd cpfd{clientFD, POLLIN, 0};
		poll(&cpfd, 1, -1);

		if(cpfd.revents & POLLHUP)
		{
			break;
		}

		std::vector<uint8_t> data;
		auto ret = readData(clientFD, data);
		if(ret > 0)
		{
			HEXDUMP_DEBUG("Received data", data.data(), ret);
			try
			{
				writeData(clientFD, processCommands(data));
			}
			catch(const AttError& err)
			{
				writeData(clientFD, createErrorResponse(err));
			}
			catch(...)
			{
				LOG_ERROR("Unhandled exception received...");
				break;
			}
		}
	}

	LOG_DEBUG("Closing ATT connection with %s", addrToStr(l2addr.l2_bdaddr).c_str());
	close(clientFD);
}

DataBuffer ATTServer::processCommands(DataBuffer& data)
{
	uint8_t opcode = toUINT8(data);
	try
	{
		LOG_DEBUG("Processing command 0x%02X", opcode);

		switch(opcode)
		{
			case ATT_READ_BY_TYPE_REQ:
				return handleReadByTypeReq(data);

			case ATT_READ_BY_GROUP_TYPE_REQ:
				return handleReadByGroupReq(data);

			default:
				LOG_ERROR("Unknown opcode received: 0x%02X", opcode);
				throw AttErrorCodes::RequestNotSupported;
		}
	}
	catch(AttErrorCodes errCode)
	{
		throw AttError(errCode, opcode);
	}
	catch(const HandleError& handleError)
	{
		throw AttError(handleError.first, opcode, handleError.second);
	}
}

DataBuffer ATTServer::handleReadByTypeReq(DataBuffer& data)
{
	LOG_DEBUG("Read by Type request received");

	uint16_t startHandle = toUINT16(data);
	uint16_t endHandle = toUINT16(data);
	UUID attType;

	const size_t remaining = data.size();
	switch(remaining)
	{
		case 2:
			attType = UUID(toUINT16(data));
			break;

		case 16:
			attType = UUID(toByteSeq(data, 16));
			break;

		default:
			LOG_ERROR("Wrong attribute type length - %ld", remaining);
			throw AttErrorCodes::InvalidAttributeValueLength;
	}

	LOG_DEBUG("    Start Handle: 0x%04X", startHandle);
	LOG_DEBUG("    End Handle: 0x%04X", endHandle);

	if(attType.isUUID16())
	{
		LOG_DEBUG("    Attribute type: 0x%04X", attType.getUUID16());
	}
	else
	{
		auto uuid128 = attType.getUUID128();
		HEXDUMP_DEBUG("Attribute type", uuid128.data(), uuid128.size());
	}

	if(startHandle > endHandle || startHandle == 0x0000)
	{
		LOG_ERROR("Wrong handle range provided!");
		throw HandleError(AttErrorCodes::InvalidHandle, startHandle);
	}

	// The first attributes BlueZ is asking are:
	// --- Database Hash 0x2B2A
	// --- Service Changed 0x2A05
	// As per Bluetooth GATT documentation:
	// "If the Database Hash and Service Changed characteristics are both present on the server, then the server shall support the Robust Caching feature."
	// Seems like hashing and service changed attributes are needed for GATT servers that expect attribute tree to be
	// changed dynamically by the user.
	// So, TODO: this in the future
	if(attType == uuids::DatabaseHash || attType == uuids::ServiceChanged)
	{
		throw HandleError(AttErrorCodes::AttributeNotFound, startHandle);
	}

	// FIXME and TODO: Do this dynamically and move creation into separate function?
	// TODO: Check that handle is within range
	uint8_t uuid[16];
	memset(uuid, 0x83, sizeof(uuid));
	uint16_t handle = 0x1234;

	std::vector<uint8_t> response;
	response.push_back(ATT_READ_BY_TYPE_RSP);

	size_t attrLen = sizeof(uuid) + sizeof(handle);
	response.push_back(attrLen & 0xFF);

	response.push_back(handle & 0xFF);
	response.push_back(handle >> 8);

	response.insert(response.end(), uuid, uuid + sizeof(uuid));

	return response;
}

DataBuffer ATTServer::handleReadByGroupReq(DataBuffer& data)
{
	LOG_DEBUG("Read by Group request received");

	uint16_t startHandle = toUINT16(data);
	uint16_t endHandle = toUINT16(data);
	UUID attType;

	const size_t remaining = data.size();
	switch(remaining)
	{
		case 2:
			attType = UUID(toUINT16(data));
			break;

		case 16:
			attType = UUID(toByteSeq(data, 16));
			break;

		default:
			LOG_ERROR("Wrong attribute type length - %ld", remaining);
			throw AttErrorCodes::InvalidAttributeValueLength;
	}

	LOG_DEBUG("    Start Handle: 0x%04X", startHandle);
	LOG_DEBUG("    End Handle: 0x%04X", endHandle);

	if(attType.isUUID16())
	{
		LOG_DEBUG("    Attribute type: 0x%04X", attType.getUUID16());
	}
	else
	{
		auto uuid128 = attType.getUUID128();
		HEXDUMP_DEBUG("Attribute type", uuid128.data(), uuid128.size());
	}

	if(startHandle > endHandle || startHandle == 0x0000)
	{
		LOG_ERROR("Wrong handle range provided!");
		throw HandleError(AttErrorCodes::InvalidHandle, startHandle);
	}

	// TODO: We won't support Secondary service for now (all services are primary)
	if(attType == uuids::SecondaryService)
	{
		throw HandleError(AttErrorCodes::UnsupportedGroupType, startHandle);
	}

	// TODO:
	throw HandleError(AttErrorCodes::AttributeNotFound, startHandle);
}

ATTServer::~ATTServer()
{
	LOG_DEBUG("ATT Server exiting...");
}
