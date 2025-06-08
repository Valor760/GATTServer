#include "betterbuffer.h"
#include "utils/log.h"

#include <algorithm>

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

std::vector<uint8_t> BetterBuffer::getBytes(size_t amount, bool reverse)
{
	if(ptrIdx + amount > data.size())
	{
		LOG_ERROR("Wanted %ld bytes, but available only %ld", amount, data.size() - ptrIdx);
		throw -1;
	}

	std::vector<uint8_t> ret;
	ret.insert(ret.begin(), data.begin() + ptrIdx, data.begin() + ptrIdx + amount);

	if(reverse)
	{
		std::reverse(ret.begin(), ret.end());
	}

	return ret;
}

size_t BetterBuffer::bytesLeft() const
{
	return data.size() - ptrIdx;
}