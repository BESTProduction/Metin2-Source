#include "stdafx.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "battle.h"
#include "guild.h"
#include "utils.h"
#include "locale_service.h"
#include "lua_incl.h"
#include "arena.h"
#include "horsename_manager.h"
#include "item.h"
#include "DragonSoul.h"
#include "../../common/Controls.h"
#ifdef ENABLE_BUFFI_SYSTEM
#include "BuffiSystem.h"
#endif
#define IS_NO_SAVE_AFFECT(type) ((type) == AFFECT_WAR_FLAG || (type) == AFFECT_REVIVE_INVISIBLE || ((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END)) // @fixme156 added MOUNT_BONUS (if the game core crashes, the bonus would double if present in player.affect)
#define IS_NO_CLEAR_ON_DEATH_AFFECT(type) ((type) == AFFECT_BLOCK_CHAT || ((type) >= 500 && (type) < 600))

#ifdef ENABLE_ADD_REALTIME_AFFECT
bool IsRealTimeAffect(DWORD affectIndex)
{
	switch(affectIndex)
	{
#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
		case AFFECT_PREMIUM:
#endif
#ifdef ENABLE_AUTO_PICK_UP
		case AFFECT_AUTO_PICK_UP:
#endif
#ifdef ENABLE_VOTE4BUFF
		case AFFECT_VOTE4BUFF:
#endif
#ifdef ENABLE_EXTENDED_BATTLE_PASS
		case AFFECT_BATTLEPASS:
#endif
#ifdef ENABLE_METIN_QUEUE
		case AFFECT_METIN_QUEUE:
#endif
#ifdef ENABLE_NAMING_SCROLL
		case AFFECT_NAMING_SCROLL_MOUNT:
		case AFFECT_NAMING_SCROLL_PET:
		case AFFECT_NAMING_SCROLL_BUFFI:
#endif
			return true;
	}

#if defined(ENABLE_EXTENDED_BLEND_AFFECT) || defined(ENABLE_BLEND_AFFECT)
	if (affectIndex >= AFFECT_FISH_ITEM_01 && affectIndex <= AFFECT_FISH_ITEM_12)
		return true;
#endif

	return false;
}
#endif

void SendAffectRemovePacket(LPDESC d, DWORD pid, DWORD type, BYTE point)
{
	TPacketGCAffectRemove ptoc;
	ptoc.bHeader = HEADER_GC_AFFECT_REMOVE;
	ptoc.dwType = type;
	ptoc.bApplyOn = point;
	d->Packet(&ptoc, sizeof(TPacketGCAffectRemove));

	TPacketGDRemoveAffect ptod;
	ptod.dwPID = pid;
	ptod.dwType = type;
	ptod.bApplyOn = point;
	db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &ptod, sizeof(ptod));
}

void SendAffectAddPacket(LPDESC d, CAffect* pkAff)
{
	TPacketGCAffectAdd ptoc;
	ptoc.bHeader = HEADER_GC_AFFECT_ADD;
	ptoc.elem.dwType = pkAff->dwType;
	ptoc.elem.bApplyOn = pkAff->bApplyOn;
	ptoc.elem.lApplyValue = pkAff->lApplyValue;
	ptoc.elem.dwFlag = pkAff->dwFlag;
#ifdef ENABLE_ADD_REALTIME_AFFECT
	ptoc.elem.lDuration		= IsRealTimeAffect(ptoc.elem.dwType) ? pkAff->lDuration - time(0): pkAff->lDuration;
#else
	ptoc.elem.lDuration		= pkAff->lDuration;
#endif
	ptoc.elem.lSPCost = pkAff->lSPCost;
	d->Packet(&ptoc, sizeof(TPacketGCAffectAdd));
}
////////////////////////////////////////////////////////////////////
// Affect
CAffect* CHARACTER::FindAffect(DWORD dwType, BYTE bApply) const
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAffect = *it++;
		if (!pkAffect)// @fixme199
			return NULL;
		if (pkAffect->dwType == dwType && (bApply == APPLY_NONE || bApply == pkAffect->bApplyOn))
			return pkAffect;
	}

	return NULL;
}

