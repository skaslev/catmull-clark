#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <GL/gl.h>
#include "geometry.h"
#include "util.h"
#include "mesh.h"

struct mesh *mesh_create()
{
	struct mesh *m = malloc(sizeof(*m));
	ARR_INIT(m->verts);
	ARR_INIT(m->idxs);
	return m;
}

void mesh_destroy(struct mesh *m)
{
	ARR_CLEAR(m->verts);
	ARR_CLEAR(m->idxs);
	free(m);
}

void mesh_add_vertex(struct mesh *m, const struct vec *vert)
{
	ARR_PUSH(m->verts, *vert);
}

void mesh_begin_face(struct mesh *m)
{
	if (ARR_SIZE(m->idxs) > 0)
		ARR_PUSH(m->idxs, -1); /* Use -1 as face marker */
}

void mesh_add_index(struct mesh *m, int idx)
{
	ARR_PUSH(m->idxs, idx);
}


void mesh_end_face(struct mesh *m)
{
	/* noop */
}

static inline const char *skip_space(const char *str)
{
	while (*str && isspace(*str))
		str++;
	return str;
}

static inline const char *skip_non_space(const char *str)
{
	while (*str && !isspace(*str))
		str++;
	return str;
}

struct mesh *mesh_read_obj(const char *file)
{
	FILE *f;
	char line[512];
	const char *str;
	struct mesh *m;

	if (!(f = fopen(file, "r")))
		return NULL;

	m = mesh_create();
	while (!feof(f)) {
		if (!fgets(line, ARRAY_SIZE(line), f))
			break;
		
		str = line;
		if (strncmp(str, "v ", 2) == 0) {
			/* Vertex command */
			struct vec v;

			sscanf(str, "v %f %f %f", &v.x, &v.y, &v.z);
			mesh_add_vertex(m, &v);
		} else if (strncmp(str, "f ", 2) == 0) {
			/* Face command */
			int idx;

			mesh_begin_face(m);
			str = skip_space(++str); /* Skip 'f ' */
			while (*str) {
				sscanf(str, "%d", &idx);
				mesh_add_index(m, idx - 1);
				str = skip_non_space(str);
				str = skip_space(str);
			}
			mesh_end_face(m);
		}
	}

	return m;
}

void mesh_calc_bounds(const struct mesh *m,
		      struct vec *min, struct vec *max)
{
	int i;

	*min = (struct vec) {  INFINITY,  INFINITY,  INFINITY };
	*max = (struct vec) { -INFINITY, -INFINITY, -INFINITY };
	for (i = 0; i < ARR_SIZE(m->verts); i++) {
		struct vec *v = ARR_ELTS(m->verts) + i;
		vec_min(min, min, v);
		vec_max(max, max, v);
	}
}

void mesh_render(const struct mesh *m)
{
	int i, j;

	for (i = 0; i < ARR_SIZE(m->idxs); i = j + 1) {
		glBegin(GL_POLYGON);
		for (j = i; j < ARR_SIZE(m->idxs); j++) {
			int idx;
			struct vec *v;

			if ((idx = ARR_AT(m->idxs, j)) == -1)
				break;
			v = ARR_ELTS(m->verts) + idx;
			glVertex3f(v->x, v->y, v->z);
		}
		glEnd();
	}
}
