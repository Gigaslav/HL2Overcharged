#include "cbase.h"
#include "mapadd.h"
#include "filesystem.h"
#include "gameweaponmanager.h"
#include "player.h"
#include "ai_basenpc.h"
#include "basehlcombatweapon.h"
//#include "weapon_custom.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// To enable me open gameinterface.cpp in method levelinit enable LevelInit_ParseAllEntities and dont forget do discomment instant_trig

void CC_CallLabel(const CCommand &args)
{
	CMapAdd *pMapadd = GetMapAddEntity();

	char szMapadd[128];
	Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);
	if (!pMapadd)
	{
		pMapadd = CreateMapAddEntity();
		pMapadd->RunLabel(szMapadd, args[1]);
	}
	else
	{
		pMapadd->RunLabel(szMapadd, args[1]);//		pMapadd->RunLabel(szMapadd, "Init");//
	}
}
static ConCommand mapadd_runlabel("mapadd_runlabel", CC_CallLabel, "Run a Mapadd label. Useful for testing.\n");

static CMapAdd *g_MapAddEntity = NULL;
LINK_ENTITY_TO_CLASS(mapadd, CMapAdd);
CMapAdd* GetMapAddEntity()
{
	if (g_MapAddEntity != NULL/* && !g_MapAddEntity->IsMarkedForDeletion()*/)
	{
		/*g_MapAddEntity->SetEFlags(EFL_KILLME);
		UTIL_Remove(g_MapAddEntity);*/
		//delete g_MapAddEntity;
		//g_MapAddEntity = NULL;
	}
	//	if(!g_MapAddEntity)
	//		return CreateMapAddEntity();
	return g_MapAddEntity;
}
CMapAdd* CreateMapAddEntity()
{
	return dynamic_cast<CMapAdd*>((CBaseEntity*)CBaseEntity::Create("mapadd", Vector(0, 0, 0), QAngle(0, 0, 0)));
}
void CMapAdd::Precache(void)
{
	g_MapAddEntity = this;
}

void CMapAdd::Destroy()
{
	/*if (g_MapAddEntity)
		UTIL_Remove(g_MapAddEntity);*/
	//UTIL_Remove(this);
	//delete this;
}
bool CMapAdd::RunLabelSun(KeyValues *keyvalues)//(const char *mapaddMap, const char *szLabel)
{
	DevMsg("RunLabelSun\n");
	/*if (AllocPooledString(mapaddMap) == AllocPooledString("") || !mapaddMap || !szLabel || AllocPooledString(szLabel) == AllocPooledString(""))//Update
	{
		DevMsg("MAPADD FAILED TO LOAD");//Update
		return false;
	}
	KeyValues *pMapAdd2 = new KeyValues("entities");
	if (pMapAdd2->LoadFromFile(filesystem, mapaddMap, "MOD"))*/
	{
		//DevMsg("LoadFromFile %s \n", mapaddMap);
		KeyValues *pMapAddEnt = keyvalues;
		if (pMapAddEnt)
		{
			DevMsg("pMapAddEnt->GetName() %s \n", pMapAddEnt->GetName());
			//KeyValues *pMapAddEnt = pMapAdd2->FindKey("env_sun");//pMapAdd2->GetFirstTrueSubKey();
			while (pMapAddEnt && !HandleSMODEntityName(pMapAddEnt->GetName()))
			{
				
				/*if (AllocPooledString(pMapAddEnt->GetName()) != AllocPooledString("event") && !HandlePlayerEntity(pMapAddEnt, false)
					&& !HandleRemoveEnitity(pMapAddEnt)
					&& !HandleSMODEntity(pMapAddEnt)
					&& !HandleSpecialEnitity(pMapAddEnt)
					&& !HandleWeaponManagerEnitity(pMapAddEnt))*///!HandleSMODEntityName(szLabel))
				{
					Vector SpawnVector = Vector(0, 0, 0);
					QAngle SpawnAngle = QAngle(0, 0, 0);
					KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
					KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
					KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
					KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
					KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
					KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
					KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
					KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");
					if (pEntKeyOrig)
					{
						const char *position = pMapAddEnt->GetString("origin", "0 0 0");
						UTIL_StringToVector(SpawnVector.Base(), position);
					}
					if (pEntKeyAng)
					{
						float tmp[2];
						const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
						UTIL_StringToFloatArray(tmp, 2, angle);
						SpawnAngle[PITCH] = tmp[2];
						SpawnAngle[YAW] = tmp[1];
						SpawnAngle[ROLL] = tmp[0];
					}
					if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
					{
						SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
						SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
						SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);
						SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
						SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
						SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
					}
					CBaseEntity *createEnt = CBaseEntity::CreateNoSpawnInstance(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);
					KeyValues *pEntKeyLongRange = pMapAddEnt->FindKey("longrange");
					KeyValues *pEntKeyFreeze = pMapAddEnt->FindKey("freeze");
					KeyValues *pEntKeyVel = pMapAddEnt->FindKey("velocity");
					KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
					KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
					if (createEnt)
					{
						if (pEntKeyLongRange)
							createEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);

						if (pEntKeyValues)
						{
							DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
							KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
							while (pEntKeyValuesAdd && createEnt)
							{
								if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
								{
									PrecacheModel(pEntKeyValuesAdd->GetString(""));
									createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
								}
								else
								{
									createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
								}
								pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
							}
						}
						if (pEntFlags)
						{
							DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
							KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
							while (pEntFlagsAdd && createEnt)
							{
								createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
								pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
							}
						}
						DispatchSpawn(createEnt); //I derped
					}
					if (createEnt)
					{
						if (pEntKeyFreeze)
						{
							createEnt->SetMoveType(MOVETYPE_NONE);
							createEnt->SetSolid(SOLID_VPHYSICS);
							createEnt->VPhysicsInitStatic();
						}
						if (pEntKeyVel)
						{
							Vector VelVector = Vector(0, 0, 0);
							const char *vel = pMapAddEnt->GetString("velocity", "0 0 0");
							UTIL_StringToVector(VelVector.Base(), vel);
							createEnt->ApplyLocalVelocityImpulse(VelVector);
						}
					}
					//createEnt->Activate();
				}
				pMapAddEnt = pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!
			}
		}
	}
	//pMapAdd2->deleteThis();
	Destroy();
	return true;
}
bool CMapAdd::RunLabel(const char *mapaddMap, const char *szLabel)
{

	/*KeyValues Error: RecursiveLoadFromBuffer:  got EOF instead of keyname in file mapadd/d1_town_01.txt
MapAdd, (*Init*), (*npc_zombie_torso*), (*KeyValues*), 
M*/

	if (AllocPooledString(mapaddMap) == AllocPooledString("") || !mapaddMap || !szLabel || AllocPooledString(szLabel) == AllocPooledString(""))//Update
	{
		DevMsg("MAPADD FAILED TO LOAD");//Update
		return false;
	}
 //Failed to load!
	//FileHandle_t fh = filesystem->Open(szMapadd,"r","MOD");
	// Open the mapadd data file, and abort if we can't

	KeyValues *pMapAdd2 = new KeyValues("entities");


	if (pMapAdd2->LoadFromFile(filesystem, mapaddMap, "MOD"))
	{


		//KeyValues *pMapAdd2 = pMapAdd->FindKey("Init");

		if (pMapAdd2)
		{

			KeyValues *pMapAddEnt = pMapAdd2->GetFirstTrueSubKey();

			while (pMapAddEnt && !HandleSMODEntityName(pMapAddEnt->GetName()))
			{

				DevMsg("while pMapAddEnt %s \n", pMapAddEnt->GetName());
				HandleSMODEntityName(szLabel);

				if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("event"))
				{
					CBaseEntity *modEnt = NULL;
					DevMsg("pEntEventValues %s \n", pMapAddEnt->GetName());
					KeyValues *pEntKeyValuesAdd = pMapAddEnt->GetFirstValue();
					while (pEntKeyValuesAdd)
					{
						DevMsg("pEntKeyValuesAdd name %s \n", pEntKeyValuesAdd->GetName());

						if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("targetname"))
						{
							if (!modEnt)
								modEnt = gEntList.FindEntityByName(this, pEntKeyValuesAdd->GetString(""));
							if (!modEnt)
								modEnt = gEntList.FindEntityByNameWithin(this, pEntKeyValuesAdd->GetString(""), this->GetAbsOrigin(), MAX_TRACE_LENGTH);
							if (!modEnt)
								modEnt = gEntList.FindEntityByNameNearest(pEntKeyValuesAdd->GetString(""), this->GetAbsOrigin(), MAX_TRACE_LENGTH);
							if (modEnt)
								DevMsg("modEnt name %s \n", modEnt->GetEntityName());
						}
						else if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model") && modEnt)
						{
							DevMsg("model \n");
							PrecacheModel(pEntKeyValuesAdd->GetString(""));
							modEnt->SetModel(pEntKeyValuesAdd->GetString(""));
						}
						else
						{
							DevMsg("else \n");
							if (modEnt)
								modEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
						}

						KeyValues *pEntFreeze = pMapAddEnt->FindKey("freeze");
						if (pEntFreeze && modEnt)
						{
							modEnt->SetMoveType(MOVETYPE_NONE);
							modEnt->SetSolid(SOLID_VPHYSICS);
							modEnt->VPhysicsInitStatic();
						}

						pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
					}

					KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");

					if (pEntFlags && modEnt)
					{
						DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
						KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
						while (pEntFlagsAdd && modEnt)
						{
							modEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
							pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
						}
					}

				}
				else if (AllocPooledString(pMapAddEnt->GetName()) != AllocPooledString("event") && !HandlePlayerEntity(pMapAddEnt, false)
					&& !HandleRemoveEnitity(pMapAddEnt) 
					&& !HandleSMODEntity(pMapAddEnt) 
					&& !HandleSpecialEnitity(pMapAddEnt) 
					&& !HandleWeaponManagerEnitity(pMapAddEnt))//!HandleSMODEntityName(szLabel))
				{


					Vector SpawnVector = Vector(0, 0, 0);
					QAngle SpawnAngle = QAngle(0, 0, 0);

					KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
					KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
					KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
					KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
					KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
					KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
					KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
					KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");

					if (pEntKeyOrig)
					{
						const char *position = pMapAddEnt->GetString("origin", "0 0 0");
						UTIL_StringToVector(SpawnVector.Base(), position);

						if (pEntKeyAng)
						{
							float tmp[2];
							const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
							UTIL_StringToFloatArray(tmp, 2, angle);
							SpawnAngle[PITCH] = tmp[0];
							SpawnAngle[YAW] = tmp[1];
							SpawnAngle[ROLL] = tmp[2];
						}
					}
					if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
					{
						SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
						SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
						SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

						SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
						SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
						SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
					}

					//CBaseEntity *createEnt = NULL;
					//if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("instant_trig"))
					//{
					//CBaseEntity *createEnt = CreateEntityByName(pMapAddEnt->GetName());
						//createEnt->SetLocalOrigin(SpawnVector);
						//createEnt->SetLocalAngles(SpawnAngle);
					//}
					//else
					CBaseEntity *createEnt = CBaseEntity::CreateNoSpawnInstance(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);

					KeyValues *pEntKeyLongRange = pMapAddEnt->FindKey("longrange");
					KeyValues *pEntKeyFreeze = pMapAddEnt->FindKey("freeze");
					KeyValues *pEntKeyVel = pMapAddEnt->FindKey("velocity");

					KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
					KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
					if (createEnt)
					{
						if (pEntKeyLongRange)
							createEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);

						if (pEntKeyValues)
						{
							DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
							KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
							while (pEntKeyValuesAdd && createEnt)
							{
								if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
								{
									PrecacheModel(pEntKeyValuesAdd->GetString(""));
									createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
								}
								else
								{
									createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
								}
								pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
							}
						}

						if (pEntFlags)
						{
							DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
							KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
							while (pEntFlagsAdd && createEnt)
							{
								createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
								pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
							}
						}
						DispatchSpawn(createEnt); //I derped
					}
					//DispatchSpawn(createEnt); //I derped
					if (createEnt)
					{
						if (pEntKeyFreeze)
						{
							//createEnt->KeyValue("freeze", true);

							/*createEnt->SetSolid(SOLID_BBOX);
							createEnt->AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);
							createEnt->SetMoveType(MOVETYPE_PUSH);*/

							createEnt->SetMoveType(MOVETYPE_NONE);
							createEnt->SetSolid(SOLID_VPHYSICS);
							createEnt->VPhysicsInitStatic();
						}
						if (pEntKeyVel)
						{
							Vector VelVector = Vector(0, 0, 0);
							const char *vel = pMapAddEnt->GetString("velocity", "0 0 0");
							UTIL_StringToVector(VelVector.Base(), vel);
							//createEnt->SetAbsVelocity(VelVector);
							//createEnt->ApplyAbsVelocityImpulse(VelVector);
							//createEnt->ApplyLocalAngularVelocityImpulse(VelVector);
							createEnt->ApplyLocalVelocityImpulse(VelVector);
						}
					}
				}
				pMapAddEnt = pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!
			}

			//KeyValues *pMapAddEnt2 = pMapAdd2->GetFirstTrueSubKey();
			//HandleRemoveEnitity(pMapAddEnt2);
		}
	}

	pMapAdd2->deleteThis();

	//if (pMapAdd->IsEmpty())
	Destroy();

	return true;
}

