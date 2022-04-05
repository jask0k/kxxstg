
#include <string.h>
#include <nbug/math/math.h>

namespace e
{
/*
	const Matrix4 Matrix4::Identity = 
	{{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}};
	*/

	static inline void Mat4Mult(Matrix4 & C, const Matrix4 & A, const Matrix4 & B)
	{
		C.m[0][0]  = A.m[0][0] * B.m[0][0]  + A.m[1][0] * B.m[0][1]  + A.m[2][0]  * B.m[0][2]  + A.m[3][0] * B.m[0][3];
		C.m[0][1]  = A.m[0][1] * B.m[0][0]  + A.m[1][1] * B.m[0][1]  + A.m[2][1]  * B.m[0][2]  + A.m[3][1] * B.m[0][3];
		C.m[0][2]  = A.m[0][2] * B.m[0][0]  + A.m[1][2] * B.m[0][1]  + A.m[2][2] * B.m[0][2]  + A.m[3][2] * B.m[0][3];
		C.m[0][3]  = A.m[0][3] * B.m[0][0]  + A.m[1][3] * B.m[0][1]  + A.m[2][3] * B.m[0][2]  + A.m[3][3] * B.m[0][3];
		C.m[1][0]  = A.m[0][0] * B.m[1][0]  + A.m[1][0] * B.m[1][1]  + A.m[2][0]  * B.m[1][2]  + A.m[3][0] * B.m[1][3];
		C.m[1][1]  = A.m[0][1] * B.m[1][0]  + A.m[1][1] * B.m[1][1]  + A.m[2][1]  * B.m[1][2]  + A.m[3][1] * B.m[1][3];
		C.m[1][2]  = A.m[0][2] * B.m[1][0]  + A.m[1][2] * B.m[1][1]  + A.m[2][2] * B.m[1][2]  + A.m[3][2] * B.m[1][3];
		C.m[1][3]  = A.m[0][3] * B.m[1][0]  + A.m[1][3] * B.m[1][1]  + A.m[2][3] * B.m[1][2]  + A.m[3][3] * B.m[1][3];
		C.m[2][0]  = A.m[0][0] * B.m[2][0]  + A.m[1][0] * B.m[2][1]  + A.m[2][0]  * B.m[2][2] + A.m[3][0] * B.m[2][3];
		C.m[2][1]  = A.m[0][1] * B.m[2][0]  + A.m[1][1] * B.m[2][1]  + A.m[2][1]  * B.m[2][2] + A.m[3][1] * B.m[2][3];
		C.m[2][2] = A.m[0][2] * B.m[2][0]  + A.m[1][2] * B.m[2][1]  + A.m[2][2] * B.m[2][2] + A.m[3][2] * B.m[2][3];
		C.m[2][3] = A.m[0][3] * B.m[2][0]  + A.m[1][3] * B.m[2][1]  + A.m[2][3] * B.m[2][2] + A.m[3][3] * B.m[2][3];
		C.m[3][0] = A.m[0][0] * B.m[3][0] + A.m[1][0] * B.m[3][1] + A.m[2][0]  * B.m[3][2] + A.m[3][0] * B.m[3][3];
		C.m[3][1] = A.m[0][1] * B.m[3][0] + A.m[1][1] * B.m[3][1] + A.m[2][1]  * B.m[3][2] + A.m[3][1] * B.m[3][3];
		C.m[3][2] = A.m[0][2] * B.m[3][0] + A.m[1][2] * B.m[3][1] + A.m[2][2] * B.m[3][2] + A.m[3][2] * B.m[3][3];
		C.m[3][3] = A.m[0][3] * B.m[3][0] + A.m[1][3] * B.m[3][1] + A.m[2][3] * B.m[3][2] + A.m[3][3] * B.m[3][3];
	}

	void  Matrix4::Multiply(const Matrix4 & _r)
	{
		Matrix4 A;
		memcpy((float*)&A, m, sizeof(A));
		Mat4Mult(*this, A, _r);
	}

