#include <math.h>
#include <GL/gl.h>
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

void mesh_compute_normals(struct mesh *mesh)
{
	int i, j;

	ARR_RESIZE(mesh->nbuf, ARR_SIZE(mesh->vbuf));
	memset(ARR_ELTS(mesh->nbuf), 0,
	       ARR_SIZE(mesh->nbuf) * sizeof(*ARR_ELTS(mesh->nbuf)));

	for (i = 0; i < ARR_SIZE(mesh->faces); i++) {
		int off, len;

		off = ARR_AT(mesh->faces, i);
		len = i != ARR_SIZE(mesh->faces) - 1 ?
			ARR_AT(mesh->faces, i + 1) - off :
			ARR_SIZE(mesh->ibuf) - off;

		for (j = 0; j < len; j++) {
			int idx0, idx1, idx2;
			const struct vec *v0, *v1, *v2;
			struct vec u, v, n;
			struct vec *vn;

			idx0 = ARR_AT(mesh->ibuf, off + j).vi;
			idx1 = ARR_AT(mesh->ibuf, off + (j + 1) % len).vi;
			idx2 = ARR_AT(mesh->ibuf, off + (j + len - 1) % len).vi;
			v0 = &ARR_AT(mesh->vbuf, idx0);
			v1 = &ARR_AT(mesh->vbuf, idx1);
			v2 = &ARR_AT(mesh->vbuf, idx2);

			vec_sub(&u, v1, v0);
			vec_sub(&v, v2, v0);
			vec_prod(&n, &u, &v);
			vec_normalize(&n);

			vn = &ARR_AT(mesh->nbuf, idx0);
			vec_add(vn, vn, &n);
		}
	}

	for (i = 0; i < ARR_SIZE(mesh->nbuf); i++)
		vec_normalize(&ARR_AT(mesh->nbuf, i));

	for (i = 0; i < ARR_SIZE(mesh->ibuf); i++) {
		struct idx *idx = &ARR_AT(mesh->ibuf, i);
		idx->ni = idx->vi;
	}
}

void mesh_calc_bounds(const struct mesh *mesh,
		      struct vec *min, struct vec *max)
{
	int i;

	*min = (struct vec) {  INFINITY,  INFINITY,  INFINITY };
	*max = (struct vec) { -INFINITY, -INFINITY, -INFINITY };
	for (i = 0; i < ARR_SIZE(mesh->vbuf); i++) {
		struct vec *v = &ARR_AT(mesh->vbuf, i);
		vec_min(min, min, v);
		vec_max(max, max, v);
	}
}

void mesh_render(const struct mesh *mesh)
{
	int i, j;

	for (i = 0; i < ARR_SIZE(mesh->faces); i++) {
		int off, len;

		off = ARR_AT(mesh->faces, i);
		len = i != ARR_SIZE(mesh->faces) - 1 ?
			ARR_AT(mesh->faces, i + 1) - off :
			ARR_SIZE(mesh->ibuf) - off;

		glBegin(GL_POLYGON);
		for (j = 0; j < len; j++) {
			struct idx *idx = &ARR_AT(mesh->ibuf, off + j);
			struct vec *v;

			if (idx->ni != -1) {
				v = &ARR_AT(mesh->nbuf, idx->ni);
				glNormal3f(v->x, v->y, v->z);
			}

			v = &ARR_AT(mesh->vbuf, idx->vi);
			glVertex3f(v->x, v->y, v->z);
		}
		glEnd();
	}
}
