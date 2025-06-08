#pragma once

#include <cstdint>
#include <vector>

class BetterBuffer
{
	const std::vector<uint8_t> data;
	size_t ptrIdx;

public:
	BetterBuffer(std::vector<uint8_t> data);

	template <typename T>
	T get(bool reverse = true);

	std::vector<uint8_t> getBytes(size_t amount, bool reverse = true);

	size_t bytesLeft() const;
};