#ifndef NB_APLAYER_H
#define NB_APLAYER_H

#include <nbug/core/def.h>
#include <nbug/core/str.h>
#include <nbug/tl/array.h>

namespace e
{
	struct AInfo
	{
		int channel_count;
		int frame_count;
		string title;
	};

	class Path;
	struct AMixer;
	class APlayer
	{
		AMixer * mixer;
	public:
		static void GetAvailableBackends(Array<stringa> & _name_ret);
		APlayer(const stringa & _backend, const Path & _seFolder, uint _bufSizeMsec = 100) throw(const char *);;
		~APlayer();

		void SetReverb(int _level=25, int _room_size=18, int _damping=50, int _freq=1000);

		void GetActiveCount(int * _bgm, int * _se) const;
		string GetBackendName();

		bool PlaySE(const stringa & _name, float _x = 0.5f, float _gain = 1.0f, int _loop=0, bool _no_pause=false) throw();
		void PlayBGM(const Path & _path, int _loop, AInfo * _info_ret = 0) throw(const char *);
		void GetGain(float & _bgm, float & _se);
		void SetGain(float _bgm, float _se);

		void Pause();
		void Resume();
		void StopBGM();
	};
}

#endif
