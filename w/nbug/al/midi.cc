#include <string.h>
#include <nbug/al/afile.h>
#include <nbug/core/debug.h>
#include <nbug/core/thread.h>
#include <nbug/al/mixing_frac.h>
#include <nbug/al/midi.h>
//#include <nbug/math/math.h>

namespace e
{
	static const int A4_MIDI_KEY = 69;
	static const int C2_MIDI_KEY = A4_MIDI_KEY - 9 - 24;
	static const int C6_MIDI_KEY = C2_MIDI_KEY + 4 * 12;

	static const int SAMPLE_SIZE = 1 << 16;
	static const int SLOT_COUNT = 256;
	static const float AMP_TIME_SCALE = 44.1f;

#ifdef NB_DEBUG
	static int g_lock_program = -1;
	static int g_lock_drum_key = -1;
	void midi_debug_lock_program(int _index)
	{
		if(_index >= 128)
		{
			g_lock_drum_key = _index - 128;
			g_lock_program = -1;
		}
		else
		{
			g_lock_program = _index;
			g_lock_drum_key = -1;
		}
	}

	const wchar_t * instrument_names[] =
	{
		L"0 Acoustic Grand Piano",
		L"1 Bright Acoustic Piano",
		L"2 Electric Grand Piano",
		L"3 Honky-tonk Piano",
		L"4 Electric Piano 1",
		L"5 Electric Piano 2",
		L"6 Harpsichord",
		L"7 Clavi",
		L"8 Celesta",
		L"9 Glockenspiel",
		L"10 Music Box",
		L"11 Vibraphone",
		L"12 Marimba",
		L"13 Xylophone",
		L"14 Tubular Bells",
		L"15 Dulcimer",
		L"16 Drawbar Organ",
		L"17 Percussive Organ",
		L"18 Rock Organ",
		L"19 Church Organ",
		L"20 Reed Organ",
		L"21 Accordion",
		L"22 Harmonica",
		L"23 Tango Accordion",
		L"24 Acoustic Guitar (nylon)",
		L"25 Acoustic Guitar (steel)",
		L"26 Electric Guitar (jazz)",
		L"27 Electric Guitar (clean)",
		L"28 Electric Guitar (muted)",
		L"29 Overdriven Guitar",
		L"30 Distortion Guitar",
		L"31 Guitar harmonics",
		L"32 Acoustic Bass",
		L"33 Electric Bass (finger)",
		L"34 Electric Bass (pick)",
		L"35 Fretless Bass",
		L"36 Slap Bass 1",
		L"37 Slap Bass 2",
		L"38 Synth Bass 1",
		L"39 Synth Bass 2",
		L"40 Violin",
		L"41 Viola",
		L"42 Cello",
		L"43 Contrabass",
		L"44 Tremolo Strings",
		L"45 Pizzicato Strings",
		L"46 Orchestral Harp",
		L"47 Timpani",
		L"48 string Ensemble 1",
		L"49 string Ensemble 2",
		L"50 SynthStrings 1",
		L"51 SynthStrings 2",
		L"52 Choir Aahs",
		L"53 Voice Oohs",
		L"54 Synth Voice",
		L"55 Orchestra Hit",
		L"56 Trumpet",
		L"57 Trombone",
		L"58 Tuba",
		L"59 Muted Trumpet",
		L"60 French Horn",
		L"61 Brass Section",
		L"62 SynthBrass 1",
		L"63 SynthBrass 2",
		L"64 Soprano Sax",
		L"65 Alto Sax",
		L"66 Tenor Sax",
		L"67 Baritone Sax",
		L"68 Oboe",
		L"69 English Horn",
		L"70 Bassoon",
		L"71 Clarinet",
		L"72 Piccolo",
		L"73 Flute",
		L"74 Recorder",
		L"75 Pan Flute",
		L"76 Blown Bottle",
		L"77 Shakuhachi",
		L"78 Whistle",
		L"79 Ocarina",
		L"80 Lead 1 (square)",
		L"81 Lead 2 (sawtooth)",
		L"82 Lead 3 (calliope)",
		L"83 Lead 4 (chiff)",
		L"84 Lead 5 (charang)",
		L"85 Lead 6 (voice)",
		L"86 Lead 7 (fifths)",
		L"87 Lead 8 (bass + lead)",
		L"88 Pad 1 (new age)",
		L"89 Pad 2 (warm)",
		L"90 Pad 3 (polysynth)",
		L"91 Pad 4 (choir)",
		L"92 Pad 5 (bowed)",
		L"93 Pad 6 (metallic)",
		L"94 Pad 7 (halo)",
		L"95 Pad 8 (sweep)",
		L"96 FX 1 (rain)",
		L"97 FX 2 (soundtrack)",
		L"98 FX 3 (crystal)",
		L"99 FX 4 (atmosphere)",
		L"100 FX 5 (brightness)",
		L"101 FX 6 (goblins)",
		L"102 FX 7 (echoes)",
		L"103 FX 8 (sci-fi)",
		L"104 Sitar",
		L"105 Banjo",
		L"106 Shamisen",
		L"107 Koto",
		L"108 Kalimba",
		L"109 Bag pipe",
		L"110 Fiddle",
		L"111 Shanai",
		L"112 Tinkle Bell",
		L"113 Agogo",
		L"114 Steel Drums",
		L"115 Woodblock",
		L"116 Taiko Drum",
		L"117 Melodic Tom",
		L"118 Synth Drum",
		L"119 Reverse Cymbal",
		L"120 Guitar Fret Noise",
		L"121 Breath Noise",
		L"122 Seashore",
		L"123 Bird Tweet",
		L"124 Telephone Ring",
		L"125 Helicopter",
		L"126 Applause",
		L"127 Gunshot",
		L"128(0) Acoustic Bass Drum",
		L"129(1) Bass Drum 1",
		L"130(2) Side Stick",
		L"131(3) Acoustic Snare",
		L"132(4) Hand Clap",
		L"133(5) Electric Snare",
		L"134(6) Low Floor Tom",
		L"135(7) Closed Hi Hat",
		L"136(8) High Floor Tom",
		L"137(9) Pedal Hi-Hat",
		L"138(10) Low Tom",
		L"139(11) Open Hi-Hat",
		L"140(12) Low-Mid Tom",
		L"141(13) Hi-Mid Tom",
		L"142(14) Crash Cymbal 1",
		L"143(15) High Tom",
		L"144(16) Ride Cymbal 1",
		L"145(17) Chinese Cymbal",
		L"146(18) Ride Bell",
		L"147(19) Tambourine",
		L"148(20) Splash Cymbal",
		L"149(21) Cowbell",
		L"150(22) Crash Cymbal 2",
		L"151(23) Vibraslap",
		L"152(24) Ride Cymbal 2",
		L"153(25) Hi Bongo",
		L"154(26) Low Bongo",
		L"155(27) Mute Hi Conga",
		L"156(28) Open Hi Conga",
		L"157(29) Low Conga",
		L"158(30) High Timbale",
		L"159(31) Low Timbale",
		L"160(32) High Agogo",
		L"161(33) Low Agogo",
		L"162(34) Cabasa",
		L"163(35) Maracas",
		L"164(36) Short Whistle",
		L"165(37) Long Whistle",
		L"166(38) Short Guiro",
		L"167(39) Long Guiro",
		L"168(40) Claves",
		L"169(41) Hi Wood Block",
		L"170(42) Low Wood Block",
		L"171(43) Mute Cuica",
		L"172(44) Open Cuica",
		L"173(45) Mute Triangle",
		L"174(46) Open Triangle "
	};

