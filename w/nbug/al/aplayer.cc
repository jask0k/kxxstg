//#include <float.h>
#include <string.h>

#ifdef NB_CFG_OPENAL
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#ifdef NB_CFG_DSOUND
#	include <dsound.h>
#endif

#ifdef  NB_CFG_ALSA
#	include <alsa/asoundlib.h>
#endif

#ifdef NB_CFG_PULSE
#	include <pulse/simple.h>
#endif

#include <nbug/al/aplayer.h>
#include <nbug/al/afile.h>
#include <nbug/core/time.h>
#include <nbug/core/debug.h>
#include <nbug/core/env.h>
#include <nbug/core/thread.h>
#include <nbug/core/dll_load.h>
#include <nbug/tl/map.h>
#include <nbug/al/mixing_frac.h>
#include <nbug/ex/async_delete.h>

namespace e
{

	static const int FRAME_RATE = 44100;

#	define MSEC_TO_FRAMES(ms) ((ms) * (FRAME_RATE/100) / 10)
#	define FRAMES_TO_MSEC(ms) ((ms) * 10 / (FRAME_RATE/100))
	struct AudioBackend
	{
		// make these configurable
		static const int SAMPLE_BIT_WIDTH     = 16;
		static const int CHANNEL_COUNT        = 2;
		static const int FRAME_SIZE_BYTE      = CHANNEL_COUNT * (SAMPLE_BIT_WIDTH>>3);
		static const int BLOCK_COUNT       	  = 5;
		int block_size_msec;
		int block_size_frame;
		int block_size_sample;
		int block_size_byte;
		int buf_size_byte;
	//	static const int FRAGMENT_SIZE_FRAME  = MSEC_TO_FRAMES(FRAGMENT_SIZE_MSEC);
	//	static const int FRAGMENT_SIZE_SAMPLE = FRAGMENT_SIZE_FRAME * CHANNEL_COUNT;
	//	static const int block_size_byte   = FRAGMENT_SIZE_FRAME * FRAME_SIZE_BYTE;
	//	static const int buf_size_byte     = block_size_byte  * BLOCK_COUNT;
		void PreInit(uint _bufSizeMsec)
		{
			block_size_msec = _bufSizeMsec / BLOCK_COUNT;
			if(block_size_msec < 1)
			{
				block_size_msec = 1;
			}
			block_size_frame =  int( float(block_size_msec) * float(FRAME_RATE) * 0.001f + 0.5f );
			if(block_size_frame < 1)
			{
				block_size_frame = 1;
			}
			block_size_msec   = int( float(block_size_frame) * 1000.0f / float(FRAME_RATE) + 0.5f );
			if(block_size_msec < 1)
			{
				block_size_msec = 1;
			}
			block_size_sample = block_size_frame * CHANNEL_COUNT;
			block_size_byte   = block_size_frame * FRAME_SIZE_BYTE;
			buf_size_byte  = block_size_byte * BLOCK_COUNT;
		}
		virtual void Init() = 0;
		virtual ~AudioBackend(){};
		virtual bool Submit(void * _buf) = 0;
		const char * name;
	};


#ifdef NB_CFG_OPENAL
	struct OpenALBackend : public AudioBackend
	{
		static const char * GetALErrorString(ALenum err)
		{
			switch(err)
			{
			case AL_NO_ERROR: return "AL_NO_ERROR";
			case AL_INVALID_NAME: return "AL_INVALID_NAME";
			case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
			case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
			case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
			case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
			default: E_ASSERT(0); return "AL_UNKOWN_ERROR";
			};
		}

		void * module;

		LPALBUFFERDATA alBufferData;
		LPALCCLOSEDEVICE alcCloseDevice;
		LPALCCREATECONTEXT alcCreateContext;
		LPALCDESTROYCONTEXT alcDestroyContext;
		LPALCGETERROR alcGetError;
		LPALCGETSTRING alcGetString;
		LPALCMAKECONTEXTCURRENT alcMakeContextCurrent;
		LPALCOPENDEVICE alcOpenDevice;
		LPALDELETEBUFFERS alDeleteBuffers;
		LPALDELETESOURCES alDeleteSources;
		LPALGENBUFFERS alGenBuffers;
		LPALGENSOURCES alGenSources;
		LPALGETSOURCEI alGetSourcei;
		LPALSOURCEI alSourcei;
		LPALSOURCEPLAY alSourcePlay;
		LPALSOURCEQUEUEBUFFERS alSourceQueueBuffers;
		LPALSOURCEUNQUEUEBUFFERS alSourceUnqueueBuffers;

		const ALchar* deviceName;
		ALCdevice*    device;
		ALCcontext*   context;
		ALuint 		  source;
		ALuint        buffers[BLOCK_COUNT];
		int 		  head;
		int           tail;
		int           queued_count;

		static void Next(int & _n)
		{
			if(++_n ==BLOCK_COUNT)
			{
				_n = 0;
			}
		}

		OpenALBackend()
		{
			module = 0;
			device  = 0;
			context = 0;
			source  = 0;
			buffers[0] = 0;
			buffers[1] = 0;
			buffers[2] = 0;
			head = 0;
			tail = 0;
			queued_count = 0;
		}

