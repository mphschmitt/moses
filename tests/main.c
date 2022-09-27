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
