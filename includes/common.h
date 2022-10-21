#ifndef __COMMON_H_
#define __COMMON_H_

#define MIN_DISTANCE 70.0
#define MAX_HAYSTACKS 100

struct args
{
	char * needle;
	char * haystacks[MAX_HAYSTACKS];
	double min_distance;
	int verbose;
};


#endif /* __COMMON_H_ */
