#include <math.h>
#include <string.h>
#include <nbug/al/afile.h>
#include <nbug/core/debug.h>
#include <nbug/core/huffman.h>

//#pragma pack(push, 1)
// #define NB_CFG_VERBOSE
namespace e
{
#ifdef NB_CFG_VERBOSE
	template<typename T> static string bits_to_str(T _n, int _sz)
	{
		string s;
		for(int i=0; i<_sz; i++)
		{
			if(((_n >> i) & 0x01) == 0)
			{
				s+= "0";
			}
			else
			{
				s+= "1";
			}
		}

		return s;
	}

	template<typename T> static void dump_buf(const T * _buf, int _sz, const char * _name = "x")
	{
		for(int i=0; i<_sz; i++)
		{
			E_TRACE_LINE(string(_buf[i]));
		}
	}
#endif

	//static char g_oga_initialized = 0;
	uint64 low_bits_mask_64[65] =
	{
		0ULL,
		0x1ULL, 0x3ULL, 0x7ULL, 0xfULL,
		0x1fULL, 0x3fULL, 0x7fULL, 0xffULL,
		0x1ffULL, 0x3ffULL, 0x7ffULL, 0xfffULL,
		0x1fffULL, 0x3fffULL, 0x7fffULL, 0xffffULL,
		0x1ffffULL, 0x3ffffULL, 0x7ffffULL, 0xfffffULL,
		0x1fffffULL, 0x3fffffULL, 0x7fffffULL, 0xffffffULL,
		0x1ffffffULL, 0x3ffffffULL, 0x7ffffffULL, 0xfffffffULL,
		0x1fffffffULL, 0x3fffffffULL, 0x7fffffffULL, 0xffffffffULL,
		0x1ffffffffULL, 0x3ffffffffULL, 0x7ffffffffULL, 0xfffffffffULL,
		0x1fffffffffULL, 0x3fffffffffULL, 0x7fffffffffULL, 0xffffffffffULL,
		0x1ffffffffffULL, 0x3ffffffffffULL, 0x7ffffffffffULL, 0xfffffffffffULL,
		0x1fffffffffffULL, 0x3fffffffffffULL, 0x7fffffffffffULL, 0xffffffffffffULL,
		0x1ffffffffffffULL, 0x3ffffffffffffULL, 0x7ffffffffffffULL, 0xfffffffffffffULL,
		0x1fffffffffffffULL, 0x3fffffffffffffULL, 0x7fffffffffffffULL, 0xffffffffffffffULL,
		0x1ffffffffffffffULL, 0x3ffffffffffffffULL, 0x7ffffffffffffffULL, 0xfffffffffffffffULL,
		0x1fffffffffffffffULL, 0x3fffffffffffffffULL, 0x7fffffffffffffffULL, 0xffffffffffffffffULL,
	};

	static int ilog(uint32 x)
	{
		int ret = 0;
		while(x)
		{
			ret++;
			x>>= 1;
		}
		return ret;
	}

	static float floor1_inverse_dB_table[256] =
	{
		1.0649863e-07f, 1.1341951e-07f, 1.2079015e-07f, 1.2863978e-07f,
		1.3699951e-07f, 1.4590251e-07f, 1.5538408e-07f, 1.6548181e-07f,
		1.7623575e-07f, 1.8768855e-07f, 1.9988561e-07f, 2.1287530e-07f,
		2.2670913e-07f, 2.4144197e-07f, 2.5713223e-07f, 2.7384213e-07f,
		2.9163793e-07f, 3.1059021e-07f, 3.3077411e-07f, 3.5226968e-07f,
		3.7516214e-07f, 3.9954229e-07f, 4.2550680e-07f, 4.5315863e-07f,
		4.8260743e-07f, 5.1396998e-07f, 5.4737065e-07f, 5.8294187e-07f,
		6.2082472e-07f, 6.6116941e-07f, 7.0413592e-07f, 7.4989464e-07f,
		7.9862701e-07f, 8.5052630e-07f, 9.0579828e-07f, 9.6466216e-07f,
		1.0273513e-06f, 1.0941144e-06f, 1.1652161e-06f, 1.2409384e-06f,
		1.3215816e-06f, 1.4074654e-06f, 1.4989305e-06f, 1.5963394e-06f,
		1.7000785e-06f, 1.8105592e-06f, 1.9282195e-06f, 2.0535261e-06f,
		2.1869758e-06f, 2.3290978e-06f, 2.4804557e-06f, 2.6416497e-06f,
		2.8133190e-06f, 2.9961443e-06f, 3.1908506e-06f, 3.3982101e-06f,
		3.6190449e-06f, 3.8542308e-06f, 4.1047004e-06f, 4.3714470e-06f,
		4.6555282e-06f, 4.9580707e-06f, 5.2802740e-06f, 5.6234160e-06f,
		5.9888572e-06f, 6.3780469e-06f, 6.7925283e-06f, 7.2339451e-06f,
		7.7040476e-06f, 8.2047000e-06f, 8.7378876e-06f, 9.3057248e-06f,
		9.9104632e-06f, 1.0554501e-05f, 1.1240392e-05f, 1.1970856e-05f,
		1.2748789e-05f, 1.3577278e-05f, 1.4459606e-05f, 1.5399272e-05f,
		1.6400004e-05f, 1.7465768e-05f, 1.8600792e-05f, 1.9809576e-05f,
		2.1096914e-05f, 2.2467911e-05f, 2.3928002e-05f, 2.5482978e-05f,
		2.7139006e-05f, 2.8902651e-05f, 3.0780908e-05f, 3.2781225e-05f,
		3.4911534e-05f, 3.7180282e-05f, 3.9596466e-05f, 4.2169667e-05f,
		4.4910090e-05f, 4.7828601e-05f, 5.0936773e-05f, 5.4246931e-05f,
		5.7772202e-05f, 6.1526565e-05f, 6.5524908e-05f, 6.9783085e-05f,
		7.4317983e-05f, 7.9147585e-05f, 8.4291040e-05f, 8.9768747e-05f,
		9.5602426e-05f, 0.00010181521f, 0.00010843174f, 0.00011547824f,
		0.00012298267f, 0.00013097477f, 0.00013948625f, 0.00014855085f,
		0.00015820453f, 0.00016848555f, 0.00017943469f, 0.00019109536f,
		0.00020351382f, 0.00021673929f, 0.00023082423f, 0.00024582449f,
		0.00026179955f, 0.00027881276f, 0.00029693158f, 0.00031622787f,
		0.00033677814f, 0.00035866388f, 0.00038197188f, 0.00040679456f,
		0.00043323036f, 0.00046138411f, 0.00049136745f, 0.00052329927f,
		0.00055730621f, 0.00059352311f, 0.00063209358f, 0.00067317058f,
		0.00071691700f, 0.00076350630f, 0.00081312324f, 0.00086596457f,
		0.00092223983f, 0.00098217216f, 0.0010459992f,  0.0011139742f,
		0.0011863665f,  0.0012634633f,  0.0013455702f,  0.0014330129f,
		0.0015261382f,  0.0016253153f,  0.0017309374f,  0.0018434235f,
		0.0019632195f,  0.0020908006f,  0.0022266726f,  0.0023713743f,
		0.0025254795f,  0.0026895994f,  0.0028643847f,  0.0030505286f,
		0.0032487691f,  0.0034598925f,  0.0036847358f,  0.0039241906f,
		0.0041792066f,  0.0044507950f,  0.0047400328f,  0.0050480668f,
		0.0053761186f,  0.0057254891f,  0.0060975636f,  0.0064938176f,
		0.0069158225f,  0.0073652516f,  0.0078438871f,  0.0083536271f,
		0.0088964928f,  0.009474637f,   0.010090352f,   0.010746080f,
		0.011444421f,   0.012188144f,   0.012980198f,   0.013823725f,
		0.014722068f,   0.015678791f,   0.016697687f,   0.017782797f,
		0.018938423f,   0.020169149f,   0.021479854f,   0.022875735f,
		0.024362330f,   0.025945531f,   0.027631618f,   0.029427276f,
		0.031339626f,   0.033376252f,   0.035545228f,   0.037855157f,
		0.040315199f,   0.042935108f,   0.045725273f,   0.048696758f,
		0.051861348f,   0.055231591f,   0.058820850f,   0.062643361f,
		0.066714279f,   0.071049749f,   0.075666962f,   0.080584227f,
		0.085821044f,   0.091398179f,   0.097337747f,   0.10366330f,
		0.11039993f,    0.11757434f,    0.12521498f,    0.13335215f,
		0.14201813f,    0.15124727f,    0.16107617f,    0.17154380f,
		0.18269168f,    0.19456402f,    0.20720788f,    0.22067342f,
		0.23501402f,    0.25028656f,    0.26655159f,    0.28387361f,
		0.30232132f,    0.32196786f,    0.34289114f,    0.36517414f,
		0.38890521f,    0.41417847f,    0.44109412f,    0.46975890f,
		0.50028648f,    0.53279791f,    0.56742212f,    0.60429640f,
		0.64356699f,    0.68538959f,    0.72993007f,    0.77736504f,
		0.82788260f,    0.88168307f,    0.9389798f,     1.0f
	};

	static const float PI = 3.141592654f;

	class MDCT
	{
		int N; // 1/1
		int N2; // 1/2
		int N4; // 1/4
		int N8; // 1/8
		int N43; // 3/4
		int LDN; // ld(n)
		float * A;
		float * B;
		float * C;
		float * buf;
	public:
		MDCT()
		{
			A  = 0;
			B  = 0;
			C  = 0;
			buf  = 0;
		}

		~MDCT()
		{
			free(A);
			free(B);
			free(C);
			free(buf);
		}

		void init(int _n)
		{
			N = _n;
			N2 = _n >> 1;
			N4 = _n >> 2;
			N8 = _n >> 3;
			N43 = 3*N4;
			LDN = ilog(_n) - 1;

			free(A);
			free(B);
			free(C);
			free(buf);

			A = (float*) malloc(sizeof(float) * N2);
			B = (float*) malloc(sizeof(float) * N2);
			for(int k=0; k<N4; k++)
			{
				A[2*k]   =  cos(4*k*PI/N);
				A[2*k+1] = -sin(4*k*PI/N);
				B[2*k]   =  cos((2*k+1)*PI/N/2);
				B[2*k+1] =  sin((2*k+1)*PI/N/2);
			}

			C = (float*) malloc(sizeof(float) * N4);
			for(int k=0; k<N8; k++)
			{
				C[2*k]   =  cos(2*(2*k+1)*PI/N);
				C[2*k+1] = -sin(2*(2*k+1)*PI/N);
			}

			buf = (float*) malloc(sizeof(float) * N);
		}

