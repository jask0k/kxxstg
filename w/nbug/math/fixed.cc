#include <nbug/math/fixed.h>

namespace e
{
	fixed::fixed(int _i)
	{ set(_i); }

	fixed::fixed(float _r)
	{ set(_r); }

	fixed::fixed(double _r)
	{ set(_r); }

	void fixed::set_bare_value(uint64 _bare)
	{ v = _bare; }

	uint64 fixed::get_bare_value() const
	{ return v; }

	void fixed::set(float _r)
	{
		int32 i = int32(_r);
		int32 d = int32((_r - i) * INTEGER_MUL) & 0x7fffffff;
		v = (uint64(i) << DECIMAL_SHIFT) | d;
	}

	void fixed::set(double _r)
	{
		int32 i = int32(_r);
		int32 d = int32((_r - i) * INTEGER_MUL) & 0x7fffffff;
		v = (uint64(i) << DECIMAL_SHIFT) | d;
	}

	void fixed::set(int _i)
	{ v = (uint64(_i) << DECIMAL_SHIFT); }

	fixed::operator int() const
	{ return int(v >> DECIMAL_SHIFT); }

	fixed::operator float() const
	{
		int i = (int)(v >> DECIMAL_SHIFT);
		int d = v & DECIMAL_MASK;
		return i + d / (float)INTEGER_MUL;
	}

	fixed::operator double() const
	{
		int i = (int)(v >> DECIMAL_SHIFT);
		int d = v & DECIMAL_MASK;
		return i + d / (double)INTEGER_MUL;
	}

	fixed & fixed::operator=(float _r)
	{ set(_r); return *this; }

	fixed & fixed::operator=(double _r)
	{ set(_r); return *this; }

	fixed & fixed::operator=(int _r)
	{ set(_r); return * this; }


	fixed fixed::operator+(const fixed & _r) const
	{ fixed ret(_r); ret.v+= this->v; return ret; }

	fixed fixed::operator-(const fixed & _r) const
	{ fixed ret(_r); ret.v-= this->v; return ret; }

	fixed fixed::operator*(const fixed & _r) const
	{ fixed ret; ret.v = (v * _r.v) >> DECIMAL_SHIFT ; return ret; }

	fixed fixed::operator/(const fixed & _r) const
	{ fixed ret; ret.v = (v << DECIMAL_SHIFT) / _r.v; return ret; }

	bool fixed::operator>(const fixed & _r) const
	{ return v > _r.v; }

	bool fixed::operator>=(const fixed & _r) const
	{ return v >= _r.v; }

	bool fixed::operator<(const fixed & _r) const
	{ return v < _r.v; }

	bool fixed::operator<=(const fixed & _r) const
	{ return v <= _r.v; }

	bool fixed::operator==(const fixed & _r) const
	{ return v == _r.v; }

}

