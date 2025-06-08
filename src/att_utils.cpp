#include "att_utils.h"
#include "utils/log.h"

#include <algorithm>

static void checkSize(DataBuffer& data, size_t size)
{
	if(data.size() < size)
	{
		LOG_ERROR("Wanted %lu bytes, but available only %lu", size, data.size());
		throw AttErrorCodes::InvalidAttributeValueLength;
	}
}

static void adjustBuff(DataBuffer& data, size_t size)
{
	data.erase(data.begin(), data.begin() + size);
}

uint8_t toUINT8(DataBuffer& data)
{
	checkSize(data, sizeof(uint8_t));

	uint8_t ret = data[0];

	adjustBuff(data, sizeof(uint8_t));
	return ret;
}

uint16_t toUINT16(DataBuffer& data)
{
	checkSize(data, sizeof(uint16_t));

	uint16_t ret = data[1] + (data[0] << 8);

	adjustBuff(data, sizeof(uint16_t));
	return ret;
}

DataBuffer toByteSeq(DataBuffer& data, size_t size)
{
	checkSize(data, size);

	DataBuffer buf;
	buf.insert(buf.begin(), data.begin(), data.begin() + size);

	adjustBuff(data, size);
	return buf;
}

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
