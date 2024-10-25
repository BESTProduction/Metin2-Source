//#define __FISHING_MAIN__
#include "stdafx.h"
#include "constants.h"
#include "fishing.h"
#include "locale_service.h"

#ifndef __FISHING_MAIN__
#include "item_manager.h"

#include "config.h"
#include "packet.h"

#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"



#include "questmanager.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "locale_service.h"

#include "affect.h"
#include "unique_item.h"
#endif
#include "skill.h"

#define ENABLE_FISHINGROD_RENEWAL
namespace fishing
{

#ifdef ENABLE_FISHING_RENEWAL
	constexpr int fishingLevelCatchCount{75};
	constexpr DWORD upgradeItemVnum{ 200212 }; // yukseltme nesnesi
	constexpr DWORD upgradeBook{ 24012 }; // kitap
	constexpr BYTE bookDropPct{ 4 }; // kitap
#endif

	enum
	{
		MAX_FISH = 37,
		NUM_USE_RESULT_COUNT = 10, // 1 : DEAD 2 : BONE 3 ~ 12 : rest
		FISH_BONE_VNUM = 27799,
		SHELLFISH_VNUM = 27987,
		EARTHWORM_VNUM = 27801,
		WATER_STONE_VNUM_BEGIN = 28030,
		WATER_STONE_VNUM_END = 28043,
		FISH_NAME_MAX_LEN = 64,
		MAX_PROB = 4,
	};

	enum
	{
		USED_NONE,
		USED_SHELLFISH,
		USED_WATER_STONE,
		USED_TREASURE_MAP,
		USED_EARTHWARM,
		MAX_USED_FISH
	};

	enum
	{
		FISHING_TIME_NORMAL,
		FISHING_TIME_SLOW,
		FISHING_TIME_QUICK,
		FISHING_TIME_ALL,
		FISHING_TIME_EASY,

		FISHING_TIME_COUNT,

		MAX_FISHING_TIME_COUNT = 31,
	};

	enum
	{
		FISHING_LIMIT_NONE,
		FISHING_LIMIT_APPEAR,
	};

