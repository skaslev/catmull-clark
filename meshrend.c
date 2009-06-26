#include <math.h>
#include <GL/gl.h>
#include "geometry.h"
#include "mesh.h"

void mesh_render(const struct mesh *mesh)
{
	int i, nr_faces;

	nr_faces = mesh_face_count(mesh);
	for (i = 0; i < nr_faces; i++) {
		int j, nr_verts;

		nr_verts = mesh_face_vertex_count(mesh, i);
		glBegin(GL_POLYGON);
		for (j = 0; j < nr_verts; j++) {
			const struct vec *v;

			if ((v = mesh_get_normal(mesh, i, j)))
				glNormal3f(v->x, v->y, v->z);

			v = mesh_get_vertex(mesh, i, j);
			glVertex3f(v->x, v->y, v->z);
		}
		glEnd();
	}
}

void mesh_calc_bounds(const struct mesh *mesh,
		      struct vec *min, struct vec *max)
{
	int i, nr;
	const struct vec *vbuf;

	nr = mesh_vertex_buffer(mesh, &vbuf);
	*min = (struct vec) {  INFINITY,  INFINITY,  INFINITY };
	*max = (struct vec) { -INFINITY, -INFINITY, -INFINITY };
	for (i = 0; i < nr; i++) {
		vec_min(min, min, vbuf + i);
		vec_max(max, max, vbuf + i);
	}
}
