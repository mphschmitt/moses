#include "child.h"
#include "pipe.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void run_child(char * haystack, int pfds[PFD_NUMBER], int verbose)
{
	int ret = 0;

	/* Close read side of the pipe. It only needs to write
	 * it stdout in it.
	 */
	ret = close_pipe_end(pfds[PFD_READ]);
	if (ret < 0)
		exit(ret);

	/* Close stderr. */
	if (!verbose)
		close(STDERR_FILENO);

	/* Redirect stdout to the pipe. */
	close(STDOUT_FILENO);
	dup2(pfds[PFD_WRITE], STDOUT_FILENO);

	char * const arguments[] = {
		"nm",
		"--dynamic",
		haystack,
		0
	};
	ret = execve("/usr/bin/nm", arguments, 0);
	if (ret < 0)
	{
		printf("Error(%d): failed to execute nm: %s\n",
			getpid(), strerror(errno));
	}

	exit(errno);
}