bool CMapAdd::RunTriggerLabel(const char *mapaddMap, const char *szLabel)
{
	DevMsg("RunTriggerLabel \n");//Update
	if (AllocPooledString(mapaddMap) == AllocPooledString("") || !mapaddMap || !szLabel || AllocPooledString(szLabel) == AllocPooledString(""))//Update
	{
		DevMsg("MAPADD FAILED TO LOAD");//Update
		return false;
	}

	KeyValues *pMapAdd2 = new KeyValues(szLabel);


	if (pMapAdd2->LoadFromFile(filesystem, mapaddMap, "MOD"))
	{

		DevMsg("pMapAdd->LoadFromFile \n");//Update

		//KeyValues *pMapAdd2 = pMapAdd->FindKey("Init");

		//while (pMapAdd2)//init
		{
			DevMsg("pMapAdd2 = pMapAdd->FindKey(Init) \n");//Update

			//KeyValues *pMapAdd3 = pMapAdd2->FindKey(szLabel);
			//if (pMapAdd3)
			{
				//DevMsg("pMapAdd3 GetName %s \n", pMapAdd3->GetName());

				KeyValues *pMapAddEnt = pMapAdd2->GetFirstTrueSubKey();//pMapAdd2->FindKey(szLabel);

				while (pMapAddEnt)//HandleSMODEntityName(szLabel))
				{

					DevMsg("while pMapAddEnt %s \n", pMapAddEnt->GetName());
					HandleSMODEntityName(szLabel);


					if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("event"))
					{
						CBaseEntity *modEnt = NULL;
						DevMsg("event %s \n", pMapAddEnt->GetName());
						KeyValues *pEntKeyValuesAdd = pMapAddEnt->GetFirstValue();
						while (pEntKeyValuesAdd)
						{
							DevMsg("pEntKeyValuesAdd name %s \n", pEntKeyValuesAdd->GetName());

							if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("targetname"))
							{
								if (!modEnt)
									modEnt = gEntList.FindEntityByName(this, pEntKeyValuesAdd->GetString(""));
								if (!modEnt)
									modEnt = gEntList.FindEntityByNameWithin(this, pEntKeyValuesAdd->GetString(""), this->GetAbsOrigin(), MAX_TRACE_LENGTH);
								if (!modEnt)
									modEnt = gEntList.FindEntityByNameNearest(pEntKeyValuesAdd->GetString(""), this->GetAbsOrigin(), MAX_TRACE_LENGTH);
								if (modEnt)
									DevMsg("modEnt name %s \n", modEnt->GetEntityName());
							}
							else if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model") && modEnt)
							{
								DevMsg("model \n");
								PrecacheModel(pEntKeyValuesAdd->GetString(""));
								modEnt->SetModel(pEntKeyValuesAdd->GetString(""));
							}
							/*else if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("freeze") && modEnt)
							{
								DevMsg("freeze \n");
								modEnt->SetMoveType(MOVETYPE_NONE);
								modEnt->SetSolid(SOLID_VPHYSICS);
								modEnt->VPhysicsInitStatic();
							}*/
							else
							{
								DevMsg("else \n");
								if (modEnt)
									modEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
							}

							KeyValues *pEntFreeze = pMapAddEnt->FindKey("freeze");
							if (pEntFreeze && modEnt)
							{
								modEnt->SetMoveType(MOVETYPE_NONE);
								modEnt->SetSolid(SOLID_VPHYSICS);
								modEnt->VPhysicsInitStatic();
							}

							pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
						}

						KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");

						if (pEntFlags && modEnt)
						{
							DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
							KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
							while (pEntFlagsAdd && modEnt)
							{
								modEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
								pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
							}
						}

					}
					else if (AllocPooledString(pMapAddEnt->GetName()) != AllocPooledString("event") && (!HandlePlayerEntity(pMapAddEnt, false)
						&& !HandleRemoveEnitity(pMapAddEnt)
						&& !HandleSMODEntity(pMapAddEnt)
						&& !HandleSpecialEnitity(pMapAddEnt)
						&& !HandleWeaponManagerEnitity(pMapAddEnt)))//!HandleSMODEntityName(szLabel))
					{
							Vector SpawnVector = Vector(0, 0, 0);
							QAngle SpawnAngle = QAngle(0, 0, 0);

							KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
							KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
							KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
							KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
							KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
							KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
							KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
							KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");

							if (pEntKeyOrig)
							{
								const char *position = pMapAddEnt->GetString("origin", "0 0 0");
								UTIL_StringToVector(SpawnVector.Base(), position);

								if (pEntKeyAng)
								{
									float tmp[2];
									const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
									UTIL_StringToFloatArray(tmp, 2, angle);
									SpawnAngle[PITCH] = tmp[0];
									SpawnAngle[YAW] = tmp[1];
									SpawnAngle[ROLL] = tmp[2];
								}
							}
							if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
							{
								SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
								SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
								SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

								SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
								SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
								SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
							}

							//CBaseEntity *createEnt = NULL;
							//if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("instant_trig"))
							//{
							//CBaseEntity *createEnt = CreateEntityByName(pMapAddEnt->GetName());
							//createEnt->SetLocalOrigin(SpawnVector);
							//createEnt->SetLocalAngles(SpawnAngle);
							//}
							//else
							CBaseEntity *createEnt = CBaseEntity::CreateNoSpawnInstance(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);

							KeyValues *pEntKeyLongRange = pMapAddEnt->FindKey("longrange");
							KeyValues *pEntKeyFreeze = pMapAddEnt->FindKey("freeze");
							KeyValues *pEntKeyVel = pMapAddEnt->FindKey("velocity");

							KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
							KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
							if (createEnt)
							{
								if (pEntKeyLongRange)
									createEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);

								if (pEntKeyValues)
								{
									DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
									KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
									while (pEntKeyValuesAdd && createEnt)
									{
										if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
										{
											PrecacheModel(pEntKeyValuesAdd->GetString(""));
											createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
										}
										else
										{
											createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
										}
										pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
									}
								}

								if (pEntFlags)
								{
									DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
									KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
									while (pEntFlagsAdd && createEnt)
									{
										createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
										pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
									}
								}
								DispatchSpawn(createEnt); //I derped
							}
							//DispatchSpawn(createEnt); //I derped
							if (createEnt)
							{
								if (pEntKeyFreeze)
								{
									//createEnt->KeyValue("freeze", true);

									/*createEnt->SetSolid(SOLID_BBOX);
									createEnt->AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);
									createEnt->SetMoveType(MOVETYPE_PUSH);*/

									createEnt->SetMoveType(MOVETYPE_NONE);
									createEnt->SetSolid(SOLID_VPHYSICS);
									createEnt->VPhysicsInitStatic();
								}
								if (pEntKeyVel)
								{
									Vector VelVector = Vector(0, 0, 0);
									const char *vel = pMapAddEnt->GetString("velocity", "0 0 0");
									UTIL_StringToVector(VelVector.Base(), vel);
									//createEnt->SetAbsVelocity(VelVector);
									//createEnt->ApplyAbsVelocityImpulse(VelVector);
									//createEnt->ApplyLocalAngularVelocityImpulse(VelVector);
									createEnt->ApplyLocalVelocityImpulse(VelVector);

								}
							}
					}
					pMapAddEnt = pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!
				}
			}
			pMapAdd2 = pMapAdd2->GetNextTrueSubKey();
			//KeyValues *pMapAddEnt2 = pMapAdd2->GetFirstTrueSubKey();
			//HandleRemoveEnitity(pMapAddEnt2);
		}
	}

	pMapAdd2->deleteThis();

	//if (pMapAdd->IsEmpty())
	Destroy();

	return true;
}

