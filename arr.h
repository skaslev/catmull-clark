#ifndef ARR_H
#define ARR_H

#include <stdlib.h>

#define arr_def(type, name)					\
	struct {						\
		int nr, alloc;					\
		type *elts;					\
	} name

#define arr_init(arr)						\
	do {							\
		(arr).nr = 0;					\
		(arr).alloc = 8;				\
		(arr).elts = malloc((arr).alloc * sizeof(*(arr).elts)); \
	} while (0)

#define arr_init2(arr, sz)					\
	do {							\
		(arr).nr = (arr).alloc = (sz);			\
		(arr).elts = malloc((arr).alloc * sizeof(*(arr).elts)); \
	} while (0)

#define arr_clear(arr)						\
	do {							\
		(arr).nr = (arr).alloc = 0;			\
		free((arr).elts);				\
		(arr).elts = NULL;				\
	} while (0)

#define arr_reserve(arr, sz)					\
	do {							\
		if ((sz) > (arr).alloc) {			\
			(arr).alloc = (sz);			\
			(arr).elts = realloc((arr).elts,	\
					     (arr).alloc * sizeof(*(arr).elts)); \
		}						\
	} while (0)

#define arr_resize(arr, sz)					\
	do {							\
		arr_reserve(arr, sz);				\
		(arr).nr = (sz);				\
	} while (0)


#define arr_size(arr)			((arr).nr)

#define arr_elts(arr)			((arr).elts)

#define arr_at(arr, idx)		((arr).elts[idx])

#define arr_first(arr)			((arr).elts[0])

#define arr_last(arr)			((arr).elts[(arr).nr-1])

#define arr_push(arr, elt)					\
	do {							\
		if ((arr).nr == (arr).alloc)			\
			arr_reserve(arr, 2 * (arr).alloc);	\
		(arr).elts[(arr).nr++] = elt;			\
	} while (0)

#define arr_foreach(it, arr)					\
	for (typeof((arr).elts) it = (arr).elts;		\
	     it < (arr).elts + (arr).nr; it++)

#endif