EVENTFUNC(affect_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("affect_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (!ch->UpdateAffect())
		return 0;
	else
		return passes_per_sec;
}

bool CHARACTER::UpdateAffect()
{
// lightwork no skills without weapons.
	if (IsAffectFlag(AFF_GWIGUM) && !GetWear(WEAR_WEAPON))
		RemoveAffect(SKILL_GWIGEOM);

	if (IsAffectFlag(AFF_GEOMGYEONG) && !GetWear(WEAR_WEAPON))
		RemoveAffect(SKILL_GEOMKYUNG);
// lightwork no skills without weapons.

	if (GetPoint(POINT_HP_RECOVERY) > 0)
	{
		if (GetMaxHP() <= GetHP())
		{
			PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
		}
		else
		{
			HP_LL iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 7 / 100);

			PointChange(POINT_HP, iVal);
			PointChange(POINT_HP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_SP_RECOVERY) > 0)
	{
		if (GetMaxSP() <= GetSP())
			PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
		else
		{
			int iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);

			PointChange(POINT_SP, iVal);
			PointChange(POINT_SP_RECOVERY, -iVal);
		}
	}

	//@Lightwork#88
	if (!GetWear(WEAR_WEAPON))
	{
		if (IsAffectFlag(AFF_GEOMGYEONG))
		{
			RemoveAffect(SKILL_GEOMKYUNG);
		}

		if (IsAffectFlag(AFF_GWIGUM))
		{
			RemoveAffect(SKILL_GWIGEOM);
		}
	}
	//

	if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));
	}

	if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));
	}

	AutoRecoveryItemProcess(AFFECT_AUTO_HP_RECOVERY);
	AutoRecoveryItemProcess(AFFECT_AUTO_SP_RECOVERY);

	if (GetMaxStamina() > GetStamina())
	{
		int iSec = (get_dword_time() - GetStopTime()) / 3000;
		if (iSec)
			PointChange(POINT_STAMINA, GetMaxStamina() / 1);
	}

	if (ProcessAffect())
		if (GetPoint(POINT_HP_RECOVERY) == 0 && GetPoint(POINT_SP_RECOVERY) == 0 && GetStamina() == GetMaxStamina())
		{
			m_pkAffectEvent = NULL;
			return false;
		}

	return true;
}