	const wchar_t * midi_debug_get_inst_name(int _index)
	{
		return _index >= 0 && _index <128 ? instrument_names[_index] : L"unkown";
	}

	const wchar_t * midi_debug_get_drum_name(int _index)
	{
		E_ASSERT(_index + 128 < sizeof(instrument_names) / sizeof(wchar_t*));
		return _index >= 0 && _index <47 ? instrument_names[_index + 128] : L"unkown";
	}
#endif


	bool InstrumentParam::Load(int _id)
	{
		bool ok = false;
		FileRef file = FS::OpenFile(Env::GetResourceFolder() | L"ins" | string(_id), false);
		if(file)
		{
			uint8 version;
			uint8 hcount;

			ok = file->Read(&version, sizeof(version)) &&
				file->Read(&bw_base, sizeof(bw_base)) &&
				file->Read(&bw_scale, sizeof(bw_scale)) &&
				file->Read(&amp_a_c2, sizeof(amp_a_c2)) &&
				file->Read(&amp_t_c2, sizeof(amp_t_c2)) &&
				file->Read(&amp_a_c6, sizeof(amp_a_c6)) &&
				file->Read(&amp_t_c6, sizeof(amp_t_c6)) &&
				file->Read(&chorus, sizeof(chorus)) &&
				file->Read(&hcount, sizeof(hcount)) &&
				file->Read(harmonic, sizeof(harmonic)) &&
				file->Read(&square, sizeof(square));

			if(ok)
			{
				if(chorus == 1)
				{
					chorus = 0;
				}
				if(version >= 2)
				{
					ok = file->Read(&vibrate_frac, sizeof(vibrate_frac)) &&
						file->Read(&vibrate_freq, sizeof(vibrate_freq));
				}
				else
				{
					vibrate_frac = 0;
					vibrate_freq = 0;
				}
			}
		}

		if(!ok)
		{
#ifdef NB_DEBUG
			message(L"[nb] (WW) MIDI: Failed to load instrument " + string(instrument_names[_id]) + L", fallback to default.");
#endif
			memset(harmonic, 0, sizeof(harmonic));
			if(_id < 128)
			{
				// default program
				harmonic[1] = 1;
				harmonic[2] = 0.5f;
				harmonic[3] = 0.05f;
				harmonic[4] = 0.04f;
				harmonic[5] = 0.03f;
				harmonic[6] = 0.07f;
				harmonic[7] = 0.10f;
				harmonic[8] = 0.05f;
				harmonic[9] = 0.05f;

				bw_base = 5.0f;
				bw_scale = 1.0f;
				amp_a_c2[0] = 0.4f;
				amp_a_c2[1] = 0.6f;
				amp_t_c2[0] = 1;
				amp_t_c2[1] = 1000;
				amp_t_c2[2] = 3000;
				amp_t_c2[3] = 200;

				amp_a_c6[0] = 0.05f;
				amp_a_c6[1] = 0.8f;
				amp_t_c6[0] = 1;
				amp_t_c6[1] = 200;
				amp_t_c6[2] = 3000;
				amp_t_c6[3] = 200;
			}
			else
			{
				// drum
				harmonic[1] = 1;
				bw_base = 5.0f;
				bw_scale = 1.0f;
				amp_a_c2[0] = 0.1f;
				amp_a_c2[1] = 0.1f;
				amp_t_c2[0] = 1;
				amp_t_c2[1] = 1;
				amp_t_c2[2] = 120;
				amp_t_c2[3] = 10;

				amp_a_c6[0] = 0;
				amp_a_c6[1] = 0;
				amp_t_c6[0] = 0;
				amp_t_c6[1] = 0;
				amp_t_c6[2] = 0;
				amp_t_c6[3] = 0;

				drum_freq = 100;
			}
			chorus = 0;
			square = 0;
			vibrate_frac = 0;
			vibrate_freq = 0;
		}

		return true;
	}

	bool InstrumentParam::Save(int _id)
	{
		FS::CreateFolder(Env::GetResourceFolder() | L"ins");
		FileRef file = FS::OpenFile(Env::GetResourceFolder() | L"ins" | string(_id), true);
		if(file)
		{
			uint8 version = 2;
			uint8 hcount = HARMONIC_COUNT;
			if(chorus == 1)
			{
				chorus = 0;
			}
			bool ok = file->Write(&version, sizeof(version)) &&
				file->Write(&bw_base, sizeof(bw_base)) &&
				file->Write(&bw_scale, sizeof(bw_scale)) &&
				file->Write(&amp_a_c2, sizeof(amp_a_c2)) &&
				file->Write(&amp_t_c2, sizeof(amp_t_c2)) &&
				file->Write(&amp_a_c6, sizeof(amp_a_c6)) &&
				file->Write(&amp_t_c6, sizeof(amp_t_c6)) &&
				file->Write(&chorus, sizeof(chorus)) &&
				file->Write(&hcount, sizeof(hcount)) &&
				file->Write(harmonic, sizeof(harmonic)) &&
				file->Write(&square, sizeof(square)) &&
				file->Write(&vibrate_frac, sizeof(vibrate_frac)) &&
				file->Write(&vibrate_freq, sizeof(vibrate_freq));
			if(ok)
			{
				return true;
			}
		}

		return false;
	}


