#ifndef __INC_METIN_II_GAME_CMD_H__
#define __INC_METIN_II_GAME_CMD_H__

#define ACMD(name)  void (name)(LPCHARACTER ch, const char *argument, int cmd, int subcmd)
#define CMD_NAME(name) cmd_info[cmd].command

struct command_info
{
	const char* command;
	void (*command_pointer) (LPCHARACTER ch, const char* argument, int cmd, int subcmd);
	int subcmd;
	int minimum_position;
	int gm_level;
};

extern struct command_info cmd_info[];

extern void interpret_command(LPCHARACTER ch, const char* argument, size_t len);
extern void interpreter_set_privilege(const char* cmd, int lvl);

enum SCMD_ACTION
{
	SCMD_SLAP,
	SCMD_KISS,
	SCMD_FRENCH_KISS,
	SCMD_HUG,
	SCMD_LONG_HUG,
	SCMD_SHOLDER,
	SCMD_FOLD_ARM
};

enum SCMD_CMD
{
	SCMD_LOGOUT,
	SCMD_QUIT,
	SCMD_PHASE_SELECT,
	SCMD_SHUTDOWN,
};

enum SCMD_RESTART
{
	SCMD_RESTART_TOWN,
	SCMD_RESTART_HERE
};

extern void Shutdown(int iSec);
extern void SendLog(const char* c_pszBuf);
#ifdef ENABLE_FULL_NOTICE
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
extern void SendLocaleNotice(const char* c_pszNotice, bool bBigFont = false, ...); // Mainly for quests (notice_all, d.notice, etc...)
extern void SendGuildPrivNotice(const char* c_pszBuf, const char* c_pszGuildName, const char* c_pszPrivName, int iValue = 0);
extern void SendPrivNotice(const char* c_pszBuf, const char* c_pszEmpireName, const char* c_pszPrivName , int iValue = 0);
extern void TransNotice(LPDESC pkDesc, const char* c_pszBuf, ...); // Translate notice for priv setting (empire & guild)

extern void SendNotice(const char* c_pszBuf, bool bBigFont = false, ...); // Only for this game server
#else
extern void SendNotice(const char* c_pszBuf, bool bBigFont = false);
#endif

extern void BroadcastNotice(const char* c_pszBuf, bool bBigFont = false);
#else
extern void SendNotice(const char* c_pszBuf);
extern void BroadcastNotice(const char* c_pszBuf);
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
extern void SendNoticeMap(const char* c_pszBuf, int nMapIndex, bool bBigFont, ...);
#else
extern void SendNoticeMap(const char* c_pszBuf, int nMapIndex, bool bBigFont);
#endif

// LUA_ADD_GOTO_INFO
extern void CHARACTER_AddGotoInfo(const std::string& c_st_name, BYTE empire, int mapIndex, DWORD x, DWORD y);
// END_OF_LUA_ADD_GOTO_INFO

#endif
