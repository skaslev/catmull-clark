#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "geometry.h"
#include "mesh.h"
#include "meshrend.h"
#include "obj.h"
#include "subd.h"
#include "gl_util.h"
#include "util.h"

int cur_obj = 0;
const char *objs[] = {
	"objs/cube.obj",
	"objs/bigguy.obj",
	"objs/monsterfrog_mapped.obj"
};

#define NR_LEVELS	3
static struct mesh *levels[ARRAY_SIZE(objs)][NR_LEVELS];
static GLuint lists[ARRAY_SIZE(objs)][5];
static int cur_level = 0;

static struct vec center = { 0.0, 0.0, 0.0 };
static float focal_len = 5.0f;
static float y_rot = 0.0f, x_rot = 0.0f;

static float fovy  = 90.0f;
static float znear = 0.1f, zfar = 1000.0f;
static GLint width = 1024, height = 1024;

static int wireframe = 0;
static int editing = 0;
static enum { NONE, ROTATING, PANNING, ZOOMING } cur_op = NONE;
static int last_x, last_y;

static void init_levels()
{
	int i, j;

	for (i = 0; i < ARRAY_SIZE(objs); i++) {
		levels[i][0] = obj_read(objs[i]);
		subdivide_levels(levels[i][0], &levels[i][1], NR_LEVELS - 1);
		for (j = 0; j < NR_LEVELS; j++)
			lists[i][j] = mesh_create_list(levels[i][j]);
	}
}

static void focus_camera()
{
	struct vec min, max;

	/* FIXME */
	mesh_calc_bounds(levels[cur_obj][0], &min, &max);
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

static void draw_scene(void)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	if (wireframe) {
		glDisable(GL_LIGHTING);
		glColor3f(0.0f, 1.0f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
			     (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	}
	glCallList(lists[cur_obj][cur_level]);
	glPopAttrib();
}

static void draw_scene_editing(void)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
		     (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	glCallList(lists[cur_obj][1]);

	glDisable(GL_LIGHTING);
	glColor3f(0.0f, 1.0f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glCallList(lists[cur_obj][0]);

	glPopAttrib();
}

static void display(void)
{
	struct vec eye, at, up;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	if (editing)
		draw_scene_editing();
	else
		draw_scene();
	draw_frame();

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
		focus_camera();
		break;
	case 'e': case 'E':
		editing = !editing;
		if (editing) {
			int i;
			for (i = 1; i < NR_LEVELS; i++)
				mesh_destroy(levels[cur_obj][i]);
			levels[cur_obj][1] = subdivide(levels[cur_obj][0], NR_LEVELS - 1);
			lists[cur_obj][1] = mesh_create_list(levels[cur_obj][1]);
		} else {
			int i;
			mesh_destroy(levels[cur_obj][1]);
			subdivide_levels(levels[cur_obj][0], &levels[cur_obj][1], NR_LEVELS - 1);
			for (i = 0; i < NR_LEVELS; i++)
				lists[cur_obj][i] = mesh_create_list(levels[cur_obj][i]);

		}
		break;
	}

	if (!editing) {
		switch (key) {
		case ' ':
			cur_obj = (cur_obj + 1) % ARRAY_SIZE(objs);
			focus_camera();
			break;
		case 8:			/* Backspace */
			cur_obj = (cur_obj - 1 + ARRAY_SIZE(objs)) % ARRAY_SIZE(objs);
			focus_camera();
			break;
		case 'w': case 'W':
			wireframe = !wireframe;
			break;
		case '=': case '+':
			if (cur_level < NR_LEVELS - 1)
				cur_level++;
			break;
		case '-': case '_':
			if (cur_level > 0)
				cur_level--;
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

	init_levels();
	focus_camera();

	glutMainLoop();

	return 0;
}