bool CMapAdd::HandlePlayerEntity(KeyValues *playerEntityKV, bool initLevel)
{
	if (AllocPooledString(playerEntityKV->GetName()) == AllocPooledString("player"))
	{
		//	if(initLevel)
		//	{
		//		return true; //Just pretend we did
		//	}

		//playerEnt->PrecacheSound();
		//playerEnt->EmitAmbientSound(playerEnt->entindex(), this->GetAbsOrigin(), "");

		KeyValues *pEntPlr = playerEntityKV->GetFirstValue();

		DevMsg("HandlePlayerEntity - player \n");
		while (pEntPlr)
		{
			DevMsg("HandlePlayerEntity - GetFirstValue \n");
			if (AllocPooledString(pEntPlr->GetName()) == AllocPooledString("music"))
			{
				DevMsg("HandlePlayerEntity - AllocPooledString(music) \n");
				DevMsg("music %s \n", pEntPlr->GetString());
				CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
				if (playerEnt)
				{
					playerEnt->PrecacheScriptSound(pEntPlr->GetString());
					playerEnt->EmitAmbientSound(playerEnt->entindex(), playerEnt->GetAbsOrigin(), pEntPlr->GetString());
				}
				/*enginesound->PrecacheSound(pEntPlr->GetString());
				enginesound->EmitAmbientSound(pEntPlr->GetString(), VOL_NORM, PITCH_NORM);*/
				//break;
				//continue;
			}
			else if (AllocPooledString(pEntPlr->GetName()) == AllocPooledString("message"))
			{
				DevMsg("HandlePlayerEntity - AllocPooledString(message) \n");
				DevMsg("message %s \n", pEntPlr->GetString());

				UTIL_ShowMessage(pEntPlr->GetString(), UTIL_GetLocalPlayer());

				/*CBaseEntity *createEnt = CBaseEntity::CreateNoSpawnInstance("mm_game_text", Vector(0,0,0), QAngle(0,0,0));
				if (createEnt)
				{
					createEnt->KeyValue("message", pEntPlr->GetString());

					DispatchSpawn(createEnt); //I derped
				}*/

			}
			pEntPlr = pEntPlr->GetNextValue();
			//enginesound->GetActiveSounds()

			if (!pEntPlr)
				break;
		}
		return true;
	}
	return false;
}
bool CMapAdd::HandleSMODEntity(KeyValues *smodEntity)
{
	/*char szSmodEntityName[128];
	Q_snprintf(szSmodEntityName, sizeof(szSmodEntityName), "%s", smodEntity->GetName());

	if (Q_stristr(szSmodEntityName, "entities:"))
		return true;*/

	return false;
}
bool CMapAdd::HandleSMODEntityName(const char *szLabel)
{
	char szSmodEntityName[128];
	Q_snprintf(szSmodEntityName, sizeof(szSmodEntityName), "%s", AllocPooledString(szLabel));

	DevMsg("HandleSMODEntityName pMapAddEnt %s \n", AllocPooledString(szLabel));
	DevMsg("HandleSMODEntityName\n");
	if (Q_stristr(szSmodEntityName, "entities:"))
	{
		DevMsg("HandleSMODEntityName true\n");
		return true;
	}

	return false;
}
bool CMapAdd::HandleSpecialEnitity(KeyValues *specialEntity)
{
	return false;
}
bool CMapAdd::HandleRemoveEnitity(KeyValues *mapaddValue)
{
	if (AllocPooledString(mapaddValue->GetName()) == AllocPooledString("remove:sphere"))
	{
		//engine->ServerCommand("ent_remove env_sun\n");
		Vector RemoveVector = Vector(0, 0, 0);
		CBaseEntity *ppEnts[1024];//256
		//		CBaseEntity *ppCandidates[256];

		KeyValues *pEntKeyOrig = mapaddValue->FindKey("origin");
		KeyValues *pEntKeyAng = mapaddValue->FindKey("angle");
		KeyValues *pEntKeyX = mapaddValue->FindKey("x");
		KeyValues *pEntKeyY = mapaddValue->FindKey("y");
		KeyValues *pEntKeyZ = mapaddValue->FindKey("z");
		KeyValues *pEntKeyRoll = mapaddValue->FindKey("roll");
		KeyValues *pEntKeyYaw = mapaddValue->FindKey("yaw");
		KeyValues *pEntKeyPitch = mapaddValue->FindKey("pitch");

		if (pEntKeyOrig && pEntKeyAng)
		{
			const char *position = mapaddValue->GetString("origin", "0 0 0");
			UTIL_StringToVector(RemoveVector.Base(), position);
		}
		if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
		{
			RemoveVector.x = mapaddValue->GetFloat("x", RemoveVector.x);
			RemoveVector.y = mapaddValue->GetFloat("y", RemoveVector.y);
			RemoveVector.z = mapaddValue->GetFloat("z", RemoveVector.z);

		}

		int nEntCount = UTIL_EntitiesInSphere(ppEnts, 1024, RemoveVector, mapaddValue->GetFloat("radius", 0), 0);//256
		//Look through the entities it found
		KeyValues *pEntKeyValues = mapaddValue->FindKey("entities");
		if (pEntKeyValues)
		{
			//DevMsg("REMOVE ENTITY \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

			KeyValues *pEntKeyValuesRemove = pEntKeyValues->GetFirstValue();
			//KeyValues *pEntKeyValuesRemove = pEntKeyValues->GetFirstTrueSubKey();
			while (pEntKeyValuesRemove)
			{
				/*DevMsg("REMOVE ENTITY \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
				DevMsg("radius: %.2f \n\n\n\n\n\n", mapaddValue->GetFloat("radius"));
			
				ppEnts[1]->SetName(AllocPooledString("env_sun"));//pEntKeyValuesRemove->GetName()
				UTIL_Remove(ppEnts[1]);
				cvar->FindVar("ent_remove")->SetValue("env_sun");
				engine->ServerCommand("ent_remove\n");
				//engine->ServerCommand("ent_remove env_sun\n");*/
				int i;
				for (i = 0; i < nEntCount; i++)
				{

					//DevMsg("nEntCount: %.2f \n\n\n\n\n\n", nEntCount);
					if (ppEnts[i] == NULL)
						continue;
					if (AllocPooledString(pEntKeyValuesRemove->GetName()) == AllocPooledString("classname"))// || (( AllocPooledString(pEntKeyValuesRemove->GetName()) == ppEnts[i]->GetEntityName())))
					{
						//DevMsg("classname for %s Found!\n", pEntKeyValuesRemove->GetName());
						//DevMsg("classname for %s Found!\n", ppEnts[i]->GetEntityName());
						//UTIL_Remove(ppEnts[i]);
						//engine->ServerCommand("ent_remove env_sun\n");
						if (AllocPooledString(pEntKeyValuesRemove->GetString()) == AllocPooledString(ppEnts[i]->GetClassname()))//pEntKeyValuesRemove->GetString()
						{
							//DevMsg("ppEnts for %s Found!\n", pEntKeyValuesRemove->GetName());

							UTIL_Remove(ppEnts[i]);
							continue;
						}
					}
					if (AllocPooledString(pEntKeyValuesRemove->GetName()) == AllocPooledString("targetname")) // || ( AllocPooledString(pEntKeyValuesRemove->GetName()) == ppEnts[i]->GetEntityName())
					{
						if (AllocPooledString(pEntKeyValuesRemove->GetString()) == ppEnts[i]->GetEntityName())
						{
							UTIL_Remove(ppEnts[i]);
							continue;
						}
					}
				}
				pEntKeyValuesRemove = pEntKeyValuesRemove->GetNextValue();
			}
		}
		return true;
	}
	else if (AllocPooledString(mapaddValue->GetName()) == AllocPooledString("removeentity"))
	{

		CBaseEntity *ppEnts[1024];//256
		//		CBaseEntity *ppCandidates[256];

		int nEntCount = UTIL_EntitiesInSphere(ppEnts, 1024, WorldSpaceCenter(), MAX_TRACE_LENGTH, 0);//256
		//Look through the entities it found
		KeyValues *pEntKeyValuesRemove = mapaddValue->GetFirstValue();

			while (pEntKeyValuesRemove)
			{
				int i;
				for (i = 0; i < nEntCount; i++)
				{

					//DevMsg("nEntCount: %.2f \n\n\n\n\n\n", nEntCount);
					if (ppEnts[i] == NULL)
						continue;
					if (AllocPooledString(pEntKeyValuesRemove->GetName()) == AllocPooledString("classname"))
					{

						if (AllocPooledString(pEntKeyValuesRemove->GetString()) == AllocPooledString(ppEnts[i]->GetClassname()))
						{
							UTIL_Remove(ppEnts[i]);
							continue;
						}
					}
					if (AllocPooledString(pEntKeyValuesRemove->GetName()) == AllocPooledString("targetname"))
					{
						if (AllocPooledString(pEntKeyValuesRemove->GetString()) == ppEnts[i]->GetEntityName())
						{
							UTIL_Remove(ppEnts[i]);
							continue;
						}
					}
				}
				pEntKeyValuesRemove = pEntKeyValuesRemove->GetNextValue();
			}
		return true;
	}
	return false;
}
bool CMapAdd::HandleWeaponManagerEnitity(KeyValues *weaponManagerEntity)
{
	if (AllocPooledString(weaponManagerEntity->GetName()) == AllocPooledString("gameweaponmanager"))
	{
		const char *pWeaponName = "";
		int iMaxAllowed = 0;
		pWeaponName = weaponManagerEntity->GetString("weaponname", pWeaponName);
		iMaxAllowed = weaponManagerEntity->GetFloat("maxallowed", iMaxAllowed);
		CreateWeaponManager(pWeaponName, iMaxAllowed);
		return true;
	}
	return false;
}
void CMapAdd::InputRunLabel(inputdata_t &inputData) //Input this directly!
{
	char szMapadd[128];
	Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);
	this->RunLabel(szMapadd, inputData.value.String());
}
BEGIN_DATADESC(CMapAdd)
// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "RunLabel", InputRunLabel),
END_DATADESC()






