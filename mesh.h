#ifndef MESH_H
#define MESH_H

struct mesh;

/*
 * Mesh construction
 */
struct mesh *mesh_create(void);
void mesh_destroy(struct mesh *mesh);

void mesh_add_vertex(struct mesh *mesh, const struct vec *v);
void mesh_add_normal(struct mesh *mesh, const struct vec *n);
void mesh_begin_face(struct mesh *mesh);
void mesh_add_index(struct mesh *mesh, int vi, int ni);
void mesh_end_face(struct mesh *mesh);
void mesh_compute_normals(struct mesh *mesh);

/*
 * Vertex buffer access
 */
int mesh_vertex_buffer(const struct mesh *mesh, const struct vec **buf);
int mesh_normal_buffer(const struct mesh *mesh, const struct vec **buf);

/*
 * Face access
 */
int mesh_face_count(const struct mesh *mesh);
int mesh_face_vertex_count(const struct mesh *mesh, int face);
void mesh_face_vertex_index(const struct mesh *mesh, int face, int vert,
			    int *vertex_idx, int *normal_idx);

struct vec *mesh_get_vertex(const struct mesh *mesh, int face, int vert);
struct vec *mesh_get_normal(const struct mesh *mesh, int face, int vert);

#endif
