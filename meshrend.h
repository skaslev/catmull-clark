#ifndef MESHREND_H

void mesh_render(const struct mesh *mesh);
void mesh_calc_bounds(const struct mesh *mesh,
		      struct vec *min, struct vec *max);

#endif
