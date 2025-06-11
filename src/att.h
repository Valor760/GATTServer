#pragma once
#include "types.h"
#include "gattserver.h"
#include "utils/socket.h"

#include <stdint.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/l2cap.h>

#include <vector>

#define ATT_PSM 0x001F /* PSM for BR/EDR */
#define ATT_CID 0x0004 /* Channel for BLE */

using L2CapSock = Socket<AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP>;

class ATTBind
{
	L2CapSock sock;

public:
	ATTBind(uint16_t psm, uint16_t cid, uint8_t addrType);

	int getFD() const;
};

class ATTClient
{
	struct sockaddr_l2 addr = {};
	unsigned int opt = sizeof(addr);

public:
	int fd;

	ATTClient(const ATTBind& sock);
	~ATTClient();
};

class ATTServer
{
	GATTServer gatt;
	ATTBind bredrHandle;
	ATTBind bleHandle;

	void handleAttConnection(int clientFD);
	DataBuffer processCommands(DataBuffer& data);

	// Remote request handling
	DataBuffer handleReadByTypeReq(DataBuffer& data);
	DataBuffer handleReadByGroupReq(DataBuffer& data);
	DataBuffer handleFindInfoReq(DataBuffer& data);
	DataBuffer handleReadReq(DataBuffer& data);
	DataBuffer handleReadBlobReq(DataBuffer& data);
	DataBuffer handleWriteReq(DataBuffer& data);
	DataBuffer handleWritePrepareReq(DataBuffer& data);
	DataBuffer handleWriteExecuteReq(DataBuffer& data);

public:
	ATTServer();
	~ATTServer();

	void finish();
	void run();
};