void CHARACTER::StartAffectEvent()
{
	if (m_pkAffectEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkAffectEvent = event_create(affect_event, info, passes_per_sec);
}

void CHARACTER::ClearAffect(bool bSave
#ifdef ENABLE_NO_CLEAR_BUFF_WHEN_MONSTER_KILL
	, bool letBuffs
#endif
)
{
	TAffectFlag afOld = m_afAffectFlag;
	WORD	wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD	wAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it;

		if (bSave)
		{
			if (IS_NO_CLEAR_ON_DEATH_AFFECT(pkAff->dwType) || IS_NO_SAVE_AFFECT(pkAff->dwType))
			{
				++it;
				continue;
			}

#ifdef ENABLE_NO_CLEAR_BUFF_WHEN_MONSTER_KILL
			if (letBuffs)
			{
				switch (pkAff->dwType)
				{
					case (SKILL_JEONGWI):
					case (SKILL_GEOMKYUNG):
					case (SKILL_CHUNKEON):
					case (SKILL_GWIGEOM):
					case (SKILL_TERROR):
					case (SKILL_JUMAGAP):
					case (SKILL_HOSIN):
					case (SKILL_REFLECT):
					case (SKILL_GICHEON):
					case (SKILL_KWAESOK):
					case (SKILL_JEUNGRYEOK):
					case (SKILL_JEOKRANG):
					case (SKILL_CHEONGRANG): // wolfman skill
					{
						++it;
						continue;
					}
				}
			}
#endif
#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
			if (pkAff->dwType == AFFECT_PREMIUM)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_METIN_QUEUE
			if (pkAff->dwType == AFFECT_METIN_QUEUE)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_AUTO_PICK_UP
			if (pkAff->dwType == AFFECT_AUTO_PICK_UP)
			{
				++it;
				continue;
			}
#endif
#if defined(ENABLE_BLEND_AFFECT) || defined(ENABLE_EXTENDED_BLEND_AFFECT)
			if (pkAff->dwType >= AFFECT_BLEND_POTION_1 && pkAff->dwType <= AFFECT_FISH_ITEM_12)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_EXTENDED_BATTLE_PASS
			if (pkAff->dwType == AFFECT_BATTLEPASS)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_LEADERSHIP_EXTENSION
			if(pkAff->dwType == AFFECT_PARTY_BONUS)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_ADDSTONE_NOT_FAIL_ITEM
			if(pkAff->dwType == AFFECT_ADDSTONE_SUCCESS_ITEM_USED)
			{
				++it;
				continue;
			}
#endif
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
			}
		}

		ComputeAffect(pkAff, false);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}

	if (afOld != m_afAffectFlag ||
		wMovSpd != GetPoint(POINT_MOV_SPEED) ||
		wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	CheckMaximumPoints();

	if (m_list_pkAffect.empty())
		event_cancel(&m_pkAffectEvent);
}

int CHARACTER::ProcessAffect()
{
	bool	bDiff = false;
	CAffect* pkAff = NULL;

	//

	//
	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int aff_idx = i + AFFECT_PREMIUM_START;

		pkAff = FindAffect(aff_idx);

		if (!pkAff)
			continue;

		int remain = GetPremiumRemainSeconds(i);

		if (remain < 0)
		{
			RemoveAffect(aff_idx);
			bDiff = true;
		}
		else
			pkAff->lDuration = remain + 1;
	}

	////////// HAIR_AFFECT
	pkAff = FindAffect(AFFECT_HAIR);
	if (pkAff)
	{
		// IF HAIR_LIMIT_TIME() < CURRENT_TIME()
		if (this->GetQuestFlag("hair.limit_time") < get_global_time())
		{
			// SET HAIR NORMAL
			this->SetPart(PART_HAIR, 0);
			// REMOVE HAIR AFFECT
			RemoveAffect(AFFECT_HAIR);
		}
		else
		{
			// INCREASE AFFECT DURATION
			++(pkAff->lDuration);
		}
	}
	////////// HAIR_AFFECT
	//

	CHorseNameManager::instance().Validate(this);

	TAffectFlag afOld = m_afAffectFlag;
	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it;

	it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		pkAff = *it;

		bool bEnd = false;

		if (!pkAff) // @fixmejanti
		{
			sys_err("Null affect! (name %s list.len: %d)", GetName(), m_list_pkAffect.size());
			it = m_list_pkAffect.erase(it);
			continue;
		}

		if (pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
		{
			if (!GetWarMap())//@fixme231
				bEnd = true;
			if (!GetGuild() || !GetGuild()->UnderAnyWar())
				bEnd = true;
		}

		if (pkAff->lSPCost > 0 && pkAff->dwType != AFFECT_AUTO_HP_RECOVERY && pkAff->dwType != AFFECT_AUTO_SP_RECOVERY && pkAff->dwType != AFFECT_AUTO_PICK_UP)
		{
			if (GetSP() < pkAff->lSPCost)
				bEnd = true;
			else
				PointChange(POINT_SP, -pkAff->lSPCost);
		}

		// AFFECT_DURATION_BUG_FIX

#ifdef ENABLE_ADD_REALTIME_AFFECT
		if(IsRealTimeAffect(pkAff->dwType))
		{
			if (time(0) > pkAff->lDuration)
			{
				bEnd = true;
#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
				if (pkAff->dwType == AFFECT_PREMIUM)
					m_isPremium = false;
#endif
#ifdef ENABLE_AUTO_PICK_UP
				else if (pkAff->dwType == AFFECT_AUTO_PICK_UP)
					m_isAutoPickUp = false;
#endif
			}

		}
		else
		{
#endif
			if (--pkAff->lDuration <= 0)
			{
				bEnd = true;
			}
#ifdef ENABLE_ADD_REALTIME_AFFECT
		}
#endif
		// END_AFFECT_DURATION_BUG_FIX

		if (bEnd)
		{
			it = m_list_pkAffect.erase(it);
			ComputeAffect(pkAff, false);
			bDiff = true;
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
			}

			CAffect::Release(pkAff);

			continue;
		}

		++it;
	}

	if (bDiff)
	{
		if (afOld != m_afAffectFlag ||
			lMovSpd != GetPoint(POINT_MOV_SPEED) ||
			lAttSpd != GetPoint(POINT_ATT_SPEED))
		{
			UpdatePacket();
		}

		CheckMaximumPoints();
	}

	if (m_list_pkAffect.empty())
		return true;

	return false;
}

