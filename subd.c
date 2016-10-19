#include <assert.h>
#include <stdlib.h>
#include "buf.h"
#include "mathx.h"
#include "mesh.h"
#include "util.h"

struct sd_vert {
	vector p, newp;
	int *es;
	int *fs;
};

struct sd_face {
	int *vs;
	int fvert;
};

struct sd_edge {
	int v0, v1;
	int f0, f1;
	int evert;
};

struct sd_mesh {
	struct sd_vert *verts;
	struct sd_face *faces;
	struct sd_edge *edges;
};

#define sd_v(vi)		(sd->verts[vi])
#define sd_f(fi)		(sd->faces[fi])
#define sd_e(ei)		(sd->edges[ei])
#define sd_vi(v)		((int)((v) - sd->verts))
#define sd_fi(f)		((int)((f) - sd->faces))
#define sd_ei(e)		((int)((e) - sd->edges))

static int sd_find_edge(struct sd_mesh *sd, int v0, int v1)
{
	int *ei;
	struct sd_vert *v;

	v = &sd_v(v0);
	buf_foreach(ei, v->es) {
		struct sd_edge *e = &sd_e(*ei);

		if ((e->v0 == v0 && e->v1 == v1) ||
		    (e->v0 == v1 && e->v1 == v0))
			return *ei;
	}
	return -1;
}

static struct sd_vert *
sd_edge_other(struct sd_mesh *sd, struct sd_edge *e, struct sd_vert *v)
{
	int vi;

	vi = sd_vi(v);
	assert(e->v0 == vi || e->v1 == vi);
	return &sd_v(e->v0 == vi ? e->v1 : e->v0);
}