	Matrix4 Matrix4::operator*(const Matrix4 & _r) const
	{
		Matrix4 C;
		Mat4Mult(C, *this, _r);
		return C;
	}

	Vector4 Matrix4::operator*(const Vector4 & _r) const
	{
		Vector4 ret;
		ret.x = _r.x * m[0][0] + _r.y * m[1][0] + _r.z * m[2][0]  + _r.w * m[3][0];
		ret.y = _r.x * m[0][1] + _r.y * m[1][1] + _r.z * m[2][1]  + _r.w * m[3][1];
		ret.z = _r.x * m[0][2] + _r.y * m[1][2] + _r.z * m[2][2] + _r.w * m[3][2];
		ret.w = _r.x * m[0][3] + _r.y * m[1][3] + _r.z * m[2][3] + _r.w * m[3][3];
		return ret;
	}

	Vector3 Matrix4::operator*(const Vector3 & _r) const
	{
		Vector3 ret;
		ret.x = _r.x * m[0][0] + _r.y * m[1][0] + _r.z * m[2][0]  + m[3][0];
		ret.y = _r.x * m[0][1] + _r.y * m[1][1] + _r.z * m[2][1]  + m[3][1];
		ret.z = _r.x * m[0][2] + _r.y * m[1][2] + _r.z * m[2][2] +  m[3][2];
		return ret;
	}

	void Matrix4::Transpose()
	{
		real a;
		a = m[0][1]; m[0][1] = m[1][0];  m[1][0] = a;
		a = m[0][2]; m[0][2] = m[2][0];  m[2][0] = a;
		a = m[0][3]; m[0][3] = m[3][0];  m[3][0] = a;
		a = m[1][2]; m[1][2] = m[2][1];  m[2][1] = a;
		a = m[1][3]; m[1][3] = m[3][1];  m[3][1] = a;
		a = m[2][3]; m[2][3] = m[3][2];  m[3][2] = a;
	}

	void Matrix4::RotateX(real _angle_radian)
	{
		real cos_a = cos(-_angle_radian);
		real sin_a = sin(-_angle_radian);
		Matrix4 rot;
		rot.LoadIdentity();
		rot.m[1][1] = cos_a;  rot.m[2][1] = sin_a;
		rot.m[1][2] = -sin_a; rot.m[2][2] = cos_a;
		Multiply(rot);
	}

	void Matrix4::RotateY(real _angle_radian)
	{
		real cos_a = cos(-_angle_radian);
		real sin_a = sin(-_angle_radian);
		Matrix4 rot;
		rot.LoadIdentity();
		rot.m[0][0] = cos_a; rot.m[2][0] = -sin_a;
		rot.m[0][2] = sin_a; rot.m[2][2] = cos_a;
		Multiply(rot);
	}

	void Matrix4::RotateZ(real _angle_radian)
	{
		real cos_a = cos(-_angle_radian);
		real sin_a = sin(-_angle_radian);
		Matrix4 rot;
		rot.LoadIdentity();
		rot.m[0][0] = cos_a;  rot.m[1][0] = sin_a;
		rot.m[0][1] = -sin_a; rot.m[1][1] = cos_a;
		Multiply(rot);
	}

	void Matrix4::Translate(real _dx, real _dy, real _dz)
	{
		Matrix4 trans;
		trans.LoadIdentity();
		trans.m[3][0] = _dx;
		trans.m[3][1] = _dy;
		trans.m[3][2] = _dz;
		Multiply(trans);
	}

	void Matrix4::Scale(real _sx, real _sy, real _sz)
	{
		Matrix4 n;
		n.LoadIdentity();
		n.m[0][0] = _sx;
		n.m[1][1] = _sy;
		n.m[2][2] = _sz;
		Multiply(n);
	}

