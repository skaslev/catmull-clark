#ifndef MESH_H
#define MESH_H

struct mesh;

struct mesh *mesh_read_obj(const char *file);

struct mesh *mesh_create();
void mesh_destroy(struct mesh *mesh);

void mesh_add_vertex(struct mesh *mesh, const struct vec *v);
void mesh_add_normal(struct mesh *mesh, const struct vec *n);
void mesh_begin_face(struct mesh *mesh);
void mesh_add_index(struct mesh *mesh, int vi, int ni);
void mesh_end_face(struct mesh *mesh);
void mesh_compute_normals(struct mesh *mesh);

void mesh_calc_bounds(const struct mesh *mesh,
		      struct vec *min, struct vec *max);

void mesh_render(const struct mesh *mesh);

#endif
