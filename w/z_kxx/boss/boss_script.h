#pragma once

#include <z_kxx/drop/drop_def.h>
#include <z_kxx/boss/boss.h>

namespace e
{
	class Boss;
	class BossScript
	{
		friend class Boss;
		BossScript * next_script;
	protected:
		Boss * boss;
	public:
		bool is_appearing;
		bool is_fight;
		bool is_sc;
		bool is_time_survival;
		float init_life;
		float max_bonus;
		uint32 time_limit;
		BossScript();
		virtual ~BossScript();
		virtual void Begin() = 0;
		virtual bool Step();
		virtual void End();
		virtual void Render();
	protected:
		virtual void AddBonus();
	};

	//class MoveScript : public BossScript
	//{
	//	Vector2 to;
	//	float speed_f;
	//	Vector2 speed;
	//public:
	//	MoveScript(float _speed, float _to_x, float _to_y);
	//	void Begin() override;
	//	bool Step() override;
	//};

	class SCScript : public BossScript
	{
	protected:
		string sc_name;
	public:
		SCScript();
	};

	class NAScript : public BossScript
	{
	public:
		NAScript();
	};

	class GatherScript : public BossScript
	{
	public:
		GatherScript();
		void Begin() override;
		bool Step() override;
	};

	// explode or escape
	class EndingScript : public BossScript
	{
		Vector2 to;
		float speed_f;
		Vector2 speed;
	public:
		EndingScript();
		void Begin() override;
		bool Step() override;
		void End() override;
	};

	class MidEndingScript : public BossScript
	{
	protected:
		Vector2 to;
		float speed_f;
		Vector2 speed;
		int state;
		uint32 state_timer;
	public:
		MidEndingScript();
		void Begin() override;
		bool Step() override;
		void End() override;
	protected:
		virtual void OnStateChange();
	};
}

