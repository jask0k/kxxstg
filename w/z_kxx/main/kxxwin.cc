
// #include "../config.h"
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/main/options.h>
#include <z_kxx/ui/ui_base.h>
#include <z_kxx/ui/main_menu.h>
#include <z_kxx/ui/options_menu.h>
#include <z_kxx/ui/player_menu.h>
#include <z_kxx/ui/player_menu.h>
#include <z_kxx/ui/pause_menu.h>
#include <z_kxx/ui/ending_screen.h>
#include <z_kxx/ui/credits_screen.h>
#include <z_kxx/ui/confirm_menu.h>
#include <z_kxx/ui/replay_slot_menu.h>
#include <z_kxx/ui/stage_menu.h>
#include <z_kxx/ui/top_score_screen.h>
#include <z_kxx/player/player.h>
#include <z_kxx/player/player_b.h>
#include <z_kxx/player/player_a.h>
#include <z_kxx/stage/1/stage_1.h>
#include <z_kxx/stage/2/stage_2.h>
#include <z_kxx/stage/3/stage_3.h>
#include <z_kxx/stage/4/stage_4.h>
#include <z_kxx/stage/5/stage_5.h>
#include <z_kxx/stage/6/stage_6.h>
#include <z_kxx/stage/7/stage_7.h>
#include <z_kxx/boss/boss.h>
#include <z_kxx/stage/kxx_dialog.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/util/render_list.h>
#include <z_kxx/shot/uniform_shot.h>
#include <z_kxx/shot/accelerate_shot.h>
#include <z_kxx/effect/lens_effect.h>
#include <z_kxx/effect/simple_fade.h>
#include <z_kxx/effect/zizo_fade.h>
#include <z_kxx/sprite/spark1.h>
#include <z_kxx/ex/lua_support.h>
#include <z_kxx/util/proc_image.h>
#include <nbug/core/env.h>
#include <nbug/core/time.h>
#include <nbug/core/thread.h>
#include <z_kxx/sprite/float_text.h>
#include <z_kxx/enemy/frozen_block.h>
#include <z_kxx/effect/pass2.h>
#include <z_kxx/shot/debug_shot.h>
#include <z_kxx/sprite/corpse.h>
#include <z_kxx/sprite/boss_spread.h>
#include <z_kxx/ui/digital_roller.h>


namespace e
{
	double g_startup_time = 0;

	const char * GetCompiledDate_yymmdd()
	{
		static bool shortYear = true;
		static char _buf[11] = {0};
		if(_buf[0] == 0)
		{
			static const char * _month[] =
			{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

			int a = shortYear ? 0 : 2;
			const char * _date = __DATE__;
			int month = 0;
			for(int i = 0; i < 12; i++)
			{
				if(memcmp(_month[i], _date, 3) == 0)
				{
					month = i+1;
					break;
				}
			}
			int year = 0;
			int day  = 0;
			sscanf(_date+3, " %d %d", &day, &year);
			year = year % 100;
			sprintf(_buf, "%02d.%02d.%02d", year, month, day);
		}

		return _buf;
	}

	float g_kxx_shot_vel[100] = {0};

	RenderList * rlist = 0;
	Graphics * graphics = 0;
	KxxWin * kxxwin = 0;

	float shot_vel(int _n)
	{
		E_ASSERT(_n >= 0 && _n < 100);
		E_ASSERT(kxxwin->IsInGame());
		return g_kxx_shot_vel[_n];
	}

	string KxxWin::FormatScore(float _score)
	{
		return string::format(L" %03d,%03d,%03d"
				, int(_score / 1000000)
				, int(_score / 1000) % 1000
				, int(_score) % 1000);
	}

	const Char * KxxWin::GetLevelText(int _index)
	{
		// NB_PROFILE_INCLUDE;
		static const Char * _table[4] =
		{
	//		L"Easy",
			L"Easy",
			L"Normal",
			L"Hard",
			L"?",
		};
		return _table[_index];
	}
	
	const char * KxxWin::GetPlayerShortName(int _index)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_index >= 0 && _index < 3);
		static const char * _table[3] =
		{
			"player-a",
			"player-b",
			"player-c",
		};
		return _table[_index];
	}

	const Char * KxxWin::GetPlayerName(int _index)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_index >= 0 && _index < 3);
		static const Char * _table[3] =
		{
			L"PlayerA",
			L"PlayerB",
			L"Kanatayuri Tokie",
		};
		return _table[_index];
	}

	SOUND_EFFECT_QUOTA::SOUND_EFFECT_QUOTA()
	{
		memset(grid, 0, sizeof(grid));
	}

	void SOUND_EFFECT_QUOTA::Step()
	{
		uint32 * p = &grid[0][0];
		uint32 * end = p + SE_MAP_W * SE_MAP_C;
		for(;p != end; p++)
		{
			if(*p)
			{
				(*p)--;
			}
		}
	}

	bool SOUND_EFFECT_QUOTA ::TryPlay(float _x)
	{
		int x = (int)(_x * SE_MAP_W / K_GAME_W);
		
		if(x < 0)
		{
			x= 0;
		}
		else if(x >= SE_MAP_W)
		{
			x = SE_MAP_W-1;
		}
		for(int i = 0; i<SE_MAP_C; i++)
		{
			uint32 & n = grid[x][i];
			if(n == 0)
			{
				n = 8;
				return true;
			}
		}
		return false;
	}

	void KxxWin::TryOpenJoystick()
	{
		if(!joystick.Ready() && joystick.OpenDefault())
		{
			message(L"[kx] Joystick is ready.");
			joystick.Map(Joystick::JS_BTN_L, MAPED_BUTTON_LEFT);
			joystick.Map(Joystick::JS_BTN_R, MAPED_BUTTON_RIGHT);
			joystick.Map(Joystick::JS_BTN_U, MAPED_BUTTON_UP);
			joystick.Map(Joystick::JS_BTN_D, MAPED_BUTTON_DOWN);

			Joystick::Button btn[8] = {Joystick::JS_BTN_0, Joystick::JS_BTN_1, Joystick::JS_BTN_2,
				Joystick::JS_BTN_3, Joystick::JS_BTN_4, Joystick::JS_BTN_5, Joystick::JS_BTN_6, Joystick::JS_BTN_7};
			int n;
			n = options->joystickPause;
			if(n >= 0 && n <= 7)
			{
				joystick.Map(btn[n], MAPED_BUTTON_PAUSE);
			}
			else
			{
				message(L"[kx] (WW) Invalid joystick button id : joystickPause = " + string(n));
			}
			
			n = options->joystickSC;
			if(n >= 0 && n <= 7)
			{
				joystick.Map(btn[n], MAPED_BUTTON_SC);
			}
			else
			{
				message(L"[kx] (WW) Invalid joystick button id : joystickSC = " + string(n));
			}
			
			n = options->joystickSlow;
			if(n >= 0 && n <= 7)
			{
				joystick.Map(btn[n], MAPED_BUTTON_SLOW);
			}
			else
			{
				message(L"[kx] (WW) Invalid joystick button id : joystickSlow = " + string(n));
			}
			
			n = options->joystickFire;
			if(n >= 0 && n <= 7)
			{
				joystick.Map(btn[n], MAPED_BUTTON_FIRE);
			}
			else
			{
				message(L"[kx] (WW) Invalid joystick button id : joystickFire = " + string(n));
			}
		}
	}

	KxxWin::KxxWin(KxxOptions * _options)
	{
		digitalRoller = 0;
		jostick_icon_x = 2;
		blurScreenImage = 0;
		
		isPractice = false;
		isReplaying = false;
		isDemoReplay = false;
		isSilient = false;
		isExStart = false;
		isPause = false;
		isWindowVisible = false;
		isWindowActive = false;

		demoReplayTimer = 0;
		demo_id = rand() % 5;

//		texPathIndexer = 0;
		ani5PathIndexer = 0;
		aniPathIndexer = 0;
		currentNotifyMessage = 0;
		currentNotifyTimer = 0;
		currentNotifyWidth = 0;
//		graphicsQuality = 0.5f;
		pass2 = 0;
		lensEffect = 0;
		fade = 0;
		options = _options;
		// init general variable
		memset(&state, 0, sizeof(state));
		backupWindowSize = options->windowSize;
		stage = 0;
		//isRenderingToMemory = false;
		uiToDelete = 0;
		currentUI = 0;
		lastMainMenuItem = 0;
		player = 0;
		state.earthQuakeTimer = 0;
		state.darkScreenTime0 = 0;
		state.darkScreenTimer = 0;
		audio = 0;
//		playEnemyDamageSound = 0;
		isReplaying = false;
		state.level = 0;
		state.playingPlayer = 0;
		state.initialRandomSeed = 0;
		state.replayPosition = 0;
		highScoreCopy = 0;
		memset(&combinedPersistData, 0, sizeof(combinedPersistData));
		state.onceContinued = false;
		state.onceRestartStage = false;
		state.onceCheated = false;
//		state.bossSCAvatarTimer = 0;
//		state.playerSCAvatarTimer = 0;
		state.replayStopPosition = 0;
		state.delay_clear_enemy_shot_timer = 0;
		state.delay_clear_enemy_shot_absorb = false;

		practiceStageID = 0;

		blurScreenSteps = 0;

		TryOpenJoystick();
		keyboard.Map(Keyboard::Left,      MAPED_BUTTON_LEFT);
		keyboard.Map(Keyboard::Right,     MAPED_BUTTON_RIGHT);
		keyboard.Map(Keyboard::Up,        MAPED_BUTTON_UP);
		keyboard.Map(Keyboard::Down,      MAPED_BUTTON_DOWN);
		keyboard.Map(Keyboard::Z,         MAPED_BUTTON_FIRE);
		keyboard.Map(Keyboard::X,         MAPED_BUTTON_SC);
		keyboard.Map(Keyboard::LeftShift, MAPED_BUTTON_SLOW);
		keyboard.Map(Keyboard::Esc,       MAPED_BUTTON_PAUSE);


#ifdef NB_DEBUG
		debugMode = 0;
	//	debugSpriteCount = 0;
//		functionKeyDownTime = 0;
		debugFastForward = false;
		debugFastForwardSpeed = 3;
		debugSmallWindow = false;
		debugFont = 0;
		debugRenderOjbectCount = 0;
		debugRenderVertexCount = 0;
		debugTexSwitchCount = 0;
		debugMaterialSwitchCount = 0;

		debugCount_rlist = 0;
		debugCount_logicStepList = 0;
		debugCount_enemyList = 0;
		debugCount_playerShotList = 0;
		debugCount_playerSCList = 0;
		debugCount_enemyShotList = 0;
		debugCount_sparkList = 0;
		debugCount_uiSpriteList = 0;
		debugCount_dropList = 0;
		debugShotType = 0;
		debugShotColor = 0;
#endif

		defaultFont = 0;
		smallFont = 0;
		rlist = enew RenderList();
	}

	template <typename T> void DeleteAllElement(T & _container)
	{
		for(typename T::iterator it = _container.begin(); it != _container.end(); it = _container.erase(it))
		{
			delete *it;
		}
	}


	KxxWin::~KxxWin()
	{
		// NB_PROFILE_INCLUDE;
		if(pass2)
		{
			delete pass2;
			pass2 = 0;
		}
		if(blurScreenImage)
		{
			blurScreenImage->Release();
			blurScreenImage = 0;
		}

		for(List<Notify>::iterator it = notifications.begin(); it != notifications.end(); ++it)
		{
			delete it->msg;
		}
		notifications.clear();

		delete currentNotifyMessage;

		int x, y;
		if(GetPosition(x, y))
		{
			options->windowLeft = x;
			options->windowTop = y;
			options->Save();
		}
		//delete joystick;
		delete audio;
		delete options;
		
		ClearKxxSpriteLists();
	
		delete lensEffect;
		lensEffect = 0;
		delete fade;
		fade = 0;
		DeleteStageAndPlayers();
		delete currentUI;
		currentUI = 0;
		delete uiToDelete;
		uiToDelete = 0;
		DeleteAllElement(uiSpriteList);
		defaultFont = 0;
		smallFont = 0;

#ifdef NB_DEBUG
		debugFont = 0;
#endif


#ifdef E_CFG_LUA
		// lua must close before destroy rlist and after destory sprite list
		L.Close(); 
#endif

		delete rlist;
		rlist = 0;


		E_SAFE_DELETE(aniPathIndexer);
//		E_SAFE_DELETE(texPathIndexer);
		E_SAFE_DELETE(ani5PathIndexer);

		delete digitalRoller;

		kxxwin = 0;
	}

	bool KxxWin::Create()
	{
		// NB_PROFILE_INCLUDE;
		WinFormat format;
		format.allowResize = true;

		format.x = options->windowLeft;
		format.y = options->windowTop;
		switch(options->windowSize)
		{
		case 0:
		default:
			format.w = K_VIEW_W;
			format.h = K_VIEW_H;
			break;
		case 1:
			format.w = K_VIEW_W*2;
			format.h = K_VIEW_H*2;
			break;
		}

		if(!Win::Create(format, K_WIN_UNIQUE_PROP))
		{
			return false;
		}

		osNameVersion = Env::GetOSName();
		message(L"[nb] OS: " + osNameVersion);
		appNameVersion = Translate(L"KXX01") + L" " + string(GetCompiledDate_yymmdd());
#ifdef NB_DEBUG
		appNameVersion+= options->graphicsBackend == 0 ? L" (GL)" : L" (DX)";
#endif
		SetTitle(appNameVersion);
		
		GraphicsFormat gf;
		gf.vsync = options->vsync;
		graphics = Win::CreateGraphics(gf);
		if(graphics == 0)
		{
			Win::SafeDeleteThis();
			return false;
		}

		if(!FS::IsFolder(ResFolder()))
		{
			message(L"[kx] (WW) Failed to load resouces pack \"res_\"");
			ShowNotifyMessage(L"Error: Failed to load resouces pack", 4.0f, 3);
		}

		InitAudio();
		InitGraphics();
		LoadPersistData();

#ifdef E_CFG_LUA
		InitLua();
#endif

		SwitchToMainMenu(true, FADE_NONE);

#ifdef NB_DEBUG
		{
			g_startup_time = Time::GetTicks() - g_startup_time;
			int ist = (int)(g_startup_time * 1000);
			string s = _TT("Start up cost") + L": " + string(ist) + L"ms.";
		//	ShowNotifyMessage(s);
			message(L"[kx] " + s);
			ShowNotifyMessage(_TT("Press F1 to visit our web site."));
		}
#endif


		return true;
	}

	void KxxWin::InitAudio()
	{
		audio = 0;
		Array<stringa> backends;
		APlayer::GetAvailableBackends(backends);
		if(backends.empty())
		{
			this->ShowNotifyMessage("Audio disabled.");
			audioDriverType = L"(Unavailable)";		
			return;
		}

		try
		{
			audio = enew APlayer(options->audioBackend, ResFolder() | L"se", options->audioBufMsec);
		}
		catch(const char * _exp)
		{
			message(L"[kx] " + string(L"Audio failed: ") + _exp);
			this->ShowNotifyMessage(string(L"Audio failed: ") + _exp);
			if(backends[0].icompare(options->audioBackend) != 0)
			{
				try
				{
					audio = enew APlayer(backends[0], ResFolder() | L"se", options->audioBufMsec);
					this->ShowNotifyMessage(L"Fallback to default audio: " + backends[0]);
				}
				catch(const char * _exp)
				{
					message(L"[kx] (WW) Failed to fallback to default audio: " + string(_exp));
					// this->ShowNotifyMessage(string(L"Failed to fallback to default audio: ") + _exp);
				}
			}
		}

		if(audio)
		{
			audio->SetGain(options->musicVolume * 0.1f, options->soundVolume * 0.1f);
			//audio->SetMixerGain(options->mixerFrac);
			audioDriverType = audio->GetBackendName();
			message(L"[kx] Audio engine: " + audioDriverType);
			audio->SetReverb(options->audioReverbLevel * 5);
		}
	}

	TexLoader * KxxWin::_TryLoadEmbededTex(void * _context, const string & _name)
	{
		KxxWin * w = (KxxWin*) _context;
		return w->TryLoadEmbededTex(_name);
	}

	void KxxWin::InitGraphics()
	{
		// NB_PROFILE_INCLUDE;
		graphics->SetErrorCallback(Callback(this, &KxxWin::OnGraphicsError));
	//	graphics->SetLoadTexCallback(this, &_LoadTex);

		graphicsHardwareString = graphics->GetHardwareString();
		graphicsSoftwareString = graphics->GetSoftwareString();
		message(L"[kx] Graphics Card: " + graphicsHardwareString);
		message(L"[kx] Graphics Driver: " + graphicsSoftwareString);

		graphics->InitTexPool(ResFolder(), this, &_TryLoadEmbededTex);
		ani5PathIndexer = enew PathIndexer(ResFolder(), L"*.an5");
		aniPathIndexer = enew PathIndexer(ResFolder(), L"*.an");

		graphics->SetClearColor(0, 0, 0, 1);
		graphics->SetColor(1, 1, 1, 1);
		graphics->SetTexMode(TM_MODULATE);
		graphics->SetBlendMode(BM_NORMAL);

		graphics->CalcOrtho2D(projectionMatrixUI, 0, K_VIEW_W, K_VIEW_H, 0);
		modelViewMatrixUI.LoadIdentity();
		graphics->CalcOrtho2D(projectionMatrixGame2D, 0, K_GAME_W, K_GAME_H, 0);
		modelViewMatrixGame2D.LoadIdentity();

		InitFonts();
		InitSprites();

		pass2 = enew Pass2();
		if(!pass2->Init(graphics, K_VIEW_W, K_VIEW_H))
		{
			delete pass2;
			pass2 = 0;
		}
	}

	void KxxWin::InitFonts()
	{
		// NB_PROFILE_INCLUDE;
		defaultFont = 0;
		const string & fontName = options->fontName;
		E_ASSERT(!fontName.empty());
		defaultFont = graphics->LoadFont(options->fontName, options->fontSize);

		if(!defaultFont)
		{
			message(L"[kx] (WW) Fail to load fonts: " + options->fontName);
		}

		defaultFontColor = MakeColor(240, 240, 240);

		Path path = ResFolder() | string(L"font") | string(L"small");
		smallFont = graphics->LoadFont(path.GetString(), 10);
		if(!smallFont)
		{
			message(L"[kx] (WW) Fail to load fonts: " + path.GetBaseName(true));
		}

#ifdef NB_DEBUG
		{
#	ifdef NB_WINDOWS
			debugFont = graphics->LoadFont(L"Courier New", 11);
#	else
			debugFont = graphics->LoadFont(L"Monospace", 11);
#	endif
			if(!debugFont)
			{
				message(L"[kx] (WW) Fail to load debug fonts.");
			}
			debugFontHSL.h = 0;
			debugFontHSL.s = 0.75f;
			debugFontHSL.l = 0.75f;
			debugFontHSL.a = 1.0f;
			debugFontColor = MakeColor(0, 0, 0);;
		}
#endif
	}

