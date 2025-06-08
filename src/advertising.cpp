#include "advertising.h"
#include "utils/socket.h"

#include <stdlib.h>
#include <string.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include <vector>

#define PACKED __attribute__((packed))

struct BTCommandHeader
{
	uint16_t cmd;
	uint16_t controller;
	uint16_t len;
} PACKED;

struct BTCommand
{
	BTCommandHeader hdr;
	uint8_t data[65535];
} PACKED;

struct HCIEventHdr
{
	uint16_t type;
	uint16_t idx;
	uint16_t len;
} PACKED;

#define MGMT_OP_ADD_EXT_ADV_PARAMS 0x0054
struct mgmt_cp_add_ext_adv_params {
	uint8_t		instance;
	uint32_t	flags;
	uint16_t	duration;
	uint16_t	timeout;
	uint32_t	min_interval;
	uint32_t	max_interval;
	int8_t		tx_power;
} PACKED;

#define MGMT_OP_ADD_EXT_ADV_DATA		0x0055
struct mgmt_cp_add_ext_adv_data {
	uint8_t	instance;
	uint8_t	adv_data_len;
	uint8_t	scan_rsp_len;
	uint8_t	data[0];
} PACKED;

class HCISocket : public Socket<AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI>
{
public:
	HCISocket()
	{
		sockaddr_hci addr{AF_BLUETOOTH, HCI_DEV_NONE, HCI_CHANNEL_CONTROL};

		if(bind(*this, (sockaddr*) &addr, sizeof(addr)))
		{
			LOG_ERROR("Failed to bind HCI [errno %d: %m]", errno);
			throw -1;
		}
	}
};

static bool sendCmd(HCISocket& sock, uint16_t cmd, void* data, size_t len)
{
	BTCommand msg{};

	msg.hdr.cmd = cmd;
	msg.hdr.controller = 0;

	if(data && len > 0)
	{
		memcpy(msg.data, data, len);
		msg.hdr.len = len;
	}

	LOG_DEBUG("Sending cmd 0x%04X", cmd);

	ssize_t ret = send(sock, &msg, sizeof(msg.hdr) + msg.hdr.len, MSG_NOSIGNAL);
	if((size_t) ret != sizeof(msg.hdr) + msg.hdr.len)
	{
		LOG_ERROR("Failed to send (ret = %ld) [errno %d: %m]", ret, errno);
		return false;
	}

	return true;
}

static void readResult(HCISocket& sock)
{
	uint8_t data[65535];

	while(true)
	{
		ssize_t ret = recv(sock, data, sizeof(data), 0);
		if((size_t) ret >= sizeof(HCIEventHdr))
		{
			HCIEventHdr& evt = (HCIEventHdr&) data;
			if(evt.type == 1)
			{
				LOG_DEBUG("Complete event received");
				break;
			}
			else if(evt.type == 2)
			{
				LOG_ERROR("Error event occurred");
				break;
			}
		}
	}
}

Advertising::Advertising()
{
	HCISocket sock;
	{
		struct mgmt_cp_add_ext_adv_params params = {
			.instance = 1,
			.flags = 0x00000001,
			.duration = 0,
			.timeout = 0,
			.min_interval = 0,
			.max_interval = 0,
			.tx_power = 0
		};

		if(sendCmd(sock, MGMT_OP_ADD_EXT_ADV_PARAMS, &params, sizeof(params)))
		{
			readResult(sock);
		}
	}

	{
		struct mgmt_cp_add_ext_adv_data* params = (mgmt_cp_add_ext_adv_data*) malloc(sizeof(mgmt_cp_add_ext_adv_data) + 4);

		params->instance = 1,
		params->adv_data_len = 4,
		params->scan_rsp_len = 0,
		params->data[0] = 0x03;
		params->data[1] = 0x19;
		params->data[2] = 0x00;
		params->data[3] = 0x01;

		if(sendCmd(sock, MGMT_OP_ADD_EXT_ADV_DATA, params, sizeof(mgmt_cp_add_ext_adv_data) + 4))
		{
			readResult(sock);
		}

		free(params);
	}
}