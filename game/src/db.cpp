#include "stdafx.h"
#include <sstream>
#include "../../common/length.h"
#include "db.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "p2p.h"
#include "login_data.h"
#include "locale_service.h"
#include "spam.h"
#include "shutdown_manager.h"

DBManager::DBManager() : m_bIsConnect(false)
{
}

DBManager::~DBManager()
{
}

bool DBManager::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	if (!m_sql_direct.Setup(host, user, pwd, db, g_stLocale.c_str(), true, port))
		sys_err("cannot open direct sql connection to host %s", host);
	return m_bIsConnect;
}

void DBManager::Query(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	m_sql.AsyncQuery(szQuery);
}

std::unique_ptr<SQLMsg> DBManager::DirectQuery(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	return m_sql_direct.DirectQuery(szQuery);
}

bool DBManager::IsConnected()
{
	return m_bIsConnect;
}

void DBManager::ReturnQuery(int iType, DWORD dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* DBManager::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void DBManager::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		if (NULL != pMsg->pvUserData)
		{
			switch (reinterpret_cast<CQueryInfo*>(pMsg->pvUserData)->iQueryType)
			{
			case QUERY_TYPE_RETURN:
				AnalyzeReturnQuery(pMsg);
				break;

			case QUERY_TYPE_FUNCTION:
			{
				CFuncQueryInfo* qi = reinterpret_cast<CFuncQueryInfo*>(pMsg->pvUserData);
				qi->f(pMsg);
				M2_DELETE(qi);
			}
			break;

			case QUERY_TYPE_AFTER_FUNCTION:
			{
				CFuncAfterQueryInfo* qi = reinterpret_cast<CFuncAfterQueryInfo*>(pMsg->pvUserData);
				qi->f();
				M2_DELETE(qi);
			}
			break;
			}
		}

		delete pMsg;
	}
}

CLoginData* DBManager::GetLoginData(DWORD dwKey)
{
	std::map<DWORD, CLoginData*>::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return NULL;

	return it->second;
}

void DBManager::InsertLoginData(CLoginData* pkLD)
{
	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
}

void DBManager::DeleteLoginData(CLoginData* pkLD)
{
	std::map<DWORD, CLoginData*>::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

	if (it == m_map_pkLoginData.end())
		return;
	M2_DELETE(it->second);
	m_map_pkLoginData.erase(it);
}

void DBManager::SendLoginPing(const char* c_pszLogin)
{
	TPacketGGLoginPing ptog;

	ptog.bHeader = HEADER_GG_LOGIN_PING;
	strlcpy(ptog.szLogin, c_pszLogin, sizeof(ptog.szLogin));

	if (!g_pkAuthMasterDesc)  // If I am master, broadcast to others
	{
		P2P_MANAGER::instance().Send(&ptog, sizeof(TPacketGGLoginPing));
	}
	else // If I am slave send login ping to master
	{
		g_pkAuthMasterDesc->Packet(&ptog, sizeof(TPacketGGLoginPing));
	}
}

void DBManager::SendAuthLogin(LPDESC d)
{
	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = GetLoginData(d->GetLoginKey());

	if (!pkLD)
		return;

	TPacketGDAuthLogin ptod;
	ptod.dwID = r.id;

	trim_and_lower(r.login, ptod.szLogin, sizeof(ptod.szLogin));
	strlcpy(ptod.szSocialID, r.social_id, sizeof(ptod.szSocialID));
	ptod.dwLoginKey = d->GetLoginKey();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	ptod.bLanguage = r.bLanguage;
#endif
	thecore_memcpy(ptod.iPremiumTimes, pkLD->GetPremiumPtr(), sizeof(ptod.iPremiumTimes));
	thecore_memcpy(&ptod.adwClientKey, pkLD->GetClientKey(), sizeof(DWORD) * 4);
#ifdef ENABLE_HWID
	strlcpy(ptod.hwid, r.hwid, sizeof(ptod.hwid));
	memcpy(ptod.hwidID, r.hwidID, sizeof(ptod.hwidID));
	ptod.descHwidID = r.descHwidID;
#endif
	db_clientdesc->DBPacket(HEADER_GD_AUTH_LOGIN, d->GetHandle(), &ptod, sizeof(TPacketGDAuthLogin));
	SendLoginPing(r.login);
}

