
#pragma once

#include <z_kxx/globals.h>
#include <nbug/gl/graphics.h>
#include <z_kxx/stage/time_line.h>
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/player/player.h>

namespace e
{
	class StageBG : public RefObject
	{
	public:
		bool opacity;
		bool visible;
		StageBG();
		virtual bool Step();
		virtual void Render();
		virtual void FadeOut();
	};
	typedef Ref<StageBG> StageBGRef;

	class Boss;
	class BossFight;
	class KxxWin;
	class KxxDialog;
	class Stage : public Timeline
	{
		friend class Boss;
	protected:
		static const int BG_LAYER_COUNT = 4;

		bool Load();

		Array<Boss*> bosses;
		static void _OnDialog(Stage * _this, int _param0, int _param1);
		static void _OnDropItem(Stage * _this, int _param0, int _param1);
		static void _OnTitle(Stage * _this, int _param0, int _param1);
		static void _OnEnemy(Stage * _this, int _param0, int _param1);
		static void _OnStageBonus(Stage * _this, int _param0, int _param1);
		static void _OnPlayMusic(Stage * _this, int _param0, int _param1);
		static void _OnStopMusic(Stage * _this, int _param0, int _param1);
	public:
		StageBGRef bg[BG_LAYER_COUNT];
		int wait_pass_state;
		uint32 wait_dark_timer;
		bool is_test_boss;
		Boss * GetFirstBoss();
		ExtRef<KxxDialog> dialog;
		const int stage_index; // 0 1 2 ...
		int human_readable_id() // 1 2 3 ...
		{ return stage_index + 1; }
	public:
		Stage(int _stage_index);
		~Stage();
		virtual void Render();
		static void _OnCreateBoss(Stage * _this, int _param0, int _param1);
		virtual void OnCreateBoss(int _index) = 0;
		static void _OnBossFight(Stage * _this, int _param0, int _param1);
		virtual void OnBossFight(int _index) = 0;
		bool Step() override;

		void GetDebugInfo(string & _info);
		void Start() override;
		void OnMissOrSC();
	};
}