	PADSynth::PADSynth(int N_, int samplerate_, int number_harmonics_)
		: fft(N_)
	{
		N=N_;
		N_OSCI = int(N * 0.0001f);
		samplerate=samplerate_;
		number_harmonics=number_harmonics_;
		freq_amp=enew float[N/2 + N_OSCI*2];
		freq = enew float[N];
		tmp_buf = enew float[N];
	}

	PADSynth::~PADSynth()
	{
		//delete[] A;
		delete[] freq_amp;
		delete[] freq;
		delete[] tmp_buf;
	}

	static inline float padsynth_profile(float fi, float bwi)
	{
		float x=fi/bwi;
		x*=x;
		if (x>14.71280603) return 0.0;//this avoids computing the e^(-x^2) where it's results are very close to zero
		return exp(-x)/bwi;
	}

/*
		static void chorus0(float * x, float * y, int len, int yoff, float osci_freq, int level)
		{
			float c = 441000 / osci_freq;
			float half_scale_c = level;
			float scale_delta = half_scale_c * 2 / c;
			float d = -half_scale_c;
			int   count = c;
			for(int n=0; n<len; n++)
			{
				d+= scale_delta;

				int i = (n + yoff) % len;
				int j = (i + int(d+0.499f));
				if(j < 0)
				{
					j+= len;
				}
				j%= len;

				y[i]+= x[j];
				if(count-- == 0)
				{
					count = c;
					scale_delta = -scale_delta;
				}
			}
		}
		*/

	void PADSynth::_synth0(bool _lock_phase, int _freq_shift)
	{
		float * R = freq;
		float * I = freq + N/2;

		R[0] = 0;
		I[0] = 0;

		if(_lock_phase)
		{
			for(int i=1; i<N/2; i++)
			{
				R[i]=freq_amp[i+_freq_shift];
				I[i]=0;
			}
		}
		else
		{
			for(int i=1; i<N/2; i++)
			{
				float phase=frand()*2.0f*3.14159265358979f;
				R[i]=freq_amp[i+_freq_shift]*cos(phase);
				I[i]=-freq_amp[i+_freq_shift]*sin(phase);
			};
		}
		fft.do_ifft(freq, tmp_buf);
	}

	void PADSynth::synth(const InstrumentParam * _param, float f, float *sample_l, float * sample_r, bool _lock_phase)
	{
		for(int i=0; i<N/2+N_OSCI*2; i++)
		{
			freq_amp[i] = 0.0;//default, all the frequency amplitudes are zero
		}

		if(_param->square)
		{
			for(int n=1; n<number_harmonics; n++)
			{
				float relF = (float)n;
				float rF = f * relF;
				float bw_Hz = (pow(2.0f, _param->bw_base/1200.0f) - 1.0f) * f * pow(relF, _param->bw_scale);
				float bw = bw_Hz / (2.0f * samplerate);
				float ff = rF / samplerate;

				int imin = int((ff - bw) * N);
				int imax = int((ff + bw) * N);
				if(imin < 0)
				{
					imin = 0;
				}
				if(imax >= N/2)
				{
					imax = N/2 - 1;
				}

				for(int i=imin; i<=imax; i++)
				{
					float fi = i/(float)N;
					freq_amp[i+N_OSCI]+= _param->harmonic[n];
				};
			};
		}
		else
		{
			for(int n=1; n<number_harmonics; n++)
			{
				float relF = (float)n;
				float rF = f * relF;
				float bw_Hz = (pow(2.0f, _param->bw_base/1200.0f) - 1.0f) * f * pow(relF, _param->bw_scale);


				float bw = bw_Hz / (2.0f * samplerate);
				float ff = rF / samplerate;

				int imin = int((ff - bw * 4) * N);
				int imax = int((ff + bw * 4) * N);
				if(imin < 0)
				{
					imin = 0;
				}
				if(imax >= N/2)
				{
					imax = N/2 - 1;
				}
				for(int i=imin; i<=imax; i++)
				{
					float fi = i/(float)N;
					float frac = padsynth_profile(fi - ff, bw);
					freq_amp[i+N_OSCI]+= _param->harmonic[n] * frac;
				};
			};
		}


		// remove very low freq

		int ilo = 5 * N / samplerate;
		int ihi = 20 * N / samplerate;
		float delta = 1.0f / (ihi-ilo);
		for(int i=0; i<ilo; i++)
		{
			freq_amp[i] = 0;
		}
		float y = 0;
		for(int i=ilo; i<ihi; i++)
		{
			freq_amp[i]*= y;
			y+= delta;
		}

		if(_param->chorus)
		{
			E_ASSERT(sample_r != 0 && sample_r != sample_l);
			memset(sample_l, 0, N * sizeof(float));
			memset(sample_r, 0, N * sizeof(float));
			float half_chrous = _param->chorus * 0.5f;
			float dp = 0.7f / half_chrous;
			if(dp > 0.2f)
			{
				dp = 0.2f;
			}
			for(int ch=0; ch <_param->chorus; ch++)
			{
				float a = 1.0f + (half_chrous - ch) * dp;
				float b = 1.0f - (half_chrous - ch) * dp;
				_synth0(_lock_phase, rand() % N_OSCI);
				for(int i=0; i<N; i++)
				{
					sample_l[i]+= tmp_buf[i] * a;
					sample_r[i]+= tmp_buf[i] * b;
				}
			}

		}
		else
		{
			_synth0(_lock_phase, N_OSCI);
			memcpy(sample_l, tmp_buf, N * sizeof(float));
		}

		float max=0.0;
		for(int i=0; i<N; i++)
		{
			float y = abs(sample_l[i]);
			if(y > max)
			{
				max = y;
			}
		}

		if(sample_r)
		{
			for(int i=0; i<N; i++)
			{
				float y = abs(sample_r[i]);
				if(y > max)
				{
					max = y;
				}
			}
		}

		if(max<0.00001f)
		{
			max=0.00001f;
		}
		float a = 1.0f / max / 1.4142f;

		if(_param->vibrate_frac > 0 && _param->vibrate_freq > 0)
		{
			int vc = (int)(44100 / _param->vibrate_freq / 2);
			if(vc<= 0)
			{
				vc = 1;
			}
			float dv = _param->vibrate_frac / vc;
			float vf = 1.0f - _param->vibrate_frac / 2;
			int count = vc;
			for(int i=0; i<N; i++)
			{
				sample_l[i]*= a * vf;
				if(sample_r)
				{
					sample_r[i]*= a * vf;
				}
				vf+= dv;
				if(--count == 0)
				{
					count = vc;
					dv = -dv;
				}
			}
		}
		else
		{
			for(int i=0; i<N; i++)
			{
				sample_l[i]*= a;
			}
			if(sample_r)
			{
				for(int i=0; i<N; i++)
				{
					sample_r[i]*= a;
				}
			}
		}
	}