#ifdef E_CFG_LUA
	int KxxWin::OnLuaError(void * _p)
	{
		report_error(L.GetLastError());
		exit(1);
	}
	void KxxWin::InitLua()
	{
		L.SetErrorCallback(Callback(this, &KxxWin::OnLuaError));

		if(!RegisterForLua(L))
		{
			message(L"[kx] (WW) Fail to expose to script.");
			exit(1);
		}

		L.Load(ResFolder() | L"lua");

		lua_getglobal(L, "Init");
		L.Call();

	}
#endif

	void KxxWin::MakeGameBorderTex(Image & _pic, uint _offset_x, uint _offset_y, uint _w, uint _h)
	{
		_pic.Alloc(_w, _h);
		for(uint x = 0; x < _w; x++)
		{
			for(uint y = 0; y < _h; y++)
			{
				uint ox = _offset_x + x;
				uint oy = _offset_y + y;
				uint8 * p = _pic.Get(x, y);
				if(((ox + oy) & 0x3) == 0 || ((ox - oy) & 0x3) == 0)
				{
					p[0] = 96;
					p[1] = 96;
					p[2] = 96;
					p[3] = 255;
				}
				else
				{
					p[0] = 128;
					p[1] = 96;
					p[2] = 160;
					p[3] = 255;
				}
			}
		}
	}

	TexLoader * KxxWin::TryLoadEmbededTex(const string & _name)
	{
		TexLoader * loader = 0;

		if(_name == "gather-star")
		{
			loader = enew ProcImageStar(32, 32);
		}
		else if(_name == "player-b-laser")
		{
			//loader = enew ProcImageLaser(16, 32, MakeColor(0, 0, 255), true);
			loader = enew ProcImageGradientDisc(40, 40, MakeColor(96,96,96,255), MakeColor(96,96,96,0), false, 3.0f);
		}
		else if(_name == "laser-blue")
		{
			loader = enew ProcImageLaser(16, 32, MakeColor(0, 0, 255), false);
		}
		else if(_name == "laser-red")
		{
			loader = enew ProcImageLaser(16, 32, MakeColor(255, 0, 0), false);
		}
		else if(_name == "blast-wave")
		{
			loader = enew ProcImageBlastWave(64, 64);
		}
		else if(_name == "flash")
		{
			loader = enew ProcImageGradientDisc(64, 64, MakeColor(127,127,127,255), MakeColor(127,127,127,0));
		}
		else if(_name == "dark-disc")
		{
			loader = enew ProcImageGradientDisc(64, 64, MakeColor(0,0,0,255), MakeColor(0,0,0,0), false, 2.0f);
		}
		else if(_name == "particle32")
		{
			loader = enew ProcImageGradientDisc(32, 32, MakeColor(255,255,255,255), MakeColor(255,255,255,0));
		}
		else if(_name == "explosion-0")
		{
			loader = enew ProcImageStar(64, 64);
		}
		else if(_name == "player-aura")
		{
			loader = enew ProcImagePlayerAura(96, 96, MakeColor(255,255,255,64));
		}
		else if(_name == "enemy-aura-1")
		{
			loader = enew ProcImageGradientDisc(128, 128, MakeColor(0, 40, 40,192), MakeColor(0, 40, 40,0), false, 2.0f);
		}
		else if(_name == "player-a-ring" || _name == "player-b-ring")
		{
			loader = enew ProcImageStripA();
		}
		else if(_name == "boss-a-ring" || _name == "boss-b-ring")
		{
			loader = enew ProcImageStripLing();
		}

		if(loader)
		{
			message(L"[kx] Load embedded tex: \"" + _name + L"\"");
		}

		return loader;
	}

	void KxxWin::InitSprites()
	{
		// NB_PROFILE_INCLUDE;
		digitalRoller = enew DigitalRoller();

		fairyBlueAni5 = this->LoadAni5(L"fairy-blue");
		fairyRedAni5 = this->LoadAni5(L"fairy-red");
		kedamaLeftAni = this->LoadAni(L"kedama-left");
		kedamaRightAni = this->LoadAni(L"kedama-right");
		kedamaPonderLeftAni = this->LoadAni(L"kedama-ponder-left");
		kedamaPonderRightAni = this->LoadAni(L"kedama-ponder-right");

		kedamaFall0Right = kxxwin->LoadTex("kedama-3");
		kedamaFall1Right = kxxwin->LoadTex("kedama-4");

		kedamaFall0Left = kedamaFall0Right->Clone();
		kedamaFall0Left->flip_x = true;
		kedamaFall1Left = kedamaFall1Right->Clone();
		kedamaFall1Left->flip_x = true;

		fire_sputter_tex = kxxwin->LoadTex("fire-sputter", true);
		shotCrashInnerTex = kxxwin->LoadTex("shot-crash-inner", true);
		uiBgTex = kxxwin->LoadTex("ui-bg", true);
		badTex = kxxwin->LoadTex("bad-tex", true);
		frozenTex = kxxwin->LoadTex("frozen");
		gamepadTex = kxxwin->LoadTex("gamepad", true);
//		redMoonTex = kxxwin->LoadTex("moon_red");
		playerAruaTex = kxxwin->LoadTex("player-aura", true);
		sparkTex = kxxwin->LoadTex("gather-star");;
		flashTex  = kxxwin->LoadTex("flash");
//		wormTex   = kxxwin->LoadTex("worm");
		shotSmokeTex0 = kxxwin->LoadTex("shot-smoke-0");
		enemyAuraTex = kxxwin->LoadTex("enemy-aura-1");
//		itemGetLineTex = kxxwin->LoadTex("item_get_line");
		blastWaveTex = kxxwin->LoadTex("blast-wave", true);
		explosionTex = kxxwin->LoadTex("explosion-0");
#if KXX_GAME_MARGIN
		borderLeftTex   = kxxwin->LoadTex("border-left", true);
		borderRightTex  = kxxwin->LoadTex("border-right", true);
		borderTopTex    = kxxwin->LoadTex("border-top", true);
		borderBottomTex = kxxwin->LoadTex("border-bottom", true);
#endif

		{
			stringa s;
			for(int i = 0; i < K_SHOT_TYPE_COUNT; i++)
			{
				for(int h = 0; h < K_SHOT_COLOR_COUNT; h++)
				{
					s = stringa("shot-") + stringa(i);
					s.append((char)('a' + h));
					shotTex[i][h] = kxxwin->LoadTex(s, true);
				}
			}
		}

		float d512 = 1.0f / 512.0f;
		
		statusLifeTex = kxxwin->LoadTex("status-life", true);
		statusSCTex   = kxxwin->LoadTex("status-sc");

		//dropTex[DROP_EC_A]        = kxxwin->LoadTex("drop_ec_a");
		dropTex[DROP_EC_B]        = kxxwin->LoadTex("drop-ec-b");
		dropTex[DROP_EC_C]        = kxxwin->LoadTex("drop-ec-c");
		dropTex[DROP_EC_D]        = kxxwin->LoadTex("drop-ec-d");
		dropTex[DROP_LIFE]    = kxxwin->LoadTex("drop-life-20");
		dropTex[DROP_BIG_POWER]   = kxxwin->LoadTex("drop-power-20");
		dropTex[DROP_SC]      = kxxwin->LoadTex("drop-sc-20");
		dropTex[DROP_PET]     = kxxwin->LoadTex("drop-pet-20");
		dropTex[DROP_FULL]    = kxxwin->LoadTex("drop-full-20");
		dropTex[DROP_SMALL_POWER] = kxxwin->LoadTex("drop-power-16");
		dropTex[DROP_SMALL_POINT] = kxxwin->LoadTex("drop-point-16");
		dropTex[DROP_TINY_POINT]  = kxxwin->LoadTex("drop-point-12");

		//dropIndicatorTex[DROP_EC_A]        = kxxwin->LoadTex("indicator_ec_a");
		dropIndicatorTex[DROP_EC_B]        = kxxwin->LoadTex("indicator-ec-b");
		dropIndicatorTex[DROP_EC_C]        = kxxwin->LoadTex("indicator-ec-c");
		dropIndicatorTex[DROP_EC_D]        = kxxwin->LoadTex("indicator-ec-d");
		dropIndicatorTex[DROP_LIFE]    = kxxwin->LoadTex("indicator-life-20");
		dropIndicatorTex[DROP_BIG_POWER]   = kxxwin->LoadTex("indicator-power-20");
		dropIndicatorTex[DROP_SC]      = kxxwin->LoadTex("indicator-sc-20");
		dropIndicatorTex[DROP_PET]     = kxxwin->LoadTex("indicator-pet-20");
		dropIndicatorTex[DROP_FULL]    = kxxwin->LoadTex("indicator-full-20");
		dropIndicatorTex[DROP_SMALL_POWER] = kxxwin->LoadTex("indicator-power-16");
		dropIndicatorTex[DROP_SMALL_POINT] = kxxwin->LoadTex("indicator-point-16");
		dropIndicatorTex[DROP_TINY_POINT]  = kxxwin->LoadTex("indicator-point-12");
	}

	void KxxWin::ClearKxxSpriteLists()
	{
		// NB_PROFILE_INCLUDE;
		DeleteAllElement(logicStepList);
		DeleteAllElement(playerShotList);
		DeleteAllElement(playerSCList);
		DeleteAllElement(enemyShotList);
		DeleteAllElement(sparkList);
		DeleteAllElement(dropList);
		DeleteAllElement(enemyList);
	}
	
	void KxxWin::PollJoystickState()
	{
		// poll keyboard state
		if(IsWindowActiveAndVisible())
		{
			uint oldKeyState = state.joystickState[0];
			uint & newKeyState = state.joystickState[0];
			keyboard.Poll(newKeyState);

			if(joystick.Ready())
			{
				uint newKeyState1;
				joystick.Poll(newKeyState1);
				newKeyState|= newKeyState1;
			}

			if(newKeyState != oldKeyState)
			{
				//E_TRACE_LINE(string(Time::GetTicks()));
				for(int i=0; i< 8; i++)
				{
					uint mask = 0x0001 << i;
					if((newKeyState & mask) != (oldKeyState & mask))
					{
						if(newKeyState & mask)
						{
							OnJoystickDown(0, mask);
						}
						else
						{
							OnJoystickUp(0, mask);
						}
					}
				}
			}
		}
	}

	bool KxxWin::DemoFastForward(uint32 _time)
	{
		while(state.logicTimer < _time &&  IsLogicActive())
		{
			StepGameLogic(1);
		}
		return state.logicTimer >= _time && IsLogicActive();
	}

	void KxxWin::StepGameLogic(int _frames)
	{
		for(int i=0; i<_frames; i++)
		{
			if(pass2)
			{
				pass2->Step();
			}
			
			PollJoystickState();
			
			uint32 mod = state.logicTimer % K_LOGIC_FPS_MUL;
			uint32 loop = mod ? mod : 4;

			while(loop-- && IsLogicActive())
			{
				LogicStep();
			}
			StepGeneral();
		}
	}

	void KxxWin::StepLensEffect()
	{
		Boss * firstBoss;
		if(stage && (firstBoss = stage->GetFirstBoss()) && firstBoss->is_show_aura)
		{
			if(!lensEffect && options->graphicsEffect > 0 && graphics->GetCapabilities().frameBufferObject)
			{
				lensEffect = enew LensEffect();
				lensEffect->Init(graphics, firstBoss->pos.x, firstBoss->pos.y, K_VIEW_W, K_VIEW_H);
			}
			if(!isPause && lensEffect)
			{
				lensEffect->Step(firstBoss->pos.x, firstBoss->pos.y);
			}
		}
		else
		{
			if(lensEffect)
			{
				delete lensEffect;
				lensEffect = 0;
			}
		}
	}

	int KxxWin::CalcFastForwardAccel()
	{
		int accel = 1;
#ifdef NB_DEBUG
		{
			// support fast forward
			if(kxxwin->debugFastForward)
			{
				int limit = isReplaying ? 9 : 60;
				if(debugFastForwardSpeed < limit)
				{
					debugFastForwardSpeed++;
				}
				if(player && !isReplaying)
				{
					player->state.transparentTimer = S2F(4);
				}
			}
			else
			{
				if(debugFastForwardSpeed > 5)
				{
					debugFastForwardSpeed-= 3;
				}
				else if(debugFastForwardSpeed > 3)
				{
					debugFastForwardSpeed--;
				}
			}
			accel = debugFastForwardSpeed / 3;
		}
#endif
		return accel;
	}

	void KxxWin::StepTimers()
	{
		int thisCycleRenderedFrames = fpsCalc.GetThisCycleFrame();

		if(thisCycleRenderedFrames == 0)
		{
			OnTimer1000ms();
		}
		if(thisCycleRenderedFrames % 12 == 0)
		{
			OnTimer200ms();
		}
		if(thisCycleRenderedFrames % 6 == 0)
		{
			OnTimer100ms();
		}
	}

	void KxxWin::OnRealTime(bool _busy)
	{
		FpsCalculator::State s = fpsCalc.Step();

		if(s == FpsCalculator::stateIdle)
		{
			if(!_busy)
			{
				Sleep(1);
			}
			return;
		}

		if(!graphics->IsDeviceOperational()
			&& !graphics->TryRestoreDevice())
		{
			return;
		}

		StepNonlogic();
		int accel = CalcFastForwardAccel();
		StepGameLogic(accel);
		if(s == FpsCalculator::stateRender)
		{
			RenderRoot();
		}
	}

	void KxxWin::LogicStep()
	{

#ifdef E_CFG_LUA
		lua_gc(L, LUA_GCCOLLECT, 0); // TODO: shall we call it here?
#endif

		state.logicTimer++;
		
		if(isReplaying)
		{
			bool is_demo_quit = isDemoReplay && state.logicTimer >= demoReplayTimer;

			// ignore user input when replaying, also don't emit OnJoystickDown/OnJoystickUp event
			if(is_demo_quit || !PopReplayKey())
			{
				int n = stage->stage_index;
				if(!is_demo_quit && player->state.curScore != state.stageHighScore)
				{
					message(L"[kx] (WW) player->state.curScore(" + string(player->state.curScore) + L") != state.stageHighScore(" + string(state.stageHighScore) + L").");
				}
				OnQuitReplay();
				return;
			}
		}
		else
		{
			PushReplayKey();
		}


		E_ASSERT(!isPause);
		E_ASSERT(stage != 0);
		if(!stage->Step())
		{
			E_ASSERT(!isPause);
			this->OnStagePassed();
			return;
		}

		if(state.earthQuakeTimer)
		{
			state.earthQuakeTimer--;
		}

		if(state.darkScreenTimer)
		{
			state.darkScreenTimer--;
		}

		if(state.delay_clear_enemy_shot_timer)
		{
			if(--state.delay_clear_enemy_shot_timer == 0)
			{
				this->ClearEnemyShots(state.delay_clear_enemy_shot_absorb, 0);
			}
		}

		E_ASSERT(!isPause);
		StepLogicSteps();
		E_ASSERT(!isPause);
		StepEnemies();
		E_ASSERT(!isPause);
		StepEnemyShots();
		E_ASSERT(!isPause);
		StepSparks();
		E_ASSERT(!isPause);
		StepDrops();
		E_ASSERT(!isPause);
		StepPlayerShots();
		E_ASSERT(!isPause);
		StepPlayer(); // may change the state, so must put at last
	}


	void KxxWin::StepPlayer()
	{
		// NB_PROFILE_INCLUDE;
		// player stepping
		uint jsState = GetJoystickState(0);

		player->Move(jsState);
		// player shot
		if(stage->dialog)
		{
			stage->dialog->CheckJoystick();
		}
		else
		{
			if((jsState & MAPED_BUTTON_FIRE)/* || player->scTimer*/)
			{
				player->Fire();
			}
			if((jsState & MAPED_BUTTON_SC) && player->CanCastSpellCard())
			{
				player->CastSpellCard();
			}
		}

		player->Step();
	}

	void KxxWin::StepLogicSteps()
	{
		SpriteList::iterator it =logicStepList.begin();
		while(it != logicStepList.end())
		{
			Sprite * p = *it;
			if(p->Step())
			{
				++it;
			}
			else
			{
				delete *it;
				it = logicStepList.erase(it);
			}
		}
	}

	void KxxWin::StepEnemies()
	{
		// NB_PROFILE_INCLUDE;
		// enemy stepping
		List<Enemy *> addedEnemies;
		EnemyList::iterator it = enemyList.begin();
		while(it != enemyList.end())
		{
			Enemy * enemy = *it;
			if(!enemy->Step())
			{
				delete enemy;
				it = enemyList.erase(it);
				continue;
			}

			if(!enemy->IsEthereal())
			{
				if(player->IsCollideWith(enemy))
				{
					AddGenericSpark(enemy->pos, 3, enemy->hue); // TODO: anthor spark style
					AddGenericSpark(enemy->pos, 3, player->hue); // TODO: anthor spark style
					player->state.pendingDie = true;
					enemy->Damage(10, false);
				}

				if(enemy->life > 0)
				{
					// player shot
					PlayerShotList::iterator it1 = playerShotList.begin();
					while(it1 != playerShotList.end())
					{
						PlayerShot * shot = *it1;
						if(shot->unavailableTimer == 0
							&& shot->Collide(enemy->pos, enemy->GetCollisionRadius()))
						{
							shot->OnHit(enemy->pos);
							PlayEnemyDamageSE(enemy->pos);
							FrozenBlock * fb = dynamic_cast<FrozenBlock *>(enemy);

							enemy->Damage(shot->power, shot->puncture);
							if(fb && fb->active && shot->fragile)
							{
								FrozenBlock * newfb = enew FrozenBlock(shot);
								addedEnemies.push_back(newfb);
								it1 = playerShotList.erase(it1);
							}
							else if(shot->fragile)
							{
								shot->OnCrash(enemy->pos);
								delete shot;
								it1 = playerShotList.erase(it1);
							}
							else
							{
								++it1;
							}
						}
						else
						{
							++it1;
						}
					}
				}
				if(enemy->life > 0)
				{
					// player sc
					PlayerShotList::iterator it1 = playerSCList.begin();
					while(it1 != playerSCList.end())
					{
						PlayerShot * shot = *it1;
						if(shot->unavailableTimer == 0
							&& shot->Collide(enemy->pos, enemy->hsz.x))
						{
							//playEnemyDamageSound++;
							PlayEnemyDamageSE(enemy->pos);
							enemy->Damage(shot->power, shot->puncture);
							shot->OnHit(enemy->pos);
							//player->AddPoint(shot->pos, damage * POINT_GAIN_HIT_ENEMY);
							if(shot->fragile)
							{
								shot->OnCrash(enemy->pos);
								delete shot;
								it1 = playerSCList.erase(it1);
							}
							else
							{
								++it1;
							}
						}
						else
						{
							++it1;
						}
					}
				}
			}

			if(enemy->life <= 0 && enemy->OnLifeEmpty())
			{
				// player->AddPoint(enemy->pos, POINT_GAIN_KILL_ENEMY0);
				delete enemy;
				it = enemyList.erase(it);
			}
			else
			{
				++it;
			}
		}

		if(!addedEnemies.empty())
		{
			it = addedEnemies.begin();
			while(it != addedEnemies.end())
			{
				Enemy * enemy = *it;
				this->AddEnemyToList(enemy);
				++it;
			}
		}
	}
	void KxxWin::StepPlayerShots()
	{
		{
			PlayerShotList::iterator it = playerShotList.begin();
			while(it != playerShotList.end())
			{
				PlayerShot * shot = *it;
				if(shot->Step())
				{
					++it;
				}
				else
				{
					delete *it;
					it = playerShotList.erase(it);
				}
			}
		}

		{
			PlayerShotList::iterator it = playerSCList.begin();
			while(it != playerSCList.end())
			{
				PlayerShot * shot = *it;
				if(shot->Step())
				{
					++it;
				}
				else
				{
					delete (*it);
					it = playerSCList.erase(it);
				}
			}
		}
	}

	int KxxWin::StepSingleEnemyShot(EnemyShot * _shot)
	{
		if(!_shot->Step())
		{
			return 2;
		}
			
		// emeny shots collide with sc
		{
			bool toDelete = _shot->dead;
			if(!toDelete)
			{
				PlayerShotList::iterator it1 = playerSCList.begin();
				while(it1 != playerSCList.end())
				{
					PlayerShot * sc = *it1;
					if(_shot->fragile && sc->Collide(_shot->pos, 10))
					{ 
						sc->OnHit(_shot->pos);
						_shot->OnHit(_shot->pos);
						_shot->OnCrash(_shot->pos);
						// TODO: two shots collide with each other?
						toDelete = true;
						break;
					}
					else
					{
						++it1;
					}
				}
			}

			if(toDelete)
			{
				_shot->OnHit(_shot->pos);
				_shot->OnCrash(_shot->pos);
				_shot->DropItem();
				return 2;
			}
		}

			
		// enemy shots collide with player
		if(player->state.transparentTimer==0 
			&& _shot->Collide(player->pos, K_GPOINT_RADIUS))
		{
			_shot->OnHit(_shot->pos);
			player->state.pendingDie = true;
			if(_shot->fragile)
			{
				_shot->OnCrash(_shot->pos);
				return 2;
			}
		}
			
		// graze
		if(player->state.transparentTimer == 0
			&& _shot->unavailableTimer == 0
			&& player->state.dyingTimer == 0
			&& _shot->Collide(player->pos, K_GRAZE_RADIUS + K_GPOINT_RADIUS))
		{
			AddSmallSpark(_shot->pos, 2, player->hue);
			AddSmallSpark(_shot->pos, 2, _shot->hue);
			_shot->unavailableTimer = K_GRAZE_UNAVAILABLE_SPAN;
			player->AddPoint(_shot->pos, POINT_GAIN_GRAZE);
			player->OnGraze();
			return 1;
		}

		return 0;
	}

	void KxxWin::StepEnemyShots()
	{
		// NB_PROFILE_INCLUDE;
		int thisStepGrazeCount = 0;
		EnemyShotList::iterator it = enemyShotList.begin();
		while(it != enemyShotList.end())
		{
			EnemyShot * shot = *it;
			bool toDelete = false;
			for(uint mf = 0; mf < 1; mf++)
			{
				int r = StepSingleEnemyShot(shot);
				if(r == 2)
				{
					toDelete = true;
					break;
				}
				if(r == 1)
				{
					//thisStepGrazeCount++;
					PlaySE("graze", player->pos, 0.2f);
				}
			}

			if(toDelete)
			{
				delete shot;
				it = enemyShotList.erase(it);
			}
			else
			{
				++it;
			}
		}

		if(thisStepGrazeCount)
		{
			if(thisStepGrazeCount > 2)
			{
				thisStepGrazeCount = 2;
			}
//			for(int i=0; i< thisStepGrazeCount; i++)
//			{
//				PlaySE("graze", player->pos, 0.5f);
//			}
		}
	}

	void KxxWin::StepSparks()
	{
		// NB_PROFILE_INCLUDE;
		SpriteList::iterator it = sparkList.begin();
		while(it != sparkList.end())
		{
			Sprite * spark = *it;
			if(spark->Step())
			{
				++it;
			}
			else
			{
				delete spark;
				it = sparkList.erase(it);
			}
		}
	}

	void KxxWin::StepGeneral()
	{
		// NB_PROFILE_INCLUDE;
		SpriteList::iterator it = uiSpriteList.begin();
		while(it != uiSpriteList.end())
		{
			Sprite * p = *it;
			if(p->Step())
			{
				++it;
			}
			else
			{
				delete p;
				it = uiSpriteList.erase(it);
			}
		}
	}


	void KxxWin::AddDrop(int _type, float _x, float _y,  bool _far, float _angle)
	{
		static const float DROP_SPEED = 2.00f / K_LOGIC_FPS_MUL;

		//E_ASSERT(_y > -400 && _y < 1000);
		if(_y < -100 || _y > K_GAME_H + 100 || _x < -100 || _x > K_GAME_W + 100)
		{
			message(L"[kx] (WW) Drop coord out of range: x=" + string(_x)+ L", y="+string(_y));
			return;
		}
		// NB_PROFILE_INCLUDE;
		if(_type < 0 || _type >= _DROP_MAX)
		{
			E_ASSERT(0);
			return;
		}
		Drop * p = enew Drop();
		p->absorbSpeed = 0;
		p->absorbing = false;
		p->dropType = _type;
		p->tex = dropTex[_type];
		p->hsz.x = p->tex->W() * 0.5f;
		p->hsz.y = p->tex->H() * 0.5f;
		p->absorb_mode = Drop::MANUAL_ABSORB;
		p->max_vel = DROP_SPEED;
		switch(_type)
		{
		case DROP_LIFE:
		case DROP_FULL:
			p->tex1 = kxxwin->LoadTex("gather-star");
			break;
		case DROP_TINY_POINT:
			p->absorb_mode = Drop::AUTO_ABSORB;
			break;
		case DROP_EC_A:
		case DROP_EC_B:
		case DROP_EC_C:
		case DROP_EC_D:
			p->absorb_mode = Drop::NO_ABSORB;
			p->max_vel*=0.60f;
			break;
		}

		p->pos.x = (real)int(_x);
		p->pos.y = (real)int(_y);
		const float DROP_SPEED0 = _far ? PS2PF(500) : PS2PF(260);
		p->speed.x = DROP_SPEED0 * cos(_angle);
		p->speed.y = DROP_SPEED0 * sin(_angle);
		p->angle_delta = _far ? 0.073f / 1 : 0.12f / 1;
		AddDropToList(p);
	}

	void KxxWin::StepDrops()
	{
		DropList::iterator it = dropList.begin();
		while(it != dropList.end())
		{
			Drop * drop = *it;
			if(drop->Step())
			{
				++it;
			}
			else
			{
				delete drop;
				it = dropList.erase(it);
			}
		}
	}

	void KxxWin::ClearPlayerShots()
	{
		if(!playerShotList.empty())
		{
			PlayerShotList::iterator it = playerShotList.begin();
			while(it != playerShotList.end())
			{
				PlayerShot * shot = *it;
				//if(shot->OnClean())
				{
					delete shot;
					it = playerShotList.erase(it);
				}
			}
		}
		if(!playerSCList.empty())
		{
			PlayerShotList::iterator it = playerSCList.begin();
			while(it != playerSCList.end())
			{
				PlayerShot * shot = *it;
				//if(shot->OnClean())
				{
					delete shot;
					it = playerSCList.erase(it);
				}
			}
		}	
	}

	void KxxWin::ClearEnemyShots(const Vector2 & _pt, float _radius)
	{
		// NB_PROFILE_INCLUDE;
		if(!enemyShotList.empty())
		{
			//int c = 0;
			float r2 = _radius*_radius;
			EnemyShotList::iterator it = enemyShotList.begin();
			while(it != enemyShotList.end())
			{
				EnemyShot * shot = *it;
				if(shot->fragile && shot->Collide(_pt, _radius))
				{
				//	c++;
					shot->OnHit(shot->pos);
					shot->OnCrash(shot->pos);
					shot->DropItem();
					delete shot;
					it = enemyShotList.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	void KxxWin::FreezePlayerShots(const Vector2 & _pt, float _radius)
	{
		if(!playerShotList.empty())
		{
			int c = 0;
			float r2 = _radius*_radius;
			PlayerShotList::iterator it = playerShotList.begin();
			while(it != playerShotList.end())
			{
				PlayerShot * shot = *it;
				if(shot->fragile && shot->Collide(_pt, _radius))
				{
					c++;
					FrozenBlock * fb = enew FrozenBlock(shot);
					this->AddEnemyToList(fb);
					it = playerShotList.erase(it);
				}
				else
				{
					++it;
				}
			}
			if(c)
			{
				Vector2 v = {K_GAME_XC, K_GAME_YC};
				this->PlaySE("frozen", v);
			}
		}
	}


	void KxxWin::ClearEnemyShots(bool _absorb, uint _delay)
	{
		if(_delay || state.delay_clear_enemy_shot_timer)
		{
			state.delay_clear_enemy_shot_timer+= _delay;
			state.delay_clear_enemy_shot_absorb = state.delay_clear_enemy_shot_absorb || _absorb;
		}
		else
		{
			if(!enemyShotList.empty())
			{
				EnemyShotList::iterator it = enemyShotList.begin();
				while(it != enemyShotList.end())
				{
					EnemyShot * shot = *it;
					if(shot->fragile)
					{
						shot->OnHit(shot->pos);
						shot->OnCrash(shot->pos);
						shot->DropItem();
						delete shot;
						it = enemyShotList.erase(it);
					}
					else
					{
						++it;
					}
				}
				if(_absorb)
				{
					AbsorbAllDrops();
				}
				Vector2 v;
				v.x = K_GAME_XC;
				v.y = K_GAME_YC;
				PlayEnemyDeadSE(v);
			}
		}
	}

//	void KxxWin::OnTimer50ms()
//	{
//		// NB_PROFILE_INCLUDE;
//		//musicPlayer->Step();
//	}

	void KxxWin::OnTimer100ms()
	{

	}

	void KxxWin::OnTimer200ms()
	{
		bool oldVisible = this->isWindowVisible;
		bool oldActive = this->isWindowActive;
//		bool oldMinimize = this->isWindowMinimize;
		bool oldActiveAndVisible = oldVisible && oldActive;

		this->isWindowVisible = IsVisible();
		this->isWindowActive = IsActive();
	//	this->isWindowMinimize = IsMinimize();
		bool newActiveAndVisible = IsWindowActiveAndVisible();

		if(!oldActiveAndVisible && newActiveAndVisible)
		{
			// inactivated => activated
			if(this->IsInGame())
			{
				// do nothing
			}
			else if(audio)
			{
				audio->Resume();
			}
		}
		else if(oldActiveAndVisible && !newActiveAndVisible)
		{
			// activated => inactivated
			if(IsInGame())
			{
				if(!isReplaying && !is_debugger_present())
				{
					Pause();
				}
			}
			else if(audio)
			{
				audio->Pause(); 
			}
		}
	}

	void KxxWin::OnTimer1000ms()
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_DEBUG
		debugRenderOjbectCount = (int)(graphics->DebugRenderOjbectCount(true));
		debugRenderVertexCount = (int)(graphics->DebugRenderVertexCount(true));
		debugTexSwitchCount = (int)(graphics->DebugTexSwitchCount(true));
		debugMaterialSwitchCount = (int)(graphics->DebugMaterialSwitchCount(true));

		debugCount_rlist = rlist->GetSize();
		debugCount_logicStepList = logicStepList.size();
		debugCount_enemyList = enemyList.size();
		debugCount_playerShotList = playerShotList.size();
		debugCount_playerSCList = playerSCList.size();
		debugCount_enemyShotList = enemyShotList.size();
		debugCount_sparkList = sparkList.size();
		debugCount_uiSpriteList = uiSpriteList.size();
		debugCount_dropList = dropList.size();
#endif

		graphics->GetTexPool()->clear(false);
		TryOpenJoystick();
	}
	

	void KxxWin::_RenderUIDebug()
	{
#ifdef NB_DEBUG
		if(this->debugMode == 2)
		{

			graphics->SetColor(debugFontColor);
			graphics->SetTexMode(TM_MODULATE);
			graphics->SetFont(debugFont);

			const float x = 10 + K_GAME_X;
			float y = 30;
			const float dy = 25;

			graphics->DrawString(x, y, L"OS: " + osNameVersion);
			y+= dy;

		//	graphics->SetFont(debugFont);
			graphics->DrawString(x, y, L"Graphics Card: " + graphicsHardwareString);
			y+= dy;

		//	graphics->SetFont(debugFont);
			graphics->DrawString(x, y, L"Graphics Driver: " + graphicsSoftwareString);
			y+= dy;

		//	graphics->SetFont(debugFont);
			graphics->DrawString(x, y,
				L"RLIST:" + string( debugCount_rlist)
				+ L", LO:" + string(debugCount_logicStepList)
				+ L", EN:" + string(debugCount_enemyList)
				+ L", PS:" + string(debugCount_playerShotList)
				+ L", SC:" + string(debugCount_playerSCList)
				+ L", ES:" + string(debugCount_enemyShotList)
				+ L", SP:" + string(debugCount_sparkList)
				+ L", UI:" + string(debugCount_uiSpriteList)
				+ L", DR:" + string(debugCount_dropList)
				);
			y+= dy;

		//	graphics->SetFont(debugFont);
			graphics->DrawString(x, y,string::format(L"Render: %dK P/S, %dK V/S, %d Tex/S, %d Mtl/S"
				, debugRenderOjbectCount/1000
				, debugRenderVertexCount/1000
				, debugTexSwitchCount
				, debugMaterialSwitchCount
				));
			y+= dy;

			float tm = float(graphics->debugTotalTextureMemory) / 1048576.0f;
		//	graphics->SetFont(debugFont); 
			graphics->DrawString(x, y, string::format(L"Textures: %.01fM", tm));
			y+= dy;

		//	graphics->SetFont(debugFont);
			graphics->DrawString(x, y, L"Audio Driver: " + audioDriverType);
			y+= dy;

			if(audio)
			{
				int bgm_count;
				int se_count;
				audio->GetActiveCount(&bgm_count, &se_count);
				string ts = string::format(L"Active Sound: BGM:%d, SE:%d", bgm_count, se_count);
		//		graphics->SetFont(debugFont);
				graphics->DrawString(x, y, ts);
				y+= dy;
			}

		//	graphics->SetFont(debugFont);
		//	graphics->DrawString(x, y, L"Logic FPS: " + string(K_LOGIC_FPS));
		//	y+= dy;

		}

		if(IsInGame())
		{
			graphics->SetColor(debugFontColor);
			graphics->SetTexMode(TM_MODULATE);
			graphics->SetFont(debugFont); 

			string t;
			stage->GetDebugInfo(t);
			graphics->DrawString(K_GAME_W - 140, 2, t);
			float x = 100;
			float dx = 80;
			float y = 2;
			if(state.onceCheated)
			{
				//graphics->SetFont(debugFont); 
				graphics->DrawString(x, y, L"Cheat");
			}
			x+= dx;

			if(debugFastForwardSpeed > 3)
			{
				//graphics->SetFont(debugFont); 
				graphics->DrawString(x, y, L">> " + string(this->debugFastForwardSpeed/3));
			}
			x+= dx;

			//graphics->SetFont(debugFont); 
			graphics->DrawString(x, y, string::format(L"%.0f APM", GetCurrentAPM()));
		}
		// graphics->SetTexMode(TextureMode::replace);
#endif
	}

	class ScopeEarthQuake
	{
		Vector2 gameAreaOffset;
	public:
		ScopeEarthQuake(uint32 _t)
		{
			if(_t)
			{
				float a = _t * 1.8f;
				float r = _t * 0.1f;
				if(r > 8)
				{
					r = 8;
				}
				gameAreaOffset.x = 0 + r * cos(a);
				gameAreaOffset.y = 0 + r * sin(2*a);
			}
			else
			{
				gameAreaOffset.x = 0;
				gameAreaOffset.y = 0;
			}
			graphics->TranslateMatrix(gameAreaOffset.x, gameAreaOffset.y, 0);
		}
		~ScopeEarthQuake()
		{
			graphics->TranslateMatrix(-gameAreaOffset.x, -gameAreaOffset.y, 0);
		}
	};

	void KxxWin::RenderClearBuffer()
	{
#ifdef NB_DEBUG
		debugFontHSL.h+= 0.003f;
		if(debugFontHSL.h > 1.0f)
		{
			debugFontHSL.h = 0;
		}
		debugFontColor.SetHsl(debugFontHSL);
#endif

		if(this->debugMode && this->IsInGame())
		{
#ifdef NB_DEBUG
			HSLA hsl = debugFontHSL;
			hsl.h+= 0.5f;
			if(hsl.h > 1.0f)
			{
				hsl.h-= 1.0f;
			}
			RGBA c;
			c.SetHsl(hsl);
			c.a = 1.0f;
			graphics->SetClearColor(c);
			graphics->ClearBuffer(true, false, false);
#endif
		}
		else
		{

			graphics->SetClearColor(0, 0, 0, 1);
			graphics->ClearBuffer(true, false, false);
		}
	}

	void KxxWin::RenderUIBack()
	{
		SetToUIViewport();
		SetToUIProjection();

	
		if(NeedRenderGame())
		{
			graphics->SetTexMode(TM_REPLACE);
			graphics->BindTex(uiBgTex);
			graphics->DrawQuad(0, 0, K_VIEW_W, K_VIEW_H);
			graphics->SetTexMode(TM_MODULATE);
		}

		if(currentUI)
		{
			currentUI->RenderBack();
		}

		rlist->Render(RL_UI_BACK_0, RL_UI_BACK_1);
	}

	void KxxWin::RenderGameBack()
	{
		SetToGameViewport();

		if(!NeedRenderGame() || blurScreenTex)
		{
			return;
		}

		ScopeEarthQuake seq(state.earthQuakeTimer);

		// background
		graphics->SetBlendMode(BM_NORMAL);

		graphics->PushMatrix();
		stage->Render();
		graphics->PopMatrix();

		SetToGame2DProjection();

		// dark background
		if(state.darkScreenTimer)
		{
			graphics->SetTexMode(TM_DISABLE);
			float alpha = state.darkMaxAlpha * CalcFadeInFadeOut(state.darkScreenTime0, state.darkScreenTimer, state.darkFadeFrac);
			graphics->SetColor(0, 0, 0, alpha);
			graphics->DrawQuad(-5, -5, K_GAME_W+5, K_GAME_H+5);
			graphics->SetTexMode(TM_MODULATE);
		}	

		E_ASSERT(rlist != 0);
		rlist->Render(RL_GAME_BOTTOM, RL_GAME_LOW_AURA);
	}

	void KxxWin::RenderGameFront()
	{
		if(!NeedRenderGame())
		{
			return;
		}

		SetToGame2DProjection();

		// NB_PROFILE_INCLUDE;
		if(blurScreenTex)
		{
			graphics->BindTex(blurScreenTex);
			graphics->Disable(GS_BLEND);
			float f = blurScreenSteps * 0.1f;
			graphics->SetColor(1.0f, f + 0.7f,  f + 0.6f, 1.0f);
			graphics->SetTexMode(TM_MODULATE);
			graphics->DrawQuad(0, 0, K_GAME_W, K_GAME_H, K_GAME_TEX_X0, K_GAME_TEX_Y0, K_GAME_TEX_X1, K_GAME_TEX_Y1);
			graphics->Enable(GS_BLEND);
		}
		else
		{
			ScopeEarthQuake seq(state.earthQuakeTimer);

			E_ASSERT(rlist != 0);
			rlist->Render(RL_GAME_LOW_AURA+1, RL_GAME_DROP);
			this->RenderDrops();
			E_ASSERT(rlist != 0);
			rlist->Render(RL_GAME_DROP+1, RL_GAME_TOP);

			if(stage->dialog)
			{
				stage->dialog->Render();
			}

			_RenderGameExtra();
			_RenderGameDebug();
		}
	}

	void KxxWin::_RenderGameExtra()
	{
		graphics->SetBlendMode(BM_NORMAL);
		if(player)
		{
			float dx = K_GAME_H - player->pos.x;
			float dy = K_GAME_H - player->pos.y;
			float alpha = (dy * dy + dx * dx * 0.1f) * 0.00004f + 0.3f;
			if(alpha > 1.0f)
			{
				alpha = 1.0f;
			}

			float x = K_GAME_W - 13;
			float y = K_GAME_H - 12;
			graphics->SetColor(1, 1, 1, alpha);
			graphics->BindTex(statusLifeTex);
			int c = int(player->state.life);
			for(int i=0; i<c; i++)
			{
				graphics->DrawQuad(x, y, x+10, y+10);
				x-= 11;
			}

			x-= 11;
			graphics->BindTex(statusSCTex);
			c = int(player->state.scCount);
			for(int i=0; i<c; i++)
			{
				graphics->DrawQuad(x, y, x+10, y+10);
				x-= 11;
			}
		}

		// fps
		{
			float fps = (float)fpsCalc.GetRenderFPS();
			if(fps < 58.0f)
			{
				graphics->SetColor(1.0f, 0.5f, 0.0f, 0.8f);
			}
			else
			{
				graphics->SetColor(0.0f, 0.5f, 0.7f, 0.8f);
			}
			graphics->SetFont(smallFont);
			graphics->DrawString(2, 2, string(fps, 0));
		}

		// score, graze bonous
		{
			float dx = 0 - player->pos.x;
			float dy = K_GAME_H - player->pos.y;
			float alpha = (dy * dy + dx * dx * 0.1f) * 0.00004f + 0.3f;
			if(alpha > 1.0f)
			{
				alpha = 1.0f;
			}

			float x0 = 2;
			float y0 = K_GAME_H - 12;
			const RGBA cs[K_LEVEL_COUNT] = 
			{
				MakeColor(  0, 128, 255),
				MakeColor(255,  128,  0),
				MakeColor(255,  64,   0),
			};
			RGBA c = cs[state.level];
			c.a = alpha;
			graphics->SetColor(c);
				
			string levelC = GetLevelText(state.level);
			graphics->SetFont(smallFont);
			graphics->DrawString(x0, y0, levelC.c_str(), 1);
			x0 = 10;
			c.r = 0.58f;
			c.g = 0.5f;
			c.b = 0.39f;
			graphics->SetColor(c);
			graphics->SetFont(smallFont); 
			graphics->DrawString(x0, y0, FormatScore(player->state.curScore));

			x0 = 110;
			c.g = 0.5f;
			c.r = 0.39f;
			c.b = 0.58f;
			graphics->SetColor(c);
			graphics->DrawString(x0, y0, string::format(L"%.0f%%", player->GetTotalBonus() * 100));

			x0 = 160;
			c.r = 0.5f + player->state.power * 0.49f;
			c.g = 0.5f - player->state.power * 0.49f;
			c.b = 0.0f;
			graphics->SetColor(c);
			graphics->DrawString(x0, y0, string(player->state.power*100, 0));

		}
		if(this->isPause)
		{
			// blur screen
			graphics->SetTexMode(TM_DISABLE);
			graphics->SetColor(0.2f, 0.1f, 0, 0.3f);
			graphics->DrawQuad(0, 0, K_GAME_W, K_GAME_H);
			graphics->SetTexMode(TM_MODULATE);
		}			
	}

	void KxxWin::_RenderGameDebug()
	{
#ifdef NB_DEBUG
		if(this->debugMode == 1 && this->IsInGame())
		{

			graphics->SetTexMode(TM_DISABLE);
			graphics->SetColor(0, 0, 0, 0.70f);
			graphics->DrawQuad(-5, -5, K_GAME_W+5, K_GAME_H+5);
			graphics->SetTexMode(TM_MODULATE);

			{
				// master <-> pet
				graphics->SetColor(1, 1, 1, 1);

				RGBA rgba0 = {1, 1, 1, 1.0f};
				RGBA rgba1 = this->debugFontColor;

				{
					EnemyList::iterator it = enemyList.begin();
					while(it != enemyList.end())
					{
						Sprite * p = *it;
						p->RenderDebug();
						Sprite * master = dynamic_cast<Sprite*>(p->master);
						if(master)
						{
							graphics->DrawLine(master->pos.x, master->pos.y, 0, rgba0, p->pos.x, p->pos.y, 0, rgba1);
						}
						++it;
					}
				}

				{
					EnemyShotList::iterator it = enemyShotList.begin();
					while(it != enemyShotList.end())
					{
						Sprite * p = *it;
						p->RenderDebug();
						Sprite * master = dynamic_cast<Sprite*>(p->master);
						if(master)
						{
							graphics->DrawLine(master->pos.x, master->pos.y,0,  rgba0, p->pos.x, p->pos.y, 0, rgba1);
						}
						++it;
					}
				}

			}

			player->RenderDebug();
		}
#endif
	}

	void KxxWin::RenderUIFront()
	{
		// NB_PROFILE_INCLUDE;
		SetToUIViewport();
		SetToUIProjection();


		rlist->Render(RL_UI_FRONT_0, RL_UI_FRONT_0);
		if(currentUI)
		{
			graphics->SetColor(1, 1, 1, 1);
			currentUI->RenderFront();
		}

		rlist->Render(RL_UI_FRONT_1, RL_UI_FRONT_2);

		if(isReplaying && IsInGame())
		{
			_RenderJoystick();
		}

		_RenderUIDebug();

		_RenderNotify();

		if(IsInGame())
		{
			digitalRoller->Render(540, 110);
		}
	}

	void KxxWin::_RenderJoystick()
	{
		uint keyState = GetJoystickState(0);
		//if(player)
		//{ 
		//	if(player->pos.x > K_GAME_W * 0.75f)
		//	{
		//		jostick_icon_x = 2;
		//	}
		//	else if(player->pos.x < K_GAME_W * 0.25f)
		//	{
		//		jostick_icon_x = K_GAME_W - 66;
		//	}
		//}

		jostick_icon_x = K_GAME_W + 66;

		const static Rect _keyPos[8] = 
		{
			{13, 14, 4, 4},
			{23, 14, 4, 4},
			{18,  9, 4, 4},
			{18, 19, 4, 4},
			{37, 18, 5, 5},
			{46, 18, 5, 5},
			{46,  9, 5, 5},
			{37,  9, 5, 5},
		};
		graphics->SetColor(0.8f, 0.8f, 0.8f, 1.0f);
		graphics->TranslateMatrix(jostick_icon_x, K_GAME_H - 44);
		graphics->BindTex(gamepadTex);
		graphics->DrawQuad(0, 0, 64, 32);
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(1, 0.3f, 0, 0.75f);
		uint pattern = 0x01;
		for(int i=0; i<8; i++)
		{
			if(pattern & keyState)
			{
				const Rect & rc = _keyPos[i];
				graphics->DrawQuad(rc.L(), rc.T(), rc.R(), rc.B());
			}
			pattern<<=1;
		}
		graphics->SetTexMode(TM_MODULATE);
		graphics->TranslateMatrix(-jostick_icon_x, 44 - K_GAME_H);
	}


	//static const int NOTIFY_DURATION = 120;
	void KxxWin::ShowNotifyMessage(const string & _msg, float _duration, int _priority)
	{
		uint32 d = (uint32)(_duration * 60);
		if(d < 1)
		{
			d = 1;
		}
		Notify n = {enew string(_msg),  d, _priority};
		List<Notify>::iterator it = notifications.begin();
		for(; it!=notifications.end(); ++it)
		{
			if(it->priority > _priority)
			{
				break;
			}
		}

		notifications.insert(it, n);

		if(currentNotifyTimer == 0)
		{
			currentNotifyTimer = 1;
		}
	}

	void KxxWin::_RenderNotify()
	{
		if(currentNotifyTimer == 0)
		{
			return;
		}
		currentNotifyTimer--;
		// E_ASSERT(currentNotifyMessage != 0);
		if(currentNotifyTimer==0)
		{
			// clear current displaying message
			if(currentNotifyMessage)
			{
				delete currentNotifyMessage;
				currentNotifyMessage = 0;
			}

			if(!notifications.empty())
			{
				// display next message
				Notify n =  notifications.front();
				currentNotifyMessage = n.msg;
				currentNotifyTimer = n.duration;
				currentNotifyTimerMax = n.duration;
				notifications.erase(notifications.begin());
				currentNotifyWidth = 0;
			}
		}

		if(currentNotifyTimer == 0)
		{
			return;
		}

		if(currentNotifyWidth == 0)
		{
			currentNotifyWidth = defaultFont->W(currentNotifyMessage->c_str(), currentNotifyMessage->length()) + 1;
			if(currentNotifyWidth < 32)
			{
				currentNotifyWidth = 32;
			}
		}
		
		// render
		float f = 0.16f * 120 / currentNotifyTimerMax;
		float a = CalcFadeInFadeOut(currentNotifyTimerMax, currentNotifyTimer, f);
		//float hw = currentNotifyWidth * 0.5f;
		//float hkw = K_VIEW_W * 0.5f;
		float h = (float)defaultFont->H();

		//   x1                   x2
		//   ______________________
		//  /                      \
		// |                        |
		// +------------------------+
		// x0                       x3

		float w = currentNotifyWidth + 32.0f;
		float x3 = K_VIEW_W;
		float x0 = x3 - w;
		float x1 = x0 + 3.0f;
		float x2 = x3;

		float y0 = K_VIEW_H - (16.0f + h) * a;
		float y1 = y0 + 3.0f;
		float y2 = y0 + 16.0f + h;

		Vector2 v[6] = 
		{
			{ x0, y2 },
			{ x0, y1 },
			{ x3, y2 },
			{ x1, y0 },
			{ x3, y1 },
			{ x2, y0 },
		};
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(0, 0, 0, a*0.5f);
		graphics->DrawQuadStripX((Vector2*)v, 6);
		graphics->SetTexMode(TM_MODULATE);
		graphics->SetColor(1, 0.5f, 0,a);
		// graphics->BlendOn();
		// graphics->SetTexMode(TextureMode::Modulate);
		x0 = floor(x3 - currentNotifyWidth - 4.5f);
		y0 = floor(y0 + 8.0f + 0.5f);
		graphics->SetFont(defaultFont); graphics->DrawString(x0 , y0, *currentNotifyMessage);
		// graphics->SetTexMode(TextureMode::replace);
	}


	void KxxWin::OnSize(int _w, int _h)
	{
		float ovw = (float)K_VIEW_W;
		float ovh = (float)K_VIEW_H;
		float s0 = ovh / ovw;
		float s1 = float(_h) / float (_w);
		float zoom;
		if(s1 > s0)
		{
			// base on x
			zoom = _w / ovw;
			if(zoom >= 0.5f && zoom <= 0.55f)
			{
				zoom = 0.5f;
			}
			if(zoom >= 1.0f && zoom <= 1.1f)
			{
				zoom = 1.0f;
			}
			if(zoom >= 2.0f && zoom <= 2.2f)
			{
				zoom = 2.0f;
			}
		}
		else
		{
			// base on y
			zoom = _h / ovh;
			if(zoom >= 0.5f && zoom <= 0.55f)
			{
				zoom = 0.5f;
			}
			if(zoom >= 1.0f && zoom <= 1.1f)
			{
				zoom = 1.0f;
			}
			if(zoom >= 2.0f && zoom <= 2.2f)
			{
				zoom = 2.0f;
			}
		}

		viewport_ui_w = int(zoom * ovw);
		viewport_ui_h = int(zoom * ovh);
		viewport_ui_x = int((_w - viewport_ui_w) * 0.5f);
		viewport_ui_y = int((_h - viewport_ui_h) * 0.5f);
		viewport_game_w = int(zoom * K_GAME_W);
		viewport_game_h = int(zoom * K_GAME_H);
		viewport_game_x = int(zoom * K_GAME_X) + viewport_ui_x;
		viewport_game_y = int(zoom * K_GAME_Y) + viewport_ui_y;
	}

	void KxxWin::SetToUIViewport()
	{
		graphics->SetViewport(viewport_ui_x, viewport_ui_y, viewport_ui_w, viewport_ui_h);
	}

	void KxxWin::SetToGameViewport()
	{
		graphics->SetViewport(viewport_game_x, viewport_game_y, viewport_game_w, viewport_game_h);
	}
	
	void KxxWin::SetToUIProjection()
	{
		//graphics->SetActiveMatrixStack(MT_PROJECTION);
		//graphics->SetMatrix(projectionMatrixUI);
		//graphics->SetActiveMatrixStack(MT_MODELVIEW);
		//graphics->SetMatrix(modelViewMatrixUI);

		graphics->SetProjectViewMatrix(projectionMatrixUI, modelViewMatrixUI);
	}

	void KxxWin::SetToGame2DProjection()
	{
		//graphics->SetActiveMatrixStack(MT_PROJECTION);
		//graphics->SetMatrix(projectionMatrixGame2D);
		//graphics->SetActiveMatrixStack(MT_MODELVIEW);
		//graphics->SetMatrix(modelViewMatrixGame2D);
		graphics->SetProjectViewMatrix(projectionMatrixGame2D, modelViewMatrixGame2D);
	}

	void KxxWin::OnKeyDown(int _sym)
	{
		switch(_sym)
		{
		case 'W':
#ifdef NB_DEBUG
			graphics->Enable(GS_WIRE_FRAME,!graphics->GetRenderState().wire_frame_mode);
#endif
			break;
		case Keyboard::F1:
			OpenUrl(L"http://kxxstg.sourceforge.net");
			break;
		case Keyboard::F2:
#ifdef NB_DEBUG
			debugMode++;
			if(debugMode > 2)
			{
				debugMode = 0;
			}
			this->PlayMenuActionSound();
#endif
			break;
		case Keyboard::F3:
#ifdef NB_DEBUG
			if(this->IsInGame() && !this->isReplaying)
			{
				this->state.onceCheated = true;
				if(player->state.transparentTimer < 480)
				{
					player->state.transparentTimer = 480;
				}
				this->PlayMenuActionSound();
				Vector2 v;
				v.x = player->pos.x;
				v.y = player->pos.y - 150;
				for(int i = 1; i<_DROP_MAX; i++)
				{
					this->AddDrop(i, float(i * K_GAME_W) / (_DROP_MAX), 50);
				}
			}
#endif
			break;
		case Keyboard::F4:
#ifdef NB_DEBUG
			if(this->IsInGame() && !this->isReplaying)
			{
				Resume();
				ReplaceUI(0, FADE_ZOOM);
				this->RestartStage(true);
			}
#endif
			break;
		case Keyboard::F5:
#ifdef NB_DEBUG
			if(this->IsInGame() && !this->isReplaying)
			{
				this->state.level = (this->state.level + 1)  % K_LEVEL_COUNT;
				this->PlayMenuActionSound();
			}
#endif
			break;
		case Keyboard::F6:
#ifdef NB_DEBUG
			if(this->IsInGame())
			{
				this->state.onceCheated = true;
				this->debugFastForward = !debugFastForward;
				this->PlayMenuActionSound();
			}
#endif
			break;
		case Keyboard::F7:
#ifdef NB_DEBUG
			if(this->IsInGame() && !this->isReplaying)
			{
				this->state.onceCheated = true;
				PlaySE("spell-card", player->pos);
				this->DamageAllEnemy(1000, false);
				this->ClearEnemyShots(false, 0);
				this->AddBlastWave(player->pos, false);
			}
#endif
			break;
		case Keyboard::F8:
#ifdef NB_DEBUG
			if(this->IsInGame() && !this->isReplaying)
			{
				this->state.onceCheated = true;
				if(player->state.transparentTimer < 12000)
				{
					player->state.transparentTimer = 12000;
				}


				float da = PI * 2 / float(K_SHOT_COLOR_COUNT);
				float a = GetLogicTimer() * 0.03f;
				Vector2 v = {K_GAME_XC, K_GAME_YC};
				for(int h = 0; h < K_SHOT_COLOR_COUNT; h++)
				{
					DebugShot * p = enew DebugShot(debugShotType, h, v, a);
					kxxwin->AddEnemyShotToList(p);
					a+= da;
				}	

				debugShotType = (debugShotType+1) % K_SHOT_TYPE_COUNT;
				debugShotColor = (debugShotColor+1) % K_SHOT_COLOR_COUNT;
				this->PlayMenuActionSound();
			}
#endif
			break;
		case Keyboard::F9:
			CaptureScreenShot();
			break;
		case Keyboard::F11:
			E_TRACE_LINE(L"[kx] F11 has no effect.");
			break;
		default:
#ifdef NB_DEBUG
			if(_sym>=Keyboard::N1 && _sym<=Keyboard::N7 && _sym-Keyboard::N1<K_STAGE_COUNT)
			{
				if(this->IsInGame() && !this->isReplaying)
				{
					this->state.onceCheated = true;
					this->PlayMenuActionSound();
					StartStage((_sym - Keyboard::N1));
				}
				else
				{
					E_TRACE_LINE(L"[kx] (CHEAT) Failed to switch stage.");
				}
			}
#endif
			break;
		}
	}

	void KxxWin::OnKeyUp(int _sym)
	{

	}

	void KxxWin::Pause()
	{
		// NB_PROFILE_INCLUDE;
		if(this->isPause)
		{
			return;
		}

		this->isPause = true;

		if(graphics->GetCapabilities().frameBufferObject)
		{
			int vx, vy, vw, vh;
			graphics->GetViewport(vx, vy, vw, vh);
			FboRef fbo = graphics->CreateFbo(vw, vh);
			if(fbo)
			{
				graphics->SetFbo(fbo);
				if(graphics->BeginScene())
				{
					RenderClearBuffer();
					RenderGameBack();
					RenderGameFront();
					graphics->EndScene();
					blurScreenImage = enew ImageTexLoader();
					blurScreenImage->AddRef();
					TexRef tex = fbo->GetTex();
					if(tex && tex->GetImage(blurScreenImage->src))
					{
						blurScreenImage->src.FillChannel(3, 0xff);
						blurScreenTex = graphics->LoadTexFromLoader(blurScreenImage);
						E_ASSERT(blurScreenTex);
						blurScreenSteps = 3;
						blurScreenTimer = 20;
					}
					else
					{
						blurScreenImage->Release();
						blurScreenImage = 0;
					}
				}
				graphics->SetFbo(0);
			}
		}

		SwitchToPauseMenu();
	}

	void KxxWin::SwitchToPauseMenu()
	{
		// NB_PROFILE_INCLUDE;
		this->isPause = true;
		if(audio)
		{
			audio->Pause();
			audio->PlaySE("pause", 0.5f, 1.0f, 0, true);
		}

		PauseMenu * menu = enew PauseMenu(this);
		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Return To Game");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnPauseReturnToGame);
			menu->items.push_back(item);
		}
		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Restart Stage");
			item->enabled = !this->isReplaying;
			//item->needConfirm = true;
			item->callback = Callback(this, &KxxWin::OnPauseRestartStage);
			menu->items.push_back(item);
		}

		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Save Replay And Quit");
			item->enabled = CanSaveReplay();
			item->needConfirm = false;
			item->callback = Callback(this, &KxxWin::OnContinueSaveAndExit);
			menu->items.push_back(item);
		}

		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Quit");
			item->enabled = true;
			//item->needConfirm = !this->isPractice && !this->isReplaying;
			item->callback = Callback(this, &KxxWin::OnPauseExitToMainMenu);
			menu->items.push_back(item);
		}
		menu->activeItem = 0;
		menu->fontFace = defaultFont;
		menu->SetBoundRect(K_GAME_X, K_GAME_Y, K_GAME_X + K_GAME_W, K_GAME_Y + K_GAME_H);
		menu->CalcMenuSize();
		ReplaceUI(menu, FADE_NONE);
	}

	void KxxWin::SwitchToContinueMenu()
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(!isReplaying);
		this->isPause = true;
		if(audio)
		{
			audio->Pause();
		}

		PauseMenu * menu = enew PauseMenu(this);
		if(player->state.credits > 0)
		{
			menu->title = L"Continue (credit = " + string(player->state.credits) + L")";
		}
		else
		{
			menu->title = L"Game Over";
		}
		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Continue");
			item->enabled = player->state.credits > 0;
			item->callback = Callback(this, &KxxWin::OnContinueYes);
			menu->items.push_back(item);
		}
		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Restart Stage");
			item->enabled = true;
			//item->needConfirm = true;
			item->callback = Callback(this, &KxxWin::OnPauseRestartStage);
			menu->items.push_back(item);
		}

		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Save Replay And Quit");
			item->enabled = CanSaveReplay();
			item->needConfirm = false;
			item->callback = Callback(this, &KxxWin::OnContinueSaveAndExit);
			menu->items.push_back(item);
		}

		{
			PauseMenu::Item * item = enew PauseMenu::Item();
			item->text = _TT("Return To Main Menu");
			item->enabled = true;
			item->needConfirm = false;
			item->callback = Callback(this, &KxxWin::OnPauseExitToMainMenu);
			menu->items.push_back(item);
		}
		menu->activeItem = 0;
		menu->fontFace = defaultFont;
		menu->SetBoundRect(K_GAME_X, K_GAME_Y, K_GAME_X + K_GAME_W, K_GAME_Y + K_GAME_H);
		menu->CalcMenuSize();
		ReplaceUI(menu, FADE_SHUTTER);
	}

	int KxxWin::OnContinueYes(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		player->Continue();
		Resume();
		ReplaceUI(0, FADE_NONE);
		return 0;
	}

	int KxxWin::OnContinueSaveAndExit(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		if(CanSaveReplay())
		{
			stageClear = false;
			PromptSaveReplay();
		}
		else
		{
#ifdef NB_DEBUG
			message(L"[nb] Can't save replay. reason: " + GetCantSaveReplayReason());
#endif
			SwitchToMainMenu(true, FADE_ZOOM);
		}
		return 0;
	}

	void KxxWin::PromptSaveReplay()
	{
		// NB_PROFILE_INCLUDE;
		//RenderCurrentToOffScreenBuffer();
		ReplaySlotMenu * menu = enew ReplaySlotMenu(this, true);
		menu->title = _TT("Select Slot To Save");
		menu->Init(Callback(this, &KxxWin::OnSaveReplay));
		ReplaceUI(menu, FADE_SHUTTER);
	}

	void KxxWin::PromptLoadReplay()
	{
		// NB_PROFILE_INCLUDE;
		//RenderCurrentToOffScreenBuffer();
		ReplaySlotMenu * menu = enew ReplaySlotMenu(this, false);
		menu->title = _TT("Select Record To Play");
		menu->Init(Callback(this, &KxxWin::OnLoadReplay));
		ReplaceUI(menu, FADE_SHUTTER);
	}

	void KxxWin::UpdateCurrentStageReplaySequencePosition()
	{
		// NB_PROFILE_INCLUDE;
		PushReplayKey(true);
		ReplayStateInfo & info = CurrentStageInfo();
		info.kxxWinState.replayStopPosition = state.replayPosition;
		info.kxxWinState.stageHighScore = player->state.curScore;
		info.isReady = true;
	}

	int KxxWin::OnSaveReplay(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		ReplaySlotMenu * menu = static_cast<ReplaySlotMenu*>(_p);
		if(menu->activeItem != menu->items.size() - 1)
		{
			if(!stageClear)
			{
				UpdateCurrentStageReplaySequencePosition();
			}
			SaveReplay(menu->GetSelectSlotPath());
		}

		// SwitchTopScoreScreen(player->state.score);
		SwitchToMainMenu(true, FADE_ZOOM);
		return 0;
	}
	
	void KxxWin::SaveHighScore(float _score)
	{
		PersistData * pd = GetCurrentPersistData();
		int index = pd->InsertTopScore(_score);
	}

	int KxxWin::OnQuitGame(void * _p)
	{
		// NB_PROFILE_INCLUDE;

		if(stageClear)
		{
			SwitchToEndingScreen();
		}
		else
		{
			SwitchToMainMenu(true, FADE_SHUTTER);
		}	
		return 0;
	}

	int KxxWin::GetReplayStageCount()
	{
		int ret = 0;
		for(int i=0; i<K_STAGE_COUNT; i++)
		{
			if(replayStageInfo[i].isReady)
			{
				ret++;
			}
		}
		return ret;
	}

	int KxxWin::GetFirstReplayStage()
	{
		for(int i=0; i<K_STAGE_COUNT; i++)
		{
			if(replayStageInfo[i].isReady)
			{
				return i;
			}
		}
		return -1;
	}

	int KxxWin::OnLoadReplay(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		ReplaySlotMenu * menu = static_cast<ReplaySlotMenu*>(_p);
		if(menu->activeItem != menu->items.size() - 1)
		{
			if(LoadReplay(menu->GetSelectSlotPath()))
			{
				int c = GetReplayStageCount();
				E_ASSERT(c > 0);
				if(c == 1)
				{
					// enter the game directly
					int n = GetFirstReplayStage();
					isReplaying = true;
					demoReplayTimer = 0;
					isDemoReplay = false;
					StartStage(n);
				}
				else
				{
					// prompt select stage
					SwitchToSelectReplayStageMenu(0, false);
				}
				return 0;
			}
		}

		SwitchToMainMenu(false, FADE_SHUTTER);
		return 0;
	}

	void KxxWin::SwitchToSelectPracticeStageMenu(int _active_state_index, bool _isInGame)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_active_state_index >= 0 && _active_state_index < K_STAGE_COUNT);
		StageMenu * menu = enew StageMenu(this, _isInGame);
		menu->title = _TT("Select Stage To Practice");
		menu->Init(Callback(this, &KxxWin::OnSelectdPracticeStage));
		PersistData * pd = GetCurrentPersistData();
		for(int i=0; i<=pd->maxPassStage && i<K_STAGE_COUNT; i++)
		{
			menu->Enable(i, true);
		}

		for(int i = _active_state_index; i<menu->items.size(); i++)
		{
			if(menu->items[i]->enabled)
			{
				menu->activeItem = i;
				break;
			}
		}

		ReplaceUI(menu, FADE_SHUTTER);
	}

	void KxxWin::SwitchToSelectReplayStageMenu(int _active_state_index, bool _isInGame)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_active_state_index >= 0 && _active_state_index < K_STAGE_COUNT);
		isReplaying = true;
		StageMenu * menu = enew StageMenu(this, _isInGame);
		menu->title = _TT("Select Stage To Replay");
		menu->Init(Callback(this, &KxxWin::OnSelectdReplayStage));
		for(int i=0; i<K_STAGE_COUNT; i++)
		{
			menu->Enable(i, replayStageInfo[i].isReady);
		}
		for(int i = _active_state_index; i<menu->items.size(); i++)
		{
			if(menu->items[i]->enabled)
			{
				menu->activeItem = i;
				break;
			}
		}
		ReplaceUI(menu, FADE_NONE);
	}


	int KxxWin::OnSelectdReplayStage(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		StageMenu * menu = static_cast<StageMenu*>(_p);
		E_ASSERT(menu);
		if(menu->activeItem != menu->items.size() - 1)
		{
			this->isReplaying = true;
		//	this->isPause = false;
//			graphicTimer = 0;
			StartStage(menu->activeItem);
			//PlayMenuActionSound();
		}
		else
		{
			// back to previous state
			PromptLoadReplay();
		}
		return 0;
	}

	int KxxWin::OnPauseReturnToGame(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		Resume();
		ReplaceUI(0, FADE_NONE);
		return 0;
	}

	int KxxWin::OnPauseRestartStage(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		Resume();
		ReplaceUI(0, FADE_ZOOM);
		this->RestartStage(false);
		return 0;
	}

	int KxxWin::OnPauseExitToMainMenu(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		this->isPause = true;
		SwitchToMainMenu(true, FADE_ZOOM);
		return 0;
	}

	void KxxWin::Resume()
	{
		// NB_PROFILE_INCLUDE;
		if(isPause)
		{
			blurScreenTex.Detach();
			if(blurScreenImage)
			{
				blurScreenImage->Release();
				blurScreenImage = 0;
			}
			blurScreenSteps = 0;

			this->isPause = false;
			if(audio)
			{
				audio->Resume();
			}
		}
	}

	void KxxWin::SwitchToMainMenu(bool _rewindMusic, FadeType _fadeType)
	{
		// NB_PROFILE_INCLUDE;
		this->isExStart = false;
		this->isPractice = false;
		this->isReplaying = false;
		this->isPause = true;
		this->isDemoReplay = false;
		DeletePlayers();
		if(_rewindMusic)
		{
			StopBGM();
		}
		// soundPlayer->Stop();
		MainMenu * mainMenu = enew MainMenu(this);
		{
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("START");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnMainMenuStart);
			mainMenu->items.push_back(item);
		}
		{
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("EX START");
			item->enabled = combinedPersistData.ex_unlocked();
			item->callback = Callback(this, &KxxWin::OnMainMenuExStart);
			mainMenu->items.push_back(item);
		}
		{
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("PRACTICE");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnMainMenuPractice);
			mainMenu->items.push_back(item);
		}
		{
			Path path = Env::GetDataFolder() | L"replay";
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("REPLAY");
			item->enabled = false;
			for(int i=1; i<= K_MAX_REPLAY_SLOT; i++)
			{
				if(FS::IsFile(path | string::format(L"%d.kxx01",i)))
				{
					item->enabled = true;
					break;
				}
			}
			item->callback = Callback(this, &KxxWin::OnMainMenuReplay);
			mainMenu->items.push_back(item);
		}

		{
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("OPTIONS");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnMainMenuOptions);
			mainMenu->items.push_back(item);
		}

		{
			MainMenu::Item * item = enew MainMenu::Item();
			item->text = _TT("EXIT");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnMainMenuQuitApplication);
			mainMenu->items.push_back(item);
		}
		mainMenu->activeItem = lastMainMenuItem;
		mainMenu->fontFace = defaultFont;
		mainMenu->SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		mainMenu->CalcMenuSize();
		if(_rewindMusic)
		{
			//soundPlayer->PlayMusic("bgm_main_menu");
			this->PlayBGM(0);
		}

		DeleteStage();
		ReplaceUI(mainMenu, _fadeType);
	}

	int KxxWin::OnMainMenuShowHighScore(void * _p)
	{
		// NB_PROFILE_INCLUDE;

		return 0;
	}

	int KxxWin::OnMainMenuStart(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		// PlayMenuActionSound();
		this->isPractice = false;
		this->isExStart = false;
//		SwitchToDifficultyMenu();
		SwitchToPlayerMenu();
		return 0;
	}

	int KxxWin::OnMainMenuExStart(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		// PlayMenuActionSound();
		this->isPractice = false;
		this->isExStart = true;
//		SwitchToDifficultyMenu();
		SwitchToPlayerMenu();
		return 0;
	}

	int KxxWin::OnMainMenuPractice(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		this->isPractice = true;
//		SwitchToDifficultyMenu();
		SwitchToPlayerMenu();
		return 0;
	}

	void KxxWin::StartNewGame(int _stage_index)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_stage_index >= 0 && _stage_index < K_STAGE_COUNT);

		options->Save();

