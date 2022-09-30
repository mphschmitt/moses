/* moses Find symbol in shared libraries.
 * Copyright (C) 2022  Mathias Schmitt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "pipe.h"
#include "levenshtein.h"

#define BUF_SIZE 2048
#define MIN_DISTANCE 70.0

struct args
{
	char * needle;
	char * haystack;
	double min_distance;
};

static void usage(void)
{
	printf(
		"Usage: moses [options] [needle] [haystack]\n"
		"Search for the symbol needle into haystack (a file or a folder).\n"
		"  -h  --help         display this help message and exit.\n"
		"  -v  --version      output version information and exit.\n"
		"  -d  --min_distance the minimum distance to needle for a "
			"string to be a match.\n");
}

static void version(void)
{
	printf(
		"moses 1.0.0\n"
		"\n"
		"Copyright (C) 2022 Mathias Schmitt\n"
		"License: GNU Affero General Public License <https://gnu.org/licenses/agpl.html>.\n"
		"This is free software, and you are welcome to change and redistribute it.\n"
		"This program comes with ABSOLUTELY NO WARRANTY.\n");
}

static char check_arguments(int argc, char *argv[], struct args * args)
{
	int opt;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{"min_distance", required_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "hvd:", long_options, NULL)) != -1) {
		switch (opt) {
		case 'v':
			if (optind < argc) {
				usage();
				return -EINVAL;
			}
			version();
			return CHAR_MIN;
		case 'd':
			args->min_distance = atof(optarg);
			if (args->min_distance == 0)
			{
				printf("Invalid argument to 'd' option.\n");
				usage();
				return -EINVAL;
			}
			printf("Minimum distance for a match: %s\n", optarg);
			break;
		case 'h':
		case '?':
			usage();
			return CHAR_MIN;
		default:
			break;
		}
	}

	if (optind + 2 <= argc)
	{
		args->needle = strndup(argv[optind], strlen(argv[optind]));
		args->haystack = strndup(argv[optind + 1], strlen(argv[optind + 1]));
	}
	else
	{
		usage();
		return -EINVAL;
	}

	return 0;
}

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

static int read_fd(FILE * stream, struct args * args)
{
	int ret = 0;
	ssize_t bytes = 0;
	size_t buffer_size = 0;
	char * buffer = NULL;

	while (1)
	{
		bytes  = getline(&buffer, &buffer_size, stream);
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
				args->needle,
				buffer);
		if (lev_distance >= args->min_distance)
			printf("symbol: %s matches %.1f%%\n", buffer, lev_distance);
	}

	free(buffer);
	buffer = NULL;
	buffer_size = 0;

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int pfds[2] = { 0 };
	struct args args = {
		NULL,
		NULL,
		MIN_DISTANCE
	};
	pid_t pid;

	ret = check_arguments(argc, argv, &args);
	if (ret < 0) {
		ret = (ret == CHAR_MIN) ? 0 : -ret;
		goto END;
	}

	ret = pipe(pfds);
	if (ret < 0)
	{
		printf("Error: failed to create pipe: %s.\n", strerror(errno));
		goto END;
	}

	pid = fork();
	switch (pid)
	{
		case -1:
			printf("Error: failed to create child process: %s\n",
					strerror(errno));
			ret = errno;
			break;
		case PID_CHILD: /* Child process. */
			/* Close read side of the pipe. It only needs to write
			 * it stdout in it.
			 */
			ret = close_pipe_end(pfds[PFD_READ]);
			if (ret < 0)
			{
				ret = errno;
				break;
			}

			/* Redirect stdout to the pipe. */
			close(STDOUT_FILENO);
			dup2(pfds[PFD_WRITE], STDOUT_FILENO);

			char * const arguments[] = {
				"nm",
				"--dynamic",
				args.haystack,
				0
			};
			ret = execve("/usr/bin/nm", arguments, 0);
			if (ret < 0)
			{
				printf("Error(%d): failed to execute nm: %s\n",
					getpid(), strerror(errno));
			}
			exit(0);
			break;
		default: /* Parent process. */
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

			FILE * istream;
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
			ret = read_fd(istream, &args);
			waitpid(pid, &wstatus, 0);

			fclose(istream);

			break;
	}

END:
	if(args.haystack)
		free(args.haystack);

	if(args.needle)
		free(args.needle);

	return ret;
}
