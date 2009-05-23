#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include "geometry.h"
#include "mesh.h"

static struct mesh *mesh;

static struct vec cam_pos = { 0.0f, 0.0f, 5.0f };
static struct vec cam_at  = { 0.0f, 0.0f, 0.0f };
static struct vec cam_up  = { 0.0f, 1.0f, 0.0f };

static float fovy  = 90.0f;
static float znear = 0.1f, zfar = 1000.0f;
static GLint width = 1024, height = 1024;

static enum { NONE, ROTATING, PANNING, ZOOMING } cur_op = NONE;
static int last_x, last_y;

static void draw_frame(void)
{
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
}

static void draw_scene(void)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,
		  (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	glLightfv(GL_LIGHT0, GL_POSITION,
		  (GLfloat []) { cam_pos.x, cam_pos.y, cam_pos.z, 1.0f });
	glEnable(GL_LIGHT0);
	glShadeModel(GL_FLAT);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat []) { 1.0f, 1.0f, 1.0f, 1.0f });
	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();
	glTranslatef(0.0, 0.0, -5.0);
	glutSolidTeapot(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-5.0, -5.0, -15.0);
	glutSolidTeapot(1.0);
	glPopMatrix();

	mesh_render(mesh);

	glPopAttrib();
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (double) width / height, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cam_pos.x, cam_pos.y, cam_pos.z,
		  cam_at.x,  cam_at.y,  cam_at.z,
		  cam_up.x,  cam_up.y,  cam_up.z);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	draw_scene();
	draw_frame();

	glutSwapBuffers();
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
}

static void keyboard(unsigned char key, int x, int y)
{
	if (key == 27 || key == 17)
		exit(0);
}

static void special(int key, int x, int y)
{
	if ((glutGetModifiers() & GLUT_ACTIVE_ALT) && key == GLUT_KEY_F4)
		exit(0);
}

static void mouse(int button, int state, int x, int y)
{
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

	last_x = x;
	last_y = y;
}

static void
rotate_point(struct vec *r, const struct vec *p,
	     const struct vec *o, const struct vec *v, float alpha)
{
	mat_t m;
	mat_rot(m, v, alpha);

	vec_sub(r, p, o);
	mat_mul_point(r, m, r);
	vec_add(r, r, o);
}

static void motion(int x, int y)
{
	struct vec dir_x, dir_y, dir_z;
	float focal_len, dx, dy;

	focal_len = vec_dist(&cam_at, &cam_pos);
	vec_sub(&dir_z, &cam_at, &cam_pos);
	vec_div(&dir_z, focal_len);
	vec_prod(&dir_x, &cam_up, &dir_z);
	vec_prod(&dir_y, &dir_z, &dir_x);

	dx = (float) (x - last_x) / width;
	dy = (float) (y - last_y) / height;

	if (cur_op == ROTATING) {
		rotate_point(&cam_pos, &cam_pos, &cam_at, &dir_y, -dx * M_PI);
		rotate_point(&cam_pos, &cam_pos, &cam_at, &dir_x, dy * M_PI);

		/* vec_add(&cam_up, &cam_up, &cam_at); */
		/* rotate_point(&cam_up, &cam_up, &cam_at, &dir_x, dy * M_PI); */
		/* vec_sub(&cam_up, &cam_up, &cam_at); */
	} else if (cur_op == PANNING) {
		float lx, ly;
		struct vec d = { 0.0, 0.0, 0.0 };

		ly = 2.0f * focal_len * tan(fovy / 2.0f * DEGREE);
		lx = ly * width / height;

		vec_mad(&d, dx * lx, &dir_x);
		vec_mad(&d, dy * ly, &dir_y);

		vec_add(&cam_pos, &cam_pos, &d);
		vec_add(&cam_at, &cam_at, &d);
	} else if (cur_op == ZOOMING) {
		vec_mad(&cam_pos, focal_len * dy, &dir_z);
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
	struct vec min, max;

	mesh = mesh_read_obj("objs/bigguy_00.obj");
	mesh_calc_bounds(mesh, &min, &max);
	cam_pos.z = 2.0f * max.y;

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

	glutMainLoop();

	return 0;
}
