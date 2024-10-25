#include "stdafx.h"

#include <math.h>
#include "ProtoReader.h"

#include "CsvReader.h"

#include <sstream>

using namespace std;

#define ENABLE_NUMERIC_FIELD
#ifdef ENABLE_NUMERIC_FIELD
bool _IsNumericString(const std::string& trimInputString)
{
	if (trimInputString.empty())
		return false;
	bool isNumber = true;
	for (auto& c : trimInputString)
	{
		if (!std::isdigit(c) && c != '-' && c != '+')
		{
			isNumber = false;
			break;
		}
	}
	return isNumber;
}
#endif

inline string trim_left(const string& str)
{
	string::size_type n = str.find_first_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(n, str.length());
}

inline string trim_right(const string& str)
{
	string::size_type n = str.find_last_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(0, n + 1);
}

string trim(const string& str) { return trim_left(trim_right(str)); }

static string* StringSplit(string strOrigin, string strTok)
{
	unsigned int cutAt;
	int index = 0;
	string* strResult = new string[30];

	while ((cutAt = strOrigin.find_first_of(strTok)) != strOrigin.npos)
	{
		if (cutAt > 0)
		{
			strResult[index++] = strOrigin.substr(0, cutAt);
		}
		strOrigin = strOrigin.substr(cutAt + 1);
	}

	if (strOrigin.length() > 0)
	{
		strResult[index++] = strOrigin.substr(0, cutAt);
	}

	for (int i = 0; i < index; i++)
	{
		strResult[i] = trim(strResult[i]);
	}

	return strResult;
}

int get_Item_Type_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arType[] = 
	{
		"ITEM_NONE",		//0
		"ITEM_WEAPON",		//1
		"ITEM_ARMOR",		//2
		"ITEM_USE",			//3
		"ITEM_AUTOUSE",		//4
		"ITEM_MATERIAL",	//5
		"ITEM_SPECIAL",		//6
		"ITEM_TOOL",		//7
		"ITEM_ELK",			//8
		"ITEM_METIN",		//9
		"ITEM_CONTAINER",	//10
		"ITEM_FISH",		//11
		"ITEM_ROD",			//12
		"ITEM_RESOURCE",	//13
		"ITEM_CAMPFIRE",	//14
		"ITEM_UNIQUE",		//15
		"ITEM_SKILLBOOK",	//16
		"ITEM_QUEST",		//17
		"ITEM_POLYMORPH",	//18
		"ITEM_TREASURE_BOX",//19
		"ITEM_TREASURE_KEY",//20
		"ITEM_SKILLFORGET",	//21
		"ITEM_GIFTBOX",		//22
		"ITEM_PICK",		//23
		"ITEM_HAIR",		//24
		"ITEM_TOTEM",		//25
		"ITEM_BLEND",		//26
		"ITEM_COSTUME",		//27
		"ITEM_DS",			//28
		"ITEM_SPECIAL_DS",	//29
		"ITEM_EXTRACT",		//30
		"ITEM_SECONDARY_COIN",//31
		"ITEM_RING",		//32
		"ITEM_BELT",		//33
#ifdef ENABLE_PET_COSTUME_SYSTEM
		"ITEM_PET",			//34
#endif
#ifdef ENABLE_MOUNT_SKIN
		"ITEM_MOUNT_SKIN",	//35
#endif
#ifdef ENABLE_PET_SKIN
		"ITEM_PET_SKIN",	//36
#endif
#ifdef ENABLE_CROWN_SYSTEM
		"ITEM_CROWN",		//37
#endif
#ifdef ENABLE_ITEMS_SHINING
		"ITEM_SHINING",		//38
#endif
#ifdef ENABLE_BOOSTER_ITEM
		"ITEM_BOOSTER",		//39
#endif
#ifdef ENABLE_PENDANT_ITEM
		"ITEM_PENDANT",		//40
#endif
#ifdef ENABLE_GLOVE_ITEM
		"ITEM_GLOVE",		//41
#endif
#ifdef ENABLE_RINGS
		"ITEM_RINGS",		//42
#endif
#ifdef ENABLE_AURA_SKIN
		"ITEM_AURA_SKIN",		//43
#endif
#ifdef ENABLE_DREAMSOUL
		"ITEM_DREAMSOUL",		//44
#endif
	};

	int retInt = -1;
	for (unsigned int j = 0; j < sizeof(arType) / sizeof(arType[0]); j++) 
	{
		string tempString = arType[j];
		if (inputString.find(tempString) != string::npos && tempString.find(inputString) != string::npos)
		{
			retInt = j;
			break;
		}
	}
	return retInt;
}

