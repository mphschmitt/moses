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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

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

/* Strip the filename from a full file path. */
static void get_path(char * file_path)
{
	size_t i = strlen(file_path);

	if (i <= 1 || !strncmp(file_path, "/", i))
		return;

	i -= 2;
	while (i > 0 && file_path[i] != '/')
		i--;
	if (file_path[i + 1] == '/')
		file_path[i + 1] = '\0';
}

static int search(struct args * args, char * file)
{
	int pfds[PFD_NUMBER] = { 0 };
	pid_t pid;
	int ret = 0;

	printf("Searching in haystack: %s\n", file);

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
			run_child(file, pfds);
			break;
		default: /* Parent process. */
			ret = run_parent(args, pfds, pid, file);
			break;
	}

	return ret;
}

/* @brief Check if a file is a shared elf object.
 *
 * Check the first four bytes of the file (magic numbers) to check its type.
 *
 * @param file_path The path of the file to open.
 * @return 1 if the file is a shared elf object, 0 otherwise.
 */
static int file_is_shared_elf(char * file_path)
{
	int res = 0;
	char magic_numbers[4] = { 0 };

	FILE * file = fopen(file_path, "r");
	if (!file)
	{
		printf("Error: failed to open file %s: %s\n", file_path,
			strerror(errno));
		return 0;
	}

	size_t bytes_read = fread(magic_numbers, sizeof(char), 4, file);
	if (bytes_read == 0)
	{
		int read_error = ferror(file);
		if (read_error)
		{
			printf("Error: failed to read from file: %s\n",
				file_path);
			return 0;
		}
	}

	res = fclose(file);
	if (res)
	{
		printf("Error: failed to close file %s: %s\n", file_path,
			strerror(errno));
		res = 0;
	}

	if (magic_numbers[0] == 0x7f &&
		magic_numbers[1] == 0x45 &&
		magic_numbers[2] == 0x4c &&
		magic_numbers[3] == 0x46)
		res = 1;

	return res;
}

static int analyze_file(struct args * args, char * file)
{
	int ret = 0;
	struct stat statbuff;

	if (!strcmp(file, ".") || !strcmp(file, ".."))
		return 0;

	ret = stat(file, &statbuff);
	if (ret < 0)
	{
		printf("Failed to stat file '%s': %s. Skipping...\n", file,
			strerror(errno));
		return ret;
	}

	switch (statbuff.st_mode & S_IFMT) {
		case S_IFDIR: /* File is a directory. */
		{
			DIR * dir = NULL;
			char * path = NULL;

			dir = opendir(file);
			if (!dir)
				return -errno;

			path = strndup(file, strlen(file));
			if (!path)
			{
				printf("Failed to allocate memory: %s\n",
					strerror(errno));
				return -errno;
			}
			get_path(path);

			while (1)
			{
				char * fullpath = NULL;
				struct dirent * dirent = readdir(dir);

				if (!dirent && errno)
				{
					free(path);
					closedir(dir);
					return -errno;
				}
				else if (!dirent)
					break;

				/* Skip hidden files, '.' and '..' */
				if (dirent->d_name[0] == '.')
					continue;

				fullpath = calloc(strlen(path) + strlen(dirent->d_name) + 2, sizeof(char));
				if (!fullpath)
				{
					printf("Failed to allocate memory: %s\n",
						strerror(errno));
					free(path);
					closedir(dir);
					return -errno;
				}

				strncat(fullpath, path, strlen(path));
				if (path[strlen(fullpath) - 1] != '/')
					fullpath[strlen(fullpath)] = '/';
				strncat(fullpath, dirent->d_name, strlen(dirent->d_name));

				analyze_file(args, fullpath);

				free(fullpath);
			}

			free(path);
			closedir(dir);

			break;
		}
		case S_IFREG: /* File is a regular file. */
		{
			if (!file_is_shared_elf(file))
				break;

			ret = search(args, file);
			if (ret < 0)
			{
				printf("The search for symbol '%s' failed. "
					"Skipping...\n", file);
				return ret;
			}
			break;
		}
		default:
			printf("File '%s' is neither a shared object nor a "
				"directory. Skipping...\n", file);
			break;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
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

		ret = analyze_file(&args, args.haystacks[i]);
		if (ret == -ENOMEM)
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
