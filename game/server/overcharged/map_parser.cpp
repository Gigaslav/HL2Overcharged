//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "map_parser.h"
#include "filesystem.h"
#include "ai_baseactor.h"
#include "ai_network.h"
#include "ai_node.h"
#include "ai_hint.h"
#include "UtlSortVector.h"
#include "smod_nodelistparser.h"
#include "lights.h"
#include "lua_mapadd.h"
#include "mapadd.h"

#include "triggers.h"

#include <iostream>
#include <fstream>  
#define DEBUG_MSG FALSE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(124, 252, 0, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

ConVar disable_loadmapadd("disable_loadmapadd", "0");


class CSortedMapAddEntList
{
public:
	CSortedMapAddEntList() : m_sortedList(), m_emptyCount(0) {}

	typedef CBaseEntity *ENTITYPTR;
	class CEntityReportLess
	{
	public:
		bool Less(const ENTITYPTR &src1, const ENTITYPTR &src2, void *pCtx)
		{
			if (stricmp(src1->GetClassname(), src2->GetClassname()) < 0)
				return true;

			return false;
		}
	};

	void AddEntityToList(CBaseEntity *pEntity)
	{
		if (!pEntity)
		{
			m_emptyCount++;
		}
		else
		{
			m_sortedList.Insert(pEntity);
		}
	}
	void ParseEntites()
	{
		bool bSuccess = false;

		KeyValues *kvEnts = NULL;
		//KeyValues *kvTrigs = NULL;

		const char *pLastClass = "";
		int count = 0;
		int edicts = 0;
		for (int i = 0; i < m_sortedList.Count(); i++)
		{
			CBaseEntity *pEntity = m_sortedList[i];
			if (!pEntity)
				continue;
			else if (pEntity->GetOwnerEntity() && pEntity->GetOwnerEntity()->IsPlayer())
				continue;
			if (pEntity->edict())
				edicts++;

			const char *pClassname = pEntity->GetClassname();
			if (!FStrEq(pClassname, pLastClass))
			{
				if (count)
				{
					ConColorMsg(Color(0, 0, 255, 255), "Class: %s (%d)\n", pLastClass, count);
				}

				pLastClass = pClassname;
				count = 1;
			}
			else
				count++;


			if (kvEnts == NULL)
				kvEnts = new KeyValues("entites");

			KeyValues *pKVMap = new KeyValues(pClassname);

			char origin[_MAX_PATH];
			Q_snprintf(origin, sizeof(origin), "%.2f %.2f %.2f", pEntity->GetAbsOrigin().x, pEntity->GetAbsOrigin().y, pEntity->GetAbsOrigin().z);
			char angle[_MAX_PATH];
			Q_snprintf(angle, sizeof(angle), "%.2f %.2f %.2f", pEntity->GetAbsAngles().x, pEntity->GetAbsAngles().y, pEntity->GetAbsAngles().z);
			pKVMap->SetString("origin", origin);
			pKVMap->SetString("angle", angle);


			KeyValues *kvPToEnt = new KeyValues("keyvalues");

			char targetname[_MAX_PATH];
			Q_snprintf(targetname, sizeof(targetname), "%s", pEntity->GetEntityName());
			kvPToEnt->SetString("targetname", targetname);
			char model[_MAX_PATH];
			Q_snprintf(model, sizeof(model), "%s", pEntity->GetModelName());
			kvPToEnt->SetString("model", model);

			if (AllocPooledString(pClassname) == AllocPooledString("instant_trig"))
			{
				KeyValues *newKV = kvEnts->CreateNewKey();
				//newKV->SetName(var.String());
				//newKV->SetString("This block is relative to instant_trig below", "___");
				//newKV->SetString("Put below some logik you want", "also for correct mapadd parsing you need to move this block from the <entity> block to an empty space in this document");

				variant_t var;
				if (pEntity->ReadKeyField("label", &var))
				{
					//KeyValues *kvs = new KeyValues("keyvalues");
					kvPToEnt->SetString("label", var.String());
					//pKVMap->AddSubKey(kvs);

					//KeyValues *newKV = kvEnts->CreateNewKey();
					char name[_MAX_PATH];
					Q_snprintf(name, sizeof(name), "entities:%s", var.String());
				
					newKV->SetName(name);
					newKV->SetString("This block is relative to instant_trig below", "___");
					newKV->SetString("Put below some logik you want", "also for correct mapadd parsing you need to move this block from the <entity> block to an empty space in this document");
				}
				variant_t var2;
				if (pEntity->ReadKeyField("radius", &var2))
				{
					kvPToEnt->SetString("radius", var2.String());
				}
				variant_t var3;
				if (pEntity->ReadKeyField("timer", &var3))
				{
					kvPToEnt->SetString("timer", var3.String());
				}
			}

			pKVMap->AddSubKey(kvPToEnt);

			kvEnts->AddSubKey(pKVMap);
		}
		if (pLastClass[0] != 0 && count)
		{
			ConColorMsg(Color(255, 155, 155, 255), "Class: %s (%d)\n", pLastClass, count);
		}
		if (m_sortedList.Count())
		{
			ConColorMsg(Color(255, 0, 255, 255), "Total %d entities (%d empty, %d edicts)\n", m_sortedList.Count(), m_emptyCount, edicts);
		}

		char fullpath[_MAX_PATH];
		Q_snprintf(fullpath, sizeof(fullpath), "mapadd_generated/%s.txt", gpGlobals->mapname);

		char	GameInfoPath[MAX_PATH];
		g_pFullFileSystem->RelativePathToFullPath("gameinfo.txt", "MOD", GameInfoPath, sizeof(GameInfoPath));

		char localpath[_MAX_PATH];
		Q_FixSlashes(GameInfoPath, '/');
		Q_snprintf(localpath, sizeof(localpath), "%s/mapadd_generated/%s.txt", GameInfoPath, gpGlobals->mapname);
		ConColorMsg(Color(255, 255, 255, 255), "Generated in: %s \n", localpath);
		ConColorMsg(Color(0, 255, 0, 255), "Done! \n");
		//Create txt file
		std::ofstream outfile(localpath);
		//outfile << "my text here!" << std::endl;
		outfile.close();
		
		// save the data out to a file	
		bSuccess = kvEnts->SaveToFile(g_pFullFileSystem, fullpath, NULL);
		if (!bSuccess)
		{
			ConColorMsg(Color(0, 0, 255, 255), "BuildMode - Error saving file", "Error: Could not save changes.  File is most likely read only.");
		}

		kvEnts->deleteThis();





		/*KeyValues *kvTrigs = NULL;
		const char *pLastClass2 = "";
		int count2 = 0;
		int edicts2 = 0;
		for (int is = 0; is < m_sortedList.Count(); is++)
		{
			CBaseEntity *pEntity = m_sortedList[is];
			if (!pEntity)
				continue;

			if (pEntity->edict())
				edicts2++;

			const char *pClassname2 = pEntity->GetClassname();
			if (!FStrEq(pClassname2, pLastClass))
			{
				if (count2)
				{
					ConColorMsg(Color(0, 0, 255, 255), "Class: %s (%d)\n", pLastClass, count2);
				}

				pLastClass = pClassname2;
				count2 = 1;
			}
			else
				count2++;



			if (kvTrigs == NULL)
			{
				kvTrigs = new KeyValues("Here you got event in instant trigs");
			}


			if (AllocPooledString(pClassname2) == AllocPooledString("instant_trig"))
			{
				variant_t var;
				if (pEntity->ReadKeyField("label", &var))
				{
					if (kvTrigs->LoadFromFile(filesystem, fullpath))
					{

					}

					KeyValues *kvs = new KeyValues("keyvalues");
					kvs->SetString("label", var.String());
					pKVMap->AddSubKey(kvs);

					KeyValues *newKV = kvEnts->CreateNewKey();
					newKV->SetName(var.String());
					newKV->SetString("Put here some logik you want -->", "");

				}
			}



			kvEnts->AddSubKey(pKVMap);

		}
		bool success = kvTrigs->SaveToFile(g_pFullFileSystem, fullpath, NULL);
		if (!bSuccess)
		{
			ConColorMsg(Color(0, 0, 255, 255), "BuildMode - Error saving file", "Error: Could not save changes.  File is most likely read only.");
		}
		kvTrigs->deleteThis();*/

	}
private:
	CUtlSortVector< CBaseEntity *, CEntityReportLess > m_sortedList;
	int		m_emptyCount;
};
CON_COMMAND(mapadd_generate, "Command will generate all entite's info on this map into folder <<mapadd_generated/cur_map*>>")
{
	if (!UTIL_IsCommandIssuedByServerAdmin())
		return;
	CSortedMapAddEntList list;
	CBaseEntity *pEntity = gEntList.FirstEnt();
	while (pEntity)
	{
		list.AddEntityToList(pEntity);
		pEntity = gEntList.NextEnt(pEntity);
	}

	CBaseTrigger *pTrig = dynamic_cast<CBaseTrigger*>(gEntList.FirstEnt());
	while (pTrig)
	{
		list.AddEntityToList(pTrig);
		pTrig = dynamic_cast<CBaseTrigger*>(gEntList.NextEnt(pTrig));
	}

	list.ParseEntites();
}


