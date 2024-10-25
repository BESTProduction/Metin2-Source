#ifndef __INC_AFFECT_H
#define __INC_AFFECT_H

class CAffect
{
public:
	DWORD	dwType;
	BYTE    bApplyOn;
	long    lApplyValue;
	DWORD   dwFlag;
	long	lDuration;
	long	lSPCost;

	static CAffect* Acquire();
	static void Release(CAffect* p);
};

enum EAffectTypes
{
	AFFECT_NONE,

	AFFECT_MOV_SPEED = 200,
	AFFECT_ATT_SPEED,
	AFFECT_ATT_GRADE,
	AFFECT_INVISIBILITY,
	AFFECT_STR,
	AFFECT_DEX,			// 205
	AFFECT_CON,
	AFFECT_INT,
	AFFECT_FISH_MIND_PILL,

	AFFECT_POISON,
	AFFECT_STUN,		// 210
	AFFECT_SLOW,
	AFFECT_DUNGEON_READY,
	AFFECT_DUNGEON_UNIQUE,

	AFFECT_BUILDING,
	AFFECT_REVIVE_INVISIBLE,	// 215
	AFFECT_FIRE,
	AFFECT_CAST_SPEED,
	AFFECT_HP_RECOVER_CONTINUE,
	AFFECT_SP_RECOVER_CONTINUE,

	AFFECT_POLYMORPH,		// 220

	AFFECT_WAR_FLAG,		// 222

	AFFECT_BLOCK_CHAT,		// 223
	AFFECT_CHINA_FIREWORK,

	AFFECT_BOW_DISTANCE,	// 225
	AFFECT_DEF_GRADE,		// 226
#ifdef ENABLE_WOLFMAN_CHARACTER
	AFFECT_BLEEDING,		// 227
#endif
	AFFECT_PREMIUM_START = 500,
	AFFECT_EXP_BONUS = 500,
	AFFECT_ITEM_BONUS = 501,
	AFFECT_SAFEBOX = 502,  // PREMIUM_SAFEBOX,
	AFFECT_AUTOLOOT = 503,	// PREMIUM_AUTOLOOT,
	AFFECT_FISH_MIND = 504,	// PREMIUM_FISH_MIND,
	AFFECT_MARRIAGE_FAST = 505,
	AFFECT_GOLD_BONUS = 506,
	AFFECT_PREMIUM_END = 509,

	AFFECT_MALL = 510,
	AFFECT_NO_DEATH_PENALTY = 511,
	AFFECT_SKILL_BOOK_BONUS = 512,
	AFFECT_SKILL_NO_BOOK_DELAY = 513,

	AFFECT_HAIR = 514,
	AFFECT_COLLECT = 515,
	AFFECT_EXP_BONUS_EURO_FREE = 516,
	AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,
	AFFECT_UNIQUE_ABILITY = 518,

	AFFECT_CUBE_1,
	AFFECT_CUBE_2,
	AFFECT_CUBE_3,
	AFFECT_CUBE_4,
	AFFECT_CUBE_5,
	AFFECT_CUBE_6,
	AFFECT_CUBE_7,
	AFFECT_CUBE_8,
	AFFECT_CUBE_9,
	AFFECT_CUBE_10,
	AFFECT_CUBE_11,
	AFFECT_CUBE_12,

	AFFECT_BLEND,

	AFFECT_HORSE_NAME,

	AFFECT_AUTO_HP_RECOVERY = 534,
	AFFECT_AUTO_SP_RECOVERY = 535,

	AFFECT_DRAGON_SOUL_QUALIFIED = 540,
	AFFECT_DRAGON_SOUL_DECK_0 = 541,
	AFFECT_DRAGON_SOUL_DECK_1 = 542,
#ifdef ENABLE_DS_SET_BONUS
	AFFECT_DS_SET = 543,
#endif
#ifdef ENABLE_NAMING_SCROLL
	AFFECT_NAMING_SCROLL_MOUNT = 544,
	AFFECT_NAMING_SCROLL_PET = 545,
	AFFECT_NAMING_SCROLL_BUFFI = 550,
#endif
#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
	AFFECT_PREMIUM = 546,
#endif
#ifdef ENABLE_VOTE4BUFF
	AFFECT_VOTE4BUFF = 547,
#endif
#ifdef ENABLE_EXTENDED_BATTLE_PASS
	AFFECT_BATTLEPASS = 548,
#endif
#ifdef ENABLE_LEADERSHIP_EXTENSION
	AFFECT_PARTY_BONUS = 549,
#endif
#ifdef ENABLE_AUTO_PICK_UP
	AFFECT_AUTO_PICK_UP = 551,
#endif
#ifdef ENABLE_ADDSTONE_NOT_FAIL_ITEM
	AFFECT_ADDSTONE_SUCCESS_ITEM_USED = 552,
#endif
#ifdef ENABLE_METIN_QUEUE
	AFFECT_METIN_QUEUE = 553,
#endif
	AFFECT_RAMADAN_ABILITY = 300,
	AFFECT_RAMADAN_RING = 301,

