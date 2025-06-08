#include "utils.h"

std::string addrToStr(const bdaddr_t& addr)
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
		addr.b[5], addr.b[4], addr.b[3], addr.b[2], addr.b[1], addr.b[0]);

	return std::string(tmp);
}