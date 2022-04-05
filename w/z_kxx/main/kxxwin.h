
#pragma once
#include <nbug/ex/fps_calc.h>
#include <nbug/input/joystick.h>
#include <nbug/input/keyboard.h>
#include <nbug/al/aplayer.h>
#include <nbug/ex/path_indexer.h>
#include <nbug/ui/win.h>
#include <nbug/tl/map.h>

#include <z_kxx/sprite/sprite.h>
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/player/player.h>
#include <z_kxx/stage/stage.h>
#include <z_kxx/util/render_list.h>
#include <z_kxx/drop/drop.h>
#include <z_kxx/util/ani.h>
#include <z_kxx/util/ani5.h>

#define K_CFG_REPLAY_EXTRA_CHECK
#define E_MAX_JOYSTICK 2

namespace e
{
	struct SOUND_EFFECT_QUOTA
	{
		static const int SE_MAP_W = 8;
		static const int SE_MAP_C = 3;
		uint32 grid[SE_MAP_W][SE_MAP_C];
		SOUND_EFFECT_QUOTA();
		void Step();
		bool TryPlay(float _x);
	};

	class UIBase;
	class KxxOptions;
	class Spark;
	class Stage;
	class DigitalRoller;
	struct KxxWinState
	{
		uint32 earthQuakeTimer;
		uint32 darkScreenTime0;
		uint32 darkScreenTimer;
		float  darkMaxAlpha;
		float  darkFadeFrac;
		bool onceContinued;
		bool onceRestartStage;
		bool onceCheated;
		int level;
		int playingPlayer;
		uint initialRandomSeed; 
		uint lastJoystickState;
		uint joystickEventCount; // for generate random seed
		uint joystickStateForReplay;
		uint joystickState[E_MAX_JOYSTICK];
		
		uint32 logicTimer;

		int replayPosition;

		// addition info genergate at stage clear
		int replayStopPosition;
		float stageHighScore;

		uint delay_clear_enemy_shot_timer;
		bool delay_clear_enemy_shot_absorb;
	};

	class Pass2;
	class Fade;
	class LensEffect;
	class KxxWin : public Win
	{
	private:
		float jostick_icon_x;
		Fade * fade;
		LensEffect * lensEffect;
		int backupWindowSize;
		FpsCalculator fpsCalc;

		uint blurScreenSteps;
		uint blurScreenTimer;
		// ExtRef<Image> blurScreenImage;
		ImageTexLoader * blurScreenImage;
		TexRef blurScreenTex;
		DigitalRoller * digitalRoller;
	public:
#ifdef E_CFG_LUA
		LuaState L;
		void InitLua();
#endif
		Pass2 * pass2;
		Matrix4 projectionMatrixUI;
		Matrix4 modelViewMatrixUI;
		Matrix4 projectionMatrixGame2D;
		Matrix4 modelViewMatrixGame2D;

		int viewport_game_x;
		int viewport_game_y;
		int viewport_game_w;
		int viewport_game_h;
		int viewport_ui_x;
		int viewport_ui_y;
		int viewport_ui_w;
		int viewport_ui_h;

		void SetToUIViewport();
		void SetToGameViewport();
		void SetToUIProjection();
		void SetToGame2DProjection();

		int lastMainMenuItem;

		//PathIndexer * texPathIndexer;
		PathIndexer * ani5PathIndexer;
		PathIndexer * aniPathIndexer;
		//static bool OnLoadTex(void * _param, const stringa & _name, Image & _pic);
		//uint32 graphicTimer; // not affect game logic
		FileRef OpenResFile(Path _relativePath);
		Player * player;

		// persist data
		struct HighScoreItem
		{
			float score;
			Char name[7];
		};
		struct PersistData
		{
			HighScoreItem topScores[K_MAX_TOP_SCORE];
			int maxPassStage;
			int InsertTopScore(float _score);
		};
		struct CombinedPersistData
		{
			PersistData persistData[4][4]; // [difficulty][player]
			bool player_ex_unlocked[4]; // a, b, c, d
			bool player_meet_ex_boss[4]; // a, b, c, d
			bool ex_unlocked()
			{ return player_ex_unlocked[0] || player_ex_unlocked[1] || player_ex_unlocked[2] || player_ex_unlocked[3]; }
			bool player_d_unlocked()
			{ return player_meet_ex_boss[0] &&  player_meet_ex_boss[1]; }
		} combinedPersistData;


