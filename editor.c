#include <GL/gl.h>
#include "geometry.h"
#include "mesh.h"
#include "meshrend.h"
#include "subd.h"
#include "obj.h"
#include "editor.h"

struct editor *ed_create()
{
	struct editor *ed;

	ed = malloc(sizeof(struct editor));
	arr_init(ed->objs);
	ed->cur_obj = 0;
	ed->wireframe = 0;
	ed->editing = 0;
	return ed;
}

void ed_add_obj(struct editor *ed, const char *file, int nr_levels)
{
	int i;
	struct ed_obj ed_obj;
	struct mesh *levels[nr_levels - 1];

	ed_obj.mesh = obj_read(file);
	ed_obj.cur_level = 0;
	ed_obj.nr_levels = nr_levels;
	ed_obj.lists = glGenLists(nr_levels);

	mesh_compile_list(ed_obj.mesh, ed_obj.lists);
	subdivide_levels(ed_obj.mesh, levels, nr_levels - 1);
	for (i = 0; i < nr_levels - 1; i++) {
		mesh_compile_list(levels[i], ed_obj.lists + i + 1);
		mesh_destroy(levels[i]);
	}

	arr_push(ed->objs, ed_obj);
}

static inline struct ed_obj *cur_obj(struct editor *ed)
{
	return &arr_at(ed->objs, ed->cur_obj);
}

struct mesh *ed_cur_obj(struct editor *ed)
{
	return cur_obj(ed)->mesh;
}

void ed_next_obj(struct editor *ed)
{
	ed->cur_obj = (ed->cur_obj + 1) % arr_size(ed->objs);
}

void ed_prev_obj(struct editor *ed)
{
	ed->cur_obj = (ed->cur_obj - 1 + arr_size(ed->objs)) % arr_size(ed->objs);
}

void ed_next_level(struct editor *ed)
{
	struct ed_obj *ed_obj;

	ed_obj = cur_obj(ed);
	if (ed_obj->cur_level < ed_obj->nr_levels - 1)
		ed_obj->cur_level++;
}

void ed_prev_level(struct editor *ed)
{
	struct ed_obj *ed_obj;

	ed_obj = cur_obj(ed);
	if (ed_obj->cur_level > 0)
		ed_obj->cur_level--;
}

void ed_toggle_wireframe(struct editor *ed)
{
	ed->wireframe = !ed->wireframe;
}

void ed_toggle_editing(struct editor *ed)
{
	struct ed_obj *ed_obj;

	ed_obj = cur_obj(ed);
	ed->editing = !ed->editing;
	if (ed->editing) {
		struct mesh *mesh;

		ed_obj->cur_level = 2;
		mesh = subdivide(ed_obj->mesh, ed_obj->cur_level);
		mesh_compile_list(mesh, ed_obj->lists + ed_obj->cur_level);
		mesh_destroy(mesh);
	} else {
		int i;
		struct mesh *levels[ed_obj->nr_levels - 1];

		mesh_compile_list(ed_obj->mesh, ed_obj->lists);
		subdivide_levels(ed_obj->mesh, levels, ed_obj->nr_levels - 1);
		for (i = 0; i < ed_obj->nr_levels - 1; i++) {
			mesh_compile_list(levels[i], ed_obj->lists + i + 1);
			mesh_destroy(levels[i]);
		}
	}
}

void ed_render(struct editor *ed)
{
	struct ed_obj *ed_obj;

	ed_obj = cur_obj(ed);
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
		     (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	if (ed->editing) {
		glCallList(ed_obj->lists + ed_obj->cur_level);
		glDisable(GL_LIGHTING);
		glColor3f(0.0f, 1.0f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(ed_obj->lists);
	} else {
		if (ed->wireframe) {
			glDisable(GL_LIGHTING);
			glColor3f(0.0f, 1.0f, 0.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glCallList(ed_obj->lists + ed_obj->cur_level);
	}
	glPopAttrib();
}