void DBManager::LoginPrepare(LPDESC d, DWORD* pdwClientKey, int* paiPremiumTimes)
{
	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = M2_NEW CLoginData;

	pkLD->SetKey(d->GetLoginKey());
	pkLD->SetLogin(r.login);
	pkLD->SetIP(d->GetHostName());
	pkLD->SetClientKey(pdwClientKey);

	if (paiPremiumTimes)
	{
		pkLD->SetPremium(paiPremiumTimes);
	}

	InsertLoginData(pkLD);
	SendAuthLogin(d);
}

void DBManager::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;

	switch (qi->iType)
	{
	case QID_AUTH_LOGIN:
	{
		TPacketCGLogin3* pinfo = (TPacketCGLogin3*)qi->pvData;
		LPDESC d = DESC_MANAGER::instance().FindByLoginKey(qi->dwIdent);

		if (!d)
		{
			M2_DELETE(pinfo);
			break;
		}

		d->SetLogin(pinfo->login);
		if (pMsg->Get()->uiNumRows == 0)
		{
			LoginFailure(d, "NOID");
			M2_DELETE(pinfo);
		}
		else
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			int col = 0;
			char szEncrytPassword[45 + 1] = { 0, };
			char szPassword[45 + 1] = { 0, };
			char szSocialID[SOCIAL_ID_MAX_LEN + 1] = { 0, };
			char szStatus[ACCOUNT_STATUS_MAX_LEN + 1] = { 0, };
			DWORD dwID = 0;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			BYTE bLanguage = LOCALE_DEFAULT;
#endif

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szEncrytPassword, row[col++], sizeof(szEncrytPassword));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szPassword, row[col++], sizeof(szPassword));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szSocialID, row[col++], sizeof(szSocialID));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			str_to_number(dwID, row[col++]);

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szStatus, row[col++], sizeof(szStatus));

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}
			str_to_number(bLanguage, row[col++]);
#endif

#ifdef ENABLE_HWID
			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}
			DWORD hwidID = 0;
			str_to_number(hwidID, row[col++]);
			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}
			DWORD hwidID2 = 0;
			str_to_number(hwidID2, row[col++]);
#endif

			BYTE bNotAvail = 0;
			str_to_number(bNotAvail, row[col++]);

			int aiPremiumTimes[PREMIUM_MAX_NUM];
			memset(&aiPremiumTimes, 0, sizeof(aiPremiumTimes));

			char szCreateDate[256] = "00000000";

			{
				str_to_number(aiPremiumTimes[PREMIUM_EXP], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_ITEM], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_SAFEBOX], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_AUTOLOOT], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_FISH_MIND], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_MARRIAGE_FAST], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_GOLD], row[col++]);

				{
					long retValue = 0;
					str_to_number(retValue, row[col]);

					time_t create_time = retValue;
					struct tm* tm1;
					tm1 = localtime(&create_time);
					strftime(szCreateDate, 255, "%Y%m%d", tm1);
				}
			}

			int nPasswordDiff = strcmp(szEncrytPassword, szPassword);

			if (nPasswordDiff)
			{
				LoginFailure(d, "WRONGPWD");
				M2_DELETE(pinfo);
			}
			else if (bNotAvail)
			{
				LoginFailure(d, "NOTAVAIL");
				M2_DELETE(pinfo);
			}
			else if (DESC_MANAGER::instance().FindByLoginName(pinfo->login))
			{
				LoginFailure(d, "ALREADY");
				M2_DELETE(pinfo);
			}
			else if (!CShutdownManager::Instance().CheckCorrectSocialID(szSocialID))
			{
				LoginFailure(d, "BADSCLID");
				M2_DELETE(pinfo);
			}
			else if (CShutdownManager::Instance().CheckShutdownAge(szSocialID) && CShutdownManager::Instance().CheckShutdownTime())
			{
				LoginFailure(d, "AGELIMIT");
				M2_DELETE(pinfo);
			}
			else if (strcmp(szStatus, "OK"))
			{
				LoginFailure(d, szStatus);
				M2_DELETE(pinfo);
			}
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			else if (!pinfo->bLanguage)
			{
				LoginFailure(d, "NOLANG");
				sys_log(0, "No language selected.");
				M2_DELETE(pinfo);
			}
			else if (pinfo->bLanguage >= LOCALE_MAX_NUM)
			{
				LoginFailure(d, "INVLANG");
				sys_log(0, "Invalid language selected.");
				M2_DELETE(pinfo);
			}
