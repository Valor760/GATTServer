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
};