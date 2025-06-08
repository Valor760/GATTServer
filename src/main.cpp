#include "att.h"
#include "utils/log.h"
#include "advertising.h"

int main()
{
	try
	{
		Advertising adv;
		ATTServer srv;
		srv.run();
	}
	catch(...)
	{
		LOG_DEBUG("Failed to start");
		return -1;
	}

	return 0;
}