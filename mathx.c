#include "mathx.h"

#define det2(a00,a01,a10,a11)				((a00)*(a11) - (a01)*(a10))
#define det3(a00,a01,a02,a10,a11,a12,a20,a21,a22)	((a00)*det2(a11,a12,a21,a22)- \
							 (a10)*det2(a01,a02,a21,a22)+ \
							 (a20)*det2(a01,a02,a11,a12))

#define M(m,i,j)	(m[i+4*j])
#define A(i,j)		M(a,i,j)
#define B(i,j)		M(b,i,j)
#define C(i,j)		M(c,i,j)
#define R(i,j)		M(r,i,j)

float mat_determinant(const matrix a)
{
	return	A(0,0)*det3(A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3))-
		A(0,1)*det3(A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3))+
		A(0,2)*det3(A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3))-
		A(0,3)*det3(A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
}

void mat_adjoint(matrix r, const matrix a, float *det)
{
	R(0,0) =  det3(A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3));
	R(0,1) = -det3(A(0,1),A(0,2),A(0,3),A(2,1),A(2,2),A(2,3),A(3,1),A(3,2),A(3,3));
	R(0,2) =  det3(A(0,1),A(0,2),A(0,3),A(1,1),A(1,2),A(1,3),A(3,1),A(3,2),A(3,3));
	R(0,3) = -det3(A(0,1),A(0,2),A(0,3),A(1,1),A(1,2),A(1,3),A(2,1),A(2,2),A(2,3));

	R(1,0) = -det3(A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3));
	R(1,1) =  det3(A(0,0),A(0,2),A(0,3),A(2,0),A(2,2),A(2,3),A(3,0),A(3,2),A(3,3));
	R(1,2) = -det3(A(0,0),A(1,0),A(3,0),A(0,2),A(1,2),A(3,2),A(0,3),A(1,3),A(3,3));
	R(1,3) =  det3(A(0,0),A(0,2),A(0,3),A(1,0),A(1,2),A(1,3),A(2,0),A(2,2),A(2,3));

	R(2,0) =  det3(A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3));
	R(2,1) = -det3(A(0,0),A(0,1),A(0,3),A(2,0),A(2,1),A(2,3),A(3,0),A(3,1),A(3,3));
	R(2,2) =  det3(A(0,0),A(0,1),A(0,3),A(1,0),A(1,1),A(1,3),A(3,0),A(3,1),A(3,3));
	R(2,3) = -det3(A(0,0),A(0,1),A(0,3),A(1,0),A(1,1),A(1,3),A(2,0),A(2,1),A(2,3));

	R(3,0) = -det3(A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
	R(3,1) =  det3(A(0,0),A(0,1),A(0,2),A(2,0),A(2,1),A(2,2),A(3,0),A(3,1),A(3,2));
	R(3,2) = -det3(A(0,0),A(0,1),A(0,2),A(1,0),A(1,1),A(1,2),A(3,0),A(3,1),A(3,2));
	R(3,3) =  det3(A(0,0),A(0,1),A(0,2),A(1,0),A(1,1),A(1,2),A(2,0),A(2,1),A(2,2));

	if (det)
		*det =	A(0,0) * R(0,0) + A(0,1) * R(1,0) +
			A(0,2) * R(2,0) + A(0,3) * R(3,0);
}

void mat_invert(matrix r, const matrix a)
{
	int i, j;
	float det, inv_det;

	mat_adjoint(r, a, &det);
	inv_det = 1.0f / det;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			R(i,j) *= inv_det;
}

void mat_mul(matrix r, const matrix a, const matrix b)
{
	int i, j, k;
	matrix c;

	mat_zero(c);
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				C(i,j) += A(i,k) * B(k,j);
	mat_copy(r, c);
}

void mat_mul_point(vector r, const matrix a, const vector v)
{
	vector res;

	res[0] = A(0,0) * v[0] + A(0,1) * v[1] + A(0,2) * v[2] + A(0,3);
	res[1] = A(1,0) * v[0] + A(1,1) * v[1] + A(1,2) * v[2] + A(1,3);
	res[2] = A(2,0) * v[0] + A(2,1) * v[1] + A(2,2) * v[2] + A(2,3);
	vec_copy(r, res);
}