class CMapAddTriggerOnce : public CBaseEntity
{
	DECLARE_CLASS(CMapAddTriggerOnce, CBaseEntity);

	DECLARE_DATADESC();
public:
	CMapAddTriggerOnce(void) {};

	CMapAddTriggerOnce *entCreate(const Vector &position);

	Vector _min = Vector(-10, -10, -10);
	Vector _max = Vector(10, 10, 10);

	string_t labelName;

	string_t RS_CH = MAKE_STRING("player");

	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold

	void SetBounds(CMapAddTriggerOnce *trigger);

	void Touch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(mapadd_trigger_once, CMapAddTriggerOnce);

BEGIN_DATADESC(CMapAddTriggerOnce)

DEFINE_KEYFIELD(labelName, FIELD_STRING, "label"),

DEFINE_KEYFIELD(RS_CH, FIELD_STRING, "Response"),

DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_ENTITYFUNC(Touch),

END_DATADESC()

CMapAddTriggerOnce *CMapAddTriggerOnce::entCreate(const Vector &position)
{
	CMapAddTriggerOnce *pEnt = CREATE_ENTITY(CMapAddTriggerOnce, "mapadd_trigger_once");
	pEnt->SetAbsOrigin(position);
	pEnt->SetCollisionGroup(COLLISION_GROUP_PLAYER);
	pEnt->SetSolid(SOLID_BBOX);
	pEnt->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	pEnt->SetMoveType(MOVETYPE_NONE);

	UTIL_SetSize(pEnt, pEnt->_min, pEnt->_max);

	return pEnt;
}

void CMapAddTriggerOnce::SetBounds(CMapAddTriggerOnce *trigger)
{
	UTIL_SetSize(trigger, _min, _max);
}

void CMapAddTriggerOnce::Touch(CBaseEntity *pOther)
{
	if (!pOther)
		return;

	bool hit = false;

	if (FStrEq(STRING(RS_CH), "player") && pOther->IsPlayer())
	{
		hit = true;

		m_OnTrigger.FireOutput(pOther, this);

		const char *mainLabelName = "entities";
		const char *childLabelName = STRING(labelName);

		CMapScriptParser *parser = GetMapScriptParser();
		if (parser)
		{
			parser->ParseTriggerEvents(mainLabelName, childLabelName);

			UTIL_Remove(this);
		}
	}
	if (FStrEq(STRING(RS_CH), "npc") && pOther->IsNPC())
	{
		hit = true;

		m_OnTrigger.FireOutput(pOther, this);

		const char *mainLabelName = "entities";
		const char *childLabelName = STRING(labelName);

		CMapScriptParser *parser = GetMapScriptParser();
		if (parser)
		{
			parser->ParseTriggerEvents(mainLabelName, childLabelName);

			UTIL_Remove(this);
		}
	}

	if (hit)
		UTIL_Remove(this);
}

class CMapAddTriggerMultiple : public CBaseEntity
{
	DECLARE_CLASS(CMapAddTriggerMultiple, CBaseEntity);

	DECLARE_DATADESC();
public:
	CMapAddTriggerMultiple();

	CMapAddTriggerMultiple *entCreate(const Vector &position);

	Vector _min = Vector(-10, -10, -10);
	Vector _max = Vector(10, 10, 10);

	string_t labelName;

	float delayThink = 5.f;

	float thinkLimit = 50.f;

	string_t RS_CH = MAKE_STRING("player");

	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold

	void SetBounds(CMapAddTriggerMultiple *trigger);

	void Touch(CBaseEntity *pOther);

private:
	float m_flNextThink;
};

LINK_ENTITY_TO_CLASS(mapadd_trigger_multiple, CMapAddTriggerMultiple);

BEGIN_DATADESC(CMapAddTriggerMultiple)

DEFINE_FIELD(m_flNextThink, FIELD_TIME),

DEFINE_KEYFIELD(labelName, FIELD_STRING, "label"),

DEFINE_KEYFIELD(RS_CH, FIELD_STRING, "Response"),

DEFINE_KEYFIELD(delayThink, FIELD_FLOAT, "delay"),

DEFINE_KEYFIELD(thinkLimit, FIELD_FLOAT, "limit"),

DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_ENTITYFUNC(Touch),

END_DATADESC()

CMapAddTriggerMultiple *CMapAddTriggerMultiple::entCreate(const Vector &position)
{
	CMapAddTriggerMultiple *pEnt = CREATE_ENTITY(CMapAddTriggerMultiple, "mapadd_trigger_multiple");
	pEnt->SetAbsOrigin(position);
	pEnt->SetCollisionGroup(COLLISION_GROUP_PLAYER);
	pEnt->SetSolid(SOLID_BBOX);
	pEnt->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	pEnt->SetMoveType(MOVETYPE_NONE);
	pEnt->m_flNextThink = 0.f;
	UTIL_SetSize(pEnt, pEnt->_min, pEnt->_max);

	return pEnt;
}

CMapAddTriggerMultiple::CMapAddTriggerMultiple()
{
	m_flNextThink = 0.f;
}

void CMapAddTriggerMultiple::SetBounds(CMapAddTriggerMultiple *trigger)
{
	UTIL_SetSize(trigger, _min, _max);
}

void CMapAddTriggerMultiple::Touch(CBaseEntity *pOther)
{
	if (!pOther)
		return;

	if (m_flNextThink < gpGlobals->curtime)
	{
		m_flNextThink = gpGlobals->curtime + delayThink;

		bool hit = false;

		if (FStrEq(STRING(RS_CH), "player") && pOther->IsPlayer())
		{
			hit = true;

			m_OnTrigger.FireOutput(pOther, this);

			const char *mainLabelName = "entities";
			const char *childLabelName = STRING(labelName);

			CMapScriptParser *parser = GetMapScriptParser();
			if (parser)
			{
				parser->ParseTriggerEvents(mainLabelName, childLabelName);

				//UTIL_Remove(this);
			}
		}
		if (FStrEq(STRING(RS_CH), "npc") && pOther->IsNPC())
		{
			hit = true;

			m_OnTrigger.FireOutput(pOther, this);

			const char *mainLabelName = "entities";
			const char *childLabelName = STRING(labelName);

			CMapScriptParser *parser = GetMapScriptParser();
			if (parser)
			{
				parser->ParseTriggerEvents(mainLabelName, childLabelName);

				//UTIL_Remove(this);
			}
		}
	}

	if (m_flNextThink >= thinkLimit)
		UTIL_Remove(this);
}





















class CMapAddNPCTrigger1 : public CMapAdd
{
public:
	DECLARE_CLASS(CMapAddNPCTrigger1, CMapAdd);
	DECLARE_DATADESC();


	// Constructor
	CMapAddNPCTrigger1(int MnEntIndex = 0);

	void Spawn()
	{
		BaseClass::Spawn();
		SetNextThink(gpGlobals->curtime); // Think now
	}
	//CMapAddITrigger *Pointer();
	void Think();
	void ActionAdd(KeyValues *pMapAddEnt);
	void ActionMod(KeyValues *pMapAddEnt);

private:
	CBaseEntity *mapaddEnt = NULL;
	string_t labelName;
	float	m_nTriggerArea;	// Count at which to fire our output
	float	m_nTimer;	// Count at which to fire our output
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold
protected:
	int m_nEntIndexM;
};
LINK_ENTITY_TO_CLASS(instant_trig_npc, CMapAddNPCTrigger1);

// Start of our data description for the class
BEGIN_DATADESC(CMapAddNPCTrigger1)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
DEFINE_KEYFIELD(m_nTimer, FIELD_FLOAT, "timer"),
DEFINE_KEYFIELD(labelName, FIELD_STRING, "label"),
// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC( FIELD_VOID, "RunLabel", InputRunLabel ),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_THINKFUNC(Think), // Register new think function

END_DATADESC()

CMapAddNPCTrigger1::CMapAddNPCTrigger1(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_nTimer = 0.f;
	m_bDeleteOnFire = true;
}
void CMapAddNPCTrigger1::Think()
{
	if (!cvar->FindVar("oc_mapadd_enabled")->GetBool())
		UTIL_Remove(this);

	SetNextThink(gpGlobals->curtime + 0.01f + m_nTimer); // Think again in 1 second
	BaseClass::Think();

	CBaseEntity *ppEnts[512];
	int nEntCount = UTIL_EntitiesInSphere(ppEnts, 512, this->GetAbsOrigin(), m_nTriggerArea, 0);

	char szSSName[50];
	Q_snprintf(szSSName, sizeof(szSSName), "Trigger_%d", entindex());
	string_t iszSSName = AllocPooledString(szSSName);
	this->SetName(iszSSName);



	int i;
	for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (ppEnts[i] != NULL)
			if (ppEnts[i]->IsNPC())
			{

				char labelname[FILENAME_MAX];
				Q_snprintf(labelname, sizeof(labelname), "entities:%s", labelName);
				KeyValues* m_pLabel = NULL;
				CMapScriptParser *parser = GetMapScriptParser();
				if (parser)
				{
					m_pLabel = parser->m_pMapScript->FindKey(labelname);
					if (m_pLabel)
						parser->ParseEntities(m_pLabel);
				}

				if (m_pLabel)
				{
					UTIL_Remove(this);
				}
			}
	}
}