void CHARACTER::SaveAffect()
{
	TPacketGDAddAffect p;

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it++;

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			continue;

		p.dwPID = GetPlayerID();
		p.elem.dwType = pkAff->dwType;
		p.elem.bApplyOn = pkAff->bApplyOn;
		p.elem.lApplyValue = pkAff->lApplyValue;
		p.elem.dwFlag = pkAff->dwFlag;
		p.elem.lDuration = pkAff->lDuration;
		p.elem.lSPCost = pkAff->lSPCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}
}

EVENTINFO(load_affect_login_event_info)
{
	DWORD pid;
	DWORD count;
	char* data;

	load_affect_login_event_info()
		: pid(0)
		, count(0)
		, data(0)
	{
	}
};

EVENTFUNC(load_affect_login_event)
{
	load_affect_login_event_info* info = dynamic_cast<load_affect_login_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("load_affect_login_event_info> <Factor> Null pointer");
		return 0;
	}

	DWORD dwPID = info->pid;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

	if (!ch)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	LPDESC d = ch->GetDesc();

	if (!d)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		ch->LoadAffect(info->count, (TPacketAffectElement*)info->data);
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
}

void CHARACTER::LoadAffect(DWORD dwCount, TPacketAffectElement* pElements)
{
	m_bIsLoadedAffect = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		load_affect_login_event_info* info = AllocEventInfo<load_affect_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPacketAffectElement) * dwCount];
		thecore_memcpy(info->data, pElements, sizeof(TPacketAffectElement) * dwCount);

		event_create(load_affect_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	ClearAffect(true);

	TAffectFlag afOld = m_afAffectFlag;

	long lMovSpd = GetPoint(POINT_MOV_SPEED);
	long lAttSpd = GetPoint(POINT_ATT_SPEED);

	for (DWORD i = 0; i < dwCount; ++i, ++pElements)
	{
		if (pElements->dwType == SKILL_MUYEONG)
			continue;

		/*if (AFFECT_AUTO_HP_RECOVERY == pElements->dwType || AFFECT_AUTO_SP_RECOVERY == pElements->dwType)
		{
			LPITEM item = FindItemByID(pElements->dwFlag);

			if (NULL == item)
				continue;

			item->Lock(true);
		}*/

		if (pElements->bApplyOn >= POINT_MAX_NUM)
		{
			sys_err("invalid affect data %s ApplyOn %u ApplyValue %d",
				GetName(), pElements->bApplyOn, pElements->lApplyValue);
			continue;
		}

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

		pkAff->dwType = pElements->dwType;
		pkAff->bApplyOn = pElements->bApplyOn;
		pkAff->lApplyValue = pElements->lApplyValue;
		pkAff->dwFlag = pElements->dwFlag;
		pkAff->lDuration = pElements->lDuration;
		pkAff->lSPCost = pElements->lSPCost;

		SendAffectAddPacket(GetDesc(), pkAff);

		ComputeAffect(pkAff, true);

	}

	if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
	{
		RemoveGoodAffect();
	}

	if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
	{
		UpdatePacket();
	}

	StartAffectEvent();

	m_bIsLoadedAffect = true;

	ComputePoints(); // @fixme156
	DragonSoul_Initialize();

	// @fixme118 BEGIN (regain affect hp/mp)
	if (!IsDead())
	{
		PointChange(POINT_HP, GetMaxHP() - GetHP());
		PointChange(POINT_SP, GetMaxSP() - GetSP());
	}

#ifdef ENABLE_NAMING_SCROLL
	if (GetMapIndex() != 113)
	{
#ifdef ENABLE_PET_COSTUME_SYSTEM
		CheckPet();
#endif

		if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == false)
		{
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			CheckMount();
#endif
#ifdef ENABLE_BUFFI_SYSTEM
			int buffiTime = GetQuestFlag("buffi.summon");

			if ((buffiTime > get_global_time() || buffiTime == 31) && GetBuffiSystem())
			{
				if (FindItemByID(GetQuestFlag("buffi.itemid")))
				{
					GetBuffiSystem()->Summon();
				}
				else
				{
					SetQuestFlag("buffi.summon", 0);
					SetQuestFlag("buffi.itemid", 0);
				}
			}
			else if (buffiTime)
			{
				SetQuestFlag("buffi.summon", 0);
				SetQuestFlag("buffi.itemid", 0);
			}
#endif
		}
	}
