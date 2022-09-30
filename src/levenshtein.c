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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* @brief Calculate the minimum cost between the three possible operations.
 *
 * Characters can be inserted, deleted or substituated between the two strings.
 * Levenshtein's algorithm requires to use the lowest value. The values can be
 * chosen arbitrarily to weight the different operations
 *
 * @param substituion The cost of a substitution at this position.
 * @param deletion The cost of a deletion at this position.
 * @param insertion The cost of an insertion at this position.
 */
static int lev_minimum(int substitution, int deletion, int insertion)
{
	if (substitution <= deletion && substitution <= insertion)
		return substitution;

	if (deletion <= insertion && deletion <= substitution)
		return deletion;

	return insertion;
}

int lev_string_dist(char const * s1, char const * s2)
{
	size_t max_size = strlen(s1) > strlen(s2) ? strlen(s1) : strlen(s2);
	size_t min_size = strlen(s1) < strlen(s2) ? strlen(s1) : strlen(s2);
	int * line1;
	int * line2;

	line1 = (int *)malloc((max_size + 1) * sizeof(int));
	if (!line1)
		return -ENOMEM;

	line2 = (int *)malloc((max_size + 1) * sizeof(int));
	if (!line2)
	{
		free(line1);
		line1 = NULL;
		return -ENOMEM;
	}

	for (size_t i = 0; i < max_size + 1; ++i)
	{
		line1[i] = (int)i;
		line2[i] = 0;
	}

	for (size_t i = 0; i < min_size; ++i)
	{
		line2[0] = (int)(i + 1);

		for (size_t j = 0; j < max_size; ++j)
		{
			int deletion_cost = line1[j + 1] + 1;
			int insertion_cost = line2[j] + 1;
			int substitution_cost;

			if (s1[i] == s2[j])
				substitution_cost = line1[j];
			else
				substitution_cost = line1[j] + 1;

			line2[j + 1] = lev_minimum(substitution_cost,
						deletion_cost, insertion_cost);
		}

		int * tmp = NULL;
		tmp = line1;
		line1 = line2;
		line2 = tmp;
	}

	int ret_val = line1[max_size];

	free(line1);
	free(line2);

	return ret_val;
}