class CMapAddITrigger1 : public CMapAdd
{
public:
	DECLARE_CLASS(CMapAddITrigger1, CMapAdd);
	DECLARE_DATADESC();


	// Constructor
	CMapAddITrigger1(int MnEntIndex = 0);

	void Spawn()
	{
		BaseClass::Spawn();
		SetNextThink(gpGlobals->curtime); // Think now
	}
	//CMapAddITrigger *Pointer();
	void Think();
	void ActionAdd(KeyValues *pMapAddEnt);
	void ActionMod(KeyValues *pMapAddEnt);

private:
	CBaseEntity *mapaddEnt = NULL;
	string_t labelName;
	float	m_nTriggerArea;	// Count at which to fire our output
	float	m_nTimer;	// Count at which to fire our output
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold
protected:
	int m_nEntIndexM;
};
LINK_ENTITY_TO_CLASS(instant_trig1111, CMapAddITrigger1);

// Start of our data description for the class
BEGIN_DATADESC(CMapAddITrigger1)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
DEFINE_KEYFIELD(m_nTimer, FIELD_FLOAT, "timer"),
DEFINE_KEYFIELD(labelName, FIELD_STRING, "label"),
// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC( FIELD_VOID, "RunLabel", InputRunLabel ),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_THINKFUNC(Think), // Register new think function

END_DATADESC()

CMapAddITrigger1::CMapAddITrigger1(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_nTimer = 0.f;
	m_bDeleteOnFire = true;
}
void CMapAddITrigger1::Think()
{
	if (!cvar->FindVar("oc_mapadd_enabled")->GetBool())
		UTIL_Remove(this);

	SetNextThink(gpGlobals->curtime + 0.01f + m_nTimer); // Think again in 1 second
	BaseClass::Think();

	CBaseEntity *ppEnts[512];
	int nEntCount = UTIL_EntitiesInSphere(ppEnts, 512, this->GetAbsOrigin(), m_nTriggerArea, 0);

	char szSSName[50];
	Q_snprintf(szSSName, sizeof(szSSName), "Trigger_%d", entindex());
	string_t iszSSName = AllocPooledString(szSSName);
	this->SetName(iszSSName);
	//DevMsg("m_nTimer %2f \n", m_nTimer);
	if (m_nTimer != 0)
	{
		//m_nTimer = m_nTimer - 0.01f;
		CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
		if (playerEnt)
			SetAbsOrigin(playerEnt->GetAbsOrigin());
	}

	//if (m_nTimer == 0)
	{
		int i;
		for (i = 0; i < nEntCount; i++)
		{
			//Look through the entities it found
			if (ppEnts[i] != NULL)
				if (ppEnts[i]->IsPlayer())
				{
					//m_OnTrigger.FireOutput(ppEnts[i], this);


					char labelname[FILENAME_MAX];
					Q_snprintf(labelname, sizeof(labelname), "entities:%s", labelName);
					KeyValues* m_pLabel = NULL;
					CMapScriptParser *parser = GetMapScriptParser();
					if (parser && parser->m_pMapScript)
					{

						m_pLabel = parser->m_pMapScript->FindKey(labelname);
						if (m_pLabel)
							parser->ParseEntities(m_pLabel);
						DevMsg("INSTANT_TRIG TRIGGERED WITH m_pLabel %s\n", m_pLabel);
					}
					DevMsg("INSTANT_TRIG TRIGGERED \n");
					DevMsg("INSTANT_TRIG TRIGGERED WITH labelName %s\n", labelName);

					if (m_pLabel)
					{
						UTIL_Remove(this);
					}

				}
		}
	}
	//SetNextThink(gpGlobals->curtime + 0.01f); // Think again in 1 second
}

//Im lazy to put this on another file
class CEvent : public CLogicalEntity
{
public:
	DECLARE_CLASS(CEvent, CLogicalEntity)
	void Spawn()
	{
		SetNextThink(gpGlobals->curtime + m_flDelay);
	}
	void Think()
	{
		CBaseEntity *pEntity = gEntList.FindEntityByName(NULL, m_szTarget);
		if (pEntity)
			UTIL_Remove(pEntity);
	}

	string_t m_szTarget;
	string_t m_szInput;
	float m_flDelay;
};

LINK_ENTITY_TO_CLASS(event, CEvent)


CMapScriptParser::CMapScriptParser(char const *name) : CAutoGameSystem(name)
{
	m_bRestored = false;
}

CMapScriptParser g_MapScriptParser("MapScriptParser");

CMapScriptParser *GetMapScriptParser()
{
	return &g_MapScriptParser;
}


void CMapScriptParser::LevelInitParsing()
{
	if (!cvar->FindVar("oc_mapadd_enabled")->GetBool())
		return;

	if (m_bRestored)
		return;


	/*GetLuaManager()->InitDll();

	if (!GetLuaHandle())
		Lua = new MapAddLua();*/


	char mapadd_preset[FILENAME_MAX];
	Q_snprintf(mapadd_preset, sizeof(mapadd_preset), (cvar->FindVar("oc_mapadd_preset")->GetString()));

	char filename[FILENAME_MAX];
	//if (GetNodeListParser())
	{
		Q_snprintf(filename, sizeof(filename), "%s/nodes/%s.snl", mapadd_preset, gpGlobals->mapname);
		GetNodeListParser()->ParseScriptFile(filename);
	}

	/*Q_snprintf(filename, sizeof(filename), "%s/official/%s.txt", mapadd_preset, gpGlobals->mapname);
	ParseScriptFile(filename, true);*/

	Q_snprintf(filename, sizeof(filename), "%s/%s.txt", mapadd_preset, gpGlobals->mapname);
	ParseScriptFile(filename);

	/*if (Lua)
	{
		delete Lua;
		Lua = NULL;
	}

	GetLuaManager()->ShutdownDll();*/
}

void CMapScriptParser::LevelPostInitParsing()
{
	if (!cvar->FindVar("oc_mapadd_enabled")->GetBool())
		return;
	char mapadd_preset[FILENAME_MAX];
	Q_snprintf(mapadd_preset, sizeof(mapadd_preset), (cvar->FindVar("oc_mapadd_preset")->GetString()));

	char filename[FILENAME_MAX];
	{
		Q_snprintf(filename, sizeof(filename), "%s/nodes/%s.snl", mapadd_preset, gpGlobals->mapname);
		GetNodeListParser()->ParseScriptFile(filename);
	}

	Q_snprintf(filename, sizeof(filename), "%s/%s.txt", mapadd_preset, gpGlobals->mapname);
	PostParseScriptFile(filename);
}

void CMapScriptParser::OnRestore()
{
	return;
}

void CMapScriptParser::ParseScriptFile(const char* filename, bool official)
{
	/*if (disable_loadmapadd.GetBool())
		return;*/

	if (!m_pMapScript)
		m_pMapScript = new KeyValues("MapScript");

	if (m_pMapScript->LoadFromFile(filesystem, filename))
	{
		//KeyValues *mainKey = m_pMapScript->GetNextKey();
		while (m_pMapScript)
		{
			if (FStrEq(m_pMapScript->GetName(), "precache"))
			{
				DebugColorMsg("Found precache Key \n");
				FOR_EACH_VALUE(m_pMapScript, pType)
				{
					if (!Q_strcmp(pType->GetName(), "entity"))
					{
						UTIL_PrecacheOther(pType->GetString());

						DebugColorMsg("Precaching Entity: ");
						DebugColorMsg(pType->GetString());
						DebugColorMsg("\n");
					}
					else if (!Q_strcmp(pType->GetName(), "model") || !Q_strcmp(pType->GetName(), "Model"))
					{
						CBaseEntity::PrecacheModel(pType->GetString());

						DebugColorMsg("Precaching Model: ");
						DebugColorMsg(pType->GetString());
						DebugColorMsg("\n");
					}
				}
			}
			
			if (FStrEq(m_pMapScript->GetName(), "entities"))
			{
				ParseEntities(m_pMapScript);
			}

			if (FStrEq(m_pMapScript->GetName(), "randomspawn"))
			{
				ParseRandomEntities(m_pMapScript);
			}

			m_pMapScript = m_pMapScript->GetNextKey();

			if (m_pMapScript == NULL)
				break;
		}
	}

	m_pMapScript->deleteThis();
	m_pMapScript = nullptr;
}

