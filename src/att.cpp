#include "att.h"
#include "log.h"
#include "betterbuffer.h"
#include "types.h"
#include "att_utils.h"

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

ATTServer::ATTServer()
	: bredrFD(socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)),
	  bleFD(socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP))
{
	LOG_DEBUG("Start Bluetooth ATT server...");

	if(bredrFD < 0)
	{
		LOG_ERROR("Failed to allocate BR/EDR socket [errno %d: %m]", errno);
		throw -1;
	}

	if(bleFD < 0)
	{
		LOG_ERROR("Failed to allocate BLE socket [errno %d: %m]", errno);
		throw -1;
	}

	// BR/EDR bind
	{
		struct sockaddr_l2 addr = {0, 0, {0}, 0, 0};
		bdaddr_t any            = {0, 0, 0, 0, 0, 0};
		addr.l2_family = AF_BLUETOOTH; /* Addressing family, always AF_BLUETOOTH */
		bacpy(&addr.l2_bdaddr, &any);  /* Bluetooth address of local bluetooth adapter */
		addr.l2_psm = htobs(ATT_PSM);
		addr.l2_cid = htobs(0);
		addr.l2_bdaddr_type = 0; // TODO: Use enum/define
	
		if(bind(bredrFD, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			LOG_ERROR("Failed to bind BR/EDR ATT %d [%s]", errno, strerror(errno));
			throw -1;
		}
	
		listen(bredrFD, 5);
	}

	// BLE bind
	{
		struct sockaddr_l2 addr = {0, 0, {0}, 0, 0};
		bdaddr_t any            = {0, 0, 0, 0, 0, 0};
		addr.l2_family = AF_BLUETOOTH; /* Addressing family, always AF_BLUETOOTH */
		bacpy(&addr.l2_bdaddr, &any);  /* Bluetooth address of local bluetooth adapter */
		addr.l2_psm = htobs(0);
		addr.l2_cid = htobs(ATT_CID);
		addr.l2_bdaddr_type = 1; // TODO: Use enum/define

		if(bind(bleFD, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			LOG_ERROR("Failed to bind BLE ATT %d [%s]", errno, strerror(errno));
			throw -1;
		}

		listen(bleFD, 5);
	}

	LOG_DEBUG("Bound ATT server");
}

void ATTServer::finish()
{
}

void ATTServer::run()
{
	/* Allow to do a connection and close it immediately. It is enough for iOS. */

	while(true)
	{
		std::vector<pollfd> pfds = {{bleFD, POLLIN, 0}, {bredrFD, POLLIN, 0}};

		LOG_DEBUG("Wait for ATT connect");

		if(poll(pfds.data(), pfds.size(), -1) <= 0 || (!(pfds[0].revents & POLLIN) && !(pfds[1].revents & POLLIN)))
		{
			LOG_DEBUG("L2CAP signalled to close");
			return;
		}

		if(pfds[0].revents & POLLIN)
		{
			LOG_DEBUG("Accepting BLE ATT connection");
			acceptAttConnection(bleFD);
		}

		if(pfds[1].revents & POLLIN)
		{
			LOG_DEBUG("Accepting BR/EDR ATT connection");
			acceptAttConnection(bleFD);
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
		// TODO: Use ADK-COM util::Thread and make a class for this perhaps?
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

	LOG_DEBUG("Handle %s ATT connection from %pM", isBLE ? "BLE" : "BR/EDR", l2addr.l2_bdaddr.b);

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

	LOG_DEBUG("Closing ATT connection with %pM", l2addr.l2_bdaddr.b);
	close(clientFD);
}

std::vector<uint8_t> ATTServer::processCommands(const std::vector<uint8_t>& data)
{
	BetterBuffer buf(data);

	uint8_t opcode = buf.get<uint8_t>();

	try
	{
		switch(opcode)
		{
			case ATT_READ_BY_TYPE_REQ:
				return handleReadByTypeReq(buf);
	
			default:
				LOG_ERROR("Unknown opcode received: %02X", opcode);
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

std::vector<uint8_t> ATTServer::handleReadByTypeReq(BetterBuffer& buf)
{
	LOG_DEBUG("Read by Type request received");

	uint16_t startHandle = buf.get<uint16_t>();
	uint16_t endHandle = buf.get<uint16_t>();
	uint16_t dbHash = buf.get<uint16_t>(); // TODO: Figure out for what this is needed

	LOG_DEBUG("    Start Handle: 0x%04X", startHandle);
	LOG_DEBUG("    End Handle: 0x%04X", endHandle);
	LOG_DEBUG("    Database Hash: 0x%04X", dbHash);

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

ATTServer::~ATTServer()
{
	close(bredrFD);
	close(bleFD);
}
