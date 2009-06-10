#include "arr.h"
#include "geometry.h"
#include "mesh.h"

struct idx {
	int vi, ni;
};

struct mesh {
	ARR_DEF(struct vec, vbuf);
	ARR_DEF(struct vec, nbuf);
	ARR_DEF(struct idx, ibuf);
	ARR_DEF(int, faces);
};

struct mesh *mesh_create(void)
{
	struct mesh *mesh = malloc(sizeof(*mesh));
	ARR_INIT(mesh->vbuf);
	ARR_INIT(mesh->nbuf);
	ARR_INIT(mesh->ibuf);
	ARR_INIT(mesh->faces);
	return mesh;
}

void mesh_destroy(struct mesh *mesh)
{
	ARR_CLEAR(mesh->vbuf);
	ARR_CLEAR(mesh->nbuf);
	ARR_CLEAR(mesh->ibuf);
	ARR_CLEAR(mesh->faces);
	free(mesh);
}

void mesh_add_vertex(struct mesh *mesh, const struct vec *v)
{
	ARR_PUSH(mesh->vbuf, *v);
}

void mesh_add_normal(struct mesh *mesh, const struct vec *n)
{
	ARR_PUSH(mesh->nbuf, *n);
}

void mesh_begin_face(struct mesh *mesh)
{
	ARR_PUSH(mesh->faces, ARR_SIZE(mesh->ibuf));
}

void mesh_add_index(struct mesh *mesh, int vi, int ni)
{
	ARR_PUSH(mesh->ibuf, ((struct idx) { vi, ni }));
}

void mesh_end_face(struct mesh *mesh)
{
	/* noop */
}

int mesh_vertex_count(const struct mesh *mesh)
{
	return ARR_SIZE(mesh->vbuf);
}

int mesh_normal_count(const struct mesh *mesh)
{
	return ARR_SIZE(mesh->nbuf);
}

const struct vec *mesh_vertex_buffer(const struct mesh *mesh)
{
	return ARR_ELTS(mesh->vbuf);
}

const struct vec *mesh_normal_buffer(const struct mesh *mesh)
{
	return ARR_ELTS(mesh->nbuf);
}

int mesh_face_count(const struct mesh *mesh)
{
	return ARR_SIZE(mesh->faces);
}

int mesh_face_vertex_count(const struct mesh *mesh, int face)
{
	int beg, end;

	beg = ARR_AT(mesh->faces, face);
	end = face != ARR_SIZE(mesh->faces) - 1 ?
		ARR_AT(mesh->faces, face + 1) : ARR_SIZE(mesh->ibuf);
	return end - beg;
}

void mesh_face_vertex_index(const struct mesh *mesh, int face, int vert,
			    int *vertex_idx, int *normal_idx)
{
	int beg;
	struct idx *idx;

	beg = ARR_AT(mesh->faces, face);
	idx = &ARR_AT(mesh->ibuf, beg + vert);
	*vertex_idx = idx->vi;
	*normal_idx = idx->ni;
}

struct vec *mesh_get_vertex(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return &ARR_AT(mesh->vbuf, vi);
}

struct vec *mesh_get_normal(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return ni != -1 ? &ARR_AT(mesh->nbuf, ni) : NULL;
}

void mesh_compute_normals(struct mesh *mesh)
{
	int i, faces;

	ARR_RESIZE(mesh->nbuf, ARR_SIZE(mesh->vbuf));
	memset(ARR_ELTS(mesh->nbuf), 0,
	       ARR_SIZE(mesh->nbuf) * sizeof(*ARR_ELTS(mesh->nbuf)));

	for (i = 0; i < ARR_SIZE(mesh->ibuf); i++) {
		struct idx *idx = &ARR_AT(mesh->ibuf, i);
		idx->ni = idx->vi;
	}

	faces = mesh_face_count(mesh);
	for (i = 0; i < faces; i++) {
		int j, verts;

		verts = mesh_face_vertex_count(mesh, i);
		for (j = 0; j < verts; j++) {
			const struct vec *v0, *v1, *v2;
			struct vec u, v, n;
			struct vec *vn;

			v0 = mesh_get_vertex(mesh, i, j);
			v1 = mesh_get_vertex(mesh, i, (j + 1) % verts);
			v2 = mesh_get_vertex(mesh, i, (j + verts - 1) % verts);

			vec_sub(&u, v1, v0);
			vec_sub(&v, v2, v0);
			vec_prod(&n, &u, &v);
			vec_normalize(&n);

			vn = mesh_get_normal(mesh, i, j);
			vec_add(vn, vn, &n);
		}
	}

	for (i = 0; i < ARR_SIZE(mesh->nbuf); i++)
		vec_normalize(&ARR_AT(mesh->nbuf, i));
}
