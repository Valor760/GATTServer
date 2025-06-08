#pragma once
#include "types.h"

#include <cstdint>
#include <vector>

uint8_t toUINT8(DataBuffer& data);
uint16_t toUINT16(DataBuffer& data);
DataBuffer toByteSeq(DataBuffer& data, size_t size);

std::vector<uint8_t> createErrorResponse(const AttError& error);
