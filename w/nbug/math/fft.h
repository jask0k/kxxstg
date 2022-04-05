#ifndef NB_FFT_H
#define NB_FFT_H

// Original codes are from Laurent de Soras's FFTReal library. about 50% speed againts fftw3.

#include <math.h>
#include <nbug/core/debug.h>

namespace e
{
	const double	DBLPI	= 3.1415926535897932384626433832795;
	const double	SQRT2	= 1.41421356237309514547462185873883;

	template <typename T> struct OscSinCos
	{
		T pos_cos;		// Current phase expressed with sin and cos. [-1 ; 1]
		T pos_sin;		// -
		T step_cos;		// Phase increment per step, [-1 ; 1]
		T step_sin;		// -

		OscSinCos ()
			:	pos_cos (1)
			,	pos_sin (0)
			,	step_cos (1)
			,	step_sin (0)
		{}

		void set_step (double angle_rad)
		{
			using namespace std;

			step_cos = static_cast <T> (cos (angle_rad));
			step_sin = static_cast <T> (sin (angle_rad));
		}

		void step ()
		{
			const T	old_cos = pos_cos;
			const T	old_sin = pos_sin;
			pos_cos = old_cos * step_cos - old_sin * step_sin;
			pos_sin = old_cos * step_sin + old_sin * step_cos;
		}

		void clear_buffers ()
		{
			pos_cos = 1;
			pos_sin = 0;
		}
	};	// class OscSinCos