void CMapScriptParser::PostParseScriptFile(const char* filename, bool official)
{

	if (!m_pMapScript)
		m_pMapScript = new KeyValues("MapScript");

	if (m_pMapScript->LoadFromFile(filesystem, filename))
	{
		while (m_pMapScript)
		{
			if (FStrEq(m_pMapScript->GetName(), "Init"))
			{
				if (gpGlobals->eLoadType != MapLoad_NewGame)
					break;

				ParsePostInit(m_pMapScript);

				break;
			}

			m_pMapScript = m_pMapScript->GetNextKey();

			if (m_pMapScript == NULL)
				break;
		}
	}

	m_pMapScript->deleteThis();
	m_pMapScript = nullptr;
}

void CMapScriptParser::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

void CMapScriptParser::ExecuteRandomEntites()
{
	KeyValues *pRandom = m_pMapScript->FindKey("randomspawn");
	if (pRandom)
	{
		ParseRandomEntities(pRandom);
	}
}

ConVar	oc_mapadd_randomspawn_safe_distance("oc_mapadd_randomspawn_safe_distance", "50", FCVAR_ARCHIVE);
void CMapScriptParser::ParseRandomEntities(KeyValues *keyvalues)
{
	FOR_EACH_SUBKEY(keyvalues, pClassname)
	{
		/*if (!Q_strcmp(pClassname->GetName(), "removenodes"))
		{
			FOR_EACH_VALUE(pClassname, value)
			{
				Vector VecOrigin;

				float radius = 0.f;

				if (!Q_strcmp(pClassname->GetName(), "origin"))
				{
					UTIL_StringToVector(VecOrigin.Base(), value->GetString());
				}
				if (!Q_strcmp(pClassname->GetName(), "radius"))
				{
					radius = value->GetFloat("radius", 50.f);
				}
				for (int i = 0; i < g_pBigAINet->NumNodes(); i++)
				{
					Vector sum = VecOrigin + g_pBigAINet->GetNode(i)->GetOrigin();
					if (VectorLength(sum) >= radius)
					{
						g_pBigAINet->GetNode(i)->Unlock();
						g_pBigAINet->GetNode(i)->ClearLinks();
					}
				}
			}

			continue;
		}
		if (!Q_strcmp(pClassname->GetName(), "removeairnodes"))
		{
			// no implementation for it now.
			continue;
		}*/

		CBaseEntity *ppEnts[512];
		
		int iCount = pClassname->GetInt("count", 0);
		for (int currentNumber = 0; currentNumber < iCount; currentNumber++)
		{
			if (!g_pBigAINet->GetNode(0))
				return;
			//CBaseEntity *pEntity = CreateEntityByName(pClassname->GetName());

			int index = RandomInt(0, g_pBigAINet->NumNodes() - 1);

			CAI_Hint *pHint = g_pBigAINet->GetNode(index)->GetHint();

			int max_iterations = g_pBigAINet->NumNodes();

			if (pHint)
			{
				/*if*/while (max_iterations < g_pBigAINet->NumNodes() && (pHint->HintType() == HINT_ANTLION_BURROW_POINT ||
					pHint->HintType() == HINT_ANTLION_THUMPER_FLEE_POINT ||
					pHint->HintType() == HINT_HEADCRAB_BURROW_POINT ||
					pHint->HintType() == HINT_HEADCRAB_EXIT_POD_POINT ||
					pHint->HintType() == HINT_NOT_USED_PSTORM_ROCK_SPAWN ||
					pHint->HintType() == HINT_NOT_USED_ROLLER_PATROL_POINT ||
					pHint->HintType() == HINT_NOT_USED_ROLLER_CLEANUP_POINT ||
					pHint->HintType() == HINT_WORLD_VISUALLY_INTERESTING ||
					pHint->HintType() == HINT_WORLD_VISUALLY_INTERESTING_DONT_AIM))
				{
					//continue;
					int newIndx = index++;

					if (newIndx >= g_pBigAINet->NumNodes() - 1)
						newIndx = 0;

					pHint = g_pBigAINet->GetNode(newIndx)->GetHint();

					max_iterations++;
				}
			}

			CBaseEntity *pEntity = NULL;

			Vector pos = g_pBigAINet->GetNode(index)->GetPosition(HULL_HUMAN);
			//if (gEntList.FindEntityGenericNearest(pClassname->GetName(), pEntity->GetAbsOrigin(), oc_mapadd_randomspawn_safe_distance.GetFloat()) != NULL)
			if (UTIL_EntitiesInSphere(ppEnts, 512, pos, oc_mapadd_randomspawn_safe_distance.GetFloat(), 0) > 0)
			{			
				//UTIL_Remove(pEntity);
				continue;
			}
			else
				pEntity = CreateEntityByName(pClassname->GetName());

			if (pEntity)
			{
				//Vector pos = g_pBigAINet->GetNode(RandomInt(1, g_pBigAINet->NumNodes() - 1))->GetPosition(HULL_HUMAN);

				const char *sWeapon = pClassname->GetString("weapon", NULL);
				if (sWeapon)
					pEntity->KeyValue("additionalequipment", sWeapon);
				const char *sModel = pClassname->GetString("model", NULL);
				if (sModel)
					pEntity->KeyValue("model", sModel);

				char key[512];
				char value[512];
				char tokenname[512];

				const char *pValue = pClassname->GetString("values", NULL);
				if (pValue)
				{
					pValue = nexttoken(tokenname, pValue, ' ');
					while (pValue && Q_strlen(tokenname) > 0)
					{
						Q_strcpy(key, tokenname);
						pValue = nexttoken(tokenname, pValue, ' ');
						Q_strcpy(value, tokenname);
						pEntity->KeyValue(key, value);
						pValue = nexttoken(tokenname, pValue, ' ');
					}
				}

				pEntity->SetAbsOrigin(pos);
				pEntity->Precache();
				DispatchSpawn(pEntity);
				pEntity->Activate();
			}
		}

	}
}

void StringToColorArray(CUtlVector<int> *pVector, const char *pString, const char *separator, int size)
{
	CUtlVector<char*, CUtlMemory<char*> > arr;

	Q_SplitString(pString, separator, arr);

	pVector->RemoveAll();

	for (int i = 0; i < arr.Count(); i++)
	{
		int k = atoi(arr.Element(i));
		pVector->AddToTail(k);
	}

	//Size check
	for (int i = 0; i < 4; i++)
	{
		if (!pVector->Element(i))
			pVector->AddToTail(0);
	}
}

#include "OverCharged/RTT/env_global_light.h"
#include "icommandline.h"

void CMapScriptParser::ParseCSMInfo(KeyValues *keyvalues, const Vector &origin, const QAngle &originAngle)
{
	/*if (CommandLine()->CheckParm("-oc_enable_dynamic_shadows") == NULL)
		return;*/
	if (!cvar->FindVar("oc_global_lightning_enabled")->GetBool())
		return;

	KeyValues *pKeyValuesOrig = keyvalues->FindKey("keyvalues");
	if (pKeyValuesOrig)
	{
		KeyValues *pKeyValues = pKeyValuesOrig->FindKey("global_lightning_keyvalues");
		if (pKeyValues)
		{
			CBaseEntity *pEntity = CBaseEntity::CreateNoSpawn("global_lightning", origin, originAngle);
			if (pEntity)
			{
				CGlobalLight *glEnt = static_cast<CGlobalLight*>(pEntity);
				//g_GlobalLightning = glEnt;

				if (glEnt)
				{
					gEntList.NotifyCreateEntity(glEnt);

					FOR_EACH_VALUE(pKeyValues, pValue)
					{
						glEnt->KeyValue(pValue->GetName(), pValue->GetString());
					}

					glEnt->Precache();

					DispatchSpawn(glEnt);

					glEnt->Activate();

					glEnt->EnableFromMapAdd();
				}
			}
		}
	}
}

