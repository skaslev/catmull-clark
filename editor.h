#ifndef EDITOR_H
#define EDITOR_H

struct ed_mesh_stat {
	int vs, fs;
};

struct ed_obj {
	struct mesh *mesh;
	int cur_level;
	int nr_levels;
	GLuint lists;
	char file[256];
	struct ed_mesh_stat *stats;
};

struct editor {
	struct ed_obj *objs;
	int cur_obj;
	int wireframe;
	int editing;
};

struct editor *ed_create();
void ed_add_obj(struct editor *ed, const char *file, int levels);

struct mesh *ed_cur_obj(struct editor *ed);
void ed_next_obj(struct editor *ed);
void ed_prev_obj(struct editor *ed);
void ed_next_level(struct editor *ed);
void ed_prev_level(struct editor *ed);

void ed_toggle_wireframe(struct editor *ed);
void ed_toggle_editing(struct editor *ed);
void ed_render(struct editor *ed);
void ed_render_overlay(struct editor *ed);

#endif
