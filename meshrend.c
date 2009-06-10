#include <math.h>
#include <GL/gl.h>
#include "geometry.h"
#include "mesh.h"

void mesh_render(const struct mesh *mesh)
{
	int i, faces;

	faces = mesh_face_count(mesh);
	for (i = 0; i < faces; i++) {
		int j, verts;

		verts = mesh_face_vertex_count(mesh, i);
		glBegin(GL_POLYGON);
		for (j = 0; j < verts; j++) {
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
	int i, verts;
	const struct vec *vbuf;

	verts = mesh_vertex_count(mesh);
	vbuf  = mesh_vertex_buffer(mesh);

	*min = (struct vec) {  INFINITY,  INFINITY,  INFINITY };
	*max = (struct vec) { -INFINITY, -INFINITY, -INFINITY };
	for (i = 0; i < verts; i++) {
		vec_min(min, min, vbuf + i);
		vec_max(max, max, vbuf + i);
	}
}