void CMapScriptParser::MapaddChangeLevel(KeyValues *keyvalues)
{
	if (FStrEq(keyvalues->GetName(), "mapadd_trigger_once") || FStrEq(keyvalues->GetName(), "instant_trig"))
	{
		CMapAddTriggerOnce *pTriggerOnce = nullptr;

		Vector origin = Vector(0, 0, 0);
		Vector bounds = Vector(10, 10, 10);
		float radius = 10.f;
		bool setSize = false;

		FOR_EACH_VALUE(keyvalues, value)
		{
			if (FStrEq(value->GetName(), "origin"))
			{
				UTIL_StringToVector(origin.Base(), value->GetString());
			}
			if (FStrEq(value->GetName(), "bounds"))
			{
				UTIL_StringToVector(bounds.Base(), value->GetString());
				setSize = true;
			}
			if (FStrEq(value->GetName(), "radius"))
			{
				radius = value->GetFloat();
				bounds.x = bounds.y = bounds.z = radius;
				setSize = true;
			}
		}

		pTriggerOnce = pTriggerOnce->entCreate(origin);

		if (setSize)
		{
			pTriggerOnce->_max = bounds;
			pTriggerOnce->_min = bounds * -1.f;

			pTriggerOnce->SetBounds(pTriggerOnce);
		}

		pTriggerOnce->Precache();

		gEntList.NotifyCreateEntity(pTriggerOnce);

		KeyValues *pKeyValues = keyvalues->FindKey("keyvalues");
		if (pKeyValues)
		{
			FOR_EACH_VALUE(pKeyValues, pValues)
			{
				pTriggerOnce->KeyValue(pValues->GetName(), pValues->GetString());

				Vector newBounds;
				float newRadius;
				bool newSize = false;
				if (FStrEq(pValues->GetName(), "bounds"))
				{
					UTIL_StringToVector(newBounds.Base(), pValues->GetString());
					newSize = true;
				}
				if (FStrEq(pValues->GetName(), "radius"))
				{
					newRadius = pValues->GetFloat();
					bounds.x = bounds.y = bounds.z = newRadius;
					newSize = true;
				}
				if (FStrEq(pValues->GetName(), "response"))
				{
					const char* responseChannel = pValues->GetString();

					if (FStrEq(responseChannel, "player"))
					{
						pTriggerOnce->SetCollisionGroup(COLLISION_GROUP_PLAYER);
					}
					if (FStrEq(responseChannel, "npc"))
					{
						pTriggerOnce->SetCollisionGroup(COLLISION_GROUP_NPC);
					}
				}
				if (newSize)
				{
					pTriggerOnce->_max = bounds;
					pTriggerOnce->_min = bounds * -1.f;

					pTriggerOnce->SetBounds(pTriggerOnce);
				}
			}
		}

		DispatchSpawn(pTriggerOnce);

		pTriggerOnce->Activate();
	}
	if (FStrEq(keyvalues->GetName(), "mapadd_trigger_multiple"))
	{
		CMapAddTriggerMultiple *pTriggerMult = nullptr;

		Vector origin = Vector(0, 0, 0);
		Vector bounds = Vector(10, 10, 10);
		float radius = 10.f;
		bool setSize = false;
		float delay = 5.f;
		float limit = 50.f;

		FOR_EACH_VALUE(keyvalues, value)
		{
			if (FStrEq(value->GetName(), "origin"))
			{
				UTIL_StringToVector(origin.Base(), value->GetString());
			}
			if (FStrEq(value->GetName(), "bounds"))
			{
				UTIL_StringToVector(bounds.Base(), value->GetString());
				setSize = true;
			}
			if (FStrEq(value->GetName(), "radius"))
			{
				radius = value->GetFloat();
				bounds.x = bounds.y = bounds.z = radius;
				setSize = true;
			}
			if (FStrEq(value->GetName(), "delay"))
			{
				delay = value->GetFloat();
			}
			if (FStrEq(value->GetName(), "limit"))
			{
				limit = value->GetFloat();
			}
		}

		pTriggerMult = pTriggerMult->entCreate(origin);

		if (setSize)
		{
			pTriggerMult->_max = bounds;
			pTriggerMult->_min = bounds * -1.f;

			pTriggerMult->SetBounds(pTriggerMult);
		}

		pTriggerMult->Precache();

		gEntList.NotifyCreateEntity(pTriggerMult);

		KeyValues *pKeyValues = keyvalues->FindKey("keyvalues");
		if (pKeyValues)
		{
			FOR_EACH_VALUE(pKeyValues, pValues)
			{
				pTriggerMult->KeyValue(pValues->GetName(), pValues->GetString());

				Vector newBounds;
				float newRadius;
				bool newSize = false;
				if (FStrEq(pValues->GetName(), "bounds"))
				{
					UTIL_StringToVector(newBounds.Base(), pValues->GetString());
					newSize = true;
				}
				if (FStrEq(pValues->GetName(), "radius"))
				{
					newRadius = pValues->GetFloat();
					bounds.x = bounds.y = bounds.z = newRadius;
					newSize = true;
				}
				if (FStrEq(pValues->GetName(), "response"))
				{
					const char* responseChannel = pValues->GetString();

					if (FStrEq(responseChannel, "player"))
					{
						pTriggerMult->SetCollisionGroup(COLLISION_GROUP_PLAYER);
					}
					if (FStrEq(responseChannel, "npc"))
					{
						pTriggerMult->SetCollisionGroup(COLLISION_GROUP_NPC);
					}
				}
				if (newSize)
				{
					pTriggerMult->_max = bounds;
					pTriggerMult->_min = bounds * -1.f;

					pTriggerMult->SetBounds(pTriggerMult);
				}
			}
		}

		DispatchSpawn(pTriggerMult);

		pTriggerMult->Activate();
	}
}

void CMapScriptParser::ParseTriggerEvents(const char *parentLabelName, const char *childLabelName)
{
	char mapadd_preset[FILENAME_MAX];
	Q_snprintf(mapadd_preset, sizeof(mapadd_preset), (cvar->FindVar("oc_mapadd_preset")->GetString()));

	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "%s/%s.txt", mapadd_preset, gpGlobals->mapname);

	if (!m_pMapScript)
		m_pMapScript = new KeyValues("MapScript");

	if (m_pMapScript->LoadFromFile(filesystem, filename))
	{
		char fullEventName[FILENAME_MAX];
		Q_snprintf(fullEventName, sizeof(fullEventName), "%s:%s", parentLabelName, childLabelName);

		while (m_pMapScript)
		{
			if (FStrEq(m_pMapScript->GetName(), fullEventName))
			{
				ParseEntities(m_pMapScript);
			}

			m_pMapScript = m_pMapScript->GetNextKey();

			if (m_pMapScript == NULL)
				break;
		}
	}

	m_pMapScript->deleteThis();
	m_pMapScript = nullptr;
}

void CMapScriptParser::ParsePostInit(KeyValues *keyvalues)
{
	FOR_EACH_SUBKEY(keyvalues, pClassname)
	{
		if (FStrEq(pClassname->GetName(), "Give_Equipment"))
		{
			CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
			if (pPlayer)
			{
				FOR_EACH_SUBKEY(pClassname, pSubKey)
				{
					if (FStrEq(pSubKey->GetName(), "Ammo"))
					{
						FOR_EACH_VALUE(pSubKey, pAmmo)
						{
							//engine->ServerCommand("give weapon_ar2\n");
							pPlayer->GiveAmmo(pAmmo->GetInt(), pAmmo->GetName());
						}
					}

					if (FStrEq(pSubKey->GetName(), "Item"))
					{
						FOR_EACH_VALUE(pSubKey, pItem)
						{
							if (FStrEq(pItem->GetString(), "item_suit"))
							{
								pPlayer->EquipSuit(true);
							}
							else
							{
								char iszItem[256];
								Q_snprintf(iszItem, ARRAYSIZE(iszItem), "%s_%s", pItem->GetName(), pItem->GetString());

								//string_t iszItem = AllocPooledString(pItem->GetString("") + pItem->GetString(""));	// Make a copy of the classname
								pPlayer->GiveNamedItem(iszItem);
							}
						}
					}
				}
			}

			break;
		}
	}
}