	static const double A4_FREQ      = 440.0;
	static float key_freq_table[128];
	static bool g_midi_cc_inited = false;
	static void g_ini_midi_cc()
	{
		double mul = pow(2.0, 1.0 / 12.0);
		// const double C4_FREQ = A4_FREQ / pow(2.0, 9.0 / 12.0 );
		double f = A4_FREQ;
		int k;
		for(k=A4_MIDI_KEY; k<128; k+=12)
		{
			double f1 = f;
			for(int k1 = k; k1 < k + 12; k1++)
			{
				if(k1 < 128)
				{
					key_freq_table[k1] = (float)f1;
				}
				f1*= mul;
			}
			f*=2;
		}
		f = A4_FREQ;
		for(k=A4_MIDI_KEY; k>-12; k-=12)
		{
			double f1 = f;
			for(int k1 = k; k1 < k + 12; k1++)
			{
				if(k1 >= 0)
				{
					key_freq_table[k1] = (float)f1;
				}
				f1*= mul;
			}
			f*=0.5;
		}

		g_midi_cc_inited = true;
	}

#ifdef NB_DEBUG
	static const wchar_t * midi_event_name_table[] =
	{
		0,
		L"TEMPO     ",
		0, 0, 0, 0, 0, 0,
		L"NOTE_OFF  ",
		L"NOTE_ON   ",
		L"NOTE_TOUCH",
		L"CONTROL   ",
		L"PROGRAM   ",
		L"CH_TOUCH  ",
		L"PITCH     ",
	};
#endif
	struct MidiEvent
	{
		enum
		{
			TEMPO_CHANGE = 0x01,

			NOTE_OFF = 0x8,
			NOTE_ON = 0x9,
			NOTE_AFTERTOUCH = 0xA,
			CONTROL_CHANGE = 0xB,
			PROGRAM_CHANGE = 0xC,
			CHANNEL_AFTERTOUCH = 0xD,
			PITCH_BEND = 0xE,
		};
		uint time;
		uint type    : 4;
		uint channel : 4;
		uint param1  : 16;
		uint param2  : 16;
		//uint param3  : 8;
#ifdef NB_DEBUG
		void Dump()
		{
			E_TRACE_LINE(L"+ " + string(time) + L"\t" + string(midi_event_name_table[type]) + L"[" + string(channel) + L"]\t" + string(param1) + L"\t" + string(param2));
		}
#endif

	};

	//struct Instrument;
	struct Note
	{
		int  slot;
		float * sample_l;
		float * sample_l_end;
		float * sample_l_cur;

		float * sample_r;
		float * sample_r_end;
		float * sample_r_cur;

		// amp
		int amp_timer[4]; // int?
		float amp_delta[4];
		int amp_cur_state; // 0, 1, 2, 3
		int amp_cur_timer;
		float amp_cur_delta;
		float amp_cur_value;

		float velocity; // intial velocity

		int pitch_timer;
		int pitch_timer_init;
		int pitch_count;
		bool pitch_insert;

		bool is_inited() const
		{
			return sample_l != 0;
		}
		bool is_playing() const
		{
			return slot>=0;
		}

		void alloc_sample_bufs(bool _stereo)
		{
			sample_l = (float*)malloc(SAMPLE_SIZE * sizeof(float));
			sample_l_end = sample_l + SAMPLE_SIZE;
			if(_stereo)
			{
				sample_r = (float*)malloc(SAMPLE_SIZE * sizeof(float));
				sample_r_end = sample_r + SAMPLE_SIZE;
			}
			else
			{
				sample_r = 0;
				sample_r_end = 0;
			}
		}
	};


	//class Midi;
	struct Instrument
	{
		int channel;
		int program;
		bool is_drum()
		{ return channel == 9; }

		const InstrumentParam * param;
		Note notes[128];
		float orig_gain;
		float orig_pan;
		float gain_l;
		float gain_r;

		Instrument()
			//: midi(_midi)
		{
			SetPanGain(0.5f, 1.0f);
			program = -1;
			param = 0;
			memset(notes, 0, sizeof(notes));
			for(int i=0; i<128; i++)
			{
				notes[i].slot = -1;
			}
		}
		~Instrument()
		{
			CleanUp();
		}

		void SetPanGain(float _x, float _gain)
		{
			orig_gain = _gain;
			orig_pan  = _x;

			float b = (_x - 0.5f) * 0.5f;
			if(b < 0)
			{
				b = -b;
			}
			gain_l = 2 * _gain * (1-_x) * (1-b);
			gain_r = 2 * _gain * _x     * (1-b);
		}

		void CleanUp()
		{
			for(int i=0; i<128; i++)
			{
				//E_ASSERT(notes[i].slot <0);
				if(notes[i].sample_l)
				{
					free(notes[i].sample_l);
					notes[i].sample_l = 0;
				}
				if(notes[i].sample_r)
				{
					free(notes[i].sample_r);
					notes[i].sample_r = 0;
				}
			}
			memset(notes, 0, sizeof(notes));
			for(int i=0; i<128; i++)
			{
				notes[i].slot = -1;
			}
		}

		void SetProgram(int id, InstrumentParam * _param)
		{
#ifdef NB_DEBUG
			if(g_lock_program >= 0)
			{
				id = g_lock_program;
			}
#endif
			if(id < 0 || id > 127)
			{
				id = 0;
			}

			if(program == id)
			{
				return;
			}

			program = id;
			if(channel == 9)
			{
				param = 0;
				return;
			}

			CleanUp();

			param = _param;
		}
	};

