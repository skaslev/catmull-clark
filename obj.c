#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "mathx.h"
#include "mesh.h"

static const char *skip_space(const char *str)
{
	while (isspace(*str))
		str++;
	return str;
}

static const char *skip_non_space(const char *str)
{
	while (*str && !isspace(*str))
		str++;
	return str;
}

struct mesh *obj_read(const char *file)
{
	FILE *f;
	char line[512];
	const char *str;
	struct mesh *mesh;
	int has_normals = 0;

	if (!(f = fopen(file, "r")))
		return NULL;

	mesh = mesh_create();
	while (!feof(f)) {
		if (!fgets(line, sizeof(line), f))
			break;

		str = line;
		if (strncmp(str, "v ", 2) == 0) {
			/* Vertex command */
			vector v;

			sscanf(str, "v %f %f %f", v, v + 1, v + 2);
			mesh_add_vertex(mesh, v);
		} else if (strncmp(str, "vn ", 3) == 0) {
			/* Normal command */
			vector n;

			sscanf(str, "vn %f %f %f", n, n + 1, n + 2);
			mesh_add_normal(mesh, n);
		} else if (strncmp(str, "f ", 2) == 0) {
			/* Face command */
			int vi, ti, ni;

			mesh_begin_face(mesh);
			str = skip_space(++str); /* Skip 'f ' */
			while (*str) {
				vi = ti = ni = 0;
				if (sscanf(str, "%d/%d/%d", &vi, &ti, &ni) == 3)
					has_normals = 1;
				mesh_add_index(mesh, vi - 1, ni - 1);
				str = skip_non_space(str);
				str = skip_space(str);
			}
			mesh_end_face(mesh);
		}
	}

	fclose(f);

	if (!has_normals)
		mesh_compute_normals(mesh);

	return mesh;
}
