#include "log.h"

void dumpHex(const char* name, const void* data, size_t size)
{
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	std::printf("%s ptr:%p size:%lu\n", name, data, size);
	std::printf("---- -----------------------------------------------   ----------------\n");
	for(i = 0; i < size; ++i)
	{
		if(i % 16 == 0)
		{
			std::printf("%03lX| ", i);
		}

		std::printf("%02X ", ((unsigned char *) data)[i]);
		if(((unsigned char *) data)[i] >= ' ' && ((unsigned char *) data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char *) data)[i];
		}
		else {
			ascii[i % 16] = '.';
		}
		if((i + 1) % 8 == 0 || i + 1 == size) {
			if((i + 1) % 16 == 0) {
				std::printf("| %s \n", ascii);
			}
			else if(i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				for(j = (i + 1) % 16; j < 16; ++j) {
					std::printf("   ");
				}
				std::printf("| %s \n", ascii);
			}
		}
	}
}