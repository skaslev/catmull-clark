#include "arr.h"
#include "geometry.h"
#include "mesh.h"

struct idx {
	int vi, ni;
};

struct mesh {
	arr_def(struct vec, vbuf);
	arr_def(struct vec, nbuf);
	arr_def(struct idx, ibuf);
	arr_def(int, faces);
};

struct mesh *mesh_create(void)
{
	struct mesh *mesh = malloc(sizeof(*mesh));
	arr_init(mesh->vbuf);
	arr_init(mesh->nbuf);
	arr_init(mesh->ibuf);
	arr_init(mesh->faces);
	return mesh;
}

void mesh_destroy(struct mesh *mesh)
{
	if (!mesh)
		return;
	arr_clear(mesh->vbuf);
	arr_clear(mesh->nbuf);
	arr_clear(mesh->ibuf);
	arr_clear(mesh->faces);
	free(mesh);
}

void mesh_add_vertex(struct mesh *mesh, const struct vec *v)
{
	arr_push(mesh->vbuf, *v);
}

void mesh_add_normal(struct mesh *mesh, const struct vec *n)
{
	arr_push(mesh->nbuf, *n);
}

void mesh_begin_face(struct mesh *mesh)
{
	arr_push(mesh->faces, arr_size(mesh->ibuf));
}

void mesh_add_index(struct mesh *mesh, int vi, int ni)
{
	arr_push(mesh->ibuf, ((struct idx) { vi, ni }));
}

void mesh_end_face(struct mesh *mesh)
{
	/* noop */
}

int mesh_vertex_buffer(const struct mesh *mesh, const struct vec **buf)
{
	if (buf)
		*buf = &arr_first(mesh->vbuf);
	return arr_size(mesh->vbuf);
}

int mesh_normal_buffer(const struct mesh *mesh, const struct vec **buf)
{
	if (buf)
		*buf = &arr_first(mesh->nbuf);
	return arr_size(mesh->nbuf);
}

int mesh_face_count(const struct mesh *mesh)
{
	return arr_size(mesh->faces);
}

int mesh_face_vertex_count(const struct mesh *mesh, int face)
{
	int beg, end;

	beg = arr_at(mesh->faces, face);
	end = face != arr_size(mesh->faces) - 1 ?
		arr_at(mesh->faces, face + 1) : arr_size(mesh->ibuf);
	return end - beg;
}

void mesh_face_vertex_index(const struct mesh *mesh, int face, int vert,
			    int *vertex_idx, int *normal_idx)
{
	int beg;
	struct idx *idx;

	beg = arr_at(mesh->faces, face);
	idx = &arr_at(mesh->ibuf, beg + vert);
	*vertex_idx = idx->vi;
	*normal_idx = idx->ni;
}

struct vec *mesh_get_vertex(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return &arr_at(mesh->vbuf, vi);
}

struct vec *mesh_get_normal(const struct mesh *mesh, int face, int vert)
{
	int vi, ni;

	mesh_face_vertex_index(mesh, face, vert, &vi, &ni);
	return ni != -1 ? &arr_at(mesh->nbuf, ni) : NULL;
}

void mesh_compute_normals(struct mesh *mesh)
{
	int i, nr_faces;

	arr_resize(mesh->nbuf, arr_size(mesh->vbuf));
	memset(&arr_first(mesh->nbuf), 0,
	       arr_size(mesh->nbuf) * sizeof(arr_first(mesh->nbuf)));

	arr_foreach(idx, mesh->ibuf)
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

	arr_foreach(n, mesh->nbuf)
		vec_normalize(n);
}
