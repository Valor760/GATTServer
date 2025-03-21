#include "att_utils.h"

std::vector<uint8_t> createErrorResponse(const AttError& error)
{
	std::vector<uint8_t> buf;
	buf.push_back(ATT_ERROR_RSP);
	buf.push_back(error.opcode);
	buf.push_back(error.handle & 0xFF);
	buf.push_back(error.handle >> 8);
	buf.push_back(static_cast<uint8_t>(error.error));

	return buf;
}