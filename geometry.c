#include <math.h>
#include "geometry.h"

const struct vec vec_null;

static const mat_t identity = { { 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 1 } };

void mat_ident(mat_t r)
{
	mat_copy(r, identity);
}

void mat_invert(mat_t r, mat_t a)
{
	/* TODO */
}

void mat_prod(mat_t r, mat_t a, mat_t b)
{
	int i,j,k;

	mat_null(r);
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				r[i][j] += a[i][k] * b[k][j];
}

void mat_mul_point(struct vec *r, mat_t m, const struct vec *a)
{
	struct vec ret;
	ret.x = m[0][0] * a->x + m[0][1] * a->y + m[0][2] * a->z + m[0][3];
	ret.y = m[1][0] * a->x + m[1][1] * a->y + m[1][2] * a->z + m[1][3];
	ret.z = m[2][0] * a->x + m[2][1] * a->y + m[2][2] * a->z + m[2][3];
	*r = ret;
}

void mat_mul_vector(struct vec *r, mat_t m, const struct vec *a)
{
	struct vec ret;
	ret.x = m[0][0] * a->x + m[0][1] * a->y + m[0][2] * a->z;
	ret.y = m[1][0] * a->x + m[1][1] * a->y + m[1][2] * a->z;
	ret.z = m[2][0] * a->x + m[2][1] * a->y + m[2][2] * a->z;
	*r = ret;
}

void mat_mul_vector_T(struct vec *r, mat_t m, const struct vec *a)
{
	struct vec ret;
	ret.x = m[0][0] * a->x + m[1][0] * a->y + m[2][0] * a->z;
	ret.y = m[0][1] * a->x + m[1][1] * a->y + m[2][1] * a->z;
	ret.z = m[0][2] * a->x + m[1][2] * a->y + m[2][2] * a->z;
	*r = ret;
}

void mat_rot(mat_t m, const struct vec *v, float alpha)
{
	float c = cos(alpha);
	float s = sin(alpha);
	float C = 1 - c;
	m[0][0] = v->x * v->x * C + c;
	m[0][1] = v->x * v->y * C - v->z * s;
	m[0][2] = v->x * v->z * C + v->y * s;
	m[0][3] = 0.0f;
	m[1][0] = v->y * v->x * C + v->z * s;
	m[1][1] = v->y * v->y * C + c;
	m[1][2] = v->y * v->z * C - v->x * s;
	m[1][3] = 0.0f;
	m[2][0] = v->z * v->x * C - v->y * s;
	m[2][1] = v->z * v->y * C + v->x * s;
	m[2][2] = v->z * v->z * C + c;
	m[2][3] = 0.0f;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}