	void Matrix4::Rotate(float _angle_radian, float _x, float _y, float _z)
	{
		Vector3 u = {_x, _y, _z};
		u.Normalize();

		real ux = u.x;
		real uy = u.y;
		real uz = u.z;

		real ux2 = u.x * u.x;
		real uy2 = u.y * u.y;
		real uz2 = u.z * u.z;

		real uxy = u.x * u.y;
		real uyz = u.y * u.z;
		real uxz = u.x * u.z;

		real c = cos(_angle_radian);
		real s = sin(_angle_radian);

		real c1 = 1 - c;

		Matrix4 rot;
		rot.LoadIdentity();

		rot.m[0][0] = ux2 + (1 - ux2) * c;
		rot.m[1][0] = uxy * c1 - uz * s;
		rot.m[2][0] = uxz * c1 + uy * s;

		rot.m[0][1] = uxy * c1 + uz * s;
		rot.m[1][1] = uy2 + (1 - uy2) * c;
		rot.m[2][1] = uyz * c1 - ux * s;

		rot.m[0][2] = uxz * c1 - uy * s;
		rot.m[1][2] = uyz * c1 + ux * s;
		rot.m[2][2] = uz2 + (1 - uz2) * c;

		Multiply(rot);
	}

	float PointLineSegDistanceSquared2D(const Vector2 & q, const Vector2 & p0, const Vector2 & p1, float & t)
	{
		Vector2 dir = p1 - p0;
		Vector2 qp0 = q - p0;
		t = dir.Dot(qp0);
		if(t <= 0)
		{
			return qp0.Dot(qp0);
		}

		float ddd = dir.Dot(dir);
		if(t >= ddd)
		{
			Vector2 qp1 = q - p1;
			return qp1.Dot(qp1);
		}

		return qp0.Dot(qp0) - t * t / ddd;
	}

	float PointLineDistanceSquared3D(const Vector3 & q, const Vector3 & p0, const Vector3 & dir, bool normalized, float & t)
	{	
		t = dir.Dot(q - p0);
		if(!normalized)
		{
			t/= dir.Dot(dir);
		}
		Vector3 vec = q - (p0 + dir * t);
		return vec.Dot(vec);
	}

	float PointLineSegDistanceSquared3D(const Vector3 & q, const Vector3 & p0, const Vector3 & p1, float & t)
	{
		Vector3 dir = p1 - p0;
		float distanceSquared = PointLineDistanceSquared3D(q, p0, dir, false, t);
		if(t < 0)
		{
			t=0;
			Vector3 vec = q - p0;
			distanceSquared = vec.Dot(vec);
		}
		else if(t > 1)
		{
			t=1;
			Vector3 vec = q - p1;
			distanceSquared = vec.Dot(vec);
		}
		return distanceSquared;
	}