		float highScoreCopy;
		////bool IsInGame() const;
		//string compiledDate;
		string osNameVersion;
		string appNameVersion;
		string graphicsHardwareString;
		string graphicsSoftwareString;
		string audioDriverType;
		FontRef defaultFont;
		FontRef smallFont;
		RGBA defaultFontColor;
		APlayer * audio;
		KxxOptions * options;
		KxxWinState state;
		uint32 practiceStageID;

		bool isPractice;
		bool isReplaying;
		bool isDemoReplay;
		uint32 demoReplayTimer;
		int demo_id;
		bool isSilient;
		bool isExStart;
		bool isPause;
		bool isWindowVisible;
		bool isWindowActive;

//#ifdef K_CFG_TEST
#	ifdef NB_DEBUG
		int debugMode;
		FontRef debugFont;
		RGBA debugFontColor;
		HSLA debugFontHSL;
		bool debugFastForward;
		int debugFastForwardSpeed;
		bool debugSmallWindow;

		int debugRenderOjbectCount;
		int debugRenderVertexCount;
		int debugTexSwitchCount;
		int debugMaterialSwitchCount;

		int debugCount_rlist;
		int debugCount_logicStepList;
		int debugCount_enemyList;
		int debugCount_playerShotList;
		int debugCount_playerSCList;
		int debugCount_enemyShotList;
		int debugCount_sparkList;
		int debugCount_uiSpriteList;
		int debugCount_dropList;
		int debugShotType;
		int debugShotColor;
#	else
		static const int debugMode = 0;
#	endif

		//TexRef itemGetLineTex;
		TexRef shotTex[K_SHOT_TYPE_COUNT][K_SHOT_COLOR_COUNT];

		TexRef dropTex[_DROP_MAX];
		TexRef dropIndicatorTex[_DROP_MAX];
		TexRef enemyAuraTex;
		TexRef statusLifeTex;
		TexRef statusSCTex;
		TexRef shotSmokeTex0;
		TexRef flashTex;
		TexRef sparkTex;
		TexRef blastWaveTex;
		TexRef explosionTex;
		TexRef playerAruaTex;
		TexRef gamepadTex;
		TexRef frozenTex;
		TexRef uiBgTex;
		TexRef shotCrashInnerTex;
		TexRef fire_sputter_tex;

#if KXX_GAME_MARGIN
		TexRef borderLeftTex;
		TexRef borderRightTex;
		TexRef borderTopTex;
		TexRef borderBottomTex;
#endif

		Ani5 fairyBlueAni5;
		Ani5 fairyRedAni5;
		Ani kedamaLeftAni;
		Ani kedamaRightAni;
		Ani kedamaPonderLeftAni;
		Ani kedamaPonderRightAni;
		TexRef kedamaFall0Left;
		TexRef kedamaFall0Right;
		TexRef kedamaFall1Left;
		TexRef kedamaFall1Right;
		TexRef badTex;

		int GetSpriteCount() const;
		void InitSprites();

		SpriteList logicStepList;
		EnemyList enemyList;
		PlayerShotList playerShotList;
		PlayerShotList playerSCList; // similar to playerShot, but can eliminate enemy shots
		EnemyShotList enemyShotList;
		SpriteList sparkList;
		SpriteList uiSpriteList;
		DropList dropList;

		void StepLogicSteps();
		void StepPlayer();
		void StepEnemies();
		void StepPlayerShots();
		// 0 - no event, 1 - graze, 2 - out or crash
		int StepSingleEnemyShot(EnemyShot * _shot);
		void StepEnemyShots();
		void StepSparks();
		void StepGeneral();
		void StepDrops();

		void ClearPlayerShots();
		void ClearEnemyShots(bool _absorb, uint _delay);
		void ClearEnemyShots(const Vector2 & _pt, float _radius);
		void ClearKxxSpriteLists();

		void FreezePlayerShots(const Vector2 & _pt, float _radius);

