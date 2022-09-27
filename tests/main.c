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

#include "levenshtein.h"

int main()
{
	char const * bob = "bob";
	char const * bib = "bib";
	printf("%d\n", lev_string_dist(bob, bib));

	char const * chiens = "chiens";
	char const * niche = "niche";
	printf("%d\n", lev_string_dist(chiens, niche));

	char const * test = "test";
	char const * test2 = "test";
	printf("%d\n", lev_string_dist(test, test));
	printf("%d\n", lev_string_dist(test, test2));

	char const * _short = "a very long string";
	char const * _long = "short";
	printf("%d\n", lev_string_dist(_short, _long));

	char const * ok = "ok";
	char const * empty = "";
	printf("%d\n", lev_string_dist(empty, ok));
	printf("%d\n", lev_string_dist(ok, empty));
	printf("%d\n", lev_string_dist(empty, empty));

	return 0;
}