//		graphicTimer = 0;
		state.logicTimer = 0;
//		state.subLogicPass = 0;
//		thisCycleRenderedFrames = 0;
		memset(&replayStageInfo[0], 0, sizeof(replayStageInfo));
		state.initialRandomSeed = GenerateRandomSeed();
		logic_random_set_seed(state.initialRandomSeed);
		state.level = options->level;
		GenerateShotVelTable();
		state.playingPlayer     = options->player;
		state.lastJoystickState = GetJoystickState(0);
		stageClear = false;
		state.onceContinued = false;
		state.onceRestartStage = false;
		state.onceCheated = false;
		state.earthQuakeTimer = 0;
//		state.playerSCAvatarTimer = 0;
//		state.bossSCAvatarTimer = 0;
		state.darkScreenTime0 = 0;
		state.darkScreenTimer = 0;
		state.joystickState[0] = 0;
		state.joystickState[1] = 0;
		//state.joystickKeyboardState[0] = 0;
		//state.joystickKeyboardState[1] = 0;
		joystickSequence.resize(10000);
		joystickSequence.resize(0);
		state.replayPosition = 0;
		state.delay_clear_enemy_shot_timer = 0;
		state.delay_clear_enemy_shot_absorb = false;


		ChangePlayer(this->state.playingPlayer);

		// enter game
		this->isReplaying = false;
		StartStage(_stage_index);
	}


	void KxxWin::ChangePlayer(int _playerID)
	{
		// NB_PROFILE_INCLUDE;
		DeletePlayers();
		switch(_playerID)
		{
		default:
			E_ASSERT(0);
		case 0:
			player = enew PlayerA();
			dropTex[DROP_EC_A] = LoadTex("drop-ec-a-0");
			dropIndicatorTex[DROP_EC_A] = LoadTex("indicator-ec-a-0");
			break;
		case 1:
			player = enew PlayerB();
			dropTex[DROP_EC_A] = LoadTex("drop-ec-a-1");
			dropIndicatorTex[DROP_EC_A] = LoadTex("indicator-ec-a-1");
			break;
		}
		player->ResetState(false);
		if(isPractice)
		{
			player->state.credits = 0;
			player->state.life*= 2;
			player->OnAteDrop(DROP_PET);
			player->AddPower(0.5f);
		}
	}

	void KxxWin::StartStage(int _stage_index, bool _test_boss)
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_DEBUG
		{
			kxxwin->debugFastForward = false;
			kxxwin->debugFastForwardSpeed= 3;
		}