static void sd_update_links(struct sd_mesh *sd)
{
	struct sd_vert *v;
	struct sd_face *f;

	buf_foreach(v, sd->verts) {
		buf_resize(v->fs, 0);
		buf_resize(v->es, 0);
	}

	buf_resize(sd->edges, 0);
	buf_foreach(f, sd->faces) {
		int j;
		for (j = 0; j < buf_len(f->vs); j++) {
			int v0, v1, ei;

			v0 = f->vs[j];
			v1 = f->vs[(j+1) % buf_len(f->vs)];
			buf_push(sd_v(v0).fs, sd_fi(f));
			ei = sd_find_edge(sd, v0, v1);
			if (ei == -1) {
				struct sd_edge edge;
				edge.v0 = v0;
				edge.v1 = v1;
				edge.f0 = sd_fi(f);
				edge.f1 = -1;
				edge.evert = -1;

				buf_push(sd->edges, edge);
				ei = buf_len(sd->edges) - 1;
				buf_push(sd_v(v0).es, ei);
				buf_push(sd_v(v1).es, ei);
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
	const float *vbuf;
	struct sd_vert *v;
	struct sd_mesh *sd;

	sd = malloc(sizeof(*sd));
	sd->verts = NULL;
	sd->faces = NULL;
	sd->edges = NULL;

	/* Create vertices */
	nr_verts = mesh_vertex_buffer(mesh, &vbuf);
	buf_resize(sd->verts, nr_verts);
	buf_foreach(v, sd->verts) {
		vec_copy(v->p, vbuf);
		v->es = NULL;
		v->fs = NULL;
		vbuf += 3;
	}

	/* Create faces */
	nr_faces = mesh_face_count(mesh);
	buf_resize(sd->faces, nr_faces);
	for (i = 0; i < nr_faces; i++) {
		struct sd_face *f;

		f = &sd_f(i);
		f->vs = NULL;
		f->fvert = -1;

		nr_verts = mesh_face_vertex_count(mesh, i);
		buf_reserve(f->vs, nr_verts);
		for (j = 0; j < nr_verts; j++) {
			int vidx, nidx;

			mesh_face_vertex_index(mesh, i, j, &vidx, &nidx);
			buf_push(f->vs, vidx);
		}
	}

	/* Create edges */
	sd_update_links(sd);

	return sd;
}

static void sd_destroy(struct sd_mesh *sd)
{
	struct sd_vert *v;
	struct sd_face *f;

	buf_foreach(v, sd->verts) {
		buf_free(v->es);
		buf_free(v->fs);
	}
	buf_free(sd->verts);

	buf_foreach(f, sd->faces)
		buf_free(f->vs);
	buf_free(sd->faces);

	buf_free(sd->edges);

	free(sd);
}

static int sd_add_vert(struct sd_mesh *sd, vector p)
{
	struct sd_vert v;
	vec_copy(v.p, p);
	v.es = NULL;
	v.fs = NULL;
	buf_push(sd->verts, v);
	return buf_len(sd->verts) - 1;
}

static void sd_do_iteration(struct sd_mesh *sd, int first_iteration, int last_iteration)
{
	int V, F, E, Vn, Fn, En;
	struct sd_face *faces = NULL;
	struct sd_vert *v;
	struct sd_face *f;
	struct sd_edge *e;
	int *vi, *fi, *ei;

	/* V' = V + F + E
	 * F' = Sum_i=0^F(f_i), (F' = 4F, when quad-mesh)
	 * E' = 2E + F'
	 */
	V = buf_len(sd->verts);
	F = buf_len(sd->faces);
	E = buf_len(sd->edges);
	Vn = V + F + E;
	if (first_iteration) {
		Fn = 0;
		buf_foreach(f, sd->faces)
			Fn += buf_len(f->vs);
	} else {
		/* After the first iteration all faces are quads */
		Fn = 4 * F;
	}
	En = 2 * E + Fn;

	/* 1. Update vertices */
	buf_reserve(sd->verts, Vn);

	/* Create face vertices */
	buf_foreach(f, sd->faces) {
		vector p;

		vec_zero(p);
		buf_foreach(vi, f->vs)
			vec_add(p, p, sd_v(*vi).p);
		vec_mul(p, 1.0f / buf_len(f->vs), p);
		f->fvert = sd_add_vert(sd, p);
	}

	/* Create edge vertices */
	buf_foreach(e, sd->edges) {
		vector p;

		assert(e->f1 != -1);
		vec_zero(p);
		vec_add(p, p, sd_v(e->v0).p);
		vec_add(p, p, sd_v(e->v1).p);
		vec_add(p, p, sd_v(sd_f(e->f0).fvert).p);
		vec_add(p, p, sd_v(sd_f(e->f1).fvert).p);
		vec_mul(p, 0.25f, p);
		e->evert = sd_add_vert(sd, p);
	}

	/* Move old vertices */
	buf_foreach(v, sd->verts) {
		int n;
		vector p;

		if (sd_vi(v) >= V)
			break;

		assert(buf_len(v->fs) == buf_len(v->es));
		n = buf_len(v->fs);

		vec_copy(v->newp, v->p);
		vec_mul(v->newp, (float) (n - 2) / n, v->newp);

		vec_zero(p);
		buf_foreach(fi, v->fs)
			vec_add(p, p, sd_v(sd_f(*fi).fvert).p);
		vec_mad(v->newp, 1.0f / (n * n), p);
		
		vec_zero(p);
		buf_foreach(ei, v->es)
			vec_add(p, p, sd_edge_other(sd, &sd_e(*ei), v)->p);
		vec_mad(v->newp, 1.0f / (n * n), p);
	}

	buf_foreach(v, sd->verts) {
		if (sd_vi(v) >= V)
			break;
		vec_copy(v->p, v->newp);
	}

	/* 2. Create new faces */
	buf_reserve(faces, Fn);
	buf_foreach(f, sd->faces) {
		int j;
		for (j = 0; j < buf_len(f->vs); j++) {
			int v0, v, v1;
			struct sd_edge *e0, *e1;
			struct sd_face new_face;

			v0 = f->vs[(j - 1 + buf_len(f->vs)) % buf_len(f->vs)];
			v  = f->vs[j];
			v1 = f->vs[(j + 1) % buf_len(f->vs)];
			e0 = &sd_e(sd_find_edge(sd, v0, v));
			e1 = &sd_e(sd_find_edge(sd, v, v1));

			new_face.vs = NULL;
			buf_reserve(new_face.vs, 4);
			buf_push(new_face.vs, e0->evert);
			buf_push(new_face.vs, v);
			buf_push(new_face.vs, e1->evert);
			buf_push(new_face.vs, f->fvert);
			new_face.fvert = -1;

			buf_push(faces, new_face);
		}
	}
	SWAP(struct sd_face *, sd->faces, faces);
	buf_free(faces);

	/* 3. Update edges */
	if (!last_iteration) {	/* Skip on last iteration */
		buf_reserve(sd->edges, En);
		sd_update_links(sd);
	}
}

static struct mesh *sd_convert(struct sd_mesh *sd)
{
	struct mesh *mesh;
	struct sd_vert *v;
	struct sd_face *f;
	int *vi;

	mesh = mesh_create();
	buf_foreach(v, sd->verts)
		mesh_add_vertex(mesh, v->p);
	buf_foreach(f, sd->faces) {
		mesh_begin_face(mesh);
		buf_foreach(vi, f->vs)
			mesh_add_index(mesh, *vi, -1);
		mesh_end_face(mesh);
	}
	mesh_compute_normals(mesh);
	return mesh;
}

struct mesh *subdivide(const struct mesh *mesh, int iterations)
{
	int i;
	struct sd_mesh *sd;
	struct mesh *ret;

	sd = sd_init(mesh);
	for (i = 0; i < iterations; i++) {
		sd_do_iteration(sd, i == 0, i + 1 == iterations);
	}
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
		sd_do_iteration(sd, i == 0, i + 1 == nr_levels);
		levels[i] = sd_convert(sd);
	}
	sd_destroy(sd);
}
