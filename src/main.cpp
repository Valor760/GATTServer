#include "att.h"
#include "log.h"

int main()
{
	try
	{
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