#endif

		E_ASSERT(_stage_index >= 0 && _stage_index < K_STAGE_COUNT);
		highScoreCopy = GetCurrentPersistData()->topScores[0].score;

		// verify there are not enemy and drop when switching stage
		//E_ASSERT(enemyShotList.empty() && enemyList.empty() && dropList.empty());
		ClearKxxSpriteLists();

		// sync state
		KxxWin::ReplayStateInfo & info = replayStageInfo[_stage_index];

		// very some important state
		ReplayStateInfo infoVerify = info;

		if(isReplaying)
		{
			// sync and change player
			memcpy(&state, &info.kxxWinState, sizeof(info.kxxWinState));
			GenerateShotVelTable();
			ChangePlayer(this->state.playingPlayer);
			memcpy(&state, &info.kxxWinState, sizeof(info.kxxWinState));
			logic_random_set_seed(state.initialRandomSeed);
			player->SetState(info.playerState);
			memcpy(&player->pos, &info.playercenter,sizeof(info.playercenter));
			//memcpy(&this->state, &info.kxxWinState, sizeof(info.kxxWinState));
		}
		else
		{
			state.joystickStateForReplay = state.joystickState[0];
			state.initialRandomSeed = logic_random_get_seed();
			memcpy(&info.kxxWinState, &state, sizeof(info.kxxWinState));
			memcpy(&info.playerState, &player->state, sizeof(info.playerState));
			memcpy(&info.playercenter,&player->pos, sizeof(info.playercenter));
		}
		
		// restore from puse state
		Resume();

		// hide previous screen
		ReplaceUI(0, isReplaying && isDemoReplay ? FADE_NONE : FADE_ZOOM);

		// create stage and run it
		delete stage;
		stage = CreateStage(_stage_index);;