		void forward_slow(float * _in, float * _out)
		{
			for(int k=0; k<N2; k++)
			{
				float sum = 0;
				for(int i=0; i<N; i++)
				{
					sum+= _in[i] * cos((i + 0.5f + N2*0.5f) * (k + 0.5f) * 3.141592654f / N2);
				}
				_out[k] = sum * 2 / N2;
			}
		}
/*
		void inverse_slow(float * _in, float * _out)
		{
			for(int i=0; i<N; i++)
			{
				float sum = 0;
				for(int k=0; k<N2; k++)
				{
					sum+= _in[k] * cos( (i + 0.5 + N2*0.5) * (k + 0.5) * 3.141592654f / N2 );
				}
				_out[i] = sum;
			}
		}
*/
		void inverse(float * _in, float * _out)
		{
			int k, k2, k4;
			float * Y = _in;
			float * y = _out;

			float * u;
			float * v;
			float * w;
			float * X;

			// init: Y => u
			u = buf;
			for(k=0; k<N2; k++)
			{
				u[k] = Y[k];
			}

			for(k=N2; k<N; k++)
			{
				u[k] = -Y[N-k-1];
			}

			// step1
			v = _out;
			for(k2=0; k2<N2; k2+=2)
			{
				int k4 = k2 << 1;
				v[N-k4-1] = (u[k4] - u[N-k4-1]) * A[k2] - (u[k4+2] - u[N-k4-3]) * A[k2+1];
				v[N-k4-3] = (u[k4] - u[N-k4-1]) * A[k2+1] + (u[k4+2] - u[N-k4-3]) * A[k2];
			}

			// step2
			w = buf;
			for(k4=0; k4<N2; k4+=4)
			{
				w[N2+3+k4] = v[N2+3+k4] + v[k4+3];
				w[N2+1+k4] = v[N2+1+k4] + v[k4+1];
				w[k4+3] = (v[N2+3+k4] - v[k4+3]) * A[N2-4-k4] - (v[N2+1+k4] - v[k4+1]) * A[N2-3-k4];
				w[k4+1] = (v[N2+1+k4] - v[k4+1]) * A[N2-4-k4] + (v[N2+3+k4] - v[k4+3]) * A[N2-3-k4];
			}

			// step3
			u = _out;
			for(int l=0; l<LDN-3; l++)
			{
				int k0 = N >> (l+2);
				int k1 = 0x00000001 << (l+3);

				int rn = N >> (l+4);
				int s2n = 0x00000001 << (l+2);
				for(int r=0; r<rn; r++)
				{
					for(int s2=0; s2<s2n; s2+=2)
					{
						int n1s0  = N-1-k0*s2-4*r;
						int n3s0  = N-3-k0*s2-4*r;
						int n1s1 = n1s0-k0;
						int n3s1 = n3s0-k0;
						u[n1s0] = w[n1s0] + w[n1s1];
						u[n3s0] = w[n3s0] + w[n3s1];
						u[n1s1] = (w[n1s0] - w[n1s1]) * A[r*k1] - (w[n3s0] - w[n3s1]) * A[r*k1+1];
						u[n3s1] = (w[n3s0] - w[n3s1]) * A[r*k1] + (w[n1s0] - w[n1s1]) * A[r*k1+1];
					}
				}

				if(l+1 < LDN-3)
				{
					memcpy(w, u, sizeof(float) * N);
				}
			}

			// step4
			v = buf;
			for(uint32 i=0; i<N8; i++)
			{
				uint32 j=nb_reverse_bits(i, LDN-3);
				E_ASSERT(j < N8);
				if(i==j)
				{
					int i8 = i << 3;
					v[i8+1] = u[i8+1];
					v[i8+3] = u[i8+3];
					v[i8+5] = u[i8+5];
					v[i8+7] = u[i8+7];
				}
				else if(i<j)
				{
					int i8 = i << 3;
					int j8 = j << 3;
					v[j8+1] = u[i8+1]; v[i8+1] = u[j8+1];
					v[j8+3] = u[i8+3]; v[i8+3] = u[j8+3];
					v[j8+5] = u[i8+5]; v[i8+5] = u[j8+5];
					v[j8+7] = u[i8+7]; v[i8+7] = u[j8+7];
				}
			}

			// step5
			w = _out;
			for(k=0; k<N2; ++k)
			{
				w[k] = v[k*2+1];
			}

			// step6
			u = buf;
			for(k=0; k<N8; k++)
			{
				u[N-1-2*k]   = w[4*k];
				u[N-2-2*k]   = w[4*k+1];
				u[N43-1-2*k] = w[4*k+2];
				u[N43-2-2*k] = w[4*k+3];
			}

			// step7
			v = _out;
			{
				for(k=k2=0; k < N8; ++k, k2+=2)
				{
					  v[N2 + k2 ] = ( u[N2 + k2] + u[N-2-k2] + C[k2+1]*(u[N2+k2]-u[N-2-k2]) + C[k2]*(u[N2+k2+1]+u[N-2-k2+1]))/2;
					  v[N-2 - k2] = ( u[N2 + k2] + u[N-2-k2] - C[k2+1]*(u[N2+k2]-u[N-2-k2]) - C[k2]*(u[N2+k2+1]+u[N-2-k2+1]))/2;
					  v[N2+1+ k2] = ( u[N2+1+k2] - u[N-1-k2] + C[k2+1]*(u[N2+1+k2]+u[N-1-k2]) - C[k2]*(u[N2+k2]-u[N-2-k2]))/2;
					  v[N-1 - k2] = (-u[N2+1+k2] + u[N-1-k2] + C[k2+1]*(u[N2+1+k2]+u[N-1-k2]) - C[k2]*(u[N2+k2]-u[N-2-k2]))/2;
				}
			}

			// step8
			X = buf;
			for(k=k2=0; k < N4; ++k,k2 += 2)
			{
			      X[k]      = v[k2+N2]*B[k2  ] + v[k2+1+N2]*B[k2+1];
			      X[N2-1-k] = v[k2+N2]*B[k2+1] - v[k2+1+N2]*B[k2  ];
			}
			// final X => y
			for(k=0; k<N4; k++)
			{
				y[k] = X[k+N4] * 0.5f;
			}

			for(k=N4; k<N43; k++)
			{
				y[k] = -X[N43-k-1] * 0.5f;
			}

			for(k=N43; k<N; k++)
			{
				y[k] = -X[k-N43] * 0.5f;
			}

		}
	};


	// ==|======== packet =======|======== packet =======|===
	//
	// ----][--------------page------------------][---
	// ....][segment|segment|segment|segment|....][...
	class Ogg
	{
	public:
		void (*switch_page_notify)(Ogg *, void *);
		void * switch_page_notify_param;
		// page info, order is critical
		uint8 capture_pattern[4]; // 0~3
		uint8 stream_structure_version; // 4
		uint8  flags; // 5
		uint64 granule; // 6 ~13
		uint32 stream_serial_number; // 14~17
		uint32 page_sequence_no; // 18 ~ 21
		uint32 page_checksum; // 22 ~ 25
		uint8  page_segments; // 26
		uint8  segment_table[255]; // 27 ~
		// end of page info

		// int end_of_packet_count;
		int segment_index;
		//int current_lacing()
		//{ return segment_table[segment_index]; }
		int bytes_of_current_segment;
		int page_index;
		int packet_index;
		bool end_of_packet;
		FileRef file;

		bool flag_con() const
		{ return flags & 0x01 ? true : false; }

		bool flag_bos() const
		{ return flags & 0x02 ? true : false; }

		bool flag_eos() const
		{ return flags & 0x04 ? true : false; }

		bool init(FileRef & _file,
			void (*_switch_page_notify)(Ogg *, void *),
			void * _switch_page_notify_param)
		{
			switch_page_notify_param = _switch_page_notify_param;
			switch_page_notify = _switch_page_notify;
			page_index = 0;
			packet_index = 0;
			file = _file;
			flags = 0;
			if(!init_next_page())
			{
				file = 0;
				return false;
			}
			return true;
		}

		void close()
		{
			file = 0;
		}

		bool rewind()
		{
			if(!file->Seek(0))
			{
				return false;
			}
			page_index = 0;
			packet_index = 0;
			if(!init_next_page())
			{
				file = 0;
				return false;
			}
			return true;
		}

		// read at least 1 byte, never cross packet edge
		int read_some(uint8 * _buf, int _sz)
		{
			int n = _sz;
			while(n)
			{
				if(bytes_of_current_segment)
				{
					int nr = n < bytes_of_current_segment ? n : bytes_of_current_segment;
					n-= nr;
					bytes_of_current_segment-=nr;
					if(!file->Read(_buf, nr))
					{
						throw(NB_SRC_LOC "not enough input");
					}
					_buf+= nr;
				}
				else
				{
					if(segment_table[segment_index] < 255)
					{
						end_of_packet = true;
						break; // end of packet
					}

					// packet not completed
					if(++segment_index >= page_segments)
					{
						if(flag_eos() || !init_next_page() || !flag_con())
						{
							throw(NB_SRC_LOC "broken packet");
						}
					}
					bytes_of_current_segment = segment_table[segment_index];
				}
			}
			return _sz - n;
		}

