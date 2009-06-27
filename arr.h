#ifndef ARR_H
#define ARR_H

#include <stdlib.h>

#define arr_type(type)						\
	struct {						\
		int nr, alloc;					\
		type *elts;					\
	}

#define arr_def(type, name)	arr_type(type) name

#define arr_init(arr)		arr_init3(arr, 0, 8)

#define arr_init2(arr, nr)	arr_init3(arr, nr, nr)

#define arr_init3(arr, _nr, sz)					\
	do {							\
		(arr).nr = (_nr);				\
		(arr).alloc = (sz) < (_nr) ? (_nr) : (sz);	\
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

#define arr_resize(arr, _nr)					\
	do {							\
		arr_reserve(arr, _nr);				\
		(arr).nr = (_nr);				\
	} while (0)

#define arr_size(arr)			((arr).nr)

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

#define arr_idx(arr, it)	((it) - (arr).elts)

#endif
