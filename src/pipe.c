#include "pipe.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int close_pipe_end(int fd)
{
	int ret = close(fd);
	if (ret < 0)
	{
		printf("Error(%d): failed to close pipe entrance (fd %d): %s\n",
			getpid(), fd, strerror(errno));
		ret = errno;
	}

	return ret;
}