		void Init() override
		{

	#	ifdef _WIN32
			module = hex_dll_open("OpenAL32.dll");
	#	else
			module = hex_dll_open("libopenal.so");
	#	endif

			if(module == 0)
			{
				throw(NB_SRC_LOC "Failed to load OpenAL DLL");
			}

			if(0 == (alBufferData = (LPALBUFFERDATA) hex_dll_get_symbol(module, "alBufferData"))
				|| 0 == (alcCloseDevice = (LPALCCLOSEDEVICE) hex_dll_get_symbol(module, "alcCloseDevice"))
				|| 0 == (alcCreateContext = (LPALCCREATECONTEXT) hex_dll_get_symbol(module, "alcCreateContext"))
				|| 0 == (alcDestroyContext = (LPALCDESTROYCONTEXT) hex_dll_get_symbol(module, "alcDestroyContext"))
				|| 0 == (alcGetError = (LPALCGETERROR) hex_dll_get_symbol(module, "alcGetError"))
				|| 0 == (alcGetString = (LPALCGETSTRING) hex_dll_get_symbol(module, "alcGetString"))
				|| 0 == (alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT) hex_dll_get_symbol(module, "alcMakeContextCurrent"))
				|| 0 == (alcOpenDevice = (LPALCOPENDEVICE) hex_dll_get_symbol(module, "alcOpenDevice"))
				|| 0 == (alDeleteBuffers = (LPALDELETEBUFFERS) hex_dll_get_symbol(module, "alDeleteBuffers"))
				|| 0 == (alDeleteSources = (LPALDELETESOURCES) hex_dll_get_symbol(module, "alDeleteSources"))
				|| 0 == (alGenBuffers = (LPALGENBUFFERS) hex_dll_get_symbol(module, "alGenBuffers"))
				|| 0 == (alGenSources = (LPALGENSOURCES) hex_dll_get_symbol(module, "alGenSources"))
				|| 0 == (alGetSourcei = (LPALGETSOURCEI) hex_dll_get_symbol(module, "alGetSourcei"))
				|| 0 == (alSourcei = (LPALSOURCEI) hex_dll_get_symbol(module, "alSourcei"))
				|| 0 == (alSourcePlay = (LPALSOURCEPLAY) hex_dll_get_symbol(module, "alSourcePlay"))
				|| 0 == (alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS) hex_dll_get_symbol(module, "alSourceQueueBuffers"))
				|| 0 == (alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS) hex_dll_get_symbol(module, "alSourceUnqueueBuffers"))
				)
			{
				throw(NB_SRC_LOC "Failed to load OpenAL functions");
			}

			int err;
			deviceName = alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
			device = alcOpenDevice(deviceName ? deviceName : "");
			if(device == 0)
			{
				throw(NB_SRC_LOC "alcOpenDevice() failed.");
			}

			alcGetError(device);
			context = alcCreateContext(device, NULL);
			if(context == 0)
			{
				err = alcGetError(device);
				throwf(NB_SRC_LOC "alcCreateContext() err=%d,%s", err, GetALErrorString(err));
			}

			alcMakeContextCurrent(context);
			err =  alcGetError(device);
			if(err != ALC_NO_ERROR)
			{
				throwf(NB_SRC_LOC "alcMakeContextCurrent() err=%d,%s", err, GetALErrorString(err));
			}

			alGenSources(1, &source);
			if(source==0)
			{
				err = alcGetError(device);
				throwf(NB_SRC_LOC "alGenSources() err=%d,%s", err, GetALErrorString(err));
			}
			alSourcei(source, AL_LOOPING, 0);

			alcGetError(device);
			alGenBuffers(BLOCK_COUNT, buffers);
			err =  alcGetError(device);
			if(err != ALC_NO_ERROR)
			{
				throwf(NB_SRC_LOC "alGenBuffers() err=%d,%s", err, GetALErrorString(err));
			}
			queued_count = 0;
		}

		~OpenALBackend()
		{
			if(module)
			{
				if(source)
				{
					alDeleteSources(1, &source);
				}

				if(alDeleteBuffers)
				{
					alDeleteBuffers(BLOCK_COUNT, buffers);
				}


				if(context)
				{
					alcDestroyContext(context);
				}

				if(device)
				{
					alcCloseDevice(device);
				}

				hex_dll_close(module);
			}
		}

		bool Submit(void * _buf) override
		{
			E_ASSERT(_buf != 0);

			int processed = 0;
			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

			while(processed--)
			{
				E_ASSERT(queued_count > 0);
				alSourceUnqueueBuffers(source, 1, &buffers[tail]);
				queued_count--;
				Next(tail);
			}

			if(queued_count >= BLOCK_COUNT)
			{
				return false;
			}

			ALuint bufid = buffers[head];
			Next(head);
			alBufferData(bufid, AL_FORMAT_STEREO16, _buf, block_size_byte, 44100);
			queued_count++;
			alSourceQueueBuffers(source, 1, &bufid);
			int state = 0;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			if(state!= AL_PLAYING)
			{
				alSourcePlay(source);
			}
			return true; // TODO: handle error
		}
	};

	static  AudioBackend* CreateOpenALMaster()
	{
		return enew OpenALBackend();
	};

#endif

