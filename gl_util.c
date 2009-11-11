#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "gl.h"
#include "gl_util.h"

void gl_begin_2d(void)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void gl_end_2d(void)
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void gl_draw_xyz(void)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	/* Draw the x axis */
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

	/* Draw the y axis */
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	/* Draw the z axis */
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);

	glEnd();
	glPopAttrib();
}

void gl_draw_quad(int flipy)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, flipy ? 1.0f : 0.0f);
	glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, flipy ? 1.0f : 0.0f);
	glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, flipy ? 0.0f : 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, flipy ? 0.0f : 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();
}

void gl_draw_fps(float x, float y)
{
	static clock_t ticks;
	static int nr_frames, fps;
	clock_t now;

	/* Calculate frame rate */
	nr_frames++;
	now = clock();
	if (now - ticks >= CLOCKS_PER_SEC) {
		fps = nr_frames;
		ticks = now;
		nr_frames = 0;
	}

	/* Render fps counter */
	glRasterPos2f(x, y);
	gl_printf(GLUT_BITMAP_HELVETICA_18, "%d fps", fps);
}

void gl_printf(void *font, const char *format, ...)
{
	char str[512], *p;
	va_list args;

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	for (p = str; *p; p++)
		glutBitmapCharacter(font, *p);
}