		bool skip_to_next_packet()
		{
			bool is_packet_edge;
			do
			{
				uint8 buf[256];
				if(bytes_of_current_segment)
				{
					// message(L"[nb] (WW) Ogg: skip " + string(bytes_of_current_segment) + L" bytes.");
					if(!file->Read(buf, bytes_of_current_segment))
					{
						end_of_packet = false;
						E_ASSERT(0);
						return false;
					}
					bytes_of_current_segment = 0;
				}

				is_packet_edge = segment_table[segment_index] < 255;
				if(++segment_index >= page_segments)
				{
					if(flag_eos() || !init_next_page() || flag_con() == is_packet_edge)
					{
						end_of_packet = true;
						// E_TRACE_LINE("[nb]  # Ogg: end of last packet.");
						return false;
					}
				}

				bytes_of_current_segment = segment_table[segment_index];
			}while(!is_packet_edge);

			end_of_packet = false;
			packet_index++;
#if NB_CFG_VERBOSE
			bool exact = false;
			int n = 0;
			for(int i=segment_index; i<page_segments; i++)
			{
				n+= segment_table[i];
				if(segment_table[segment_index] < 255)
				{
					exact = true;
					break;
				}
			}
			E_TRACE("[nb]  # packet: " + string(packet_index));
			if(exact)
			{
				E_TRACE_LINE(", " + string(n) + " bytes");
			}
			else
			{
				E_TRACE_LINE(", aleast " + string(n) + " bytes");
			}
#endif
			return true;
		}

		bool skip_to_next_page()
		{
			if(flag_eos())
			{
				return false;
			}
			uint32 skip = bytes_of_current_segment;
			for(int i=segment_index+1; i<page_segments; i++)
			{
				skip+= segment_table[i];
			}
			return file->Skip(skip) && init_next_page();
		}

	private:
		bool init_next_page()
		{
			if(flag_eos())
			{
				return false;
			}

			if(!file->Read(capture_pattern, 4)
				|| memcmp(capture_pattern, "OggS", 4)!=0
				|| !file->Read(&stream_structure_version, 1)
				|| stream_structure_version != 0 // version
				|| !file->Read(&flags, 1)
				|| !file->Read(&granule, 8)
				|| !file->Read(&stream_serial_number, 4)
				|| !file->Read(&page_sequence_no, 4)
				|| !file->Read(&page_checksum, 4)
				|| !file->Read(&page_segments, 1)
				)
			{
				return false;
			}

			if(!file->Read(segment_table, page_segments))
			{
				return false;
			}

			page_index ++;
			segment_index = 0;
			bytes_of_current_segment = segment_table[0];

#ifdef NB_CFG_VERBOSE
			E_TRACE("[nb] ogg: page[" + string(page_index) + L"]: segs=" + string(int(page_segments)));
			//E_TRACE(L", sn=" + string(stream_serial_number));
			E_TRACE(L", page=" + string(page_sequence_no));
			E_TRACE(L", granule=" + string((int)granule));
			if(flag_bos())
			{
				E_TRACE(" (first)");
			}
			if(flag_con())
			{
				E_TRACE(" (continued)");
			}
			if(flag_eos())
			{
				E_TRACE(" (last)");
			}

			E_TRACE_LINE("");
#endif

			if(switch_page_notify)
			{
				switch_page_notify(this, switch_page_notify_param);
			}

			return true;
		}

	};


	class Vorbis : public SoundFileReader
	{
		static const int MAX_PARTITION_COUNT = 256;
		Ogg ogg;
		bool   header_decode_complete;
		bool   end_of_packet; // difference with ogg.end_of_packet
		bool   end_of_stream;
		uint64 bit_buf;
		uint32 bit_num;

		uint32 vorbis_version;
		uint8  audio_channels;
		uint32 audio_frame_rate;
		uint32 bitrate_maximum;
		uint32 bitrate_nominal;
		uint32 bitrate_minimum;
		uint32 block_size[2];
		//int    frame_type[2];
		float * slope[2];
		int    pcm_block_align;

		// pre frame

		//   |<------- w0 ------->|
		//   +========+     +=====|==+
		//   |        |\   /      |  |
		//   0       a0 \ /       |  |
		//               +        |  |
		//       0   a1 / \       |  |
		//       |    |/   \      |  |
		//       +====+     +=====+  |
		//       |<------- w1 ------>|


		struct OVERLAP
		{
			float* s; // slope
			int sw; // slope width
			int a0;
			int w0;
			int a1;
			int w1;
			int pcm_count;
			void add(float * _o, const float * _l, const float * _r)
			{
				//int ret = 0;
				for(int i=a0; i>0; i--)
				{
					*_o++ = *_l++;
				//	ret++;
				}
				_r+= a1;

				float* s0 = s + sw - 1;
				float* s1 = s;
				for(int i=sw; i>0; i--)
				{
					*_o++ = (*_l++) * (*s0--) + (*_r++) * (*s1++);
				//	ret++;
				}

				for(int i=a1+sw; i<w1; i++)
				{
					*_o++ = *_r++;
				//	ret++;
				}

				//E_ASSERT(ret == pcm_count);
			}
		} overlap[2][2];

		void init_overlap()
		{
			for(int i=0; i<2; i++)
			{
				int w = block_size[i];
				int hw = w >> 1;
				E_ASSERT(slope[i] == 0);
				slope[i] = (float*)malloc(sizeof(float) * hw);
				float * s = slope[i];
				for(int n=0; n<hw; n++)
				{
					float a = sin((n + 0.5f) / w * 3.141592654f);
					s[n] = sin(0.5f * 3.141592654f * a*a);
				}
				// dump_buf(s, hw);
			}

			OVERLAP * ov;
			ov = &overlap[0][0];
			ov->sw = block_size[0] >> 1;
			ov->s  = slope[0];
			ov->w0 = block_size[0] >> 1;
			ov->w1 = block_size[0] >> 1;
			ov->a0 = 0;
			ov->a1 = 0;
			ov->pcm_count = (ov->w0 >> 1) + (ov->w1 >> 1);

			ov = &overlap[0][1];
			ov->sw = block_size[0] >> 1;
			ov->s  = slope[0];
			ov->w0 = block_size[0] >> 1;
			ov->w1 = block_size[1] >> 1;
			ov->a0 = 0;
			ov->a1 = (ov->w1>>1) - (ov->w0>>1);
			ov->pcm_count = (ov->w0 >> 1) + (ov->w1 >> 1);


			ov = &overlap[1][0];
			ov->sw = block_size[0] >> 1;
			ov->s  = slope[0];
			ov->w0 = block_size[1] >> 1;
			ov->w1 = block_size[0] >> 1;
			ov->a0 = (ov->w0>>1) - (ov->w1>>1);
			ov->a1 = 0;
			ov->pcm_count = (ov->w0 >> 1) + (ov->w1 >> 1);

			ov = &overlap[1][1];
			ov->sw = block_size[1] >> 1;
			ov->s  = slope[1];
			ov->w0 = block_size[1] >> 1;
			ov->w1 = block_size[1] >> 1;
			ov->a0 = 0;
			ov->a1 = 0;
			ov->pcm_count = (ov->w0 >> 1) + (ov->w1 >> 1);
		}

		//uint32 bytesTotal;
		string title;

		MDCT mdct[2];

#define FLOOR1_BUF_SZ (9*32+2)
		struct CHANNEL_BUF
		{
			int floor1_Y[FLOOR1_BUF_SZ];
			int floor1_Y_size;
			bool floor_unused;
			float floor[4096];
			float residue[4096];
			//float audio[8192];
			float prev_half_audio[4096];
			float pcm[4096];
		};

		CHANNEL_BUF * channel_bufs;
		int pre_window_flag;
		float * tmp_buf;
		uint32  tmp_buf_size;

		// inner buf
		float *  inner_buf_float;
		uint32   inner_buf_float_size;
		uint32   inner_buf_float_len;
		uint32   inner_buf_float_offset;

		// pointer to ourter buf
		float *  out_buf_float;
		int      out_buf_float_size;
		int      out_buf_float_len;

		uint64   pcm_write_total; // write_to_inner_buf + write_to_outer_buf
		int      pcm_write_partial;

		bool     total_pcm_avaible_known;
		uint64   total_pcm_avaible;
		class CODEBOOK : protected ::e::DecodeHuffman32
		{
		public:
			int    id;
			// huffman code lengths
			uint32 code_dimensions;
			bool   code_length_is_ordered;
			int    code_count;
			uint8* code_lengths;

			int    lookup_type;
			float  minimum_value;
			float  delta_value;
			uint32 value_bits;
			bool   sequence_p;
			uint32 lookup_value_count;
			uint8* multiplicands;

			CODEBOOK()
			{
				code_lengths = 0;
				multiplicands = 0;
			}

			~CODEBOOK()
			{
				free(code_lengths);
				free(multiplicands);
			}

			bool construct_huffman()
			{
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE(L"[nb]  book[" + string(id) + L"]:"
					+ L" dim=" + string(code_dimensions)
					+ L",\tcount=" + string(code_count)
					+ L",\tVQ=" + string(lookup_type)
					);
#endif

				if(code_lengths==0
					|| !DecodeHuffman32::Init(code_lengths, code_count, 1))
				{
					return false;
				}

				free(code_lengths);
				code_lengths = 0;
				return true;
			}


