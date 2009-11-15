#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>

#define HUGE		(1e36f)
#define EPS		(1e-5f)
#define PI		((float) 3.14159265358979323846)
#define DEGREE		(2.0f * PI / 360.0f)

struct vec {
	float x, y, z;
};

struct mat {
	float m[4][4];
};

/* Vector operations */
extern const struct vec vec_null;

#define vec_set(r, xx, yy, zz)	((r)->x = (xx),\
				 (r)->y = (yy),\
				 (r)->z = (zz))

#define vec_neg(r)		((r)->x = -(r)->x,\
				 (r)->y = -(r)->y,\
				 (r)->z = -(r)->z)

#define vec_add(r, a, b)	((r)->x = (a)->x + (b)->x,\
				 (r)->y = (a)->y + (b)->y,\
				 (r)->z = (a)->z + (b)->z)

#define vec_sub(r, a, b)	((r)->x = (a)->x - (b)->x,\
				 (r)->y = (a)->y - (b)->y,\
				 (r)->z = (a)->z - (b)->z)

#define vec_mul(r, f)		((r)->x *= (f),\
				 (r)->y *= (f),\
				 (r)->z *= (f))

#define vec_div(r, d)		do { float _d = 1.0f / (d);\
				     vec_mul(r, _d); } while (0)

#define vec_mad(r, f, a)	((r)->x += (f) * (a)->x,\
				 (r)->y += (f) * (a)->y,\
				 (r)->z += (f) * (a)->z)

#define vec_dot(a, b)		((a)->x * (b)->x +\
				 (a)->y * (b)->y +\
				 (a)->z * (b)->z)

#define vec_norm(a)		((float) sqrt(vec_dot(a, a)))

#define vec_normalize(r)	do { float _n = vec_norm(r);\
				     vec_div(r, _n); } while (0)

#define vec_prod(r, a, b)	((r)->x = (a)->y * (b)->z - (a)->z * (b)->y,\
				 (r)->y = (a)->z * (b)->x - (a)->x * (b)->z,\
				 (r)->z = (a)->x * (b)->y - (a)->y * (b)->x)

#define vec_dist(a,b)		((float) sqrt(((a)->x - (b)->x) * ((a)->x - (b)->x)+\
					      ((a)->y - (b)->y) * ((a)->y - (b)->y)+ \
					      ((a)->z - (b)->z) * ((a)->z - (b)->z)))

#define vec_min(r, a, b)	(((r)->x = (a)->x < (b)->x ? (a)->x : (b)->x),\
				 ((r)->y = (a)->y < (b)->y ? (a)->y : (b)->y),\
				 ((r)->z = (a)->z < (b)->z ? (a)->z : (b)->z))

#define vec_max(r, a, b)	(((r)->x = (a)->x > (b)->x ? (a)->x : (b)->x),\
				 ((r)->y = (a)->y > (b)->y ? (a)->y : (b)->y),\
				 ((r)->z = (a)->z > (b)->z ? (a)->z : (b)->z))

#define vec_lerp(r, a, b, t)	((r)->x = (t) * (a)->x + (1.0f - (t)) * (b)->x,\
				 (r)->y = (t) * (a)->y + (1.0f - (t)) * (b)->y,\
				 (r)->z = (t) * (a)->z + (1.0f - (t)) * (b)->z)

/* Matrix operations */
extern const struct mat mat_null;
extern const struct mat mat_ident;

void mat_invert(struct mat *r, const struct mat *a);
void mat_prod(struct mat *r, const struct mat *a, const struct mat *b);

void mat_rot(struct mat *r, const struct vec *v, float alpha);

void mat_mul_point(struct vec *r, const struct mat *m, const struct vec *a);
void mat_mul_vector(struct vec *r, const struct mat *m, const struct vec *a);
void mat_mul_vector_T(struct vec *r, const struct mat *m, const struct vec *a);

#endif
