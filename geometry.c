#include <math.h>
#include "geometry.h"

const struct vec vec_null;
const struct mat mat_null;
const struct mat mat_ident = { { { 1, 0, 0, 0 },
				 { 0, 1, 0, 0 },
				 { 0, 0, 1, 0 },
				 { 0, 0, 0, 1 } } };

void mat_invert(struct mat *r, const struct mat *a)
{
	/* TODO */
}

void mat_prod(struct mat *r, const struct mat *a, const struct mat *b)
{
	int i,j,k;

	*r = mat_null;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				r->m[i][j] += a->m[i][k] * b->m[k][j];
}

void mat_mul_point(struct vec *r, const struct mat *m, const struct vec *a)
{
	struct vec ret;
	ret.x = m->m[0][0] * a->x + m->m[0][1] * a->y + m->m[0][2] * a->z + m->m[0][3];
	ret.y = m->m[1][0] * a->x + m->m[1][1] * a->y + m->m[1][2] * a->z + m->m[1][3];
	ret.z = m->m[2][0] * a->x + m->m[2][1] * a->y + m->m[2][2] * a->z + m->m[2][3];
	*r = ret;
}

void mat_mul_vector(struct vec *r, const struct mat *m, const struct vec *a)
{
	struct vec ret;
	ret.x = m->m[0][0] * a->x + m->m[0][1] * a->y + m->m[0][2] * a->z;
	ret.y = m->m[1][0] * a->x + m->m[1][1] * a->y + m->m[1][2] * a->z;
	ret.z = m->m[2][0] * a->x + m->m[2][1] * a->y + m->m[2][2] * a->z;
	*r = ret;
}

void mat_mul_vector_T(struct vec *r, const struct mat *m, const struct vec *a)
{
	struct vec ret;
	ret.x = m->m[0][0] * a->x + m->m[1][0] * a->y + m->m[2][0] * a->z;
	ret.y = m->m[0][1] * a->x + m->m[1][1] * a->y + m->m[2][1] * a->z;
	ret.z = m->m[0][2] * a->x + m->m[1][2] * a->y + m->m[2][2] * a->z;
	*r = ret;
}

void mat_rot(struct mat *r, const struct vec *v, float alpha)
{
	float c = (float) cos(alpha);
	float s = (float) sin(alpha);
	float C = 1 - c;
	r->m[0][0] = v->x * v->x * C + c;
	r->m[0][1] = v->x * v->y * C - v->z * s;
	r->m[0][2] = v->x * v->z * C + v->y * s;
	r->m[0][3] = 0.0f;
	r->m[1][0] = v->y * v->x * C + v->z * s;
	r->m[1][1] = v->y * v->y * C + c;
	r->m[1][2] = v->y * v->z * C - v->x * s;
	r->m[1][3] = 0.0f;
	r->m[2][0] = v->z * v->x * C - v->y * s;
	r->m[2][1] = v->z * v->y * C + v->x * s;
	r->m[2][2] = v->z * v->z * C + c;
	r->m[2][3] = 0.0f;
	r->m[3][0] = 0.0f;
	r->m[3][1] = 0.0f;
	r->m[3][2] = 0.0f;
	r->m[3][3] = 1.0f;
}
