
#pragma once
#include <z_kxx/sprite/sprite.h>
namespace e
{
	class SCName : public Sprite
	{
		bool is_boss;
		int state;
		uint32 timer;
		string name;
		float text_w;
		float text_h;
		FontRef font;
		RGBA text_clr;
		Vector2 text_org;
		Vector2 state3_vel;
	public:
		SCName(bool _is_boss, TexRef & _bg, FontRef & _font);
		void Start(const string & _name);
		void Explode();
		void Render() override;
		bool Step() override;
	};
}



