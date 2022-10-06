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
#include <stdlib.h>

#include "common.h"
#include "pipe.h"
#include "child.h"
#include "parent.h"

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
	int haystack_nb = 0;
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

	while (optind < argc)
	{
		if (!args->needle)
		{
			args->needle = strndup(argv[optind],
					strlen(argv[optind]));
		}
		else
		{
			args->haystacks[haystack_nb++] = strndup(argv[optind],
					strlen(argv[optind]));
		}

		optind++;
	}

	if (!args->needle || !args->haystacks[0])
	{
		usage();
		return -EINVAL;
	}

	return 0;
}

static int search(struct args * args, int index, int pfds[PFD_NUMBER])
{
	pid_t pid;
	int ret = 0;

	printf("Searching in haystack: %s\n", args->haystacks[index]);

	ret = pipe(pfds);
	if (ret < 0)
	{
		printf("Error: failed to create pipe: %s.\n",
				strerror(errno));
		return ret;
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
			run_child(args->haystacks[index], pfds);
			break;
		default: /* Parent process. */
			ret = run_parent(args, pfds, pid);
			break;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int pfds[PFD_NUMBER] = { 0 };
	struct args args = {
		NULL,
		{ 0 },
		MIN_DISTANCE
	};

	ret = check_arguments(argc, argv, &args);
	if (ret < 0) {
		ret = (ret == CHAR_MIN) ? 0 : -ret;
		goto END;
	}

	for(int i = 0; i < MAX_HAYSTACKS; i++)
	{
		if (!args.haystacks[i])
			break;

		{
		}

		ret = search(&args, i, pfds);
		if (ret < 0)
			break;
	}

END:
	for(int i = 0; i < MAX_HAYSTACKS; i++)
	{
		if(!args.haystacks[i])
			break;

		free(args.haystacks[i]);
	}

	if(args.needle)
		free(args.needle);

	return ret;
}
