#ifndef SUBD_H
#define SUBD_H

struct mesh *subdivide(const struct mesh *mesh, int iterations);
void subdivide_levels(const struct mesh *mesh,
		      struct mesh **levels, int nr_levels);

#endif
