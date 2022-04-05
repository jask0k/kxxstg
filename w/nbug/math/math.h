
#ifndef NB_MATH_H
#define NB_MATH_H

#include <float.h>
#include <math.h>
#include <nbug/core/debug.h>

#ifdef NB_WINDOWS
#	define finite _finite
#endif

namespace e
{
//#ifdef E_REAL_IS_DOUBLE
//	#define E_M_E		2.71828182845904523536
//	#define E_M_LOG2E   1.44269504088896340736
//	#define E_M_LOG10E  0.434294481903251827651
//	#define E_M_LN2     0.693147180559945309417
//	#define E_M_LN10    2.30258509299404568402
//	#define PI      3.14159265358979323846
//	#define E_M_SQRT2   1.41421356237309504880
//#else
//	#define E_M_E       2.718281828f
//	#define E_M_LOG2E   1.442695041f
//	#define E_M_LOG10E  0.434294482f
//	#define E_M_LN2     0.693147181f
//	#define E_M_LN10    2.302585093f
//	#define PI      3.141592654f
//	#define E_M_SQRT2   1.414213562f
//#endif
	static const real PI = 3.141592654f;

	inline static float frand()
	{ return float(rand()) / float(RAND_MAX); }

	inline static real DegToRad(real _deg)
	{ return _deg * PI / 180.0f; }

	inline static real RadToDeg(real _rad)
	{ return _rad * 180.0f / PI; }

	struct Vector2
	{
		real x, y;

		bool operator!=(const Vector2 & _r) const
		{ return x != _r.x || y != _r.y; }

		real Dot(const Vector2 & _r) const
		{return x * _r.x + y * _r.y; }

		real Kross(const Vector2 & _r) const
		{ return x * _r.y - _r.x * y; }

		Vector2 operator-(const Vector2 & _r) const
		{ Vector2 ret; ret.x = x - _r.x; ret.y = y - _r.y; return ret; }

		Vector2 operator-() const
		{ Vector2 ret; ret.x = -x; ret.y = -y; return ret; }

		Vector2 operator*(real _r) const
		{ Vector2 ret; ret.x = x * _r; ret.y = y * _r; return ret; }

		Vector2 operator/(real _r) const
		{ Vector2 ret; ret.x = x / _r; ret.y = y / _r; return ret; }

		real length() const
		{ return sqrt(x * x + y * y); }

		real LengthSquared() const
		{ return x * x + y * y; }

		real Angle() const
		{ 
			float ret;
			float abs_x = x < 0 ? -x : x;
			float abs_y = y < 0 ? -y : y;
			if(abs_x >= abs_y)
			{
				ret = atan2(y, x);
			}
			else
			{
				ret = PI / 2 - atan2(x, y);
				if(ret >= PI)
				{
					ret-= PI * 2;
				}
			}
			return ret;
		}

		Vector2 operator+(const Vector2 & _r) const
		{ Vector2 ret; ret.x = x + _r.x; ret.y = y + _r.y; return ret; }

		Vector2 GetPrep() const
		{ Vector2 ret; ret.x = -y; ret.y = x; return ret; }

		void Prep()
		{ real t = x; x = -y; y = t; }

		void Normalize()
		{
			real a = sqrt(x * x + y * y);
			if(a!=0)
			{
				x/=a;
				y/=a;
			}
		}

		Vector2 GetNormal() const
		{
			float a = sqrt(x * x + y * y);
			if(a!=0)
			{
				Vector2 ret;
				ret.x = x / a;
				ret.y = y / a;
				return ret;
			}
			else
			{
				return *this;
			}
		}

		void SetPolar(real _r, real _a)
		{ x = _r * cos(_a); y = _r * sin(_a); }

		void GetPolar(real & _r, real & _a) const
		{ _r = length(); _a = Angle(); }

		void operator+=(const Vector2 & _r)
		{ x+= _r.x; y+= _r.y; }

		void operator-=(const Vector2 & _r)
		{ x-= _r.x;	y-= _r.y; }
	};

	struct Vector3
	{
		float x, y, z;

		bool operator!=(const Vector3 & _r) const
		{ return x != _r.x || y != _r.y || z != _r.z;}

		void Normalize()
		{
			float a = sqrt(x * x + y * y + z * z);
			if(a!=0)
			{
				x/= a;
				y/= a;
				z/= a;
			}
		}

		Vector3 GetNormal()
		{
			float a = sqrt(x * x + y * y + z * z);
			if(a!=0)
			{
				Vector3 ret = {x/a, y/a, z/a};
				return ret;
			}
			else
			{
				return *this;
			}
		}

		float Dot(const Vector3 & _r) const
		{ return x*_r.x + y*_r.y + z*_r.z; }

		Vector3 Cross(const Vector3 & _r) const
		{ Vector3 ret = {y * _r.z - z * _r.y,  z * _r.x - x * _r.z, x * _r.y - y * _r.x}; return ret; }

		Vector3 operator+(const Vector3 & _r) const
		{ Vector3 ret = {x+_r.x, y+_r.y, z+_r.z}; return ret;}

		Vector3 operator-(const Vector3 & _r) const
		{ Vector3 ret = {x - _r.x, y - _r.y, z - _r.z}; return ret; }