			void read_config(Vorbis * _vorbis)
			{
//				E_TRACE_LINE(L"CODEBOOK::read_config(): id=" + string(id));
				uint8 buf[4];
				_vorbis->read_bytes_and_drop(buf, 3);
				if(buf[0] != 0x42 || buf[1] != 0x43 || buf[2] != 0x56)
				{
					throwf(NB_SRC_LOC "corrupted codebook %d", id);
				}

				this->code_dimensions =_vorbis->read_bits_and_drop(2*8);
				this->code_count = _vorbis->read_bits_and_drop(3*8);
				this->code_length_is_ordered = _vorbis->read_bits_and_drop(1) ? true : false;
				this->code_lengths = (uint8*) malloc(sizeof(uint8) * this->code_count);
				memset(this->code_lengths, 0, sizeof(uint8) * this->code_count);

				if(this->code_length_is_ordered)
				{
					int current_entry = 0;
					uint32 current_length = _vorbis->read_bits_and_drop(5) + 1;
					while(current_entry < this->code_count)
					{
						int n = ilog(this->code_count - current_entry);
						uint32 number = _vorbis->read_bits_and_drop(n);
						//current_length = _vorbis->read_bits_and_drop(n);

						for(int i=current_entry; i<current_entry+number; i++)
						{
							this->code_lengths[i] = current_length;
						}
						current_entry+= number;
						current_length++;
						if(current_entry > this->code_count)
						{
							throw(NB_SRC_LOC "extra codebook entry");
						}
					}
				}
				else
				{
					// uint32 length = 0;
					uint32 sparse= _vorbis->read_bits_and_drop(1);

					if(sparse)
					{
						for(int current_entry = 0; current_entry < this->code_count; current_entry++)
						{
							if(_vorbis->read_bits_and_drop(1))
							{
								this->code_lengths[current_entry]= _vorbis->read_bits_and_drop(5) + 1;
							}
							else
							{
								// leave it as 0
							}
						}
					}
					else
					{
						for(int current_entry = 0; current_entry < this->code_count; current_entry++)
						{
							this->code_lengths[current_entry]= _vorbis->read_bits_and_drop(5) + 1;
						}
					}
				}


				this->lookup_type = _vorbis->read_bits_and_drop(4);
				if(this->lookup_type > 0)
				{
					if(this->lookup_type > 2)
					{
						throw(NB_SRC_LOC "unsupported lookup type");
					}

					this->minimum_value = float32_unpack(_vorbis->read_bits_and_drop(32));
					this->delta_value = float32_unpack(_vorbis->read_bits_and_drop(32));
					this->value_bits = _vorbis->read_bits_and_drop(4) + 1;
					this->sequence_p = _vorbis->read_bits_and_drop(1) ? true : false;

					if(this->lookup_type == 1)
					{
						this->lookup_value_count = lookup1_values(this->code_count, this->code_dimensions);
					}
					else
					{
						E_ASSERT(this->lookup_type == 2);
						this->lookup_value_count = this->code_count * this->code_dimensions;
					}

					this->multiplicands = (uint8*) malloc(sizeof(uint8) * this->lookup_value_count);
					for(int i=0; i<this->lookup_value_count; i++)
					{
						this->multiplicands[i] = (uint8)(_vorbis->read_bits_and_drop(this->value_bits) & 0xff);
					}
				}

				if(!this->construct_huffman())
				{
					throw(NB_SRC_LOC "corrupped huffman book");
				}

			}

			inline uint32 decode(Vorbis * _vorbis)
			{ return DecodeHuffman32::Decode(_vorbis); }

			void decode_vector(Vorbis * _vorbis, float * _vector, int _sz)
			{
				E_ASSERT(lookup_type == 1 || lookup_type == 2);
				uint32 lookup_offset = DecodeHuffman32::Decode(_vorbis);
				int sz = code_dimensions < _sz ? code_dimensions : _sz;
				switch(lookup_type)
				{
				case 1:
					{
						float last = 0;
						int index_divisor = 1;
						for(int i=0; i<sz; i++)
						{
							int multiplicand_offset = (lookup_offset / index_divisor) % lookup_value_count;
							_vector[i] = this->multiplicands[multiplicand_offset] * this->delta_value + this->minimum_value + last;
							if(this->sequence_p)
							{
								last = _vector[i];
							}
							index_divisor*= lookup_value_count;
						}
					}
					break;
				case 2:
					{
						float last = 0;
						int multiplicand_offset = lookup_offset * code_dimensions;
						for(int i=0; i<sz; i++)
						{
							_vector[i] = this->multiplicands[multiplicand_offset] * this->delta_value + this->minimum_value + last;
							if(this->sequence_p)
							{
								last = _vector[i];
							}
							multiplicand_offset++;
						}
					}
					break;
				default:
					throw(NB_SRC_LOC "unkown vector lookup type");
				}
			}
		};

		int codebook_count;
		CODEBOOK * codebooks;


		// floor0 is unsupported
		struct FLOOR
		{
			struct F1AS
			{
				int flag; // floor1_step2_flag
				int x; // floor1_x_list'
				int y; // floor1_final_y'
			};

			static int compare_f1as(const void * _l, const void * _r)
			{
				return ((const F1AS*)_l)->x - ((const F1AS*)_r)->x;
			}

			uint32 type;

			uint32 partitions;
			uint32 class_count;
			uint8 partition_class_list[32];
			uint8 class_dimensions[17];
			uint8 class_subclasses[17];
			uint8 class_masterbooks[17];
			int   subclass_books[17][8];
			uint8 multiplier;
			uint8 rangebits;
			int   x_list_size;
			int   x_list[34];
			int   values;

			void read_config(Vorbis* _vorbis)
			{
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE("[nb]   read floor config.");
#endif
				type = _vorbis->read_bits_and_drop(16);
				if(type != 1)
				{
					throwf(NB_SRC_LOC "unsupported floor type: %d", type);
				}
				this->partitions = _vorbis->read_bits_and_drop(5);
				this->class_count = 0;
				for(int i=0; i<this->partitions; i++)
				{
					this->partition_class_list[i] = _vorbis->read_bits_and_drop(4);
					if(this->partition_class_list[i] + 1 > this->class_count)
					{
						this->class_count = this->partition_class_list[i] + 1;
					}
				}

				for(int i=0; i<this->class_count; i++)
				{
					this->class_dimensions[i] = _vorbis->read_bits_and_drop(3) + 1;
					this->class_subclasses[i] = _vorbis->read_bits_and_drop(2);
					if(this->class_subclasses[i])
					{
						this->class_masterbooks[i] = _vorbis->read_bits_and_drop(8);
						if(class_masterbooks[i] >= _vorbis->codebook_count)
						{
							throw(NB_SRC_LOC "corrupted floor config");
						}
					}
					int jj = 0x00000001 << this->class_subclasses[i];
					E_ASSERT(jj < 8);
					for(int j=0; j<jj; j++)
					{
						this->subclass_books[i][j] = int(_vorbis->read_bits_and_drop(8)) - 1;
						if(subclass_books[i][j] >= _vorbis->codebook_count)
						{
							throw(NB_SRC_LOC "corrupted floor config");
						}
					}
				}

				this->multiplier = _vorbis->read_bits_and_drop(2) + 1;
				this->rangebits =  _vorbis->read_bits_and_drop(4);
				this->x_list_size = this->partitions + 2;
				this->x_list[0] = 0;
				this->x_list[1] = 0x0000001 << this->rangebits;
				this->values = 2;
				for(int i=0; i<this->partitions; i++)
				{
					uint8 current_class_number = this->partition_class_list[i];
					for(int j=0; j<this->class_dimensions[current_class_number]; j++)
					{
						if(this->values >= this->x_list_size)
						{
							this->x_list_size+= this->partitions;
							E_ASSERT(this->values < sizeof(this->x_list));
						}
						this->x_list[this->values] = _vorbis->read_bits_and_drop(this->rangebits);
						this->values++;
					}
				}
			}
			void decode(Vorbis * _vorbis, CHANNEL_BUF & _buf, int _half_n)
			{
				// 7.2.2 floor1 packet decode
				E_ASSERT(this->type == 1);
				uint32 nonzero = _vorbis->read_bits_and_drop(1);
				_buf.floor_unused = nonzero ? false : true;
				if(_buf.floor_unused)
				{
					memset(_buf.floor, 0, _half_n * sizeof(float));
					return;
				}

				//_buf.has_floor = true;
				static const int v1[] = {256, 128, 86, 64};
				int range = v1[this->multiplier-1];
				int bits = ilog(range - 1);
				// cdim <= 9
				// partitions <= 32
				_buf.floor1_Y[0] = _vorbis->read_bits_and_drop(bits);
				_buf.floor1_Y[1] = _vorbis->read_bits_and_drop(bits);
				int offset = 2;
				for(int i=0; i<this->partitions; i++)
				{
					uint8 clss = this->partition_class_list[i];
					uint8 cdim = this->class_dimensions[clss];
					uint8 cbits = this->class_subclasses[clss];
					uint32 csub = (0x00000001 << cbits)-1;
					// 10
					uint32 cval = 0;
					// 11
					if(cbits)
					{
						CODEBOOK & cb = _vorbis->codebooks[this->class_masterbooks[clss]];
						cval = cb.decode(_vorbis);
					}

					// 13
					for(int j=0; j<cdim; j++)
					{
						// 14
						int book = this->subclass_books[clss][cval & csub];
						// 15
						cval = cval >> cbits;
						// 16
						if(book >= 0)
						{
							// 17
							_buf.floor1_Y[j + offset] = _vorbis->codebooks[book].decode(_vorbis);
						}
						else
						{
							// 18
							_buf.floor1_Y[j + offset] = 0;
						}
					}
					// 19
					offset+= cdim;
				}
				_buf.floor1_Y_size = offset;

				// 7.2.2 curve computation
				// step 1: amplitude value synthesis
				F1AS f1as[34];
				f1as[0].flag = true;
				f1as[1].flag = true;
				f1as[0].y = _buf.floor1_Y[0];
				f1as[1].y = _buf.floor1_Y[1];
				for(int i=0; i<this->values; i++)
				{
					f1as[i].x = this->x_list[i];
				}

				for(int i=2; i<this->values; i++)
				{
					int low_neighbor_offset  = low_neighbor(this->x_list, i);
					int high_neighbor_offset = high_neighbor(this->x_list, i);
					E_ASSERT(low_neighbor_offset != i && high_neighbor_offset != i && high_neighbor_offset != low_neighbor_offset);
					int predicted = render_point(
						f1as[low_neighbor_offset].x, f1as[low_neighbor_offset].y,
						f1as[high_neighbor_offset].x, f1as[high_neighbor_offset].y,
						f1as[i].x
						);
					int val = _buf.floor1_Y[i];
					int highroom = range - predicted;
					int lowroom = predicted;
					int room = (highroom < lowroom) ? highroom*2 : lowroom * 2;
					if(val)
					{
						f1as[low_neighbor_offset].flag = true;
						f1as[high_neighbor_offset].flag = true;
						f1as[i].flag = true;
						// 20
						if(val >= room)
						{
							//21
							if(highroom > lowroom)
							{
								//22
								f1as[i].y = val - lowroom + predicted;
							}
							else
							{
								//23
								f1as[i].y = predicted - val + highroom - 1;
							}
						}
						else
						{
							// 24
							if(val & 0x01)
							{
								// 25
								f1as[i].y = predicted - ((val+1) / 2);
							}
							else
							{
								// 26
								f1as[i].y = predicted + (val / 2);

							}
						}
					}
					else
					{
						// 27
						f1as[i].flag = false;
						// 28
						f1as[i].y = predicted;
					}
				}
				//29

				// step 2, curve synthesis
				int floor_buf[8192 + 1024]; // ?

				qsort(f1as, this->values, sizeof(F1AS), &compare_f1as);
				int hx = 0;
				int hy = 0;
				int lx = 0;
				int ly = f1as[0].y * this->multiplier;
				// 4
				for(int i=1; i <this->values; i++)
				{
					// 5
					if(f1as[i].flag)
					{
						// 6
						hy = f1as[i].y * this->multiplier;
						// 7
						hx = f1as[i].x;
						// 8
						render_line(lx, ly, hx, hy, floor_buf);
						// 9
						lx = hx;
						// 10
						ly = hy;
					}
				}
				if(hx < _half_n) // TODO: _half_n or n?
				{
					render_line(hx, hy, _half_n, hy, floor_buf);
				}
				else if(hx > _half_n)
				{
					// truncate
				}
				// E_TRACE_LINE("//\n//\ndata\nfloor\n");
				for(int i=0; i<_half_n; i++)
				{
					E_ASSERT(floor_buf[i]>= 0 && floor_buf[i] < 256);
					_buf.floor[i] = floor1_inverse_dB_table[floor_buf[i]];
					// E_TRACE_LINE(string(floor_buf[i]));
				}
				// E_TRACE_LINE("//\nplot type=line data.invert=true\n//\n//\n");
				return;
			}
		private:
//			void read_config_0(Vorbis * _vorbis)
//			{
//				E_TRACE_LINE("[nb]   floor configuration type 0");
//				floor0.order = _vorbis->read_bits_and_drop(8);
//				floor0.rate = _vorbis->read_bits_and_drop(16);
//				floor0.bark_map_size = _vorbis->read_bits_and_drop(16);
//				floor0.amplitude_bits = _vorbis->read_bits_and_drop(6);
//				floor0.amplitude_offset = _vorbis->read_bits_and_drop(8);
//				floor0.number_of_books = _vorbis->read_bits_and_drop(4) + 1;
//				for(int i=0; i<floor0.number_of_books; i++)
//				{
//					_vorbis->read_bytes_and_drop(floor0.book_list + i, 1);
//					if(*(floor0.book_list + i) > _vorbis->codebook_count)
//					{
//						throw(NB_SRC_LOC "bad floor0 configuration");
//					}
//				}
//
//			}

		};