#ifdef NB_CFG_DSOUND
	struct DSoundBackend : public AudioBackend
	{
		double lastSubmitTime;
		double minTimeSpan;
		DWORD myWriteCursor;
		LPDIRECTSOUND dsound8;
		LPDIRECTSOUNDBUFFER primary;
		LPDIRECTSOUNDBUFFER buffer;
		bool playing;
//		bool hasNotify;
//		HANDLE hEventRewind;
//		HANDLE hEventHalf;
//		DWORD lastPlayPos;

		DSoundBackend()
		{
			dsound8 = 0;
			primary = 0;
			buffer  = 0;
			myWriteCursor = 0;
			playing = false;
		//	hasNotify = false;
			//hEventRewind = 0;
			//hEventHalf   = 0;
//			lastPlayPos = 0;
			lastSubmitTime = 0;
			minTimeSpan = 0;
		}
		~DSoundBackend()
		{
			if(buffer)
			{
				buffer->Release();
			}
			if(primary)
			{
				primary->Release();
			}
			if(dsound8)
			{
				dsound8->Release();
			}
//
//			if(hEventRewind)
//			{
//				CloseHandle(hEventRewind);
//			}
		}

		void Init() override
		{

			WAVEFORMATEX format;
			format.wFormatTag = 1;
			format.nChannels = 2;
			format.nSamplesPerSec = 44100; // actually frame rate
			format.wBitsPerSample = 16;
			format.nBlockAlign = (format.wBitsPerSample/8) * format.nChannels;
			format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
			format.cbSize = 0;

			// dll
			HMODULE hDSoundDll = LoadLibrary(L"dsound.dll");
			if(hDSoundDll)
			{
				// function
				typedef HRESULT (WINAPI *FUNC_DirectSoundCreate8)(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
				FUNC_DirectSoundCreate8 pDirectSoundCreate8;
				pDirectSoundCreate8 = (FUNC_DirectSoundCreate8) GetProcAddress(hDSoundDll, "DirectSoundCreate8");
				if(pDirectSoundCreate8)
				{
					// dsound
					HRESULT hr = pDirectSoundCreate8(NULL, &dsound8, NULL);
					if(SUCCEEDED(hr))
					{
						// init dsound
						// DSSCL_NORMAL is 22k 8bit sound, we need 44k 16bit, so we must set cooperative level to DSSCL_PRIORITY, or higher
						hr = dsound8->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
						if(SUCCEEDED(hr))
						{
							// for cooperative level DSSCL_PRIORITY, we must create a PRIMARY buffer.
							DSBUFFERDESC dsb;
							memset(&dsb, 0, sizeof(dsb));
							dsb.dwSize = sizeof(dsb);
							dsb.dwFlags = DSBCAPS_PRIMARYBUFFER;
							hr = dsound8->CreateSoundBuffer(&dsb, &primary, NULL);
							if(SUCCEEDED(hr))
							{
								hr = primary->SetFormat(&format);
								E_ASSERT(SUCCEEDED(hr));
							}
						}

						if(primary == 0)
						{
							dsound8->Release();
							dsound8 = 0;
						}
					}
				}
				FreeLibrary(hDSoundDll);
			}
			if(primary==0)
			{
				throw(NB_SRC_LOC "Failed to init DirectSound.");
			}

			DSBUFFERDESC dsb;
			memset(&dsb, 0, sizeof(dsb));
			dsb.dwSize = sizeof(dsb);
			dsb.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
			dsb.lpwfxFormat = &format;
			dsb.dwBufferBytes = buf_size_byte;


			HRESULT hr = dsound8->CreateSoundBuffer(&dsb, &buffer, NULL);
			if(FAILED(hr))
			{
				throw(NB_SRC_LOC "Failed to create DirectSound buffer.");
			}

			void * buf0;
			DWORD len0;
			void * buf1;
			DWORD len1;
			 hr = buffer->Lock(0, DSBLOCK_ENTIREBUFFER, &buf0, &len0, &buf1, &len1, 0);
			if(hr == DSERR_BUFFERLOST)
			{
				buffer->Restore();
				hr = buffer->Lock(0, DSBLOCK_ENTIREBUFFER, &buf0, &len0, &buf1, &len1, 0);
			}
			if(SUCCEEDED(hr))
			{
				memset(buf0, 0, len0);
				memset(buf1, 0, len1);
				hr = buffer->Unlock(buf0, len0, buf1, len1);
				if(hr == DSERR_BUFFERLOST)
				{
					buffer->Restore();
					hr = buffer->Unlock(buf0, len0, buf1, len1);
				}
			}
			minTimeSpan = block_size_msec * 0.0005;
		//	buffer->SetFormat(&format);

//
//			LPDIRECTSOUNDNOTIFY lpDsNotify;
//			static const _GUID guid = {0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16} };
//			hr = buffer->QueryInterface(guid, (LPVOID *)&lpDsNotify);
//			if(FAILED(hr))
//			{
//				throw(NB_SRC_LOC);
//			}
//
//			hEventRewind = ::CreateEvent(NULL, FALSE, FALSE, NULL);
//			if(hEventRewind == NULL)
//			{
//				throw(NB_SRC_LOC);
//			}
//
//			DSBPOSITIONNOTIFY pos;
//			pos.dwOffset = 0;
//			pos.hEventNotify = hEventRewind;
//			hr = lpDsNotify->SetNotificationPositions(1, &pos);
//			if(FAILED(hr))
//			{
//				throw(NB_SRC_LOC);
//			}
		}

		bool Submit(void * _buf) override
		{
			E_ASSERT(_buf != 0);

			DWORD playCursor, dummy;
			if(FAILED(buffer->GetCurrentPosition(&playCursor, &dummy)))
			{
				return false;
			}

			DWORD sz = playCursor > myWriteCursor ? playCursor - myWriteCursor : playCursor + buf_size_byte -myWriteCursor;

			if(sz < block_size_byte)
			{
				return false;
			}
			double t = Time::GetTicks() - lastSubmitTime;
			if(t < minTimeSpan )
			{
				return false;
			}

			void * buf0;
			DWORD len0;
			void * buf1;
			DWORD len1;
			HRESULT hr = buffer->Lock(myWriteCursor, block_size_byte, &buf0, &len0, &buf1, &len1, 0);
			if(hr == DSERR_BUFFERLOST)
			{
				buffer->Restore();
				playing = false;
				hr = buffer->Lock(myWriteCursor, block_size_byte, &buf0, &len0, &buf1, &len1, 0);
			}

			if(SUCCEEDED(hr))
			{
				E_ASSERT(len0 <= block_size_byte);
				memcpy(buf0, _buf, len0);
				if(len0 < block_size_byte)
				{
					E_ASSERT(block_size_byte - len0 == len1);
					memcpy(buf1, ((uint8*)_buf)+len0, len1);
				}
				hr = buffer->Unlock(buf0, len0, buf1, len1);
//				if(hr == DSERR_BUFFERLOST)
//				{
//					buffer->Restore();
//					hr = buffer->Unlock(buf0, len0, buf1, len1);
//				}
				if(SUCCEEDED(hr))
				{
					lastSubmitTime = Time::GetTicks();
					myWriteCursor+= block_size_byte;
					if(myWriteCursor >= buf_size_byte)
					{
						myWriteCursor-= buf_size_byte;
					}
					if(!playing)
					{
						buffer->Play(0, 0, DSBPLAY_LOOPING);
						E_ASSERT(SUCCEEDED(hr));
						playing = true;
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}


		/*
		bool Submit(void * _buf) override
		{
			E_ASSERT(_buf != 0);

			bool ready = false;

			DWORD writable_begin, writable_end;
			if(SUCCEEDED(buffer->GetCurrentPosition(&writable_end, &writable_begin)))
			{
				//bool underrun = false;
				if(writable_begin < writable_end)
				{
					// |=================b----------e===================|
					DWORD sz = writable_end  + buf_size_byte - writable_begin;
					if(sz >= block_size_byte)
					{
						ready = myWriteCursor < writable_begin || myWriteCursor >=  writable_end
							|| writable_end - myWriteCursor >= block_size_byte;
					}
				}
				else
				{
					// |------------e=====================b--------------|
					DWORD end1 = writable_end  + buf_size_byte;
					DWORD sz = end1 - writable_begin;
					if(sz >= block_size_byte)
					{
						if(myWriteCursor >= writable_end && myWriteCursor < writable_begin)
						{
							ready = true;
						}
						else
						{
							ready = myWriteCursor < writable_end ?
									writable_end - myWriteCursor >= block_size_byte
									: end1 - myWriteCursor >= block_size_byte;
						}
					}
				}
			}

			if(!ready)
			{
				return false;
			}


			void * buf0;
			DWORD len0;
			void * buf1;
			DWORD len1;
			HRESULT hr = buffer->Lock(myWriteCursor, block_size_byte, &buf0, &len0, &buf1, &len1, 0);
			if(hr == DSERR_BUFFERLOST)
			{
				buffer->Restore();
				hr = buffer->Lock(myWriteCursor, block_size_byte, &buf0, &len0, &buf1, &len1, 0);
			}

			if(SUCCEEDED(hr))
			{
				E_ASSERT(len0 <= block_size_byte);
				memcpy(buf0, _buf, len0);
				if(len0 < block_size_byte)
				{
					E_ASSERT(block_size_byte - len0 == len1);
					memcpy(buf1, ((uint8*)_buf)+len0, len1);
				}
				hr = buffer->Unlock(buf0, len0, buf1, len1);

//				if(hr == DSERR_BUFFERLOST)
//				{
//					buffer->Restore();
//					hr = buffer->Unlock(buf0, len0, buf1, len1);
//				}
//
				if(SUCCEEDED(hr))
				{
					myWriteCursor+= block_size_byte;
					if(myWriteCursor >= buf_size_byte)
					{
						myWriteCursor-= buf_size_byte;
					}
					if(!playing)
					{
						buffer->Play(0, 0, DSBPLAY_LOOPING);
						E_ASSERT(SUCCEEDED(hr));
						playing = true;
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}
		*/

	};

	static  AudioBackend* CreateDSoundMaster()
	{
		return enew DSoundBackend();
	};
#endif

#ifdef NB_CFG_ALSA
	struct AlsaBackend : public AudioBackend
	{
		void * module;

		typedef int (*func_snd_pcm_open)(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode);
		typedef int (*func_snd_pcm_close)(snd_pcm_t *pcm);
		typedef int (*func_snd_pcm_hw_params_any)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
		typedef int (*func_snd_pcm_hw_params_set_access)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access);
		typedef int (*func_snd_pcm_hw_params_set_format)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val);
		typedef int (*func_snd_pcm_hw_params_set_rate_near)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
		typedef int (*func_snd_pcm_hw_params_set_channels)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);
		typedef int (*func_snd_pcm_hw_params_set_periods)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir);
		//typedef int (*func_snd_pcm_hw_params_set_buffer_size)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t val);
		typedef int (*func_snd_pcm_hw_params_set_buffer_size_min)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
		typedef int (*func_snd_pcm_hw_params_set_buffer_size_near)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
		typedef int (*func_snd_pcm_hw_params)(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
		typedef snd_pcm_sframes_t (*func_snd_pcm_writei)(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);
		typedef int (*func_snd_pcm_prepare)(snd_pcm_t *pcm);
		typedef int (*func_snd_pcm_drop)(snd_pcm_t *pcm);
		typedef size_t (*func_snd_pcm_hw_params_sizeof)(void);

		func_snd_pcm_open p_snd_pcm_open;
		func_snd_pcm_close p_snd_pcm_close;
		func_snd_pcm_hw_params_any p_snd_pcm_hw_params_any;
		func_snd_pcm_hw_params_set_access p_snd_pcm_hw_params_set_access;
		func_snd_pcm_hw_params_set_format p_snd_pcm_hw_params_set_format;
		func_snd_pcm_hw_params_set_rate_near p_snd_pcm_hw_params_set_rate_near;
		func_snd_pcm_hw_params_set_channels p_snd_pcm_hw_params_set_channels;
		func_snd_pcm_hw_params_set_periods p_snd_pcm_hw_params_set_periods;
		//func_snd_pcm_hw_params_set_buffer_size p_snd_pcm_hw_params_set_buffer_size;
		func_snd_pcm_hw_params_set_buffer_size_min  p_snd_pcm_hw_params_set_buffer_size_min;
		func_snd_pcm_hw_params_set_buffer_size_near p_snd_pcm_hw_params_set_buffer_size_near;
		func_snd_pcm_hw_params p_snd_pcm_hw_params;
		func_snd_pcm_writei p_snd_pcm_writei;
		func_snd_pcm_prepare p_snd_pcm_prepare;
		func_snd_pcm_drop p_snd_pcm_drop;
		func_snd_pcm_hw_params_sizeof snd_pcm_hw_params_sizeof;

		snd_pcm_t * pcm_handle;
	//	const char * pcm_name;


		AlsaBackend()
		{
			name = 0;
			module = 0;
			pcm_handle = 0;
		//	pcm_name = "default";
		}
		void Init() override
		{
			int rc;
			module = hex_dll_open("libasound.so");
			if(!module)
			{
				throw(NB_SRC_LOC "Failed to load libasound.so");
			}

			if(0 == (p_snd_pcm_open = (func_snd_pcm_open) hex_dll_get_symbol(module, "snd_pcm_open"))
				|| 0 == (p_snd_pcm_close = (func_snd_pcm_close) hex_dll_get_symbol(module, "snd_pcm_close"))
				|| 0 == (p_snd_pcm_hw_params_any = (func_snd_pcm_hw_params_any) hex_dll_get_symbol(module, "snd_pcm_hw_params_any"))
				|| 0 == (p_snd_pcm_hw_params_set_access = (func_snd_pcm_hw_params_set_access) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_access"))
				|| 0 == (p_snd_pcm_hw_params_set_format = (func_snd_pcm_hw_params_set_format) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_format"))
				|| 0 == (p_snd_pcm_hw_params_set_rate_near = (func_snd_pcm_hw_params_set_rate_near) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_rate_near"))
				|| 0 == (p_snd_pcm_hw_params_set_channels = (func_snd_pcm_hw_params_set_channels) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_channels"))
				|| 0 == (p_snd_pcm_hw_params_set_periods = (func_snd_pcm_hw_params_set_periods) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_periods"))
				//|| 0 == (p_snd_pcm_hw_params_set_buffer_size = (func_snd_pcm_hw_params_set_buffer_size) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_buffer_size"))
				|| 0 == (p_snd_pcm_hw_params_set_buffer_size_min = (func_snd_pcm_hw_params_set_buffer_size_min) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_buffer_size_min"))
				|| 0 == (p_snd_pcm_hw_params_set_buffer_size_near = (func_snd_pcm_hw_params_set_buffer_size_near) hex_dll_get_symbol(module, "snd_pcm_hw_params_set_buffer_size_near"))
				|| 0 == (p_snd_pcm_hw_params = (func_snd_pcm_hw_params) hex_dll_get_symbol(module, "snd_pcm_hw_params"))
				|| 0 == (p_snd_pcm_writei = (func_snd_pcm_writei) hex_dll_get_symbol(module, "snd_pcm_writei"))
				|| 0 == (p_snd_pcm_prepare = (func_snd_pcm_prepare) hex_dll_get_symbol(module, "snd_pcm_prepare"))
				|| 0 == (p_snd_pcm_drop = (func_snd_pcm_drop) hex_dll_get_symbol(module, "snd_pcm_drop"))
				|| 0 == (snd_pcm_hw_params_sizeof = (func_snd_pcm_hw_params_sizeof) hex_dll_get_symbol(module, "snd_pcm_hw_params_sizeof"))
				)
			{
				throw(NB_SRC_LOC "Failed to load ALSA functions");
			}

			rc = p_snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error opening default PCM device\n");
			}

			snd_pcm_hw_params_t * hwparams = 0;
			snd_pcm_hw_params_alloca(&hwparams);
			rc = p_snd_pcm_hw_params_any(pcm_handle, hwparams);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_any");
			}

			rc = p_snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_set_access");
			}

			rc = p_snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_set_format");
			}

			unsigned int rate = 44100;
			unsigned int exact_rate = rate;
			rc = p_snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_rate, 0);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_set_rate_near");
			}

			if(rate != exact_rate)
			{
				throwf(NB_SRC_LOC "Error unsupported frame rate %dHz", rate);
			}

			rc = p_snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_set_channels");
			}

			rc = p_snd_pcm_hw_params_set_periods(pcm_handle, hwparams, BLOCK_COUNT, 0);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params_set_periods");
			}

//			snd_pcm_uframes_t buf_size = FRAGMENT_SIZE_FRAME * BLOCK_COUNT;
//			snd_pcm_uframes_t exact_buf_size = buf_size;
//			rc = p_snd_pcm_hw_params_set_buffer_size_min(pcm_handle, hwparams, &exact_buf_size);

			snd_pcm_uframes_t buf_size = block_size_frame * BLOCK_COUNT;
			snd_pcm_uframes_t exact_buf_size = buf_size;
			rc = p_snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams, &exact_buf_size);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error p_snd_pcm_hw_params_set_buffer_size_near");
			}

			if(buf_size != exact_buf_size)
			{
				message(L"[nb] (WW) APlayer: Can not set buf_size to " + string((int)buf_size) + L", use " + string((int)exact_buf_size) + L" instead.");
			}

			rc = p_snd_pcm_hw_params(pcm_handle, hwparams);
			if(rc < 0)
			{
				throw(NB_SRC_LOC "Error snd_pcm_hw_params");
			}
		}

		~AlsaBackend()
		{
			if(pcm_handle)
			{
				p_snd_pcm_drop(pcm_handle);
				p_snd_pcm_close(pcm_handle);
			}

			if(module)
			{
				hex_dll_close(module);
			}
		}

		bool Submit(void * _buf) override
		{
			E_ASSERT(_buf != 0);

			int n = p_snd_pcm_writei(pcm_handle, _buf, block_size_frame);
			if(n >= 0)
			{
				return true;
			}
			else if(n == -EPIPE)
			{
				return p_snd_pcm_prepare(pcm_handle) == 0;
			}
			else
			{
				return false;
			}
		}
	};

	static  AudioBackend* CreateAlsaMaster()
	{
		return enew AlsaBackend();
	};