		void RenderDrops();

		void AddExplosion(const Vector2 & _v, float _maxRadius, int _hue);
		void AddGenericSpark(const Vector2 & _v, uint _count, int _hue);
		void AddSmallSpark(const Vector2 & _v, uint _count, int _hue);
		void AddFlashSpark(float _x, float _y, int _hue);
		void AddEnemyDrop(const DropDef & _def, const Vector2 & _pt);
		void AddDrop(int _type, float _x, float _y, bool _far = false, float _angle = -(PI * 0.5f));
		void AddBlastWave(const Vector2 & _v, bool _reverse);
		void FreeAbsorbingDrops();
		void AbsorbAllDrops();
		void StopBGM();
		void PlayBGM(int _id, int _loop = -1);
		void PlaySE_NoCheck(const stringa & _name, const Vector2 & _v, float _gain);
		void PlaySE(const stringa & _name, const Vector2 & _v, float _gain = 1.0f);
		SOUND_EFFECT_QUOTA enemy_dead_se_map;
		SOUND_EFFECT_QUOTA enemy_damage_se_map;
		void PlayEnemyDeadSE(const Vector2 & _v);
		void PlayEnemyDamageSE(const Vector2 & _v);
		void PlayMenuActionSound();
		void PlayMenuCancelSound();
		void PlayMenuSelectSound();
		void DarkScreen(float _second, float _maxDark=0.7f, float fadeFrac=0.2f);
		//void LoadOptions();
		bool LoadPersistData();
		bool SavePersistData();
		void InitFonts();
		//void InitWindow();
		void InitOffScreenBuffer();
		void InitGraphics();
		void InitAudio();

		KxxWin(KxxOptions * _options);
		~KxxWin();
		bool Create();

		void StepTimers();
		void RenderRoot();
		int CalcFastForwardAccel();
		void StepGameLogic(int _frames);
		void StepLensEffect();
		void StepNonlogic();
		bool DemoFastForward(uint32 _time);
		void OnRealTime(bool _busy) override;
		int OnGraphicsError(void *);

		void RenderClearBuffer();
		void RenderUIBack(); 
		void RenderGameBack();
		void RenderGameFront();
		void RenderUIFront(); 

		void _RenderGameExtra();
		void _RenderGameDebug();
		void _RenderUIDebug();

		void _RenderNotify();
		void _RenderJoystick();

		bool NeedRenderGame() const
		{ return IsInGame() && (this->isPause || this->currentUI == 0);	}

		void LogicStep();
		void OnTimer100ms();
		void OnTimer200ms();
		void OnTimer1000ms();
		void OnSize(int _w, int _h) override;
		void OnMouseMove(float _x, float _y) override;
		void OnKeyDown(int _sym) override;
		void OnKeyUp(int _sym) override;
		void Pause();
		void Resume();
		void DamageAllEnemy(float _damage, bool _includeEthereal);
		//void KillAllEnemy(bool _includeBoss);