	int aFishingTime[FISHING_TIME_COUNT][MAX_FISHING_TIME_COUNT] =
	{
		{   0,   0,   0,   0,   0,   2,   4,   8,  12,  16,  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20,  16,  12,   8,   4,   2,   0,   0,   0,   0,   0 },
		{   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   4,   8,  12,  16,  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20 },
		{  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20,  16,  12,   8,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
		{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
		{  20,  20,  20,  20,  20,  22,  24,  28,  32,  36,  40,  42,  45,  50,  70, 100,  70,  50,  45,  42,  40,  36,  32,  28,  24,  22,  20,  20,  20,  20,  20 },
	};

	struct SFishInfo
	{
		char name[FISH_NAME_MAX_LEN];

		DWORD vnum;
		DWORD dead_vnum;
		DWORD grill_vnum;

		int prob[MAX_PROB];
		int difficulty;

		int time_type;
		int length_range[3]; // MIN MAX EXTRA_MAX : 99% MIN~MAX, 1% MAX~EXTRA_MAX

		int used_table[NUM_USE_RESULT_COUNT];
		// 6000 2000 1000 500 300 100 50 30 10 5 4 1
	};

	bool operator < (const SFishInfo& lhs, const SFishInfo& rhs)
	{
		return lhs.vnum < rhs.vnum;
	}

	int g_prob_sum[MAX_PROB];
	int g_prob_accumulate[MAX_PROB][MAX_FISH];

	SFishInfo fish_info[MAX_FISH] = { { "\0", }, };

	void Initialize()
	{
		SFishInfo fish_info_bak[MAX_FISH];
		thecore_memcpy(fish_info_bak, fish_info, sizeof(fish_info));

		memset(fish_info, 0, sizeof(fish_info));

		// LOCALE_SERVICE
		const int FILE_NAME_LEN = 256;
		char szFishingFileName[FILE_NAME_LEN + 1];
		snprintf(szFishingFileName, sizeof(szFishingFileName),
			"/lightwork/main/srv1/share/locale/europe/common/fishing.txt");
		FILE* fp = fopen(szFishingFileName, "r");
		// END_OF_LOCALE_SERVICE

		if (*fish_info_bak[0].name)
			SendLog("Reloading fish table.");

		if (!fp)
		{
			SendLog("error! cannot open fishing.txt");

			if (*fish_info_bak[0].name)
			{
				thecore_memcpy(fish_info, fish_info_bak, sizeof(fish_info));
				SendLog("  restoring to backup");
			}
			return;
		}

		memset(fish_info, 0, sizeof(fish_info));

		char buf[512];
		int idx = 0;

		while (fgets(buf, 512, fp))
		{
			if (*buf == '#')
				continue;

			char* p = strrchr(buf, '\n');
			*p = '\0';

			const char* start = buf;
			const char* tab = strchr(start, '\t');

			if (!tab)
			{
				printf("Tab error on line: %s\n", buf);
				SendLog("error! parsing fishing.txt");

				if (*fish_info_bak[0].name)
				{
					thecore_memcpy(fish_info, fish_info_bak, sizeof(fish_info));
					SendLog("  restoring to backup");
				}
				break;
			}

			char szCol[256], szCol2[256];
			int iColCount = 0;

			do
			{
				strlcpy(szCol2, start, MIN(sizeof(szCol2), (tab - start) + 1));
				szCol2[tab - start] = '\0';

				trim_and_lower(szCol2, szCol, sizeof(szCol));

				if (!*szCol || *szCol == '\t')
					iColCount++;
				else
				{
					switch (iColCount++)
					{
					case 0: strlcpy(fish_info[idx].name, szCol, sizeof(fish_info[idx].name)); break;
					case 1: str_to_number(fish_info[idx].vnum, szCol); break;
					case 2: str_to_number(fish_info[idx].dead_vnum, szCol); break;
					case 3: str_to_number(fish_info[idx].grill_vnum, szCol); break;
					case 4: str_to_number(fish_info[idx].prob[0], szCol); break;
					case 5: str_to_number(fish_info[idx].prob[1], szCol); break;
					case 6: str_to_number(fish_info[idx].prob[2], szCol); break;
					case 7: str_to_number(fish_info[idx].prob[3], szCol); break;
					case 8: str_to_number(fish_info[idx].difficulty, szCol); break;
					case 9: str_to_number(fish_info[idx].time_type, szCol); break;
					case 10: str_to_number(fish_info[idx].length_range[0], szCol); break;
					case 11: str_to_number(fish_info[idx].length_range[1], szCol); break;
					case 12: str_to_number(fish_info[idx].length_range[2], szCol); break;
					case 13: // 0
					case 14: // 1
					case 15: // 2
					case 16: // 3
					case 17: // 4
					case 18: // 5
					case 19: // 6
					case 20: // 7
					case 21: // 8
					case 22: // 9
						str_to_number(fish_info[idx].used_table[iColCount - 1 - 12], szCol);
						break;
					}
				}

				start = tab + 1;
				tab = strchr(start, '\t');
			} while (tab);

			idx++;

			if (idx == MAX_FISH)
				break;
		}

		fclose(fp);

		for (int j = 0; j < MAX_PROB; ++j)
		{
			g_prob_accumulate[j][0] = fish_info[0].prob[j];

			for (int i = 1; i < MAX_FISH; ++i)
				g_prob_accumulate[j][i] = fish_info[i].prob[j] + g_prob_accumulate[j][i - 1];

			g_prob_sum[j] = g_prob_accumulate[j][MAX_FISH - 1];
		}
	}

	int DetermineFishByProbIndex(int prob_idx)
	{
		int rv = number(1, g_prob_sum[prob_idx]);
		int* p = std::lower_bound(g_prob_accumulate[prob_idx], g_prob_accumulate[prob_idx] + MAX_FISH, rv);
		int fish_idx = p - g_prob_accumulate[prob_idx];
		return fish_idx;
	}

	int GetProbIndexByMapIndex(int index)
	{
		if (index > 60)
			return -1;

		switch (index)
		{
		case 1:
		case 21:
		case 41:
			return 0;

		case 3:
		case 23:
		case 43:
			return 1;
		}

		return -1;
	}

#ifndef __FISHING_MAIN__
	int DetermineFish(LPCHARACTER ch)
	{
		int map_idx = ch->GetMapIndex();
		int prob_idx = GetProbIndexByMapIndex(map_idx);

		if (prob_idx < 0)
			return 0;

		// ADD_PREMIUM
		if (ch->GetPremiumRemainSeconds(PREMIUM_FISH_MIND) > 0 ||
			ch->IsEquipUniqueGroup(UNIQUE_GROUP_FISH_MIND))
		{
			if (quest::CQuestManager::instance().GetEventFlag("manwoo") != 0)
				prob_idx = 3;
			else
				prob_idx = 2;
		}
		// END_OF_ADD_PREMIUM

		int adjust = 0;
		if (quest::CQuestManager::instance().GetEventFlag("fish_miss_pct") != 0)
		{
			int fish_pct_value = MINMAX(0, quest::CQuestManager::instance().GetEventFlag("fish_miss_pct"), 200);
			adjust = (100 - fish_pct_value) * fish_info[0].prob[prob_idx] / 100;
		}

		int rv = number(adjust + 1, g_prob_sum[prob_idx]);

		int* p = std::lower_bound(g_prob_accumulate[prob_idx], g_prob_accumulate[prob_idx] + MAX_FISH, rv);
		int fish_idx = p - g_prob_accumulate[prob_idx];

		{
			DWORD vnum = fish_info[fish_idx].vnum;

			if (vnum == 50008 || vnum == 50009 || vnum == 80008)
				return 0;
		}

		return (fish_idx);
	}

	void FishingReact(LPCHARACTER ch)
	{
		TPacketGCFishing p;
		p.header = HEADER_GC_FISHING;
		p.subheader = FISHING_SUBHEADER_GC_REACT;
		p.info = ch->GetVID();
		ch->PacketAround(&p, sizeof(p));
	}

	void FishingSuccess(LPCHARACTER ch)
	{
		TPacketGCFishing p;
		p.header = HEADER_GC_FISHING;
		p.subheader = FISHING_SUBHEADER_GC_SUCCESS;
		p.info = ch->GetVID();
		ch->PacketAround(&p, sizeof(p));
#ifdef ENABLE_FISHING_RENEWAL
		ch->m_bFishGameFishCounter = 0;
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenFish 0");
		event_cancel(&ch->m_pkFishingEvent);
#endif
	}
#ifdef ENABLE_FISHING_RENEWAL
	void CheckFishingLevelCanIncrease(LPCHARACTER ch)
	{
		unsigned totalFishCount = ch->GetPoint(POINT_FISHING);

		int fishingSkillLevel{ ch->GetSkillLevel(SKILL_PASSIVE_FISHING) };
		int remainingLevelUpCount = totalFishCount % fishingLevelCatchCount;

		if (fishingSkillLevel < 50)
		{
			if (remainingLevelUpCount == 0)
			{
				ch->SetSkillLevel(SKILL_PASSIVE_FISHING, fishingSkillLevel + 1);
				ch->NewChatPacket(STRING_D188, "%d", fishingLevelCatchCount);

				ch->Save();
				ch->ComputePoints();
				ch->SkillLevelPacket();
			}
		}
	}
#endif

	void FishingFail(LPCHARACTER ch)
	{
		TPacketGCFishing p;
		p.header = HEADER_GC_FISHING;
		p.subheader = FISHING_SUBHEADER_GC_FAIL;
		p.info = ch->GetVID();
		ch->PacketAround(&p, sizeof(p));
#ifdef ENABLE_FISHING_RENEWAL
		ch->m_bFishGameFishCounter = 0;
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenFish 0");
		event_cancel(&ch->m_pkFishingEvent);
#endif
	}

	void FishingPractice(LPCHARACTER ch)
	{
		LPITEM rod = ch->GetWear(WEAR_WEAPON);
		if (rod && rod->GetType() == ITEM_ROD)
		{
			if (rod->GetRefinedVnum() > 0 && rod->GetSocket(0) < rod->GetValue(2) && number(1, rod->GetValue(1)) == 1)
			{
				rod->SetSocket(0, rod->GetSocket(0) + 1);

				ch->NewChatPacket(FISHING_TALK_01, "%d|%d", rod->GetSocket(0), rod->GetValue(2));

				if (rod->GetSocket(0) == rod->GetValue(2))
				{
					ch->NewChatPacket(FISHING_TALK_02);
					ch->NewChatPacket(FISHING_TALK_03);
				}
			}
		}

		rod->SetSocket(2, 0);
	}

	bool PredictFish(LPCHARACTER ch)
	{
		// ADD_PREMIUM

		if (ch->FindAffect(AFFECT_FISH_MIND_PILL) ||
			ch->GetPremiumRemainSeconds(PREMIUM_FISH_MIND) > 0 ||
			ch->IsEquipUniqueGroup(UNIQUE_GROUP_FISH_MIND))
			return true;
		// END_OF_ADD_PREMIUM

		return false;
	}

	EVENTFUNC(fishing_event)
	{
		fishing_event_info* info = dynamic_cast<fishing_event_info*>(event->info);

		if (info == NULL)
		{
			sys_err("fishing_event> <Factor> Null pointer");
			return 0;
		}

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(info->pid);

		if (!ch)
			return 0;

		LPITEM rod = ch->GetWear(WEAR_WEAPON);

		if (!(rod && rod->GetType() == ITEM_ROD))
		{
			ch->m_pkFishingEvent = nullptr;
			return 0;
		}

		switch (info->step)
		{
		case 0:
			++info->step;

			//info->ch->Motion(MOTION_FISHING_SIGN);
			info->hang_time = get_dword_time();
			info->fish_id = DetermineFish(ch);
			FishingReact(ch);

			if (PredictFish(ch))
			{
				TPacketGCFishing p;
				p.header = HEADER_GC_FISHING;
				p.subheader = FISHING_SUBHEADER_GC_FISH;
				p.info = 27803;
				ch->GetDesc()->Packet(&p, sizeof(TPacketGCFishing));
			}
#ifdef ENABLE_FISHING_RENEWAL
			return (PASSES_PER_SEC(15));
#else
			return (PASSES_PER_SEC(6));
#endif

		default:
			++info->step;

			if (info->step > 5)
				info->step = 5;

			ch->m_pkFishingEvent = nullptr;
			FishingFail(ch);
			rod->SetSocket(2, 0);
			return 0;
		}
	}

	LPEVENT CreateFishingEvent(LPCHARACTER ch)
	{
		fishing_event_info* info = AllocEventInfo<fishing_event_info>();
		info->pid = ch->GetPlayerID();
		info->step = 0;
		info->hang_time = 0;

		TPacketGCFishing p;
		p.header = HEADER_GC_FISHING;
		p.subheader = FISHING_SUBHEADER_GC_START;
		p.info = ch->GetVID();
		p.dir = (BYTE)(ch->GetRotation() / 5);
		ch->PacketAround(&p, sizeof(TPacketGCFishing));

#ifdef ENABLE_FISHING_RENEWAL
		ch->m_bFishGameFishCounter = 0;
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenFish 1");
		return event_create(fishing_event, info, PASSES_PER_SEC(0));
#else
		int time = number(10, 40);
		return event_create(fishing_event, info, PASSES_PER_SEC(time));
#endif

	}

	int GetFishingLevel(LPCHARACTER ch)
	{
		LPITEM rod = ch->GetWear(WEAR_WEAPON);

		if (!rod || rod->GetType() != ITEM_ROD)
			return 0;

		return rod->GetSocket(2) + rod->GetValue(0);
	}

	int GetFishLength(int fish_id)
	{
		if (number(0, 99))
		{
			// 99% : normal size
			return (int)(fish_info[fish_id].length_range[0] +
				(fish_info[fish_id].length_range[1] - fish_info[fish_id].length_range[0])
				* (number(0, 2000) + number(0, 2000) + number(0, 2000) + number(0, 2000) + number(0, 2000)) / 10000);
		}
		else
		{
			// 1% : extra LARGE size
			return (int)(fish_info[fish_id].length_range[1] +
				(fish_info[fish_id].length_range[2] - fish_info[fish_id].length_range[1])
				* 2 * asin(number(0, 10000) / 10000.) / M_PI);
		}
	}

	void Take(fishing_event_info* info, LPCHARACTER ch)
	{

#ifdef ENABLE_FISHING_RENEWAL
		if (ch->m_bFishGameFishCounter < 3)
		{
			FishingFail(ch);
			return;
		}
#endif

		if (info->step == 1)
		{
			const long ms = static_cast<long>((get_dword_time() - info->hang_time));

			if (ms > 15000)
			{
				FishingFail(ch);
				return;
			}

			LPITEM rod = ch->GetWear(WEAR_WEAPON);

			if (!(rod && rod->GetType() == ITEM_ROD))
			{
				FishingFail(ch);
				return;
			}

			DWORD itemVnum{ 27803 }; // sudak
			BYTE uniqueItemDropChance{ 5 };
			const DWORD rodLevel{ rod->GetVnum() % 27400 };

			if (rodLevel > 0)
				uniqueItemDropChance += (rodLevel / 10) * 5;

			if (number(0, 100) <= uniqueItemDropChance)
				itemVnum = 27805;

			MAX_COUNT dropItemCount{ 1 };

			int getSkillLevel{ ch->GetSkillLevel(SKILL_PASSIVE_FISHING) };
			int doubleDropChance{ getSkillLevel / 2 };

			if (doubleDropChance >= 1)
			{
				if (number(1, 100) <= doubleDropChance)
				{
					dropItemCount = 2;
				}
			}

			FishingSuccess(ch);
			CheckFishingLevelCanIncrease(ch);

			TPacketGCFishing p;
			p.header = HEADER_GC_FISHING;
			p.subheader = FISHING_SUBHEADER_GC_FISH;
			p.info = itemVnum;
			ch->GetDesc()->Packet(&p, sizeof(TPacketGCFishing));

			ch->AutoGiveItem(itemVnum, dropItemCount, -1, true);
			ch->AutoGiveItem(upgradeItemVnum, dropItemCount, -1, true);
#ifdef ENABLE_EXTENDED_BATTLE_PASS
			ch->UpdateExtBattlePassMissionProgress(FISH_FISHING, 1, itemVnum);
#endif
#ifdef ENABLE_PLAYER_STATS_SYSTEM
			ch->PointChange(POINT_FISHING, dropItemCount);
#endif
			if (number(0, 100) <= bookDropPct)
			{
				ch->AutoGiveItem(upgradeBook, dropItemCount, -1, true);
			}

		}
		else if (info->step > 1)
		{
			FishingFail(ch);
		}
		else
		{
			TPacketGCFishing p;
			p.header = HEADER_GC_FISHING;
			p.subheader = FISHING_SUBHEADER_GC_STOP;
			p.info = ch->GetVID();
			ch->PacketAround(&p, sizeof(p));
		}

		if (info->step)
		{
			FishingPractice(ch);
		}
		//Motion(MOTION_FISHING_PULL);
	}

	void UseFish(LPCHARACTER ch, LPITEM item)
	{
		int idx = item->GetVnum() - fish_info[2].vnum + 2;

		if (idx <= 1 || idx >= MAX_FISH)
			return;

		int r = number(1, 10000);

		item->SetCount(item->GetCount() - 1);

		if (r >= 4001)
		{
			ch->AutoGiveItem(fish_info[idx].dead_vnum);
		}
		else if (r >= 2001)
		{
			ch->AutoGiveItem(FISH_BONE_VNUM);
		}
		else
		{
			// 1000 500 300 100 50 30 10 5 4 1
			static int s_acc_prob[NUM_USE_RESULT_COUNT] = { 1000, 1500, 1800, 1900, 1950, 1980, 1990, 1995, 1999, 2000 };
			int u_index = std::lower_bound(s_acc_prob, s_acc_prob + NUM_USE_RESULT_COUNT, r) - s_acc_prob;

			switch (fish_info[idx].used_table[u_index])
			{
			case USED_TREASURE_MAP:	// 3
			case USED_NONE:		// 0
			case USED_WATER_STONE:	// 2
				ch->NewChatPacket(FISHING_3);
				break;

			case USED_SHELLFISH:	// 1
				ch->NewChatPacket(FISHING_4);
				ch->AutoGiveItem(SHELLFISH_VNUM);
				break;

			case USED_EARTHWARM:	// 4
				ch->NewChatPacket(FISHING_5);
				ch->AutoGiveItem(EARTHWORM_VNUM);
				break;

			default:
				ch->AutoGiveItem(fish_info[idx].used_table[u_index]);
				break;
			}
		}

#ifdef ENABLE_EXTENDED_BATTLE_PASS
		ch->UpdateExtBattlePassMissionProgress(FISH_CATCH, 1, item->GetVnum());
#endif
	}

	void Grill(LPCHARACTER ch, LPITEM item)
	{
		int idx = -1;
		DWORD vnum = item->GetVnum();
		if (vnum >= 27803 && vnum <= 27830)
			idx = vnum - 27800;
		if (vnum >= 27833 && vnum <= 27860)
			idx = vnum - 27830;
		if (idx == -1)
			return;

		int count = item->GetCount();

		ch->NewChatPacket(FISHING_TALK_06, "%s", item->GetName());
		item->SetCount(0);
		ch->AutoGiveItem(fish_info[idx].grill_vnum, count);

#ifdef ENABLE_EXTENDED_BATTLE_PASS
		ch->UpdateExtBattlePassMissionProgress(FISH_GRILL, count, item->GetVnum());
#endif
	}

	bool RefinableRod(LPITEM rod)
	{
		if (rod->GetType() != ITEM_ROD)
			return false;

		if (rod->IsEquipped())
			return false;

		return (rod->GetSocket(0) == rod->GetValue(2));
	}

	int RealRefineRod(LPCHARACTER ch, LPITEM item)
	{
		if (!ch || !item)
			return 2;

		// REFINE_ROD_HACK_BUG_FIX
		if (!RefinableRod(item))
		{
			sys_err("REFINE_ROD_HACK pid(%u) item(%s:%d)", ch->GetPlayerID(), item->GetName(), item->GetID());
			return 2;
		}
		// END_OF_REFINE_ROD_HACK_BUG_FIX

		LPITEM rod = item;

		if (number(1, 100) <= rod->GetValue(3))
		{
			LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(rod->GetRefinedVnum(), 1);

			if (pkNewItem)
			{
				CELL_UINT bCell = rod->GetCell();

				ITEM_MANAGER::instance().RemoveItem(rod, "REMOVE (REFINE FISH_ROD)");
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, bCell));
				return 1;
			}

			return 2;
		}
		else
		{
#ifdef ENABLE_FISHINGROD_RENEWAL
			{
				int cur = rod->GetSocket(0);
				rod->SetSocket(0, (cur > 0) ? (cur - (cur * 10 / 100)) : 0);
				return 0;
			}
#else
			LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(rod->GetValue(4), 1);
			if (pkNewItem)
			{
				CELL_UINT bCell = rod->GetCell();

				ITEM_MANAGER::instance().RemoveItem(rod, "REMOVE (REFINE FISH_ROD)");
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, bCell));
				return 0;
			}
#endif

