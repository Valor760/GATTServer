#pragma once
#include "types.h"

#include <cstdint>
#include <vector>


std::vector<uint8_t> createErrorResponse(const AttError& error);