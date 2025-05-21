#include "igamesystem.h"

#include <iostream>
#include <vector>
#include "OverCharged/luamanager.h"

class MapAddLua;
class CMapScriptParser : public CAutoGameSystem
{
public:
	CMapScriptParser(char const *name);

	void LevelInitParsing(void);
	void LevelPostInitParsing(void);
	virtual void OnRestore(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual void PostParseScriptFile(const char* filename, bool official = false);
	virtual void ParseTriggerEvents(const char *parentLabelName, const char *childLabelName);
	virtual void ParseEntities(KeyValues *keyvalues);
	virtual void ParseSystemTriggers(KeyValues *pClassname);
	virtual void ParsePostInit(KeyValues *keyvalues);
	virtual void ParseRandomEntities(KeyValues *keyvalues);
	void		 ParseCSMInfo(KeyValues *keyvalues, const Vector &origin, const QAngle &originAngle);
	void		 MapaddChangeLevel(KeyValues *keyvalues);
	void ExecuteRandomEntites();
	void ModifyEnt(CBaseEntity *modEnt, KeyValues *pClassname, Vector &VelVector, variant_t &emptyVariant, char szSmodEntityName[128], bool &freeze, bool findedEnt);

	void SetRestored(bool restored) 
	{ 
		m_bRestored = restored; 

		//GetLuaManager()->InitDll();//Lua
		if (!m_bRestored)
		GetLuaManager()->InitDll();//Lua

		CMapAdd *pMapadd = GetMapAddEntity();
		pMapadd = CreateMapAddEntity();
	}

	KeyValues* m_pMapScript;
private:
	bool		m_bRestored;
	bool		m_hasLua;

	class MapAddLua *Lua;
};
extern CMapScriptParser *GetMapScriptParser();