	struct Track
	{
		Array<MidiEvent> events;
		uint32 last_event_abs_time;
		uint32 event_delay_timer;
		int next_event;

		Track()
		{
			last_event_abs_time = 0;
			event_delay_timer = 0;
			next_event = 0;
		}

		~Track()
		{
		}

		/*
		void SyncDelayTimer()
		{
			if(next_event < events.size())
			{
				event_delay_timer = events[next_event].time;
			}
			else
			{
				event_delay_timer = 0;
			}
		}
		*/
	};

	class Reader
	{
		FileRef file;
		uint8 buf[128];
		uint8 * buf_cur;
		uint8 * buf_end;
		void prepare()
		{
			E_ASSERT(buf_cur == buf_end);
			int n = file->ReadSome(buf, 128);
			buf_cur = buf;
			buf_end = buf_cur + n;
		}
	public:
		Reader(FileRef  _file)
		{
			file = _file;
			buf_cur = 0;
			buf_end = 0;
		}

		uint8 read_byte_no_drop()
		{
			if(buf_cur == buf_end)
			{
				prepare();
				if(buf_cur == buf_end)
				{
					return 0;
				}
			}
			return *buf_cur;
		}
		void drop_byte()
		{
			E_ASSERT(buf_cur < buf_end);
			buf_cur++;
		}
		uint8 read_byte()
		{
			if(buf_cur == buf_end)
			{
				prepare();
				if(buf_cur == buf_end)
				{
					return 0;
				}
			}
			return *buf_cur++;
		}

		uint32 read_vl(int & _byte_count)
		{
			_byte_count = 1;
			uint8 byte;
			uint32 ret = read_byte();
			if(ret & 0x80)
			{
				ret&= 0x7f;
				do
				{
					ret = (ret << 7) + ((byte = read_byte()) & 0x7f);
					_byte_count++;
				}while (byte & 0x80);
			}
			return (ret);
		}

		uint32 read_dword()
		{
			return (uint32(read_byte()) << 24)
				| (uint32(read_byte()) << 16)
				| (uint32(read_byte()) << 8)
				| (uint32(read_byte()));
		}

		uint16 read_word()
		{
			return (uint32(read_byte()) << 8)
				| (uint32(read_byte()));
		}

	};

	class Midi : public SoundFileReader
	{
		struct Slot
		{
			Instrument * inst;
			Note * note;
		} slots[SLOT_COUNT];
		PADSynth padSynth;
		int idle_slots[SLOT_COUNT];
		int idle_slot_count;
		int used_slot_count;
		Instrument instruments[16];
		Array<Track*> tracks;
		uint32 seq_sub_timer;
		uint32 frames_per_tick;
		bool end_of_stream;
		string title;
		float tempo;
		uint16 time_division;
		MixingFrac<50, 100> mixing_frac;

		InstrumentParam * inst_params[128];
		InstrumentParam * drum_params[47];
		inline InstrumentParam * GetInstrumentParam(int _id)
		{
			if(!inst_params[_id])
			{
				InstrumentParam * p = (InstrumentParam*)malloc(sizeof(InstrumentParam));
				p->Load(_id);
				inst_params[_id] = p;
			}
			return inst_params[_id];
		}
		inline InstrumentParam * GetDrumParam(int _id)
		{
			if(!drum_params[_id])
			{
				InstrumentParam * p = (InstrumentParam*)malloc(sizeof(InstrumentParam));
				p->Load(_id + 128);
				drum_params[_id] = p;
			}
			return drum_params[_id];
		}
	public:
		/*
		Instrument * GetInstrument(uint _idx)
		{
			while(instruments.size() <= _idx)
			{
				instruments.push_back(0);
			}
			if(instruments[_idx] == 0)
			{
				instruments[_idx] = enew Instrument();
			}
			return instruments[_idx];
		}
		*/
		void LoadFile_Track(Reader & _in, char _used_note_map[16][128])
		{
			bool is_ticks_per_beat = (0x8000 & time_division) != 0;
			//int _inst_offset = instruments.size();
			Track * track = enew Track();

			uint32 chunk_type = _in.read_dword();
			if(chunk_type != 0x4D54726B)
			{
				throw(NB_SRC_LOC "Unkown chunk.");
			}
			uint32 chunk_size = _in.read_dword();
			uint32 read = 0;
			uint8 status = 0;
			//int  abs_time = 0;
			stringa sTmp;
			while(read < chunk_size)
			{
				MidiEvent ev;
				int tmp;
				ev.time = _in.read_vl(tmp);
				read+= tmp;
				uint8 status1 = _in.read_byte_no_drop();
				if(status1 & 0x80)
				{
					_in.drop_byte();
					read++;
					status = status1;
				}

				if(status == 0xff)
				{
					// meta
					uint8 type = _in.read_byte();
					read++;
					uint32 len = _in.read_vl(tmp);
					read+= len + tmp;
					if(len)
					{
						switch(type)
						{
						case 0x03: // track name
							{
								sTmp.reserve(len);
								for(int i=0; i<len; i++)
								{
									sTmp[i] = (char)_in.read_byte();
								}
								sTmp[len] = 0;
								if(title.empty())
								{
									title = sTmp;
								}
							}
							break;
						case 0x51:
							if(len == 3)
							{
								uint32 t = _in.read_byte();
								t = (t << 8) | _in.read_byte();
								t = (t << 8) | _in.read_byte();
								//t|= _in.read_byte() << 8;
							//	t|= _in.read_byte() << 16;
								ev.type    = MidiEvent::TEMPO_CHANGE;
								ev.channel = 0;
								ev.param1  = 600000000 / t;
								ev.param2  = 0;
								track->events.push_back(ev);
							}
							else
							{
								for(int i=0; i<len; i++)
								{
									_in.read_byte();
								}
							}
							break;
						default:
							for(int i=0; i<len; i++)
							{
								_in.read_byte();
							}
							break;
						}
					}
				}
				else if((status & 0xf0) == 0xf0)
				{
					// sysex
					uint32 len = _in.read_vl(tmp);
					read+= len + tmp;
					for(int i=0; i<len; i++)
					{
						_in.read_byte();
					}
				}
				else
				{
					//abs_time+= ev.time;
					ev.type    = (status >> 4) & 0x0f;
					ev.channel = status & 0x0f;
					ev.param1  = _in.read_byte();
					read++;
					if(ev.type == MidiEvent::NOTE_ON && ev.param1 < 128)
					{
						_used_note_map[ev.channel][ev.param1] = 1;
					}
					if(ev.type != MidiEvent::PROGRAM_CHANGE && ev.type != MidiEvent::CHANNEL_AFTERTOUCH )
					{
						ev.param2  = _in.read_byte();
						read++;
					}
					else
					{
						ev.param2 = 0;
					}

					track->events.push_back(ev);
				}
			}
			E_ASSERT(chunk_size == read);
			if(track->events.empty())
			{
				delete track;
			}
			else
			{
				track->event_delay_timer = track->events[0].time;
				tracks.push_back(track);
			}
		}

