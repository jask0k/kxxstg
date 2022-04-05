
#pragma once
#include <nbug/tl/map.h>
#include <nbug/core/def.h>
#include <nbug/gl/tex.h>
#include <nbug/math/math.h>
#include <z_kxx/stage/time_line.h>

namespace e
{
	class Actor;
	class Stage;
	class KxxDialog
	{
		uint32 timeTick;
		enum Action
		{
			Say,
		//	Move,
			ShowName,
		};
		struct Item
		{
			Action action;
			Actor * actor;
			string actorShortName;
			Vector2 moveFrom;
			Vector2 moveTo;
			string text;
			int emotion;
		};

		Stage * stage;
		const int dialogID;
		TexRef boxTex;
		Array<Item*> items;
		typedef e::Map<string, Actor*> ActorMap;
		e::Map<string, Actor*> actorMap;
		
		int GetCurrentItemIndex();
		bool SwitchToItem(int _index);

		Vector2 leftCurrentPos;
		Vector2 leftTargetPos;
		float leftTargetAlpha;
		float leftCurrentAlpha;
		Vector2 rightCurrentPos;
		Vector2 rightTargetPos;
		float rightTargetAlpha;
		float rightCurrentAlpha;
		Item * leftItem;
		TexRef leftTex;
		float leftTexW;
		float leftTexH;
		Item * rightItem;
		TexRef rightTex;
		float rightTexW;
		float rightTexH;
		bool isLeftSpeeking;
		string currentContent;

		//
		uint32 showNameTimer;
		TexRef nameTex;


		//bool isBossMoving;
		//Vector2 targetPostion;
		//float moveSpeed;
		void LoadItems();
		void SwitchToNextItem(bool _force);
		bool fireButtonDown;
	public:
		void CheckJoystick();
		//void Start()  override;
		bool Step();
		void Render();
		void RenderLeft();
		void RenderRight();
		KxxDialog(Stage * _stage, int _dialogID);
		~KxxDialog();
	};

}
