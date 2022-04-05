#ifndef NB_MATH_FIXED_H
#define NB_MATH_FIXED_H

#include <nbug/core/debug.h>

namespace e
{
	struct fixed
	{
		static const int DECIMAL_SHIFT = 16;
		static const int INTEGER_MUL = (1 << DECIMAL_SHIFT);
		static const int DECIMAL_MASK = (INTEGER_MUL-1);

		uint64 v;

		fixed(){}
		fixed(int _i);
		fixed(float _r);
		fixed(double _r);

		void set_bare_value(uint64 _bare);
		uint64 get_bare_value() const;

		void set(float _r);
		void set(double _r);
		void set(int _i);

		fixed & operator = (float _r);
		fixed & operator = (double _r);
		fixed & operator = (int _r);
		operator int() const;
		operator float() const;
		operator double() const;

		fixed operator+(const fixed & _r) const;
		fixed operator-(const fixed & _r) const;
		fixed operator*(const fixed & _r) const;
		fixed operator/(const fixed & _r) const;
		bool operator>(const fixed & _r) const;
		bool operator>=(const fixed & _r) const;
		bool operator<(const fixed & _r) const;
		bool operator<=(const fixed & _r) const;
		bool operator==(const fixed & _r) const;

	};
}

#endif
