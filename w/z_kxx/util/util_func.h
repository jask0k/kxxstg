

namespace e
{
	// calc angles of circle danmaku, last one point to _angle0
	inline static void CalcCircleShot(float _angles[], int _count, float _angle0)
	{
		E_ASSERT(_count > 1);
		float da = PI * 2 / _count;
		if(_count & 0x0000001)
		{

			int n = (_count - 1) / 2;
			float a1 = _angles[0] - da * 0.5f;
			float a2 = _angles[0] + da * 0.5f;
			for(int i = 0; i < n; i++)
			{
				_angles[i*2 + 0] = a1;
				_angles[i*2 + 1] = a2;
				a1-= da;
				a2+= da;
			}
			_angles[_count - 1] = _angle0;
		}
		else
		{
			_angles[0] = _angle0 + PI;
			int n = (_count - 2) / 2;
			float a1 = _angles[0] - da;
			float a2 = _angles[0] + da;
			for(int i = 0; i < n; i++)
			{
				_angles[i*2 + 1] = a1;
				_angles[i*2 + 2] = a2;
				a1-= da;
				a2+= da;
			}
			_angles[_count - 1] = _angle0;
		}
	}

	inline static float CalcFadeInFadeOut(uint32 _tMax, uint32 _t, float _frac = 0.2f)
	{
		E_ASSERT(_frac >0 && _frac <= 0.5f);
		E_ASSERT(_tMax >= _t);
		uint32 t0 = _tMax;
		uint32 t1 = (int)(t0 - t0 * _frac);
		uint32 t2 = (int)(t0 * _frac);
		float alpha;
		if(_t < t2)
		{
			alpha = float(_t) / t2;
		}
		else if(_t <= t1)
		{
			alpha = 1.0f;
		}
		else
		{
			alpha = float(t0 -  _t) / t2;
		}
		E_ASSERT(alpha >=0 && alpha < 1.000001f);
		return alpha;
	}

	inline static bool PositionApproach(Vector2 & _src, const Vector2 & _target, float _speed)
	{
		E_ASSERT(_speed > 0);
		Vector2 delta = _target - _src;
		if(delta.length() < _speed)
		{
			_src+= delta;
		}
		else
		{
			_src+= delta.GetNormal() * _speed;
		}
		return fabs(_src.x - _target.x) < 0.0001f && fabs(_src.y - _target.y) < 0.0001f;
	}

	inline static void FloatApproach(float & _src, float _dst, float _speed)
	{
		E_ASSERT(_speed > 0);
		float delta = _dst - _src;
		if(delta < -_speed)
		{
			delta = -_speed;
		}
		else if(delta > _speed)
		{
			delta = _speed;
		}
		_src+= delta;
	}

	inline static uint GetRandomState(uint _prob[], uint _count)
	{
		E_ASSERT(_count > 0);
		uint total = 0;
		for(uint i = 0; i < _count; i++)
		{
			total+= _prob[i];
		}

		if(total == 0)
		{
			return 0;
		}

		uint r = logic_random_int() % total;
		for(uint i = 0; i < _count; i++)
		{
			if(r < _prob[i])
			{
				return i;
			}
			r-= _prob[i];
		}

		E_ASSERT(0);
		return 0;
	}
}