		void CalcDelayUnit()
		{
			float ticks_per_second = (time_division & 0x8000) ?
				((time_division >> 8 ) & 0x7f) * (time_division & 0xff)
				: (time_division *  tempo / 60);
			float frames_per_second = 44100.0f;
			frames_per_tick = (uint32)(frames_per_second / ticks_per_second + 0.5f);
			if(frames_per_tick <= 0)
			{
				frames_per_tick = 1;
			}
		}

		void LoadFile(const Path & _path)
		{
			tempo = 120;
			CalcDelayUnit();
			title.clear();
			DeleteAllTrack();

			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				throw(NB_SRC_LOC "Failed to open file.");
			}

			Reader in(file);
			// header chunk
			uint32 chunk_type = in.read_dword();
			if(chunk_type == 0x52494646)//0x46464952
			{
				// RMI - RIFF wrapper
				E_TRACE_LINE(L"[nb] MIDI: RIFF wrapped midi file (RMI).");
				in.read_dword();
				in.read_dword();
				in.read_dword();
				in.read_dword();
				chunk_type = in.read_dword();
			}

			if(chunk_type != 0x4d546864)
			{
				throw(NB_SRC_LOC "Not a midi file.");
			}
			uint32 chunk_size = in.read_dword();
			if(chunk_size != 6)
			{
				throw(NB_SRC_LOC "Corrupted midi file.");
			}
			char used_note_map[16][128];
			memset(used_note_map, 0, sizeof(used_note_map));

			uint16 format_type = in.read_word();
			uint16 track_count = in.read_word();
			time_division = in.read_word();
			try
			{
				switch(format_type)
				{
				case 0:
					LoadFile_Track(in, used_note_map);
					break;
				case 1:
					for(int track=0; track<track_count; track++)
					{
						LoadFile_Track(in, used_note_map);
					}
					break;
				case 2:
				default:
					throw(NB_SRC_LOC "Unsupported format.");
				}
			}
			catch(const char * _exp)
			{
				DeleteAllTrack();
				throw(_exp);
			}

			int n = 0;
			for(int i=0; i<16; i++)
			{
				for(int j=0; j<128; j++)
				{
					if(used_note_map[i][j])
					{
						n++;
						//GetNote(&instruments[i], j);
					}
				}
			}
			// message(L"[nb] MIDI: note_count = " + string(n));
			if(title.empty())
			{
				title = _path.GetBaseName(false);
			}
		}

		Midi(const Path & _path)
			: padSynth(SAMPLE_SIZE, 44100, HARMONIC_COUNT)
		{
			if(!g_midi_cc_inited)
			{
				g_ini_midi_cc();
			}

			memset(inst_params, 0, sizeof(inst_params));
			memset(drum_params, 0, sizeof(drum_params));

			int default_program = 0;
#ifdef NB_DEBUG
			if(g_lock_program >= 0)
			{
				default_program = g_lock_program;
			}
#endif
			for(int i=0; i<16; i++)
			{
				instruments[i].channel = i;
				if(i != 9)
				{
					instruments[i].SetProgram(default_program, GetInstrumentParam(default_program));
				}
			}
			seq_sub_timer = 0;
			time_division = 480;
			used_slot_count = 0;
			idle_slot_count = SLOT_COUNT;
			for(int i=0; i<SLOT_COUNT; i++)
			{
				idle_slots[i] = SLOT_COUNT - i - 1;
			}
			memset(slots, 0, sizeof(slots));
			LoadFile(_path);
			end_of_stream = false;
		}

		~Midi()
		{
			DeleteAllTrack();

			for(int i=0; i<sizeof(inst_params)/sizeof(InstrumentParam*); i++)
			{
				free(inst_params[i]);
			}
			for(int i=0; i<sizeof(drum_params)/sizeof(InstrumentParam*); i++)
			{
				free(drum_params[i]);
			}
		}

		void ReleaseSlot(int _idx)
		{
			E_ASSERT(_idx >= 0 && _idx < SLOT_COUNT);
			Slot & slot = slots[_idx];
			if(slot.note)
			{
				slot.note->slot = -1;
				slot.note = 0;
				slot.inst = 0;
				idle_slots[idle_slot_count++] = _idx;
			}
		}

		Note * GetNote(Instrument * _inst, int _note_idx)
		{
			if(_note_idx < 0 || _note_idx >= 128)
			{
				return 0;
			}

			if(_inst->is_drum())
			{
#ifdef NB_DEBUG
				if(g_lock_drum_key>=0)
				{
					_note_idx = g_lock_drum_key+35;
				}
#endif
				if(_note_idx < 35 || _note_idx > 81)
				{
					return 0; // unused key, according to GM spec
				}

				Note * p = &_inst->notes[_note_idx];
				if(!p->is_inited())
				{
					const InstrumentParam * param = GetDrumParam(_note_idx-35);
					p->alloc_sample_bufs(param->chorus > 0);
					padSynth.synth(param, param->drum_freq, p->sample_l, p->sample_r);
				}
				return p;
			}
			else
			{
				Note * p = &_inst->notes[_note_idx];
				if(!p->is_inited())
				{
					p->alloc_sample_bufs(_inst->param->chorus > 0);
					padSynth.synth(_inst->param, key_freq_table[_note_idx], p->sample_l, p->sample_r);
				}
				return p;
			}
		}

