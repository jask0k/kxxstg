#include <z_kxx/ui/digital_roller.h>
#include <z_kxx/globals.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	DigitalRoller::DigitalRoller()
	{
		strip_tex[0] = kxxwin->LoadTex("digital-roller-strip-1");
		strip_tex[1] = kxxwin->LoadTex("digital-roller-strip-12");
		strip_tex[2] = kxxwin->LoadTex("digital-roller-strip-25");
		strip_tex[3] = kxxwin->LoadTex("digital-roller-strip-50");
		strip_tex[4] = kxxwin->LoadTex("digital-roller-strip-100");
		panel_tex = kxxwin->LoadTex("digital-roller-panel");

		num_delta_angle = 2*PI / 20;
		float a = 0;
		for(int i=0; i<20; i++)
		{
			num_fix_angle[i] = a;
			a+= num_delta_angle;
		}

		E_ASSERT(YC % 2 == 0);
		y_offset = 9;
		float x=16;
		for(int i=0; i<9; i++)
		{
//			shift[i] = 0;
			num_current_angle[i] = num_fix_angle[0];
			num_target_angle[i] = num_fix_angle[0];
			num[i] = 0;
			x_offset[9-i-1] = x;
			x+= 18;
			if(i%3 == 2)
			{
				x+=5;
			}
		}
	//	shift[9] = 0;

		float digital_h = 29;
		r = digital_h / 2 / sin(PI/10);
		float dya = 2 * PI / 10 / YC;
		for(int i=0; i<9; i++)
		{
			float a = -PI / 10;
			for(int j=0; j<YC; j++)
			{
				{
					VCT * p = &v[i][j*2+0];
					p->c = 0xFFFFFFFF;
					p->v.z = 0;
					p->v.x = x_offset[i];
					p->v.y = y_offset + digital_h/2 - r * sin(a);
					p->t.x = 0;
				}

				{
					VCT * p = &v[i][j*2+1];
					p->c = 0xFFFFFFFF;
					p->v.z = 0;
					p->v.x = x_offset[i] + 18;
					p->v.y = y_offset + digital_h/2 - r * sin(a);
					p->t.x = 1;
				}
				a+= dya;
			}

		}
		display_num = 0;
		target_num = 0;


	}

	void DigitalRoller::Set(int _v)
	{
		if(_v < target_num)
		{
			for(int i=0; i<9; i++)
			{
				num[i] = _v%10;
				num_target_angle[i] = num_fix_angle[num[i]];
				num_current_angle[i] = num_target_angle[i];
				_v/=10;
			}
			target_num = _v;
			display_num = _v;
		}
		else
		{
			target_num = _v;
		}
	}

	void DigitalRoller::Step()
	{
	//	for(int repeat = 0; repeat < 10; repeat++)
		{
			float da = 2*PI/60;
			if(num_target_angle[0] - num_current_angle[0] < 0.001f &&
				target_num > display_num)
			{
				int add = target_num - display_num;
				//if(add > 1000)
				//{
				//	shift[0] = 3;
				//	shift[1] = 2;
				//	shift[2] = 1;
				//	shift[3] = 0;
				//}
				//else if(add > 100)
				//{
				//	shift[0] = 2;
				//	shift[1] = 1;
				//	shift[2] = 0;
				//	shift[3] = 0;
				//}
				//else if(add > 10)
				//{
				//	shift[0] = 1;
				//	shift[1] = 0;
				//	shift[2] = 0;
				//	shift[3] = 0;
				//}
				if(add > 9)
				{
					add = add * 7 / 10;
				}
				if(add > 9999)
				{
					add = 9999;
				}
				
				display_num+=add;
				for(int i=0; i<9; i++)
				{
					int real_sum;
					num_current_angle[i] = num_fix_angle[num[i]];
					real_sum = num[i] + add;
					num[i] = real_sum % 10;
					num_target_angle[i] = num_current_angle[i] + num_delta_angle * add;
					rot[i]= num_delta_angle * add / 7;
					if(real_sum < 10)
					{
						break;
					}
					else
					{
						add = real_sum / 10;
					}
				}
			}

			for(int i=8; i>=0; i--)
			{
				float a1 = num_target_angle[i] - num_current_angle[i];
				if(a1 > 0)
				{
					if(a1 > rot[i])
					{
						a1 = rot[i];
					}

					num_current_angle[i]+= a1;
				}
				else
				{
					rot[i] = 0;
				}
			}
		}
	}



	void DigitalRoller::Render(float _x, float _y)
	{
		graphics->TranslateMatrix(_x, _y);
		for(int i=0; i<9; i++)
		{
			VCT * p = v[i];
			float a = angle_normalize(num_current_angle[i]);
			if(a < 0)
			{
				a+= PI;
			}
			float dty = -1.0f / 20 / YC;
			float ty0 = 1.0f - 0.5f * a / PI;
			float ty = ty0;
			for(int j=0; j<YC; j++)
			{
				p[j*2+0].t.y = p[j*2+1].t.y = ty;
				ty+= dty;
			}
			int shift = (int)sqrt(rot[i]*5.5f);
			if(shift > 4)
			{
				shift = 4;
			}
			graphics->BindTex(strip_tex[shift]);
			graphics->SetVertexSource(p, sizeof(VCT), VCT::FORMAT, YC*2);
			graphics->DrawPrimitive(E_TRIANGLESTRIP, 0, YC*2);
		}
		graphics->BindTex(panel_tex);
		graphics->SetColor(1, 1, 1, 1);
		graphics->DrawQuad(0, 0, 204, 48);
		graphics->TranslateMatrix(-_x, -_y);
	}
}

