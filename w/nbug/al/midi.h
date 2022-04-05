#ifndef NB_INSTRUMENT_PARAM_H
#define NB_INSTRUMENT_PARAM_H

#include <string.h>
#include <nbug/math/math.h>
#include <nbug/core/debug.h>
#include <nbug/core/file.h>
#include <nbug/core/env.h>
#include <nbug/math/fft.h>

namespace e
{
#ifdef NB_DEBUG
	extern void midi_debug_lock_program(int _index);
	extern const wchar_t * instrument_names[];
	extern const wchar_t * midi_debug_get_inst_name(int _index);
	extern const wchar_t * midi_debug_get_drum_name(int _index);
#endif

	static const int HARMONIC_COUNT = 64;
	struct InstrumentParam
	{

		float harmonic[HARMONIC_COUNT];

		float bw_base;
		float bw_scale;

		// amplifier params (standard forte)
		//      +             --
		//     / \
		//    /   \           a1
		//   /     +---+      --
		//  /           \
		// +             \    a0
		// | t0 |t1|t2 |t3|   --

		float amp_a_c2[2];  // (0 ~ 1)
		float amp_t_c2[4];  // ms
		union
		{
			float drum_freq;
			float amp_a_c6[2];  // (0 ~ 1)
		};
		float amp_t_c6[4];  // ms

		uint8 chorus;
		uint8 square;
		float vibrate_frac;
		float vibrate_freq;

		bool Load(int _id);
		bool Save(int _id);
	};

	class PADSynth
	{
		FFTReal<float> fft;
		float *freq;
		//float *freq_real;
		//float *freq_imaginary;
		float *tmp_buf;
		int N;
		int N_OSCI;
		//float profile(float fi, float bwi);
		float *freq_amp;
		int samplerate;
		int number_harmonics;
	public:
		PADSynth(int N_, int samplerate_, int number_harmonics_);
		~PADSynth();
		void _synth0(bool _lock_phase, int _freq_shift);
		void synth(const InstrumentParam * _param, float f, float *sample_l, float * sample_r, bool _lock_phase = false);
	};
}


#endif
