
#include <nbug/core/ini.h>
#include <nbug/core/env.h>
#include <z_kxx/main/options.h>
#include <z_kxx/globals.h>

namespace e
{
#ifdef NB_WINDOWS
#	define KXX_DEFAULT_FONTNAME  L"Simsun"
#else
#	define KXX_DEFAULT_FONTNAME  L"Sans"
#endif
	template <typename T> 
	static inline void MinMaxLimit(T & _v, const T & _min, const T & _max)
	{
		if(_v < _min)
		{
			_v = _min;
		}
		else if(_v > _max)
		{
			_v = _max;
		}
	}

	KxxOptions::KxxOptions()
	{
		graphicsBackend = 0;
		vsync = true;
		soundVolume = 5;
		musicVolume = 8;
		//mixerFrac = 0.6f;
		level = 1;
		player = 0;
		// ----------------
		// [ 4 ]      [ 5 ]
		// [ 6 ]      [ 7 ]
		// ----------------
		//              0
		//            3   1
		//              2
		// ----------------
		joystickFire = 3;
		joystickSC = 2;
		joystickSlow = 7;
		joystickPause = 6;
		windowSize = 0;
		windowLeft = 100;
		windowTop =  100;
		fontName = KXX_DEFAULT_FONTNAME;
#ifdef NB_WINDOWS
		audioBackend = "DirectSound";
		audioBufMsec = 100;
#else
		audioBackend = "ALSA";
		audioBufMsec = 50;
#endif
		audioReverbLevel = 1;
		fontSize = 14;
		graphicsEffect = 0;

	}

	void KxxOptions::Load()
	{
		IniFile iniFile;
		iniFile.Load(Env::GetDataFolder() | L"options.ini");
		iniFile.Get(L"[Graphics]effect", graphicsEffect);
		iniFile.Get(L"[Graphics]vsync", vsync);
		iniFile.Get(L"[Sound]soundVolume", soundVolume);
		iniFile.Get(L"[Sound]musicVolume", musicVolume);
		iniFile.Get(L"[Sound]audioBackend", audioBackend);
		iniFile.Get(L"[Sound]audioBufMsec", audioBufMsec);
		iniFile.Get(L"[Sound]audioReverbLevel", audioReverbLevel);
		iniFile.Get(L"[Game Play]level", level);
		iniFile.Get(L"[Game Play]player", player);
		iniFile.Get(L"[Controls]joystickFire", joystickFire);
		iniFile.Get(L"[Controls]joystickSC", joystickSC);
		iniFile.Get(L"[Controls]joystickSlow", joystickSlow);
		iniFile.Get(L"[Controls]joystickPause", joystickPause);
		iniFile.Get(L"[UI]windowSize", windowSize);
		iniFile.Get(L"[UI]windowLeft", windowLeft);
		iniFile.Get(L"[UI]windowTop", windowTop);
		iniFile.Get(L"[UI]fontName", fontName);
		iniFile.Get(L"[UI]fontSize", fontSize);
//		iniFile.Get(L"[Tweak]useCache", useCache);
//		iniFile.Get(L"[Tweak]debug", debug);
		iniFile.Get(L"[Graphics]backend", graphicsBackend);
		//iniFile.Get(L"[Graphics]border", border);
		MinMaxLimit(soundVolume, 0, 10);
		MinMaxLimit(musicVolume, 0, 10);
		MinMaxLimit(audioReverbLevel, 0, 10);
		MinMaxLimit(audioBufMsec, 10, 300);
	//	MinMaxLimit(mixerFrac, 0.0f, 1.0f);
		MinMaxLimit(level, 0, K_LEVEL_COUNT - 1);
		MinMaxLimit(player, 0, 3);
		MinMaxLimit(graphicsEffect, 0, 1);
		MinMaxLimit(windowSize, 0, 2);
		MinMaxLimit(fontSize, 8, 64);
		MinMaxLimit(graphicsBackend, 0, 1);
		if(fontName.empty())
		{
			fontName = KXX_DEFAULT_FONTNAME;
		}
#ifdef NB_LINUX
		graphicsBackend = 0;
#endif
	}

	void KxxOptions::Save()
	{
		IniFile iniFile;
		iniFile.Set(L"[Graphics]effect", graphicsEffect);
		iniFile.Set(L"[Graphics]vsync", vsync);
		iniFile.Set(L"[Sound]soundVolume", soundVolume);
		iniFile.Set(L"[Sound]musicVolume", musicVolume);
		iniFile.Set(L"[Sound]audioBackend", audioBackend);
		iniFile.Set(L"[Sound]audioBufMsec", audioBufMsec);
		iniFile.Set(L"[Sound]audioReverbLevel", audioReverbLevel);
		iniFile.Set(L"[Game Play]level", level);
		iniFile.Set(L"[Game Play]player", player);
		iniFile.Set(L"[Controls]joystickFire", joystickFire);
		iniFile.Set(L"[Controls]joystickSC", joystickSC);
		iniFile.Set(L"[Controls]joystickSlow", joystickSlow);
		iniFile.Set(L"[Controls]joystickPause", joystickPause);
		iniFile.Set(L"[UI]windowSize", windowSize);
		iniFile.Set(L"[UI]windowLeft", windowLeft);
		iniFile.Set(L"[UI]windowTop", windowTop);
		iniFile.Set(L"[UI]fontName", fontName);
		iniFile.Set(L"[UI]fontSize", fontSize);
//		iniFile.Set(L"[Tweak]useCache", useCache);
//		iniFile.Set(L"[Tweak]debug", debug);
//		iniFile.Set(L"vsync", vsync);
		iniFile.Set(L"[Graphics]backend", graphicsBackend);
		//iniFile.Set(L"[Graphics]border", border);
		iniFile.Save(Env::GetDataFolder() | L"options.ini");
	}
}

