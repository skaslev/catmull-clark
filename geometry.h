#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DEGREE		(2 * M_PI / 360)

struct vec {
	float x, y, z;
};

typedef float mat_t[4][4];

/* Vector operations */
extern const struct vec vec_null;

static inline void
vec_add(struct vec *r, const struct vec *a, const struct vec *b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
}

static inline void
vec_sub(struct vec *r, const struct vec *a, const struct vec *b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

static inline void
vec_mul(struct vec *r, float f)
{
	r->x *= f;
	r->y *= f;
	r->z *= f;
}

static inline void
vec_div(struct vec *r, float f)
{
	vec_mul(r, 1.0f / f);
}

static inline void
vec_mad(struct vec *r, float f, const struct vec *a)
{
	r->x += f * a->x;
	r->y += f * a->y;
	r->z += f * a->z;
}

static inline float
vec_dot(const struct vec *a, const struct vec *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline float
vec_norm(const struct vec *a)
{
	return sqrt(vec_dot(a, a));
}

static inline void
vec_normalize(struct vec *r)
{
	vec_mul(r, 1.0f / vec_norm(r));
}

static inline void
vec_prod(struct vec *r, const struct vec *a, const struct vec *b)
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
}

static inline float
vec_dist(const struct vec *a, const struct vec *b)
{
	struct vec d;
	vec_sub(&d, a, b);
	return vec_norm(&d);
}

static inline void
vec_min(struct vec *r, const struct vec *a, const struct vec *b)
{
	r->x = a->x < b->x ? a->x : b->x;
	r->y = a->y < b->y ? a->y : b->y;
	r->z = a->z < b->z ? a->z : b->z;
}

static inline void
vec_max(struct vec *r, const struct vec *a, const struct vec *b)
{
	r->x = a->x > b->x ? a->x : b->x;
	r->y = a->y > b->y ? a->y : b->y;
	r->z = a->z > b->z ? a->z : b->z;
}

static inline void
vec_lerp(struct vec *r, const struct vec *a, const struct vec *b, float t)
{
	r->x = t * a->x + (1.0f - t) * b->x;
	r->y = t * a->y + (1.0f - t) * b->y;
	r->z = t * a->z + (1.0f - t) * b->z;
}

/* Matrix operations */
static inline void
mat_null(mat_t r)
{
	memset(r, 0, sizeof(mat_t));
}

static inline void
mat_copy(mat_t r, const mat_t a)
{
	memcpy(r, a, sizeof(mat_t));
}

void mat_ident(mat_t r);
void mat_invert(mat_t r, mat_t a);
void mat_prod(mat_t r, mat_t a, mat_t b);

void mat_rot(mat_t m, const struct vec *v, float alpha);

void mat_mul_point(struct vec *r, mat_t m, const struct vec *a);
void mat_mul_vector(struct vec *r, mat_t m, const struct vec *a);
void mat_mul_vector_T(struct vec *r, mat_t m, const struct vec *a);

#endif
