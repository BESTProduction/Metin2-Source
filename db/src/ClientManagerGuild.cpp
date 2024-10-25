// vim:ts=4 sw=4
#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "GuildManager.h"

void CClientManager::GuildCreate(CPeer* peer, DWORD dwGuildID)
{
	ForwardPacket(HEADER_DG_GUILD_LOAD, &dwGuildID, sizeof(DWORD));
	CGuildManager::instance().Load(dwGuildID);
}

void CClientManager::GuildChangeGrade(CPeer* peer, TPacketGuild* p)
{
	ForwardPacket(HEADER_DG_GUILD_CHANGE_GRADE, p, sizeof(TPacketGuild));
}

void CClientManager::GuildAddMember(CPeer* peer, TPacketGDGuildAddMember* p)
{
	CGuildManager::instance().TouchGuild(p->dwGuild);
	char szQuery[512];

	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO guild_member%s VALUES(%u, %u, %d, 0, 0)",
		GetTablePostfix(), p->dwPID, p->dwGuild, p->bGrade);

	std::unique_ptr<SQLMsg> pmsg_insert(CDBManager::instance().DirectQuery(szQuery));

	snprintf(szQuery, sizeof(szQuery),
		"SELECT pid, grade, is_general, offer, level, job, name FROM guild_member%s, player%s WHERE guild_id = %u and pid = id and pid = %u", GetTablePostfix(), GetTablePostfix(), p->dwGuild, p->dwPID);

	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(szQuery));
	if (pmsg->Get()->uiNumRows == 0)
	{
		sys_err("Query failed when getting guild member data %s", pmsg->stQuery.c_str());
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

	if (!row[0] || !row[1])
		return;

	TPacketDGGuildMember dg;

	dg.dwGuild = p->dwGuild;
	str_to_number(dg.dwPID, row[0]);
	str_to_number(dg.bGrade, row[1]);
	str_to_number(dg.isGeneral, row[2]);
	str_to_number(dg.dwOffer, row[3]);
	str_to_number(dg.bLevel, row[4]);
	str_to_number(dg.bJob, row[5]);
	strlcpy(dg.szName, row[6], sizeof(dg.szName));

	ForwardPacket(HEADER_DG_GUILD_ADD_MEMBER, &dg, sizeof(TPacketDGGuildMember));
}

void CClientManager::GuildRemoveMember(CPeer* peer, TPacketGuild* p)
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery), "DELETE FROM guild_member%s WHERE pid=%u and guild_id=%u", GetTablePostfix(), p->dwInfo, p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	// @fixme202 new_+withdraw_time
	snprintf(szQuery, sizeof(szQuery), "REPLACE INTO quest%s (dwPID, szName, szState, lValue) VALUES(%u, 'guild_manage', 'new_withdraw_time', %u)", GetTablePostfix(), p->dwInfo, (DWORD)GetCurrentTime());
	CDBManager::instance().AsyncQuery(szQuery);

	ForwardPacket(HEADER_DG_GUILD_REMOVE_MEMBER, p, sizeof(TPacketGuild));
}

void CClientManager::GuildSkillUpdate(CPeer* peer, TPacketGuildSkillUpdate* p)
{
	ForwardPacket(HEADER_DG_GUILD_SKILL_UPDATE, p, sizeof(TPacketGuildSkillUpdate));
}

void CClientManager::GuildExpUpdate(CPeer* peer, TPacketGuildExpUpdate* p)
{
	ForwardPacket(HEADER_DG_GUILD_EXP_UPDATE, p, sizeof(TPacketGuildExpUpdate), 0, peer);
}

void CClientManager::GuildChangeMemberData(CPeer* peer, TPacketGuildChangeMemberData* p)
{
	ForwardPacket(HEADER_DG_GUILD_CHANGE_MEMBER_DATA, p, sizeof(TPacketGuildChangeMemberData), 0, peer);
}