		FLOOR * floors;
		uint32 floor_count;

		struct RESIDUE
		{
			uint32 type;
			uint32 begin;
			uint32 end;
			uint32 partition_size;
			uint32 resudue_classifications; // don't rename
			uint32 classbook;
			uint8 residue_cascade[65];
			int residue_books[65][8];

			void read_config(Vorbis * _vorbis)
			{
				type = _vorbis->read_bits_and_drop(16);
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE(L"[nb]   read residue config, type=" + string(type));
#endif
				if(type > 3)
				{
					throw(NB_SRC_LOC "unsupported residues type");
				}
				begin = _vorbis->read_bits_and_drop(24);
				end   = _vorbis->read_bits_and_drop(24);
				partition_size = _vorbis->read_bits_and_drop(24) + 1;
				resudue_classifications = _vorbis->read_bits_and_drop(6) + 1;
				classbook = _vorbis->read_bits_and_drop(8);
				//residue_cascade = (uint8*) malloc(classifications);
				for(int i=0; i<resudue_classifications; i++)
				{
					uint32 hight_bits = 0;
					uint32 low_bits = _vorbis->read_bits_and_drop(3);
					uint32 bitflag = _vorbis->read_bits_and_drop(1);
					if(bitflag)
					{
						hight_bits = _vorbis->read_bits_and_drop(5);
					}
					residue_cascade[i] = (hight_bits << 3) | low_bits;
				}
				//residue_books = (RB1*) malloc(sizeof(RB1) * classifications);
				//memset(residue_books, 0, sizeof(RB1) * classifications);
				for(int i=0; i<resudue_classifications; i++)
				{
					int * rb = residue_books[i];
					for(int j=0; j<8; j++)
					{
						if((residue_cascade[i] >> j) & 0x01)
						{
							rb[j] = _vorbis->read_bits_and_drop(8); // TODO: check overflow?
						}
						else
						{
							rb[j] = -1;
						}
					}
				}
			}

			void decode_partition_format_0(Vorbis * _vorbis, CODEBOOK & _vqbook, float * v, int offset, int n)
			{
/*
1   1) [step] = [n] / [codebook_dimensions]
2   2) iterate [i] over the range 0 ... [step]-1 {
4        3) vector [entry_temp] = read vector from packet using current codebook in VQ context
5        4) iterate [j] over the range 0 ... [codebook_dimensions]-1 {
7             5) vector [v] element ([offset]+[i]+[j]*[step]) =
8           vector [v] element ([offset]+[i]+[j]*[step]) +
9                  vector [entry_temp] element [j]
11           }
13      }
15    6) done
 */
				E_ASSERT(_vqbook.code_dimensions <= 64);
				float entry_temp[64];
				int step = n / _vqbook.code_dimensions;
 	 	 	 	for(int i=0; i<step; i++)
 	 	 	 	{
 	 	 	 		_vqbook.decode_vector(_vorbis, entry_temp, _vqbook.code_dimensions);
 	 	 	 		for(int j=0; j<_vqbook.code_dimensions; j++)
 	 	 	 		{
 	 	 	 			v[offset+i+j*step]+= entry_temp[j];
 	 	 	 		}
 	 	 	 	}
			}

			void decode_partition_format_1(Vorbis * _vorbis, CODEBOOK & _vqbook, float * v, int offset, int n)
			{
				/*
				1   1) [i] = 0
				2   2) vector [entry_temp] = read vector from packet using current codebook in VQ context
				3   3) iterate [j] over the range 0 ... [codebook_dimensions]-1 {
				4
				5        4) vector [v] element ([offset]+[i]) =
				6     vector [v] element ([offset]+[i]) +
				7            vector [entry_temp] element [j]
				8        5) increment [i]
				9
				10      }
				11
				12    6) if ( [i] is less than [n] ) continue at step 2
				13    7) done
				*/

				E_ASSERT(_vqbook.code_dimensions <= 64);
				float entry_temp[64];
				int i=0;
				do
				{
					_vqbook.decode_vector(_vorbis, entry_temp, _vqbook.code_dimensions);
					for(int j=0; j<_vqbook.code_dimensions; j++)
					{
						v[offset+i]+= entry_temp[j];
						i++;
					}
				}while(i < n);

			}

			void decode_format_01(Vorbis * _vorbis, CHANNEL_BUF * _buf_ch_order[], int _ch_count, int _sz)
			{
				E_ASSERT(this->type != 2);

				for(int i=0; i<_ch_count; i++)
				{
					memset(_buf_ch_order[i]->residue, 0, _sz * sizeof(float));
				}

				// 8.6.2
				// 1
				uint32 actual_size = _sz;
				// 2

				uint32 limit_residue_begin = this->begin;
				uint32 limit_residue_end   = actual_size < this->end   ? actual_size : this->end ;
				int bytes_to_read = limit_residue_end - limit_residue_begin;
				if(bytes_to_read == 0)
				{
					return ;
				}
				int partition_count = bytes_to_read / this->partition_size;

				// 1
				CODEBOOK & classbook = _vorbis->codebooks[this->classbook];
				int classwords_per_codeword = classbook.code_dimensions;
				// 2
				// 3
				if(_sz > 4096 || classwords_per_codeword > 64 || partition_count > MAX_PARTITION_COUNT || _ch_count > 64)
				{
					throw(NB_SRC_LOC "internal errors");
				}

				//float format2_buf[4096 * 2];

				// int classifications[65536];
				int classifications[64][MAX_PARTITION_COUNT+64];
				for(int pass=0; pass<8; pass++)
				{
					int partition_id = 0;
					while(partition_id < partition_count)
					{
						// 6
						if(pass == 0)
						{
							for(int ch=0; ch<_ch_count; ch++)
							{
								// 8
								if(!_buf_ch_order[ch]->floor_unused)
								{
									// 9
									uint32 temp = classbook.decode(_vorbis);
									// 10
									for(int i=classwords_per_codeword-1; i>=0; i--)
									{
										uint32 cls = temp % this->resudue_classifications;
										classifications[ch][i+partition_id] = cls;
										temp = temp / this->resudue_classifications;
									}
								}
							}
						}
						// 13
						for(int i = 0; i<classwords_per_codeword && partition_id < partition_count; i++)
						{
							// 14
							for(int ch=0; ch<_ch_count; ch++)
							{
								CHANNEL_BUF * buf = _buf_ch_order[ch];
								if(!buf->floor_unused)
								{
									// 16
									int vqclass = classifications[ch][partition_id];
									// 17
									int vqbookid = residue_books[vqclass][pass];
									// 18
									if(vqbookid >= 0)
									{
										// 19
										CODEBOOK & vqbook = _vorbis->codebooks[vqbookid];
										int n = partition_size;
										float * v = buf->residue;
										int offset = limit_residue_begin + partition_id*partition_size;
										switch(type)
										{
										case 0:
											decode_partition_format_0(_vorbis, vqbook, v, offset, n);
											break;
										case 1:
											decode_partition_format_1(_vorbis, vqbook, v, offset, n);
											break;
										}
									}
								}
								partition_id++;
							}
						}
					}
				}
				// 21done
			}