int get_Item_SubType_Value(unsigned int type_value, string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	static string arSub1[] = {
		"WEAPON_SWORD",
		"WEAPON_DAGGER",
		"WEAPON_BOW",
		"WEAPON_TWO_HANDED",
		"WEAPON_BELL",
		"WEAPON_FAN",
		"WEAPON_ARROW",
		"WEAPON_MOUNT_SPEAR",
		"WEAPON_CLAW",
		"WEAPON_QUIVER",
		"WEAPON_BOUQUET" 
	};

	static string arSub2[] = {
		"ARMOR_BODY",
		"ARMOR_HEAD",
		"ARMOR_SHIELD",
		"ARMOR_WRIST",
		"ARMOR_FOOTS",
		"ARMOR_NECK",
		"ARMOR_EAR",
		"ARMOR_NUM_TYPES" 
	};

	static string arSub3[] = {
		"USE_POTION",
		"USE_TALISMAN",
		"USE_TUNING",
		"USE_MOVE",
		"USE_TREASURE_BOX",
		"USE_MONEYBAG",
		"USE_BAIT",
		"USE_ABILITY_UP",
		"USE_AFFECT",
		"USE_CREATE_STONE",
		"USE_SPECIAL",
		"USE_POTION_NODELAY",
		"USE_CLEAR",
		"USE_INVISIBILITY",
		"USE_DETACHMENT",
		"USE_BUCKET",
		"USE_POTION_CONTINUE",
		"USE_CLEAN_SOCKET",
		"USE_CHANGE_ATTRIBUTE",
		"USE_ADD_ATTRIBUTE",
		"USE_ADD_ACCESSORY_SOCKET",
		"USE_PUT_INTO_ACCESSORY_SOCKET",
		"USE_ADD_ATTRIBUTE2",
		"USE_RECIPE",
		"USE_CHANGE_ATTRIBUTE2",
		"USE_BIND", "USE_UNBIND",
		"USE_TIME_CHARGE_PER",
		"USE_TIME_CHARGE_FIX",
		"USE_PUT_INTO_BELT_SOCKET",
		"USE_PUT_INTO_RING_SOCKET",
		"USE_CHANGE_COSTUME_ATTR",
		"USE_RESET_COSTUME_ATTR",
#ifdef ENABLE_ATTR_RARE_RENEWAL
		"USE_CHANGE_RARE_ATTRIBUTE",
		"USE_ADD_RARE_ATTRIBUTE",
#endif
	};

	static string arSub4[] = {
		"AUTOUSE_POTION",
		"AUTOUSE_ABILITY_UP",
		"AUTOUSE_BOMB",
		"AUTOUSE_GOLD",
		"AUTOUSE_MONEYBAG",
		"AUTOUSE_TREASURE_BOX" 
	};

	static string arSub5[] = {
		"MATERIAL_LEATHER",
		"MATERIAL_BLOOD",
		"MATERIAL_ROOT",
		"MATERIAL_NEEDLE",
		"MATERIAL_JEWEL",
		"MATERIAL_DS_REFINE_NORMAL",
		"MATERIAL_DS_REFINE_BLESSED",
		"MATERIAL_DS_REFINE_HOLLY" 
	};

	static string arSub6[] = {
		"SPECIAL_MAP",
		"SPECIAL_KEY",
		"SPECIAL_DOC",
		"SPECIAL_SPIRIT"
	};

	static string arSub7[] = {
		"TOOL_FISHING_ROD" 
	};

	static string arSub10[] = {
		"METIN_NORMAL",
		"METIN_GOLD"
	};

	static string arSub12[] = {
		"FISH_ALIVE",
		"FISH_DEAD" 
	};

	static string arSub14[] = {
		"RESOURCE_FISHBONE",
		"RESOURCE_WATERSTONEPIECE",
		"RESOURCE_WATERSTONE",
		"RESOURCE_BLOOD_PEARL",
		"RESOURCE_BLUE_PEARL",
		"RESOURCE_WHITE_PEARL",
		"RESOURCE_BUCKET",
		"RESOURCE_CRYSTAL",
		"RESOURCE_GEM",
		"RESOURCE_STONE",
		"RESOURCE_METIN",
		"RESOURCE_ORE"
	};

	static string arSub16[] = {
		"UNIQUE_NONE",
		"UNIQUE_BOOK",
		"UNIQUE_SPECIAL_RIDE",
		"UNIQUE_3",
		"UNIQUE_4",
		"UNIQUE_5",
		"UNIQUE_6",
		"UNIQUE_7",
		"UNIQUE_8",
		"UNIQUE_9",
		"USE_SPECIAL"
	};


	static string arSub28[] = {
		"COSTUME_BODY",
		"COSTUME_HAIR",
		"COSTUME_MOUNT",
		"COSTUME_ACCE",
		"COSTUME_WEAPON",
#ifdef ENABLE_AURA_SYSTEM
		"COSTUME_AURA",
#endif
#ifdef ENABLE_ACCE_COSTUME_SKIN
		"COSTUME_WING",
#endif
	};


	static string arSub29[] = {
		"DS_SLOT1",
		"DS_SLOT2",
		"DS_SLOT3",
		"DS_SLOT4",
		"DS_SLOT5",
		"DS_SLOT6",
	};

	static string arSub31[] = {
		"EXTRACT_DRAGON_SOUL",
		"EXTRACT_DRAGON_HEART"
	};

#ifdef ENABLE_ITEMS_SHINING
static string arSub38[] = {
		"SHINING_WRIST_LEFT",
		"SHINING_WRIST_RIGHT",
		"SHINING_ARMOR"
	};
#endif

#ifdef ENABLE_BOOSTER_ITEM
static string boosterSubType[] = {
	"BOOSTER_WEAPON",
	"BOOSTER_BODY",
	"BOOSTER_HEAD",
	"BOOSTER_SASH",
	"BOOSTER_PET",
	"BOOSTER_MOUNT",
};
#endif

#ifdef ENABLE_DREAMSOUL
static string dreamsoulSubType[] = {
	"DREAMSOUL_RED",
	"DREAMSOUL_BLUE",
};
#endif

#ifdef ENABLE_RINGS
static string ringsSubtypes[] = {
	"RING_0",
	"RING_1",
	"RING_2",
	"RING_3",
	"RING_4",
	"RING_5",
	"RING_6",
	"RING_7",
};
#endif

	static string* arSubType[] = {
		0,			//0
		arSub1,		//1
		arSub2,		//2
		arSub3,		//3
		arSub4,		//4
		arSub5,		//5
		arSub6,		//6
		arSub7,		//7
		0,			//8
		arSub10,	//9
		0,			//10
		arSub12,	//11
		0,			//12
		arSub14,	//13
		0,			//14
		arSub16,	//15
		0,			//16
		0,			//17
		0,			//18
		0,			//19
		0,			//20
		0,			//21
		0,			//22
		0,			//23
		0,			//24
		0,			//25
		0,			//26
		arSub28,	//27
		arSub29,	//28
		arSub29,	//29
		arSub31,	//30
		0,			//31
		0,			//32
		0,			//33
#ifdef ENABLE_PET_COSTUME_SYSTEM
		0,			//34
#endif
#ifdef ENABLE_MOUNT_SKIN
		0,			//35
#endif
#ifdef ENABLE_PET_SKIN
		0,			//36
#endif
#ifdef ENABLE_CROWN_SYSTEM
		0,			//37
#endif
#ifdef ENABLE_ITEMS_SHINING
		arSub38, //38
#endif
#ifdef ENABLE_BOOSTER_ITEM
		boosterSubType,//39
#endif
#ifdef ENABLE_PENDANT_ITEM
		0,			//40
#endif
#ifdef ENABLE_GLOVE_ITEM
		0,			//41
#endif
#ifdef ENABLE_RINGS
		ringsSubtypes,//42
#endif
#ifdef ENABLE_AURA_SKIN
		0,			//43
#endif
#ifdef ENABLE_DREAMSOUL
		dreamsoulSubType,//44
#endif
	};

	static int arNumberOfSubtype[_countof(arSubType)] = {
		/*0*/0,
		/*1*/sizeof(arSub1) / sizeof(arSub1[0]),
		/*2*/sizeof(arSub2) / sizeof(arSub2[0]),
		/*3*/sizeof(arSub3) / sizeof(arSub3[0]),
		/*4*/sizeof(arSub4) / sizeof(arSub4[0]),
		/*5*/sizeof(arSub5) / sizeof(arSub5[0]),
		/*6*/sizeof(arSub6) / sizeof(arSub6[0]),
		/*7*/sizeof(arSub7) / sizeof(arSub7[0]),
		/*8*/0,
		/*9*/sizeof(arSub10) / sizeof(arSub10[0]),
		/*10*/0,
		/*11*/sizeof(arSub12) / sizeof(arSub12[0]),
		/*12*/0,
		/*13*/sizeof(arSub14) / sizeof(arSub14[0]),
		/*14*/0,
		/*15*/sizeof(arSub16) / sizeof(arSub16[0]),
		/*16*/0,
		/*17*/0,
		/*18*/0,
		/*19*/0,
		/*20*/0,
		/*21*/0,
		/*22*/0,
		/*23*/0,
		/*24*/0,
		/*25*/0,
		/*26*/0,
		/*27*/sizeof(arSub28) / sizeof(arSub28[0]),
		/*28*/sizeof(arSub29) / sizeof(arSub29[0]),
		/*29*/sizeof(arSub29) / sizeof(arSub29[0]),
		/*30*/sizeof(arSub31) / sizeof(arSub31[0]),
		/*31*/0,
		/*32*/0,
		/*33*/0,
#ifdef ENABLE_PET_COSTUME_SYSTEM
		/*34*/0,
#endif
#ifdef ENABLE_MOUNT_SKIN
		/*35*/0,
#endif
#ifdef ENABLE_PET_SKIN
		/*36*/0,
#endif
#ifdef ENABLE_CROWN_SYSTEM
		/*37*/0,
#endif
#ifdef ENABLE_ITEMS_SHINING
		/*38*/sizeof(arSub38)/sizeof(arSub38[0]),
#endif
#ifdef ENABLE_BOOSTER_ITEM
		/*39*/sizeof(boosterSubType) / sizeof(boosterSubType[0]),
#endif
#ifdef ENABLE_PENDANT_ITEM
		/*40*/0,
#endif
#ifdef ENABLE_GLOVE_ITEM
		/*41*/0,
#endif
#ifdef ENABLE_RINGS
		/*42*/sizeof(ringsSubtypes) / sizeof(ringsSubtypes[0]),
#endif
#ifdef ENABLE_AURA_SKIN
		/*43*/0,
#endif
#ifdef ENABLE_DREAMSOUL
		/*44*/sizeof(dreamsoulSubType) / sizeof(dreamsoulSubType[0]),
#endif
	};

	assert(_countof(arSubType) > type_value && "Subtype rule: Out of range!!");

	if (_countof(arSubType) <= type_value)
	{
		sys_err("SubType : Out of range!! (type_value: %d, count of registered subtype: %d", type_value, _countof(arSubType));
		return -1;
	}

	if (arSubType[type_value] == 0) {
		return 0;
	}
	//

	int retInt = -1;
	//cout << "SubType : " << subTypeStr << " -> ";
	string tempInputString = trim(inputString);
	for (int j = 0; j < arNumberOfSubtype[type_value]; j++) {
		string tempString = arSubType[type_value][j];
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Item_AntiFlag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arAntiFlag[] = { "ANTI_FEMALE", "ANTI_MALE", "ANTI_MUSA", "ANTI_ASSASSIN", "ANTI_SURA", "ANTI_MUDANG",
							"ANTI_GET", "ANTI_DROP", "ANTI_SELL", "ANTI_EMPIRE_A", "ANTI_EMPIRE_B", "ANTI_EMPIRE_C",
							"ANTI_SAVE", "ANTI_GIVE", "ANTI_PKDROP", "ANTI_STACK", "ANTI_MYSHOP", "ANTI_SAFEBOX", "ANTI_WOLFMAN",
							"ANTI_PET20", "ANTI_PET21" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");
	for (unsigned int i = 0; i < sizeof(arAntiFlag) / sizeof(arAntiFlag[0]); i++) {
		string tempString = arAntiFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "AntiFlag : " << antiFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Flag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arFlag[] = { "ITEM_TUNABLE", "ITEM_SAVE", "ITEM_STACKABLE", "COUNT_PER_1GOLD", "ITEM_SLOW_QUERY", "ITEM_UNIQUE",
			"ITEM_MAKECOUNT", "ITEM_IRREMOVABLE", "CONFIRM_WHEN_USE", "QUEST_USE", "QUEST_USE_MULTIPLE",
			"QUEST_GIVE", "ITEM_QUEST", "LOG", "STACKABLE", "SLOW_QUERY", "REFINEABLE", "IRREMOVABLE", "ITEM_APPLICABLE" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");
	for (unsigned int i = 0; i < sizeof(arFlag) / sizeof(arFlag[0]); i++) {
		string tempString = arFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Flag : " << flagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_WearFlag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arWearrFlag[] = {
		"WEAR_BODY",
		"WEAR_HEAD",
		"WEAR_FOOTS",
		"WEAR_WRIST",
		"WEAR_WEAPON",
		"WEAR_NECK",
		"WEAR_EAR",
		"WEAR_SHIELD",
		"WEAR_UNIQUE",
		"WEAR_ARROW",
		"WEAR_HAIR",
#ifdef ENABLE_ACCE_COSTUME_SKIN
		"WEAR_COSTUME_WING",
#endif
#ifdef ENABLE_MOUNT_SKIN
		"WEAR_MOUNT_SKIN",
#endif
#ifdef ENABLE_PET_SKIN
		"WEAR_PET_SKIN",
#endif
#ifdef ENABLE_CROWN_SYSTEM
		"WEAR_CROWN",
#endif
#ifdef ENABLE_ITEMS_SHINING
		"WEAR_SHINING_WRIST_LEFT",
		"WEAR_SHINING_WRIST_RIGHT",
		"WEAR_SHINING_ARMOR",
#endif
#ifdef ENABLE_BOOSTER_ITEM
		"WEAR_BOOSTER_WEAPON",
		"WEAR_BOOSTER_BODY",
		"WEAR_BOOSTER_HEAD",
		"WEAR_BOOSTER_SASH",
		"WEAR_BOOSTER_PET",
		"WEAR_BOOSTER_MOUNT",
#endif
#ifdef ENABLE_PENDANT_ITEM
		"WEAR_PENDANT",
#endif
#ifdef ENABLE_GLOVE_ITEM
		"WEAR_GLOVE",
#endif
#ifdef ENABLE_RINGS
		"WEAR_RING_0",
		"WEAR_RING_1",
		"WEAR_RING_2",
		"WEAR_RING_3",
		"WEAR_RING_4",
		"WEAR_RING_5",
		"WEAR_RING_6",
		"WEAR_RING_7",
#endif
#ifdef ENABLE_AURA_SKIN
		"WEAR_AURA_SKIN",
#endif
#ifdef ENABLE_DREAMSOUL
		"WEAR_DREAMSOUL_RED",
		"WEAR_DREAMSOUL_BLUE",
#endif
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");
	for (unsigned int i = 0; i < sizeof(arWearrFlag) / sizeof(arWearrFlag[0]); i++) {
		string tempString = arWearrFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "WearFlag : " << wearFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Immune_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arImmune[] = { "PARA","CURSE","STUN","SLEEP","SLOW","POISON","TERROR" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");
	for (unsigned int i = 0; i < sizeof(arImmune) / sizeof(arImmune[0]); i++) {
		string tempString = arImmune[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune : " << immuneStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_LimitType_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arLimitType[] = { "LIMIT_NONE", "LEVEL", "STR", "DEX", "INT", "CON","REAL_TIME", "REAL_TIME_FIRST_USE", "TIMER_BASED_ON_WEAR" };

	int retInt = -1;
	//cout << "LimitType : " << limitTypeStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arLimitType) / sizeof(arLimitType[0]); j++) {
		string tempString = arLimitType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Item_ApplyType_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arApplyType[] = { 
		"APPLY_NONE",
		"APPLY_MAX_HP",
		"APPLY_MAX_SP",
		"APPLY_CON",
		"APPLY_INT",
		"APPLY_STR",
		"APPLY_DEX",
		"APPLY_ATT_SPEED",
		"APPLY_MOV_SPEED",
		"APPLY_CAST_SPEED",
		"APPLY_HP_REGEN",
		"APPLY_SP_REGEN",
		"APPLY_POISON_PCT",
		"APPLY_STUN_PCT",
		"APPLY_SLOW_PCT",
		"APPLY_CRITICAL_PCT",
		"APPLY_PENETRATE_PCT",
		"APPLY_ATTBONUS_HUMAN",
		"APPLY_ATTBONUS_ANIMAL",
		"APPLY_ATTBONUS_ORC",
		"APPLY_ATTBONUS_MILGYO",
		"APPLY_ATTBONUS_UNDEAD",
		"APPLY_ATTBONUS_DEVIL",
		"APPLY_STEAL_HP",
		"APPLY_STEAL_SP",
		"APPLY_MANA_BURN_PCT",
		"APPLY_DAMAGE_SP_RECOVER",
		"APPLY_BLOCK",
		"APPLY_DODGE",
		"APPLY_RESIST_SWORD",
		"APPLY_RESIST_TWOHAND",
		"APPLY_RESIST_DAGGER",
		"APPLY_RESIST_BELL",
		"APPLY_RESIST_FAN",
		"APPLY_RESIST_BOW",
		"APPLY_RESIST_FIRE",
		"APPLY_RESIST_ELEC",
		"APPLY_RESIST_MAGIC",
		"APPLY_RESIST_WIND",
		"APPLY_REFLECT_MELEE",
		"APPLY_REFLECT_CURSE",
		"APPLY_POISON_REDUCE",
		"APPLY_KILL_SP_RECOVER",
		"APPLY_EXP_DOUBLE_BONUS",
		"APPLY_GOLD_DOUBLE_BONUS",
		"APPLY_ITEM_DROP_BONUS",
		"APPLY_POTION_BONUS",
		"APPLY_KILL_HP_RECOVER",
		"APPLY_IMMUNE_STUN",
		"APPLY_IMMUNE_SLOW",
		"APPLY_IMMUNE_FALL",
		"APPLY_SKILL",
		"APPLY_BOW_DISTANCE",
		"APPLY_ATT_GRADE_BONUS",
		"APPLY_DEF_GRADE_BONUS",
		"APPLY_MAGIC_ATT_GRADE",
		"APPLY_MAGIC_DEF_GRADE",
		"APPLY_CURSE_PCT",
		"APPLY_MAX_STAMINA",
		"APPLY_ATTBONUS_WARRIOR",
		"APPLY_ATTBONUS_ASSASSIN",
		"APPLY_ATTBONUS_SURA",
		"APPLY_ATTBONUS_SHAMAN",
		"APPLY_ATTBONUS_MONSTER",
		"APPLY_MALL_ATTBONUS",
		"APPLY_MALL_DEFBONUS",
		"APPLY_MALL_EXPBONUS",
		"APPLY_MALL_ITEMBONUS",
		"APPLY_MALL_GOLDBONUS",
		"APPLY_MAX_HP_PCT",
		"APPLY_MAX_SP_PCT",
		"APPLY_SKILL_DAMAGE_BONUS",
		"APPLY_NORMAL_HIT_DAMAGE_BONUS",
		"APPLY_SKILL_DEFEND_BONUS",
		"APPLY_NORMAL_HIT_DEFEND_BONUS",
		"APPLY_EXTRACT_HP_PCT",
		"APPLY_RESIST_WARRIOR",
		"APPLY_RESIST_ASSASSIN",
		"APPLY_RESIST_SURA",
		"APPLY_RESIST_SHAMAN",
		"APPLY_ENERGY",
		"APPLY_DEF_GRADE",
		"APPLY_COSTUME_ATTR_BONUS",
		"APPLY_MAGIC_ATTBONUS_PER",
		"APPLY_MELEE_MAGIC_ATTBONUS_PER",
		"APPLY_RESIST_ICE",
		"APPLY_RESIST_EARTH",
		"APPLY_RESIST_DARK",
		"APPLY_ANTI_CRITICAL_PCT",
		"APPLY_ANTI_PENETRATE_PCT",
#ifdef ENABLE_WOLFMAN_CHARACTER
		"APPLY_BLEEDING_REDUCE",
		"APPLY_BLEEDING_PCT",
		"APPLY_ATTBONUS_WOLFMAN",
		"APPLY_RESIST_WOLFMAN",
		"APPLY_RESIST_CLAW",
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		"APPLY_ACCEDRAIN_RATE",
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
		"APPLY_RESIST_MAGIC_REDUCTION",
#endif
#ifdef ENABLE_EXTRA_APPLY_BONUS
		"APPLY_ATTBONUS_STONE",
		"APPLY_ATTBONUS_BOSS",
		"APPLY_ATTBONUS_PVM_STR",
		"APPLY_ATTBONUS_PVM_AVG",
		"APPLY_ATTBONUS_PVM_BERSERKER",
		"APPLY_ATTBONUS_ELEMENT",
		"APPLY_DEFBONUS_PVM",
		"APPLY_DEFBONUS_ELEMENT",
		"APPLY_ATTBONUS_PVP",
		"APPLY_DEFBONUS_PVP",

		"APPLY_ATTBONUS_FIRE",
		"APPLY_ATTBONUS_ICE",
		"APPLY_ATTBONUS_WIND",
		"APPLY_ATTBONUS_EARTH",
		"APPLY_ATTBONUS_DARK",
		"APPLY_ATTBONUS_ELEC",
#endif
		"MAX_APPLY_NUM",
	};

	int retInt = -1;
	//cout << "ApplyType : " << applyTypeStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arApplyType) / sizeof(arApplyType[0]); j++) {
		string tempString = arApplyType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Rank_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arRank[] = { "PAWN", "S_PAWN", "KNIGHT", "S_KNIGHT", "BOSS", "KING" };

	int retInt = -1;
	//cout << "Rank : " << rankStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arRank) / sizeof(arRank[0]); j++) {
		string tempString = arRank[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Type_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arType[] = { "MONSTER", "NPC", "STONE", "WARP", "DOOR", "BUILDING", "PC", "POLYMORPH_PC", "HORSE", "GOTO" };

	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arType) / sizeof(arType[0]); j++) {
		string tempString = arType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_BattleType_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arBattleType[] = { "MELEE", "RANGE", "MAGIC", "SPECIAL", "POWER", "TANKER", "SUPER_POWER", "SUPER_TANKER" };

	int retInt = -1;
	//cout << "Battle Type : " << battleTypeStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arBattleType) / sizeof(arBattleType[0]); j++) {
		string tempString = arBattleType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Size_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arSize[] = { "SMALL", "MEDIUM", "BIG" }; //@fixme201 SAMLL to SMALL

	int retInt = 0;
	//cout << "Size : " << sizeStr << " -> ";
	for (unsigned int j = 0; j < sizeof(arSize) / sizeof(arSize[0]); j++) {
		string tempString = arSize[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j + 1;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_AIFlag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arAIFlag[] = { "AGGR","NOMOVE","COWARD","NOATTSHINSU","NOATTCHUNJO","NOATTJINNO","ATTMOB","BERSERK","STONESKIN","GODSPEED","DEATHBLOW","REVIVE",
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");
	for (unsigned int i = 0; i < sizeof(arAIFlag) / sizeof(arAIFlag[0]); i++) {
		string tempString = arAIFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "AIFlag : " << aiFlagStr << " -> " << retValue << endl;

	return retValue;
}
int get_Mob_RaceFlag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arRaceFlag[] = { "ANIMAL","UNDEAD","DEVIL","HUMAN","ORC","MILGYO","INSECT","FIRE","ICE","DESERT","TREE",
		"ATT_ELEC","ATT_FIRE","ATT_ICE","ATT_WIND","ATT_EARTH","ATT_DARK" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");
	for (unsigned int i = 0; i < sizeof(arRaceFlag) / sizeof(arRaceFlag[0]); i++) {
		string tempString = arRaceFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Race Flag : " << raceFlagStr << " -> " << retValue << endl;

	return retValue;
}
int get_Mob_ImmuneFlag_Value(string inputString)
{
#ifdef ENABLE_NUMERIC_FIELD
	if (_IsNumericString(inputString))
		return std::stoi(inputString);
#endif
	string arImmuneFlag[] = { "STUN","SLOW","FALL","CURSE","POISON","TERROR", "REFLECT" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");
	for (unsigned int i = 0; i < sizeof(arImmuneFlag) / sizeof(arImmuneFlag[0]); i++) {
		string tempString = arImmuneFlag[i];
		for (unsigned int j = 0; j < 30; j++)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) {
				retValue = retValue + pow((float)2, (float)i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune Flag : " << immuneFlagStr << " -> " << retValue << endl;

	return retValue;
}

#ifndef __DUMP_PROTO__

bool Set_Proto_Mob_Table(TMobTable* mobTable, cCsvTable& csvTable, std::map<int, const char*>& nameMap)
{
	int col = 0;
	str_to_number(mobTable->dwVnum, csvTable.AsStringByIndex(col++));
	strlcpy(mobTable->szName, csvTable.AsStringByIndex(col++), sizeof(mobTable->szName));

	map<int, const char*>::iterator it;
	it = nameMap.find(mobTable->dwVnum);
	if (it != nameMap.end()) {
		const char* localeName = it->second;
		strlcpy(mobTable->szLocaleName, localeName, sizeof(mobTable->szLocaleName));
	}
	else {
		strlcpy(mobTable->szLocaleName, mobTable->szName, sizeof(mobTable->szLocaleName));
	}

	//RANK
	int rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
	mobTable->bRank = rankValue;
	//TYPE
	int typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
	mobTable->bType = typeValue;
	//BATTLE_TYPE
	int battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
	mobTable->bBattleType = battleTypeValue;

	str_to_number(mobTable->bLevel, csvTable.AsStringByIndex(col++));
	//SIZE
	int sizeValue = get_Mob_Size_Value(csvTable.AsStringByIndex(col++));
	mobTable->bSize = sizeValue;
	//AI_FLAG
	int aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlag = aiFlagValue;
	//mount_capacity;
	col++;
	//RACE_FLAG
	int raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwRaceFlag = raceFlagValue;
	//IMMUNE_FLAG
	int immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwImmuneFlag = immuneFlagValue;

	str_to_number(mobTable->bEmpire, csvTable.AsStringByIndex(col++));  //col = 11

	strlcpy(mobTable->szFolder, csvTable.AsStringByIndex(col++), sizeof(mobTable->szFolder));

	str_to_number(mobTable->bOnClickType, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bStr, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDex, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bCon, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bInt, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[0], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[1], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwMaxHP, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenCycle, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenPercent, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMin, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMax, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwExp, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wDef, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sAttackSpeed, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sMovingSpeed, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bAggresiveHPPct, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wAggressiveSight, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wAttackRange, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->dwDropItemVnum, csvTable.AsStringByIndex(col++));	//32
	str_to_number(mobTable->dwResurrectionVnum, csvTable.AsStringByIndex(col++));
	for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
		str_to_number(mobTable->cEnchants[i], csvTable.AsStringByIndex(col++));

	for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
		str_to_number(mobTable->cResists[i], csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->fDamMultiply, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwSummonVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDrainSP, csvTable.AsStringByIndex(col++));

	//Mob_Color
	++col;

	str_to_number(mobTable->dwPolymorphItemVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->Skills[0].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[0].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].dwVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bBerserkPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bStoneSkinPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bGodSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDeathBlowPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRevivePoint, csvTable.AsStringByIndex(col++));
	return true;
}

bool Set_Proto_Item_Table(TItemTable* itemTable, cCsvTable& csvTable, std::map<int, const char*>& nameMap)
{
	int col = 0;
#ifdef ENABLE_EXTENDED_YANG_LIMIT
#ifdef ENABLE_ITEM_PROTO_APPLY_LIMIT
	int64_t dataArray[37];
#else
	int64_t dataArray[33];
#endif
#else
#ifdef ENABLE_ITEM_PROTO_APPLY_LIMIT
	int dataArray[37];
#else
	int dataArray[33];
#endif
#endif

	for (unsigned int i = 0; i < sizeof(dataArray) / sizeof(dataArray[0]); i++) {
		int validCheck = 0;
		if (i == 2) {
			dataArray[i] = get_Item_Type_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 3) {
			dataArray[i] = get_Item_SubType_Value(dataArray[i - 1], csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 5) {
			dataArray[i] = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 6) {
			dataArray[i] = get_Item_Flag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 7) {
			dataArray[i] = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 8) {
			dataArray[i] = get_Item_Immune_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 14) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 16) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 18) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 20) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 22) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
#ifdef ENABLE_ITEM_PROTO_APPLY_LIMIT
		else if (i == 24) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 26) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
#endif
		else {
			str_to_number(dataArray[i], csvTable.AsStringByIndex(col));
		}

		if (validCheck == -1)
		{
			std::ostringstream dataStream;

			for (unsigned int j = 0; j < i; ++j)
				dataStream << dataArray[j] << ",";

			//fprintf(stderr, "ItemProto Reading Failed : Invalid value.\n");
			sys_err("ItemProto Reading Failed : Invalid value. (index: %d, col: %d, value: %s)", i, col, csvTable.AsStringByIndex(col));
			sys_err("\t%d ~ %d Values: %s", 0, i, dataStream.str().c_str());

			exit(0);
		}

		col = col + 1;
	}

	{
		std::string s(csvTable.AsStringByIndex(0));
		unsigned int pos = s.find("~");
		if (std::string::npos == pos)
		{
			itemTable->dwVnum = dataArray[0];
			itemTable->dwVnumRange = 0;
		}
		else
		{
			std::string s_start_vnum(s.substr(0, pos));
			std::string s_end_vnum(s.substr(pos + 1));

			int start_vnum = atoi(s_start_vnum.c_str());
			int end_vnum = atoi(s_end_vnum.c_str());
			if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
			{
				sys_err("INVALID VNUM %s", s.c_str());
				return false;
			}
			itemTable->dwVnum = start_vnum;
			itemTable->dwVnumRange = end_vnum - start_vnum;
		}
	}

	strlcpy(itemTable->szName, csvTable.AsStringByIndex(1), sizeof(itemTable->szName));
	map<int, const char*>::iterator it;
	it = nameMap.find(itemTable->dwVnum);
	if (it != nameMap.end()) {
		const char* localeName = it->second;
		strlcpy(itemTable->szLocaleName, localeName, sizeof(itemTable->szLocaleName));
	}
	else {
		strlcpy(itemTable->szLocaleName, itemTable->szName, sizeof(itemTable->szLocaleName));
	}
	itemTable->bType = dataArray[2];
	itemTable->bSubType = dataArray[3];
	itemTable->bSize = MINMAX(1, dataArray[4], 3); // @fixme179
	itemTable->dwAntiFlags = dataArray[5];
	itemTable->dwFlags = dataArray[6];
	itemTable->dwWearFlags = dataArray[7];
	itemTable->dwImmuneFlag = dataArray[8];
	itemTable->dwGold = dataArray[9];
	itemTable->dwShopBuyPrice = dataArray[10];
	itemTable->dwRefinedVnum = dataArray[11];
	itemTable->wRefineSet = dataArray[12];
	itemTable->bAlterToMagicItemPct = dataArray[13];
	itemTable->cLimitRealTimeFirstUseIndex = -1;
	itemTable->cLimitTimerBasedOnWearIndex = -1;

	int i;

	for (i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		itemTable->aLimits[i].bType = dataArray[14 + i * 2];
		itemTable->aLimits[i].lValue = dataArray[15 + i * 2];

		if (LIMIT_REAL_TIME_START_FIRST_USE == itemTable->aLimits[i].bType)
			itemTable->cLimitRealTimeFirstUseIndex = (char)i;

		if (LIMIT_TIMER_BASED_ON_WEAR == itemTable->aLimits[i].bType)
			itemTable->cLimitTimerBasedOnWearIndex = (char)i;
	}

	for (i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		itemTable->aApplies[i].bType = dataArray[18 + i * 2];
		itemTable->aApplies[i].lValue = dataArray[19 + i * 2];
	}

#ifdef ENABLE_ITEM_PROTO_APPLY_LIMIT
	for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = dataArray[28 + i];

	//column for 'Specular'
	itemTable->bGainSocketPct = dataArray[35];
	itemTable->sAddonType = dataArray[36];
#else
	for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = dataArray[24 + i];

	//column for 'Specular'
	itemTable->bGainSocketPct = dataArray[31];
	itemTable->sAddonType = dataArray[32];
#endif
	//test
	str_to_number(itemTable->bWeight, "0");

	return true;
}

#endif