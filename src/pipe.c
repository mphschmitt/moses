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