			void decode_format_2(Vorbis * _vorbis, CHANNEL_BUF * _buf_ch_order[], int _ch_count, int _sz)
			{
				E_ASSERT(this->type == 2);
				bool need_decode = false;
				for(int ch=0; ch<_ch_count; ch++)
				{
					if(!_buf_ch_order[ch]->floor_unused)
					{
						need_decode = true;
					}
				}



				// 8.6.2
				uint32 actual_size = _sz * _ch_count;
				_vorbis->require_tmp_buf_size(actual_size);
				memset(_vorbis->tmp_buf, 0, actual_size*sizeof(float));
				if(need_decode)
				{
					uint32 limit_residue_begin = this->begin;
					uint32 limit_residue_end   = actual_size < this->end   ? actual_size : this->end ;
					int bytes_to_read = limit_residue_end - limit_residue_begin;
					if(bytes_to_read == 0)
					{
						return ;
					}
					int partition_count = bytes_to_read / this->partition_size;

					// 1
					CODEBOOK & classbook = _vorbis->codebooks[this->classbook];
					int classwords_per_codeword = classbook.code_dimensions;
					// 2
					if(_sz > 4096 || classwords_per_codeword > 64 || partition_count > MAX_PARTITION_COUNT || _ch_count > 64)
					{
						throw(NB_SRC_LOC "internal errors");
					}

					//float format2_buf[4096 * 2];

					// int classifications[65536];
					int classifications[MAX_PARTITION_COUNT+64];
					for(int pass=0; pass<8; pass++)
					{
						int partition_id = 0;
						while(partition_id < partition_count)
						{
							// 6
							if(pass == 0)
							{
								// 9
								uint32 temp = classbook.decode(_vorbis);
								// 10
								for(int i=classwords_per_codeword-1; i>=0; i--)
								{
									uint32 cls = temp % this->resudue_classifications;
									classifications[i+partition_id] = cls;
									temp = temp / this->resudue_classifications;
								}
							}
							// 13
							for(int i = 0; i<classwords_per_codeword && partition_id < partition_count; i++)
							{
								// 16
								int vqclass = classifications[partition_id];
								// 17
								int vqbookid = residue_books[vqclass][pass];
								// 18
								if(vqbookid >= 0)
								{
									// 19
									CODEBOOK & vqbook = _vorbis->codebooks[vqbookid];
									int n = partition_size;
									float * v = _vorbis->tmp_buf;
									int offset = limit_residue_begin + partition_id*partition_size;
									decode_partition_format_1(_vorbis, vqbook, v, offset, n);
								}

								partition_id++;
							}
						}
					}
				}

				// post decode
				for(int ch=0; ch<_ch_count; ch++)
				{
					float * r = _buf_ch_order[ch]->residue;
					float * p = _vorbis->tmp_buf + ch;
					float * end = p + actual_size;
					while(p < end)
					{
						*r++ = *p;
						p+= _ch_count;
					}
				}
			}
		};

		RESIDUE * residues;
		uint32 vorbis_residue_count;

		struct MAPPING
		{
			// uint32 type; // no reason to save
			uint8 mapping_submaps;
			uint32 vorbis_mapping_coupling_steps;
			uint32 vorbis_mapping_magnitude[256];
			uint32 vorbis_mapping_angle[256];
			uint8 vorbis_mapping_mux[256];
			uint8 vorbis_mapping_submap_floor[16];
			uint8 vorbis_mapping_submap_residue[16];

			void read_config(Vorbis * _vorbis)
			{
				uint32 type = _vorbis->read_bits_and_drop(16);
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE(L"[nb]   read mapping config, type=" + string(type));
#endif
				if(type != 0)
				{
					throw(NB_SRC_LOC "unsupported mapping type");
				}

				// i
				if(_vorbis->read_bits_and_drop(1))
				{
					// A
					mapping_submaps = _vorbis->read_bits_and_drop(4) + 1;
				}
				else
				{
					// B
					mapping_submaps = 1;
				}

				// ii
				if(_vorbis->read_bits_and_drop(1))
				{
					// A
					vorbis_mapping_coupling_steps = _vorbis->read_bits_and_drop(8) + 1;
					//vorbis_mapping_magnitude = (uint32*) malloc(sizeof(uint32) * vorbis_mapping_coupling_steps);
					//vorbis_mapping_angle = (uint32*) malloc(sizeof(uint32) * vorbis_mapping_coupling_steps);
					for(int j=0; j<vorbis_mapping_coupling_steps; j++)
					{
						int n = ilog(_vorbis->audio_channels - 1);
						vorbis_mapping_magnitude[j] = _vorbis->read_bits_and_drop(n);
						vorbis_mapping_angle[j] = _vorbis->read_bits_and_drop(n);
						if(vorbis_mapping_magnitude[j] == vorbis_mapping_angle[j] ||
							vorbis_mapping_magnitude[j] >= _vorbis->audio_channels ||
							vorbis_mapping_angle[j] >= _vorbis->audio_channels)
						{
							throw(NB_SRC_LOC "corrupted mapping info");
						}
					}
				}
				else
				{
					// B
					vorbis_mapping_coupling_steps = 0;
				}

				// iii
				if(_vorbis->read_bits_and_drop(2) != 0)
				{
					throw(NB_SRC_LOC "corrupted mapping info");
				}

				// iv
				if(mapping_submaps > 1)
				{
					// vorbis_mapping_mux = (uint8*) malloc(_vorbis->audio_channels);
					for(int j=0; j<_vorbis->audio_channels; j++)
					{
						// A
						vorbis_mapping_mux[j] = _vorbis->read_bits_and_drop(4);
						// B
						if(vorbis_mapping_mux[j] > mapping_submaps-1)
						{
							throw(NB_SRC_LOC "corrupted mapping info");
						}
					}
				}

				// v
				//vorbis_mapping_submap_floor = (uint8*) malloc(mapping_submaps);
				//vorbis_mapping_submap_residue = (uint8*) malloc(mapping_submaps);
				for(int j=0; j<mapping_submaps; j++)
				{
					_vorbis->read_bits_and_drop(8);
					vorbis_mapping_submap_floor[j] = _vorbis->read_bits_and_drop(8);
					vorbis_mapping_submap_residue[j] = _vorbis->read_bits_and_drop(8);
					if(vorbis_mapping_submap_floor[j] >= _vorbis->floor_count
						|| vorbis_mapping_submap_residue[j] >= _vorbis->vorbis_residue_count)
					{
						throw(NB_SRC_LOC "corrupted mapping info");
					}
				}

				// vi
			}
		};
		MAPPING * mappings;
		uint32 vorbis_mapping_count;

		struct MODE
		{
			int vorbis_mode_blockflag;
			uint16 vorbis_mode_windowtype;
			uint16 vorbis_mode_transformtype;
			uint8 vorbis_mode_mapping;
			void read_config(Vorbis * _vorbis)
			{
				vorbis_mode_blockflag = _vorbis->read_bits_and_drop(1);
				vorbis_mode_windowtype = _vorbis->read_bits_and_drop(16);
				vorbis_mode_transformtype = _vorbis->read_bits_and_drop(16);
				vorbis_mode_mapping = _vorbis->read_bits_and_drop(8);
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE(L"[nb]   read mode config, blockflag=" + string(vorbis_mode_blockflag)
					+ L", windowtype=" + string(vorbis_mode_windowtype)
					+ L", transformtype=" + string(vorbis_mode_transformtype)
					+ L", mapping=" + string(vorbis_mode_mapping)
					);
#endif
					if(vorbis_mode_windowtype != 0 ||
					vorbis_mode_transformtype != 0 ||
					vorbis_mode_mapping >= _vorbis->vorbis_mapping_count)
				{
					throw(NB_SRC_LOC "unsupported modes or corrupted");
				}
			}
		};
		MODE * modes;
		uint32 vorbis_mode_count;

	private:
		void init_vars()
		{
			codebooks = 0;
			floors = 0;
			residues = 0;
			mappings = 0;
			modes = 0;
			channel_bufs = 0;
			inner_buf_float = 0;
			slope[0] = 0;
			slope[1] = 0;
			tmp_buf  = 0;
			inner_buf_float_size = 0;
			inner_buf_float_len = 0;
			inner_buf_float_offset = 0;
			out_buf_float = 0;
			memset(overlap, 0, sizeof(overlap));
			tmp_buf_size = 0;
			pcm_write_total = 0;
			pcm_write_partial = 0;
			total_pcm_avaible_known = false;
			//bytesTotal = 0;
			end_of_packet = false;
			end_of_stream = false;

		}

		void free_vars()
		{
			delete[] codebooks;
			free(residues);
			free(floors);
			free(mappings);
			free(modes);
			free(channel_bufs);
			free(inner_buf_float);
			free(slope[0]);
			free(slope[1]);
			free(tmp_buf);

			codebooks = 0;
			floors = 0;
			residues = 0;
			mappings = 0;
			modes = 0;
			channel_bufs = 0;
			inner_buf_float = 0;
			slope[0] = 0;
			slope[1] = 0;
			tmp_buf  = 0;
		}
	public:
		Vorbis(const Path & _path)
		{
			init_vars();
			try
			{
				this->init(_path);
			}
			catch(const char * _exp)
			{
				free_vars();
				throw(_exp);
			}
		}

		~Vorbis()
		{
			free_vars();
		}

		inline void require_tmp_buf_size(uint32 _size)
		{
			if(_size > tmp_buf_size)
			{
				tmp_buf_size = _size;
				tmp_buf  = (float*) realloc(tmp_buf, sizeof(float) * tmp_buf_size);
			}
		}

		inline static void dot_product(float * _a, const float * _b, uint32 _len)
		{
			while(_len--)
			{
				*_a++ *= *_b++;
			}
		}

		void prepare_in()
		{
			E_ASSERT(!end_of_packet);

			uint32 byte_room = sizeof(bit_buf) - ((bit_num + 0x07) >> 3);
			E_ASSERT(byte_room > 0);

			uint64 tmp = 0;
			int by_read = ogg.read_some((uint8*) &tmp, byte_room);

			bit_buf|= tmp << bit_num;
			bit_num+= (by_read << 3);
		}

		static int low_neighbor(const int * _v, int _x)
		{
			E_ASSERT(_x > 0);
			int ret = -1;
			int m = 0;
			int limit = _v[_x];
			for(int i=0; i<_x; i++)
			{
				if(_v[i] >= m && _v[i] < limit)
				{
					ret = i;
					m = _v[i];
				}
			}
			return ret;
		}

		static int high_neighbor(const int * _v, int _x)
		{
			E_ASSERT(_x > 0);
			int ret= -1;
			int m = 100000;
			int limit = _v[_x];
			for(int i=0; i<_x; i++)
			{
				if(_v[i] <= m && _v[i] > limit)
				{
					ret = i;
					m = _v[i];
				}
			}
			return ret;
		}

		static int render_point(int _x0, int _y0, int _x1, int _y1, int _x)
		{
			int dy = _y1 - _y0;
			int adx = _x1 - _x0;
			int y;
			if(dy < 0)
			{
				//int ady = -dy;
				int err = -dy * (_x - _x0);
				int off = err / adx;
				y = _y0 - off;
			}
			else
			{
				//int ady = dy;
				int err = dy * (_x - _x0);
				int off = err / adx;
				y = _y0 + off;
			}
			return y;
		}

//		static int round_div(int _a, int _b)
//		{
//			return (int)round((float)_a/ (float)_b);
//		}

