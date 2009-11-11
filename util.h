#ifndef UTIL_H
#define UTIL_H

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define SQ(a)		((a) * (a))

#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define MAX(a, b)	((a) > (b) ? (a) : (b))

#define SWAP(type, a, b)	do { type _tmp = (a);\
				     (a) = (b);\
				     (b) = _tmp; } while (0)

#endif
