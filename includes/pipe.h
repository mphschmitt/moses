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

#ifndef __PIPE_H__
#define __PIPE_H__

#define PID_CHILD 0

enum PFD
{
	PFD_READ = 0,
	PFD_WRITE = 1,
	PFD_NUMBER
};

/* @brief Close the end of the pipe.
 *
 * @param fd The file descriptor of the end of the pipe to close.
 * @return 0 if the fd has successfully been closed, less than 0 otherwise.
 */
int close_pipe_end(int fd);


#endif /* __PIPE_H__ */