		void OnNoteOn(Instrument * _inst, int _note_idx, float _velocity)
		{
			if(idle_slot_count <= 0 )
			{
				return;
			}

			// note
			Note * p = GetNote(_inst, _note_idx);
			if(!p)
			{
				return;
			}

			if(p->is_playing())
			{
				ReleaseSlot(p->slot);
			}

			// slot
			int n = idle_slots[--idle_slot_count];
			if(n >= used_slot_count)
			{
				E_ASSERT(n == used_slot_count);
				used_slot_count = n + 1;
			}
			Slot & slot = slots[n];
			slot.inst = _inst;
			slot.note = p;
			p->slot = n;

			float a0,a1,t0,t1,t2,t3;
			const InstrumentParam * param;
			if(_inst->is_drum())
			{
#ifdef NB_DEBUG
				if(g_lock_drum_key>=0)
				{
					_note_idx = g_lock_drum_key + 35;
				}
#endif
				param = drum_params[_note_idx-35];
				t0 = param->amp_t_c2[0];
				t1 = param->amp_t_c2[1];
				t2 = param->amp_t_c2[2];
				t3 = param->amp_t_c2[3];
				a0 = param->amp_a_c2[0];
				a1 = param->amp_a_c2[1];
			}
			else
			{
				param = _inst->param;
				float frac = float(_note_idx - C2_MIDI_KEY) / (C6_MIDI_KEY-C2_MIDI_KEY);
				t0 = param->amp_t_c2[0] + frac * (param->amp_t_c6[0] - param->amp_t_c2[0]);
				t1 = param->amp_t_c2[1] + frac * (param->amp_t_c6[1] - param->amp_t_c2[1]);
				t2 = param->amp_t_c2[2] + frac * (param->amp_t_c6[2] - param->amp_t_c2[2]);
				t3 = param->amp_t_c2[3] + frac * (param->amp_t_c6[3] - param->amp_t_c2[3]);
				a0 = param->amp_a_c2[0] + frac * (param->amp_a_c6[0] - param->amp_a_c2[0]);
				a1 = param->amp_a_c2[1] + frac * (param->amp_a_c6[1] - param->amp_a_c2[1]);
			}

			// sample
			int pos = rand() % SAMPLE_SIZE;
			p->sample_l_cur = p->sample_l + pos;
			p->sample_r_cur = p->sample_r + pos;


			// amplifier
			p->amp_timer[0] = int(t0 * AMP_TIME_SCALE + 0.5f);
			if(p->amp_timer[0]<1)
			{
				p->amp_timer[0] = 1;
			}
			p->amp_timer[1] = int(t1 * AMP_TIME_SCALE + 0.5f);
			if(p->amp_timer[1]<1)
			{
				p->amp_timer[1] = 1;
			}
			p->amp_timer[2] = int(t2 * AMP_TIME_SCALE + 0.5f);
			if(p->amp_timer[2]<1)
			{
				p->amp_timer[2] = 1;
			}
			p->amp_timer[3] = int(t3 * AMP_TIME_SCALE + 0.5f);
			if(p->amp_timer[3]<1)
			{
				p->amp_timer[3] = 1;
			}
			p->amp_delta[0] = (a0 + a1) * _velocity / p->amp_timer[0];
			p->amp_delta[1] = -a1 * _velocity / p->amp_timer[1];
			p->amp_delta[2] = -a0 * _velocity / p->amp_timer[2];
			p->amp_delta[3] =  0;
			p->amp_cur_state = 0;
			p->amp_cur_timer = (int)p->amp_timer[0];
			p->amp_cur_delta = p->amp_delta[0];
			p->amp_cur_value = 0;
			p->velocity = _velocity; // for after touch
			p->pitch_timer = 0;
		}

		void OnNoteOff(Instrument * _inst, int _note_idx, float _velocity)
		{
			Note * p = GetNote(_inst, _note_idx);
			if(p==0 || !p->is_playing())
			{
				return;
			}

			if(p->amp_cur_state < 3)
			{
				p->amp_cur_state = 3;
				p->amp_cur_timer = (int)(p->amp_timer[3] * (1 - _velocity*0.8f) + 0.5f);
				if(p->amp_cur_timer <= 0)
				{
					p->amp_cur_timer = 1;
				}
				p->amp_cur_delta = -p->amp_cur_value / p->amp_cur_timer;
			}

		}

		void OnNoteAfterTouch(Instrument * _inst, int _note_idx, float _velocity)
		{
			Note * p = GetNote(_inst, _note_idx);
			if(p==0 || !p->is_playing())
			{
				return;
			}

			p->amp_cur_value*= _velocity / p->velocity;
			E_ASSERT(finite(p->amp_cur_value));
		}

		void OnPitchBend(Instrument * _inst, int _v)
		{
			float frac = float(_v - 8192) * 2.0f / 8192;
			float freq_shift = pow(2, frac/12.0f);
			float frames = 44100 - freq_shift * 44100 ;
			bool pitch_insert = frames >=0;
			if(!pitch_insert)
			{
				frames= -frames;
			}
			int timer = int(44100 / frames + 0.5f);
			for(int i=0; i<used_slot_count; i++)
			{
				Slot & slot = slots[i];
				if(slot.inst == _inst)
				{
					slot.note->pitch_count = (int)(frames + 0.5f);
					slot.note->pitch_insert = pitch_insert;
					slot.note->pitch_timer_init = timer;
					slot.note->pitch_timer = timer;
				}
			}

		}

		void OnProgramChange(Instrument * _inst, int _program)
		{
			for(int i=0; i<used_slot_count; i++)
			{
				Slot & slot = slots[i];
				if(slot.inst == _inst)
				{
					ReleaseSlot(i);
				}
			}
			_inst->SetProgram(_program, this->GetInstrumentParam(_program));
		}

		void DeleteTrack(int _n)
		{
			if(_n < 0 || _n >= tracks.size())
			{
				return;
			}
			Track * & p = tracks[_n];
			if(p)
			{
//				for(int i=0; i<SLOT_COUNT; i++)
//				{
//					if(slots[i].track == p)
//					{
//						ReleaseSlot(i);
//					}
//				}
				delete p;
				p=0;
			}
		}

		void DeleteAllTrack()
		{
			for(int i=0; i<tracks.size(); i++)
			{
				DeleteTrack(i);
			}
		}