class CMapChange : public CBaseEntity
{
public:
	DECLARE_CLASS(CMapChange, CBaseEntity);
	DECLARE_DATADESC();

	CMapChange(int MnEntIndex = 0);

	void Spawn()
	{
		BaseClass::Spawn();
		SetNextThink(gpGlobals->curtime);
	}

	void Think();

private:
	float	m_nTriggerArea;
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;
protected:
	int m_nEntIndexM;
};

CMapChange::CMapChange(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_bDeleteOnFire = true;
}
void CMapChange::Think()
{
	BaseClass::Think();

	CBaseEntity *ppEnts[256];
	int nEntCount = UTIL_EntitiesInSphere(ppEnts, 256, this->GetAbsOrigin(), m_nTriggerArea, 0);
	int i;
	for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (ppEnts[i] != NULL)
			if (ppEnts[i]->IsPlayer())
			{
				Msg("mapadd_trig was triggered!\n");
				m_OnTrigger.FireOutput(ppEnts[i], this);
				//	if(m_bDeleteOnFire)
				//	{
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
				//UTIL_Remove(this);

				char szMapadd[128];
				Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);
				//KeyValues *pMapAdd = new KeyValues("AddEntity");
				KeyValues *pMapAdd = new KeyValues("MapAdd");


				if (pMapAdd->LoadFromFile(filesystem, szMapadd, "MOD"))
				{
					DevMsg("KeyValues(AddEntity)");
					//KeyValues *pMapAdd2 = pMapAdd->FindKey("Init");

					//if (pMapAdd2)
					{

						//KeyValues *pMapAddEntP = pMapAdd->FindKey("AddEntity");
						KeyValues *pMapAddEntP = pMapAdd->FindKey("Init");
						if (pMapAddEntP)
						{

							DevMsg("KeyValues(pMapAddEntP)");


							KeyValues *pMapAddEntS = pMapAddEntP->FindKey("mapadd_changelevel");
							if (pMapAddEntS)
							{
								/*DevMsg("m_nEntIndexM: %.2f \n", m_nEntIndexM);
								KeyValues *pMapAddEntV = pMapAddEntS->GetFirstValue();
								if (pMapAddEntV->GetInt() == m_nEntIndexM)
								{*/

								DevMsg("KeyValues(mapadd_trig)");
								KeyValues *pMapAddEntD = pMapAddEntS->FindKey("AddEntity");
								if (pMapAddEntS)
								{

									DevMsg("KeyValues(AddEntity)");
									//KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
									KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
									while (pMapAddEntD)//if (pMapAddEnt)
									{
										DevMsg("KeyValues(AddEntity)");

										Vector SpawnVector = Vector(0, 0, 0);
										QAngle SpawnAngle = QAngle(0, 0, 0);

										KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
										KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
										KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
										KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
										KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
										KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
										KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
										KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");

										if (pEntKeyOrig && pEntKeyAng)
										{
											const char *position = pMapAddEnt->GetString("origin", "0 0 0");
											UTIL_StringToVector(SpawnVector.Base(), position);
											float tmp[2];
											const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
											UTIL_StringToFloatArray(tmp, 2, angle);
											SpawnAngle[PITCH] = tmp[0];
											SpawnAngle[YAW] = tmp[1];
											SpawnAngle[ROLL] = tmp[2];
										}
										if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
										{
											SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
											SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
											SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

											SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
											SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
											SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
										}

										CBaseEntity *createEnt = CBaseEntity::CreateNoSpawn(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);


										KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
										KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
										if (createEnt)
										{
											if (pEntKeyValues)
											{
												DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
												KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
												while (pEntKeyValuesAdd && createEnt)
												{
													if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("map"))
													{
														/*CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
														if (!playerEnt) //He doesn't exist, just pretend
															break;*/
														CBasePlayer *playerEnt = ToBasePlayer(ppEnts[i]);
														if (!playerEnt) //He doesn't exist, just pretend
															break;

														cvar->FindVar("oc_mapadd_changelevel")->SetValue(1);

														cvar->FindVar("PlayerHealth")->SetValue(playerEnt->GetHealth());
														cvar->FindVar("PlayerArmor")->SetValue(playerEnt->ArmorValue());

														if (playerEnt->HasNamedPlayerItem("weapon_airboatgun"))
															cvar->FindVar("weapon_airboatgun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("weapon_ar3"))
															cvar->FindVar("weapon_ar3")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_ak47"))
															cvar->FindVar("Weapon_ak47")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_autoshotgun"))
															cvar->FindVar("Weapon_autoshotgun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_barnacle"))
															cvar->FindVar("Weapon_barnacle")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_beretta"))
															cvar->FindVar("Weapon_beretta")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_binoculars"))
															cvar->FindVar("Weapon_binoculars")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_bee"))
															cvar->FindVar("Weapon_bee")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_adrenaline"))
															cvar->FindVar("Weapon_adrenaline")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_cguard"))
															cvar->FindVar("Weapon_cguard")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_deagle"))
															cvar->FindVar("Weapon_deagle")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_disc"))
															cvar->FindVar("Weapon_disc")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_displacer"))
															cvar->FindVar("Weapon_displacer")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_dual_pistols"))
															cvar->FindVar("Weapon_dual_pistols")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_egon"))
															cvar->FindVar("Weapon_egon")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_extinguisher"))
															cvar->FindVar("Weapon_extinguisher")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_fireball"))
															cvar->FindVar("Weapon_fireball")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_flamethrower"))
															cvar->FindVar("Weapon_flamethrower")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_flaregun"))
															cvar->FindVar("Weapon_flaregun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_gauss"))
															cvar->FindVar("Weapon_gauss")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_glock"))
															cvar->FindVar("Weapon_glock")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_grapple"))
															cvar->FindVar("Weapon_grapple")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_grenadelauncher"))
															cvar->FindVar("Weapon_grenadelauncher")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_hopwire"))
															cvar->FindVar("Weapon_hopwire")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_iceaxe"))
															cvar->FindVar("Weapon_iceaxe")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_immolator"))
															cvar->FindVar("Weapon_immolator")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_ionrifle"))
															cvar->FindVar("Weapon_ionrifle")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_knife"))
															cvar->FindVar("Weapon_knife")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_laser"))
															cvar->FindVar("Weapon_laser")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_laserpistol"))
															cvar->FindVar("Weapon_laserpistol")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_m4"))
															cvar->FindVar("Weapon_m4")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_machinegun"))
															cvar->FindVar("Weapon_machinegun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_molotov"))
															cvar->FindVar("Weapon_molotov")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_mp5"))
															cvar->FindVar("Weapon_mp5")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_objective"))
															cvar->FindVar("Weapon_objective")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_oicw"))
															cvar->FindVar("Weapon_oicw")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_physgun"))
															cvar->FindVar("Weapon_physgun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_pistol_silent"))
															cvar->FindVar("Weapon_pistol_silent")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_sg552"))
															cvar->FindVar("Weapon_sg552")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_shockrifle"))
															cvar->FindVar("Weapon_shockrifle")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_smg2"))
															cvar->FindVar("Weapon_smg2")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_smokegrenade"))
															cvar->FindVar("Weapon_smokegrenade")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_sniper"))
															cvar->FindVar("Weapon_sniper")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_teleport"))
															cvar->FindVar("Weapon_teleport")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_thumper"))
															cvar->FindVar("Weapon_thumper")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_wrench"))
															cvar->FindVar("Weapon_wrench")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("grenade_ar2"))
															cvar->FindVar("grenade_ar2")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_frag"))
															cvar->FindVar("Weapon_frag")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_bugbait"))
															cvar->FindVar("Weapon_bugbait")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_357"))
															cvar->FindVar("Weapon_357")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_pistol"))
															cvar->FindVar("Weapon_pistol")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_ar2"))
															cvar->FindVar("Weapon_ar2")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_smg1"))
															cvar->FindVar("Weapon_smg1")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_rpg"))
															cvar->FindVar("Weapon_rpg")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_shotgun"))
															cvar->FindVar("Weapon_shotgun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_stunstick"))
															cvar->FindVar("Weapon_stunstick")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_crowbar"))
															cvar->FindVar("Weapon_crowbar")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_cubemap"))
															cvar->FindVar("Weapon_cubemap")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_crossbow"))
															cvar->FindVar("Weapon_crossbow")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_physcannon"))
															cvar->FindVar("Weapon_physcannon")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_alyxgun"))
															cvar->FindVar("Weapon_alyxgun")->SetValue(1);
														if (playerEnt->HasNamedPlayerItem("Weapon_annabelle"))
															cvar->FindVar("Weapon_annabelle")->SetValue(1);

														if (playerEnt->HasNamedPlayerItem("item_suit"))
															cvar->FindVar("item_suit")->SetValue(1);

														//cvar->FindVar("last_weapon")->SetDefault(playerEnt->GetActiveWeapon()->GetName());

														char szCommand[256];
														Q_snprintf(szCommand, sizeof(szCommand), "%s\n", pEntKeyValuesAdd->GetString(""));
														engine->ServerCommand(szCommand);

													}

													if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
													{
														PrecacheModel(pEntKeyValuesAdd->GetString(""));
														createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
													}
													else
													{
														createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
													}
													pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
												}
											}

											if (pEntFlags)
											{
												DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
												KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
												while (pEntFlagsAdd && createEnt)
												{
													createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
													pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
												}
											}
										}
										//createEnt->Activate();//Is this a good idea? Not sure!
										//createEnt->Spawn();
										DispatchSpawn(createEnt); //I derped

										pMapAddEnt = pMapAddEnt->GetNextKey();//pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!

										if (pMapAddEnt->GetNextKey() == NULL)
											break;

										if (pMapAddEnt->IsEmpty())
											break;

										if (pMapAddEnt->FindKey("break"))
											break;
									}

								}
								//}
							}
						}
					}
				}

				UTIL_Remove(this);
				break;
				//	}
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
			}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second
}