		static void render_line(int _x0, int _y0, int _x1, int _y1, int * _v)
		{
			int dy   = _y1 - _y0;
			int adx  = _x1 - _x0;
			int ady  = dy < 0 ? -dy : dy;
			int base = dy / adx;
			int x    = _x0;
			int y    = _y0;
			int err  = 0;

			int sy = dy < 0 ? base - 1 : base + 1;
			int abase = base < 0 ? -base : base;
			ady = ady - abase * adx;
			_v[x] = y;

			for(x = _x0 + 1; x <= _x1-1; x++)
			{
				err = err+ady;
				if(err >= adx)
				{
					err = err - adx;
					y = y + sy;
				}
				else
				{
					y = y + base;
				}
				_v[x] = y;
			}
		}

		uint32 read_bits(uint32 _n)
		{
			E_ASSERT(_n <= 32);
			if(!_n)
			{
				message(L"[nb] (WW) Vorbis: read 0 bit.");
				return 0;
			}

			if(end_of_packet)
			{
				message(L"[nb] (WW) Vorbis: read exeed end_of_packet.");
				return 0;
			}

			if(bit_num < _n)
			{
				prepare_in();
			}

			return (uint32)(low_bits_mask_64[_n] & bit_buf);
		}

		void drop_bits(uint32 _bit_count)
		{
			if(_bit_count > bit_num)
			{
				if(!end_of_packet)
				{
#ifdef NB_CFG_VERBOSE
					E_TRACE_LINE("[nb] end of packet");
#endif
					end_of_packet = true;
				}
				bit_buf = 0;
				bit_num = 0;
			}
			else
			{
				bit_buf>>=_bit_count;
				bit_num-= _bit_count;
			}
		}

		uint32 read_bits_and_drop(int _bit_count)
		{
			E_ASSERT(_bit_count <= 32);
			uint32 ret = read_bits(_bit_count);
			drop_bits(_bit_count);
			return ret;
		}

		// can read more than 32bit, not aligned
		void read_bytes_and_drop(void * _buf0, int _byte_count)
		{
			uint8 * _buf = (uint8*) _buf0;
			E_ASSERT(_buf != 0);
			E_ASSERT(_byte_count > 0);
			int dword_count = _byte_count >> 2;
			int remain_byte_count = _byte_count & 0x00000003;
			while(dword_count--)
			{
				uint32 n = read_bits_and_drop(32);
				*((uint32*)_buf) = n;
				_buf+=4;
			}
			if(remain_byte_count)
			{
				uint32 n= read_bits_and_drop(remain_byte_count * 8);

				//*((uint32*)_buf) = n;
				uint8 * p = (uint8*)&n;
				for(int i=0; i<remain_byte_count; i++)
				{
					*_buf++ = *p++;
				}
			}
		}

		bool next_packet()
		{
			if(end_of_stream)
			{
				return false;
			}
			bit_buf = 0;
			bit_num = 0;
			if(ogg.skip_to_next_packet())
			{
				end_of_packet = false;
				return true;
			}
			else
			{
				end_of_packet = true;
				end_of_stream = true;
				return false;
			}
		}

		void read_string(stringa & _s)
		{
			uint32 length;
			read_bytes_and_drop((uint8*)&length, 4);
			if(length == 0)
			{
				_s.clear();
			}
			else
			{
				_s.reserve(length+1);
				read_bytes_and_drop((uint8*)_s.c_str(), length);
				_s.c_str()[length] = 0;
			}
		}

		uint64 get_frame_count() const
		{
			return total_pcm_avaible_known ? total_pcm_avaible : 0;
		}

		void read_identification_header()
		{
#ifdef NB_CFG_VERBOSE
			E_TRACE_LINE("[nb]  # packet: 0");
#endif

			uint8 buf[8];
			read_bytes_and_drop(buf, 7);
			if(buf[0] != 1 || memcmp(buf+1, "vorbis", 6) != 0)
			{
				throw(NB_SRC_LOC "corrupted identification header");
			}
			read_bytes_and_drop(&vorbis_version, 4);
			if(vorbis_version != 0)
			{
				throw(NB_SRC_LOC "unsupported vorbis version");
			}
			read_bytes_and_drop(&audio_channels, 1);
			pcm_block_align = audio_channels * sizeof(float);
			read_bytes_and_drop(&audio_frame_rate, 4);
			read_bytes_and_drop(&bitrate_maximum, 4);
			read_bytes_and_drop(&bitrate_nominal, 4);
			read_bytes_and_drop(&bitrate_minimum, 4);
			uint32 tmp = read_bits_and_drop(4);
			block_size[0] = 0x00000001 << tmp;
			//frame_type[0] = tmp - 5;
			tmp = read_bits_and_drop(4);
			block_size[1] = 0x00000001 << tmp;
			//frame_type[1] = tmp - 5;
			//E_ASSERT(frame_width[frame_type[0]] == block_size[0]);
			//E_ASSERT(frame_width[frame_type[1]] == block_size[1]);
			if(block_size[0] > block_size[1] || block_size[0] < 64 || block_size[1] > 8192)
			{
				throwf(NB_SRC_LOC "unsupported blocksize pair %d,%d", block_size[0], block_size[1]);
			}

			bool framing_flag = read_bits_and_drop(1) ? true : false;

			//read_bits_and_drop(1);

			mdct[0].init(block_size[0]);
			mdct[1].init(block_size[1]);
		}

		void read_comments_header()
		{
			if(!next_packet())
			{
				throw(NB_SRC_LOC "missing comments header");
			}

			uint8 buf[32];
			read_bytes_and_drop(buf, 7);
			if(buf[0] != 3 || memcmp(buf+1, "vorbis", 6) != 0)
			{
				throw(NB_SRC_LOC "corrupted comments header");
			}

			stringa s;
			read_string(s);

			uint32 list_count;
			read_bytes_and_drop((uint8*)&list_count, 4);

			for(uint32 j=0; j<list_count; j++)
			{
				read_string(s);
				if(s.length() > 6
					&& s[5] == '='
					&& s.substr(0, 5).icompare("TITLE") == 0)
				{
					title = string(s.substr(6, -1), CHARSET_UTF8);
					break;
				}
			}

			// E_TRACE_LINE(L"[nb] Vorbis: title = \"" + title + L"\"");

		}

		inline static float float32_unpack(uint32 _x)
		{
			int mantissa = _x & 0x1fffff;
			uint32 sign = _x & 0x80000000;
			int exponent = ( _x & 0x7fe00000) >> 21;
			if(sign)
			{
				mantissa=-mantissa;
			}
			return mantissa * pow(2.0f, exponent - 788);
		}

		inline static uint32 ipower(uint32 _b, uint32 _e)
		{
			int ret = 1;
			for(uint32 i=0; i<_e; i++)
			{
				ret*= _b;
			}
			return ret;
		}
		inline static uint32 lookup1_values(int _codebook_entries, int _codebook_dimensions)
		{
			if(_codebook_dimensions == 0)
			{
				return 0;
			}
			// max ( ret^_codebook_dimensions <= _codebook_entries)
			uint32 ret =  (uint32)pow((float)_codebook_entries, 1.0f/_codebook_dimensions);
			if(ret == 0)
			{
				ret = 1;
			}
			while(true)
			{
				if(ipower(ret, _codebook_dimensions) > _codebook_entries)
				{
					ret--;
					if(ret == 0)
					{
						break;
					}
				}
				else if(ipower(ret+1, _codebook_dimensions) <= _codebook_entries)
				{
					ret++;
				}
				else
				{
					break;
				}
			}
			return ret;
		}

		void read_setup_header()
		{
			if(!next_packet())
			{
				throw(NB_SRC_LOC "missing setup header");
			}

			uint8 buf[32];
			// setup header packet
			read_bytes_and_drop(buf, 7);
			if(buf[0] != 5 || memcmp(buf+1, "vorbis", 6) != 0)
			{
				throw(NB_SRC_LOC "corrupted setup header");
			}

			// codebooks
			codebook_count = read_bits_and_drop(8) + 1;
			delete[] codebooks;
			codebooks = enew CODEBOOK[codebook_count];

			for(int i=0; i<codebook_count; i++)
			{
				codebooks[i].id = i;
				codebooks[i].read_config(this);
			}

			//  time domain transforms (unused)
			uint32 vorbis_tdt_count = read_bits_and_drop(6) + 1;
			while(vorbis_tdt_count--)
			{
				if(read_bits_and_drop(16) != 0)
				{
					throw(NB_SRC_LOC "unsupported time domain transforms");
				}
			}

			// floors
			E_ASSERT(floors == 0);
			floor_count = read_bits_and_drop(6) + 1;
			if(floor_count)
			{
				floors = (FLOOR*)malloc(sizeof(FLOOR) * floor_count);
				memset(floors, 0, sizeof(FLOOR) * floor_count);
				for(int i=0; i<floor_count; i++)
				{
					floors[i].read_config(this);
				}
			}

			// residues
			vorbis_residue_count = read_bits_and_drop(6) + 1;
			E_ASSERT(residues == 0);
			residues = (RESIDUE*) malloc(sizeof(RESIDUE) * vorbis_residue_count);
			memset(residues, 0, sizeof(RESIDUE) * vorbis_residue_count);
			for(int i=0; i<vorbis_residue_count; i++)
			{
				residues[i].read_config(this);
			}

			// mapping
			vorbis_mapping_count = read_bits_and_drop(6) + 1;
			E_ASSERT(mappings == 0);
			mappings = (MAPPING*) malloc(sizeof(MAPPING) * vorbis_mapping_count);
			memset(mappings, 0, sizeof(MAPPING) * vorbis_mapping_count);
			for(int i=0; i<vorbis_mapping_count; i++)
			{
				mappings[i].read_config(this);
			}

			// modes
			vorbis_mode_count = read_bits_and_drop(6) + 1;
			E_ASSERT(modes == 0);
			modes = (MODE*) malloc(sizeof(MODE) * vorbis_mode_count);
			memset(modes, 0, sizeof(MODE) * vorbis_mode_count);
			for(int i=0; i<vorbis_mode_count; i++)
			{
				modes[i].read_config(this);
			}

			uint32 framing_flag = read_bits_and_drop(1);
			if(framing_flag == 0)
			{
				throw(NB_SRC_LOC "corrupted header three");
			}

		}

