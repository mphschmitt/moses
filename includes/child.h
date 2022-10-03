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

#ifndef __CHILD_H__
#define __CHILD_H__

#include "pipe.h"

/* @brief Run the child process passing the arguments to the program.
 *
 * @param haystack The file to analyze.
 * @param pfds The file descriptors of the pipe.
 * @return 0 if the fd has successfully been closed, less than 0 otherwise.
 */
void run_child(char * haystack, int pfds[PFD_NUMBER]);

#endif /* __CHILD_H__ */