#endif

#ifdef NB_CFG_PULSE
	struct PulseBackend : public AudioBackend
	{
		void * module;
		pa_simple *s;

		typedef pa_simple* (*func_pa_simple_new)(const char *,const char *, pa_stream_direction_t,   const char *,   const char *,   const pa_sample_spec *,   const pa_channel_map *, const pa_buffer_attr *, int *);
		typedef void (*func_pa_simple_free)(pa_simple *);
		typedef int (*func_pa_simple_write)(pa_simple *, const void*, size_t , int *);

		func_pa_simple_new p_pa_simple_new;
		func_pa_simple_free p_pa_simple_free;
		func_pa_simple_write p_pa_simple_write;

		PulseBackend()
		{
			module = 0;
			s = 0;
		}

		void Init() override
		{
			int rc;
#	ifdef _WIN32
			module = hex_dll_open("libpulse-simple.dll");
			if(module == 0)
			{
				module = hex_dll_open("libpulse-simple-0.dll");
			}
#	else
			module = hex_dll_open("libpulse-simple.so");
#	endif

			if(!module)
			{
				throw(NB_SRC_LOC "Failed to load libpulse-simple");
			}

			if(0 == (p_pa_simple_new = (func_pa_simple_new) hex_dll_get_symbol(module, "pa_simple_new"))
				|| 0 == (p_pa_simple_free = (func_pa_simple_free) hex_dll_get_symbol(module, "pa_simple_free"))
				|| 0 == (p_pa_simple_write = (func_pa_simple_write) hex_dll_get_symbol(module, "pa_simple_write"))
				)
			{
				throw(NB_SRC_LOC "Failed to load PulseAudio functions");
			}

			pa_sample_spec ss;
			memset(&ss, 0, sizeof(ss));
			ss.format = PA_SAMPLE_S16NE;
			ss.channels = 2;
			ss.rate = 44100;

			pa_buffer_attr attr;
			memset(&attr, 0, sizeof(attr));
			attr.maxlength = this->buf_size_byte;
			attr.minreq    = 0xffffffffUL;
			attr.prebuf    = 0xffffffffUL;
			attr.tlength   = 0xffffffffUL;

			stringa appName = stringa(Env::GetShortName());
			int err;
			s = p_pa_simple_new(NULL,               // Use the default server.
				appName.c_str(),           // Our application's name.
				PA_STREAM_PLAYBACK,
				NULL,               // Use the default device.
				"Any",            // Description of our stream.
				&ss,                // Our sample format.
				NULL,               // Use default channel map
				&attr,               // Use default buffering attributes.
				&err               // Ignore error code.
				);
			if(!s)
			{
				throwf(NB_SRC_LOC "Failed to int simple pulse playback, err = %d", err);
			}
		}

		~PulseBackend()
		{
			if(module)
			{
				if(s)
				{
					p_pa_simple_free(s);
				}

				hex_dll_close(module);
			}
		}

		bool Submit(void * _buf) override
		{
			E_ASSERT(_buf != 0);
			int err;
			return p_pa_simple_write(s, _buf, block_size_byte, &err) == 0;
		}
	};

	static  AudioBackend* CreatePulseMaster()
	{
		return enew PulseBackend();
	};