	template <typename T> class FFTReal
	{
	public:
		static const int MAX_BIT_DEPTH	= 30;	// So length can be represented as long int
	private:
	   	static const int TRIGO_BD_LIMIT	= 12;
		typedef	OscSinCos <T>	OscType;
		const long		_length;
		const int		_nbr_bits;
		long *			_br_lut;
		T *  	_trigo_lut;
		mutable T * _buffer;
	    mutable OscType * _trigo_osc;

		static inline int get_next_pow2(long x)
		{
			--x;

			int				p = 0;
			while ((x & ~0xFFFFL) != 0)
			{
				p += 16;
				x >>= 16;
			}
			while ((x & ~0xFL) != 0)
			{
				p += 4;
				x >>= 4;
			}
			while (x > 0)
			{
				++p;
				x >>= 1;
			}

			return (p);
		}

	public:

		explicit FFTReal(long length)
			: _length(length)
			, _nbr_bits(get_next_pow2(length))
			, _trigo_lut(0)
			, _trigo_osc(0)
		{
			_buffer = (T*)malloc(length * sizeof(T));
			E_ASSERT1(((length & -length) == length), L"length() must be power of 2.");
			E_ASSERT (_nbr_bits <= MAX_BIT_DEPTH);

			{
				const long		length = 1L << _nbr_bits;
				//_br_lut.resize (length);
				_br_lut = (long*) malloc(sizeof(long) * length);

				_br_lut [0] = 0;
				long				br_index = 0;
				for (long cnt = 1; cnt < length; ++cnt)
				{
					// ++br_index (bit reversed)
					long				bit = length >> 1;
					while (((br_index ^= bit) & bit) == 0)
					{
						bit >>= 1;
					}

					_br_lut [cnt] = br_index;
				}
			}

			{
				using namespace std;

				if (_nbr_bits > 3)
				{
					const long		total_len = (1L << (_nbr_bits - 1)) - 4;
					_trigo_lut = (T*) malloc(sizeof(T) * total_len);

					for (int level = 3; level < _nbr_bits; ++level)
					{
						const long		level_len = 1L << (level - 1);
						T	* const	level_ptr =
							&_trigo_lut [get_trigo_level_index (level)];
						const double	mul = DBLPI / (level_len << 1);

						for (long i = 0; i < level_len; ++ i)
						{
							level_ptr [i] = static_cast <T> (cos (i * mul));
						}
					}
				}
			}

			{
				const int		nbr_osc = _nbr_bits - TRIGO_BD_LIMIT;
				if(nbr_osc > 0)
				{
					_trigo_osc = (OscType*) malloc(sizeof(OscType) * nbr_osc);

					for (int osc_cnt = 0; osc_cnt < nbr_osc; ++osc_cnt)
					{
						OscType &		osc = _trigo_osc [osc_cnt];

						const long		len = 1L << (TRIGO_BD_LIMIT + osc_cnt);
						const double	mul = (0.5 * DBLPI) / len;
						osc.set_step (mul);
					}
				}
			}
		}

		~FFTReal ()
		{
			free(_trigo_osc);
			free(_br_lut);
			free(_trigo_lut);
			free(_buffer);
		}

		/*
		==============================================================================
		Name: do_fft
		Description:
			Compute the FFT of the array.
		Input parameters:
			- x: pointer on the source array (time).
		Output parameters:
			- f: pointer on the destination array (frequencies).
				f [0...length(x)/2] = real values,
				f [length(x)/2+1...length(x)-1] = negative imaginary values of
				coefficents 1...length(x)/2-1.
		Throws: Nothing
		==============================================================================
		*/

		void do_fft (T f [], const T x []) const
		{
			E_ASSERT (f != 0);
			E_ASSERT (f != _buffer);
			E_ASSERT (x != 0);
			E_ASSERT (x != _buffer);
			E_ASSERT (x != f);

			// General case
			if (_nbr_bits > 2)
			{
				compute_fft_general (f, x);
			}

			// 4-point FFT
			else if (_nbr_bits == 2)
			{
				f [1] = x [0] - x [2];
				f [3] = x [1] - x [3];

				const T	b_0 = x [0] + x [2];
				const T	b_2 = x [1] + x [3];

				f [0] = b_0 + b_2;
				f [2] = b_0 - b_2;
			}

			// 2-point FFT
			else if (_nbr_bits == 1)
			{
				f [0] = x [0] + x [1];
				f [1] = x [0] - x [1];
			}

			// 1-point FFT
			else
			{
				f [0] = x [0];
			}
		}

		/*
		==============================================================================
		Name: do_ifft
		Description:
			Compute the inverse FFT of the array. Note that data must be post-scaled:
			IFFT (FFT (x)) = x * length (x).
		Input parameters:
			- f: pointer on the source array (frequencies).
				f [0...length(x)/2] = real values
				f [length(x)/2+1...length(x)-1] = negative imaginary values of
				coefficents 1...length(x)/2-1.
		Output parameters:
			- x: pointer on the destination array (time).
		Throws: Nothing
		==============================================================================
		*/

		void do_ifft (const T f [], T x []) const
		{
			E_ASSERT (f != 0);
			E_ASSERT (f != _buffer);
			E_ASSERT (x != 0);
			E_ASSERT (x != _buffer);
			E_ASSERT (x != f);

			// General case
			if (_nbr_bits > 2)
			{
				compute_ifft_general (f, x);
			}

			// 4-point IFFT
			else if (_nbr_bits == 2)
			{
				const T	b_0 = f [0] + f [2];
				const T	b_2 = f [0] - f [2];

				x [0] = b_0 + f [1] * 2;
				x [2] = b_0 - f [1] * 2;
				x [1] = b_2 + f [3] * 2;
				x [3] = b_2 - f [3] * 2;
			}

			// 2-point IFFT
			else if (_nbr_bits == 1)
			{
				x [0] = f [0] + f [1];
				x [1] = f [0] - f [1];
			}

			// 1-point IFFT
			else
			{
				x [0] = f [0];
			}
		}

		/*
			==============================================================================
			Name: rescale
			Description:
				Scale an array by divide each element by its length. This function should
				be called after FFT + IFFT.
			Input parameters:
				- x: pointer on array to rescale (time or frequency).
			Throws: Nothing
			==============================================================================
			*/

		void rescale (T x []) const
		{
			const T	mul = T (1.0 / _length);

			if (_length < 4)
			{
				long				i = _length - 1;
				do
				{
					x [i] *= mul;
					--i;
				}
				while (i >= 0);
			}

			else
			{
				//E_ASSERT ((_length & 3) == 0);

				// Could be optimized with SIMD instruction sets (needs alignment check)
				long				i = _length - 4;
				do
				{
					x [i + 0] *= mul;
					x [i + 1] *= mul;
					x [i + 2] *= mul;
					x [i + 3] *= mul;
					i -= 4;
				}
				while (i >= 0);
			}
		}

	private:
		const T * get_trigo_ptr (int level) const
		{
			//E_ASSERT (level >= 3);
			return (&_trigo_lut [get_trigo_level_index (level)]);
		}

		long get_trigo_level_index (int level) const
		{
			//E_ASSERT (level >= 3);
			return ((1L << (level - 1)) - 4);
		}

		// Transform in several passes
		void compute_fft_general (T f [], const T x []) const
		{
			//E_ASSERT (f != 0);
			//E_ASSERT (f != _buffer);
			//E_ASSERT (x != 0);
			//E_ASSERT (x != _buffer);
			//E_ASSERT (x != f);

			T *		sf;
			T *		df;

			if ((_nbr_bits & 1) != 0)
			{
				df = _buffer;
				sf = f;
			}
			else
			{
				df = f;
				sf = _buffer;
			}

			compute_direct_pass_1_2 (df, x);
			compute_direct_pass_3 (sf, df);

			for (int pass = 3; pass < _nbr_bits; ++ pass)
			{
				compute_direct_pass_n (df, sf, pass);

				T * const	temp_ptr = df;
				df = sf;
				sf = temp_ptr;
			}
		}

		void compute_direct_pass_1_2 (T df [], const T x []) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (x != 0);
			//E_ASSERT (df != x);

			const long * const	bit_rev_lut_ptr = _br_lut;
			long				coef_index = 0;
			do
			{
				const long		rev_index_0 = bit_rev_lut_ptr [coef_index];
				const long		rev_index_1 = bit_rev_lut_ptr [coef_index + 1];
				const long		rev_index_2 = bit_rev_lut_ptr [coef_index + 2];
				const long		rev_index_3 = bit_rev_lut_ptr [coef_index + 3];

				T	* const	df2 = df + coef_index;
				df2 [1] = x [rev_index_0] - x [rev_index_1];
				df2 [3] = x [rev_index_2] - x [rev_index_3];

				const T	sf_0 = x [rev_index_0] + x [rev_index_1];
				const T	sf_2 = x [rev_index_2] + x [rev_index_3];

				df2 [0] = sf_0 + sf_2;
				df2 [2] = sf_0 - sf_2;

				coef_index += 4;
			}
			while (coef_index < _length);
		}