#endif

#ifdef ENABLE_PREMIUM_MEMBER_SYSTEM
	if (FindAffect(AFFECT_PREMIUM))
		m_isPremium = true;
#endif

#ifdef ENABLE_AUTO_PICK_UP
	CAffect* autoPickup = FindAffect(AFFECT_AUTO_PICK_UP);
	if (autoPickup && autoPickup->lSPCost == 1)
	{
		m_isAutoPickUp = true;
	}	
#endif

#ifdef ENABLE_VOTE4BUFF
	CheckVoteBuff();
#endif

	// @fixme118 END
}

bool CHARACTER::AddAffect(DWORD dwType, BYTE bApplyOn, long lApplyValue, DWORD dwFlag, long lDuration, long lSPCost, bool bOverride, bool IsCube)
{
	// CHAT_BLOCK
	if (dwType == AFFECT_BLOCK_CHAT && lDuration > 1)
	{
		NewChatPacket(GM_MESSENGER_IN_YOU_BLOCK);
	}
	// END_OF_CHAT_BLOCK

	if (lDuration == 0)
	{
		sys_err("Character::AddAffect lDuration == 0 type %d", lDuration, dwType);
		lDuration = 1;
	}

#ifdef ENABLE_PASSIVE_SKILLS
	const int herbSkillLevel{ GetSkillLevel(SKILL_PASSIVE_HERBOLOGY) };
	const int herbologyRate{ (herbSkillLevel / 2) };

	if (herbologyRate >= 1)
	{

		if (dwType >= AFFECT_BLEND_POTION_1 && dwType <= AFFECT_POTION_ELEMENT_EARTH)
		{
			long newRate{ (lApplyValue * (herbologyRate+100)) / 100};

			lApplyValue = newRate;

		}
	}

#endif



#ifdef ENABLE_IGNORE_LOW_POWER_BUFF
	switch (dwType)
	{
	case SKILL_HOSIN:
	case SKILL_REFLECT:
	case SKILL_GICHEON:
	case SKILL_JEONGEOP:
	case SKILL_KWAESOK:
	case SKILL_JEUNGRYEOK:
#if defined(ENABLE_WOLFMAN_CHARACTER)
	case SKILL_CHEONGRANG:
#endif
	{
		const CAffect* pkAffect = FindAffect(dwType);
		if (!pkAffect)
			break;

		if (pkAffect->lDuration > get_global_time())
		{
			if ((int)lApplyValue < (int)pkAffect->lApplyValue)
			{
				NewChatPacket(BUFF_MSG_01, "%s|%ld|%ld", CSkillManager::instance().Get(dwType)->szName, (int)lApplyValue, (int)pkAffect->lApplyValue);
				return false;
			}
			else
			{
				RemoveAffect(dwType);
				ChatPacket(BUFF_MSG_02, "%s|%ld|%ld", CSkillManager::instance().Get(dwType)->szName, (int)lApplyValue, (int)pkAffect->lApplyValue);
			}
		}
	}
	break;

	default:
		break;
	}
#endif

	CAffect* pkAff = NULL;

	if (IsCube)
		pkAff = FindAffect(dwType, bApplyOn);
	else
		pkAff = FindAffect(dwType);

	if (dwFlag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	if (pkAff && bOverride)
	{
		ComputeAffect(pkAff, false);

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
	}
	else
	{
		pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);
	}

	pkAff->dwType = dwType;
	pkAff->bApplyOn = bApplyOn;
	pkAff->lApplyValue = lApplyValue;
	pkAff->dwFlag = dwFlag;
#ifdef ENABLE_ADD_REALTIME_AFFECT
	pkAff->lDuration = IsRealTimeAffect(dwType) ? lDuration + time(0) : lDuration;
#else
	pkAff->lDuration = lDuration;
#endif
	pkAff->lSPCost = lSPCost;

	WORD wMovSpd = GetPoint(POINT_MOV_SPEED);
	WORD wAttSpd = GetPoint(POINT_ATT_SPEED);

	ComputeAffect(pkAff, true);

	if (pkAff->dwFlag || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	if (IsPC())
	{
		SendAffectAddPacket(GetDesc(), pkAff);

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			return true;

		TPacketGDAddAffect p;
		p.dwPID = GetPlayerID();
		p.elem.dwType = pkAff->dwType;
		p.elem.bApplyOn = pkAff->bApplyOn;
		p.elem.lApplyValue = pkAff->lApplyValue;
		p.elem.dwFlag = pkAff->dwFlag;
		p.elem.lDuration = pkAff->lDuration;
		p.elem.lSPCost = pkAff->lSPCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}
#ifdef ENABLE_POWER_RANKING
	CalcutePowerRank();
#endif
	return true;
}

void CHARACTER::RefreshAffect()
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it++;
		ComputeAffect(pkAff, true);
	}
}

