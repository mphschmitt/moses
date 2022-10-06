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

#ifndef __PARENT_H__
#define __PARENT_H__

#include "pipe.h"

struct args;

/* @brief Run the parent process passing the arguments to the program.
 *
 * @param args The arguments of the program.
 * @param pfds The file descriptors of the pipe.
 * @param pid The pid of the parent process.
 *
 * @return 0 if the parent successfully read from pipe to the child process.
 */
int run_parent(struct args * args, int pfds[PFD_NUMBER], pid_t pid);

#endif /* __PARENT_H__ */


