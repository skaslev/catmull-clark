#ifndef MATHX_H
#define MATHX_H

#include <math.h>
#include <string.h>

#define M_PIf		((float) M_PI)

static inline float degrees(float rad)
{
	return rad * 180.0f / M_PIf;
}

static inline float radians(float deg)
{
	return deg * M_PIf / 180.0f;
}

static inline float sqrf(float a)
{
	return a * a;
}

static inline float clampf(float a, float min, float max)
{
	return a < min ? min : a > max ? max : a;
}

static inline float minf(float a, float b)
{
	return a < b ? a : b;
}

static inline float maxf(float a, float b)
{
	return a < b ? b : a;
}

static inline float mixf(float a, float b, float t)
{
	return t * a + (1.0f - t) * b;
}

/* Vector operations */
typedef float vector[3];

static inline void vec_copy(vector r, const vector a)
{
	memcpy(r, a, sizeof(vector));
}

static inline void vec_zero(vector r)
{
	memset(r, 0, sizeof(vector));
}

static inline void vec_set(vector r, float x, float y, float z)
{
	r[0] = x;
	r[1] = y;
	r[2] = z;
}

static inline void vec_neg(vector r, const vector a)
{
	r[0] = -a[0];
	r[1] = -a[1];
	r[2] = -a[2];
}

static inline void vec_add(vector r, const vector a, const vector b)
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
}

static inline void vec_sub(vector r, const vector a, const vector b)
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
}

static inline void vec_mul(vector r, float f, const vector a)
{
	r[0] = f * a[0];
	r[1] = f * a[1];
	r[2] = f * a[2];
}

static inline void vec_mad(vector r, float f, const vector a)
{
	r[0] += f * a[0];
	r[1] += f * a[1];
	r[2] += f * a[2];
}

static inline float vec_dot(const vector a, const vector b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline float vec_len(const vector a)
{
	return sqrtf(vec_dot(a, a));
}

static inline void vec_normalize(vector r, const vector a)
{
	vec_mul(r, 1.0f / vec_len(a), a);
}

static inline void vec_cross(vector r, const vector a, const vector b)
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
}

static inline float vec_dist(const vector a, const vector b)
{
	vector d;

	vec_sub(d, a, b);
	return vec_len(d);
}

static inline void vec_min(vector r, const vector a, const vector b)
{
	r[0] = minf(a[0], b[0]);
	r[1] = minf(a[1], b[1]);
	r[2] = minf(a[2], b[2]);
}

static inline void vec_max(vector r, const vector a, const vector b)
{
	r[0] = maxf(a[0], b[0]);
	r[1] = maxf(a[1], b[1]);
	r[2] = maxf(a[2], b[2]);
}

static inline void vec_mix(vector r, const vector a, const vector b, float t)
{
	r[0] = mixf(a[0], b[0], t);
	r[1] = mixf(a[1], b[1], t);
	r[2] = mixf(a[2], b[2], t);
}

static inline void vec_spherical(vector r, float phi, float theta)
{
	r[0] = sinf(theta) * cosf(phi);
	r[1] = sinf(theta) * sinf(phi);
	r[2] = cosf(theta);
}

static inline void vec_to_spherical(const vector r, float *phi, float *theta)
{
	*theta = acosf(r[2]);
	*phi = atan2f(r[1], r[0]);
	if (*phi < 0.0f)
		*phi += 2.0f * M_PIf;
}

/* Matrix operations */
typedef float matrix[16];

static inline void mat_copy(matrix r, const matrix a)
{
	memcpy(r, a, sizeof(matrix));
}

static inline void mat_zero(matrix r)
{
	memset(r, 0, sizeof(matrix));
}

static inline void mat_identity(matrix r)
{
	static const matrix ident = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat_copy(r, ident);
}

float mat_determinant(const matrix m);
void mat_invert(matrix r, const matrix a);
void mat_adjoint(matrix r, const matrix a, float *det);

void mat_mul(matrix r, const matrix a, const matrix b);
void mat_mul_point(vector r, const matrix a, const vector v);
void mat_mul_vector(vector r, const matrix a, const vector v);

void mat_scale(matrix r, float s);
void mat_translate(matrix r, const vector v);
void mat_rotate(matrix r, const vector axis, float angle);
void mat_lookat(matrix r, const vector eye, const vector at, const vector up);
void mat_frame(matrix r, const vector o, const vector x, const vector y, const vector z);
void mat_frustum(matrix r, float left, float right, float bot, float top, float near, float far);
void mat_ortho(matrix r, float left, float right, float bot, float top, float near, float far);
void mat_persp(matrix r, float fovy, float aspect, float near, float far);

#endif
