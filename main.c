#include <stdio.h>
#include <stdlib.h>
#include "gl.h"
#include "gl_util.h"
#include "mathx.h"
#include "mesh.h"
#include "meshrend.h"
#include "editor.h"

static struct editor *ed;

static vector center = { 0.0, 0.0, 0.0 };
static float focal_len = 5.0f;
static float y_rot = 0.0f, x_rot = 0.0f;

static float fovy  = 60.0f;
static float znear = 0.1f, zfar = 1000.0f;
static GLint width = 960, height = 960;

static enum { NONE, ROTATING, PANNING, ZOOMING } cur_op = NONE;
static int last_x, last_y;

static void focus_camera(const struct mesh *mesh)
{
	vector min, max;

	/* FIXME */
	mesh_calc_bounds(mesh, min, max);
	vec_add(center, min, max);
	vec_mul(center, 0.5f, center);
	focal_len = 6.0f * max[1];
}

static void get_camera_frame(vector x, vector y, vector z)
{
	matrix m;

	vec_set(x, 1.0f, 0.0f, 0.0f);
	vec_set(y, 0.0f, 1.0f, 0.0f);
	vec_set(z, 0.0f, 0.0f, 1.0f);

	mat_rotate(m, y, y_rot);
	mat_mul_vector(x, m, x);
	mat_mul_vector(z, m, z);

	mat_rotate(m, x, x_rot);
	mat_mul_vector(y, m, y);
	mat_mul_vector(z, m, z);
}

static void get_camera(vector eye, vector at, vector up)
{
	vector x, y, z;

	get_camera_frame(x, y, z);
	vec_copy(at, center);
	vec_copy(eye, center);
	vec_mad(eye, focal_len, z);
	vec_copy(up, y);
}

static void display(void)
{
	matrix m;
	vector eye, at, up;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Render scene */
	glMatrixMode(GL_PROJECTION);
	mat_persp(m, fovy, (double) width / height, znear, zfar);
	glLoadMatrixf(m);
	glMatrixMode(GL_MODELVIEW);
	get_camera(eye, at, up);
	mat_lookat(m, eye, at, up);
	glLoadMatrixf(m);

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,
		  (GLfloat[4]) { 1.0f, 1.0f, 1.0f, 1.0f });
	glLightfv(GL_LIGHT0, GL_POSITION,
		  (GLfloat[4]) { eye[0], eye[1], eye[2], 1.0f });
	glEnable(GL_LIGHT0);

	ed_render(ed);
	gl_draw_xyz();

	/* Render overlays */
	gl_begin_2d();
	gl_draw_fps(0.925f, 0.975f);
	ed_render_overlay(ed);
	gl_end_2d();

	/* Swap buffers */
	glutSwapBuffers();
	glutPostRedisplay();
}

static void reshape(int w, int h)
{
	width = w > 1 ? w : 1;
	height = h > 1 ? h : 1;
	glViewport(0, 0, width, height);
	glClearDepth(1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
}

static void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27: case 17:	/* Esc or Ctrl-Q */
		exit(0);
		break;
	case 'f': case 'F':
		focus_camera(ed_cur_obj(ed));
		break;
	case 'e': case 'E':
		ed_toggle_editing(ed);
		break;
	}

	if (!ed_is_editing(ed)) {
		switch (key) {
		case ' ':
			ed_next_obj(ed);
			focus_camera(ed_cur_obj(ed));
			break;
		case 8:	case 127:	/* Backspace or Delete */
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
	if (ed_is_editing(ed))
		return;
	switch (key) {
	case GLUT_KEY_RIGHT:
		ed_next_obj(ed);
		focus_camera(ed_cur_obj(ed));
		break;
	case GLUT_KEY_LEFT:
		ed_prev_obj(ed);
		focus_camera(ed_cur_obj(ed));
		break;
	case GLUT_KEY_UP:
		ed_next_level(ed);
		break;
	case GLUT_KEY_DOWN:
		ed_prev_level(ed);
		break;
	}
}

static void mouse(int button, int state, int x, int y)
{
	static const int cursor[] = {
		GLUT_CURSOR_RIGHT_ARROW,		/* NONE */
		GLUT_CURSOR_CYCLE,			/* ROTATING */
		GLUT_CURSOR_CROSSHAIR,			/* PANNING */
		GLUT_CURSOR_UP_DOWN			/* ZOOMING */
	};

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON)
			cur_op = ROTATING;
		else if (button == GLUT_RIGHT_BUTTON)
			cur_op = ZOOMING;
		else if (button == GLUT_MIDDLE_BUTTON)
			cur_op = PANNING;
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
		y_rot -= dx * 360.0f;
		x_rot -= dy * 360.0f;
	} else if (cur_op == PANNING) {
		float lx, ly;
		vector x, y, z;

		ly = 2.0f * focal_len * tanf(radians(fovy / 2.0f));
		lx = ly * width / height;

		get_camera_frame(x, y, z);
		vec_mad(center, -dx * lx, x);
		vec_mad(center,  dy * ly, y);
	} else if (cur_op == ZOOMING) {
		focal_len *= (1.0f - dy) - dx;
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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
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
