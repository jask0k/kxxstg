#include <z_kxx/sprite/sprite.h>

namespace e
{
	class Scroll : public Sprite
	{
		int state;
		uint32 timer;
		TexRef bg_tex;
		VCT roller0[20];
		VCT roller1[20];
		VC roller2[20];
		VC roller3[20];
		float offset;
		float len;
		float max_len;
		float delta;
		float bg_h;
		float bg_w;
		float fg_h;
		float fg_w;
		float fg_x;
		float fg_y;
	public:
		Scroll(TexRef & _fgTex, TexRef & _bgTex);
		~Scroll();
		void Render() override;
		bool Step() override;
	private:
		void UpdateRoller(VCT v[], float _ty0, float _ty1);
	};
}