			return 2;
		}
	}
#endif
}

#ifdef __FISHING_MAIN__
int main(int argc, char** argv)
{
	//srandom(time(0) + getpid());
	srandomdev();
	/*
	   struct SFishInfo
	   {
	   const char* name;

	   DWORD vnum;
	   DWORD dead_vnum;
	   DWORD grill_vnum;

	   int prob[3];
	   int difficulty;

	   int limit_type;
	   int limits[3];

	   int time_type;
	   int length_range[3]; // MIN MAX EXTRA_MAX : 99% MIN~MAX, 1% MAX~EXTRA_MAX

	   int used_table[NUM_USE_RESULT_COUNT];
	// 6000 2000 1000 500 300 100 50 30 10 5 4 1
	};
	 */
	using namespace fishing;

	Initialize();

	for (int i = 0; i < MAX_FISH; ++i)
	{
		printf("%s\t%u\t%u\t%u\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
			fish_info[i].name,
			fish_info[i].vnum,
			fish_info[i].dead_vnum,
			fish_info[i].grill_vnum,
			fish_info[i].prob[0],
			fish_info[i].prob[1],
			fish_info[i].prob[2],
			fish_info[i].difficulty,
			fish_info[i].time_type,
			fish_info[i].length_range[0],
			fish_info[i].length_range[1],
			fish_info[i].length_range[2]);

		for (int j = 0; j < NUM_USE_RESULT_COUNT; ++j)
			printf("\t%d", fish_info[i].used_table[j]);

		puts("");
	}

	return 1;
}

#endif