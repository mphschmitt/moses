#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "levenshtein.h"
#include "common.h"
#include "parent.h"

static void extract_symbol(char * str)
{
	size_t len = strlen(str);
	size_t i = 0;

	while(len > 0 && str[len] != ' ')
		len--;

	len++; /* Remove whitespace. */

	while(str[len] && str[len] != '\n')
		str[i++] = str[len++];
	str[i] = '\0';
}


static int read_fd(FILE * stream, struct args * args, char const * file)
{
	int ret = 0;
	size_t buffer_size = 0;
	char * buffer = NULL;

	while (1)
	{
		ssize_t bytes = 0;

		bytes = getline(&buffer, &buffer_size, stream);
		if (bytes < 0 && errno)
		{
			free(buffer);
			buffer = NULL;
			buffer_size = 0;

			printf("Error(%d): failed to read from fd %d: %s\n",
				getpid(), fileno(stream), strerror(errno));
			ret = errno;
			break;
		}
		else if (bytes < 0)
		{
			break;
		}

		extract_symbol(buffer);
		double lev_distance = lev_dist_percent(
				lev_string_dist(args->needle, buffer),
				args->needle, buffer);
		if (lev_distance >= args->min_distance)
			printf("%s\t%s%s%.1f%%\n", file, buffer,
					args->verbose ? " matches " : "\t",
					lev_distance);
	}

	free(buffer);
	buffer = NULL;
	buffer_size = 0;

	return ret;
}


int run_parent(struct args * args, int pfds[PFD_NUMBER], pid_t pid, char const * file)
{
	int ret = 0;
	FILE * istream = NULL;

	/* Close write side of the pipe. It only needs to read
	 * the output of the child process from it.
	 */
	ret = close_pipe_end(pfds[PFD_WRITE]);
	if (ret < 0)
	{
		ret = errno;
		goto END;
	}

	/* Redirect the pipe to stdin. */
	close(STDIN_FILENO);
	dup2(pfds[PFD_READ], STDIN_FILENO);

	istream = fdopen(pfds[PFD_READ], "r");
	if (istream == NULL)
	{
		printf("Error(%d): failed to read from child "
			"process: %s\n", getpid(),
			strerror(errno));
		ret = errno;

		goto END;
	}

	int wstatus = 0;
	ret = read_fd(istream, args, file);
	waitpid(pid, &wstatus, 0);

END:
	if (istream)
		fclose(istream);

	return ret;
}
