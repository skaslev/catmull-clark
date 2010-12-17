#ifndef EDITOR_H
#define EDITOR_H

struct editor *ed_create(void);
void ed_add_obj(struct editor *ed, const char *file, int levels);

struct mesh *ed_cur_obj(struct editor *ed);
void ed_next_obj(struct editor *ed);
void ed_prev_obj(struct editor *ed);
void ed_next_level(struct editor *ed);
void ed_prev_level(struct editor *ed);

void ed_toggle_wireframe(struct editor *ed);
void ed_toggle_editing(struct editor *ed);
int  ed_is_editing(struct editor *ed);
void ed_render(struct editor *ed);
void ed_render_overlay(struct editor *ed);

#endif
