#ifndef NB_MIXING_FRAC_H
#define NB_MIXING_FRAC_H

namespace e
{
	template <int MAX_FRAC, int MAX_AMP> class MixingFrac
	{
		float frac;
		float frac_target;
		uint32 counter;
	public:
		MixingFrac()
		{
			frac = MAX_FRAC * 0.0075f;
			frac_target = MAX_FRAC * 0.0075f;
			counter= 1;
		}
		void OnDistor(float _l, float _r)
		{
			if(_l < 0)
			{
				_l= -_l;
			}

			if(_r < 0)
			{
				_r= -_r;
			}
			float v = _l < _r ? _r : _l;

			E_ASSERT(v >= MAX_AMP*0.0099f);
			float frac1 = frac * MAX_AMP*0.0099f / v;
			if(frac1 < frac_target)
			{
				frac_target = frac1;
			}
		}

		void Step()
		{
			frac+= (frac_target-frac) * 0.000005f;
			if(--counter == 0)
			{
				counter= 10000;
				if(frac_target < MAX_FRAC * 0.005f)
				{
					frac_target+= 0.005f;
				}
			}
		}

		float Get() const
		{
			return frac;
		}
#ifdef NB_DEBUG
		void Dump()
		{
			E_TRACE_LINE(string(frac) +L"\t " + string(frac_target));
		}
#endif
	};
}

#endif