void mat_mul_vector(vector r, const matrix a, const vector v)
{
	vector res;

	res[0] = A(0,0) * v[0] + A(0,1) * v[1] + A(0,2) * v[2];
	res[1] = A(1,0) * v[0] + A(1,1) * v[1] + A(1,2) * v[2];
	res[2] = A(2,0) * v[0] + A(2,1) * v[1] + A(2,2) * v[2];
	vec_copy(r, res);
}

void mat_scale(matrix r, float s)
{
	mat_identity(r);
	R(0,0) = R(1,1) = R(2,2) = s;
}

void mat_translate(matrix r, const vector v)
{
	mat_identity(r);
	R(0,3) = v[0];
	R(1,3) = v[1];
	R(2,3) = v[2];
}

void mat_rotate(matrix r, const vector axis, float angle)
{
	vector a;
	float c, s;

	vec_normalize(a, axis);
	c = cosf(radians(angle));
	s = sinf(radians(angle));

	mat_identity(r);
	R(0,0) = a[0] * a[0] * (1.0f - c) + c;
	R(0,1) = a[0] * a[1] * (1.0f - c) - a[2] * s;
	R(0,2) = a[0] * a[2] * (1.0f - c) + a[1] * s;
	R(1,0) = a[0] * a[1] * (1.0f - c) + a[2] * s;
	R(1,1) = a[1] * a[1] * (1.0f - c) + c;
	R(1,2) = a[1] * a[2] * (1.0f - c) - a[0] * s;
	R(2,0) = a[0] * a[2] * (1.0f - c) - a[1] * s;
	R(2,1) = a[1] * a[2] * (1.0f - c) + a[0] * s;
	R(2,2) = a[2] * a[2] * (1.0f - c) + c;
}

void mat_lookat(matrix r, const vector eye, const vector at, const vector up)
{
	vector x, y, z;

	vec_sub(z, eye, at);
	vec_normalize(z, z);
	vec_cross(x, up, z);
	vec_normalize(x, x);
	vec_cross(y, z, x);
	mat_frame(r, eye, x, y, z);
}

void mat_frame(matrix r, const vector o, const vector x, const vector y, const vector z)
{
	vector p;
	matrix m;

	mat_identity(r);
	R(0,0) = x[0]; R(0,1) = x[1]; R(0,2) = x[2];
	R(1,0) = y[0]; R(1,1) = y[1]; R(1,2) = y[2];
	R(2,0) = z[0]; R(2,1) = z[1]; R(2,2) = z[2];
	vec_neg(p, o);
	mat_translate(m, p);
	mat_mul(r, r, m);
}

void mat_frustum(matrix r, float left, float right, float bot, float top, float near, float far)
{
	mat_zero(r);
	R(0,0) = 2 * near / (right - left);
	R(0,2) = (right + left) / (right - left);
	R(1,1) = 2 * near / (top - bot);
	R(1,2) = (top + bot) / (top - bot);
	R(2,2) = -(far + near) / (far - near);
	R(2,3) = -(2 * far * near) / (far - near);
	R(3,2) = -1;
}

void mat_ortho(matrix r, float left, float right, float bot, float top, float near, float far)
{
	mat_identity(r);
	R(0,0) = 2 / (right - left);
	R(0,3) = -(right + left) / (right - left);
	R(1,1) = 2 / (top - bot);
	R(1,3) = -(top + bot) / (top - bot);
	R(2,2) = -2 / (far - near);
	R(2,3) = -(far + near) / (far - near);
}

void mat_persp(matrix r, float fovy, float aspect, float near, float far)
{
	float top, right;

	top = near * tanf(radians(fovy) / 2.0f);
	right = aspect * top ;
	mat_frustum(r, -right, right, -top, top, near, far);
}