#endif
			else
			{
				{
					if (strncmp(szCreateDate, g_stBlockDate.c_str(), 8) >= 0)
					{
						LoginFailure(d, "BLKLOGIN");
						M2_DELETE(pinfo);
						break;
					}

#ifdef ENABLE_CLIENT_VERSION_AND_ONLY_GM				
					if (g_serverState == true)
					{
						std::unique_ptr<SQLMsg> _gm_login(DBManager::instance().DirectQuery("SELECT mAccount FROM common.gmlist WHERE mAccount='%s'", pinfo->login));
						if (!_gm_login->uiSQLErrno)
						{
							if (_gm_login->Get()->uiNumRows < 1)
							{
								LoginFailure(d, "NOSERVER");
								M2_DELETE(pinfo);
								break;
							}
						}
					}
#endif

					char szQuery[1024];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
					snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW(), language=%u WHERE id=%u", pinfo->bLanguage, dwID);
#else
					snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW() WHERE id=%u", dwID);
#endif
					std::unique_ptr<SQLMsg> msg(DBManager::instance().DirectQuery(szQuery));
				}

				TAccountTable& r = d->GetAccountTable();

				r.id = dwID;
				trim_and_lower(pinfo->login, r.login, sizeof(r.login));
				strlcpy(r.passwd, pinfo->passwd, sizeof(r.passwd));
				strlcpy(r.social_id, szSocialID, sizeof(r.social_id));
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				r.bLanguage = pinfo->bLanguage;
#endif
#ifdef ENABLE_HWID
				strlcpy(r.hwid, pinfo->hwid, sizeof(r.hwid));
				r.hwidID[0] = hwidID;
				r.hwidID[1] = hwidID2;
				r.descHwidID = 0;
#endif
				DESC_MANAGER::instance().ConnectAccount(r.login, d);

				LoginPrepare(d, pinfo->adwClientKey, aiPremiumTimes);
				//By SeMinZ
				M2_DELETE(pinfo);
				break;
			}
		}
	}
	break;

	case QID_SAFEBOX_SIZE:
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

		if (ch)
		{
			if (pMsg->Get()->uiNumRows > 0)
			{
				MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
				int	size = 0;
				str_to_number(size, row[0]);
				ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * size);
			}
		}
	}
	break;

	case QID_HIGHSCORE_REGISTER:
	{
		THighscoreRegisterQueryInfo* info = (THighscoreRegisterQueryInfo*)qi->pvData;
		bool bQuery = true;

		if (pMsg->Get()->uiNumRows)
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

			if (row && row[0])
			{
				int iCur = 0;
				str_to_number(iCur, row[0]);

				if ((info->bOrder && iCur >= info->iValue) ||
					(!info->bOrder && iCur <= info->iValue))
					bQuery = false;
			}
		}

		if (bQuery)
			Query("REPLACE INTO highscore%s VALUES('%s', %u, %d)",
				get_table_postfix(), info->szBoard, info->dwPID, info->iValue);

		M2_DELETE(info);
	}
	break;

	case QID_HIGHSCORE_SHOW:
	{
	}
	break;

	// BLOCK_CHAT
	case QID_BLOCK_CHAT_LIST:
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

		if (ch == NULL)
			break;
		if (pMsg->Get()->uiNumRows)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %s sec", row[0], row[1]);
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "No one currently blocked.");
		}
	}
	break;
	// END_OF_BLOCK_CHAT
	default:
		sys_err("FATAL ERROR!!! Unhandled return query id %d", qi->iType);
		break;
	}

	M2_DELETE(qi);
}

