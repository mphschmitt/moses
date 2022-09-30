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

#ifndef __LEVENSTEIN_H__
#define __LEVENSTEIN_H__

/* @brief Levenshtein algorithm
 *
 * Calculate the distance between two string.
 * Code written using the pseudo-code from the wikipedia page:
 * https://en.wikipedia.org/wiki/Levenshtein_distance
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return The Levenshtein's distance, or less than 0 if it fails.
 */
int lev_string_dist(char const * s1, char const * s2);

#endif /* __LEVENSTEIN_H__ */
