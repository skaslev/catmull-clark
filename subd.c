#include <assert.h>
#include "arr.h"
#include "geometry.h"
#include "mesh.h"
#include "util.h"

struct sd_vert {
	struct vec p, newp;
	arr_type(int) es;
	arr_type(int) fs;
};

struct sd_face {
	arr_type(int) vs;
	int fvert;
};

struct sd_edge {
	int v0, v1;
	int f0, f1;
	int evert;
};

struct sd_mesh {
	int first_iteration;
	arr_type(struct sd_vert) verts;
	arr_type(struct sd_face) faces;
	arr_type(struct sd_edge) edges;
};

#define sd_v(vi)		(arr_at(sd->verts, (vi)))
#define sd_f(fi)		(arr_at(sd->faces, (fi)))
#define sd_e(ei)		(arr_at(sd->edges, (ei)))
#define sd_vi(v)		(arr_idx(sd->verts, (v)))
#define sd_fi(f)		(arr_idx(sd->faces, (f)))
#define sd_ei(e)		(arr_idx(sd->edges, (e)))

static int sd_find_edge(struct sd_mesh *sd, int v0, int v1)
{
	struct sd_vert *v;

	v = &sd_v(v0);
	arr_foreach(ei, v->es) {
		struct sd_edge *e = &sd_e(*ei);

		if ((e->v0 == v0 && e->v1 == v1) ||
		    (e->v0 == v1 && e->v1 == v0))
			return *ei;
	}
	return -1;
}

static inline struct sd_vert *
sd_edge_other(struct sd_mesh *sd, struct sd_edge *e, struct sd_vert *v)
{
	int vi;

	vi = sd_vi(v);
	assert(e->v0 == vi || e->v1 == vi);
	return &sd_v(e->v0 == vi ? e->v1 : e->v0);
}

static void sd_update_links(struct sd_mesh *sd)
{
	arr_foreach(v, sd->verts) {
		arr_resize(v->fs, 0);
		arr_resize(v->es, 0);
	}

	arr_resize(sd->edges, 0);
	arr_foreach(f, sd->faces) {
		for (int j = 0; j < arr_size(f->vs); j++) {
			int v0, v1, ei;

			v0 = arr_at(f->vs, j);
			v1 = arr_at(f->vs, (j+1) % arr_size(f->vs));
			arr_push(sd_v(v0).fs, sd_fi(f));
			ei = sd_find_edge(sd, v0, v1);
			if (ei == -1) {
				struct sd_edge edge = {
					.v0 = v0,
					.v1 = v1,
					.f0 = sd_fi(f),
					.f1 = -1,
					.evert = -1
				};

				arr_push(sd->edges, edge);
				ei = arr_size(sd->edges) - 1;
				arr_push(sd_v(v0).es, ei);
				arr_push(sd_v(v1).es, ei);
			} else {
				assert(sd_e(ei).f1 == -1);
				sd_e(ei).f1 = sd_fi(f);
			}
		}
	}
}

static struct sd_mesh *sd_init(const struct mesh *mesh)
{
	int i, j, nr_verts, nr_faces;
	const struct vec *vbuf;
	struct sd_mesh *sd;

	sd = malloc(sizeof(struct sd_mesh));
	sd->first_iteration = 1;

	/* Create vertices */
	nr_verts = mesh_vertex_buffer(mesh, &vbuf);
	arr_init2(sd->verts, nr_verts);
	arr_foreach(v, sd->verts) {
		v->p = *vbuf++;
		arr_init(v->es);
		arr_init(v->fs);
	}

	/* Create faces */
	nr_faces = mesh_face_count(mesh);
	arr_init2(sd->faces, nr_faces);
	for (i = 0; i < nr_faces; i++) {
		struct sd_face *f;

		f = &sd_f(i);
		f->fvert = -1;

		nr_verts = mesh_face_vertex_count(mesh, i);
		arr_init3(f->vs, 0, nr_verts);
		for (j = 0; j < nr_verts; j++) {
			int vidx, nidx;

			mesh_face_vertex_index(mesh, i, j, &vidx, &nidx);
			arr_push(f->vs, vidx);
		}
	}

	/* Create edges */
	arr_init(sd->edges);
	sd_update_links(sd);

	return sd;
}

static void sd_destroy(struct sd_mesh *sd)
{
	arr_foreach(v, sd->verts) {
		arr_clear(v->es);
		arr_clear(v->fs);
	}
	arr_clear(sd->verts);

	arr_foreach(f, sd->faces)
		arr_clear(f->vs);
	arr_clear(sd->faces);

	arr_clear(sd->edges);

	free(sd);
}

static int sd_add_vert(struct sd_mesh *sd, const struct vec *p)
{
	struct sd_vert v;
	v.p = *p;
	arr_init(v.es);
	arr_init(v.fs);
	arr_push(sd->verts, v);
	return arr_size(sd->verts) - 1;
}

