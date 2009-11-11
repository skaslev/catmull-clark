#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "geometry.h"
#include "mesh.h"

struct idx {
	int vi, ni;
};

struct mesh {
	struct vec *vbuf;
	struct vec *nbuf;
	struct idx *ibuf;
	int *faces;
};

struct mesh *mesh_create(void)
{
	struct mesh *mesh = malloc(sizeof(*mesh));
	mesh->vbuf = NULL;
	mesh->nbuf = NULL;
	mesh->ibuf = NULL;
	mesh->faces = NULL;
	return mesh;
}

void mesh_destroy(struct mesh *mesh)
{
	if (!mesh)
		return;
	buf_free(mesh->vbuf);
	buf_free(mesh->nbuf);
	buf_free(mesh->ibuf);
	buf_free(mesh->faces);
	free(mesh);
}

void mesh_add_vertex(struct mesh *mesh, const struct vec *v)
{
	buf_push(mesh->vbuf, *v);
}

void mesh_add_normal(struct mesh *mesh, const struct vec *n)
{
	buf_push(mesh->nbuf, *n);
}

void mesh_begin_face(struct mesh *mesh)
{
	buf_push(mesh->faces, buf_len(mesh->ibuf));
}

void mesh_add_index(struct mesh *mesh, int vi, int ni)
{
	struct idx idx;
	idx.vi = vi;
	idx.ni = ni;
	buf_push(mesh->ibuf, idx);
}

void mesh_end_face(struct mesh *mesh)
{
	/* noop */
}

int mesh_vertex_buffer(const struct mesh *mesh, const struct vec **buf)
{
	if (buf)
		*buf = mesh->vbuf;
	return buf_len(mesh->vbuf);
}

int mesh_normal_buffer(const struct mesh *mesh, const struct vec **buf)
{
	if (buf)
		*buf = mesh->nbuf;
	return buf_len(mesh->nbuf);
}

int mesh_face_count(const struct mesh *mesh)
{
	return buf_len(mesh->faces);
}

int mesh_face_vertex_count(const struct mesh *mesh, int face)
{
	int beg, end;

	beg = mesh->faces[face];
	end = face != buf_len(mesh->faces) - 1 ?
		mesh->faces[face + 1] : buf_len(mesh->ibuf);
	return end - beg;
}

void mesh_face_vertex_index(const struct mesh *mesh, int face, int vert,
			    int *vertex_idx, int *normal_idx)
{
	int beg;
	struct idx *idx;

	beg = mesh->faces[face];
	idx = &mesh->ibuf[beg + vert];
	*vertex_idx = idx->vi;
	*normal_idx = idx->ni;
}

struct vec *mesh_get_vertex(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return &mesh->vbuf[vi];
}

struct vec *mesh_get_normal(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return ni != -1 ? &mesh->nbuf[ni] : NULL;
}

void mesh_compute_normals(struct mesh *mesh)
{
	int i, nr_faces;
	struct idx *idx;
	struct vec *n;

	buf_resize(mesh->nbuf, buf_len(mesh->vbuf));
	memset(mesh->nbuf, 0, buf_len(mesh->nbuf) * sizeof(*mesh->nbuf));

	buf_foreach(idx, mesh->ibuf)
		idx->ni = idx->vi;

	nr_faces = mesh_face_count(mesh);
	for (i = 0; i < nr_faces; i++) {
		int j, nr_verts;

		nr_verts = mesh_face_vertex_count(mesh, i);
		for (j = 0; j < nr_verts; j++) {
			const struct vec *v0, *v1, *v2;
			struct vec u, v, n;
			struct vec *vn;

			v0 = mesh_get_vertex(mesh, i, j);
			v1 = mesh_get_vertex(mesh, i, (j + 1) % nr_verts);
			v2 = mesh_get_vertex(mesh, i, (j + nr_verts - 1) % nr_verts);

			vec_sub(&u, v1, v0);
			vec_sub(&v, v2, v0);
			vec_prod(&n, &u, &v);
			vec_normalize(&n);

			vn = mesh_get_normal(mesh, i, j);
			vec_add(vn, vn, &n);
		}
	}

	buf_foreach(n, mesh->nbuf)
		vec_normalize(n);
}
