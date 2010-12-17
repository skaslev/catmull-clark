#include <math.h>
#include <stdlib.h>
#include "gl.h"
#include "mesh.h"
#include "mathx.h"

void mesh_render(const struct mesh *mesh)
{
	int i, nr_faces;

	nr_faces = mesh_face_count(mesh);
	for (i = 0; i < nr_faces; i++) {
		int j, nr_verts;

		nr_verts = mesh_face_vertex_count(mesh, i);
		glBegin(GL_POLYGON);
		for (j = 0; j < nr_verts; j++) {
			const float *v;

			v = mesh_get_normal(mesh, i, j);
			if (v)
				glNormal3fv(v);

			v = mesh_get_vertex(mesh, i, j);
			glVertex3fv(v);
		}
		glEnd();
	}
}

void mesh_compile_list(const struct mesh *mesh, GLuint list)
{
	glNewList(list, GL_COMPILE);
	mesh_render(mesh);
	glEndList();
}

void mesh_calc_bounds(const struct mesh *mesh, float *min, float *max)
{
	int i, nr;
	const float *vbuf;

	nr = mesh_vertex_buffer(mesh, &vbuf);
	vec_set(min, INFINITY, INFINITY, INFINITY);
	vec_neg(max, min);
	for (i = 0; i < nr; i++) {
		vec_min(min, min, vbuf + 3 * i);
		vec_max(max, max, vbuf + 3 * i);
	}
}
