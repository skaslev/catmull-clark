#ifndef UTIL_H
#define UTIL_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define MAX(a, b)	((a) > (b) ? (a) : (b))

#define SWAP(a, b)				\
	do {					\
		typeof(a) __tmp = (a);		\
		(a) = (b);			\
		(b) = __tmp;			\
	} while (0)

#endif