void CHARACTER::ComputeAffect(CAffect* pkAff, bool bAdd)
{
	if (bAdd && pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
	{
		if (!GetGuild())
			return;

		if (!GetGuild()->UnderAnyWar())
			return;
	}

	if (pkAff->dwFlag)
	{
		if (!bAdd)
			m_afAffectFlag.Reset(pkAff->dwFlag);
		else
			m_afAffectFlag.Set(pkAff->dwFlag);
	}

	if (bAdd)
		PointChange(pkAff->bApplyOn, pkAff->lApplyValue);
	else
		PointChange(pkAff->bApplyOn, -pkAff->lApplyValue);

	if (pkAff->dwType == SKILL_MUYEONG)
	{
		if (bAdd)
			StartMuyeongEvent();
		else
			StopMuyeongEvent();
	}
}

bool CHARACTER::RemoveAffect(CAffect* pkAff)
{
	if (!pkAff)
		return false;

	// AFFECT_BUF_FIX
	m_list_pkAffect.remove(pkAff);
	// END_OF_AFFECT_BUF_FIX

	ComputeAffect(pkAff, false);

	if (AFFECT_REVIVE_INVISIBLE != pkAff->dwType) //@Lightwork555
		ComputePoints();
	else  // @fixme110
		UpdatePacket();

	CheckMaximumPoints();

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
	}

	CAffect::Release(pkAff);
	return true;
}