		UIBase * currentUI;
		UIBase * uiToDelete;
		enum FadeType
		{
			FADE_NONE,
			FADE_ALPHA,
			FADE_SHUTTER,
			FADE_ZOOM,
		};
		void StartFade(FadeType _type);
		void ReplaceUI(UIBase * _newUI, FadeType _type);
		void SaveHighScore(float _score);
	//	void SwitchTopScoreScreen(float _score);
		void SwitchToMainMenu(bool _rewindMusic, FadeType _type);
		void SwitchToOptionsMenu();
		void SwitchToPlayerMenu();
		void SwitchToContinueMenu();
		void SwitchToPauseMenu();
		void SwitchToSelectReplayStageMenu(int _active_state_index, bool _isInGame);
		void SwitchToSelectPracticeStageMenu(int _active_state_index, bool _isInGame);
		void SwitchToEndingScreen();
		void SwitchToCreditsScreen();
		void DeletePlayers();
		void DeleteStage();
		void DeleteStageAndPlayers();
		void ChangePlayer(int _playerID);
		int PeekDemo(int _id, uint32 & _time0, uint32 & _time1);
		bool StartDemoReplay();
		void StartNewGame(int _stage_index);
		void StartStage(int _stage, bool _test_boss = false);
		int OnMainMenuStart(void *);
		int OnMainMenuExStart(void *);
		int OnMainMenuPractice(void *);
		int OnMainMenuOptions(void *);
		int OnMainMenuReplay(void *);
		int OnOptionWindowSize(void *);
		int OnOptionDifficulty(void *);
		int OnOptionGraphicsBackend(void *);
		int OnOptionAudioBackend(void *);
		int OnOptionGraphicsLevel(void *);
		int OnOptionMusicVolume(void *);
		int OnOptionSoundVolume(void *);
		int OnOptionReverb(void *);
		int OnOptionAudioBuffer(void *);
		int OnOptionSaveAndQuit(void *);
		int OnPauseReturnToGame(void *);
		int OnPauseRestartStage(void *);
		int OnPauseExitToMainMenu(void *);
		int OnPlayerSelected(void *);
		int OnPlayerCancel(void *);
		int OnMainMenuQuitApplication(void *);
		int OnContinueYes(void *);
		int OnContinueSaveAndExit(void *);
		int OnQuitEndingScreen(void *);
		int OnQuitCreditsScreen(void *);
		int OnSaveReplay(void *);
		int OnLoadReplay(void *);
		int OnSelectdReplayStage(void *);
		int OnSelectdPracticeStage(void *);
		int OnMainMenuShowHighScore(void *);
		int OnQuitGame(void *);
		void OnQuitReplay();
		void OnDifficulty(int _n);
		void OnStagePassed();

		void PollJoystickState();
		bool OnJoystickDown(int _joystick_id, int _button);
		void OnJoystickUp(int _joystick_id, int _button);

		Stage * stage;

		struct ReplayStateInfo
		{
			bool isReady;
			KxxWinState kxxWinState;
			PlayerState playerState;
			Vector2     playercenter;
			bool IsValid(int _sequenceSize) const;
		}replayStageInfo[K_STAGE_COUNT]; 

		int GetReplayStageCount();
		int GetFirstReplayStage();

		struct ReplayKey
		{
			uint32 logicTimer;
			uint keyState : 8;
			uint check1 : 12;
			uint check2 : 12;
#ifdef K_CFG_REPLAY_EXTRA_CHECK
			KxxWinState winState;
			PlayerState playerState;
#endif
		};

		Array<ReplayKey> joystickSequence;
		uint GenerateRandomSeed();
		bool PopReplayKey();
		void PushReplayKey(bool _force = false);
		bool SaveReplay(const Path & _path);
		bool LoadReplay(const Path & _path);
		static bool LoadReplay(const Path & _path, ReplayStateInfo infos[], int _stageCount, Array<ReplayKey> * _sequence);
		void UpdateCurrentStageReplaySequencePosition();
		uint GetJoystickState(uint _joystick)
		{
			E_ASSERT(_joystick < E_MAX_JOYSTICK);
			return isReplaying ? state.joystickStateForReplay : state.joystickState[_joystick];
		}

		Keyboard keyboard;
		Joystick joystick;
		void TryOpenJoystick();
		void RestartStage(bool _test_boss);
		void PromptSaveReplay();
		void PromptLoadReplay();
		bool stageClear;
		static Stage * CreateStage(int _stage_index);

		float GetCurrentAPM();
		ReplayStateInfo & CurrentStageInfo();
		void SavePassedStageCount();

		static const char * GetPlayerShortName(int _index);
		static const Char * GetPlayerName(int _index);
		static const Char * GetLevelText(int _index);

		void SetFade(Fade * _fade, float _secound);
		//void SetLensEffect(LensEffect * _pe);


		PersistData * GetCurrentPersistData()
		{ return &combinedPersistData.persistData[state.level][state.playingPlayer]; }

		bool IsInGame() const
		{ return stage != 0/* && player != 0*/; 	}

		bool IsLogicActive() const
		{ return !isPause && IsInGame() &&  currentUI == 0;	}

		bool IsWindowActiveAndVisible() const
		{ return isWindowVisible && isWindowActive; }

		bool CanSaveReplay() const
		{ return !this->isReplaying && !state.onceContinued && !state.onceRestartStage && !state.onceCheated && !this->isPractice; }