		void HandleEvent(MidiEvent & _event)
		{
#ifdef NB_DEBUG
			if(_event.channel == 9)
			{
				//_event.Dump();
				//return;
				if(_event.param1 == 35)
				{
				//	_event.param1 = 36;
				}
			}
#endif

			Instrument * inst = &instruments[_event.channel];
			switch(_event.type)
			{
			case MidiEvent::NOTE_ON:
				if(_event.param2)
				{
					OnNoteOn(inst, _event.param1, _event.param2 / 127.0f);
				}
				else
				{
					OnNoteOff(inst, _event.param1, 0.5f);
				}
				break;
			case MidiEvent::NOTE_OFF:
				OnNoteOff(inst, _event.param1, _event.param2 / 127.0f);
				break;
			case MidiEvent::PROGRAM_CHANGE:
				// E_TRACE_LINE("[nb] MIDI: Program change: [" + string(_event.channel) + L"] = " + string(midi_debug_get_inst_name(_event.param1)));
				OnProgramChange(inst, _event.param1);
				break;
			case MidiEvent::CONTROL_CHANGE:
				switch(_event.param1)
				{
				case 0x07: // Main Volume
					inst->SetPanGain(inst->orig_pan, _event.param2 / 127.0f);
					break;
				case 0x0A: // Pan
					inst->SetPanGain(_event.param2/127.0f, inst->orig_gain);
					break;
				}
				break;
			case MidiEvent::TEMPO_CHANGE:
				this->tempo = _event.param1 / 10.0f;
				this->CalcDelayUnit();
				break;
			case MidiEvent::NOTE_AFTERTOUCH:
				E_TRACE_LINE(L"[nb] MIDI: NOTE_AFTERTOUCH");
				OnNoteAfterTouch(inst, _event.param1, _event.param2 / 127.0f);
				break;
			case MidiEvent::PITCH_BEND:
				OnPitchBend(inst, (int(_event.param2) << 7) | _event.param1);
				break;
			default:
				message(L"[nb] (WW) MIDI: Unsupported event: [" + string(_event.channel) + L"] " + string(_event.type));
				break;
			}
		}

		bool GetInfo(AInfo & _info) override
		{
			_info.title = title;
			_info.frame_count = 0;
			_info.channel_count = 2;
			return true;
		}

		bool Rewind() override
		{
			return false;
		}


		int Read(float * _buf, int _read_frames, int _channels) override
		{
			if(end_of_stream)
			{
				return NB_READ_END;
			}
			E_ASSERT(_channels == 2);
			float * cur = _buf;
			float * end = cur + _read_frames * _channels;
			int ret = 0;
			while(cur < end)
			{
				if(seq_sub_timer == 0)
				{
					seq_sub_timer = this->frames_per_tick;
					int active_instrument = 0;
					for(int i=0; i<tracks.size(); i++)
					{
						Track * p = tracks[i];
						if(!p || p->next_event >= p->events.size())
						{
							continue;
						}
						active_instrument++;
						while(p->event_delay_timer == 0 && p->next_event < p->events.size())
						{
							MidiEvent & event = p->events[p->next_event];
							HandleEvent(event);
							p->next_event++;
							if(p->next_event < p->events.size())
							{
								p->event_delay_timer = p->events[p->next_event].time;
							}
							else
							{
								p->event_delay_timer = 0;
							}
						}

						if(p->event_delay_timer)
						{
							p->event_delay_timer--;
						}
					}
					if(active_instrument==0 && idle_slot_count==SLOT_COUNT)
					{
						this->end_of_stream = true;
						return ret;
					}
				}
				seq_sub_timer--;

				float sum_l = 0;
				float sum_r = 0;

				for(int i=0; i<used_slot_count; i++)
				{
					Slot & slot = slots[i];
					if(slot.note == 0)
					{
						continue;
					}
					Instrument * inst = slot.inst;
					Note * note = slot.note;

					if(note->amp_cur_timer==0)
					{
						if(note->amp_cur_state < 3)
						{
							note->amp_cur_state++;
							note->amp_cur_timer = note->amp_timer[note->amp_cur_state];
							note->amp_cur_delta = note->amp_delta[note->amp_cur_state];
						}
						else
						{
							ReleaseSlot(i);
							continue;
						}
					}

					int move_cursor = 1;
					if(note->pitch_timer)
					{
						note->pitch_timer--;
						if(note->pitch_timer == 0)
						{
							move_cursor = note->pitch_insert ? 0 : 2;
							if(note->pitch_count)
							{
								note->pitch_count--;
								note->pitch_timer = note->pitch_timer_init;
							}
						}
					}

					float al,ar;
					if(note->sample_r)
					{
						al = *note->sample_l_cur;
						ar = *note->sample_r_cur;
						while(move_cursor--)
						{
							note->sample_l_cur++;
							if(note->sample_l_cur >= note->sample_l_end)
							{
								note->sample_l_cur = note->sample_l;
							}
							note->sample_r_cur++;
							if(note->sample_r_cur >= note->sample_r_end)
							{
								note->sample_r_cur = note->sample_r;
							}
						}
					}
					else
					{
						al = ar = *note->sample_l_cur;

						while(move_cursor--)
						{
							note->sample_l_cur++;
							if(note->sample_l_cur >= note->sample_l_end)
							{
								note->sample_l_cur = note->sample_l;
							}
						}
					}
					//E_ASSERT(finite(note->amp_cur_delta));

					note->amp_cur_timer--;
					note->amp_cur_value+= note->amp_cur_delta;
					al*= note->amp_cur_value * inst->gain_l;
					ar*= note->amp_cur_value * inst->gain_r;
					sum_l+= al;
					sum_r+= ar;
				}
				float frac = mixing_frac.Get();
				sum_l*= frac;
				sum_r*= frac;
				bool b1 = sum_l > 1.0f;
				bool b2 = sum_l < -1.0f;
				bool b3 = sum_r > 1.0f;
				bool b4 = sum_r < -1.0f;
				if(b1||b2||b3||b4)
				{
					mixing_frac.OnDistor(sum_l, sum_r);
					//mixing_frac.Dump();
				}
				mixing_frac.Step();
				*cur++ = sum_l;
				*cur++ = sum_r;
				ret++;
			}
			return ret;
		}
	};

	SoundFileReader * create_midi_reader(const Path & _path)
	{
		return enew Midi(_path);
	}

}
