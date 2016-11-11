#ifndef BUF_H
#define BUF_H

#include <stdlib.h>

/* Based on Sean Barrett's stretchy buffer at http://www.nothings.org/stb/stretchy_buffer.txt
 * init: NULL, free: buf_free(), push_back: buf_push(), size: buf_len()
 */
#define buf_len(a)		((a) ? buf_n_(a) : 0)
#define buf_push(a, v)		(buf_maybegrow1_(a), (a)[buf_n_(a)++] = (v))
#define buf_last(a)		((a)[buf_n_(a) - 1])
#define buf_resize(a, n)	(buf_maybegrow_(a, n), (a) ? buf_n_(a) = (n) : 0)
#define buf_reserve(a, n)	(buf_maybegrow_(a, n))
#define buf_free(a)		((a) ? free(buf_raw_(a)) : (void) 0)
#define buf_foreach(it, a)	for ((it) = (a); (it) < (a) + buf_len(a); (it)++)

/* Private */
#define buf_raw_(a)		((size_t *) (a) - 2)
#define buf_m_(a)		(buf_raw_(a)[0])
#define buf_n_(a)		(buf_raw_(a)[1])

#define buf_maybegrow_(a, n)	(((n) > 0) && (!(a) || (n) >= buf_m_(a)) ? buf_realloc_(a, n) : (void) 0)
#define buf_maybegrow1_(a)	(!(a) || buf_m_(a) == 0 ? buf_realloc_(a, 8) : \
				 buf_n_(a) == buf_m_(a) ? buf_realloc_(a, 3 * buf_m_(a) / 2) : (void) 0)
#define buf_realloc_(a, n)	buf_do_realloc_((void **) &(a), n, sizeof(*(a)))

void buf_do_realloc_(void **a, size_t nr, size_t sz);

#endif