		void read_vorbis_headers()
		{
			header_decode_complete = false;
			read_identification_header();
			read_comments_header();
			read_setup_header();
			header_decode_complete = true;
			// E_TRACE_LINE(L"[nb] Vorbis: header decode complete.");
		}

		void on_switch_page()
		{
			if(ogg.page_index > 0 && header_decode_complete && ogg.flag_eos())
			{
				total_pcm_avaible_known = true;
				total_pcm_avaible = ogg.granule;
			}
		}

		static void _on_switch_page(Ogg * _ogg, void * _this)
		{
			((Vorbis*)_this)->on_switch_page();
		}

		void init(const Path & _path)
		{
			ogg.close();

			free_vars();
			init_vars();
#ifdef E_CFG_VERBOSE
			E_TRACE_LINE("[nb] Vorbis::Init() : path = " + _path.GetString());
#endif
			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				throw(NB_SRC_LOC "failed open file");
			}

			if(file->RandomAccess())
			{
				if(!ogg.init(file, 0, 0))
				{
					throw(NB_SRC_LOC "invalid ogg stream");
				}
				while(ogg.skip_to_next_page())
				{
					// do nothing;
				}
				E_ASSERT(ogg.flag_eos());
				if(ogg.flag_eos())
				{
					total_pcm_avaible_known = true;
					total_pcm_avaible = ogg.granule;
				}
				file->Seek(0);
			}

			if(!ogg.init(file, &Vorbis::_on_switch_page, this))
			{
				throw(NB_SRC_LOC "invalid ogg stream");
			}

			title = _path.GetBaseName(false);

			bit_buf = 0;
			bit_num = 0;

			read_vorbis_headers();

			E_ASSERT(channel_bufs == 0);
			channel_bufs = (CHANNEL_BUF*) malloc(sizeof(CHANNEL_BUF) * audio_channels);
			memset(channel_bufs, 0, sizeof(CHANNEL_BUF) * audio_channels);

			pre_window_flag = -1;

			if(inner_buf_float_size==0)
			{
				inner_buf_float_size = 64;
				inner_buf_float = (float*) malloc(inner_buf_float_size * sizeof(float));
			}

			init_overlap();

			require_tmp_buf_size(block_size[1]);
		}

		bool write_pcm(float _a)
		{
			if(total_pcm_avaible_known && pcm_write_total >= total_pcm_avaible)
			{
				this->end_of_packet = true;
				this->end_of_stream = true;
				return false;
			}

			if(++pcm_write_partial >= audio_channels)
			{
				pcm_write_total++;
				pcm_write_partial=0;
			}

			//_a*= 0.98f;
			if(out_buf_float_len < out_buf_float_size)
			{
				out_buf_float[out_buf_float_len++]= _a;
			}
			else
			{
				E_ASSERT(inner_buf_float_offset == 0);
				if(inner_buf_float_len >= inner_buf_float_size)
				{
					inner_buf_float_size+= (inner_buf_float_size >> 1) + 1;
					inner_buf_float = (float*) realloc(inner_buf_float, inner_buf_float_size * sizeof(float));
				}
				inner_buf_float[inner_buf_float_len++] = _a;
			}
			return true;
		}

		static void inverse_coupling(float * mag, float * ang, int half_n)
		{
			float * end = mag + half_n;
			while(mag < end)
			{
				float m = *mag;
				float a = *ang;
				if(m > 0)
				{
					if(a > 0)
					{
						//*mag = m;
						*ang = m - a;
					}
					else
					{
						*ang = m;
						*mag = m + a;
					}
				}
				else
				{
					if(a > 0)
					{
						//*mag = m;
						*ang = m + a;
					}
					else
					{
						*ang = m;
						*mag = m - a;
					}
				}

				mag++;
				ang++;
			}
		}

		void flush_remain_pcm()
		{
			E_ASSERT(inner_buf_float_offset < inner_buf_float_len);
			int remain_n = inner_buf_float_len - inner_buf_float_offset;
			int n = out_buf_float_size < remain_n ? out_buf_float_size : remain_n;
			memcpy(out_buf_float, inner_buf_float + inner_buf_float_offset, n * sizeof(float));
			inner_buf_float_offset+= n;
			out_buf_float_len+= n;
			if(inner_buf_float_offset == inner_buf_float_len)
			{
				inner_buf_float_len = 0;
				inner_buf_float_offset = 0;
			}
		}

//		int get_channel_count() const
//		{
//			return audio_channels;
//		}

		int Read(float * _buf, int _read_frames, int _channels)
		{
			if(_channels != this->audio_channels)
			{
				return NB_READ_ERR;
			}

			if(end_of_stream && inner_buf_float_len == 0)
			{
				return NB_READ_END;
			}

			out_buf_float      = _buf;
			out_buf_float_size = _read_frames * _channels;
			out_buf_float_len  = 0;

			// flush remain pcm
			try
			{
				if(inner_buf_float_len)
				{
					flush_remain_pcm();
				}

				while(out_buf_float_len < out_buf_float_size && next_packet())
				{
					// 4.3.1 packet type, mode and window decode
					// 1
					uint32 packet_type = read_bits_and_drop(1);

					if(packet_type != 0)
					{
						E_ASSERT1(0, L"[nb]   skip non-audio packet. type = " + string(packet_type));
						continue;
					}

					// 2
					uint32 bits = ilog(vorbis_mode_count-1);
					uint32 mode_number = read_bits_and_drop(bits);
					// 3
					MODE & mode = modes[mode_number];
					int cur_window_flag  = mode.vorbis_mode_blockflag;

					uint32 n = block_size[mode.vorbis_mode_blockflag];
					uint32 half_n = n >> 1;
					if(mode.vorbis_mode_blockflag)
					{
						read_bits_and_drop(1);
						read_bits_and_drop(1);
					}

#ifdef NB_CFG_VERBOSE
					E_TRACE_LINE(L"[nb]   decode audio frame: id=" + string(ogg.packet_index-3)
						+ L", n=" + string(n)
						+ L", pcm=" + string((int)pcm_write_total/audio_channels)
						);
#endif

					// 4.3.2 floor curve decode
					MAPPING & mapping = mappings[mode.vorbis_mode_mapping];
					for(int ch=0; ch<audio_channels; ch++)
					{
						CHANNEL_BUF & buf = channel_bufs[ch];
						uint8 submap_number = mapping.vorbis_mapping_mux[ch];
						uint8 floor_number  = mapping.vorbis_mapping_submap_floor[submap_number];
						floors[floor_number].decode(this, buf, half_n);
					}

					for(int i=0; i<mapping.vorbis_mapping_coupling_steps; i++)
					{
						if(channel_bufs[mapping.vorbis_mapping_magnitude[i]].floor_unused
							|| channel_bufs[mapping.vorbis_mapping_angle[i]].floor_unused)
						{
							//E_ASSERT(channel_bufs[mapping.vorbis_mapping_magnitude[i]].floor_unused
							//	&& channel_bufs[mapping.vorbis_mapping_angle[i]].floor_unused);
							channel_bufs[mapping.vorbis_mapping_magnitude[i]].floor_unused = true;
							channel_bufs[mapping.vorbis_mapping_angle[i]].floor_unused = true;
						}
					}

					// 4.3.4 residue decode
					CHANNEL_BUF * buf_ch_order[256];
					for(int i=0; i<mapping.mapping_submaps; i++)
					{
						// 1
						int ch = 0;
						// 2
						for(int j=0; j<this->audio_channels; j++)
						{
							// a)
							uint8 submap_number = mapping.vorbis_mapping_mux[i];
							if(submap_number == i)
							{
								// i
								buf_ch_order[ch] = &channel_bufs[j];
								// ii
								ch++;
							}
						}

						// 3
						uint8 residue_number = mapping.vorbis_mapping_submap_residue[i];
						// 4
						RESIDUE & residue = residues[residue_number];
						// 5, 6, 7
						if(residue.type == 2)
						{
							residue.decode_format_2(this, buf_ch_order, ch, half_n);
						}
						else
						{
							residue.decode_format_01(this, buf_ch_order, ch, half_n);
						}
					}

					// 4.3.5 inverse coupling
					for(int i=mapping.vorbis_mapping_coupling_steps-1; i>=0 ; i--)
					{
						uint32 magnitude_vector_index = mapping.vorbis_mapping_magnitude[i];
						uint32 angle_vector_index = mapping.vorbis_mapping_angle[i];
						float * mag = channel_bufs[magnitude_vector_index].residue;
						float * ang = channel_bufs[angle_vector_index].residue;
						inverse_coupling(mag, ang, half_n);
					}

					OVERLAP * ov = pre_window_flag >= 0 ? &overlap[pre_window_flag][cur_window_flag] : 0;
					int this_frame_pcm_count = ov ? ov->pcm_count : 0;
					for(int ch=0; ch<audio_channels; ch++)
					{
						// 4.3.6 dot product
						CHANNEL_BUF & buf = channel_bufs[ch];
						dot_product(buf.residue, buf.floor, half_n);
						mdct[cur_window_flag].inverse(buf.residue, tmp_buf);
						if(ov)
						{
							ov->add(buf.pcm, buf.prev_half_audio, tmp_buf);
						}
						memcpy(buf.prev_half_audio, tmp_buf + half_n, half_n * sizeof(float));
					}

					if(ov)
					{
						for(int i=0; i<this_frame_pcm_count; i++)
						{
							for(int ch=0; ch<audio_channels; ch++)
							{
								if(!write_pcm(channel_bufs[ch].pcm[i]))
								{
									goto _BREAK_OUT_PCM;
								}
							}
						}
					}
_BREAK_OUT_PCM:
					pre_window_flag = cur_window_flag;
				}
			}
			catch(const char * _exp)
			{
				E_ASSERT1(0, _exp);
				return NB_READ_ERR;
			}

			E_ASSERT(out_buf_float_len % _channels == 0);
			return out_buf_float_len / _channels;
		}

		bool GetInfo(AInfo & _info) override
		{
			_info.channel_count = this->audio_channels;
			_info.frame_count   = (int)this->get_frame_count();
			_info.title         = this->title;
			return true;
		}
	};


	SoundFileReader * create_oga_reader(const Path & _path)
	{
		return enew Vorbis(_path);
	}




}

//#pragma pack(pop)