#ifdef NB_DEBUG
		if(isReplaying)
		{
			E_ASSERT(memcmp(&infoVerify.kxxWinState,  &state, sizeof(infoVerify.kxxWinState)) == 0);
			E_ASSERT(memcmp(&infoVerify.playerState,  &player->state, sizeof(infoVerify.playerState)) == 0);
			E_ASSERT(memcmp(&infoVerify.playercenter,  &player->pos, sizeof(infoVerify.playercenter)) == 0);
			E_ASSERT(logic_random_get_seed() ==  ((infoVerify.kxxWinState.initialRandomSeed ^ (infoVerify.kxxWinState.initialRandomSeed >> 16)) & 0xffff));
		}
#endif
		stage->is_test_boss = _test_boss;
		stage->Start();
	}


	void KxxWin::SwitchToPlayerMenu()
	{
		// NB_PROFILE_INCLUDE;
		isReplaying = false;
		PlayerMenu * playerMenu = enew PlayerMenu(this);

		{
			PlayerMenu::Item * item = enew PlayerMenu::Item();
			item->text = _TT("PlayerA");
			item->enabled = isExStart ? combinedPersistData.player_ex_unlocked[0] : true;
			item->callback = Callback(this, &KxxWin::OnPlayerSelected);
			playerMenu->items.push_back(item);
		}
		{
			PlayerMenu::Item * item = enew PlayerMenu::Item();
			item->text = _TT("PlayerB");
			item->enabled = isExStart ? combinedPersistData.player_ex_unlocked[1] : true;
			item->callback = Callback(this, &KxxWin::OnPlayerSelected);
			playerMenu->items.push_back(item);
		}

		{
			PlayerMenu::Item * item = enew PlayerMenu::Item();
			if(!isExStart && combinedPersistData.player_d_unlocked())
			{
				item->text = _TT("Kanatayuri Tokie");
				item->enabled = true;
			}
			else
			{
				item->text = _TT("[LOCKED]");
				item->enabled = false;
			}
			item->callback = Callback(this, &KxxWin::OnPlayerSelected);
			playerMenu->items.push_back(item);
		}

		{
			PlayerMenu::Item * item = enew PlayerMenu::Item();
			item->text = _TT("Cancel");
			item->enabled = true;
			item->callback = Callback(this, &KxxWin::OnPlayerCancel);
			playerMenu->items.push_back(item);
		}

		playerMenu->Init(options->player);
		playerMenu->fontFace = defaultFont;
		playerMenu->SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		playerMenu->CalcMenuSize();
		ReplaceUI(playerMenu, FADE_NONE);
	}

	int KxxWin::OnPlayerSelected(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		PlayerMenu * menu = static_cast<PlayerMenu*>(_p);
		E_ASSERT(menu != 0);
		options->player = menu->GetActiveItem();
		options->level = this->isExStart ? K_LEVEL_COUNT - 1 : menu->level;
		state.playingPlayer = options->player;
		if(isPractice)
		{
			SwitchToSelectPracticeStageMenu(0, false);
		}
		else if(isExStart)
		{
			StartNewGame(6);
		}
		else
		{
			StartNewGame(0);
		}
		return 0;
	}

	int KxxWin::OnPlayerCancel(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		//SwitchToMainMenu(0, false);
		SwitchToMainMenu(false, FADE_NONE);
		return 0;
	}

	int KxxWin::OnSelectdPracticeStage(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		StageMenu * menu = static_cast<StageMenu*>(_p);
		E_ASSERT(menu);
		if(menu->activeItem != menu->items.size() - 1)
		{
			StartNewGame(menu->activeItem);
		}
		else
		{
			// back to previous state
			SwitchToMainMenu(false, FADE_ALPHA);
		}
		return 0;
	}


	void KxxWin::SwitchToOptionsMenu()
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu * optionsMenu = enew OptionsMenu(this);


