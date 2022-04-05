
#pragma once


namespace e
{
	class KxxOptions
	{
	public:
		int graphicsBackend; // 0 = opengl, 1 = direct3d
		int graphicsEffect; // 0 - speed,  1 - effect
		bool vsync;
		//bool useCache;
		//bool debug;
		//bool border;

		int soundVolume; // 0 ~ 10
		int musicVolume; // 0 ~ 10
		string audioBackend;
		int audioBufMsec;
		int audioReverbLevel;
		//float mixerFrac;

		int level; // 0 Easy 1 Normal 2 Hard 3 Lunatic  
		int player;
		int joystickFire;
		int joystickSC;
		int joystickSlow;
		int joystickPause;
		int windowSize; // 0=1, 1=2, 2=full
		int windowLeft;
		int windowTop;
		string fontName;
		int fontSize;
		KxxOptions();
		void Load();
		void Save();
	};
}