		Vector3 operator*(float _a) const
		{ Vector3 ret = {x * _a, y * _a, z * _a}; return ret; };

		Vector3 operator-() const
		{ Vector3 ret = {-x, -y, -z}; return ret; }

		Vector3 operator/(float _a) const
		{ Vector3 ret = {x / _a, y / _a, z / _a}; return ret; };


		real length() const
		{ return sqrt(x * x + y * y + z * z); }

		real LengthSquared() const
		{ return x * x + y * y + z * z; }

		void operator+=(const Vector3 & _r)
		{ x+= _r.x;	y+= _r.y; z+= _r.z;	}

		void operator-=(const Vector3 & _r)
		{ x-= _r.x;	y-= _r.y; z-= _r.z;	}
	};

	struct Vector4
	{
		real x, y, z, w;

		bool operator!=(const Vector4 & _r) const
		{ return x != _r.x || y != _r.y || z != _r.z || w != _r.w;}

		void Normalize()
		{
			float a = sqrt(x * x + y * y + z * z + w * w);
			if(a!=0)
			{
				x/= a;
				y/= a;
				z/= a;
				w/= a;
			}
		}

		Vector4 GetNormal()
		{
			float a = sqrt(x * x + y * y + z * z + w * w);
			if(a!=0)
			{
				Vector4 ret = {x/a, y/a, z/a, w/a};
				return ret;
			}
			else
			{
				return *this;
			}
		}

		float Dot(const Vector4 & _r) const
		{ return x*_r.x + y*_r.y + z*_r.z + w*_r.w; }

		Vector4 operator+(const Vector4 & _r) const
		{ Vector4 ret = {x + _r.x, y + _r.y, z + _r.z, w + _r.w}; return ret; }

		Vector4 operator-(const Vector4 & _r) const
		{ Vector4 ret = {x - _r.x, y - _r.y, z - _r.z, w - _r.w}; return ret; }

		Vector4 operator*(float _a) const
		{ Vector4 ret = {x * _a, y * _a, z * _a, w * _a}; return ret; }

		Vector4 operator-() const
		{ Vector4 ret = {-x, -y, -z, -w}; return ret; }

		void operator+=(const Vector4 & _r)
		{ x+= _r.x; y+= _r.y; z+= _r.z; w+= _r.w; }

		void operator-=(const Vector4 & _r)
		{ x-= _r.x;	y-= _r.y; z-= _r.z; w-= _r.w; }
	};

	// matrix4x4 in column-major order, i.e. m[col][row]
	// You should init Matrix4 with following syntax, 
	// but keep in mind the "rows" are actually "cols" here.
	// Matrix4 m = 
	// {{
	//    1, 0, 0, 0,
	//    0, 1, 0, 0,
	//    0, 0, 1, 0,
	//    0, 0, 0, 1,
	// }};

	struct Matrix4
	{
		real m[4][4];

		void LoadIdentity()
		{ *this = Identity(); }

		void Multiply(const Matrix4 & _r);
		void operator*=(const Matrix4 & _r)
		{ Multiply(_r); }

		Matrix4 operator*(const Matrix4 & _r) const;
		void Transpose();
		Vector4 operator*(const Vector4 & _r) const;
		Vector3 operator*(const Vector3 & _r) const;

		void RotateX(real _angle_radian);
		void RotateY(real _angle_radian);
		void RotateZ(real _angle_radian);
		void Translate(real _dx, real _dy, real _dz);
		void Scale(real _sx, real _sy, real _sz);
		void Rotate(float _angle_radian, float _x, float _y, float _z);

		static const Matrix4 & Identity()
		{
			static const Matrix4 _identity =
			{{
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1,
			}};
			return _identity;
		}

		float * operator[](int _col)
		{ E_ASSERT(_col >= 0 && _col <= 3); return m[_col]; }

		const float * operator[](int _col) const
		{ E_ASSERT(_col >= 0 && _col <= 3); return m[_col]; }
	};

	float PointLineSegDistanceSquared2D(const Vector2 & q, const Vector2 & p0, const Vector2 & p1, float & t);
	float PointLineDistanceSquared3D(const Vector3 & q, const Vector3 & p0, const Vector3 & dir, bool normalized, float & t);
	float PointLineSegDistanceSquared3D(const Vector3 & q, const Vector3 & p0, const Vector3 & p1, float & t);
	float PointTriangleDistanceSquared3D(const Vector3 & p, const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);
	Vector3 CalcNormal(const Vector3 & _center, const Vector3 _v[], int _n);


	static inline float angle_normalize(float _a)
	{ 
		uint32 mask = 0x80000000 & (*reinterpret_cast<uint32*>(&_a));
		float b = PI;
		*reinterpret_cast<uint32*>(&b)|= mask;
		return _a - int((_a+b)*0.5f/PI) * PI*2;
	}

	static inline float angle_limit(float _a, float _limit)
	{
		E_ASSERT(_limit > 0);
		if(_a > _limit)
		{
			_a = _limit;
		}
		else if(_a < -_limit)
		{
			_a = -_limit;
		}
		return _a;
	}

	//static inline float calc_normalized_angle_delta(float _a0, float _a1)
	//{
	//	float da = _a1 - _a0;
	//	normalize_angle(da);
	//	return da;
	//}

}

#endif


