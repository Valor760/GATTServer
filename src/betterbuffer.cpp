#include "betterbuffer.h"
#include "log.h"

BetterBuffer::BetterBuffer(std::vector<uint8_t> data)
	: data(std::move(data)), ptrIdx(0)
{
}

template <>
uint8_t BetterBuffer::get<uint8_t>(bool reverse)
{
	if(ptrIdx + 1 > data.size())
	{
		LOG_ERROR("Wanted 1 byte, but available only %ld", data.size() - ptrIdx);
		throw -1;
	}

	uint8_t ret = data[ptrIdx];
	ptrIdx++;
	return ret;
}

template <>
uint16_t BetterBuffer::get<uint16_t>(bool reverse)
{
	if(ptrIdx + 2 > data.size())
	{
		LOG_ERROR("Wanted 2 bytes, but available only %ld", data.size() - ptrIdx);
		throw -1;
	}

	uint16_t ret = 0;
	if(reverse)
	{
		ret = data[ptrIdx] + (data[ptrIdx + 1] << 8);
	}
	else
	{
		ret = data[ptrIdx + 1] + (data[ptrIdx] << 8);
	}

	ptrIdx += 2;
	return ret;
}