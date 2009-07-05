#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include "geometry.h"
#include "mesh.h"
#include "meshrend.h"
#include "gl_util.h"
#include "editor.h"

static struct editor *ed;

static struct vec center = { 0.0, 0.0, 0.0 };
static float focal_len = 5.0f;
static float y_rot = 0.0f, x_rot = 0.0f;

static float fovy  = 90.0f;
static float znear = 0.1f, zfar = 1000.0f;
static GLint width = 1024, height = 1024;

static enum { NONE, ROTATING, PANNING, ZOOMING } cur_op = NONE;
static int last_x, last_y;

static void focus_camera(const struct mesh *mesh)
{
	struct vec min, max;

	/* FIXME */
	mesh_calc_bounds(mesh, &min, &max);
	vec_add(&center, &min, &max);
	vec_div(&center, 2.0f);
	focal_len = 4.0f * max.y;
}

static void
get_camera_frame(struct vec *x, struct vec *y, struct vec *z)
{
	mat_t m;

	*x = (struct vec) { 1.0f, 0.0f, 0.0f };
	*y = (struct vec) { 0.0f, 1.0f, 0.0f };
	*z = (struct vec) { 0.0f, 0.0f, 1.0f };

	mat_rot(m, y, y_rot);
	mat_mul_vector(x, m, x);
	mat_mul_vector(z, m, z);

	mat_rot(m, x, x_rot);
	mat_mul_vector(y, m, y);
	mat_mul_vector(z, m, z);
}

static inline void
get_camera(struct vec *eye, struct vec *at, struct vec *up)
{
	struct vec x, y, z;

	get_camera_frame(&x, &y, &z);
	*eye = *at = center;
	vec_mad(eye, focal_len, &z);
	*up = y;
}

static void draw_frame(void)
{
	glPushAttrib(GL_LIGHTING_BIT);
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

static void draw_fps(void)
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
	glRasterPos2f(0.925f, 0.975f);
	gl_printf(GLUT_BITMAP_HELVETICA_18, "%d fps", fps);
}

static void display(void)
{
	struct vec eye, at, up;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Render scene */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (double) width / height, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	get_camera(&eye, &at, &up);
	gluLookAt(eye.x, eye.y, eye.z, at.x, at.y, at.z, up.x, up.y, up.z);

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,
		  (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	glLightfv(GL_LIGHT0, GL_POSITION,
		  (GLfloat []) { eye.x, eye.y, eye.z, 1.0f });
	glEnable(GL_LIGHT0);

	ed_render(ed);
	draw_frame();

	/* Render overlays */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0f, 1.0f);

	draw_fps();
	ed_render_overlay(ed);

	/* Swap buffers */
	glutSwapBuffers();
	glutPostRedisplay();
}

static void reshape(int w, int h)
{
	width = w > 1 ? w : 1;
	height = h > 1 ? h : 1;

	glViewport(0, 0, width, height);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearDepth(1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
}

static void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27: case 17:
		exit(0);
		break;
	case 'f': case 'F':
		focus_camera(ed_cur_obj(ed));
		break;
	case 'e': case 'E':
		ed_toggle_editing(ed);
		break;
	}

	if (!ed->editing) {
		switch (key) {
		case ' ':
			ed_next_obj(ed);
			focus_camera(ed_cur_obj(ed));
			break;
		case 8:			/* Backspace */
			ed_prev_obj(ed);
			focus_camera(ed_cur_obj(ed));
			break;
		case 'w': case 'W':
			ed_toggle_wireframe(ed);
			break;
		case '=': case '+':
			ed_next_level(ed);
			break;
		case '-': case '_':
			ed_prev_level(ed);
			break;
		}
	}
}

static void special(int key, int x, int y)
{
	if ((glutGetModifiers() & GLUT_ACTIVE_ALT) && key == GLUT_KEY_F4)
		exit(0);
}

static void mouse(int button, int state, int x, int y)
{
	static const int cursor[] = {
		[NONE]     = GLUT_CURSOR_RIGHT_ARROW,
		[ROTATING] = GLUT_CURSOR_CYCLE,
		[PANNING]  = GLUT_CURSOR_CROSSHAIR,
		[ZOOMING]  = GLUT_CURSOR_UP_DOWN
	};

	if ((glutGetModifiers() & GLUT_ACTIVE_ALT) && state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON)
			cur_op = ROTATING;
		else if (button == GLUT_MIDDLE_BUTTON)
			cur_op = PANNING;
		else if (button == GLUT_RIGHT_BUTTON)
			cur_op = ZOOMING;
	} else {
		cur_op = NONE;
	}

	glutSetCursor(cursor[cur_op]);

	last_x = x;
	last_y = y;
}

static void motion(int x, int y)
{
	float dx, dy;

	dx = (float) (x - last_x) / width;
	dy = (float) (y - last_y) / height;

	if (cur_op == ROTATING) {
		y_rot -= dx * 2.0f * M_PI;
		x_rot -= dy * 2.0f * M_PI;
	} else if (cur_op == PANNING) {
		float lx, ly;
		struct vec x, y, z;

		ly = 2.0f * focal_len * tan(fovy / 2.0f * DEGREE);
		lx = ly * width / height;

		get_camera_frame(&x, &y, &z);
		vec_mad(&center, -dx * lx, &x);
		vec_mad(&center,  dy * ly, &y);
	} else if (cur_op == ZOOMING) {
		focal_len *= 1.0f - dy;
	}

	last_x = x;
	last_y = y;
}

static void idle(void)
{
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("catmull-clark");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);

	printf("Loading... "); fflush(stdout);
	ed = ed_create();
	ed_add_obj(ed, "objs/cube.obj", 5);
	ed_add_obj(ed, "objs/tetra.obj", 5);
	ed_add_obj(ed, "objs/bigguy.obj", 3);
	ed_add_obj(ed, "objs/monsterfrog.obj", 3);
	printf("done.\n");

	focus_camera(ed_cur_obj(ed));

	glutMainLoop();

	return 0;
}