#endif

	typedef AudioBackend* (*BACKEND_CREATOR_FUNC)();
	struct BACKEND_CREATOR
	{
		const char * name;
		BACKEND_CREATOR_FUNC create;
	};

	static BACKEND_CREATOR _backendTable[] =
	{
#ifdef NB_CFG_DSOUND
		{ "DirectSound", &CreateDSoundMaster},
#endif

#ifdef NB_CFG_ALSA
		{ "ALSA", &CreateAlsaMaster},
#endif

#ifdef NB_CFG_OPENAL
		{ "OpenAL", &CreateOpenALMaster},
#endif

#ifdef NB_CFG_PULSE
		{ "PulseAudio", &CreatePulseMaster},
#endif
	};
	static const int AVAILABLE_BACKEND_COUNT = sizeof(_backendTable)/sizeof(BACKEND_CREATOR);

	void APlayer::GetAvailableBackends(Array<stringa> & _name_ret)
	{
		_name_ret.clear();
		for(int i=0; i<AVAILABLE_BACKEND_COUNT; i++)
		{
			_name_ret.push_back(_backendTable[i].name);
		}
	}

	class AStream : public IAsyncDelete
	{
		Path path; // keep path for rewind.
		static const int MASTER_TAKE = 1;
		static const int MASTER_QUIT = 2;

		static const int WORKER_BUF_READY = 0;
		static const int WORKER_END_OF_STREAM = 1;
		static const int WORKER_NORMAL_QUIT = 2;
		static const int WORKER_ERROR_QUIT = 3;
		static const int WORKER_INITED = 4;

		static const int BUF_SIZE = FRAME_RATE;
		SoundFileReader * reader; // will set to 0 by worker when error quit
		int loop;
		Event worker_quit;
		Event master_event;
		int   master_msg;
		Event worker_event;
		int   worker_msg;
		Thread * thread;
		bool end_of_stream;
		AInfo info;
		float  buf[BUF_SIZE];
		int    buf_size_sample;
		int    samples_in_buf;
		float  buf2[BUF_SIZE];

		static int _WorkerProc(void * _p)
		{
			AStream * _this = (AStream*)_p;
			return _this->WorkerProc();
		}

		int WorkerProc()
		{
			while(true)
			{
				master_event.Wait(true);
				switch(master_msg)
				{
				case MASTER_TAKE:
					if(end_of_stream)
					{
						worker_msg = WORKER_END_OF_STREAM;
						worker_event.Set();
					}
					else
					{
_READ_AGAIN:
						int n = reader->Read(buf, this->buf_size_sample, this->info.channel_count);
						switch(n)
						{
						case 0:
						case NB_READ_END:
							if(loop != 0)
							{
								if(loop > 0)
								{
									loop--;
								}
								if(reader->Rewind())
								{
									goto _READ_AGAIN;
								}
								else
								{
									delete reader;
									try
									{
										reader = SoundFileReader::Create(path);
										AInfo info1;
										reader->GetInfo(info1);
										if(info1.channel_count == info.channel_count)
										{
											goto _READ_AGAIN;
										}
										else
										{
											delete reader;
											reader = 0;
										}
									}
									catch(...)
									{
										reader = 0;
									}
								}
							}

							worker_msg = WORKER_END_OF_STREAM;
							worker_event.Set();
							break;
						case NB_READ_ERR:
							delete reader;
							reader = 0;
							worker_msg = WORKER_ERROR_QUIT;
							worker_event.Set();
							goto _ERROR_QUIT;
						default:
							samples_in_buf = n;
							worker_msg = WORKER_BUF_READY;
							worker_event.Set();
							break;
						}
					}
					break;
				case MASTER_QUIT:
					delete reader;
					reader = 0;
					worker_msg = WORKER_NORMAL_QUIT;
					worker_event.Set();
					goto _NORMAL_QUIT;
				};
			}
_NORMAL_QUIT:
			worker_quit.Set();
			return 0;
_ERROR_QUIT:
			worker_quit.Set();
			return 1;
		}


		int AsyncRead()
		{
			if(worker_event.IsSet())
			{
				if(samples_in_buf)
				{
					E_ASSERT(this->samples_in_buf <= this->buf_size_sample);
					memcpy(buf2, buf, sizeof(buf2));
					int ret = samples_in_buf;
					samples_in_buf = 0;
					if(worker_msg == WORKER_BUF_READY)
					{
						worker_event.Reset();
						master_msg = MASTER_TAKE;
						master_event.Set();
					}
					return ret;
				}
					
				if(reader == 0)
				{
					return -1;
				}

				switch(worker_msg)
				{
				case WORKER_BUF_READY:
					{
						E_ASSERT(0);
						worker_event.Reset();
						master_msg = MASTER_TAKE;
						master_event.Set();
						return 0;
					}
				case WORKER_ERROR_QUIT:
				case WORKER_NORMAL_QUIT:
					return NB_READ_ERR;
				case WORKER_END_OF_STREAM:
					return NB_READ_END;
				}
			}
			return 0;
		}

		~AStream()
		{
			delete thread;
		}

	public:
		AStream(const Path  & _path, int _loop)
		{
			reader = SoundFileReader::Create(_path);
			reader->GetInfo(info);
			path = _path;
			loop = _loop;
			end_of_stream = false;
			thread = enew Thread(Callback(&AStream::_WorkerProc), this);
			buf_size_sample = BUF_SIZE / info.channel_count;
			this->samples_in_buf = 0;
			master_msg = MASTER_TAKE;
			master_event.Set();

			message(L"[nb] AStream: buf size = " + string(FRAMES_TO_MSEC(BUF_SIZE/AudioBackend::CHANNEL_COUNT)) + L"ms");
		}

		void GetInfo(AInfo & _info)
		{
			_info = info;
		}
		inline int Read(float * & _cur, float * & _end)
		{
			E_ASSERT(_cur == _end);
			int n = AsyncRead();
			if(n > 0)
			{
				_cur = buf2;
				_end = _cur + n * info.channel_count;
			}
			else
			{
#ifdef NB_CFG_VERBOSE
				E_TRACE_LINE("[nb] AStream: (WW) underrun ");
#endif
			}
			return n;
		}

		bool Rewind()
		{
			if(reader->Rewind())
			{
				samples_in_buf = 0;
				master_msg = MASTER_TAKE;
				master_event.Set();
				return true;
			}
			else
			{
				return false;
			}
		}

		bool OnAsyncDelete() override
		{
			if(worker_quit.IsSet())
			{
				delete this;
				return true;
			}
			if(worker_event.IsSet())
			{
				if(worker_msg != WORKER_NORMAL_QUIT && worker_msg != WORKER_ERROR_QUIT)
				{
					master_msg = MASTER_QUIT;
					worker_event.Reset();
					master_event.Set();
					worker_event.Wait();
				}
				delete this;
				return true;
			}
			
			return false;
		}
	};