	AFFECT_NOG_ABILITY = 302,
	AFFECT_HOLLY_STONE_POWER = 303,

#if defined(ENABLE_EXTENDED_BLEND_AFFECT) || defined(ENABLE_BLEND_AFFECT)
	AFFECT_BLEND_POTION_1 = 304,
	AFFECT_BLEND_POTION_2 = 305, 
	AFFECT_BLEND_POTION_3 = 306, 
	AFFECT_BLEND_POTION_4 = 307, 
	AFFECT_BLEND_POTION_5 = 308, 
	AFFECT_BLEND_POTION_6 = 309, 

	AFFECT_BLEND_POTION_7 = 310, // Monster
	AFFECT_BLEND_POTION_8 = 311, // Metinstone
	AFFECT_BLEND_POTION_9 = 312, // Boss

	AFFECT_ENERGY = 313,
	AFFECT_DRAGON_GOD_1 = 314,
	AFFECT_DRAGON_GOD_2 = 315, 
	AFFECT_DRAGON_GOD_3 = 316, 
	AFFECT_DRAGON_GOD_4 = 317,
	AFFECT_CRITICAL = 318,
	AFFECT_PENETRATE = 319,
	AFFECT_ATTACK_SPEED = 320,
	AFFECT_MOVE_SPEED = 321,
	AFFECT_WATER_POTION_1 = 322,
	AFFECT_WATER_POTION_2 = 323,

	AFFECT_POTION_ELEMENT_ICE = 324,
	AFFECT_POTION_ELEMENT_FIRE = 325,
	AFFECT_POTION_ELEMENT_WIND = 326,
	AFFECT_POTION_ELEMENT_DARK = 327,
	AFFECT_POTION_ELEMENT_ELEC = 328,
	AFFECT_POTION_ELEMENT_EARTH = 329,

	AFFECT_FISH_ITEM_01 = 330,
	AFFECT_FISH_ITEM_02 = 331,
	AFFECT_FISH_ITEM_03 = 332,
	AFFECT_FISH_ITEM_04 = 333,
	AFFECT_FISH_ITEM_05 = 334,
	AFFECT_FISH_ITEM_06 = 335,
	AFFECT_FISH_ITEM_07 = 336,
	AFFECT_FISH_ITEM_08 = 337,
	AFFECT_FISH_ITEM_09 = 338,
	AFFECT_FISH_ITEM_10 = 339,
	AFFECT_FISH_ITEM_11 = 340,
	AFFECT_FISH_ITEM_12 = 341,
#endif

	AFFECT_QUEST_START_IDX = 1000
};

enum EAffectBits
{
	AFF_NONE,

	AFF_YMIR,
	AFF_INVISIBILITY,
	AFF_SPAWN,

	AFF_POISON,
	AFF_SLOW,
	AFF_STUN,

	AFF_DUNGEON_READY,
	AFF_DUNGEON_UNIQUE,

	AFF_BUILDING_CONSTRUCTION_SMALL,
	AFF_BUILDING_CONSTRUCTION_LARGE,
	AFF_BUILDING_UPGRADE,

	AFF_MOV_SPEED_POTION,
	AFF_ATT_SPEED_POTION,

	AFF_FISH_MIND,

	AFF_JEONGWIHON,
	AFF_GEOMGYEONG,
	AFF_CHEONGEUN,
	AFF_GYEONGGONG,
	AFF_EUNHYUNG,
	AFF_GWIGUM,
	AFF_TERROR,
	AFF_JUMAGAP,
	AFF_HOSIN,
	AFF_BOHO,
	AFF_KWAESOK,
	AFF_MANASHIELD,
	AFF_MUYEONG,
	AFF_REVIVE_INVISIBLE,
	AFF_FIRE,
	AFF_GICHEON,
	AFF_JEUNGRYEOK,
	AFF_TANHWAN_DASH,
	AFF_PABEOP,
	AFF_CHEONGEUN_WITH_FALL,
	AFF_POLYMORPH,
	AFF_WAR_FLAG1,
	AFF_WAR_FLAG2,
	AFF_WAR_FLAG3,

	AFF_CHINA_FIREWORK,
	AFF_HAIR,
	AFF_GERMANY,
	AFF_RAMADAN_RING,

#ifdef ENABLE_WOLFMAN_CHARACTER
	AFF_BLEEDING,			// 42
	AFF_RED_POSSESSION,		// 44
	AFF_BLUE_POSSESSION,	// 43
#endif
#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
	AFF_PREMIUM,
#endif
#ifdef ENABLE_EXTENDED_BATTLE_PASS
	AFF_BATTLEPASS,
#endif
#ifdef ENABLE_METIN_QUEUE
	AFF_METIN_QUEUE,
#endif
	AFF_BITS_MAX
};

extern void SendAffectAddPacket(LPDESC d, CAffect* pkAff);

// AFFECT_DURATION_BUG_FIX
enum AffectVariable
{
	INFINITE_AFFECT_DURATION_REALTIME = 2 * 365 * 24 * 60 * 60,
	INFINITE_AFFECT_DURATION = 60 * 365 * 24 * 60 * 60,
#ifdef ENABLE_EXTENDED_BLEND_AFFECT
	INFINITE_AFFECT_DURATION_BLEEND = 8640000
#endif
};
// END_AFFECT_DURATION_BUG_FIX

#endif