bool CHARACTER::RemoveAffect(DWORD dwType)
{
	bool flag = false;

	CAffect* pkAff;

	while ((pkAff = FindAffect(dwType)))
	{
		RemoveAffect(pkAff);
		flag = true;
	}

	return flag;
}

bool CHARACTER::IsAffectFlag(DWORD dwAff) const
{
	return m_afAffectFlag.IsSet(dwAff);
}

void CHARACTER::RemoveGoodAffect()
{
	RemoveAffect(AFFECT_MOV_SPEED);
	RemoveAffect(AFFECT_ATT_SPEED);
	RemoveAffect(AFFECT_STR);
	RemoveAffect(AFFECT_DEX);
	RemoveAffect(AFFECT_INT);
	RemoveAffect(AFFECT_CON);
	RemoveAffect(AFFECT_CHINA_FIREWORK);
	RemoveAffect(SKILL_JEONGWI);
	RemoveAffect(SKILL_GEOMKYUNG);
	RemoveAffect(SKILL_CHUNKEON);
	RemoveAffect(SKILL_EUNHYUNG);
	RemoveAffect(SKILL_GYEONGGONG);
	RemoveAffect(SKILL_GWIGEOM);
	RemoveAffect(SKILL_TERROR);
	RemoveAffect(SKILL_JUMAGAP);
	RemoveAffect(SKILL_MANASHILED);
	RemoveAffect(SKILL_MUYEONG);//@Lightwork#35
	RemoveAffect(SKILL_HOSIN);
	RemoveAffect(SKILL_REFLECT);
	RemoveAffect(SKILL_KWAESOK);
	RemoveAffect(SKILL_JEUNGRYEOK);
	RemoveAffect(SKILL_GICHEON);
#ifdef ENABLE_WOLFMAN_CHARACTER
	RemoveAffect(SKILL_JEOKRANG);
	RemoveAffect(SKILL_CHEONGRANG);
#endif
}

bool CHARACTER::IsGoodAffect(BYTE bAffectType) const
{
	switch (bAffectType)
	{
	case (AFFECT_MOV_SPEED):
	case (AFFECT_ATT_SPEED):
	case (AFFECT_STR):
	case (AFFECT_DEX):
	case (AFFECT_INT):
	case (AFFECT_CON):
	case (AFFECT_CHINA_FIREWORK):

	case (SKILL_JEONGWI):
	case (SKILL_GEOMKYUNG):
	case (SKILL_CHUNKEON):
	case (SKILL_EUNHYUNG):
	case (SKILL_GYEONGGONG):
	case (SKILL_GWIGEOM):
	case (SKILL_TERROR):
	case (SKILL_JUMAGAP):
	case (SKILL_MANASHILED):
	case (SKILL_HOSIN):
	case (SKILL_REFLECT):
	case (SKILL_KWAESOK):
	case (SKILL_JEUNGRYEOK):
	case (SKILL_GICHEON):
#ifdef ENABLE_WOLFMAN_CHARACTER

	case (SKILL_JEOKRANG):
	case (SKILL_CHEONGRANG):
#endif
		return true;
	}
	return false;
}

void CHARACTER::RemoveBadAffect()
{
	RemovePoison();
#ifdef ENABLE_WOLFMAN_CHARACTER
	RemoveBleeding();
#endif
	RemoveFire();

	RemoveAffect(AFFECT_STUN);

	RemoveAffect(AFFECT_SLOW);

	RemoveAffect(SKILL_TUSOK);

	//RemoveAffect(SKILL_CURSE);

	//RemoveAffect(SKILL_PABUP);

	//RemoveAffect(AFFECT_FAINT);

	//RemoveAffect(AFFECT_WEB);

	//RemoveAffect(AFFECT_SLEEP);

	//RemoveAffect(AFFECT_CURSE);

	//RemoveAffect(AFFECT_PARALYZE);

	//RemoveAffect(SKILL_BUDONG);
}