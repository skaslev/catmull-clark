#include <stdlib.h>
#include <string.h>
#include "gl.h"
#include "buf.h"
#include "mesh.h"
#include "meshrend.h"
#include "subd.h"
#include "obj.h"
#include "gl_util.h"
#include "editor.h"

struct ed_obj {
	struct mesh *mesh;
	int cur_level;
	int nr_levels;
	GLuint lists;
	char file[256];
	struct { int vs, fs; } *stats;
};

struct editor {
	struct ed_obj *objs;
	int cur_obj;
	int wireframe;
	int editing;
};

#define MAX_LEVELS		16

struct editor *ed_create()
{
	struct editor *ed;
	ed = malloc(sizeof(struct editor));
	ed->objs = NULL;
	ed->cur_obj = 0;
	ed->wireframe = 0;
	ed->editing = 0;
	return ed;
}

void ed_add_obj(struct editor *ed, const char *file, int nr_levels)
{
	int i;
	struct ed_obj ed_obj;
	struct mesh *levels[MAX_LEVELS];

	ed_obj.mesh = obj_read(file);
	ed_obj.cur_level = 0;
	ed_obj.nr_levels = nr_levels;
	ed_obj.lists = glGenLists(nr_levels);
	strncpy(ed_obj.file, file, sizeof(ed_obj.file));
	ed_obj.stats = NULL;
	buf_resize(ed_obj.stats, nr_levels);

	ed_obj.stats[0].vs = mesh_vertex_buffer(ed_obj.mesh, NULL);
	ed_obj.stats[0].fs = mesh_face_count(ed_obj.mesh);
	mesh_compile_list(ed_obj.mesh, ed_obj.lists);
	subdivide_levels(ed_obj.mesh, levels, nr_levels - 1);
	for (i = 0; i < nr_levels - 1; i++) {
		ed_obj.stats[i+1].vs = mesh_vertex_buffer(levels[i], NULL);
		ed_obj.stats[i+1].fs = mesh_face_count(levels[i]);
		mesh_compile_list(levels[i], ed_obj.lists + i + 1);
		mesh_destroy(levels[i]);
	}

	buf_push(ed->objs, ed_obj);
}

#define cur_obj(ed)		((ed)->objs[(ed)->cur_obj])

struct mesh *ed_cur_obj(struct editor *ed)
{
	return cur_obj(ed).mesh;
}

void ed_next_obj(struct editor *ed)
{
	ed->cur_obj = (ed->cur_obj + 1) % buf_len(ed->objs);
}

void ed_prev_obj(struct editor *ed)
{
	ed->cur_obj = (ed->cur_obj - 1 + buf_len(ed->objs)) % buf_len(ed->objs);
}

void ed_next_level(struct editor *ed)
{
	struct ed_obj *ed_obj = &cur_obj(ed);

	if (ed_obj->cur_level < ed_obj->nr_levels - 1)
		ed_obj->cur_level++;
}

void ed_prev_level(struct editor *ed)
{
	struct ed_obj *ed_obj = &cur_obj(ed);

	if (ed_obj->cur_level > 0)
		ed_obj->cur_level--;
}

void ed_toggle_wireframe(struct editor *ed)
{
	ed->wireframe = !ed->wireframe;
}

void ed_toggle_editing(struct editor *ed)
{
	struct ed_obj *ed_obj = &cur_obj(ed);

	ed->editing = !ed->editing;
	if (ed->editing) {
		struct mesh *mesh;

		ed_obj->cur_level = 2;
		mesh = subdivide(ed_obj->mesh, ed_obj->cur_level);
		mesh_compile_list(mesh, ed_obj->lists + ed_obj->cur_level);
		mesh_destroy(mesh);
	} else {
		int i;
		struct mesh *levels[MAX_LEVELS];

		mesh_compile_list(ed_obj->mesh, ed_obj->lists);
		subdivide_levels(ed_obj->mesh, levels, ed_obj->nr_levels - 1);
		for (i = 0; i < ed_obj->nr_levels - 1; i++) {
			mesh_compile_list(levels[i], ed_obj->lists + i + 1);
			mesh_destroy(levels[i]);
		}
	}
}

int ed_is_editing(struct editor *ed)
{
	return ed->editing;
}

void ed_render(struct editor *ed)
{
	struct ed_obj *ed_obj = &cur_obj(ed);

	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
		     (GLfloat[4]) { 1.0f, 1.0f, 1.0f, 1.0f });
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

void ed_render_overlay(struct editor *ed)
{
	struct ed_obj *ed_obj = &cur_obj(ed);

	glRasterPos2f(0.005f, 0.975f);
	gl_printf(GLUT_BITMAP_HELVETICA_18, "%s@%d",
		  ed_obj->file, ed_obj->cur_level);
	glRasterPos2f(0.005f, 0.950f);
	gl_printf(GLUT_BITMAP_HELVETICA_18, "%d verts %d faces",
		  ed_obj->stats[ed_obj->cur_level].vs,
		  ed_obj->stats[ed_obj->cur_level].fs);
}