void CMapScriptParser::ModifyEnt(CBaseEntity *modEnt, KeyValues *pClassname, Vector &VelVector, variant_t &emptyVariant, char szSmodEntityName[128], bool &freeze, bool findedEnt)
{
	FOR_EACH_VALUE(pClassname, value)
	{
		if (FStrEq(value->GetName(), "model") && findedEnt)
		{
			modEnt->PrecacheModel(value->GetString(""));
			modEnt->SetModel(value->GetString(""));
		}
		else if (FStrEq(value->GetName(), "models") && findedEnt)
		{
			CUtlVector<char*, CUtlMemory<char*> > arr;
			Q_SplitString(value->GetString(), "||", arr);

			int i = random->RandomInt(0, arr.Count()-1);

			modEnt->PrecacheModel(arr.Element(i));
			modEnt->SetModel(arr.Element(i));
		}

		if (FStrEq(value->GetName(), "action") && findedEnt)
		{
			Q_snprintf(szSmodEntityName, sizeof(szSmodEntityName), "%s", AllocPooledString(value->GetString("")));
			CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
			if (playerEnt)
			{
				modEnt->AcceptInput(value->GetString(""), playerEnt, modEnt, emptyVariant, 0);
			}
			else
				modEnt->AcceptInput(value->GetString(""), NULL, modEnt, emptyVariant, 0);
		}

		if (FStrEq(value->GetName(), "value") && findedEnt)
		{
			if (value->GetString("value", ""))
			{
				emptyVariant.SetString(AllocPooledString(value->GetString("")));
			}
			else if (value->GetFloat("value"))
			{
				emptyVariant.SetFloat(value->GetFloat());
			}
			CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
			if (playerEnt)
			{
				modEnt->AcceptInput(szSmodEntityName, playerEnt, modEnt, emptyVariant, 0);
			}
			else
				modEnt->AcceptInput(szSmodEntityName, NULL, modEnt, emptyVariant, 0);
		}

		if (FStrEq(value->GetName(), "delaytime") && findedEnt)
		{
			emptyVariant.SetFloat(value->GetFloat());
			CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
			if (playerEnt)
			{
				modEnt->AcceptInput(szSmodEntityName, playerEnt, modEnt, emptyVariant, 0);
			}
			else
				modEnt->AcceptInput(szSmodEntityName, NULL, modEnt, emptyVariant, 0);
		}

		if (FStrEq(value->GetName(), "freeze") && findedEnt)
		{
			modEnt->SetMoveType(MOVETYPE_NONE);
			modEnt->SetSolid(SOLID_VPHYSICS);
			modEnt->VPhysicsInitStatic();
			freeze = false;
		}

		if (FStrEq(value->GetName(), "velocity") && findedEnt)
		{
			VelVector = Vector(0, 0, 0);
			UTIL_StringToVector(VelVector.Base(), value->GetString());
			modEnt->ApplyLocalVelocityImpulse(VelVector);
			//vel = false;
		}

		if (FStrEq(value->GetName(), "longrange") && findedEnt)
		{
			modEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);
		}
		else
		{
			if (modEnt)
				modEnt->KeyValue(value->GetName(), value->GetString(""));
		}
	}

	KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
	if (pKeyValues)
	{
		FOR_EACH_VALUE(pKeyValues, pValues)
		{
			//if (!Q_strcmp(pValues->GetName(), "model") || !Q_strcmp(pValues->GetName(), "Model"))
			if (FStrEq(pValues->GetName(), "model") || FStrEq(pValues->GetName(), "Model"))
			{
				CBaseEntity::PrecacheModel(pValues->GetString());
				modEnt->SetModel(pValues->GetString());
				continue;
			}

			modEnt->KeyValue(pValues->GetName(), pValues->GetString());
		}
	}
}

void CMapScriptParser::ParseSystemTriggers(KeyValues *pClassname)
{
	if (FStrEq(pClassname->GetName(), "trigger_changelevel"))
	{
		CBaseTrigger *pTrigger = dynamic_cast<CBaseTrigger*>(CreateEntityByName(pClassname->GetName()));
		gEntList.NotifyCreateEntity(pTrigger);

		Vector extendMaxSize = Vector(100, 100, 100);
		Vector extendMinSize = Vector(-100, -100, -100);

		if (pTrigger)
		{
			Vector vecOriginTrig;

			pTrigger->SetAbsOrigin(Vector(0, 0, 0));

			FOR_EACH_VALUE(pClassname, value)
			{
				//if (!Q_strcmp(value->GetName(), "origin"))
				if (FStrEq(value->GetName(), "origin"))
				{
					UTIL_StringToVector(vecOriginTrig.Base(), value->GetString());

					pTrigger->SetAbsOrigin(vecOriginTrig);
				}

				if (FStrEq(value->GetName(), "bounds"))
				{
					UTIL_StringToVector(extendMaxSize.Base(), value->GetString());
					UTIL_StringToVector(extendMinSize.Base(), value->GetString());
				}
			}


			//ent_bbox trigger_changelevel
			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);
			pTrigger->SetCollisionGroup(COLLISION_GROUP_PLAYER);
			pTrigger->SetSolid(SOLID_NONE);
			pTrigger->AddSolidFlags(FSOLID_TRIGGER);
			pTrigger->CollisionProp()->UseTriggerBounds(true, extendMaxSize.x);

			KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
			if (pKeyValues)
			{
				FOR_EACH_VALUE(pKeyValues, pValues)
				{
					pTrigger->KeyValue(pValues->GetName(), pValues->GetString());
				}
			}
			pTrigger->Precache();
			DispatchSpawn(pTrigger);

			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);
			/*pTrigger->SetCollisionGroup(COLLISION_GROUP_PLAYER);*/

			pTrigger->Activate();
			pTrigger->InitTrigger();
		}
	}
	else if (FStrEq(pClassname->GetName(), "trigger_transition"))
	{
		CTriggerVolume *pTrigger = dynamic_cast<CTriggerVolume*>(CreateEntityByName(pClassname->GetName()));
		gEntList.NotifyCreateEntity(pTrigger);

		Vector extendMaxSize = Vector(100, 100, 100);
		Vector extendMinSize = Vector(-100, -100, -100);

		if (pTrigger)
		{
			Vector vecOriginTrig;

			pTrigger->SetAbsOrigin(Vector(0, 0, 0));

			FOR_EACH_VALUE(pClassname, value)
			{
				//if (!Q_strcmp(value->GetName(), "origin"))
				if (FStrEq(value->GetName(), "origin"))
				{
					UTIL_StringToVector(vecOriginTrig.Base(), value->GetString());

					pTrigger->SetAbsOrigin(vecOriginTrig);
				}

				if (FStrEq(value->GetName(), "bounds"))
				{
					UTIL_StringToVector(extendMaxSize.Base(), value->GetString());
					UTIL_StringToVector(extendMinSize.Base(), value->GetString());
				}
			}


			//ent_bbox trigger_changelevel
			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);
			pTrigger->SetCollisionGroup(COLLISION_GROUP_PLAYER);
			pTrigger->SetSolid(SOLID_NONE);
			pTrigger->AddSolidFlags(FSOLID_TRIGGER);
			pTrigger->CollisionProp()->UseTriggerBounds(true, extendMaxSize.x);

			KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
			if (pKeyValues)
			{
				FOR_EACH_VALUE(pKeyValues, pValues)
				{
					pTrigger->KeyValue(pValues->GetName(), pValues->GetString());
				}
			}
			pTrigger->Precache();
			DispatchSpawn(pTrigger);

			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);
			/*pTrigger->SetCollisionGroup(COLLISION_GROUP_PLAYER);*/

			pTrigger->Activate();
			//pTrigger->InitTrigger();
		}
	}
	else
	{
		CBaseTrigger *pTrigger = dynamic_cast<CBaseTrigger*>(CreateEntityByName(pClassname->GetName()));
		gEntList.NotifyCreateEntity(pTrigger);

		Vector extendMaxSize = Vector(100, 100, 100);
		Vector extendMinSize = Vector(-100, -100, -100);

		Collision_Group_t group = COLLISION_GROUP_PLAYER;

		/*char iszItem[64];

		bool hasName = false;*/

		if (pTrigger)
		{
			Vector vecOriginTrig;

			pTrigger->SetAbsOrigin(Vector(0, 0, 0));

			FOR_EACH_VALUE(pClassname, value)
			{
				/*if (FStrEq(value->GetName(), "trigger_name"))
				{
					Q_snprintf(iszItem, ARRAYSIZE(iszItem), "%s", value->GetString());
					hasName = true;
				}*/

				if (FStrEq(value->GetName(), "origin"))
				{
					UTIL_StringToVector(vecOriginTrig.Base(), value->GetString());

					pTrigger->SetAbsOrigin(vecOriginTrig);
				}

				if (FStrEq(value->GetName(), "bounds"))
				{
					UTIL_StringToVector(extendMaxSize.Base(), value->GetString());
					UTIL_StringToVector(extendMinSize.Base(), value->GetString());
				}

				if (FStrEq(value->GetName(), "Response"))
				{
					group = (Collision_Group_t)value->GetInt();
				}
			}


			/*if (!hasName)
			{
				Q_snprintf(iszItem, ARRAYSIZE(iszItem), "%s", "NULL");
			}*/

			//ent_bbox trigger_changelevel
			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);
			pTrigger->SetCollisionGroup(group);
			pTrigger->SetSolid(SOLID_NONE);
			pTrigger->AddSolidFlags(FSOLID_TRIGGER);
			pTrigger->CollisionProp()->UseTriggerBounds(true, extendMaxSize.x);

			KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
			if (pKeyValues)
			{
				FOR_EACH_VALUE(pKeyValues, pValues)
				{
					pTrigger->KeyValue(pValues->GetName(), pValues->GetString());
				}
			}
			pTrigger->Precache();
			DispatchSpawn(pTrigger);

			pTrigger->SetSize(extendMinSize, extendMaxSize);
			UTIL_SetSize(pTrigger, extendMinSize, extendMaxSize);

			pTrigger->Activate();
			pTrigger->InitTrigger();
		}
	}
}