	// TODO: shall not PointLineSegDistanceSquared3D?
	float PointTriangleDistanceSquared3D(const Vector3 & p, const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	{
		Vector3 e0 = v1 - v0;
		Vector3 e1 = v2 - v0;
		Vector3 vd = v0 - p;

		// Q(s,t) = a*s*s + 2*b*s*t + c*t*t +2*d*s + 2*e*t + f
		float a = e0.Dot(e0);
		float b = e0.Dot(e1);
		float c = e1.Dot(e1);
		float d = e0.Dot(vd);
		float e = e1.Dot(vd);
		float f = vd.Dot(vd);

		float det = a*c - b*b;
		float s = b*e - c*d;
		float t = b*d - a*e;
		// \2^ t
		//  \| 
		//   \ 
		//   |\ 
		//   | \  1
		// 3 |  \
		//   | 0 \
		// --+----\-> s
		// 4 | 5   \ 6

		float ret;
		if(s + t <= det)
		{
			if(s < 0)
			{
				if(t < 0)
				{
					//region = 4;
					ret = (p - v0).LengthSquared();
				}
				else
				{
					//region = 3;
					float t;
					ret =  PointLineSegDistanceSquared3D(p, v0, v2, t);
				}
			}
			else
			{
				if(t < 0)
				{
					//region = 5;
					float t;
					ret =  PointLineSegDistanceSquared3D(p, v0, v1, t);
				}
				else
				{
					//region = 0;
					float invDet = 1 / det;
					s*= invDet;
					t*= invDet;
					ret = (p - (v0 + e0*s + e1*t)).LengthSquared();
				}
			}
		}
		else
		{
			if(s < 0)
			{
				//region = 2;
				ret = (p - v2).LengthSquared();
			}
			else if(t < 0)
			{
				//region = 6;
				ret = (p - v1).LengthSquared();
			}
			else
			{
				//region = 1;
				float t;
				ret = PointLineSegDistanceSquared3D(p, v1, v2, t);
			}
		}
		return ret;
	}

	Vector3 CalcNormal(const Vector3 & _center, const Vector3 _v[], int _n)
	{
		if(_n > 20)
		{
			_n = 20;
		}
		Vector3 normal;

		Vector3 t[20];
		for(int i=0; i<_n; i++)
		{
			t[i] = _v[i] - _center;
		}

		for(int i=0; i<_n - 1; i++)
		{
			normal+= t[i].Cross(t[i+1]).GetNormal();
		}

		return normal / float(_n-1);
	}





#ifdef E_CFG_UNIT_TEST
	E_UNIT_TEST_CASE(Vector2D_Precision)
	{
		for(float a = -PI + 0.001f; a < PI; a+= 0.001f)
		{
			Vector2 v = {cosf(a), sinf(a)};
			float v_angle = v.Angle();
			E_UNIT_TEST_ASSERT(E_ABS(v_angle - a) < 0.000001f);
		}		
		return true;
	}

	//E_UNIT_TEST_CASE(FastSqrt_Precision)
	//{
	//	//float error_max = 0;
	//	//float error_max_float = 0;
	//	for(float a = 1; a < 1E60; a*=1.01f)
	//	{
	//		float sqrt_a = sqrt(a);
	//		float e_fast_sqrt_a = e_fast_sqrt(a);
	//		float error_e_fast_sqrt = E_ABS(sqrt_a - e_fast_sqrt_a) / sqrt_a;
	//		//if(error_max < error_e_fast_sqrt)
	//		//{
	//		//	error_max = error_e_fast_sqrt;
	//		//	error_max_float = a;
	//		//}
	//	//	E_TRACE_LINE(string(error_e_fast_sqrt_a));
	//		E_UNIT_TEST_ASSERT(error_e_fast_sqrt < 0.005f);
	//	}
	//	//E_TRACE_LINE(string(error_max));
	//	//E_TRACE_LINE(string(error_max_float));
	//	return true;
	//}

	//E_UNIT_TEST_CASE(FastTriFunc_Precision)
	//{
	//	//e_math_init();
	////	float error_max = 0;
	////	float error_max_float = 0;
	//	for(float a = -100; a < 100; a+= 0.001f)
	//	{
	//		float sin_a = sin(a);
	//		float e_fast_sin_a = e_fast_sin(a);
	//		float error_e_fast_sin_a = E_ABS(sin_a - e_fast_sin_a);
	//		E_ASSERT(error_e_fast_sin_a < 0.01f);
	//		E_UNIT_TEST_ASSERT(error_e_fast_sin_a < 0.01f);
	//	}
	//	for(float a = -100; a < 100; a+= 0.001f)
	//	{
	//		float cos_a = cos(a);
	//		float e_fast_cos_a = e_fast_cos(a);
	//		float error_e_fast_cos_a = E_ABS(cos_a - e_fast_cos_a);
	//		E_ASSERT(error_e_fast_cos_a < 0.01f);
	//		E_UNIT_TEST_ASSERT(error_e_fast_cos_a < 0.01f);
	//	}
	////	E_TRACE_LINE(string(error_max));
	////	E_TRACE_LINE(string(error_max_float));
	//	return true;
	//}
#endif
}