#define ALLPASS_G 0.5f
	struct AllPass
	{
		float * buf;
		float * cur;
		float * end;
		int n;

		AllPass()
		{
			buf = 0;
		}
		void Init(int _n)
		{
			n = _n;
			free(buf);
			buf = (float*)malloc(sizeof(float) * n);
			memset(buf, 0, sizeof(float) * n);
			cur = buf;
			end = buf + n;
		}

		~AllPass()
		{
			free(buf);
		}

		//                       *g
		//           |----------->>--->
		// in ---+--x1----->[M]----y--+------> out
		//       <-------<<--------|
		//               *g
		float step(float x)
		{
			float y = *cur;
			x+= y * ALLPASS_G;
			*cur++ = x;
			if(cur == end)
			{
				cur = buf;
			}
			return y - x*ALLPASS_G;
		}
	};

	struct LowPass
	{
		float   y;
		float   alpha; // low pass param
		// float   beta;  // = 1 - alpha
		void Init(float freq)
		{
			y = 0;
			float rc = 1.0f / (2.0f*3.14159265f*freq);
			float dt = 1.0f / FRAME_RATE;
			alpha = dt / (rc + dt);
		}

		float step(float x)
		{
			y = alpha * x + (1-alpha) * y;
			return y;
		}
	};

	struct TabDelay
	{
		static const int SZ = 24;
		float buf[SZ+1];
		float * cur_a;
		float * cur_b;
		void Init(int n)
		{
			if(n < 0)
			{
				n = SZ -n;
			}
			if(n < 0)
			{
				n = 0;
			}
			if(n > SZ)
			{
				n = SZ;
			}
			memset(buf, 0, sizeof(buf));
			cur_a = buf;
			cur_b = cur_a + n;
		}

		float step(float x, float &a, float &b)
		{
			a = *cur_a;
			*cur_a++= x;
			if(cur_a == buf + SZ + 1)
			{
				cur_a = buf;
			}
			b = *cur_b++;
			if(cur_b == buf + SZ + 1)
			{
				cur_b = buf;
			}
			return b;
		}
	};

	/*
   2     3     5     7    11    13    17    19    23    29
   31    37    41    43    47    53    59    61    67    71
   73    79    83    89    97   101   103   107   109   113
  127   131   137   139   149   151   157   163   167   173
  179   181   191   193   197   199   211   223   227   229
  233   239   241   251   257   263   269   271   277   281
  283   293   307   311   313   317   331   337   347   349
  353   359   367   373   379   383   389   397   401   409
  419   421   431   433   439   443   449   457   461   463
  467   479   487   491   499   503   509   521   523   541
  547   557   563   569   571   577   587   593   599   601
  607   613   617   619   631   641   643   647   653   659
  661   673   677   683   691   701   709   719   727   733
  739   743   751   757   761   769   773   787   797   809
  811   821   823   827   829   839   853   857   859   863
  877   881   883   887   907   911   919   929   937   941
  947   953   967   971   977   983   991   997  1009  1013

	 */
	struct Reverb
	{
		AllPass  A[8];
		TabDelay D[3];
		LowPass  L[3];
		float y;
		float damping;
		float in_level0;
		float in_level1;
		float in_level2;
		float out_level0;
		float out_level1;
		float out_level2;
		bool  on;
		float level;

		Reverb()
		{
			on = false;
			// Init(1.0f, 20.0f, 0.5f, 700.0f);
		}
		void Init(int _leveli, int _room_sizei, int _dampingi, int _freqi)
		{
			this->on = _leveli > 0;
			level = _leveli * 0.01f;
			damping= 0.01f * _dampingi;
			float room_size = (float)_room_sizei;
			float freq = (float)_freqi;

			y = 0;
			if(room_size < 1)
			{
				room_size = 1;
			}
			if(room_size > 100)
			{
				room_size = 100;
			}
			if(level < 0.1f)
			{
				level = 0.1f;
			}
			if(level > 1.0f)
			{
				level = 1.0f;
			}
			if(damping < 0.00f)
			{
				damping = 0.00f;
			}
			if(damping > 1.0f)
			{
				damping = 1.0f;
			}
			if(freq < 1.0f)
			{
				freq = 1.0f;
			}
			if(freq > 5000.0f)
			{
				freq = 5000.0f;
			}

			in_level0 = level * 0.7f;
			in_level1 = level * 0.5f;
			in_level2 = level * 0.11f;
			out_level0 = level * 2.0f;
			out_level1 = level * 2.8f;
			out_level2 = level * 3.9f;
			damping = 1 - damping;

			//m = 1;
			float ms = 0.7f * 1000 * room_size / 314.0f;
			float m = ms * 0.1f;
			A[0].Init((int)(m * 227));
			A[1].Init((int)(m * 199));
			A[2].Init((int)(m * 331));
			A[3].Init((int)(m * 241));
			A[4].Init((int)(m * 137));
			A[5].Init((int)(m * 479));
			A[6].Init((int)(m * 337));
			A[7].Init((int)(m * 151));
			D[0].Init(18);
			D[1].Init(19);
			D[2].Init(15);
			L[0].Init(freq * 1.3f);
			L[1].Init(freq * 1.0f);
			L[2].Init(freq * 0.8f);
		}
		//  i                    o  i             o  i                 o   (-)
		// -+->[A0][A1]{L0}+[A2](D0)+[A3][A4]{L1}(D1)+[A5][A6]{L2}[A7](D2)--|
		//  ^---------------------------------------------------------------|
		void step(float x, float &a, float &b)
		{
			float a0, b0;
			//y = last;
			y+= x * in_level0;
			y = A[0].step(y);
			y = A[1].step(y);
			y = L[0].step(y);
			y = A[2].step(y);
			y = D[0].step(y, a0, b0); a+= a0 * out_level0; b+= b0 * out_level0;
			y+= x * in_level1;
			y = A[3].step(y);
			y = A[4].step(y);
			y = L[1].step(y);
			y = D[1].step(y, b0, a0); a+= a0 * out_level1; b+= b0 * out_level1;
			y+= x * in_level2;
			y = A[5].step(y);
			y = A[6].step(y);
			y = L[2].step(y);
			y = A[7].step(y);
			y = D[2].step(y, a0, b0); a+= a0 * out_level2; b+= b0 * out_level2;
			y*= damping;
		}
	};

	struct ATrack
	{
		int  channel_count;
		inline bool is_on() const
		{ return channel_count != 0; }
		bool is_paused;
		bool no_pause;
		float fade_out;
		int  loop;
		AStream * stream;
		float * buf_begin;
		float * buf_cur;
		float * buf_end;
		// pan and gain
		float gain_l;
		float gain_r;
		void SetPanGain(float _x, float _gain)
		{
			float b = (_x - 0.5f) * 0.5f;
			if(b < 0)
			{
				b = -b;
			}
			gain_l = 2 * _gain * (1-_x) * (1-b);
			gain_r = 2 * _gain * _x     * (1-b);
		}
	};


