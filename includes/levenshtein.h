#ifndef __LEVENSTEIN_H__
#define __LEVENSTEIN_H__

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
static unsigned int lev_minimum(unsigned int substitution,
		unsigned int deletion, unsigned int insertion)
{
	if (substitution <= deletion && substitution <= insertion)
		return substitution;

	if (deletion <= insertion && deletion <= substitution)
		return deletion;

	return insertion;
}

/* @brief Levenshtein algorithm
 *
 * Calculate the distance between two string.
 * Code written using the pseudo-code from the wikipedia page:
 * https://en.wikipedia.org/wiki/Levenshtein_distance
 *
 * @param s1 The first string.
 * @param s2 The second string.
 */
int lev_string_dist(char const * s1, char const * s2)
{
	size_t max_size = strlen(s1) > strlen(s2) ? strlen(s1) : strlen(s2);
	size_t min_size = strlen(s1) < strlen(s2) ? strlen(s1) : strlen(s2);
	unsigned int * line1;
	unsigned int * line2;

	line1 = (unsigned int *)malloc((max_size + 1) * sizeof(unsigned int));
	if (!line1)
		return -ENOMEM;

	line2 = (unsigned int *)malloc((max_size + 1) * sizeof(unsigned int));
	if (!line2)
		return -ENOMEM;

	for (unsigned int i = 0; i < max_size + 1; ++i)
	{
		line1[i] = i;
		line2[i] = 0;
	}

	for (unsigned int i = 0; i < min_size; ++i)
	{
		line2[0] = i + 1;

		for (unsigned int j = 0; j < max_size; ++j)
		{
			unsigned int deletion_cost = line1[j + 1] + 1;
			unsigned int insertion_cost = line2[j] + 1;
			unsigned int substitution_cost;

			if (s1[i] == s2[j])
				substitution_cost = line1[j];
			else
				substitution_cost = line1[j] + 1;

			line2[j + 1] = lev_minimum(substitution_cost,
						deletion_cost, insertion_cost);
		}

		unsigned int * tmp = NULL;
		tmp = line1;
		line1 = line2;
		line2 = tmp;
	}

	unsigned int ret_val = line1[max_size];

	free(line1);
	free(line2);

	return ret_val;
}

#endif /* __LEVENSTEIN_H__ */
