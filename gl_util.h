#ifndef GL_UTIL_H
#define GL_UTIL_H

void gl_begin_2d(void);
void gl_end_2d(void);

void gl_draw_xyz(void);
void gl_draw_quad(int flipy);
void gl_draw_fps(float x, float y);

void gl_printf(void *font, const char *format, ...);

#endif