//#ifdef NB_WINDOWS
		//{
		//	backupWindowSize = options->windowSize;
		//	OptionsMenu::Item * item = enew OptionsMenu::Item();
		//	item->text = _TT("Display Mode:");
		//	item->options.push_back(_TT("Fixed"));
		//	item->options.push_back(_TT("Resizeable"));
		//	item->options.push_back(_TT("Full Screen"));
		//	item->activeOption = options->windowSize;
		//	item->callback = Callback(this, &KxxWin::OnOptionWindowSize);
		//	optionsMenu->items.push_back(item);
		//}
//#endif
#ifdef NB_WINDOWS
		{
			backupWindowSize = options->windowSize;
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Graphics Backend:");
			item->options.push_back(_TT("OpenGL"));
			item->options.push_back(_TT("Direct3D"));
			item->activeOption = options->graphicsBackend;
			item->require_restart = true;
			item->callback = Callback(this, &KxxWin::OnOptionGraphicsBackend);
			optionsMenu->items.push_back(item);
		}
#endif

		{
			backupWindowSize = options->windowSize;
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Graphics Level:");
			item->options.push_back(_TT("Speed"));
			item->options.push_back(_TT("Quality"));
			item->activeOption = options->graphicsEffect;
			item->callback = Callback(this, &KxxWin::OnOptionGraphicsLevel);
			optionsMenu->items.push_back(item);
		}

		{
			backupWindowSize = options->windowSize;
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Audio Backend:");
			Array<stringa> audioBackends;
			APlayer::GetAvailableBackends(audioBackends);
//			item->options.push_back(_TT("OpenGL"));
//			item->options.push_back(_TT("Direct3D"));
			int & act = item->activeOption;
			act = 0;
			for(int i=0; i<audioBackends.size(); i++)
			{
				string s = audioBackends[i];
				item->options.push_back(s);
				if(s.icompare(options->audioBackend) == 0)
				{
					act = i;
				}
			}

			item->require_restart = true;
			item->callback = Callback(this, &KxxWin::OnOptionAudioBackend);
			optionsMenu->items.push_back(item);
		}

		{
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Audio Buffer(ms):");
			int & act = item->activeOption;
			act = 0;
			int err = 1000;
			for(int i=0; i<=5; i++)
			{
				int n = i*20 + 40;
				item->options.push_back(string(n));
				int e0 = abs(n - options->audioBufMsec);
				if(e0 <= err)
				{
					act = i;
					err = e0;
				}
			}
			item->require_restart = true;
			item->callback = Callback(this, &KxxWin::OnOptionAudioBuffer);
			optionsMenu->items.push_back(item);
		}

		{
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Music Volume:");
			for(int i=0; i<=10; i++)
			{
				item->options.push_back(string(i));
			}
			item->activeOption = options->musicVolume;
			item->callback = Callback(this, &KxxWin::OnOptionMusicVolume);
			optionsMenu->items.push_back(item);
		}

		{
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Sound Volume:");
			for(int i=0; i<=10; i++)
			{
				item->options.push_back(string(i));
			}
			item->activeOption = options->soundVolume;
			item->callback = Callback(this, &KxxWin::OnOptionSoundVolume);
			optionsMenu->items.push_back(item);
		}

		{
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Reverb:");
			for(int i=0; i<=10; i++)
			{
				item->options.push_back(string(i));
			}
			item->activeOption = options->audioReverbLevel;
			item->callback = Callback(this, &KxxWin::OnOptionReverb);
			optionsMenu->items.push_back(item);
		}

		{
			OptionsMenu::Item * item = enew OptionsMenu::Item();
			item->text = _TT("Return");
			item->callback = Callback(this, &KxxWin::OnOptionSaveAndQuit);
			optionsMenu->items.push_back(item);
		}

		optionsMenu->fontFace = defaultFont;
		optionsMenu->SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		optionsMenu->CalcMenuSize();
		ReplaceUI(optionsMenu, FADE_SHUTTER);
	}

	int KxxWin::OnOptionWindowSize(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->windowSize = item->activeOption;
		PlayMenuSelectSound();
		return 0;
	}

	int KxxWin::OnOptionDifficulty(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->level = item->activeOption;
		PlayMenuSelectSound();
		return 0;
	}

	int KxxWin::OnOptionGraphicsBackend(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->graphicsBackend = item->activeOption;
		PlayMenuSelectSound();
		return 0;
	}

	int KxxWin::OnOptionGraphicsLevel(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->graphicsEffect = item->activeOption;
		PlayMenuSelectSound();
		return 0;
	}

	int KxxWin::OnOptionAudioBackend(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->audioBackend = item->options[item->activeOption];
		PlayMenuSelectSound();
		return 0;
	}

	int KxxWin::OnOptionSoundVolume(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->soundVolume = item->options[item->activeOption].to_int();
		if(audio)
		{
			audio->SetGain(options->musicVolume * 0.1f, options->soundVolume * 0.1f);
			audio->PlaySE("volume-test");
		}
		return 0;
	}

	int KxxWin::OnOptionMusicVolume(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->musicVolume = item->options[item->activeOption].to_int();
		if(audio)
		{
			audio->SetGain(options->musicVolume * 0.1f, options->soundVolume * 0.1f);
		}
		return 0;
	}

	int KxxWin::OnOptionReverb(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->audioReverbLevel = item->options[item->activeOption].to_int();
		if(audio)
		{
			audio->SetReverb(options->audioReverbLevel * 5);
			audio->PlaySE("volume-test");
		}
		return 0;
	}


	int KxxWin::OnOptionAudioBuffer(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->audioBufMsec = item->options[item->activeOption].to_int();
		if(audio)
		{
			audio->SetGain(options->musicVolume * 0.1f, options->soundVolume * 0.1f);
			audio->PlaySE("volume-test");
		}
		return 0;
	}

	int KxxWin::OnOptionSaveAndQuit(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		OptionsMenu::Item * item = static_cast<OptionsMenu::Item *>(_p);
		options->Save();
		if(backupWindowSize != options->windowSize)
		{
			switch(options->windowSize)
			{
			default:
			case 0:
				Win::Resize(K_VIEW_W, K_VIEW_H, false);
				break;
			case 1:
				Win::Resize(K_VIEW_W*2, K_VIEW_H*2, false);
				break;
			}
		}
		SwitchToMainMenu(false, FADE_SHUTTER);
		return 0;
	}

	int KxxWin::OnMainMenuReplay(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		PromptLoadReplay();
		return 0;
	}

	int KxxWin::OnMainMenuOptions(void * _p)
	{
		// NB_PROFILE_INCLUDE;
	//	StartFade(FadeType(logic_random_int() % (int) _FADE_MAX), 700);
		SwitchToOptionsMenu();
		return 0;
	}

	int KxxWin::OnMainMenuQuitApplication(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		this->SafeDeleteThis();
		return 0;
	}

	void KxxWin::ReplaceUI(UIBase * _newUI, FadeType _type)
	{
		// NB_PROFILE_INCLUDE;
		if(_type > 0 && (_newUI != 0 || currentUI != 0))
		{
			StartFade(_type);
		}
		if(_newUI != 0 && uiToDelete)
		{
			delete uiToDelete;
			uiToDelete = 0;
		}
		if(currentUI != 0)
		{
			uiToDelete = currentUI;
		}
		currentUI = _newUI;
	}

	void KxxWin::StartFade(FadeType _type)
	{
		// NB_PROFILE_INCLUDE;
		if(options->graphicsEffect >0 && graphics->GetCapabilities().frameBufferObject)
		{
			switch(_type)
			{
			case FADE_ALPHA:
				SetFade(enew SimpleFade(SimpleFade::typeAlpha, true), 0.8f);
				break;
			case FADE_SHUTTER:
				SetFade(enew SimpleFade(SimpleFade::typeShutter, true), 0.8f);
				break;
			case FADE_ZOOM:
				SetFade(enew ZIZOFade(true, true), 1.0f);
				break;
			}
		}
		else if(graphics->GetCapabilities().frameBufferObject)
		{
			switch(_type)
			{
			case FADE_ZOOM:
#ifdef NB_DEBUG
				ShowNotifyMessage(_TT("Zoom effect is disabled."));
#endif
				SetFade(enew SimpleFade(SimpleFade::typeAlpha, true), 0.8f);
				break;
			case FADE_ALPHA:
				SetFade(enew SimpleFade(SimpleFade::typeAlpha, true), 0.8f);
				break;
			default:
				SetFade(enew SimpleFade(SimpleFade::typeShutter, false), 0.8f);
				break;
			}
		}
		else
		{
			SetFade(enew SimpleFade(SimpleFade::typeShutter, false), 0.8f);
		}
	}

	bool KxxWin::OnJoystickDown(int _joystick_id, int _button)
	{
		// NB_PROFILE_INCLUDE;
		//E_TRACE_LINE("[kx] OnJoystickDown(), _button=" + string(_button));
		state.joystickEventCount++;

		if(currentUI && currentUI->OnJoystickDown(_joystick_id, _button))
		{
			return true;
		}
		if(_joystick_id != 0)
		{
			return false;
		}

		demoReplayTimer = 0;

		switch(_button)
		{
		case MAPED_BUTTON_PAUSE:
			if(this->IsInGame() && !this->isPause && currentUI == 0)
			{
				Pause();
			}
			break;
		default:
			return false;
		};

		return true;
	}

	void KxxWin::OnJoystickUp(int _joystick_id, int _button)
	{
		//E_TRACE_LINE("[kx] OnJoystickUp(), _button=" + string(_button));
	}

	bool KxxWin::LoadPersistData()
	{
		// NB_PROFILE_INCLUDE;
		Path persistPath = Env::GetDataFolder() | L"persist";

		FileRef file = FS::OpenFile(persistPath);
		if(!file)
		{
			return false;
		}

		if(file->Read(&combinedPersistData, sizeof(combinedPersistData)))
		{
			highScoreCopy = combinedPersistData.persistData[0][0].topScores[0].score;
			return true;
		}
		else
		{
			FS::Delete(persistPath);
			memset(&combinedPersistData, 0, sizeof(combinedPersistData));
			highScoreCopy = 0;
			return false;
		}
	}

	bool KxxWin::SavePersistData()
	{
		// NB_PROFILE_INCLUDE;
		Path persistPath = Env::GetDataFolder() | L"persist";

		FileRef file = FS::OpenFile(persistPath, true);
		if(!file)
		{
			return false;
		}
		// E_SCOPE_RELEASE(file);

		if(!file->SetSize(0))
		{
			return false;
		}
		if(!file->Write(&combinedPersistData, sizeof(combinedPersistData)))
		{
			return false;
		}
		return true;
	}

	int KxxWin::GetSpriteCount() const
	{
		// NB_PROFILE_INCLUDE;
		return dropList.size() + rlist->GetSize();
	}


	void KxxWin::AddPieEnemyShot(const Vector2 & _v, float _angle, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_splitCount < 1024);
		E_ASSERT(_shotCount <= _splitCount);
		E_ASSERT(_shotTexIndex >= 0 && _shotTexIndex < K_SHOT_TYPE_COUNT);
		TexRef tex = kxxwin->shotTex[_shotTexIndex][0];
		float angles[1024];
		CalcCircleShot(angles, _splitCount, _angle);
		Vector2 speed0, speed1;
		for(int i = _splitCount - _shotCount; i < _splitCount; i++)
		{
			speed1.SetPolar(_speed , angles[i]);
			speed0.x = speed1.x * 3.0f;
			speed0.y = speed1.y * 3.0f;
			AccelerateShot * b = enew AccelerateShot(tex, speed0, speed1, 0.3f
				,shotSmokeTex0, 20, 10, 0.3f);
			b->isEllipseCollision = true;
			b->collisionFrac.x = 0.35f;
			b->collisionFrac.y = 0.35f;
			b->pos = _v;
			b->Step();
			if(_rotate)
			{
				b->RotateNorthToVectorDirection(b->speed);
			}
			this->AddEnemyShotToList(b);
		}
	}

	void KxxWin::AddEnemyDrop(const DropDef & _def, const Vector2 & _pt)
	{
		Vector2 v;
		// NB_PROFILE_INCLUDE;
		bool exactPos = _def.allRandomPos ? false : true;
		for(int i=0; i<3; i++)
		{
			const DropDef::ENEMY_DROP & d = _def.Get(i);
			if(d.type == DROP_NONE)
			{
				continue;
			}
			int n;
			if(d.random_min >= d.random_max)
			{
				n = d.random_min;
			}
			else
			{
				n = d.random_min + logic_random_int() % (d.random_max + 1 - d.random_min);
			}
			for(int j = 0; j < n; j++)
			{
				if(exactPos)
				{
					v = _pt;
					exactPos = false;
				}
				else
				{
					v.x = _pt.x + logic_random_int() % 100 - 50;
					v.y = _pt.y + logic_random_int() % 100 - 50;
				}
				AddDrop(d.type, v.x, v.y, false, -(PI * 0.5f));
			}
		}
	}

	void KxxWin::AddBossExplosion(Sprite * _s)
	{
		AddCorpseExplosion(_s, 200, 20, 480);
		BossSpread * p = enew BossSpread(LoadTex("gather-star"), _s->pos, 20 * K_LOGIC_FPS_MUL);
		AddLogicActionToList(p);
		AddBlastWave(_s->pos, false);
		EarthQuake(0.7f);
	}

	void KxxWin::AddEnemyExplosion(Sprite * _s)
	{
		AddCorpseExplosion(_s, 100, 8, 240);
	}

	void KxxWin::AddCorpseExplosion(Sprite * _s, float _r, int _n, uint32 _t)
	{
		for(int i=0; i<_n; i++)
		{
			float r = frand() * 0.45f;
			float a = CrtRandAngle();
			float x = 0.5f + r * cos(a);
			float y = 0.5f + r * sin(a);
			float w = frand() * 0.3f + 0.1f;
			float h = frand() * 0.3f + 0.1f;
			Corpse * p = enew Corpse();
			p->tex = _s->tex;
			p->pos.x = _s->pos.x + x * _s->hsz.x * _s->scl.x;
			p->pos.y = _s->pos.y + y * _s->hsz.y * _s->scl.y;
			p->hsz.x = w * _s->hsz.x * _s->scl.x;
			p->hsz.y = h * _s->hsz.y * _s->scl.y;
			p->tx0 = x - w * 0.5f;
			p->ty0 = y - h * 0.5f;
			p->tx1 = x + w * 0.5f;
			p->ty1 = y + h * 0.5f;
			p->timer = (int)(_t * (frand() * 0.3f + 0.85f));
			p->alpha_delta = -0.99f / p->timer;
			p->scale_delta = - (0.30f + 0.65f * frand()) / p->timer;
			float s = _r * 2 * (0.20f + frand() * 1.0f) / K_LOGIC_FPS;
			p->vel.SetPolar(s, a + (frand() -0.5f) * PI * 0.5f);
			p->acc = p->vel / -(float)p->timer;
			p->acc.y+= 2 * 80.0f / (float(p->timer) * float(p->timer));
			p->rot = (frand() - 0.5f) * 0.3f;
			kxxwin->AddSparkToList(p);
		}

		{
			float t = 30.0f / _r;
			Spark1 * p = enew Spark1(kxxwin->explosionTex, t, 0, 0, 15, _r, 1, 0.2f);
			p->blm = BM_ADD;
			p->pos = _s->pos;
			AddSparkToList(p);
		}

		{
			float t = 15.0f / _r;
			Spark1 * p = enew Spark1(kxxwin->playerAruaTex, t, 0, 0, 10, _r, 1, 0.2f);
			p->pos = _s->pos;
			AddSparkToList(p);
		}
	}


	void KxxWin::AddExplosion(const Vector2 & _v, float _maxRadius, int _hue)
	{
		const RGBA & c = hue_color(_hue);
		{
			float t = 30.0f / _maxRadius;
			Spark1 * p = enew Spark1(kxxwin->explosionTex, t, 0, 0, 15, _maxRadius, 1, 0.2f);
			p->blm = BM_ADD;
			p->pos = _v;
			AddSparkToList(p);
		}

		{
			float t = 15.0f / _maxRadius;
			Spark1 * p = enew Spark1(kxxwin->playerAruaTex, t, 0, 0, 10, _maxRadius, 1, 0.2f);
			//p->shineBlend = false;
			p->clr = c;
			p->pos = _v;
			AddSparkToList(p);
		}

		AddGenericSpark(_v, 4, _hue);
	}

	//void KxxWin::AddShotExplosion(Shot * _shot)
	//{
	//	float r = (1 + _shot->hsz.x * _shot->scl.x) * 3;
	//	const RGBA & c = hue_color(_shot->hue);
	//	{
	//		float t = 30.0f / r;
	//		Spark1 * p = enew Spark1(kxxwin->explosionTex, t, 0, 0, 15, r, 1, 0.2f);
	//		p->clr = c;
	//		p->blm = BM_ADD;
	//		p->pos = _shot->pos;
	//		AddSparkToList(p);
	//	}

	//	{
	//		float t = 15.0f / r;
	//		Spark1 * p = enew Spark1(kxxwin->playerAruaTex, t, 0, 0, 10, r, 1, 0.2f);
	//		//p->shineBlend = false;
	//		p->clr = c;
	//		p->pos = _shot->pos;
	//		AddSparkToList(p);
	//	}

	//	AddSmallSpark(_shot->pos, 2, _shot->hue);
	//}


	void KxxWin::AddGenericSpark(const Vector2 & _v, uint _count, int _hue)
	{
		const RGBA & c = hue_color(_hue);
		for(uint i=0; i<_count; i++)
		{
			float a0 = CrtRandAngle();
			float da = (CrtRandAngle() - PI) * 5;

			Spark1 * p = enew Spark1(kxxwin->sparkTex, 0.7f, a0, a0 + da, 0, frand() * 10 + 20, 1, 0);
			p->blm = BM_ADD;
			p->clr = c;
			p->pos = _v;
			
			float s = float(rand() % 300 + 50.0f) / K_LOGIC_FPS;
			p->speed.x = s * cos(a0);
			p->speed.y = s * sin(a0);
			AddSparkToList(p);
		}
	}

	void KxxWin::AddSmallSpark(const Vector2 & _v, uint _count, int _hue)
	{
		const RGBA & c = hue_color(_hue);
		for(uint i=0; i<_count; i++)
		{
			float a0 = CrtRandAngle();
			float da = (CrtRandAngle() - PI) * 5;
			Spark1 * p = enew Spark1(kxxwin->sparkTex, 0.5f, a0, a0 + da, 0, frand() * 5 + 10, 1, 0);
			p->blm = BM_ADD;
			p->clr = c;
			p->pos = _v;
			float s = float(rand() % 300 + 50.0f) / K_LOGIC_FPS;
			p->speed.x = s * cos(a0);
			p->speed.y = s * sin(a0);
			AddSparkToList(p);
		}
	}

	void KxxWin::AddFlashSpark(float _x, float _y, int _hue)
	{
		const RGBA & c = hue_color(_hue);
		Spark1 * p = enew Spark1(kxxwin->flashTex, 0.1f, 0, 0, 0, 64, 1, 1);
		p->blm = BM_ADD;
		p->pos.x = _x;
		p->pos.y = _y;
		p->clr = c;
		AddSparkToList(p);
	}

	void KxxWin::AddBlastWave(const Vector2 & _v, bool _reverse)
	{
		if(_reverse)
		{
			Spark1 * p = enew Spark1(kxxwin->blastWaveTex, 0.3f, 0, 0, 512, 0, 1, 0);
			p->blm = BM_ADD;
			p->pos = _v;
			AddSparkToList(p);
		}
		else
		{
			Spark1 * p = enew Spark1(kxxwin->blastWaveTex, 0.3f, 0, 0, 0, 512, 1, 0);
			p->blm = BM_ADD;
			p->pos = _v;
			AddSparkToList(p);
		}
	}

	void KxxWin::RenderDrops()
	{
		// NB_PROFILE_INCLUDE;
		DropList::iterator it = dropList.begin();
		while(it != dropList.end())
		{
			Drop * drop = *it;
			float bottom = drop->Bottom();
			if(drop->Bottom() < 0)
			{
				TexRef tex = dropIndicatorTex[drop->dropType];
				float alpha = 1 + bottom * 0.008f;
				if(alpha < 0.1f)
				{
					alpha = 0.1f;
				}
				// graphics->SetTexMode(TextureMode::Modulate);
				graphics->BindTex(tex);
				graphics->SetColor(1, 1, 1, alpha);
				graphics->DrawQuad(drop->Left(), 0, drop->Right(), drop->hsz.y * drop->scl.y * 2);
			}
			else
			{
				// graphics->SetTexMode(TextureMode::replace);
				drop->Render();
			}
			++it;
		}
		// graphics->SetTexMode(TextureMode::replace);
	}

	void KxxWin::DamageAllEnemy(float _damage, bool _includeEthereal)
	{
		// NB_PROFILE_INCLUDE;
		EnemyList::iterator it = enemyList.begin();
		while(it != enemyList.end())
		{
			Enemy * enemy = *it;
			if(_includeEthereal || !enemy->IsEthereal())
			{
				enemy->Damage(_damage, false);
				AddFlashSpark(enemy->pos.x, enemy->pos.y, 0);
				//playEnemyDamageSound++;
				PlayEnemyDamageSE(enemy->pos);
			}
			++it;
		}
	}


	void KxxWin::FreeAbsorbingDrops()
	{
		// NB_PROFILE_INCLUDE;
		DropList::iterator it = dropList.begin();
		while(it != dropList.end())
		{
			Drop * drop = *it;
			drop->absorbing = false;
			drop->absorbSpeed = 0;
			++it;
		}
	}

	void KxxWin::AbsorbAllDrops()
	{
		// NB_PROFILE_INCLUDE;
		DropList::iterator it = dropList.begin();
		while(it != dropList.end())
		{
			Drop * drop = *it;
			drop->absorbing = true;
			drop->absorbSpeed = 0;
			++it;
		}
	}

	void KxxWin::StopBGM()
	{
		if(audio)
		{
			audio->StopBGM();
		}
	}

	void KxxWin::PlayBGM(int _id, int _loop)
	{
		if(CanPlaySound())
		{
			Path folder = BgmFolder();
			StopBGM();

			AInfo info;
			try
			{
				string name = L"bgm_" + string(_id);
				if(FS::IsFile(folder | name + ".wav"))
				{
					audio->PlayBGM(folder | name + ".wav", _loop, &info);
				}
				else if(FS::IsFile(folder | name + ".ogg"))
				{
					 audio->PlayBGM(folder | name + ".ogg", _loop, &info);
				}
				else if(FS::IsFile(folder | name + ".mid"))
				{
					audio->PlayBGM(folder | name + ".mid", _loop, &info);
				}
				else
				{
					throwf(NB_SRC_LOC "File not found, id=%d", _id);
				}
				this->ShowNotifyMessage(L"BGM: " + info.title);
			}
			catch(const char * _exp)
			{
				this->ShowNotifyMessage(L"BGM Error: " +string(_exp));
			}
		}
	}

	void KxxWin::PlaySE_NoCheck(const stringa & _name, const Vector2 & _v, float _gain)
	{
		float x = _v.x / K_GAME_W;
		float y = _v.y / K_GAME_H;
		audio->PlaySE(_name, x, (0.5f + y*0.5f) * _gain);
	}

	void KxxWin::PlaySE(const stringa & _name, const Vector2 & _v, float _gain)
	{
		if(CanPlaySound())
		{
			PlaySE_NoCheck(_name, _v, _gain);
		}
	}

	void KxxWin::PlayEnemyDeadSE(const Vector2 & _v)
	{
		if(CanPlaySound() && enemy_dead_se_map.TryPlay(_v.x))
		{
			PlaySE_NoCheck("enemy-dead", _v, 0.4f);
		}
	}

	void KxxWin::PlayEnemyDamageSE(const Vector2 & _v)
	{
		if(CanPlaySound() && enemy_damage_se_map.TryPlay(_v.x))
		{
			PlaySE_NoCheck("enemy-damage-1", _v, 0.2f);
		}
	}

	void KxxWin::RestartStage(bool _test_boss)
	{
		// NB_PROFILE_INCLUDE;
		StopBGM();
		state.earthQuakeTimer = 0;
//		state.playerSCAvatarTimer = 0;
//		state.bossSCAvatarTimer = 0;
		state.darkScreenTimer = 0;
		state.onceRestartStage = stage->stage_index != 0;

		ReplayStateInfo & info = CurrentStageInfo();
		player->SetState(info.playerState);
		player->pos = info.playercenter;
		state.replayPosition = info.kxxWinState.replayPosition;
		joystickSequence.resize(state.replayPosition);

		E_ASSERT(!this->isReplaying);
		StartStage(stage->stage_index, _test_boss);
		if(_test_boss)
		{
			this->state.onceCheated = true;
		}
	}

	void KxxWin::OnQuitReplay()
	{
		if(isDemoReplay)
		{
			isDemoReplay = false;
			SwitchToMainMenu(true, FADE_ZOOM);
		}
		else
		{
			this->isPause = true;
			PlayBGM(0);
			int n = stage->stage_index + 1;
			if(n > K_STAGE_COUNT)
			{
				n = K_STAGE_COUNT;
			}
			SwitchToSelectReplayStageMenu(n, true);
		}
	}

	void KxxWin::OnStagePassed()
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_DEBUG
		dbg_logic_random_dump();
