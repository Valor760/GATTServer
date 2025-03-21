#pragma once
#include "betterbuffer.h"

#include <stdint.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/l2cap.h>

#include <vector>

#define ATT_PSM 0x001F /* PSM for BR/EDR */
#define ATT_CID 0x0004 /* Channel for BLE */

class ATTServer
{
	int bredrFD;
	int bleFD;

	void acceptAttConnection(int serverFD);
	void handleAttConnection(int clientFD, struct sockaddr_l2 l2addr);
	std::vector<uint8_t> processCommands(const std::vector<uint8_t>& data);

	// Remote request handling
	std::vector<uint8_t> handleReadByTypeReq(BetterBuffer& buf);

public:
	ATTServer();
	~ATTServer();

	void finish();
	void run();
};
