
#pragma once

#include <nbug/math/math.h>
#include <nbug/gl/color.h>

namespace e
{
	class RenderList;
	extern RenderList * rlist;
	class Graphics;
	extern Graphics * graphics;
	class KxxWin;
	extern KxxWin * kxxwin;
	//extern asILuaEngine * script;
	//extern asILuaContext * ctx;

	// the logic of record depend on consistent random series.
	#define LOGIC_RANDOM_MAX 32768
	uint32 logic_random_get_seed();
#ifdef NB_DEBUG
#	define logic_random_set_seed(x) dbg_logic_random_set_seed((x), __FILE__, __LINE__)
#	define logic_random_int() dbg_logic_random(__FILE__, __LINE__)
#	define logic_random_float() dbg_logic_random_float(__FILE__, __LINE__)
	void dbg_logic_random_dump();
	void dbg_logic_random_set_seed(uint32 _seed, const char * _file, int _line);
	int dbg_logic_random(const char * _file, int _line);
	float dbg_logic_random_float(const char * _file, int _line);
	//void DumpLastRandomCall();
#else
	void logic_random_set_seed(uint32 _seed);
	int logic_random_int();
	float logic_random_float();
#endif


	enum
	{
		MAPED_BUTTON_LEFT  = 0x01,
		MAPED_BUTTON_RIGHT = 0x02,
		MAPED_BUTTON_UP    = 0x04,
		MAPED_BUTTON_DOWN  = 0x08,
		MAPED_BUTTON_DIRECTION_MASK = 0x0f,
		MAPED_BUTTON_FIRE  = 0x10, // FIRE
		MAPED_BUTTON_SLOW  = 0x20, // SLOW
		MAPED_BUTTON_SC    = 0x40, // SC
		MAPED_BUTTON_PAUSE = 0x80, // PAUSE
	};

#undef PS2PF
#undef S2F

static const int K_LOGIC_FPS_MUL  = 4;
static const int K_RENDER_FPS = 60;
static const int K_LOGIC_FPS = 240;

	static inline float PS2PF(float _delta) // speed per second => speed per logic frame
	{
		return _delta / K_LOGIC_FPS;
	}

	static inline int S2F(float _second) // _second => logic frame
	{
		return int(K_LOGIC_FPS * _second);
	}

	static inline float LogicRandomAngle()
	{
		return PI * 2 * logic_random_float();
	}

	static inline float CrtRandAngle()
	{
		return PI * 2 * frand();
	}

#define K_SHOT_TYPE_COUNT 17
#define K_SHOT_COLOR_COUNT 12

#define K_GAME_X 40
#define K_GAME_Y 21
#define K_GAME_W 480
#define K_GAME_H 560
#define K_GAME_XC (K_GAME_W * 0.5f)
#define K_GAME_YC (K_GAME_H * 0.5f)
#define K_INIT_Y (K_GAME_H - 100)
#define K_VIEW_W 800 // befor scale window
#define K_VIEW_H 600 // befor scale window

#define K_GAME_TEX_X0 (float(K_GAME_X)/float(K_VIEW_W))
#define K_GAME_TEX_Y0 (float(K_GAME_Y)/float(K_VIEW_H))
#define K_GAME_TEX_X1 (float(K_GAME_X+K_GAME_W)/float(K_VIEW_W))
#define K_GAME_TEX_Y1 (float(K_GAME_Y+K_GAME_H)/float(K_VIEW_H))

#define SPELL_CARD_DURATION_S 4
#define SPELL_CARD_DURATION (SPELL_CARD_DURATION_S * K_LOGIC_FPS)
#define SPELL_CARD_RECAST_LINE (K_LOGIC_FPS / 2)
#define SPELL_CARD_EARTH_QUAKE_DURATION_S (1.5f)
#define K_PLAYER_BORN_TIME uint32(1.0f*K_LOGIC_FPS)
#define K_PLAYER_BORN_TRANSPARENT_TIME (4*K_LOGIC_FPS) //
#define SPELL_CARD_DURATION_TRANSPARENT_DURATION (3*K_LOGIC_FPS)
#define K_GRAZE_UNAVAILABLE_SPAN (K_LOGIC_FPS) //
#define K_GRAZE_RADIUS 14
#define K_GPOINT_RADIUS 2
#define K_SHOT_UNAVAILABLE_SPAN (K_LOGIC_FPS / 3) //

#define POINT_GAIN_ABSORB_SMALL_POINT 1000
#define POINT_GAIN_HIT_ENEMY (POINT_GAIN_ABSORB_SMALL_POINT*0.005f) //
#define POINT_GAIN_ABSORB_TINY_POINT (POINT_GAIN_ABSORB_SMALL_POINT * 0.1f)
#define POINT_GAIN_KILL_ENEMY0 (POINT_GAIN_ABSORB_SMALL_POINT)
#define POINT_GAIN_GRAZE POINT_GAIN_ABSORB_TINY_POINT //

#define TITLE_DURATION 300
#define K_INIT_PLAYER_LIFE 2
#define DEFALUT_DROP_THROW_SPEED 4.0f
#define K_ITEM_GET_LINE 0.285f
#define K_MAX_REPLAY_SLOT 16
#define K_STAGE_COUNT 7 // include EX state
//#define K_IMPLETMENTED_STAGE_COUNT 7
#define K_EXTRA_STAGE_INDEX (K_STAGE_COUNT - 1)
#define K_MAX_TOP_SCORE 3

#define K_WIN_UNIQUE_PROP L"KXX_UNIQUE"

#define K_LEVEL_COUNT 3

#define K_PLAYER_SPEED_FAST PS2PF(280)
#define K_PLAYER_SPEED_SLOW PS2PF(130)

	static inline bool IsOutOfGameArea(const Vector2 & _v, float _extend)
	{
		return _v.x < -_extend || _v.x > K_GAME_W + _extend || _v.y < -_extend || _v.y > K_GAME_H + _extend;
	}

	////
	//enum
	//{
	//	//
	//	IFG_KEDAMA_LEFT_PINK,
	//	IFG_KEDAMA_RIGHT_PINK,
	//	IFG_KEDAMA_PONDER,
	//	IFG_KEDAMA_FALL_DOWN1,
	//	IFG_KEDAMA_FALL_DOWN2,
	//	_IFG_MAX,
	//};

	enum
	{
		DROP_NONE, // 0
		DROP_LIFE, // 1
		DROP_BIG_POWER, // 2
		DROP_SC, // 3
		DROP_PET, // 4
		DROP_FULL, // 5
		DROP_SMALL_POWER,// 6
		DROP_SMALL_POINT, // 7
		DROP_TINY_POINT, // 8
		DROP_EC_A, // 9
		DROP_EC_B, // 10
		DROP_EC_C, // 11
		DROP_EC_D, // 12
		_DROP_MAX,
	};

	float shot_vel(int _n);
	const RGBA & hue_color(int _hue);

#define INDEX_A 0
#define INDEX_B 1
#define INDEX_C 2
#define INDEX_D 3
#define INDEX_E 4
#define INDEX_F 5
#define INDEX_G 6
#define INDEX_H 7
#define INDEX_I 8
#define INDEX_J 9
#define INDEX_K 10
#define INDEX_L 11

}