LINK_ENTITY_TO_CLASS(mapadd_changelevel, CMapChange);
BEGIN_DATADESC(CMapChange)

DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
DEFINE_THINKFUNC(Think),

END_DATADESC()







class CMapSave : public CBaseEntity
{
public:
	DECLARE_CLASS(CMapSave, CBaseEntity);
	DECLARE_DATADESC();

	CMapSave(int MnEntIndex = 0);

	void Spawn()
	{
		BaseClass::Spawn();
		SetNextThink(gpGlobals->curtime);
	}

	void Think();

private:
	float	m_nTriggerArea;
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;
protected:
	int m_nEntIndexM;
};

CMapSave::CMapSave(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_bDeleteOnFire = true;
}
void CMapSave::Think()
{
	BaseClass::Think();

	CBaseEntity *ppEnts[256];
	int nEntCount = UTIL_EntitiesInSphere(ppEnts, 256, this->GetAbsOrigin(), m_nTriggerArea, 0);
	int i;
	for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (ppEnts[i] != NULL)
			if (ppEnts[i]->IsPlayer())
			{
				Msg("mapadd_trig was triggered!\n");
				m_OnTrigger.FireOutput(ppEnts[i], this);
				//	if(m_bDeleteOnFire)
				//	{
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
				//UTIL_Remove(this);

				char szMapadd[128];
				Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);
				//KeyValues *pMapAdd = new KeyValues("AddEntity");
				KeyValues *pMapAdd = new KeyValues("MapAdd");


				if (pMapAdd->LoadFromFile(filesystem, szMapadd, "MOD"))
				{
					DevMsg("KeyValues(AddEntity)");
					//KeyValues *pMapAdd2 = pMapAdd->FindKey("Init");

					//if (pMapAdd2)
					{

						//KeyValues *pMapAddEntP = pMapAdd->FindKey("AddEntity");
						KeyValues *pMapAddEntP = pMapAdd->FindKey("Init");
						if (pMapAddEntP)
						{

							DevMsg("KeyValues(pMapAddEntP)");


							KeyValues *pMapAddEntS = pMapAddEntP->FindKey("mapadd_save");
							if (pMapAddEntS)
							{
								/*DevMsg("m_nEntIndexM: %.2f \n", m_nEntIndexM);
								KeyValues *pMapAddEntV = pMapAddEntS->GetFirstValue();
								if (pMapAddEntV->GetInt() == m_nEntIndexM)
								{*/

								DevMsg("KeyValues(mapadd_trig)");
								KeyValues *pMapAddEntD = pMapAddEntS->FindKey("Restore_Supply");
								while (pMapAddEntD)
								{


														DevMsg("save\n\n\n\n");
														/*CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
														if (!playerEnt) //He doesn't exist, just pretend
														break;*/
														CBasePlayer *playerEnt = ToBasePlayer(ppEnts[i]);
														/*if (!playerEnt) //He doesn't exist, just pretend
															break;*/

														//DevMsg("MapAddNewLevel: %i \n", MapAddNewLevel);
														if (cvar->FindVar("oc_mapadd_changelevel")->GetInt())
														{
															DevMsg("oc_mapadd_changelevel\n");
															playerEnt->SetHealth(cvar->FindVar("PlayerHealth")->GetInt());
															playerEnt->SetArmorValue(cvar->FindVar("PlayerArmor")->GetInt());

															if (cvar->FindVar("Weapon_airboatgun")->GetInt())
																engine->ServerCommand("give weapon_airboatgun\n");
															if (cvar->FindVar("Weapon_ar3")->GetInt())
																engine->ServerCommand("give weapon_ar3\n");
															if (cvar->FindVar("Weapon_ak47")->GetInt())
																engine->ServerCommand("give weapon_ak47\n");
															if (cvar->FindVar("Weapon_autoshotgun")->GetInt())
																engine->ServerCommand("give weapon_autoshotgun\n");
															if (cvar->FindVar("Weapon_barnacle")->GetInt())
																engine->ServerCommand("give weapon_barnacle\n");
															if (cvar->FindVar("Weapon_beretta")->GetInt())
																engine->ServerCommand("give weapon_beretta\n");
															if (cvar->FindVar("Weapon_binoculars")->GetInt())
																engine->ServerCommand("give weapon_binoculars\n");
															if (cvar->FindVar("Weapon_bee")->GetInt())
																engine->ServerCommand("give weapon_bee\n");
															if (cvar->FindVar("Weapon_adrenaline")->GetInt())
																engine->ServerCommand("give weapon_adrenaline\n");
															if (cvar->FindVar("Weapon_cguard")->GetInt())
																engine->ServerCommand("give weapon_cguard\n");
															if (cvar->FindVar("Weapon_deagle")->GetInt())
																engine->ServerCommand("give weapon_deagle\n");
															if (cvar->FindVar("Weapon_disc")->GetInt())
																engine->ServerCommand("give weapon_disc\n");
															if (cvar->FindVar("Weapon_displacer")->GetInt())
																engine->ServerCommand("give weapon_displacer\n");
															if (cvar->FindVar("Weapon_dual_pistols")->GetInt())
																engine->ServerCommand("give weapon_dual_pistols\n");
															if (cvar->FindVar("Weapon_egon")->GetInt())
																engine->ServerCommand("give weapon_egon\n");
															if (cvar->FindVar("Weapon_extinguisher")->GetInt())
																engine->ServerCommand("give weapon_extinguisher\n");
															if (cvar->FindVar("Weapon_fireball")->GetInt())
																engine->ServerCommand("give weapon_fireball\n");
															if (cvar->FindVar("Weapon_flamethrower")->GetInt())
																engine->ServerCommand("give weapon_flamethrower\n");
															if (cvar->FindVar("Weapon_flaregun")->GetInt())
																engine->ServerCommand("give weapon_flaregun\n");
															if (cvar->FindVar("Weapon_gauss")->GetInt())
																engine->ServerCommand("give weapon_gauss\n");
															if (cvar->FindVar("Weapon_glock")->GetInt())
																engine->ServerCommand("give weapon_glock\n");
															if (cvar->FindVar("Weapon_grapple")->GetInt())
																engine->ServerCommand("give weapon_grapple\n");
															if (cvar->FindVar("Weapon_grenadelauncher")->GetInt())
																engine->ServerCommand("give weapon_grenadelauncher\n");
															if (cvar->FindVar("Weapon_hopwire")->GetInt())
																engine->ServerCommand("give weapon_hopwire\n");
															if (cvar->FindVar("Weapon_iceaxe")->GetInt())
																engine->ServerCommand("give weapon_iceaxe\n");
															if (cvar->FindVar("Weapon_immolator")->GetInt())
																engine->ServerCommand("give weapon_immolator\n");
															if (cvar->FindVar("Weapon_ionrifle")->GetInt())
																engine->ServerCommand("give weapon_ionrifle\n");
															if (cvar->FindVar("Weapon_knife")->GetInt())
																engine->ServerCommand("give weapon_knife\n");
															if (cvar->FindVar("Weapon_laser")->GetInt())
																engine->ServerCommand("give weapon_laser\n");
															if (cvar->FindVar("Weapon_laserpistol")->GetInt())
																engine->ServerCommand("give weapon_laserpistol\n");
															if (cvar->FindVar("Weapon_m4")->GetInt())
																engine->ServerCommand("give weapon_m4\n");
															if (cvar->FindVar("Weapon_machinegun")->GetInt())
																engine->ServerCommand("give weapon_machinegun\n");
															if (cvar->FindVar("Weapon_molotov")->GetInt())
																engine->ServerCommand("give weapon_molotov\n");
															if (cvar->FindVar("Weapon_mp5")->GetInt())
																engine->ServerCommand("give weapon_mp5\n");
															if (cvar->FindVar("Weapon_objective")->GetInt())
																engine->ServerCommand("give weapon_objective\n");
															if (cvar->FindVar("Weapon_oicw")->GetInt())
																engine->ServerCommand("give weapon_oicw\n");
															if (cvar->FindVar("Weapon_physgun")->GetInt())
																engine->ServerCommand("give weapon_physgun\n");
															if (cvar->FindVar("Weapon_pistol_silent")->GetInt())
																engine->ServerCommand("give weapon_pistol_silent\n");
															if (cvar->FindVar("Weapon_sg552")->GetInt())
																engine->ServerCommand("give weapon_sg552\n");
															if (cvar->FindVar("Weapon_shockrifle")->GetInt())
																engine->ServerCommand("give weapon_shockrifle\n");
															if (cvar->FindVar("Weapon_smg2")->GetInt())
																engine->ServerCommand("give weapon_smg2\n");
															if (cvar->FindVar("Weapon_smokegrenade")->GetInt())
																engine->ServerCommand("give weapon_smokegrenade\n");
															if (cvar->FindVar("Weapon_sniper")->GetInt())
																engine->ServerCommand("give weapon_sniper\n");
															if (cvar->FindVar("Weapon_teleport")->GetInt())
																engine->ServerCommand("give weapon_teleport\n");
															if (cvar->FindVar("Weapon_thumper")->GetInt())
																engine->ServerCommand("give weapon_thumper\n");
															if (cvar->FindVar("Weapon_wrench")->GetInt())
																engine->ServerCommand("give weapon_wrench\n");
															if (cvar->FindVar("grenade_ar2")->GetInt())
																engine->ServerCommand("give grenade_ar2\n");
															if (cvar->FindVar("Weapon_frag")->GetInt())
																engine->ServerCommand("give weapon_frag\n");
															if (cvar->FindVar("Weapon_bugbait")->GetInt())
																engine->ServerCommand("give weapon_bugbait\n");
															if (cvar->FindVar("Weapon_357")->GetInt())
															{
																DevMsg("Weapon_357\n");
																engine->ServerCommand("give weapon_357\n");
																//playerEnt->GiveNamedItem("Weapon_357");
															}
															if (cvar->FindVar("Weapon_pistol")->GetInt())
															{
																engine->ServerCommand("give weapon_pistol\n");
															}
															if (cvar->FindVar("Weapon_ar2")->GetInt())
																engine->ServerCommand("give weapon_ar2\n");
															if (cvar->FindVar("Weapon_smg1")->GetInt())
																engine->ServerCommand("give weapon_smg1\n");
															if (cvar->FindVar("Weapon_rpg")->GetInt())
																engine->ServerCommand("give weapon_rpg\n");
															if (cvar->FindVar("Weapon_shotgun")->GetInt())
																engine->ServerCommand("give weapon_shotgun\n");
															if (cvar->FindVar("Weapon_stunstick")->GetInt())
																engine->ServerCommand("give weapon_stunstick\n");
															if (cvar->FindVar("Weapon_crowbar")->GetInt())
																engine->ServerCommand("give weapon_crowbar\n");
															if (cvar->FindVar("Weapon_cubemap")->GetInt())
																engine->ServerCommand("give weapon_cubemap\n");
															if (cvar->FindVar("Weapon_crossbow")->GetInt())
																engine->ServerCommand("give weapon_crossbow\n");
															if (cvar->FindVar("Weapon_physcannon")->GetInt())
																engine->ServerCommand("give weapon_physcannon\n");
															if (cvar->FindVar("Weapon_alyxgun")->GetInt())
																engine->ServerCommand("give weapon_alyxgun\n");
															if (cvar->FindVar("Weapon_annabelle")->GetInt())
																engine->ServerCommand("give weapon_annabelle\n");

															if (cvar->FindVar("item_suit")->GetInt())
																engine->ServerCommand("give item_suit\n");


															cvar->FindVar("weapon_airboatgun")->SetValue(0);

															cvar->FindVar("weapon_ar3")->SetValue(0);

															cvar->FindVar("Weapon_ak47")->SetValue(0);

															cvar->FindVar("Weapon_autoshotgun")->SetValue(0);

															cvar->FindVar("Weapon_barnacle")->SetValue(0);

															cvar->FindVar("Weapon_beretta")->SetValue(0);

															cvar->FindVar("Weapon_binoculars")->SetValue(0);

															cvar->FindVar("Weapon_bee")->SetValue(0);

															cvar->FindVar("Weapon_adrenaline")->SetValue(0);

															cvar->FindVar("Weapon_cguard")->SetValue(0);

															cvar->FindVar("Weapon_deagle")->SetValue(0);

															cvar->FindVar("Weapon_disc")->SetValue(0);

															cvar->FindVar("Weapon_displacer")->SetValue(0);

															cvar->FindVar("Weapon_dual_pistols")->SetValue(0);

															cvar->FindVar("Weapon_egon")->SetValue(0);

															cvar->FindVar("Weapon_extinguisher")->SetValue(0);

															cvar->FindVar("Weapon_fireball")->SetValue(0);

															cvar->FindVar("Weapon_flamethrower")->SetValue(0);

															cvar->FindVar("Weapon_flaregun")->SetValue(0);

															cvar->FindVar("Weapon_gauss")->SetValue(0);

															cvar->FindVar("Weapon_glock")->SetValue(0);

															cvar->FindVar("Weapon_grapple")->SetValue(0);

															cvar->FindVar("Weapon_grenadelauncher")->SetValue(0);

															cvar->FindVar("Weapon_hopwire")->SetValue(0);

															cvar->FindVar("Weapon_iceaxe")->SetValue(0);

															cvar->FindVar("Weapon_immolator")->SetValue(0);

															cvar->FindVar("Weapon_ionrifle")->SetValue(0);

															cvar->FindVar("Weapon_knife")->SetValue(0);

															cvar->FindVar("Weapon_laser")->SetValue(0);

															cvar->FindVar("Weapon_laserpistol")->SetValue(0);

															cvar->FindVar("Weapon_m4")->SetValue(0);

															cvar->FindVar("Weapon_machinegun")->SetValue(0);

															cvar->FindVar("Weapon_molotov")->SetValue(0);

															cvar->FindVar("Weapon_mp5")->SetValue(0);

															cvar->FindVar("Weapon_objective")->SetValue(0);

															cvar->FindVar("Weapon_oicw")->SetValue(0);

															cvar->FindVar("Weapon_physgun")->SetValue(0);

															cvar->FindVar("Weapon_pistol_silent")->SetValue(0);

															cvar->FindVar("Weapon_sg552")->SetValue(0);

															cvar->FindVar("Weapon_shockrifle")->SetValue(0);

															cvar->FindVar("Weapon_smg2")->SetValue(0);

															cvar->FindVar("Weapon_smokegrenade")->SetValue(0);

															cvar->FindVar("Weapon_sniper")->SetValue(0);

															cvar->FindVar("Weapon_teleport")->SetValue(0);

															cvar->FindVar("Weapon_thumper")->SetValue(0);

															cvar->FindVar("Weapon_wrench")->SetValue(0);

															cvar->FindVar("grenade_ar2")->SetValue(0);

															cvar->FindVar("Weapon_frag")->SetValue(0);

															cvar->FindVar("Weapon_bugbait")->SetValue(0);

															cvar->FindVar("Weapon_357")->SetValue(0);

															cvar->FindVar("Weapon_pistol")->SetValue(0);

															cvar->FindVar("Weapon_ar2")->SetValue(0);

															cvar->FindVar("Weapon_smg1")->SetValue(0);

															cvar->FindVar("Weapon_rpg")->SetValue(0);

															cvar->FindVar("Weapon_shotgun")->SetValue(0);

															cvar->FindVar("Weapon_stunstick")->SetValue(0);

															cvar->FindVar("Weapon_crowbar")->SetValue(0);

															cvar->FindVar("Weapon_cubemap")->SetValue(0);

															cvar->FindVar("Weapon_crossbow")->SetValue(0);

															cvar->FindVar("Weapon_physcannon")->SetValue(0);

															cvar->FindVar("Weapon_alyxgun")->SetValue(0);

															cvar->FindVar("Weapon_annabelle")->SetValue(0);

															cvar->FindVar("item_suit")->SetValue(0);

															//CBaseCombatWeapon *name = "";//cvar->FindVar("last_weapon")->GetString();

															//playerEnt->Weapon_Switch(name);
															//playerEnt->wea;
															//cvar->FindVar("last_weapon")->GetString();

															
															playerEnt->Weapon_Switch(playerEnt->Weapon_GetLast());

															cvar->FindVar("oc_mapadd_changelevel")->SetValue(0);
														}





														if (pMapAddEntD->FindKey("break"))
											                break;


								}
								//}
							}
						}
					}
				}

				UTIL_Remove(this);
				break;
				//	}
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
			}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second
}

