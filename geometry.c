#include <math.h>
#include "geometry.h"

const struct vec vec_null;
const struct mat mat_null;
const struct mat mat_ident = { { { 1, 0, 0, 0 },
				 { 0, 1, 0, 0 },
				 { 0, 0, 1, 0 },
				 { 0, 0, 0, 1 } } };

#define det2x2(a00,a01,a10,a11)				((a00) * (a11) - (a01) * (a10))
#define det3x3(a00,a01,a02,a10,a11,a12,a20,a21,a22)	((a00) * det2x2(a11,a12,a21,a22) -\
							 (a10) * det2x2(a01,a02,a21,a22) +\
							 (a20) * det2x2(a01,a02,a11,a12))

#define A(i,j)		(a->m[i][j])
#define B(i,j)		(b->m[i][j])
#define C(i,j)		(c.m[i][j])
#define R(i,j)		(r->m[i][j])

float mat_determinant(const struct mat *a)
{
	return  A(0,0) * det3x3(A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3)) -
		A(0,1) * det3x3(A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3)) +
		A(0,2) * det3x3(A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3)) -
		A(0,3) * det3x3(A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
}

static void mat_adjoint(struct mat *r, const struct mat *a)
{
	R(0,0) =  det3x3(A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3));
	R(0,1) = -det3x3(A(0,1),A(0,2),A(0,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3));
	R(0,2) =  det3x3(A(0,1),A(0,2),A(0,3),A(1,1),A(1,2),A(1,3),A(3,1),A(3,2),A(3,3));
	R(0,3) = -det3x3(A(0,1),A(0,2),A(0,3),A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3));

	R(1,0) = -det3x3(A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3));
	R(1,1) =  det3x3(A(0,0),A(0,2),A(0,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3));
	R(1,2) = -det3x3(A(0,0),A(1,0),A(3,0),A(0,2),A(1,2),A(3,2),A(0,3),A(1,3),A(3,3));
	R(1,3) =  det3x3(A(0,0),A(0,2),A(0,3),A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3));

	R(2,0) =  det3x3(A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3));
	R(2,1) = -det3x3(A(0,0),A(0,1),A(0,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3));
	R(2,2) =  det3x3(A(0,0),A(0,1),A(0,3),A(1,0),A(1,1),A(1,3),A(3,0),A(3,1),A(3,3));
	R(2,3) = -det3x3(A(0,0),A(0,1),A(0,3),A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3));

	R(3,0) = -det3x3(A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
	R(3,1) =  det3x3(A(0,0),A(0,1),A(0,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
	R(3,2) = -det3x3(A(0,0),A(0,1),A(0,2),A(1,0),A(1,1),A(1,2),A(3,0),A(3,1),A(3,2));
	R(3,3) =  det3x3(A(0,0),A(0,1),A(0,2),A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2));
}

void mat_invert(struct mat *r, const struct mat *a)
{
	int i, j;
	float det;

	mat_adjoint(r, a);
	det = mat_determinant(a);
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			R(i,j) = R(i,j) / det;
}

void mat_prod(struct mat *r, const struct mat *a, const struct mat *b)
{
	int i, j, k;
	struct mat c;

	c = mat_null;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				C(i,j) += A(i,k) * B(k,j);
	*r = c;
}

void mat_mul_point(struct vec *r, const struct mat *a, const struct vec *v)
{
	struct vec res;
	res.x = A(0,0) * v->x + A(0,1) * v->y + A(0,2) * v->z + A(0,3);
	res.y = A(1,0) * v->x + A(1,1) * v->y + A(1,2) * v->z + A(1,3);
	res.z = A(2,0) * v->x + A(2,1) * v->y + A(2,2) * v->z + A(2,3);
	*r = res;
}

void mat_mul_vector(struct vec *r, const struct mat *a, const struct vec *v)
{
	struct vec res;
	res.x = A(0,0) * v->x + A(0,1) * v->y + A(0,2) * v->z;
	res.y = A(1,0) * v->x + A(1,1) * v->y + A(1,2) * v->z;
	res.z = A(2,0) * v->x + A(2,1) * v->y + A(2,2) * v->z;
	*r = res;
}

void mat_mul_vector_T(struct vec *r, const struct mat *a, const struct vec *v)
{
	struct vec res;
	res.x = A(0,0) * v->x + A(1,0) * v->y + A(2,0) * v->z;
	res.y = A(0,1) * v->x + A(1,1) * v->y + A(2,1) * v->z;
	res.z = A(0,2) * v->x + A(1,2) * v->y + A(2,2) * v->z;
	*r = res;
}

void mat_rotate(struct mat *r, const struct vec *v, float angle)
{
	float c, s;
	struct vec a;

	c = (float) cos(angle);
	s = (float) sin(angle);
	a = *v;
	vec_normalize(&a);

	R(0,0) = (1.0f - c) * a.x * a.x + c;
	R(0,1) = (1.0f - c) * a.x * a.y - s * a.z;
	R(0,2) = (1.0f - c) * a.x * a.z + s * a.y;
	R(0,3) = 0;
	R(1,0) = (1.0f - c) * a.y * a.x + s * a.z;
	R(1,1) = (1.0f - c) * a.y * a.y + c;
	R(1,2) = (1.0f - c) * a.y * a.z - s * a.x;
	R(1,3) = 0;
	R(2,0) = (1.0f - c) * a.z * a.x - s * a.y;
	R(2,1) = (1.0f - c) * a.z * a.y + s * a.x;
	R(2,2) = (1.0f - c) * a.z * a.z + c;
	R(2,3) = 0;
	R(3,0) = 0;
	R(3,1) = 0;
	R(3,2) = 0;
	R(3,3) = 1;
}