#endif
		if(!this->isReplaying)
		{
			UpdateCurrentStageReplaySequencePosition();
		}
		if(stage->human_readable_id() == 6 || stage->human_readable_id() == 7)
		{
			// last stage
			stageClear = true;
			this->ClearKxxSpriteLists();
			if(isPractice)
			{
				E_ASSERT(!isReplaying);
				SwitchToSelectPracticeStageMenu(stage->stage_index, true);
				DeleteStageAndPlayers();
			}
			else if(isReplaying)
			{
				OnQuitReplay();
				DeleteStageAndPlayers();
			}
			else if(CanSaveReplay())
			{
				SavePassedStageCount();
				SaveHighScore(player->state.curScore);
				PromptSaveReplay();
			}
			else
			{
#ifdef NB_DEBUG
				message(L"[kx] Can't save replay. reason: " + GetCantSaveReplayReason());
#endif
				SaveHighScore(player->state.curScore);
				OnQuitGame(0);
			}

		}
		else 
		{
			int nextStageID = stage->stage_index + 1;
			if(isPractice)
			{
				E_ASSERT(!isReplaying);
				SwitchToSelectPracticeStageMenu(nextStageID, false);
				DeleteStageAndPlayers();
			}
			else if(isReplaying)
			{
				OnQuitReplay();
				DeleteStageAndPlayers();
			}
			else
			{
				SavePassedStageCount();
				SaveHighScore(player->state.curScore);
				StartStage(nextStageID);
			}
		}
	}

	void KxxWin::SavePassedStageCount()
	{
		// NB_PROFILE_INCLUDE;
		PersistData * pd = GetCurrentPersistData();
		if(pd->maxPassStage < stage->stage_index)
		{
			if(CanSaveReplay())
			{
				pd->maxPassStage = stage->stage_index;
				SavePersistData();
			}
			else
			{
#ifdef NB_DEBUG
				message(L"[kx] Failed to save passed stage. reason: " + GetCantSaveReplayReason());
#endif
			}
		}
	}

#ifdef NB_DEBUG
	string KxxWin::GetCantSaveReplayReason()
	{
		if(isReplaying)
		{
			return L"replay mode";
		}
		else if(isPractice)
		{
			return L"practice mode";
		}
		else if(state.onceContinued)
		{
			return L"continue";
		}
		else if(state.onceRestartStage)
		{
			return L"restart stage";
		}
		else if(state.onceCheated)
		{
			return L"cheat";
		}
		else
		{
			return L"unkown reason";
		}
	}
#endif

	void KxxWin::SwitchToEndingScreen()
	{
		// NB_PROFILE_INCLUDE;
		isPause = true;
	//	audio->StopBGM();
		kxxwin->PlayBGM(98, 0);
		EndingScreen * ending = enew EndingScreen(this);
		ending->SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		ending->callback = Callback(this, &KxxWin::OnQuitEndingScreen);
		ReplaceUI(ending, FADE_SHUTTER);
		DeleteStage();
	}
	
	int KxxWin::OnQuitEndingScreen(void * _p)
	{
		// NB_PROFILE_INCLUDE;
		SwitchToCreditsScreen();
		return 0;
	}


	void KxxWin::SwitchToCreditsScreen()
	{
		// NB_PROFILE_INCLUDE;
		kxxwin->PlayBGM(99);
		CreditsScreen * credits = enew CreditsScreen(this);
		credits->SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		credits->callback = Callback(this, &KxxWin::OnQuitCreditsScreen);
		ReplaceUI(credits, FADE_SHUTTER);
	}

	int KxxWin::OnQuitCreditsScreen(void * _p)
	{
		SwitchToMainMenu(true, FADE_SHUTTER);
		return 0;
	}

	uint KxxWin::GenerateRandomSeed()
	{
		// NB_PROFILE_INCLUDE;
		double d = Time::GetTicks();
		d-= floor(d);
		return uint32(d * LOGIC_RANDOM_MAX) ^ state.joystickEventCount;
	}

	bool KxxWin::SaveReplay(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		if(!CanSaveReplay())
		{
			return false;
		}

		FileRef file = FS::OpenFile(_path, true);
		if(!file)
		{
			return false;
		}
		// E_SCOPE_RELEASE(file);
		if(!file->SetSize(0))
		{
			return false;
		}
		if(!file->Write("KXX01REPLAY", 11))
		{
			return false;
		}
		//int version = KXX01_VERSION;
		const char * p = GetCompiledDate_yymmdd();
		E_ASSERT(strlen(p) == 8);
		if(!file->Write(p, 8))
		{
			return false;
		}
		if(!file->Write(&replayStageInfo[0], sizeof(replayStageInfo)))
		{
			return false;
		}
		int size = this->joystickSequence.size();
		if(!file->Write(&size, sizeof(size)))
		{
			return false;
		}
		for(int i=0; i<size; i++)
		{
			ReplayKey & key = joystickSequence[i];
			if(!file->Write(&key, sizeof(key)))
			{
				return false;
			}
		}
		return true;
		
	}

	bool KxxWin::LoadReplay(const Path & _path, ReplayStateInfo infos[], int _stageCount, Array<ReplayKey> * _sequence)
	{
		// NB_PROFILE_INCLUDE;
		memset(&infos[0], 0, sizeof(ReplayStateInfo) * _stageCount);
		FileRef file = FS::OpenFile(_path);
		if(!file)
		{
			return false;
		}

		char buf[11];
		if(!file->Read(buf, 11)
			|| memcmp(buf, "KXX01REPLAY", 11) != 0)
		{
			message(L"[kx] (WW) KxxWin::LoadReplay(): Corrupted file: " + _path.GetBaseName());
			return false;
		}

		const char * p = GetCompiledDate_yymmdd();
		if(!file->Read(buf, 8)
			|| memcmp(buf, p, 8) != 0)
		{
			// E_ASSERT(0);
			message(L"[kx] (WW) KxxWin::LoadReplay(): File was created by old version: " + _path.GetBaseName());
#ifndef NB_DEBUG
			return false;
#endif
		}

		int size;
		if(!file->Read(&infos[0], sizeof(ReplayStateInfo) * _stageCount)
			|| !file->Read(&size, sizeof(size))
			|| size < 0 )
		{
			return false;
		}

		for(int i=0; i<_stageCount; i++)
		{
			if(!infos[i].IsValid(size))
			{
				return false;
			}
		}
		if(_sequence)
		{
			_sequence->clear();
			for(int i=0; i<size; i++)
			{
				ReplayKey key;
				key.keyState = 0;
				if(!file->Read(&key, sizeof(key)))
				{
					return false;
				}
				_sequence->push_back(key);
			}
		}
		return true;
	}

	bool KxxWin::LoadReplay(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		joystickSequence.clear();
		bool ret = LoadReplay(_path, this->replayStageInfo, K_STAGE_COUNT, &this->joystickSequence);

		return ret;
	}

	bool KxxWin::ReplayStateInfo::IsValid(int _sequenceSize) const
	{
		// NB_PROFILE_INCLUDE;
		// TODO: compete it
		if(!isReady)
		{
			return true;
		}

		if(kxxWinState.level < 0 || kxxWinState.level > 3)
		{
			E_ASSERT(0);
			return false;
		}
		if(kxxWinState.playingPlayer < 0 || kxxWinState.playingPlayer > 3)
		{
			E_ASSERT(0);
			return false;
		}

		return true;
	}

	Stage * KxxWin::CreateStage(int _stage_index)
	{
		// NB_PROFILE_INCLUDE;
		Stage * ret = 0;
		switch(_stage_index)
		{
		case 0: ret = enew Stage1(); break;
		case 1: ret = enew Stage2(); break;
		case 2: ret = enew Stage3(); break;
		case 3: ret = enew Stage4(); break;
		case 4: ret = enew Stage5(); break;
		case 5: ret = enew Stage6(); break;
		case 6: ret = enew Stage7(); break;
		default: break;
		}

		E_ASSERT(ret != 0 && ret->stage_index == _stage_index);
		return ret;
	}

	void KxxWin::PlayMenuActionSound()
	{
		if(audio)
		{
			audio->PlaySE("menu-action", 0.5f, 1.0f, 0, true);
		}
	}

	void KxxWin::PlayMenuCancelSound()
	{
		if(audio)
		{
			audio->PlaySE("menu-cancel", 0.5f, 1.0f, 0, true);
		}
	}

	void KxxWin::PlayMenuSelectSound()
	{
		if(audio)
		{
			audio->PlaySE("menu-select", 0.5f, 1.0f, 0, true);
		}
	}


	int KxxWin::PersistData::InsertTopScore(float _score)
	{
		// NB_PROFILE_INCLUDE;
		int i = 0;
		while(i< K_MAX_TOP_SCORE && topScores[i].score >= _score)
		{
			i++;
		}

		if(i < K_MAX_TOP_SCORE)
		{
			for(int j=K_MAX_TOP_SCORE-1; j>i; j--)
			{
				topScores[j] = topScores[j-1];
			}

			topScores[i].name[0] = 0;
			topScores[i].score = _score;
		}
		return i;
	}
			
	
	FileRef KxxWin::OpenResFile(Path _relativePath)
	{
		return FS::OpenFile(ResFolder() | _relativePath);
	}