void CMapScriptParser::ParseEntities(KeyValues *keyvalues)
{
	bool freeze = false;
	//bool vel = false;
	//bool isOldCoord = false;
	Vector VelVector;
	Vector VecOrigin;
	Vector VecAngle;
	Vector VecOldOrigin = Vector(0, 0, 0);
	QAngle AngOldOrigin = QAngle(0, 0, 0);
	variant_t emptyVariant;
	char szSmodEntityName[128];
	FOR_EACH_SUBKEY(keyvalues, pClassname)
	{
		//if (cvar->FindVar("oc_mapadd_lua_enabled")->GetBool())
		{
			if (FStrEq(pClassname->GetName(), "lua") && GetLuaHandle())
			{
				if (GetLuaHandle()->OpenFile(gpGlobals->mapname.ToCStr()))
				{
					m_hasLua = true;
					GetLuaHandle()->CallFunction(pClassname->GetString("callfunc", ""));
				}
				continue;
			}
			else
			{
				m_hasLua = false;
			}
		}
		/*else
		{
			m_hasLua = false;
		}*/

		if (FStrEq(pClassname->GetName(), "SystemTriggers"))
		{
			FOR_EACH_SUBKEY(pClassname, pSubKey)
			{
				ParseSystemTriggers(pSubKey);
			}

			continue;
		}
	
		if (Q_strstr(pClassname->GetName(), "trigger") || Q_strstr(pClassname->GetName(), "instant_trig"))
		{
			if (cvar->FindVar("oc_mapadd_enabled")->GetBool())
				MapaddChangeLevel(pClassname);
			continue;
		}

		/*if (FStrEq(pClassname->GetName(), "env_sun"))
		{
			if (cvar->FindVar("oc_mapadd_enabled")->GetBool())
				ParseCSMInfo(pClassname);
			continue;
		}*/

		//if (!Q_strcmp(pClassname->GetName(), "player"))
		if (FStrEq(pClassname->GetName(), "player"))
		{
			FOR_EACH_VALUE(pClassname, value)
			{

				//if (!Q_strcmp(value->GetName(), "origin"))
				if (FStrEq(value->GetName(), "origin"))
				{
					Vector VecOrigin;
					UTIL_StringToVector(VecOrigin.Base(), value->GetString());

					UTIL_GetLocalPlayer()->SetAbsOrigin(VecOrigin);
				}
				//else if (!Q_strcmp(value->GetName(), "angle"))
				else if (FStrEq(value->GetName(), "angle"))
				{
					Vector VecAngle;
					UTIL_StringToVector(VecAngle.Base(), value->GetString());

					UTIL_GetLocalPlayer()->SetLocalAngles(QAngle(VecAngle.x, VecAngle.y, VecAngle.z));
				}
				//else if (!Q_strcmp(value->GetName(), "message"))
				else if (FStrEq(value->GetName(), "message"))
				{
					CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
					if (playerEnt)
					{
						engine->Con_NPrintf(9, value->GetString());
						/*UTIL_ShowMessage(value->GetString(), playerEnt);
						UTIL_SayText(value->GetString(), playerEnt);*/
						/*hudtextparms_s htp;
						htp.r2 = 255;
						htp.g2 = 255;
						htp.b2 = 255;
						htp.a2 = 255;
						htp.channel = 1;
						htp.fxTime = 10.f;
						htp.holdTime = 10.f;*/
						//htp.x = 50;
						//htp.y = 50;
						//UTIL_HudMessage(playerEnt, htp, value->GetString());
						//UTIL_HudHintText(playerEnt, value->GetString());
					}
				}
				//else if (!Q_strcmp(value->GetName(), "fadein"))
				else if (FStrEq(value->GetName(), "fadein"))
				{
					color32 black = { 0, 0, 0, 255 };
					UTIL_ScreenFade(UTIL_GetLocalPlayer(), black, 0, value->GetInt(), FFADE_IN);
				}
				//else if (!Q_strcmp(value->GetName(), "fadeout"))
				else if (FStrEq(value->GetName(), "fadeout"))
				{
					color32 black = { 32, 63, 100, 200 };
					UTIL_ScreenFade(UTIL_GetLocalPlayer(), black, value->GetInt(), 0.5, FFADE_OUT);
				}
				//else if (!Q_strcmp(value->GetName(), "music"))
				else if (FStrEq(value->GetName(), "music"))
				{
					CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
					if (playerEnt)
					{
						playerEnt->PrecacheScriptSound(value->GetString());
						playerEnt->EmitAmbientSound(playerEnt->entindex(), playerEnt->GetAbsOrigin(), value->GetString());
					}
				}
			}
			continue;
		}
		else if (!Q_strcmp(pClassname->GetName(), "removeentity"))
		{
			FOR_EACH_VALUE(pClassname, value)
			{
				CBaseEntity *pEntity = NULL;

				//if (!Q_strcmp(value->GetName(), "classname"))
				if (FStrEq(value->GetName(), "classname"))
				{
					if (UTIL_GetLocalPlayer())
						pEntity = gEntList.FindEntityByClassname(UTIL_GetLocalPlayer(), value->GetString());
					if (!pEntity)
						pEntity = gEntList.FindEntityByClassname(NULL, value->GetString());
				}

				bool searchBySphere = false;

				float sRadius = 100.f;

				Vector sphereLoc;

				//if (!Q_strcmp(value->GetName(), "targetname"))
				if (FStrEq(value->GetName(), "targetname"))
				{
					if (UTIL_GetLocalPlayer())
						pEntity = gEntList.FindEntityByName(UTIL_GetLocalPlayer(), value->GetString());
					if (!pEntity)
						pEntity = gEntList.FindEntityByName(NULL, value->GetString());
				}
				else if (FStrEq(value->GetName(), "sphere_location"))
				{
					searchBySphere = true;

					
					UTIL_StringToVector(sphereLoc.Base(), value->GetString());
				}
				else if (FStrEq(value->GetName(), "sphere_radius"))
				{
					if (searchBySphere)
					{
						sRadius = value->GetFloat();

						pEntity = gEntList.FindEntityInSphere(NULL, sphereLoc, sRadius);
					}
				}
				//else if (!Q_strcmp(value->GetName(), "model") || !Q_strcmp(value->GetName(), "Model"))
				else if (FStrEq(value->GetName(), "model") || FStrEq(value->GetName(), "Model"))
				{
					if (UTIL_GetLocalPlayer())
						pEntity = gEntList.FindEntityByModel(UTIL_GetLocalPlayer(), value->GetString());
					if (!pEntity)
						pEntity = gEntList.FindEntityByModel(NULL, value->GetString());

				}
				if (pEntity)
					UTIL_Remove(pEntity);
			}
		}
		else if (!Q_strcmp(pClassname->GetName(), "event"))
		{
				CBaseEntity *modEnt = NULL;

				bool findedEnt = false;
				
				FOR_EACH_VALUE(pClassname, value)
				{
					if (!modEnt)
					{
						if (FStrEq(value->GetName(), "targetname"))
						{
							modEnt = gEntList.FindEntityByName(NULL, value->GetString(""));
							if (modEnt)
								findedEnt = true;
						}
						else if (FStrEq(value->GetName(), "classname"))
						{						
							while ((modEnt = gEntList.FindEntityByClassname(modEnt, value->GetString(""))) != NULL)
							{
								if (modEnt)
								{
									findedEnt = true;

									ModifyEnt(modEnt, pClassname, VelVector, emptyVariant, szSmodEntityName, freeze, findedEnt);
								}
							}
							break;
						}
						else
						{
							if (!findedEnt)
							{
								float sRadius = 100.f;

								if (FStrEq(value->GetName(), "sphere_radius"))
								{
									sRadius = value->GetFloat();
								}
								if (FStrEq(value->GetName(), "sphere_location"))
								{
									Vector VecSphere;
									UTIL_StringToVector(VecSphere.Base(), value->GetString());

									modEnt = gEntList.FindEntityInSphere(NULL, VecSphere, sRadius);
									if (modEnt)
										findedEnt = true;
								}
							}
						}
					}

					if (findedEnt && modEnt)
					{
						ModifyEnt(modEnt, pClassname, VelVector, emptyVariant, szSmodEntityName, freeze, findedEnt);
						break;
					}

				}
		}


		if (AllocPooledString(pClassname->GetName()) != AllocPooledString("event")
			&& AllocPooledString(pClassname->GetName()) != AllocPooledString("removeentity")
			&& AllocPooledString(pClassname->GetName()) != AllocPooledString("env_sun")
			&& AllocPooledString(pClassname->GetName()) != AllocPooledString("lua")
			&& AllocPooledString(pClassname->GetName()) != AllocPooledString("player"))
		{
			CBaseEntity *pEntity = CreateEntityByName(pClassname->GetName());

			gEntList.NotifyCreateEntity(pEntity);

			if (pEntity)
			{
				pEntity->SetAbsOrigin(Vector(0,0,0));

				FOR_EACH_VALUE(pClassname, value)
				{
					//if (!Q_strcmp(value->GetName(), "origin"))
					if (FStrEq(value->GetName(), "origin"))
					{
						UTIL_StringToVector(VecOrigin.Base(), value->GetString());

						pEntity->SetAbsOrigin(VecOrigin);
					}
					else if (FStrEq(value->GetName(), "angle"))//if (!Q_strcmp(value->GetName(), "angle"))
					{

						UTIL_StringToVector(VecAngle.Base(), value->GetString());

						pEntity->SetAbsAngles(QAngle(VecAngle.x, VecAngle.y, VecAngle.z));
						DebugColorMsg("With Angles: ");
						DebugColorMsg(value->GetString());
						DebugColorMsg("\n");
					}

					//if (!Q_strcmp(value->GetName(), "x"))
					if (FStrEq(value->GetName(), "x"))
					{
						VecOldOrigin.x = value->GetFloat("x", VecOldOrigin.x);
						pEntity->SetAbsOrigin(VecOldOrigin);
					}

					//if (!Q_strcmp(value->GetName(), "y"))
					if (FStrEq(value->GetName(), "y"))
					{
						VecOldOrigin.y = value->GetFloat("y", VecOldOrigin.y);
						pEntity->SetAbsOrigin(VecOldOrigin);
					}

					//if (!Q_strcmp(value->GetName(), "z"))
					if (FStrEq(value->GetName(), "z"))
					{
						VecOldOrigin.z = value->GetFloat("z", VecOldOrigin.z);
						pEntity->SetAbsOrigin(VecOldOrigin);
					}

					//if (!Q_strcmp(value->GetName(), "roll"))
					if (FStrEq(value->GetName(), "roll"))
					{
						AngOldOrigin[ROLL] = value->GetInt("yaw", AngOldOrigin[ROLL]);
						pEntity->SetAbsAngles(AngOldOrigin);
					}

					//if (!Q_strcmp(value->GetName(), "yaw"))
					if (FStrEq(value->GetName(), "yaw"))
					{
						AngOldOrigin[YAW] = value->GetInt("yaw", AngOldOrigin[YAW]);
						pEntity->SetAbsAngles(AngOldOrigin);
					}

					//if (!Q_strcmp(value->GetName(), "pitch"))
					if (FStrEq(value->GetName(), "pitch"))
					{
						AngOldOrigin[PITCH] = value->GetInt("pitch", AngOldOrigin[PITCH]);
						pEntity->SetAbsAngles(AngOldOrigin);
					}


				}

				KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
				if (pKeyValues)
				{
					FOR_EACH_VALUE(pKeyValues, pValues)
					{
						//if (!Q_strcmp(pValues->GetName(), "model") || !Q_strcmp(pValues->GetName(), "Model"))
						if (FStrEq(pValues->GetName(), "model") || FStrEq(pValues->GetName(), "Model"))
						{
							CBaseEntity::PrecacheModel(pValues->GetString());
							pEntity->SetModel(pValues->GetString());
							continue;
						}

						pEntity->KeyValue(pValues->GetName(), pValues->GetString());
					}
				}
				
				pEntity->Precache();
				DispatchSpawn(pEntity);
				
				if (pEntity)
				{
					FOR_EACH_VALUE(pClassname, value)
					{
						//if (!Q_strcmp(value->GetName(), "freeze"))
						if (FStrEq(value->GetName(), "freeze"))
						{
							freeze = true;
							pEntity->SetMoveType(MOVETYPE_NONE);
							pEntity->SetSolid(SOLID_VPHYSICS);
							pEntity->VPhysicsInitStatic();
						}

						//if (!Q_strcmp(value->GetName(), "velocity"))
						if (FStrEq(value->GetName(), "velocity"))
						{
							//vel = true;
							VelVector = Vector(0, 0, 0);
							UTIL_StringToVector(VelVector.Base(), value->GetString());
							pEntity->ApplyLocalVelocityImpulse(VelVector);
						}

						//if (!Q_strcmp(value->GetName(), "longrange"))
						if (FStrEq(value->GetName(), "longrange"))
						{
							pEntity->AddSpawnFlags(SF_NPC_LONG_RANGE);
						}
					}
				}

				pEntity->Activate();
			}
		}
		else if (AllocPooledString(pClassname->GetName()) == AllocPooledString("env_sun"))
		{

			FOR_EACH_VALUE(pClassname, value)
			{
				//if (!Q_strcmp(value->GetName(), "x"))
				if (FStrEq(value->GetName(), "x"))
				{
					VecOldOrigin.x = value->GetFloat("x", VecOldOrigin.x);
				}

				//if (!Q_strcmp(value->GetName(), "y"))
				if (FStrEq(value->GetName(), "y"))
				{
					VecOldOrigin.y = value->GetFloat("y", VecOldOrigin.y);
				}

				//if (!Q_strcmp(value->GetName(), "z"))
				if (FStrEq(value->GetName(), "z"))
				{
					VecOldOrigin.z = value->GetFloat("z", VecOldOrigin.z);
				}

				//if (!Q_strcmp(value->GetName(), "roll"))
				if (FStrEq(value->GetName(), "roll"))
				{
					AngOldOrigin[ROLL] = value->GetFloat();
				}

				//if (!Q_strcmp(value->GetName(), "yaw"))
				if (FStrEq(value->GetName(), "yaw"))
				{
					AngOldOrigin[YAW] = value->GetFloat();
				}

				//if (!Q_strcmp(value->GetName(), "pitch"))
				if (FStrEq(value->GetName(), "pitch"))
				{
					AngOldOrigin[PITCH] = value->GetFloat();
				}
			}

			ParseCSMInfo(pClassname, VecOldOrigin, AngOldOrigin);

			CBaseEntity *pEntity = CBaseEntity::CreateNoSpawn(pClassname->GetName(), VecOldOrigin, AngOldOrigin);

			gEntList.NotifyCreateEntity(pEntity);
			KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
			if (pKeyValues)
			{
				FOR_EACH_VALUE(pKeyValues, pValues)
				{
					//if (!Q_strcmp(pValues->GetName(), "model") || !Q_strcmp(pValues->GetName(), "Model"))
					if (FStrEq(pValues->GetName(), "model") || FStrEq(pValues->GetName(), "Model"))
					{
						CBaseEntity::PrecacheModel(pValues->GetString());
						pEntity->SetModel(pValues->GetString());
						continue;
					}
					pEntity->KeyValue(pValues->GetName(), pValues->GetString());
				}
			}

			pEntity->Precache();

			DispatchSpawn(pEntity);

			if (pEntity)
			{
				FOR_EACH_VALUE(pClassname, value)
				{
					//if (!Q_strcmp(value->GetName(), "freeze"))
					if (FStrEq(value->GetName(), "freeze"))
					{
						freeze = true;
						pEntity->SetMoveType(MOVETYPE_NONE);
						pEntity->SetSolid(SOLID_VPHYSICS);
						pEntity->VPhysicsInitStatic();
					}

					//if (!Q_strcmp(value->GetName(), "velocity"))
					if (FStrEq(value->GetName(), "velocity"))
					{
						//vel = true;
						VelVector = Vector(0, 0, 0);
						UTIL_StringToVector(VelVector.Base(), value->GetString());
						pEntity->ApplyLocalVelocityImpulse(VelVector);
					}

					//if (!Q_strcmp(value->GetName(), "longrange"))
					if (FStrEq(value->GetName(), "longrange"))
					{
						pEntity->AddSpawnFlags(SF_NPC_LONG_RANGE);
					}
				}
			}
			pEntity->Activate();
		}
	}
}