LINK_ENTITY_TO_CLASS(mapadd_save, CMapSave);
BEGIN_DATADESC(CMapSave)

DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
DEFINE_THINKFUNC(Think),

END_DATADESC()






#if 0

class CMapAddITrigger : public CMapAdd
{
public:
	DECLARE_CLASS(CMapAddITrigger, CMapAdd);
	DECLARE_DATADESC();


	// Constructor
	CMapAddITrigger(int MnEntIndex = 0);

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
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold
protected:
	int m_nEntIndexM;
};
LINK_ENTITY_TO_CLASS(instant_trig, CMapAddITrigger);

// Start of our data description for the class
BEGIN_DATADESC(CMapAddITrigger)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
DEFINE_KEYFIELD(labelName, FIELD_STRING, "label"),
// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC( FIELD_VOID, "RunLabel", InputRunLabel ),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_THINKFUNC(Think), // Register new think function

END_DATADESC()

CMapAddITrigger::CMapAddITrigger(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_bDeleteOnFire = true;
}
void CMapAddITrigger::Think()
{
	BaseClass::Think();
	
	if (!mapaddEnt)
		mapaddEnt = this->GetParent();

	//CBaseEntity *nearestEnt = gEntList.FindEntityInSphere(this, this->GetAbsOrigin(), m_nTriggerArea);

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
			if (ppEnts[i]->IsPlayer())
			{
				m_OnTrigger.FireOutput(ppEnts[i], this);

				//CMapAdd *mapaddEnt2 = GetMapAddEntity();// = dynamic_cast<CMapAdd *>(mapaddEnt);
				CMapAdd *mapaddEnt2 = dynamic_cast<CMapAdd *>(mapaddEnt);

				if (!mapaddEnt2)
				{
					char szMapadd[128];
					Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);

					char szLabelName[128];
					Q_snprintf(szLabelName, sizeof(szLabelName), "entities:%s", labelName);

					DevMsg("MapAdd found \n");
					mapaddEnt2 = CreateMapAddEntity();

					if (mapaddEnt2->RunTriggerLabel(szMapadd, szLabelName))
						UTIL_Remove(this);
				}
				if (mapaddEnt2)
				{
					UTIL_Remove(this);
				}

			}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second
	/*char szSSName[50];
	Q_snprintf(szSSName, sizeof(szSSName), "Trigger_%d", entindex());
	string_t iszSSName = AllocPooledString(szSSName);
	this->SetName(iszSSName);

	if (nearestEnt)
	{
		if (nearestEnt->IsPlayer())
		{
			CMapAdd *mapaddEnt2 = dynamic_cast< CMapAdd * >(mapaddEnt);

			if (!mapaddEnt2)
			{
				char szMapadd[128];
				Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);

				char szLabelName[128];
				Q_snprintf(szLabelName, sizeof(szLabelName), "entities:%s", labelName);

				DevMsg("MapAdd found \n");
				mapaddEnt2 = CreateMapAddEntity();

				if (mapaddEnt2->RunTriggerLabel(szMapadd, szLabelName))
					UTIL_Remove(this);
			}
			if (mapaddEnt2)
			{
				UTIL_Remove(this);
			}
		}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second*/
}
#endif

