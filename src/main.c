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

int main(int argc, char *argv[])
{
	char * needle = NULL;
	char * haystack = NULL;
	char args = 0;
	int res = 0;

	args = check_arguments(argc, argv, &needle, &haystack);
	if (args < 0) {
		res = (args == CHAR_MIN) ? 0 : -args;
		goto END;
	}

END:
	return res;
}