size_t DBManager::EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize)
{
	return m_sql_direct.EscapeString(dst, dstSize, src, srcSize);
}

AccountDB::AccountDB() :
	m_IsConnect(false)
{
}

bool AccountDB::IsConnected()
{
	return m_IsConnect;
}

bool AccountDB::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	m_IsConnect = m_sql_direct.Setup(host, user, pwd, db, "", true, port);

	if (false == m_IsConnect)
	{
		fprintf(stderr, "cannot open direct sql connection to host: %s user: %s db: %s\n", host, user, db);
		return false;
	}

	return m_IsConnect;
}

bool AccountDB::ConnectAsync(const char* host, const int port, const char* user, const char* pwd, const char* db, const char* locale)
{
	m_sql.Setup(host, user, pwd, db, locale, false, port);
	return true;
}

void AccountDB::SetLocale(const std::string& stLocale)
{
	m_sql_direct.SetLocale(stLocale);
	m_sql_direct.QueryLocaleSet();
}

std::unique_ptr<SQLMsg> AccountDB::DirectQuery(const char* query)
{
	return m_sql_direct.DirectQuery(query);
}

void AccountDB::AsyncQuery(const char* query)
{
	m_sql.AsyncQuery(query);
}

void AccountDB::ReturnQuery(int iType, DWORD dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* AccountDB::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void AccountDB::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;

		switch (qi->iQueryType)
		{
		case QUERY_TYPE_RETURN:
			AnalyzeReturnQuery(pMsg);
			break;
		}
	}

	delete pMsg;
}

extern unsigned int g_uiSpamReloadCycle;

enum EAccountQID
{
	QID_SPAM_DB,
};

static LPEVENT s_pkReloadSpamEvent = NULL;

EVENTINFO(reload_spam_event_info)
{
	// used to send command
	DWORD empty;
};

EVENTFUNC(reload_spam_event)
{
	AccountDB::instance().ReturnQuery(QID_SPAM_DB, 0, NULL, "SELECT word, score FROM spam_db WHERE type='SPAM'");
	return PASSES_PER_SEC(g_uiSpamReloadCycle);
}

// #define ENABLE_SPAMDB_REFRESH
void LoadSpamDB()
{
	AccountDB::instance().ReturnQuery(QID_SPAM_DB, 0, NULL, "SELECT word, score FROM spam_db WHERE type='SPAM'");
#ifdef ENABLE_SPAMDB_REFRESH
	if (NULL == s_pkReloadSpamEvent)
	{
		reload_spam_event_info* info = AllocEventInfo<reload_spam_event_info>();
		s_pkReloadSpamEvent = event_create(reload_spam_event, info, PASSES_PER_SEC(g_uiSpamReloadCycle));
	}
#endif
}

void CancelReloadSpamEvent() {
	s_pkReloadSpamEvent = NULL;
}

void AccountDB::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;

	switch (qi->iType)
	{
	case QID_SPAM_DB:
	{
		if (pMsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row;

			SpamManager::instance().Clear();

			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
				SpamManager::instance().Insert(row[0], atoi(row[1]));
		}
	}
	break;
	}

	M2_DELETE(qi);
}