#if 0
//-----------------------------------------------------------------------------
// Purpose: Handle a tick input from another entity
//-----------------------------------------------------------------------------

class CMapAddITrigger : public CBaseEntity
{
public:
	DECLARE_CLASS(CMapAddITrigger, CBaseEntity);
	DECLARE_DATADESC();


	// Constructor
	CMapAddITrigger(int MnEntIndex = 0);

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
	float	m_nTriggerArea;	// Count at which to fire our output
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold
protected:
	int m_nEntIndexM;
};

/*CMapAddITrigger *CMapAddITrigger::Pointer()
{
	CMapAddITrigger *trig = (CMapAddITrigger *)CreateEntityByName("instant_trig");
	return trig;
}*/
CMapAddITrigger::CMapAddITrigger(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_bDeleteOnFire = true;
}

void CMapAddITrigger::ActionAdd(KeyValues *pMapAddEnt)
{
	DevMsg("Add Entity \n");
	Vector SpawnVector = Vector(0, 0, 0);
	QAngle SpawnAngle = QAngle(0, 0, 0);

	KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
	KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
	KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
	KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
	KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
	KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
	KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
	KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");

	if (pEntKeyOrig && pEntKeyAng)
	{
		const char *position = pMapAddEnt->GetString("origin", "0 0 0");
		UTIL_StringToVector(SpawnVector.Base(), position);
		float tmp[2];
		const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
		UTIL_StringToFloatArray(tmp, 2, angle);
		SpawnAngle[PITCH] = tmp[0];
		SpawnAngle[YAW] = tmp[1];
		SpawnAngle[ROLL] = tmp[2];
	}
	if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
	{
		SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
		SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
		SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

		SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
		SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
		SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
	}

	CBaseEntity *createEnt = CBaseEntity::CreateNoSpawn(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);

	KeyValues *pEntKeyLongRange = pMapAddEnt->FindKey("longrange");
	KeyValues *pEntKeyFreeze = pMapAddEnt->FindKey("freeze");
	KeyValues *pEntKeyVel = pMapAddEnt->FindKey("velocity");

	KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
	KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
	if (createEnt)
	{
		if (pEntKeyLongRange)
			createEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);
		if (pEntKeyFreeze)
		{
			createEnt->VPhysicsInitStatic();
			createEnt->SetMoveType(MOVETYPE_PUSH);
		}
		if (pEntKeyVel)
		{
			Vector VelVector = Vector(0, 0, 0);
			const char *vel = pMapAddEnt->GetString("velocity", "0 0 0");
			UTIL_StringToVector(VelVector.Base(), vel);
			createEnt->SetAbsVelocity(VelVector);
		}

		if (pEntKeyValues)
		{
			DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
			KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
			while (pEntKeyValuesAdd && createEnt)
			{
				if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
				{
					PrecacheModel(pEntKeyValuesAdd->GetString(""));
					createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
				}
				else
				{
					createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
				}
				pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
			}
		}

		if (pEntFlags)
		{
			DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
			KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
			while (pEntFlagsAdd && createEnt)
			{
				createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
				pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
			}
		}
	}
	//createEnt->Activate();//Is this a good idea? Not sure!
	//createEnt->Spawn();
	DispatchSpawn(createEnt); //I derped
}
void CMapAddITrigger::ActionMod(KeyValues *pMapAddEnt)
{
	DevMsg("pName is %s \n", pMapAddEnt->GetName());
	const char *pName = pMapAddEnt->GetName();
	DevMsg("pName is %s \n", pName);

	CBaseEntity *modEnt = gEntList.FindEntityGenericWithin(NULL, pName, this->GetAbsOrigin(), MAX_TRACE_LENGTH);

	//CBaseEntity *modEnt = gEntList.FindEntityByName(this, AllocPooledString(pMapAddEnt->GetName()));

	if (modEnt)
	{
		DevMsg("modEnt is %s \n", pName);
		DevMsg("modEnt is %s \n", modEnt->GetEntityName());
	}

	KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
	KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
	if (modEnt)
	{
		DevMsg("modEnt func \n");
		if (pEntKeyValues)
		{
			DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
			KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
			while (pEntKeyValuesAdd && modEnt)
			{
				if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
				{
					PrecacheModel(pEntKeyValuesAdd->GetString(""));
					modEnt->SetModel(pEntKeyValuesAdd->GetString(""));
				}
				else
				{
					modEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
				}
				pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
			}
		}

		if (pEntFlags)
		{
			DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
			KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
			while (pEntFlagsAdd && modEnt)
			{
				modEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
				pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
			}
		}
	}
	//createEnt->Activate();//Is this a good idea? Not sure!
	//createEnt->Spawn();
	//DispatchSpawn(createEnt); //I derped

}
void CMapAddITrigger::Think()
{
	BaseClass::Think(); // Always do this if you override Think()

	CBaseEntity *nearestEnt = gEntList.FindEntityInSphere(this, GetAbsOrigin(), m_nTriggerArea);

	char szSSName[50];
	Q_snprintf(szSSName, sizeof(szSSName), "Trigger_%d", entindex());
	string_t iszSSName = AllocPooledString(szSSName);
	this->SetName(iszSSName);
	
	//int i;
	//for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (nearestEnt)//(ppEnts[i] != NULL)
			if (nearestEnt->IsPlayer())
			{
				Msg("mapadd_trig was triggered!\n");
				m_OnTrigger.FireOutput(nearestEnt, this);
				//	if(m_bDeleteOnFire)
				//	{
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
				//UTIL_Remove(this);

				char szMapadd[128];
				Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);

				KeyValues *pMapAdd = new KeyValues("MapAdd");


				if (pMapAdd->LoadFromFile(filesystem, szMapadd, "MOD"))
				{
					KeyValues *pMapAddEntP = pMapAdd->FindKey("Init");
					if (pMapAddEntP)
					{
						KeyValues *pMapAddEntS = pMapAddEntP->FindKey("instant_trig");
						if (pMapAddEntS)
						{

							DevMsg("KeyValues(mapadd_trig) \n");
							KeyValues *pMapAddEntD = pMapAddEntS->FindKey("AddEntity");
							if (pMapAddEntS && pMapAddEntD)
							{

								DevMsg("AddEntity \n");
								//KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
								KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
								while (pMapAddEntD)//if (pMapAddEnt)
								{
									if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("remove:sphere"))
									{
										DevMsg("remove:sphere \n");
										GetMapAddEntity()->HandleRemoveEnitity(pMapAddEnt);
									}
									DevMsg("%s \n", pMapAddEntD->GetName());

									ActionAdd(pMapAddEnt);

									pMapAddEnt = pMapAddEnt->GetNextKey();

									if (pMapAddEnt == NULL)
										break;
									if (pMapAddEnt->GetNextKey() == NULL)
										break;

									if (pMapAddEnt->IsEmpty())
										break;

									if (pMapAddEnt->FindKey("break"))
										break;
								}

							}

							KeyValues *pMapAddEntM = pMapAddEntS->FindKey("ModifyEntity");
							if (pMapAddEntS && pMapAddEntM)
							{
								DevMsg("ModifyEntity \n");
								KeyValues *pMapAddEnt = pMapAddEntM->GetFirstTrueSubKey();
								while (pMapAddEntM)
								{
									DevMsg("%s \n", pMapAddEntM->GetName());

									ActionMod(pMapAddEnt);

									pMapAddEnt = pMapAddEnt->GetNextKey();

									if (pMapAddEnt == NULL)
										break;
									if (pMapAddEnt->GetNextKey() == NULL)
										break;

									if (pMapAddEnt->IsEmpty())
										break;

									if (pMapAddEnt->FindKey("break"))
										break;
								}

							}



						}
					}
				}

				UTIL_Remove(this);
				//break;
				//	}
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
			}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second
}

LINK_ENTITY_TO_CLASS(instant_trig, CMapAddITrigger);

// Start of our data description for the class
BEGIN_DATADESC(CMapAddITrigger)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC( FIELD_VOID, "RunLabel", InputRunLabel ),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_THINKFUNC(Think), // Register new think function

END_DATADESC()


#endif









//-----------------------------------------------------------------------------
// Purpose: Handle a tick input from another entity
//-----------------------------------------------------------------------------

class CMapAddLabel : public CBaseEntity
{
public:
	DECLARE_CLASS(CMapAddLabel, CBaseEntity);
	DECLARE_DATADESC();


