
#pragma once

#include <z_kxx/enemy/enemy.h>
#include <z_kxx/sprite/ring.h>
#include <z_kxx/sprite/general_aura.h>
#include <z_kxx/util/ani.h>

namespace e
{
	class SCName;
	class BossScript;
	class Stage;
	class Boss : public Enemy
	{
	public:
		Ani ani_stand_left;
		Ani ani_stand_right;
		Ani ani_move_left;
		Ani ani_move_right;
		Ani ani_fire;
		uint32 mid_boss_timer;
		Stage * const stage;
		const int index_in_stage;
		const string short_name;
		TexRef avatar_tex;
		float low_ring0_scale;
		float low_ring1_scale;
		bool low_ring0_scale_sign;
		bool low_ring1_scale_sign;
		bool is_show_aura;
		GeneralAura * aura1;
		Ring * low_ring0;
		Ring * low_ring1;
		Ring * ring0;
		Ring * ring1;
		Sprite * dark;
		SCName * sc_name;
		Boss(const string & _short_name, Stage * _stage, int _index_in_stage);
		~Boss();
		bool is_fighting; // fighting is in special time line
		bool is_sc_time; // SC mode color
		bool is_perfect_round; // no miss
		uint32 script_timer;
		float display_life;

		//void Appear();
		virtual void BeginFight();
		void Render() override;
		void ShowAvatar();
		void ShowPerfectText();
		void ShowSCName(const string & _s);
		void HideSCName();
		void ShowRing(bool _show); // sc ring
		void ShowAura(bool _show); // general aura
		void SetDark(float _radius, float _darkness);
	protected:
		bool is_ring_visible;
		float current_ring_scale;
		float display_dark_radius;
		float target_dark_radius;
		float display_dark_alpha;
		float target_dark_alpha;
		BossScript * script;
		void AddScript(BossScript * _p);
		void _InitCurrentScript();
		void _EndCurrentScript();
		//virtual void SetBossDrop(int _n);
		bool OnLifeEmpty() override;
		bool Step() override;
	};
}
