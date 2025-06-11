#include "att.h"
#include "utils/log.h"
#include "advertising.h"

#include <thread>
#include <iostream>

extern DataBuffer generateRandomData(size_t size);

int main()
{
	try
	{
		Advertising adv;
		ATTServer srv;

		std::thread th(&ATTServer::run, &srv);
		th.detach();

		while(true)
		{
			printf("---------------------------------\n");
			printf("1 - Update 0x0028 handle value (notification)\n");
			printf("2 - Update 0x0029 handle value (indication)\n");
			printf("Choice >>> ");

			char ch;
			std::cin >> ch;
			switch(ch)
			{
				case '1':
					srv.updateCharValue(0x0028, generateRandomData(64));
					break;

				case '2':
					srv.updateCharValue(0x0029, generateRandomData(64));
					break;

				default:
					printf("Unknown option '%c'", ch);
					break;
			}
		}
	}
	catch(...)
	{
		LOG_DEBUG("Failed to start");
		return -1;
	}

	return 0;
}