#pragma once

#include "log.h"

#include <sys/socket.h>
#include <errno.h>

template <int DOMAIN, int TYPE, int PROTOCOL>
class Socket
{
	int fd;

public:
	Socket()
		: fd(socket(DOMAIN, TYPE, PROTOCOL))
	{
		if(fd < 0)
		{
			LOG_ERROR("Failed to create socket (d: %d t: %d p: %d) [errno %d: %m]", DOMAIN, TYPE, PROTOCOL, errno);
			throw -1;
		}
	}

	~Socket()
	{
		close(fd);
	}

	operator int() const
	{
		return fd;
	}
};