static void sd_do_iteration(struct sd_mesh *sd, int last_iteration)
{
	int V, F, E, Vn, Fn, En;
	typeof(sd->faces) faces;

	/* V' = V + F + E
	 * F' = Sum_i=0^F(f_i), (F' = 4F, when quad-mesh)
	 * E' = 2E + F'
	 */
	V = arr_size(sd->verts);
	F = arr_size(sd->faces);
	E = arr_size(sd->edges);
	Vn = V + F + E;
	if (sd->first_iteration) {
		Fn = 0;
		arr_foreach(f, sd->faces)
			Fn += arr_size(f->vs);
		sd->first_iteration = 0;
	} else {
		/* After the first iteration all faces are quads */
		Fn = 4 * F;
	}
	En = 2 * E + Fn;

	/* 1. Update vertices */
	arr_reserve(sd->verts, Vn);

	/* Create face vertices */
	arr_foreach(f, sd->faces) {
		struct vec p = vec_null;
		arr_foreach(vi, f->vs)
			vec_add(&p, &p, &sd_v(*vi).p);
		vec_div(&p, (float) arr_size(f->vs));
		f->fvert = sd_add_vert(sd, &p);
	}

	/* Create edge vertices */
	arr_foreach(e, sd->edges) {
		struct vec p = vec_null;

		assert(e->f1 != -1);
		vec_add(&p, &p, &sd_v(e->v0).p);
		vec_add(&p, &p, &sd_v(e->v1).p);
		vec_add(&p, &p, &sd_v(sd_f(e->f0).fvert).p);
		vec_add(&p, &p, &sd_v(sd_f(e->f1).fvert).p);
		vec_div(&p, 4.0f);
		e->evert = sd_add_vert(sd, &p);
	}

	/* Move old vertices */
	arr_foreach(v, sd->verts) {
		int n;
		struct vec p;

		if (sd_vi(v) >= V)
			break;

		assert(arr_size(v->fs) == arr_size(v->es));
		n = arr_size(v->fs);

		v->newp = v->p;
		vec_mul(&v->newp, (float) (n - 2) / n);

		p = vec_null;
		arr_foreach(fi, v->fs)
			vec_add(&p, &p, &sd_v(sd_f(*fi).fvert).p);
		vec_mad(&v->newp, 1.0f / (n * n), &p);
		
		p = vec_null;
		arr_foreach(ei, v->es)
			vec_add(&p, &p, &sd_edge_other(sd, &sd_e(*ei), v)->p);
		vec_mad(&v->newp, 1.0f / (n * n), &p);
	}

	arr_foreach(v, sd->verts) {
		if (sd_vi(v) >= V)
			break;
		v->p = v->newp;
	}

	/* 2. Create new faces */
	arr_init3(faces, 0, Fn);
	arr_foreach(f, sd->faces) {
		for (int j = 0; j < arr_size(f->vs); j++) {
			int v0, v, v1;
			struct sd_edge *e0, *e1;
			struct sd_face new_face;

			v0 = arr_at(f->vs, (j - 1 + arr_size(f->vs)) % arr_size(f->vs));
			v  = arr_at(f->vs, j);
			v1 = arr_at(f->vs, (j + 1) % arr_size(f->vs));
			e0 = &sd_e(sd_find_edge(sd, v0, v));
			e1 = &sd_e(sd_find_edge(sd, v, v1));

			arr_init3(new_face.vs, 0, 4);
			arr_push(new_face.vs, e0->evert);
			arr_push(new_face.vs, v);
			arr_push(new_face.vs, e1->evert);
			arr_push(new_face.vs, f->fvert);
			new_face.fvert = -1;

			arr_push(faces, new_face);
		}
	}
	SWAP(sd->faces, faces);
	arr_clear(faces);

	/* 3. Update edges */
	if (!last_iteration) {	/* Skip on last iteration */
		arr_reserve(sd->edges, En);
		sd_update_links(sd);
	}
}

static struct mesh *sd_convert(struct sd_mesh *sd)
{
	struct mesh *mesh;

	mesh = mesh_create();
	arr_foreach(v, sd->verts)
		mesh_add_vertex(mesh, &v->p);
	arr_foreach(f, sd->faces) {
		mesh_begin_face(mesh);
		arr_foreach(vi, f->vs)
			mesh_add_index(mesh, *vi, -1);
		mesh_end_face(mesh);
	}
	mesh_compute_normals(mesh);
	return mesh;
}

struct mesh *subdivide(const struct mesh *mesh, int iterations)
{
	struct sd_mesh *sd;
	struct mesh *ret;

	sd = sd_init(mesh);
	while (iterations--)
		sd_do_iteration(sd, iterations == 0);
	ret = sd_convert(sd);
	sd_destroy(sd);
	return ret;
}

void subdivide_levels(const struct mesh *mesh,
		      struct mesh **levels, int nr_levels)
{
	int i;
	struct sd_mesh *sd;

	sd = sd_init(mesh);
	for (i = 0; i < nr_levels; i++) {
		sd_do_iteration(sd, i == nr_levels - 1);
		levels[i] = sd_convert(sd);
	}
	sd_destroy(sd);
}