	// Constructor
	CMapAddLabel(int MnEntIndex = 0);

	void Spawn()
	{
		BaseClass::Spawn();
		SetNextThink(gpGlobals->curtime); // Think now
	}
	// Input function
	//void InputRunLabel( inputdata_t &inputData ); //Runmapadd
	void Think();
private:
	float	m_nTriggerArea;	// Count at which to fire our output
	//	int	m_nCounter;	// Internal counter
	bool	m_bDeleteOnFire;
	COutputEvent	m_OnTrigger;	// Output event when the counter reaches the threshold
protected:
	int m_nEntIndexM;
};

CMapAddLabel::CMapAddLabel(int MnEntIndex)
{
	m_nEntIndexM = MnEntIndex;
	m_nTriggerArea = 32.0f;
	m_bDeleteOnFire = true;
}
void CMapAddLabel::Think()
{
	BaseClass::Think(); // Always do this if you override Think()



	//What this does, is look through entities in a sphere, and then
	//checks to see if they are valid, and if they are
	//adds them to a second list of valid entities.
	//Create an array of CBaseEntity pointers
	CBaseEntity *ppEnts[256];
	int nEntCount = UTIL_EntitiesInSphere(ppEnts, 256, this->GetAbsOrigin(), m_nTriggerArea*2, 0);
	int i;
	for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (ppEnts[i] != NULL)
			if (ppEnts[i]->IsPlayer())
			{
				Msg("mapadd_trig was triggered!\n");
				m_OnTrigger.FireOutput(ppEnts[i], this);
				//	if(m_bDeleteOnFire)
				//	{
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
				//UTIL_Remove(this);
				
				char szMapadd[128];
				Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", gpGlobals->mapname);
				//KeyValues *pMapAdd = new KeyValues("AddEntity");
				KeyValues *pMapAdd = new KeyValues("MapAdd");


				if (pMapAdd->LoadFromFile(filesystem, szMapadd, "MOD"))
				{
						KeyValues *pMapAddEntP = pMapAdd->FindKey("Init");
						if (pMapAddEntP)
						{							
							KeyValues *pMapAddEntS = pMapAddEntP->FindKey("mapadd_trig");
							if (pMapAddEntS)
							{
									DevMsg("KeyValues(mapadd_trig)");
									KeyValues *pMapAddEntD = pMapAddEntS->FindKey("AddEntity");
									if (pMapAddEntS && pMapAddEntD)
									{

										DevMsg("KeyValues(AddEntity)");
										//KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
										KeyValues *pMapAddEnt = pMapAddEntD->GetFirstTrueSubKey();
										while (pMapAddEntD)//if (pMapAddEnt)
										{
											if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("remove:sphere"))
											{
												DevMsg("remove:sphere \n");
												GetMapAddEntity()->HandleRemoveEnitity(pMapAddEnt);
											}

											Vector SpawnVector = Vector(0, 0, 0);
											QAngle SpawnAngle = QAngle(0, 0, 0);

											KeyValues *pEntKeyOrig = pMapAddEnt->FindKey("origin");
											KeyValues *pEntKeyAng = pMapAddEnt->FindKey("angle");
											KeyValues *pEntKeyX = pMapAddEnt->FindKey("x");
											KeyValues *pEntKeyY = pMapAddEnt->FindKey("y");
											KeyValues *pEntKeyZ = pMapAddEnt->FindKey("z");
											KeyValues *pEntKeyRoll = pMapAddEnt->FindKey("roll");
											KeyValues *pEntKeyYaw = pMapAddEnt->FindKey("yaw");
											KeyValues *pEntKeyPitch = pMapAddEnt->FindKey("pitch");

											if (pEntKeyOrig && pEntKeyAng)
											{
												const char *position = pMapAddEnt->GetString("origin", "0 0 0");
												UTIL_StringToVector(SpawnVector.Base(), position);
												float tmp[2];
												const char *angle = pMapAddEnt->GetString("angle", "0 0 0");
												UTIL_StringToFloatArray(tmp, 2, angle);
												SpawnAngle[PITCH] = tmp[0];
												SpawnAngle[YAW] = tmp[1];
												SpawnAngle[ROLL] = tmp[2];
											}
											if (pEntKeyX && pEntKeyY && pEntKeyZ && pEntKeyRoll && pEntKeyYaw && pEntKeyPitch)
											{
												SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
												SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
												SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

												SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
												SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
												SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);
											}

											CBaseEntity *createEnt = CBaseEntity::CreateNoSpawn(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);

											KeyValues *pEntKeyLongRange = pMapAddEnt->FindKey("longrange");
											KeyValues *pEntKeyFreeze = pMapAddEnt->FindKey("freeze");
											KeyValues *pEntKeyVel = pMapAddEnt->FindKey("velocity");

											KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
											KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
											if (createEnt)
											{
												if (pEntKeyLongRange)
													createEnt->AddSpawnFlags(SF_NPC_LONG_RANGE);
												if (pEntKeyFreeze)
												{
													createEnt->VPhysicsInitStatic();
													createEnt->SetMoveType(MOVETYPE_PUSH);
												}
												if (pEntKeyVel)
												{
													Vector VelVector = Vector(0, 0, 0);
													const char *vel = pMapAddEnt->GetString("velocity", "0 0 0");
													UTIL_StringToVector(VelVector.Base(), vel);
													createEnt->SetAbsVelocity(VelVector);
												}

												if (pEntKeyValues)
												{
													DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
													KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
													while (pEntKeyValuesAdd && createEnt)
													{
														if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
														{
															PrecacheModel(pEntKeyValuesAdd->GetString(""));
															createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
														}
														else
														{
															createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
														}
														pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
													}
												}

												if (pEntFlags)
												{
													DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
													KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
													while (pEntFlagsAdd && createEnt)
													{
														createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
														pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
													}
												}
											}
											//createEnt->Activate();//Is this a good idea? Not sure!
											//createEnt->Spawn();
											DispatchSpawn(createEnt); //I derped


											pMapAddEnt = pMapAddEnt->GetNextKey();//pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!
											if (pMapAddEnt == NULL)
												break;
											if (pMapAddEnt->GetNextKey() == NULL)
												break;

											if (pMapAddEnt->IsEmpty())
												break;

											if (pMapAddEnt->FindKey("break"))
												break;
										}

									}




									KeyValues *pMapAddEntM = pMapAddEntS->FindKey("ModifyEntity");
									if (pMapAddEntS && pMapAddEntM)
									{
										DevMsg("KeyValues(ModifyEntity)");
										KeyValues *pMapAddEnt = pMapAddEntM->GetFirstTrueSubKey();
										while (pMapAddEntM)
										{
											/*if (AllocPooledString(pMapAddEnt->GetName()) == AllocPooledString("remove:sphere"))
											{
												DevMsg("remove:sphere \n");
												GetMapAddEntity()->HandleRemoveEnitity(pMapAddEnt);
											}*/

											/*CBaseEntity *createEnt = CBaseEntity::CreateNoSpawn(pMapAddEnt->GetName(), SpawnVector, SpawnAngle);*/

											DevMsg("pName is %s \n", pMapAddEnt->GetName());
											const char *pName = pMapAddEnt->GetName();
											DevMsg("pName is %s \n", pName);

											CBaseEntity *modEnt = gEntList.FindEntityGenericWithin(NULL, pName, this->GetAbsOrigin(), MAX_TRACE_LENGTH);
											
											//CBaseEntity *modEnt = gEntList.FindEntityByName(this, AllocPooledString(pMapAddEnt->GetName()));

											if (modEnt)
											{
												DevMsg("modEnt is %s \n", pName);
												DevMsg("modEnt is %s \n", modEnt->GetEntityName());
											}

											KeyValues *pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
											KeyValues *pEntFlags = pMapAddEnt->FindKey("Flags");
											if (modEnt)
											{
												DevMsg("modEnt func \n");
												if (pEntKeyValues)
												{
													DevMsg("KeyValue for %s Found!\n", pMapAddEnt->GetName());
													KeyValues *pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
													while (pEntKeyValuesAdd && modEnt)
													{
														if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
														{
															PrecacheModel(pEntKeyValuesAdd->GetString(""));
															modEnt->SetModel(pEntKeyValuesAdd->GetString(""));
														}
														else
														{
															modEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
														}
														pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
													}
												}

												if (pEntFlags)
												{
													DevMsg("Flag for %s Found!\n", pMapAddEnt->GetName());
													KeyValues *pEntFlagsAdd = pEntFlags->GetFirstValue();
													while (pEntFlagsAdd && modEnt)
													{
														modEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
														pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
													}
												}
											}
											//createEnt->Activate();//Is this a good idea? Not sure!
											//createEnt->Spawn();
											//DispatchSpawn(createEnt); //I derped

											pMapAddEnt = pMapAddEnt->GetNextKey();//pMapAddEnt->GetNextTrueSubKey(); //Got to keep this!
											if (pMapAddEnt == NULL)
												break;
											if (pMapAddEnt->GetNextKey() == NULL)
												break;

											if (pMapAddEnt->IsEmpty())
												break;

											if (pMapAddEnt->FindKey("break"))
												break;
										}

									}



							}
					}
				}
				
				UTIL_Remove(this);
				break;
				//	}
				//	SetNextThink( gpGlobals->curtime + 1 ); // Think again in 1 second
			}
	}
	SetNextThink(gpGlobals->curtime + 0.3f); // Think again in 1 second
}

LINK_ENTITY_TO_CLASS(mapadd_trig, CMapAddLabel);

// Start of our data description for the class
BEGIN_DATADESC(CMapAddLabel)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_nTriggerArea, FIELD_FLOAT, "radius"),
// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_bDeleteOnFire, FIELD_BOOLEAN, "deleteonfire"),
// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC( FIELD_VOID, "RunLabel", InputRunLabel ),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

DEFINE_THINKFUNC(Think), // Register new think function

END_DATADESC()