void CClientManager::GuildDisband(CPeer* peer, TPacketGuild* p)
{
	char szQuery[512];

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM guild%s WHERE id=%u", GetTablePostfix(), p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM guild_grade%s WHERE guild_id=%u", GetTablePostfix(), p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	// @fixme401 (withdraw -> new_disband)_time
	snprintf(szQuery, sizeof(szQuery), "REPLACE INTO quest%s (dwPID, szName, szState, lValue) SELECT pid, 'guild_manage', 'new_disband_time', %u FROM guild_member%s WHERE guild_id = %u", GetTablePostfix(), (DWORD)GetCurrentTime(), GetTablePostfix(), p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM guild_member%s WHERE guild_id=%u", GetTablePostfix(), p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM guild_comment%s WHERE guild_id=%u", GetTablePostfix(), p->dwGuild);
	CDBManager::instance().AsyncQuery(szQuery);

	ForwardPacket(HEADER_DG_GUILD_DISBAND, p, sizeof(TPacketGuild));
}

const char* __GetWarType(int n)
{
	switch (n)
	{
	case 0:
		return "Field";
	case 1:
		return "Theater";
	case 2:
		return "CTF"; //Capture The Flag
	default:
		return "Wrong number";
	}
}

void CClientManager::GuildWar(CPeer* peer, TPacketGuildWar* p)
{
	switch (p->bWar)
	{
	case GUILD_WAR_SEND_DECLARE:
		CGuildManager::instance().AddDeclare(p->bType, p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_REFUSE:
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_WAIT_START:
	case GUILD_WAR_RESERVE:
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);

		if (!CGuildManager::instance().ReserveWar(p))
			p->bWar = GUILD_WAR_CANCEL;
		else
			p->bWar = GUILD_WAR_RESERVE;

		break;

	case GUILD_WAR_ON_WAR:
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().StartWar(p->bType, p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_OVER:
		CGuildManager::instance().RecvWarOver(p->dwGuildFrom, p->dwGuildTo, p->bType, p->lWarPrice);
		break;

	case GUILD_WAR_END:
		CGuildManager::instance().RecvWarEnd(p->dwGuildFrom, p->dwGuildTo);
		return;

	case GUILD_WAR_CANCEL:
		CGuildManager::instance().CancelWar(p->dwGuildFrom, p->dwGuildTo);
		break;
	}

	ForwardPacket(HEADER_DG_GUILD_WAR, p, sizeof(TPacketGuildWar));
}

void CClientManager::GuildWarScore(CPeer* peer, TPacketGuildWarScore* p)
{
	CGuildManager::instance().UpdateScore(p->dwGuildGainPoint, p->dwGuildOpponent, p->lScore, p->lBetScore);
}

void CClientManager::GuildChangeLadderPoint(TPacketGuildLadderPoint* p)
{
	CGuildManager::instance().ChangeLadderPoint(p->dwGuild, p->lChange);
}

void CClientManager::GuildUseSkill(TPacketGuildUseSkill* p)
{
	CGuildManager::instance().UseSkill(p->dwGuild, p->dwSkillVnum, p->dwCooltime);
	SendGuildSkillUsable(p->dwGuild, p->dwSkillVnum, false);
}

void CClientManager::SendGuildSkillUsable(DWORD guild_id, DWORD dwSkillVnum, bool bUsable)
{
	TPacketGuildSkillUsableChange p;

	p.dwGuild = guild_id;
	p.dwSkillVnum = dwSkillVnum;
	p.bUsable = bUsable;

	ForwardPacket(HEADER_DG_GUILD_SKILL_USABLE_CHANGE, &p, sizeof(TPacketGuildSkillUsableChange));
}

void CClientManager::GuildChangeMaster(TPacketChangeGuildMaster* p)
{
	if (CGuildManager::instance().ChangeMaster(p->dwGuildID, p->idFrom, p->idTo) == true)
	{
		TPacketChangeGuildMaster packet;
		packet.dwGuildID = p->dwGuildID;
		packet.idFrom = 0;
		packet.idTo = 0;

		ForwardPacket(HEADER_DG_ACK_CHANGE_GUILD_MASTER, &packet, sizeof(packet));
	}
}