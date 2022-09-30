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

#define BUF_SIZE 2048

static void usage(void)
{
	printf(
		"Usage: moses [options] [needle] [haystack]\n"
		"Search for the symbol needle into haystack (a file or a folder).\n"
		"  -h  --help       display this help message and exit\n"
		"  -v  --version    output version information and exit\n");
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

static char check_arguments(int argc, char *argv[], char ** needle,
		char ** haystack)
{
	int opt;
	char args = 0;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
		switch (opt) {
		case 'v':
			if (optind < argc) {
				usage();
				return -EINVAL;
			}
			version();
			return CHAR_MIN;
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
		*needle = strndup(argv[optind], strlen(argv[optind]));
		*haystack = strndup(argv[optind + 1], strlen(argv[optind + 1]));
	}
	else
	{
		usage();
		return -EINVAL;
	}

	return args;
}

static int read_fd(int fd)
{
	int ret = 0;
	ssize_t bytes = 0;
	char buffer[BUF_SIZE] = { 0 };

	while (1)
	{
		bytes = read(fd, buffer, sizeof(buffer));
		if (bytes < 0)
		{
			printf("Error(%d): failed to read from fd %d: %s\n",
				getpid(), fd, strerror(errno));
			ret = errno;
			break;
		}
		else if (bytes == 0)
			break;

		printf("%s", buffer);

		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

int main(int argc, char *argv[])
{
	char * needle = NULL;
	char * haystack = NULL;
	char args = 0;
	int ret = 0;
	int pfds[2] = { 0 };
	pid_t pid;

	args = check_arguments(argc, argv, &needle, &haystack);
	if (args < 0) {
		ret = (args == CHAR_MIN) ? 0 : -args;
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
				haystack,
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
				break;
			}

			/* Redirect the pipe to stdin. */
			close(STDIN_FILENO);
			dup2(pfds[PFD_READ], STDIN_FILENO);

			int wstatus = 0;
			ret = read_fd(STDIN_FILENO);
			waitpid(pid, &wstatus, 0);

			break;
	}

END:
	return ret;
}
