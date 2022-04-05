
#pragma once

#include <z_kxx/sprite/unit.h>
// #include "SpriteList.h"
#include <z_kxx/sprite/simple_sprite.h>
#include <z_kxx/util/render_list.h>
#include <z_kxx/shot/uniform_shot.h>
#include <z_kxx/shot/shot.h>
#include <z_kxx/util/ani5.h>

namespace e
{
#define MAX_PET 3

	class KxxWin;
	struct PlayerState
	{
		int extra_card;
		float extra_bonus;
		float fastSpeed;
		float slowSpeed;
		float curScore;
		float bonus;
		float graze;
		int scCount;
		int petCount;
		int life;
		float power;
		int credits;
		Vector2 petCenter;
		Vector2 petPosition[MAX_PET];
	//	float petParam[MAX_PET];
		int flyDirection; // 1,2,3,4
		bool pendingDie;
		uint32 scTimer;
		uint32 dyingTimer;
		uint32 deadWaitingTimer;
		uint32 bornTimer;
		uint32 transparentTimer;
		uint32 lastShot0Time;
		uint32 lastShot0Match;
		uint32 lastShot1Time;
		uint32 lastShot1Match;
		uint slowDuration; // 0 ~ 60;
		//bool hasDirectDamage;
		float scScaleDelta; // player_a
		float smooth_da; // player_a

		float addPointAcc;
		Vector2 addPointAccPos;
		float addPointAccCount;
	};

#define MAX_SLOW_DURATION 28
	class SCName;
	class Ring;
	class Enemy;
	class Player : public Unit
	{
	public:
		const int id;
		PlayerState state;
	//	int state.petCount const;
		TexRef petTex;
		Ring * scRing;
		SCName * sc_name;
		string sc_name_text;
		Sprite * dark;
		void SetDark(bool _on, float _radius, float _darkness);
		//bool isRotatePet;
		enum PetPlaceEnum
		{
			ROTATE_PET,
			FRONT_PET,
			BACK_PET,
		};
		string short_name;
		const PetPlaceEnum petPlace;
		Player(int _id, PetPlaceEnum _petPlace);
		Ani5 ani5;
		SimpleSprite * aura;
		SimpleSprite * gpoint;
		TexRef avatar_tex;
		void Move(uint _joystickState);
		virtual void Fire() = 0;
		virtual void CastSpellCard() = 0;
		bool IsSlow() const
		{ return state.slowDuration!= 0; }
		float SlowFrac() const
		{ return float(state.slowDuration) / float(MAX_SLOW_DURATION); }
		// 0 = free, 1 = enter absorb range, 2 = absorbed
		int TestAbsorbDrop(const Vector2 & _pt, float _r);
		void OnAteDrop(int _type);
		void AddPower(float _a);
		bool Step() override;
		void SetPetPostion();
		//void RenderShots();
		void Render() override;
		virtual void RenderPets() = 0;
		//virtual void AddShot0HitSpark(EnemyShot * _shot) = 0;
		//virtual void AddShot1HitSpark(EnemyShot * _shot) = 0;
		void ScExplode();
		void Die();
		void Dead();
		void Born();
		void ThrowDeadDrops();
		void ResetState(bool _bornSprite);
		void SetState(const PlayerState & _state);
		bool CanCastSpellCard() const;
		bool CanCastNormalShot() const;

		float GetTotalBonus() const;

		void OnGraze();

		void AddPoint(const Vector2 & _pt, float _i, bool _frequently = false);
		void Continue();
		bool IsControlable() const;
		void RenderDebug() override;
		bool IsFullPower() const
		{ return state.power >= 1.0f; }
		~Player();
		bool IsCollideWith(Enemy * _enemy);
		bool BlockEnemyShot(EnemyShot * _p);
	protected:
		void SyncEC();
		void OnSCBegin();
		void OnSCEnd();
		void ShowSlow();
		void HideSlow();
		void AddAvatar();
	};
}
