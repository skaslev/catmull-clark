#include "buf.h"

void buf_do_realloc_(void **a, size_t nr, size_t sz)
{
	size_t *p = realloc(*a ? buf_raw_(*a) : NULL, 2 * sizeof(size_t) + nr * sz);
	p[0] = nr;
	if (!*a)
		p[1] = 0;
	*a = p + 2;
}