#	define FADE_OUT_DECR (0.00005f)
#	define FADE_OUT_INIT (0.9999f)
	struct AMixer
	{
		Reverb reverb;
		typedef Map<stringa, ABlock*> ABlockPool;
		static const int MAX_TRACK_COUNT = 64;
		AudioBackend * backend;
		Path folder;
		ABlockPool ablock_pool;
		ATrack track[MAX_TRACK_COUNT];
		int track_count; // initial 0, alloc on use
		int idle_track[MAX_TRACK_COUNT];
		int idle_track_count;

		float bgm_gain;
		float se_gain;
		float sum_l;
		float sum_r;
		MixingFrac<90, 95> mixing_frac;
//		float mixing_gain;

		int16 * out_block_buf;
		/*
		static const int CYCLE_BUF_SIZE_MSEC = 1000;
		int cycle_buf_size_block;
		int cycle_buf_size_frame;
		int cycle_buf_size_sample;
		float * out_cycle_buf;
		float * out_cycle_cur;
		float * out_cycle_end;

		static const int MAX_REVERB_UNIT_COUNT = 10;
		struct REVERB_UNIT
		{
			float * px;
			float   gain;
			float   y;
			float   alpha; // low pass param
			float   beta;  // = 1 - alpha
		};
		REVERB_UNIT reverb_uint[MAX_REVERB_UNIT_COUNT];
		int reverb_uint_count;
		*/

		Thread * mixer_thread;
		bool     to_quit;
		Event    quit;
		Mutex    mutex;

		AMixer(AudioBackend * _backend)
		{
//			all_pass[0].Init(1051);
//			all_pass[1].Init(337);
//			all_pass[2].Init(113);
//			mixing_gain = 1.0f;
			bgm_gain = 1.0f;
			se_gain  = 1.0f;

			E_ASSERT(_backend->block_size_msec > 0);
			//cycle_buf_size_block = CYCLE_BUF_SIZE_MSEC / _backend->block_size_msec + 1;
			//cycle_buf_size_frame = cycle_buf_size_block * _backend->block_size_frame;
			//cycle_buf_size_sample = cycle_buf_size_frame * _backend->CHANNEL_COUNT;
			//out_cycle_buf = (float*) malloc(cycle_buf_size_sample * sizeof(float));
			//memset(out_cycle_buf, 0, cycle_buf_size_sample * sizeof(float));
			//out_cycle_cur = out_cycle_buf;
			//out_cycle_end = out_cycle_buf + _backend->block_size_sample;

			out_block_buf = (int16*) malloc(_backend->block_size_byte);
			memset(out_block_buf, 0, _backend->block_size_byte);

//			out_buf_ready = false;
			backend = _backend;
			memset(idle_track, 0, sizeof(idle_track));
			track_count = 0;
			idle_track_count = 0;
			to_quit = false;
			mixer_thread = enew Thread(_WorkerThread, this);
		}

		~AMixer()
		{
			to_quit = true;
			quit.Wait();

			for(int i=0; i<track_count; i++)
			{
				ATrack &t = track[i];
				if(t.is_on() && t.stream)
				{
					AsyncDelete(t.stream);
					t.stream = 0;
				}
			}

			for(ABlockPool::iterator it=ablock_pool.begin(); it != ablock_pool.end(); ++it)
			{
				delete it->second;
			}

			if(backend)
			{
				delete backend;
			}
			delete mixer_thread;
			free(out_block_buf);
		}

		static int _WorkerThread(void * _this)
		{
			return ((AMixer*)_this)->WorkerThread();
		}

		int WorkerThread()
		{
			int sleep_ms = 5;
			Mixing();
			while(!to_quit)
			{
				if(backend->Submit(out_block_buf))
				{
					Mixing();
				}
				else
				{
					Sleep(sleep_ms);
				}
			}
			quit.Set();
			return 0;
		}

		inline void ReleaseTrack(ATrack & _t, int _idx)
		{
			_t.channel_count = 0;
			if(_t.stream)
			{
				AsyncDelete(_t.stream);
				_t.stream = 0;
			}
			idle_track[idle_track_count++] = _idx;
		}

		void Mixing()
		{
			mutex.Lock();
		//	REVERB_UNIT * pru_end = reverb_uint + reverb_uint_count;
			int16 * out_block_cur = out_block_buf;
			bool reverb_on = reverb.on;
			float reverb_direct_frac = 1 - reverb.level * 0.5f;
			int frame = backend->block_size_frame;
			while(frame--)
			{
				sum_l = 0;
				sum_r = 0;
				//div = 0;
				for(int i=0; i < track_count; i++)
				{
					ATrack & t = track[i];
					if(!t.is_on() || t.is_paused)
					{
						continue;
					}

					if(t.fade_out <1.0f)
					{
						t.fade_out-= FADE_OUT_DECR;
						if(t.fade_out < 0)
						{
							ReleaseTrack(t, i);
							continue;
						}
					}

					if(t.buf_cur == t.buf_end)
					{
						bool end = false;
						if(t.stream)
						{
							int n = t.stream->Read(t.buf_cur, t.buf_end);
							if(n==0)
							{
								continue;
							}
							else if(n == NB_READ_END || n == NB_READ_ERR)
							{
								end = true;
							}
						}
						else
						{
							if(t.loop != 0)
							{
								if(t.loop > 0)
								{
									t.loop--;
								}
								t.buf_cur = t.buf_begin;
							}
							else
							{
								end = true;
							}
						}
						if(end)
						{
							ReleaseTrack(t, i);
							continue;
						}
					}

					float l, r;
					if(t.channel_count==1)
					{
						l = *t.buf_cur++;
						r = l;
					}
					else
					{
						l = *t.buf_cur++;
						E_ASSERT(t.buf_cur < t.buf_end);
						r = *t.buf_cur++;
					}
					sum_l+= l * t.gain_l * t.fade_out;
					sum_r+= r * t.gain_r * t.fade_out;
				}
				float frac = mixing_frac.Get();
				sum_l*= frac;
				sum_r*= frac;
				if(reverb_on)
				{
					float combined = sum_l + sum_r;
					sum_l*= reverb_direct_frac;
					sum_r*= reverb_direct_frac;
					reverb.step(combined, sum_l, sum_r);
				}

				bool b1 = sum_l > 0.95f;
				bool b2 = sum_l < -0.95f;
				bool b3 = sum_r > 0.95f;
				bool b4 = sum_r < -0.95f;
				if(b1||b2||b3||b4)
				{
					mixing_frac.OnDistor(sum_l, sum_r);
				}
				mixing_frac.Step();
				if(sum_l > 1.0f)
				{
					sum_l = 1.0f;
				}
				else if(sum_l < -1.0f)
				{
					sum_l = -1.0f;
				}
				if(sum_r > 1.0f)
				{
					sum_r = 1.0f;
				}
				else if(sum_r < -1.0f)
				{
					sum_r = -1.0f;
				}

				int li = int16(sum_l * 32767);
				*out_block_cur++= li;
				E_ASSERT(out_block_cur < out_block_buf + backend->block_size_sample);
				int ri = int16(sum_r * 32767);
				*out_block_cur++= ri;
			}
			mutex.Unlock();
		}

		ABlock * GetABlock(const stringa & _name)
		{
			ABlock * p = 0;
			ABlockPool::iterator it = ablock_pool.find(_name);
			if(it == ablock_pool.end())
			{
				try
				{
					const static Char * exts[3] =
					{
						L".wav",
						L".ogg",
					};

					Path path;
					for(int i=0; i<3 && p==0; i++)
					{
						path = folder | (string(_name) + exts[i]);
						if(FS::IsFile(path))
						{
							p = enew ABlock;
							p->Load(path);
						}
					}
				}
				catch(const char * _err)
				{
					(_err);
					message(L"[nb] (WW) AMixer::GetPCM(\"" + string() + L"\" : " + string(_err));
					p = 0;
				}
				catch(...)
				{
					E_ASSERT(0);
					p = 0;
				}

				if(p)
				{
					if(p->data == 0)
					{
						delete p;
						p = 0;
					}
				}
				ablock_pool[_name] = p;
			}
			else
			{
				p = it->second;
			}
			return p;
		}

		ATrack * AllocTrack()
		{
			mutex.Lock();
			ATrack * p = 0;
			if(idle_track_count)
			{
				int n = idle_track[--idle_track_count];
				E_ASSERT(n < MAX_TRACK_COUNT);
				p = &track[n];
				E_ASSERT(!p->is_on());
			}
			else if(track_count < MAX_TRACK_COUNT)
			{
				p = &track[track_count++];
				p->channel_count = 0; // uninit
				p->stream = 0;
				//p->gain_l = 0.75f;
				//p->gain_r = 0.75f;
			}
			mutex.Unlock();
			return p;
		}

		bool PlaySE(const stringa & _name, float _x, float _gain, int _loop, bool _no_pause)
		{
			ABlock * block = GetABlock(_name);
			if(block == 0)
			{
				return false;
			}

			ATrack * track = AllocTrack();
			if(track == 0)
			{
				message(L"[nb] (WW) Failed to play Sound Effect \"" + _name + L"\", all " + string(MAX_TRACK_COUNT) + " tracks are busy.");
				return false;
			}

			mutex.Lock();
			track->stream = 0;
			track->buf_begin = block->data;
			track->buf_cur = block->data;
			track->buf_end = block->data + block->float_count();
			track->no_pause = _no_pause;
			track->is_paused = false;
			track->loop = _loop;
			track->channel_count = block->channel_count;
			track->fade_out = 1.0f;
			track->SetPanGain(_x, _gain * se_gain);
			mutex.Unlock();

			return true;
		}

		void PlayBGM(const Path & _path, int _loop, AInfo * _info) throw(const char *)
		{
			if(_info)
			{
				_info->channel_count = 0;
				_info->title.clear();
				_info->frame_count = 0;
			}

			AStream * stream = enew AStream(_path, _loop);

			AInfo info;
			if(_info == 0)
			{
				_info = &info;
			}

			stream->GetInfo(*_info);
			int channel_count = _info->channel_count;


			if(channel_count!=1 && channel_count!=2)
			{
				AsyncDelete(stream);
				E_ASSERT(0);
				throwf(NB_SRC_LOC "Unsupported channel number: %d", channel_count);
			}

			ATrack * track = AllocTrack();
			if(track == 0)
			{
				message(L"[nb] (WW) Failed to play BGM \"" + _info->title + L"\", all " + string(MAX_TRACK_COUNT) + " tracks are busy.");
			//	delete reader;
				AsyncDelete(stream);
				throw(NB_SRC_LOC "All audio track are busy.");
			}

			mutex.Lock();
			track->stream = stream;
			track->buf_begin = 0;
			track->buf_cur = 0;
			track->buf_end = 0;
			track->no_pause = false;
			track->is_paused = false;
			track->loop = 0; // handle loop in AStream
			track->channel_count = channel_count;
			track->fade_out = 1.0f;
			track->SetPanGain(0.5f, bgm_gain);
			mutex.Unlock();
		}
	};

	string APlayer::GetBackendName()
	{
		return mixer->backend->name;
	}

	APlayer::APlayer(const stringa & _backend, const Path & _seFolder, uint _bufSizeMsec) throw(const char *)
	{
		mixer = 0;

		if(AVAILABLE_BACKEND_COUNT==0)
		{
			throw(NB_SRC_LOC "This program was compiled without audio support");
		}

		BACKEND_CREATOR * p =0;

		if(_backend.empty() || _backend.icompare("default") == 0)
		{
			p = &_backendTable[0];
		}

		for(int i=0; i<AVAILABLE_BACKEND_COUNT; i++)
		{
			BACKEND_CREATOR & t = _backendTable[i];
			if(_backend.icompare(t.name) == 0)
			{
				p = &_backendTable[i];
				break;
			}
		}

		if(p)
		{
			AudioBackend * backend = p->create();
			try
			{
				backend->PreInit(_bufSizeMsec);
				backend->Init();
			}
			catch(const char * _exp)
			{
				delete backend;
				stringa s(_exp);
				throwf(NB_SRC_LOC "Audio backend \"%s\" init failed.\n%s", _backend.c_str(), s.c_str());
			}
			backend->name = p->name;
			mixer = enew AMixer(backend);
			mixer->folder = _seFolder;
		}
		else
		{
			throwf(NB_SRC_LOC "Audio backend \"%s\" not found", _backend.c_str());
		}
	}

	APlayer::~APlayer()
	{
		delete mixer;
	}


	void APlayer::GetActiveCount(int * _bgm, int * _se) const
	{
		mixer->mutex.Lock();
		int a=0, b=0;
		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on())
			{
				if(t.stream)
				{
					a++;
				}
				else
				{
					b++;
				}
			}
		}
		mixer->mutex.Unlock();
		if(_bgm)
		{
			*_bgm  = a;
		}
		if(_se)
		{
			*_se  = b;
		}
	}

	void APlayer::Pause()
	{
		mixer->mutex.Lock();
		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on() && !t.no_pause)
			{
				t.is_paused = true;
			}
		}
		mixer->mutex.Unlock();
	}

	void APlayer::Resume()
	{
		mixer->mutex.Lock();
		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on())
			{
				t.is_paused = false;
			}
		}
		mixer->mutex.Unlock();
	}

	void APlayer::StopBGM()
	{
		mixer->mutex.Lock();
		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on() && t.stream && t.fade_out>FADE_OUT_INIT)
			{
				t.fade_out = FADE_OUT_INIT;
			}
		}
		mixer->mutex.Unlock();
	}

	bool APlayer::PlaySE(const stringa & _name, float _x, float _gain, int _loop, bool _no_pause) throw()
	{
		return mixer->PlaySE(_name, _x, _gain, _loop, _no_pause);
	}

	void APlayer::PlayBGM(const Path & _path, int _loop, AInfo * _info) throw(const char *)
	{
		mixer->PlayBGM(_path, _loop, _info);
	}