		bool CanPlaySound() const
		{ return audio != 0 && !isSilient; }

#ifdef NB_DEBUG
		string GetCantSaveReplayReason();
#endif
		//void AddSparkToList(Sprite * _p, int _layer);
		//void AddDropToList(Drop * _p, int _layer);
		//void AddEnemyShotToList(EnemyShot * _p, int _layer);
		//void AddEnemyToList(Enemy * _p, int _layer);
		//void AddGeneralSpriteToList(Sprite * _p, int _layer);
		
		void AddCorpseExplosion(Sprite * _p, float _r, int _n, uint32 _t = 180);
		void AddBossExplosion(Sprite * _s);
		void AddEnemyExplosion(Sprite * _s);

		void AddSparkToList(Sprite * _p, int _layer = RL_GAME_SPARK);
		void AddDropToList(Drop * _p, int _layer = RL_GAME_DROP);
		bool AddEnemyShotToList(EnemyShot * _p, bool _canblock = true, int _layer = RL_GAME_ENEMY_SHOT);
		void AddPlayerShotToList(PlayerShot * _p, int _layer = RL_GAME_PLAYER_SHOT);
		void AddPlayerSCToList(PlayerShot * _p, int _layer = RL_GAME_PLAYER_SHOT);
		void AddEnemyToList(Enemy * _p, int _layer = RL_GAME_ENEMY);
		void AddUISpriteToList(Sprite * _p, int _layer = RL_GAME_SPARK);
		void AddLogicActionToList(Sprite * _p);
		void AddCircleEnemyShot(const Vector2 & _v, float _angle,  float _speed, int _splitCount, int _shotTexIndex, bool _rotate);
		void AddPieEnemyShot(const Vector2 & _v, float _angle, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate);
		// down
		void AddPieEnemyShot(const Vector2 & _v, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate);
		void AddCircleEnemyShot(const Vector2 & _v, float _speed, int _splitCount, int _shotTexIndex, bool _rotate);
		// snipe
		void AddPieEnemySnipe(const Vector2 & _v, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate);
		void AddCircleEnemySnipe(const Vector2 & _v, float _speed, int _splitCount, int _shotTexIndex, bool _rotate);
		void EarthQuake(float _second);
		uint32 GetRenderTimer() const
		{ return state.logicTimer / K_LOGIC_FPS_MUL; }
		uint32 GetLogicTimer() const
		{ return state.logicTimer; }

		struct Notify
		{
			string * msg;
			uint duration;
			int priority;
		};
		List<Notify> notifications;
		string * currentNotifyMessage;
		uint32 currentNotifyTimer;
		uint32 currentNotifyTimerMax;
		int  currentNotifyWidth;
		void ShowNotifyMessage(const string & _msg, float _duration = 2.0f, int _priority=5);

		static void MakeGameBorderTex(Image & _pic, uint _offset_x, uint _offset_y, uint _w, uint _h);

		//void ClearTexPool(bool _force_clear_all);
		Tex * LoadTex(const string & _name, bool _delay = false);
		static TexLoader * _TryLoadEmbededTex(void * _context, const string & _name);

		TexLoader * TryLoadEmbededTex(const string & _name);
		Ani LoadAni(const string & _name);
		Ani5 LoadAni5(const string & _name);
		//FSRef ResFS() const;
		Path ResFolder() const;
		Path BgmFolder() const;

		int OnLuaError(void * _p);

		void AddFloatText(float _x, float _y, FontRef _font, const string & _s, uint _life, float _speed, float _r, float _g, float _b, float _a);
		void AddFloatText(float _x, float _y, FontRef _font, const string & _s, uint _life, float _speed, const RGBA & _color)
		{ AddFloatText(_x, _y, _font, _s, _life, _speed, _color.r, _color.g, _color.b, _color.a); }

		void CaptureScreenShot();
		void GenerateShotVelTable();

		Enemy * FindNearestEnemy(const Vector2 & _pos, float _min_dis = 0);
		int FindSomeNearEnemy(Enemy * ret[], int _n, const Vector2 & _pos, float _min_dis = 0);

		static string FormatScore(float _score);
	};
}
