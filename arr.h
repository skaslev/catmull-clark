#ifndef ARR_H
#define ARR_H

#include <stdlib.h>

#define ARR_DEF(type, name)					\
	struct {						\
		int nr, alloc;					\
		type *elts;					\
	} name

#define ARR_INIT(arr)						\
	do {							\
		(arr).nr = 0;					\
		(arr).alloc = 8;				\
		(arr).elts = malloc((arr).alloc * sizeof(*(arr).elts)); \
	} while (0)

#define ARR_CLEAR(arr)						\
	do {							\
		(arr).nr = (arr).alloc = 0;			\
		free((arr).elts);				\
	} while (0)

#define ARR_SIZE(arr)			((arr).nr)

#define ARR_ELTS(arr)			((arr).elts)

#define ARR_AT(arr, idx)		((arr).elts[idx])

#define ARR_PUSH(arr, elt)					\
	do {							\
		if ((arr).nr == (arr).alloc) {			\
			(arr).alloc *= 2;			\
			(arr).elts = realloc((arr).elts,	\
					     (arr).alloc * sizeof(*(arr).elts)); \
		}						\
		(arr).elts[(arr).nr++] = elt;			\
	} while (0)

#endif