/*
	void APlayer::SetMixerGain(float _gain)
	{
		if(_gain < 0)
		{
			_gain = 0;
		}
		if(_gain > 1.0f)
		{
			_gain = 1.0f;
		}
		mixer->mutex.Lock();
		mixer->mixing_gain = _gain;
		mixer->mutex.Unlock();
	}
*/
	void APlayer::GetGain(float & _bgm, float & _se)
	{
		mixer->mutex.Lock();
		_bgm = mixer->bgm_gain;
		_se  = mixer->se_gain;
		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on() && t.stream )
			{
				t.SetPanGain(0.5f, _bgm);
			}
		}
		mixer->mutex.Unlock();
	}

	void APlayer::SetGain(float _bgm, float _se)
	{
		mixer->mutex.Lock();
		mixer->bgm_gain = _bgm;
		mixer->se_gain  = _se;

		for(int i=0; i<mixer->track_count; i++)
		{
			ATrack &t = mixer->track[i];
			if(t.is_on() && t.stream )
			{
				t.SetPanGain(0.5f, _bgm);
			}
		}
		mixer->mutex.Unlock();
	}

	void APlayer::SetReverb(int _level, int _room_size, int _damping, int _freq)
	{
		mixer->mutex.Lock();
		mixer->reverb.Init(_level, _room_size, _damping, _freq);
		mixer->mutex.Unlock();
	}
}

