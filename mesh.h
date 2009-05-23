#ifndef MESH_H
#define MESH_H

#include "arr.h"

struct mesh {
	ARR_DEF(struct vec, verts);
	ARR_DEF(int, idxs);
};

struct mesh *mesh_create();
void mesh_destroy(struct mesh *m);

struct mesh *mesh_read_obj(const char *file);

void mesh_add_vertex(struct mesh *m, const struct vec *vert);
void mesh_begin_face(struct mesh *m);
void mesh_add_index(struct mesh *m, int idx);
void mesh_end_face(struct mesh *m);

void mesh_calc_bounds(const struct mesh *m,
		      struct vec *min, struct vec *max);

void mesh_render(const struct mesh *m);

#endif
