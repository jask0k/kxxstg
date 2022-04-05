
#pragma once
#include <nbug/core/str.h>
#include <nbug/gl/tex.h>
namespace e
{
	class Actor
	{
	private:
		Actor();
		TexRef emotionTex[8];
		bool emotionTexLoadFailed[8];
		TexRef nameTex;
		bool nameTexLoadFailed;
	public:
		bool isEnemy;
		string name;
		string short_name;
		TexRef LoadEmotionTex(int _index);
		TexRef LoadNameTex();
		~Actor();
		static Actor * Create(const string & _short_name);
	};
}
