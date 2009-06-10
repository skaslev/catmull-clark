#ifndef MESH_H
#define MESH_H

struct mesh;

struct mesh *mesh_create(void);
void mesh_destroy(struct mesh *mesh);

void mesh_add_vertex(struct mesh *mesh, const struct vec *v);
void mesh_add_normal(struct mesh *mesh, const struct vec *n);
void mesh_begin_face(struct mesh *mesh);
void mesh_add_index(struct mesh *mesh, int vi, int ni);
void mesh_end_face(struct mesh *mesh);

int mesh_face_count(const struct mesh *mesh);
int mesh_face_vertex_count(const struct mesh *mesh, int face);
struct vec *mesh_get_vertex(const struct mesh *mesh, int face, int vert);
struct vec *mesh_get_normal(const struct mesh *mesh, int face, int vert);

void mesh_compute_normals(struct mesh *mesh);

void mesh_calc_bounds(const struct mesh *mesh,
		      struct vec *min, struct vec *max);

void mesh_render(const struct mesh *mesh);

#endif