//	static inline uint32 MakeReplayCheckCode(uint32 _a, float _b)
//	{
//		uint32 b1 = *((uint32*)&_b);
//		return (_a & 0x00000fff) | ((b1 & 0x00000fff) << 12);
//	}
//
//	static inline bool MatchReplayCheckCode(uint32 _code, uint _a, uint32 _b)
//	{
//		_a = _a & 0x00000fff;
//		_b = _b & 0x00000fff;
//		uint32 x = _code & 0x00000fff;
//		uint32 y = (_code >> 12) & 0x00000fff;
//#ifdef NB_DEBUG
//		if(_a != x)
//		{
//			DumpLastRandomCall();
//			E_ASSERT(0); // random seed mismatch
//			return false;
//		}
//		if(_b != y)
//		{
//			DumpLastRandomCall();
//			E_ASSERT(0); // score mismatch
//			return false;
//		}
//		return true;
//#else
//		return _a == x && _b == y;
//#endif
//	}

	void KxxWin::PushReplayKey(bool _force)
	{
		E_ASSERT(!isReplaying);
		uint state1 = this->GetJoystickState(0) & (~MAPED_BUTTON_PAUSE);
		if(_force || state1 != state.lastJoystickState)
		{
			state.lastJoystickState = state1;
			ReplayKey key;
			key.logicTimer = state.logicTimer;
			key.keyState = state.lastJoystickState;
			key.check1 = logic_random_get_seed() & 0x00000fff;
			uint32 c2 = *((uint32*) &player->state.curScore);
			key.check2 = c2 & 0x00000fff;
#ifdef K_CFG_REPLAY_EXTRA_CHECK
			memcpy(&key.winState, &this->state, sizeof(key.winState));
			memcpy(&key.playerState, &player->state, sizeof(key.playerState));
#endif
			joystickSequence.push_back(key);
			state.replayPosition++;
		}
	}

	bool KxxWin::PopReplayKey()
	{
		E_ASSERT(isReplaying);
		if(state.replayStopPosition > (int)joystickSequence.size())
		{
			message(L"[kx] (WW) This record is broken: state.replayStopPosition > (int)joystickSequence.size()");
			goto _BROKEN;
		}

		if(state.replayPosition >= state.replayStopPosition)
		{
			return false; // end of replay
		}

		{
			ReplayKey & key = joystickSequence[state.replayPosition];

			if(key.logicTimer != state.logicTimer)
			{
				E_ASSERT(key.logicTimer > state.logicTimer);
				state.joystickStateForReplay = state.lastJoystickState;
				return true;
			}

			// check random series and score
			bool check1Pass = (logic_random_get_seed() & 0x00000fff) == key.check1;
			uint32 c2 = *((uint32*) &player->state.curScore);
			bool check2Pass = (c2 & 0x00000fff) == key.check2;
			if(!check1Pass || !check2Pass)
			{
				message(L"[kx] (WW) This record is broken: !check1Pass || !check2Pass");
				goto _BROKEN;
			}

#ifdef K_CFG_REPLAY_EXTRA_CHECK
			// check logic timer
			if(state.replayPosition < state.replayStopPosition - 1)
			{
				if(key.winState.logicTimer != state.logicTimer
	//				|| key.winState.subLogicPass != state.subLogicPass
					|| key.winState.earthQuakeTimer != state.earthQuakeTimer
					|| 0 != memcmp(&key.playerState, &player->state, sizeof(key.playerState)))
				{
					message(L"[kx] (WW) This record is broken: state mismatch");
					goto _BROKEN;
				}
			}
#endif
			state.lastJoystickState = key.keyState;
			state.replayPosition++;
			state.joystickStateForReplay = state.lastJoystickState;
			return true;
		}

_BROKEN:
		ShowNotifyMessage(_TT("This record is broken."), 3.0f, 2);
		return false;
	}

	float KxxWin::GetCurrentAPM()
	{
		return state.logicTimer > K_LOGIC_FPS * 3 ? float(state.replayPosition) * K_LOGIC_FPS * 60.0f / float(state.logicTimer) : 0;
	}

	KxxWin::ReplayStateInfo & KxxWin::CurrentStageInfo()
	{
		return replayStageInfo[stage->stage_index];
	}

	void KxxWin::DarkScreen(float _second, float _maxDark, float fadeFrac)
	{
		state.darkMaxAlpha = _maxDark;
		state.darkFadeFrac = fadeFrac;
		state.darkScreenTime0 = uint32(_second * K_LOGIC_FPS);
		state.darkScreenTimer = state.darkScreenTime0;
	}

	void KxxWin::StepNonlogic()
	{
		if(uiToDelete)
		{
			delete uiToDelete;
			uiToDelete = 0;
		}
		if(currentUI)
		{
			currentUI->Step();
		}

		StepTimers();

		enemy_dead_se_map.Step();
		enemy_damage_se_map.Step();

		StepLensEffect();

		if(fade && !fade->Step())
		{
			delete fade;
			fade = 0;
		}

		if(blurScreenSteps && blurScreenTimer)
		{
			blurScreenTimer--;
			if(blurScreenTimer == 0)
			{
				blurScreenImage->src.Blur(2);
				blurScreenTex = graphics->LoadTexFromLoader(blurScreenImage); // TODO: avoid copy image
				blurScreenTimer = 20;	
				blurScreenSteps--;
			}
		}
		if(player)
		{
			digitalRoller->Set(int(player->state.curScore));
			digitalRoller->Step();
		}
	}

	void KxxWin::RenderRoot()
	{

#ifdef DEBUG_RENDER_NONE
		return;
#endif
		
		Graphics * g = graphics;
		if(fade)
		{
			if(fade->fboB)
			{
				// screen => fade
				if(g->SetFbo(fade->fboB))
				{
					if(g->BeginScene())
					{
						RenderClearBuffer();
						RenderUIBack();
						RenderGameBack();
						RenderGameFront();
						RenderUIFront();
						g->EndScene();
					}
					g->SetFbo(0);
				}

				// fade => screen
				if(g->BeginScene())
				{
					SetToUIViewport();
					SetToUIProjection();
					fade->Render();
					g->EndScene();
				}
			}
			else // overide fade
			{
				if(g->BeginScene())
				{
					if(fade->overlay)
					{
						RenderClearBuffer();
						RenderUIBack();
						RenderGameBack();
						RenderGameFront();
						RenderUIFront();
					}
					SetToUIViewport();
					SetToUIProjection();
					fade->Render();
					g->EndScene();
				}

			}
		}
		else if(lensEffect && lensEffect->fbo)
		{
			// screen => lensEffect
			if(g->SetFbo(lensEffect->fbo))
			{
				if(g->BeginScene())
				{
					RenderClearBuffer();
					RenderUIBack();
					RenderGameBack();
					g->EndScene();
				}
				g->SetFbo(0);
			}

			// lensEffec=> screen
			if(g->BeginScene())
			{
				//SetTo2DView();
				
				SetToUIViewport();
				SetToUIProjection();
				lensEffect->Render();	
				SetToGameViewport();
				SetToGame2DProjection();

				RenderGameFront();
				RenderUIFront();
				g->EndScene();
			}
		}
		else if(pass2 && this->IsInGame())
		{
			if(g->SetFbo(pass2->fbo))
			{
				if(g->BeginScene())
				{
					RenderClearBuffer();
					RenderUIBack();
					RenderGameBack();
					RenderGameFront();
					g->EndScene();
				}
				g->SetFbo(0);
			}

			if(g->BeginScene())
			{
				SetToUIViewport();
				SetToUIProjection();
				pass2->Render();
				RenderUIFront();
				g->EndScene();
			}
		}
		else
		{
			// render directly
			if(g->BeginScene())
			{
				RenderClearBuffer();
				RenderUIBack();
				RenderGameBack();
				RenderGameFront();
				RenderUIFront();
				g->EndScene();
			}
		}
	}

	void KxxWin::SetFade(Fade * _fade, float _secound)
	{
		// NB_PROFILE_INCLUDE;
		if(fade)
		{
			delete fade;
		}
		fade = _fade;
		if(_fade)
		{
			_fade->Init(graphics, K_VIEW_W, K_VIEW_H, uint32(_secound * 60));
			if(_fade->fboA)
			{
				// screen => fade
				Graphics * g = graphics;
				g->SetFbo(_fade->fboA);
				if(g->BeginScene())
				{
					RenderClearBuffer();
					RenderUIBack();
					RenderGameBack();
					RenderGameFront();
					RenderUIFront();
					g->EndScene();
				}
				g->SetFbo(0);
			}
		}
	}

//	void KxxWin::SetLensEffect(LensEffect * _le)


	void KxxWin::DeletePlayers()
	{
		delete player;
		player = 0;
	}

	void KxxWin::DeleteStage()
	{
		delete stage;
		stage = 0;
	}

	void KxxWin::DeleteStageAndPlayers()
	{
		delete player;
		player = 0;
		delete stage;
		stage = 0;
	}


	Path KxxWin::ResFolder() const
	{
		return Env::GetResourceFolder();
	}


	Path KxxWin::BgmFolder() const
	{
		return Env::GetResourceFolder() | L"bgm";
	}

	Ani5 KxxWin::LoadAni5(const string & _name)
	{
		E_ASSERT(ani5PathIndexer);
		Ani5 ret; 
		Path path = ani5PathIndexer->GetPath(_name);
		if(!(path.IsValid() && ret.Load(graphics, K_LOGIC_FPS, path)))
		{
			message(L"[kx] (WW) Failed to load Ani5: \"" + _name + L"\"");
		}
		return ret;
	}

	Ani KxxWin::LoadAni(const string & _name)
	{
		E_ASSERT(aniPathIndexer);
		Ani ret;
		Path path = aniPathIndexer->GetPath(_name);
		if(!(path.IsValid() && ret.Load(graphics, K_LOGIC_FPS, path)))
		{
			message(L"[kx] (WW) Failed to load Ani: \"" + _name + L"\"");
		}
		return ret;
	}

	int KxxWin::OnGraphicsError(void * _p)
	{
		string & s = *((string*) _p);
		message(L"[kx] (WW) Graphics Error: " + s);
		return 0;
	}

	void KxxWin::AddFloatText(float _x, float _y, FontRef _font, const string & _s, uint _life, float _speed, float _r, float _g, float _b, float _a)
	{
		FloatText * ft = enew FloatText;

		ft->clr.r = _r;
		ft->clr.g = _g;
		ft->clr.b = _b;
		ft->clr.a = _a;

		ft->text = _s;

		ft->font = _font;

		ft->pos.x = _x;
		ft->pos.y = _y;

		ft->pos.x = (float)(int)ft->pos.x;
		ft->pos.y = (float)(int)ft->pos.y;

		ft->life = _life;
		ft->speed = _speed;

		kxxwin->AddSparkToList(ft, RL_GAME_TEXT);
	}

	void KxxWin::CaptureScreenShot()
	{
		if(audio)
		{
			audio->Pause();
		}
		Path path;
		string date = Time::Current().GetString(L"%Y-%m-%d-%H-%M-%S", false);
		int n = -1;
		do
		{
			n++;
			path = Env::GetDataFolder() | L"screenshot" | (date + L"-" + string(n) + L".png");
		}while(FS::IsFile(path));
		bool succeeded = false;
		if(graphics->GetCapabilities().frameBufferObject)
		{
			int vx, vy, vw, vh;
			graphics->GetViewport(vx, vy, vw, vh);
			FboRef fbo = graphics->CreateFbo(vw, vh);
			if(fbo)
			{
				graphics->SetFbo(fbo);
				if(graphics->BeginScene())
				{
					RenderClearBuffer();
					RenderUIBack();
					RenderGameBack();
					RenderGameFront();
					RenderUIFront();
					graphics->EndScene();

					TexRef tex = fbo->GetTex();
					Image img;
					if(tex && tex->GetImage(img))
					{
						img.Save(path, false);
						succeeded = true;
					}
				}
				graphics->SetFbo(0);
			}
		}

		if(succeeded)
		{
			ShowNotifyMessage(path.GetString());
		}
		else
		{
			ShowNotifyMessage(L"ERROR: Failed to capture screen shot.");
		}
		if(audio)
		{
			audio->Resume();
		}
	}

	void KxxWin::GenerateShotVelTable()
	{
		float a[] = {60, 80, 100};
		float b[] = {240, 320, 400};
		float x0 = a[state.level] / K_LOGIC_FPS;
		float x1 = b[state.level] / K_LOGIC_FPS;
		float d = x1 - x0;
		for(int i=0; i<100; i++)
		{
			float f = (i * 0.01f);
			g_kxx_shot_vel[i] = x0 + d * f;
			//E_TRACE_LINE(string(g_kxx_shot_vel[i]*K_LOGIC_FPS));
		}
	}

	Enemy * KxxWin::FindNearestEnemy(const Vector2 & _pos, float _min_dis)
	{
		Enemy * ret = 0;
		EnemyList::iterator it1 = enemyList.begin();
		float distance = 1000000.0f;
		while(it1 != enemyList.end())
		{
			Enemy * enemy = *it1;
			if(!enemy->IsEthereal())
			{
				float d = (enemy->pos - _pos).length();
				if(d >= _min_dis && d < distance)
				{
					distance = d;
					ret = enemy;
				}
			}
			++it1;
		}

		return ret;
	}

	int KxxWin::FindSomeNearEnemy(Enemy * ret[], int _n, const Vector2 & _pos, float _min_dis)
	{
		E_ASSERT(_n <= 3);
		int ret_n = 0;
		memset(ret, 0, sizeof(Enemy*) * _n);
		float distance[3] = {1000000.0f, 1000000.0f, 1000000.0f};
		EnemyList::iterator it1 = enemyList.begin();
		while(it1 != enemyList.end())
		{
			Enemy * enemy = *it1;
			if(!enemy->IsEthereal())
			{
				float d = (enemy->pos - _pos).length();
				if(d >= _min_dis && d < distance[_n-1])
				{
					ret_n++;
					int i;
					for(i = _n - 2; i >= 0 && d < distance[i]; i--)
					{
						distance[i+1] = distance[i];
						ret[i+1] = ret[i];
					}
					i++;
					distance[i] = d;
					ret[i] = enemy;
				}
			}
			++it1;
		}

		return ret_n > _n ? _n : ret_n;
	}

	void KxxWin::OnMouseMove(float _x, float _y)
	{
#ifdef NB_DEBUG
		string s = string::format(L"  MOUSE: %.0f, %.0f (%.0f, %.0f)", _x, _y, _x - K_GAME_X, _y - K_GAME_Y);
		SetTitle(appNameVersion + s);
#endif
	}

	void KxxWin::AddSparkToList(Sprite * _p, int _layer)
	{
		_p->AddToRenderList(_layer);
		sparkList.push_back(_p);
	}

	void KxxWin::AddDropToList(Drop * _p, int _layer)
	{
		dropList.push_back(_p);
	}	

	bool KxxWin::AddEnemyShotToList(EnemyShot * _p, bool _canblock, int _layer)
	{
		if(_canblock && player->BlockEnemyShot(_p))
		{
			this->AddSmallSpark(_p->pos, 3, _p->hue);
			delete _p;
			return false;
		}
		else
		{
			_p->AddToRenderList(_layer);
			enemyShotList.push_back(_p);
			return true;
		}
	}

	void KxxWin::AddPlayerShotToList(PlayerShot * _p, int _layer)
	{
		_p->AddToRenderList(_layer);
		playerShotList.push_back(_p);
	}

	void KxxWin::AddPlayerSCToList(PlayerShot * _p, int _layer)
	{
		_p->AddToRenderList(_layer);
		playerSCList.push_back(_p);
	}

	void KxxWin::AddEnemyToList(Enemy * _p, int _layer)
	{
		_p->AddToRenderList(_layer);
		enemyList.push_back(_p);
	}

	void KxxWin::AddUISpriteToList(Sprite * _p, int _layer)
	{
		_p->AddToRenderList(_layer);
		uiSpriteList.push_back(_p);
	}

	void KxxWin::AddLogicActionToList(Sprite * _p)
	{
		this->logicStepList.push_back(_p);
	}

	void KxxWin::AddCircleEnemyShot(const Vector2 & _v, float _angle,  float _speed, int _splitCount, int _shotTexIndex, bool _rotate)
	{
		AddPieEnemyShot(_v, _angle, _speed, _splitCount, _splitCount, _shotTexIndex, _rotate);
	}

	// down
	void KxxWin::AddPieEnemyShot(const Vector2 & _v, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate)
	{
		AddPieEnemyShot(_v, PI * 0.5f, _speed, _splitCount, _shotCount, _shotTexIndex, _rotate);
	}

	void KxxWin::AddCircleEnemyShot(const Vector2 & _v, float _speed, int _splitCount, int _shotTexIndex, bool _rotate)
	{
		AddPieEnemyShot(_v, _speed, _splitCount, _splitCount, _shotTexIndex, _rotate);
	}

	// snipe
	void KxxWin::AddPieEnemySnipe(const Vector2 & _v, float _speed, int _splitCount, int _shotCount, int _shotTexIndex, bool _rotate)
	{
		AddPieEnemyShot(_v, (player->pos - _v).Angle(), _speed, _splitCount, _shotCount, _shotTexIndex, _rotate);
	}

	void KxxWin::AddCircleEnemySnipe(const Vector2 & _v, float _speed, int _splitCount, int _shotTexIndex, bool _rotate)
	{
		AddPieEnemySnipe(_v, _speed, _splitCount, _splitCount, _shotTexIndex, _rotate);
	}

	void KxxWin::EarthQuake(float _second)
	{
		state.earthQuakeTimer+= uint32(_second * K_LOGIC_FPS);
	}

	Tex * KxxWin::LoadTex(const string & _name, bool _delay)
	{
		return graphics->LoadTexFromPool(_name, _delay);
	}

	int KxxWin::PeekDemo(int _id, uint32 & _time0, uint32 & _time1)
	{
		FileRef file = kxxwin->OpenResFile(Path(L"./demo/demo.txt"));
		if(!file)
		{
			message(L"[kx] (WW) Failed to load demo.txt");
			return 0;
		}

		stringa lineA;
		string line;
		StringArray words;
		int line_num = -1;
		while(file->ReadLine(lineA))
		{
			line = string(lineA, CHARSET_UTF8);
			int n = line.find(L'#');
			if(n != -1)
			{
				line = line.substr(0, n);
			}
			line.trim();
			if(line.empty())
			{
				continue;
			}
			line_num++;
			if(line_num < _id)
			{
				continue;
			}

			words = Split(line, L" \t");
			int wordCount = words.size();
			if(wordCount == 3)
			{
				_time0 = words[1].to_int() * K_LOGIC_FPS;
				_time1 = words[2].to_int() * K_LOGIC_FPS;
				int stage_id = words[0].to_int();
				if(stage_id > 0 && stage_id < K_EXTRA_STAGE_INDEX + 1
					&& _time1 > _time0)
				{
					return stage_id;
				}
				else
				{
					return 0;
				}
			}
		}
		return 0;
	}


	bool KxxWin::StartDemoReplay()
	{
		int id = demo_id;
		demo_id = (id+1) % 5;;
		uint32 demoTime0, demoTime1;
		int stage_id = PeekDemo(id, demoTime0, demoTime1);
		if(stage_id == 0)
		{
			return false;
		}

		if(!LoadReplay(ResFolder() | L"demo/demo.kxx01")
			|| !replayStageInfo[stage_id-1].isReady)
		{
			return false;
		}
		uint32 stateStartTime = replayStageInfo[stage_id-1].kxxWinState.logicTimer;
		if(demoTime1 < stateStartTime)
		{
			return false;
		}
		ReplaceUI(0, FADE_NONE);
		isSilient = true;
		demoReplayTimer = demoTime1;
		isDemoReplay = true;
		isReplaying = true;
		StartStage(stage_id-1);

		DemoFastForward(demoTime0);
		isSilient = false;

		PlayBGM(666);

		ShowNotifyMessage(L"Demo No." + string(id+1), 5);
		return true;
	}
}