		void compute_direct_pass_3 (T df [], const T sf []) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);

			const T	sqrt2_2 = T (SQRT2 * 0.5);
			long				coef_index = 0;
			do
			{
				T			v;

				df [coef_index] = sf [coef_index] + sf [coef_index + 4];
				df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
				df [coef_index + 2] = sf [coef_index + 2];
				df [coef_index + 6] = sf [coef_index + 6];

				v = (sf [coef_index + 5] - sf [coef_index + 7]) * sqrt2_2;
				df [coef_index + 1] = sf [coef_index + 1] + v;
				df [coef_index + 3] = sf [coef_index + 1] - v;

				v = (sf [coef_index + 5] + sf [coef_index + 7]) * sqrt2_2;
				df [coef_index + 5] = v + sf [coef_index + 3];
				df [coef_index + 7] = v - sf [coef_index + 3];

				coef_index += 8;
			}
			while (coef_index < _length);
		}

		void compute_direct_pass_n (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass >= 3);
			//E_ASSERT (pass < _nbr_bits);

			if (pass <= TRIGO_BD_LIMIT)
			{
				compute_direct_pass_n_lut (df, sf, pass);
			}
			else
			{
				compute_direct_pass_n_osc (df, sf, pass);
			}
		}

		void compute_direct_pass_n_lut (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass >= 3);
			//E_ASSERT (pass < _nbr_bits);

			const long		nbr_coef = 1 << pass;
			const long		h_nbr_coef = nbr_coef >> 1;
			const long		d_nbr_coef = nbr_coef << 1;
			long				coef_index = 0;
			const T	* const	cos_ptr = get_trigo_ptr (pass);
			do
			{
				const T	* const	sf1r = sf + coef_index;
				const T	* const	sf2r = sf1r + nbr_coef;
				T			* const	dfr = df + coef_index;
				T			* const	dfi = dfr + nbr_coef;

				// Extreme coefficients are always real
				dfr [0] = sf1r [0] + sf2r [0];
				dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
				dfr [h_nbr_coef] = sf1r [h_nbr_coef];
				dfi [h_nbr_coef] = sf2r [h_nbr_coef];

				// Others are conjugate complex numbers
				const T * const	sf1i = sf1r + h_nbr_coef;
				const T * const	sf2i = sf1i + nbr_coef;
				for (long i = 1; i < h_nbr_coef; ++ i)
				{
					const T	c = cos_ptr [i];					// cos (i*PI/nbr_coef);
					const T	s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
					T	 		v;

					v = sf2r [i] * c - sf2i [i] * s;
					dfr [i] = sf1r [i] + v;
					dfi [-i] = sf1r [i] - v;	// dfr [nbr_coef - i] =

					v = sf2r [i] * s + sf2i [i] * c;
					dfi [i] = v + sf1i [i];
					dfi [nbr_coef - i] = v - sf1i [i];
				}

				coef_index += d_nbr_coef;
			}
			while (coef_index < _length);
		}

		void compute_direct_pass_n_osc (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass > TRIGO_BD_LIMIT);
			//E_ASSERT (pass < _nbr_bits);

			const long		nbr_coef = 1 << pass;
			const long		h_nbr_coef = nbr_coef >> 1;
			const long		d_nbr_coef = nbr_coef << 1;
			long				coef_index = 0;
			OscType &		osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];
			do
			{
				const T	* const	sf1r = sf + coef_index;
				const T	* const	sf2r = sf1r + nbr_coef;
				T			* const	dfr = df + coef_index;
				T			* const	dfi = dfr + nbr_coef;

				osc.clear_buffers ();

				// Extreme coefficients are always real
				dfr [0] = sf1r [0] + sf2r [0];
				dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
				dfr [h_nbr_coef] = sf1r [h_nbr_coef];
				dfi [h_nbr_coef] = sf2r [h_nbr_coef];

				// Others are conjugate complex numbers
				const T * const	sf1i = sf1r + h_nbr_coef;
				const T * const	sf2i = sf1i + nbr_coef;
				for (long i = 1; i < h_nbr_coef; ++ i)
				{
					osc.step ();
					const T	c = osc.get_cos ();
					const T	s = osc.get_sin ();
					T	 		v;

					v = sf2r [i] * c - sf2i [i] * s;
					dfr [i] = sf1r [i] + v;
					dfi [-i] = sf1r [i] - v;	// dfr [nbr_coef - i] =

					v = sf2r [i] * s + sf2i [i] * c;
					dfi [i] = v + sf1i [i];
					dfi [nbr_coef - i] = v - sf1i [i];
				}

				coef_index += d_nbr_coef;
			}
			while (coef_index < _length);
		}

		// Transform in several pass
		void compute_ifft_general (const T f [], T x []) const
		{
			//E_ASSERT (f != 0);
			//E_ASSERT (f != _buffer);
			//E_ASSERT (x != 0);
			//E_ASSERT (x != _buffer);
			//E_ASSERT (x != f);

			T *		sf = const_cast <T *> (f);
			T *		df;
			T *		df_temp;

			if (_nbr_bits & 1)
			{
				df = _buffer;
				df_temp = x;
			}
			else
			{
				df = x;
				df_temp = _buffer;
			}

			for (int pass = _nbr_bits - 1; pass >= 3; -- pass)
			{
				compute_inverse_pass_n (df, sf, pass);

				if (pass < _nbr_bits - 1)
				{
					T	* const	temp_ptr = df;
					df = sf;
					sf = temp_ptr;
				}
				else
				{
					sf = df;
					df = df_temp;
				}
			}

			compute_inverse_pass_3 (df, sf);
			compute_inverse_pass_1_2 (x, df);
		}

		void compute_inverse_pass_n (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass >= 3);
			//E_ASSERT (pass < _nbr_bits);

			if (pass <= TRIGO_BD_LIMIT)
			{
				compute_inverse_pass_n_lut (df, sf, pass);
			}
			else
			{
				compute_inverse_pass_n_osc (df, sf, pass);
			}
		}

		void compute_inverse_pass_n_lut (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass >= 3);
			//E_ASSERT (pass < _nbr_bits);

			const long		nbr_coef = 1 << pass;
			const long		h_nbr_coef = nbr_coef >> 1;
			const long		d_nbr_coef = nbr_coef << 1;
			long				coef_index = 0;
			const T * const	cos_ptr = get_trigo_ptr (pass);
			do
			{
				const T	* const	sfr = sf + coef_index;
				const T	* const	sfi = sfr + nbr_coef;
				T			* const	df1r = df + coef_index;
				T			* const	df2r = df1r + nbr_coef;

				// Extreme coefficients are always real
				df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
				df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
				df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
				df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

				// Others are conjugate complex numbers
				T * const	df1i = df1r + h_nbr_coef;
				T * const	df2i = df1i + nbr_coef;
				for (long i = 1; i < h_nbr_coef; ++ i)
				{
					df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
					df1i [i] = sfi [i] - sfi [nbr_coef - i];

					const T	c = cos_ptr [i];					// cos (i*PI/nbr_coef);
					const T	s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
					const T	vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
					const T	vi = sfi [i] + sfi [nbr_coef - i];

					df2r [i] = vr * c + vi * s;
					df2i [i] = vi * c - vr * s;
				}

				coef_index += d_nbr_coef;
			}
			while (coef_index < _length);
		}

		void compute_inverse_pass_n_osc (T df [], const T sf [], int pass) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);
			//E_ASSERT (pass > TRIGO_BD_LIMIT);
			//E_ASSERT (pass < _nbr_bits);

			const long		nbr_coef = 1 << pass;
			const long		h_nbr_coef = nbr_coef >> 1;
			const long		d_nbr_coef = nbr_coef << 1;
			long				coef_index = 0;
			OscType &		osc = _trigo_osc [pass - (TRIGO_BD_LIMIT + 1)];
			do
			{
				const T	* const	sfr = sf + coef_index;
				const T	* const	sfi = sfr + nbr_coef;
				T			* const	df1r = df + coef_index;
				T			* const	df2r = df1r + nbr_coef;

				osc.clear_buffers ();

				// Extreme coefficients are always real
				df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
				df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
				df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
				df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

				// Others are conjugate complex numbers
				T * const	df1i = df1r + h_nbr_coef;
				T * const	df2i = df1i + nbr_coef;
				for (long i = 1; i < h_nbr_coef; ++ i)
				{
					df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
					df1i [i] = sfi [i] - sfi [nbr_coef - i];

					osc.step ();
					const T	c = osc.pos_cos;
					const T	s = osc.pos_sin;
					const T	vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
					const T	vi = sfi [i] + sfi [nbr_coef - i];

					df2r [i] = vr * c + vi * s;
					df2i [i] = vi * c - vr * s;
				}

				coef_index += d_nbr_coef;
			}
			while (coef_index < _length);
		}

		void compute_inverse_pass_3 (T df [], const T sf []) const
		{
			//E_ASSERT (df != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (df != sf);

			const T	sqrt2_2 = T (SQRT2 * 0.5);
			long				coef_index = 0;
			do
			{
				df [coef_index] = sf [coef_index] + sf [coef_index + 4];
				df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
				df [coef_index + 2] = sf [coef_index + 2] * 2;
				df [coef_index + 6] = sf [coef_index + 6] * 2;

				df [coef_index + 1] = sf [coef_index + 1] + sf [coef_index + 3];
				df [coef_index + 3] = sf [coef_index + 5] - sf [coef_index + 7];

				const T	vr = sf [coef_index + 1] - sf [coef_index + 3];
				const T	vi = sf [coef_index + 5] + sf [coef_index + 7];

				df [coef_index + 5] = (vr + vi) * sqrt2_2;
				df [coef_index + 7] = (vi - vr) * sqrt2_2;

				coef_index += 8;
			}
			while (coef_index < _length);
		}

		void compute_inverse_pass_1_2 (T x [], const T sf []) const
		{
			//E_ASSERT (x != 0);
			//E_ASSERT (sf != 0);
			//E_ASSERT (x != sf);

			const long *	bit_rev_lut_ptr = _br_lut;
			const T *	sf2 = sf;
			long				coef_index = 0;
			do
			{
				{
					const T	b_0 = sf2 [0] + sf2 [2];
					const T	b_2 = sf2 [0] - sf2 [2];
					const T	b_1 = sf2 [1] * 2;
					const T	b_3 = sf2 [3] * 2;

					x [bit_rev_lut_ptr [0]] = b_0 + b_1;
					x [bit_rev_lut_ptr [1]] = b_0 - b_1;
					x [bit_rev_lut_ptr [2]] = b_2 + b_3;
					x [bit_rev_lut_ptr [3]] = b_2 - b_3;
				}
				{
					const T	b_0 = sf2 [4] + sf2 [6];
					const T	b_2 = sf2 [4] - sf2 [6];
					const T	b_1 = sf2 [5] * 2;
					const T	b_3 = sf2 [7] * 2;

					x [bit_rev_lut_ptr [4]] = b_0 + b_1;
					x [bit_rev_lut_ptr [5]] = b_0 - b_1;
					x [bit_rev_lut_ptr [6]] = b_2 + b_3;
					x [bit_rev_lut_ptr [7]] = b_2 - b_3;
				}

				sf2 += 8;
				coef_index += 8;
				bit_rev_lut_ptr += 8;
			}
			while (coef_index < _length);
		}

	};	// class FFTReal

}	// namespace e



#endif
