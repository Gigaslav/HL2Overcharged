// (More) Reallistic simulated bullets
//
// This code is originally based from article on Valve Developer Community
// The original code you can find by this link:
// http://developer.valvesoftware.com/wiki/Simulated_Bullets

// NOTENOTE: Tested only on localhost. There maybe a latency errors and others
// NOTENOTE: The simulation might be strange.

#include "cbase.h"
#include "util_shared.h"
#include "bullet_manager_shotgun.h"
#include "effect_dispatch_data.h"
#include "tier0/vprof.h"
#include "decals.h"


#include "ai_debug_shared.h"
#include "baseentity_shared.h"
#include "IEffects.h"


#ifdef CLIENT_DLL

#else
#include "vehicle_base.h"
#include "func_break.h"
#include "waterbullet.h"
#include "WaterBulletDynamic.h"
#endif

CBulletManagerShotgun *g_pBulletManagerShotgun;
CUtlLinkedList<CSimulatedBulletShotgun*> g_BulletsShotgun;

#ifdef CLIENT_DLL//-------------------------------------------------
#include "engine/ivdebugoverlay.h"
#include "c_te_effect_dispatch.h"
ConVar g_debug_client_bullets_shotgun("g_debug_client_bullets_shotgun", "0", FCVAR_CHEAT);
extern void FX_PlayerTracer(Vector& start, Vector& end);
#else//-------------------------------------------------------------
#include "te_effect_dispatch.h"
#include "soundent.h"
#include "player_pickup.h"
#include "ilagcompensationmanager.h"
ConVar g_debug_bullets_shotgun("g_debug_bullets_shotgun", "0", FCVAR_CHEAT, "Debug of bullet simulation\nThe white line shows the bullet trail.\nThe red line shows not passed penetration test.\nThe green line shows passed penetration test. Turn developer mode for more information.");
#endif//------------------------------------------------------------

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_RICO_DOT_ANGLEShotgun 0.15f  //Maximum dot allowed for any ricochet
#define MIN_RICO_SPEED_PERCShotgun 0.55f //Minimum speed percent allowed for any ricochet


static void BulletSpeedModifierCallbackShotgun(ConVar *var, const char *pOldString)
{
if (cvar->FindVar("sv_bullet_speed_modifier_shotgun")->GetFloat() == 0.0f)//var->GetFloat() == 0.0f) //To avoid math exception
cvar->FindVar("sv_bullet_speed_modifier_shotgun")->Revert();// var->Revert();
}
ConVar sv_bullet_speed_modifier_shotgun("sv_bullet_speed_modifier_shotgun", "100.000000", (FCVAR_ARCHIVE | FCVAR_REPLICATED),
"Density/(This Value) * (Distance Penetrated) = (Change in Speed)",
((FnChangeCallback_t)BulletSpeedModifierCallbackShotgun));

static void UnrealRicochetCallbackShotgun(ConVar *var, const char *pOldString)
{
if (gpGlobals->maxClients > 1)
{
cvar->FindVar("sv_bullet_unrealricochet_shotgun")->Revert(); //var->Revert();
Msg("Cannot use unreal ricochet in multiplayer game\n");
}

if (cvar->FindVar("sv_bullet_unrealricochet_shotgun")->GetBool())//var->GetBool()) //To avoid math exception
Warning("\nWarning! Enabling unreal ricochet may cause the game crash.\n\n");
}
ConVar sv_bullet_unrealricochet_shotgun("sv_bullet_unrealricochet_shotgun", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Unreal ricochet", ((FnChangeCallback_t)UnrealRicochetCallbackShotgun));



static void BulletStopSpeedCallbackShotgun(ConVar *var, const char *pOldString)
{
int val = cvar->FindVar("sv_bullet_stop_speed_shotgun")->GetInt();//var->GetInt();
if (val<ONE_HIT_MODEShotgun)
cvar->FindVar("sv_bullet_stop_speed_shotgun")->Revert(); //var->Revert();
else if (BulletManagerShotgun())
BulletManagerShotgun()->UpdateBulletStopSpeedShotgun();
}
ConVar sv_bullet_stop_speed_shotgun("sv_bullet_stop_speed_shotgun", "40", FCVAR_REPLICATED,
"Integral speed at which to remove the bullet from the bullet queue\n-1 is butter mode\n-2 is 1 hit mode",
((FnChangeCallback_t)BulletStopSpeedCallbackShotgun));



LINK_ENTITY_TO_CLASS(bullet_manager_shotgun, CBulletManagerShotgun);



//==================================================
// Purpose:	Constructor
//==================================================
CSimulatedBulletShotgun::CSimulatedBulletShotgun()
{
	m_vecOrigin.Init();
	m_vecDirShooting.Init();
	m_flInitialBulletSpeed = m_flBulletSpeed = 0;
	m_flEntryDensity = 0.0f;
	bStuckInWall = false;
	isUnderWaterShotgun = false;

	m_iDamageType = 2;
}

//==================================================
// Purpose:	Constructor
//==================================================
CSimulatedBulletShotgun::CSimulatedBulletShotgun(FireBulletsInfo_t info, int nDamageType, int nAmmoFlags, Vector newdir, CBaseEntity *pInfictor, CBaseEntity *pAdditionalIgnoreEnt,
	bool bTraceHull
#ifndef CLIENT_DLL
	, CBaseEntity *pCaller
#endif
	)
{
	// Input validation
	Assert(pInfictor);
#ifndef CLIENT_DLL
	Assert(pCaller);
#endif

/*#ifndef CLIENT_DLL	
	if (gEntList.FindEntityByClassname(g_pBulletManagerShotgun, g_pBulletManagerShotgun->GetClassname()) != NULL)
		delete gEntList.FindEntityByClassname(g_pBulletManagerShotgun, g_pBulletManagerShotgun->GetClassname());
#endif*/

	bulletinfo = info;			// Setup Fire bullets information here

	p_eInfictor = pInfictor;	// Setup inflictor

	isPlayer = p_eInfictor->IsPlayer();
	// Create a list of entities with which this bullet does not collide.
	m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
	//Assert(m_pIgnoreList);

	bStuckInWall = false;

	// Don't collide with the entity firing the bullet.
	m_pIgnoreList->AddEntityToIgnore(p_eInfictor);

	// Don't collide with some optionally-specified entity.
	if (pAdditionalIgnoreEnt != NULL)
		m_pIgnoreList->AddEntityToIgnore(pAdditionalIgnoreEnt);

	m_iDamageType = nDamageType;// GetAmmoDef()->DamageType(bulletinfo.m_iAmmoType);

	/*if (p_eInfictor->IsNPC())
		IsPlayer = false;// 
	else if (p_eInfictor->IsPlayer())
		IsPlayer = true;*/

	AmmoType = bulletinfo.m_iAmmoType;
	bulletinfo.m_flDamageForceScale = bulletinfo.m_flDamageForceScale * 7.5;////////////////
	pInfictorRemember = p_eInfictor;
	flActualForce = bulletinfo.m_flDamageForceScale;
	//m_szTracerName = (char*)p_eInfictor->GetTracerType(); 

	// Basic information about the bullet
	m_flInitialBulletSpeed = m_flBulletSpeed = GetAmmoDef()->GetAmmoOfIndex(bulletinfo.m_iAmmoType)->bulletSpeed;
	m_vecDirShooting = newdir;

	if (isPlayer)
	{
		if (p_eInfictor->GetWaterLevel() == 3)
		{
			m_vecOrigin = pInfictor->EyePosition();
		}
		else
		{
#ifndef CLIENT_DLL
			CBasePlayer *pPlayer = ToBasePlayer(p_eInfictor);
			if (pPlayer)
			{
				if (pPlayer->IsInControl)
					m_vecOrigin = bulletinfo.m_vecSrc;
				else
					m_vecOrigin = pInfictor->EyePosition();
			}
			else
				m_vecOrigin = pInfictor->EyePosition();
				
#endif
			//m_vecOrigin = pInfictor->EyePosition();
			//m_vecOrigin = bulletinfo.m_vecSrc;
		}
	}
	else
	{
		m_vecOrigin = bulletinfo.m_vecSrc;
	}

	m_bTraceHull = bTraceHull;
#ifndef CLIENT_DLL
	m_hCaller = pCaller;
#endif
	m_flEntryDensity = 0.0f;
	m_vecTraceRay = m_vecOrigin + m_vecDirShooting * m_flBulletSpeed;
	m_flRayLength = m_flInitialBulletSpeed;


	Vector m_vecTraceRay2;
	m_vecTraceRay2 = m_vecOrigin + m_vecDirShooting * info.m_flDistance;
	g_pBulletManagerShotgun->EndPoint = m_vecTraceRay2;
	EndPoint = m_vecTraceRay2;

	trace_t tr;
	if (m_bTraceHull)
		AI_TraceHull(m_vecOrigin, m_vecTraceRay2, Vector(-3, -3, -3), Vector(3, 3, 3), MASK_SHOT_HULL, m_pIgnoreList, &tr);
	else
		AI_TraceLine(m_vecOrigin, m_vecTraceRay2, MASK_SHOT, m_pIgnoreList, &tr);
	trEnd = tr;

	m_vecDirShootingRemember = bulletinfo.m_vecDirShooting;

	int iPlayerDamage = bulletinfo.m_iPlayerDamage;
	pAmmoDef = GetAmmoDef();

	flActualDamage = bulletinfo.m_flDamage;

	//if (tr.m_pEnt)
	{
		if (isPlayer)
		{
			flActualDamage = iPlayerDamage = pAmmoDef->PlrDamage(bulletinfo.m_iAmmoType);
		}
#ifdef GAME_DLL
		else if (p_eInfictor->GetServerVehicle())
		{
			IServerVehicle *pVehicle = p_eInfictor->GetServerVehicle();
			if (pVehicle != NULL)
			{
				CBaseCombatCharacter *pDriver = pVehicle->GetPassenger();
				if (pDriver && pDriver->IsPlayer())
				{
					flActualDamage = iPlayerDamage;
				}
			}
			//CPropVehicle *pVehicle = reinterpret_cast<CPropVehicle*>(tr.m_pEnt->GetServerVehicle());
		}
#endif
		else
			flActualDamage = iPlayerDamage = pAmmoDef->NPCDamage(bulletinfo.m_iAmmoType);
	}

	int nActualDamageType = nDamageType;
	if (flActualDamage == 0.0)
	{
		flActualDamage = g_pGameRules->GetAmmoDamage(p_eInfictor, tr.m_pEnt, info.m_iAmmoType);
	}
	else
	{
		nActualDamageType = nDamageType | ((flActualDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB);
	}

	//if (tr.m_pEnt != NULL)
	//flActualDamage = pFireBulletsInfo->m_flDamage;//g_pGameRules->GetAmmoDamage(p_eInfictor, tr.m_pEnt, pFireBulletsInfo->m_iAmmoType);

	m_vecDirShootingRemember = bulletinfo.m_vecDirShooting;

	info2 = new CTakeDamageInfo(pInfictorRemember, bulletinfo.m_pAttacker, flActualDamage, GetDamageType());

	//CTakeDamageInfo dmgInfo(pInfictorRemember, pFireBulletsInfo->m_pAttacker, flActualDamage, GetDamageType());
	CalculateBulletDamageForce(info2, AmmoType, m_vecDirShooting, tr.endpos);
	info2->ScaleDamageForce(flActualForce);
	info2->SetAmmoType(AmmoType);
	
	isUnderWaterShotgun = false;
}



//==================================================
// Purpose:	Deconstructor
//==================================================
CSimulatedBulletShotgun::~CSimulatedBulletShotgun()
{
	delete m_pIgnoreList;
	m_pIgnoreList = NULL;

	delete info2;
	info2 = NULL;
}
//==================================================
// Purpose:	Simulates a bullet through a ray of its bullet speed
//==================================================
bool CSimulatedBulletShotgun::SimulateBulletShotgun(void)
{
	VPROF("C_SimulatedBulletShotgun::SimulateBulletShotgun");

	if (!IsFinite(m_flBulletSpeed))
		return false;		 //prevent a weird crash

	trace_t trace;
	trace = trEnd;

	if (m_flBulletSpeed <= 0) //Avoid errors;
		return false;

	if (!p_eInfictor)
	{
		p_eInfictor = pInfictorRemember;//bulletinfo.m_pAttacker;

		if (!p_eInfictor)
			return false;
	}

	m_flRayLength = m_flBulletSpeed;
	if (!isUnderWaterShotgun)
	{
		m_flBulletSpeed += 0.8f * m_vecDirShooting.z; //TODO: Bullet mass
	}
	/*else
	{
		m_flBulletSpeed *= 0.4f; //TODO: Bullet mass
	}*/

	//m_flBulletSpeed += 0.8f * m_vecDirShooting.z; //TODO: Bullet mass
	m_vecTraceRay = m_vecOrigin + m_vecDirShooting * m_flBulletSpeed;

	if (m_flBulletSpeed <= 0.f)
		return false;
	//m_vecDirShooting.z -= 0.1 / m_flBulletSpeed;//Bullet mass

#ifdef GAME_DLL
	if (bulletinfo.m_flLatency != 0)
	{
		m_vecTraceRay *= bulletinfo.m_flLatency * 100;
	}
#endif

	if (!IsInWorldShotgun())
	{
		return false;
	}

	if (bStuckInWall)
		return false;

	if ((pInfictorRemember) && (enginetrace->GetPointContents(m_vecOrigin) & (CONTENTS_WATER | CONTENTS_SLIME)) && m_flBulletSpeed != 0.f)//(!m_bWasInWater)
	{
		/*if (m_flBulletSpeed >= m_flInitialBulletSpeed * 0.7f)
			m_flBulletSpeed = m_flBulletSpeed * 0.9f;*/

		UTIL_Bubbles(m_vecOrigin, m_vecOrigin, 3);

		if (!isUnderWaterShotgun && pInfictorRemember->GetWaterLevel() != 3)
			HandleShotImpactingWaterShotgun(bulletinfo, m_vecOrigin, m_vecOrigin, m_pIgnoreList, &trEnd.endpos, isPlayer);
	}


	bool bulletSpeedCheck;
	bulletSpeedCheck = false;

	//MikeD UTIL_TraceLine or AI_TraceLine affects to SendTraceAttackToTriggers, so be careful
	if (m_bTraceHull)
		AI_TraceHull(m_vecOrigin, m_vecTraceRay, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SHOT_HULL, m_pIgnoreList, &trace);
	else
		AI_TraceLine(m_vecOrigin, m_vecTraceRay, MASK_SHOT, m_pIgnoreList, &trace);

#ifdef CLIENT_DLL
	if (g_debug_client_bullets_shotgun.GetBool())
	{
	debugoverlay->AddLineOverlay(trace.startpos, trace.endpos, 255, 0, 0, true, 10.0f);
	}
#else
	if (g_debug_bullets_shotgun.GetBool())
	{
	NDebugOverlay::Line(trace.startpos, trace.endpos, 255, 255, 255, true, 10.0f);
	}

	// Now hit all triggers along the ray that respond to shots...
	// Clip the ray to the first collided solid returned from traceline
	//if (bulletinfo.m_pAttacker != NULL)
	{
		/*CTakeDamageInfo triggerInfo(p_eInfictor, bulletinfo.m_pAttacker, bulletinfo.m_flDamage, GetDamageTypeShotgun());
		CalculateBulletDamageForce(&triggerInfo, bulletinfo.m_iAmmoType, m_vecDirShooting, trace.endpos);
		triggerInfo.ScaleDamageForce(bulletinfo.m_flDamageForceScale);
		triggerInfo.SetAmmoType(bulletinfo.m_iAmmoType);*/
		BulletManagerShotgun()->SendTraceAttackToTriggersShotgun(*info2, trace.startpos, trace.endpos, m_vecDirShooting);
	}
#endif/*
	
	// Now hit all triggers along the ray that respond to shots...
	// Clip the ray to the first collided solid returned from traceline

	////////////////////////////p_eInfictor/////////////bulletinfo.m_pAttacker////////////////////////
	if (p_eInfictor != NULL)
	{
		CTakeDamageInfo triggerInfo(p_eInfictor, bulletinfo.m_pAttacker, bulletinfo.m_flDamage, GetDamageTypeShotgun());
	CalculateBulletDamageForce(&triggerInfo, bulletinfo.m_iAmmoType, m_vecDirShootingShotgun, trace.endpos);
	triggerInfo.ScaleDamageForce(bulletinfo.m_flDamageForceScale);
	triggerInfo.SetAmmoType(bulletinfo.m_iAmmoType);
	BulletManagerShotgun()->SendTraceAttackToTriggersShotgun(triggerInfo, trace.startpos, trace.endpos, m_vecDirShootingShotgun);
	}

#endif*/

	if (trace.fraction == 1.0f)
	{
		m_vecOrigin += m_vecDirShooting *m_flBulletSpeed; //Do the WAY

		/*CEffectData data;
		data.m_vStart = trace.StartPos;
		data.m_vOrigin = trace.endpos;
		data.m_nDamageType = GetDamageTypeShotgun();

		DispatchEffect("RagdollImpact", data);*/

		return  true;
	}
	else
	{


		if (trace.m_pEnt == p_eInfictor) //HACK: Remove bullet if we hit self (for frag grenades)
			return false;


		//if (this != NULL && p_eInfictor != NULL)
		EntityImpactShotgun(trace);

		if (!(trace.surface.flags&SURF_SKY))
		{
		/*if (!m_bWasInWater)
		{
			//if ((trace.m_pEnt == NULL))
			UTIL_ImpactTrace(&trace, NULL);

			delete m_pIgnoreListShotgun;
			return false;
			//UTIL_Remove(g_pBulletManagerShotgun);
		}*/


			if (trace.allsolid)//in solid
			{
				if (!AllSolidShotgun(trace))
					return false;

				m_vecOrigin += m_vecDirShooting * m_flBulletSpeed; //Do the WAY

				bulletSpeedCheck = true;
			}
			else if (trace.fraction != 1.0f)//hit solid
			{
				if (!EndSolidShotgun(trace))
				return false;

				bulletSpeedCheck = true;
			}
			else if (trace.startsolid)//exit solid
			{
				if (!StartSolidShotgun(trace))
				return false;

				m_vecOrigin += m_vecDirShooting * m_flBulletSpeed; //Do the WAY

				bulletSpeedCheck = true;
			}
			else
			{
				//don't do a bullet speed check for not touching anything
			}
		}
		else
		{
			return false; //Through sky? No.
		}
	}

	/*if (sv_bullet_unrealricochet_shotgun.GetBool()) //Fun bullet ricochet fix
	{
	delete m_pIgnoreListShotgun; //Prevent causing of memory leak
	m_pIgnoreListShotgun = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
	}*/

	if (bulletSpeedCheck)
	{
		if (m_flBulletSpeed <= BulletManagerShotgun()->BulletStopSpeedShotgun())
		{
			return false;
		}
	}

	return true;
}


//==================================================
// Purpose:	Simulates when a solid is exited
//==================================================
bool CSimulatedBulletShotgun::StartSolidShotgun(trace_t &ptr)
{
	switch (BulletManagerShotgun()->BulletStopSpeedShotgun())
	{
	case BUTTER_MODEShotgun:
	{
		//Do nothing to bullet speed
		return true;
	}
	case ONE_HIT_MODEShotgun:
	{
		return false;
	}
	default:
	{
		float flPenetrationDistance = VectorLength(AbsEntry - AbsExit);

		m_flBulletSpeed -= flPenetrationDistance * m_flEntryDensity / sv_bullet_speed_modifier_shotgun.GetFloat();
		return true;
	}
	}
	return true;
	//return true;
}


//==================================================
// Purpose:	Simulates when a solid is being passed through
//==================================================
bool CSimulatedBulletShotgun::AllSolidShotgun(trace_t &ptr)
{
	switch (BulletManagerShotgun()->BulletStopSpeedShotgun())
	{
	case BUTTER_MODEShotgun:
	{
		//Do nothing to bullet speed
		return true;
	}
	case ONE_HIT_MODEShotgun:
	{
		return false;
	}
	default:
	{
		m_flBulletSpeed -= m_flBulletSpeed * m_flEntryDensity / sv_bullet_speed_modifier_shotgun.GetFloat();
		return true;
	}
	}
	return true;
}


//==================================================
// Purpose:	Simulate when a surface is hit
//==================================================
bool CSimulatedBulletShotgun::EndSolidShotgun(trace_t &ptr)
{
	m_vecEntryPosition = ptr.endpos;
	
#ifndef CLIENT_DLL
	int soundEntChannel = (bulletinfo.m_nFlags&FIRE_BULLETS_TEMPORARY_DANGER_SOUND) ? SOUNDENT_CHANNEL_BULLET_IMPACT : SOUNDENT_CHANNEL_UNSPECIFIED;

	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, m_vecEntryPosition, 200, 0.5, NULL, soundEntChannel);
#endif

	if (ptr.surface.name && FStrEq(ptr.surface.name, "tools/toolsblockbullets"))
	{
		return false;
	}
	m_flEntryDensity = physprops->GetSurfaceData(ptr.surface.surfaceProps)->physics.density;

	trace_t rtr;
	Vector vecEnd = m_vecEntryPosition + m_vecDirShooting * 32; //32 units

	//Doing now test of penetration
	AI_TraceLine(m_vecEntryPosition + m_vecDirShooting * 1.1, vecEnd, MASK_SHOT, m_pIgnoreList, &rtr);

	AbsEntry = m_vecEntryPosition;
	AbsExit = rtr.startpos;

	float flPenetrationDistance = VectorLength(AbsEntry - AbsExit);

	DesiredDistance = 0.0f;

	surfacedata_t *p_penetrsurf = physprops->GetSurfaceData(ptr.surface.surfaceProps);
	switch (p_penetrsurf->game.material)
	{
	case CHAR_TEX_WOOD:
		DesiredDistance = 9.0f; // 9 units in hammer
		break;
	case CHAR_TEX_GRATE:
		DesiredDistance = 6.0f; // 6 units in hammer
		break;
	case CHAR_TEX_CONCRETE:
		DesiredDistance = 4.0f; // 4 units in hammer
		break;
	case CHAR_TEX_TILE:
		DesiredDistance = 5.0f; // 5 units in hammer
		break;
	case CHAR_TEX_COMPUTER:
		DesiredDistance = 5.0f; // 5 units in hammer
		break;
	case CHAR_TEX_GLASS:
		DesiredDistance = 8.0f; // maximum 8 units in hammer.
		break;
	case CHAR_TEX_VENT:
		DesiredDistance = 4.0f; // 4 units in hammer and no more(!)
		break;
	case CHAR_TEX_METAL:
		DesiredDistance = 5.0f; // 2 units in hammer. We cannot penetrate a really 'fat' metal wall. Corners are good.
		break;
	case CHAR_TEX_PLASTIC:
		DesiredDistance = 8.0f; // 8 units in hammer: Plastic can more
		break;
	case CHAR_TEX_BLOODYFLESH:
		DesiredDistance = 16.0f; // 16 units in hammer
		break;
	case CHAR_TEX_FLESH:
		DesiredDistance = 16.0f; // 16 units in hammer
		break;
	case CHAR_TEX_DIRT:
		DesiredDistance = 6.0f; // 6 units in hammer: >4 cm of plaster can be penetrated
		break;
	}





	Vector	reflect;
	float fldot = m_vecDirShooting.Dot(ptr.plane.normal);						//Getting angles from lasttrace

	bool bMustDoRico = (fldot > -MAX_RICO_DOT_ANGLEShotgun && GetBulletSpeedRatioShotgun() > MIN_RICO_SPEED_PERCShotgun); // We can't do richochet when bullet has lowest speed

	if (sv_bullet_unrealricochet_shotgun.GetBool() && p_eInfictor->IsPlayer()) //Cheat is only for player,yet =)
		bMustDoRico = true;

	if (bMustDoRico)
	{
		if (!sv_bullet_unrealricochet_shotgun.GetBool())
		{
			if (gpGlobals->maxClients == 1) //Use more simple for multiplayer
			{
				m_flBulletSpeed *= (1.0f / -fldot) * random->RandomFloat(0.005, 0.1);
			}
			else
			{
				m_flBulletSpeed *= (1.0f / -fldot) * 0.025;
			}
		}
		else
		{
			m_flBulletSpeed *= 0.9;
		}

		reflect = m_vecDirShooting + (ptr.plane.normal * (fldot*-2.0f));	//reflecting

		if (gpGlobals->maxClients == 1 && !sv_bullet_unrealricochet_shotgun.GetBool()) //Use more simple for multiplayer
		{
			reflect[0] += random->RandomFloat(fldot, -fldot);
			reflect[1] += random->RandomFloat(fldot, -fldot);
			reflect[2] += random->RandomFloat(fldot, -fldot);
		}

		m_flEntryDensity *= 0.2;

		m_vecDirShooting = reflect;

		m_vecOrigin = ptr.endpos + m_vecDirShooting;//(ptr.endpos + m_vecDirShooting*1.1) + m_vecDirShooting * m_flBulletSpeed;
	}
	else
	{
		if (flPenetrationDistance > DesiredDistance || ptr.IsDispSurface()) //|| !bulletinfo.m_bMustPenetrate
		{
			bStuckInWall = true;

#ifdef GAME_DLL
			if (g_debug_bullets_shotgun.GetBool())
			{
				NDebugOverlay::Line(AbsEntry, AbsExit, 255, 0, 0, true, 10.0f);


			}
#endif
		}
		else
		{
			trace_t tr;
			AI_TraceLine(AbsExit + m_vecDirShooting * 1.1, AbsEntry, MASK_SHOT, m_pIgnoreList, &tr);
			UTIL_ImpactTrace(&tr, GetDamageTypeShotgun()); // On exit

#ifdef GAME_DLL
			if (g_debug_bullets_shotgun.GetBool())
			{
				NDebugOverlay::Line(AbsEntry, AbsExit, 0, 255, 0, true, 10.0f);
			}
#endif

			if (gpGlobals->maxClients == 1) //Use more simple for multiplayer
			{
				m_vecDirShooting[0] += (random->RandomFloat(-flPenetrationDistance, flPenetrationDistance))*0.03;
				m_vecDirShooting[1] += (random->RandomFloat(-flPenetrationDistance, flPenetrationDistance))*0.03;
				m_vecDirShooting[2] += (random->RandomFloat(-flPenetrationDistance, flPenetrationDistance))*0.03;

				VectorNormalize(m_vecDirShooting);
			}

			m_vecOrigin = AbsExit + m_vecDirShooting;
		}
	}

	


#ifdef GAME_DLL
/*if (g_debug_bullets_shotgun.GetBool())
{
	NDebugOverlay::Line(m_vecOriginShotgun, ptr.endpos, 0, 255, 0, true, 10.0f);
}*/
	//Cancel making dust underwater:
	//if (!m_bWasInWater)
	{
		/*trace_t tr2;
		AI_TraceLine(m_vecEntryPositionShotgun, m_vecEntryPositionShotgun + m_vecDirShootingShotgun * 12, MASK_SHOT, m_pIgnoreListShotgun, &tr2);
		if (g_debug_bullets_shotgun.GetBool())
		{
			NDebugOverlay::Line(m_vecEntryPositionShotgun, m_vecEntryPositionShotgun + m_vecDirShootingShotgun * 22, 255, 0, 0, true, 15.0f);
		}*/

		UTIL_ImpactTrace(&ptr, GetDamageTypeShotgun());
	}
#endif

	m_flBulletSpeed -= flPenetrationDistance * m_flEntryDensity / sv_bullet_speed_modifier_shotgun.GetFloat();

	if (BulletManagerShotgun()->BulletStopSpeedShotgun() == ONE_HIT_MODEShotgun)
	{
		return false;
	}
	return false;
}

//-----------------------------------------------------------------------------
// analog of HandleShotImpactingWater
//-----------------------------------------------------------------------------
/*bool CSimulatedBulletShotgun::WaterHitShotgun(const Vector &vecStart, const Vector &vecEnd)
{
	trace_t	waterTrace;
	// Trace again with water enabled
	AI_TraceLine(vecStart, vecEnd, (MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME), m_pIgnoreListShotgun, &waterTrace);

	// See if this is the point we entered
	if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER | CONTENTS_SLIME)) == 0)
		return false;

	int	nMinSplashSize = GetAmmoDef()->MinSplashSize(GetAmmoTypeIndexShotgun()) * (1.5 - GetBulletSpeedRatioShotgun());
	int	nMaxSplashSize = GetAmmoDef()->MaxSplashSize(GetAmmoTypeIndexShotgun()) * (1.5 - GetBulletSpeedRatioShotgun()); //High speed - small splash

	CEffectData	data;
	data.m_vOrigin = waterTrace.endpos;
	data.m_vNormal = waterTrace.plane.normal;
	data.m_flScale = random->RandomFloat(nMinSplashSize, nMaxSplashSize);
	if (waterTrace.contents & CONTENTS_SLIME)
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}
	DispatchEffect("gunshotsplash", data);

#ifndef CLIENT_DLL
	data.m_fFlags = TRACER_TYPE_WATERBULLET;
	DispatchEffect("TracerSound", data);
#endif
	m_bWasInWater = true;
	return true;
}*/

void CSimulatedBulletShotgun::HandleShotImpactingWaterShotgun(const FireBulletsInfo_t &info,
	const Vector &vecStart, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest, bool isPlayer)
{

	trace_t	waterTrace;
	AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME), pTraceFilter, &waterTrace);


	{
		int	nMinSplashSize = GetAmmoDef()->MinSplashSize(info.m_iAmmoType);
		int	nMaxSplashSize = GetAmmoDef()->MaxSplashSize(info.m_iAmmoType);

		CEffectData	data;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(nMinSplashSize, nMaxSplashSize);
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}
		DispatchEffect("gunshotsplash", data);
	}

	isUnderWaterShotgun = true;
}


#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Hits triggers with raycasts
//-----------------------------------------------------------------------------
class CTriggerTraceEnumBulletShotgun : public IEntityEnumerator
{
public:
	CTriggerTraceEnumBulletShotgun(Ray_t *pRay, const CTakeDamageInfo &info, const Vector& dir, int contentsMask) :
		m_info(info), m_VecDir(dir), m_ContentsMask(contentsMask), m_pRay(pRay)
	{
	}

	virtual bool EnumEntity(IHandleEntity *pHandleEntity)
	{
		trace_t tr;

		CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

		// Done to avoid hitting an entity that's both solid & a trigger.
		if (pEnt->IsSolid())
			return true;

		enginetrace->ClipRayToEntity(*m_pRay, m_ContentsMask, pHandleEntity, &tr);
		if (tr.fraction < 1.0f)
		{
			pEnt->DispatchTraceAttack(m_info, m_VecDir, &tr);
			ApplyMultiDamage();
		}

		return true;
	}

private:
	Vector m_VecDir;
	int m_ContentsMask;
	Ray_t *m_pRay;
	CTakeDamageInfo m_info;
};
#endif
//==================================================
// Purpose:	Entity impact procedure
//==================================================
void CSimulatedBulletShotgun::EntityImpactShotgun(trace_t &ptr)
{
	if (bStuckInWall)
		return;

	/*if (!pInfictorRemember || !m_pAttackerRemember)
		return;*/

	if (ptr.m_pEnt != NULL)
	{
		//Hit inflicted once to avoid perfomance errors

		if (pInfictorRemember && isPlayer)
		{
			if (ptr.m_pEnt->IsPlayer())
			{
				if (m_pIgnoreList->ShouldHitEntity(ptr.m_pEnt, MASK_SHOT))
				{
					m_pIgnoreList->AddEntityToIgnore(ptr.m_pEnt);

				}
				else
				{
					return;
				}
			}
		}

		if (ptr.m_pEnt == m_hLastHit)
			return;

		m_hLastHit = ptr.m_pEnt;

		if (!ptr.m_pEnt->IsPlayer() || !ptr.m_pEnt->IsNPC())
		{
			//To make it more reallistic
			float fldot = m_vecDirShooting.Dot(ptr.plane.normal);
			//We affecting damage by angle. If we has lower angle of reflection, doing lower damage.
			flActualDamage *= -fldot;
			flActualForce *= -fldot;
			//But we not doing it on the players or NPC's
		}

		flActualDamage *= GetBulletSpeedRatioShotgun(); //And also affect damage by speed modifications
		flActualForce *=  GetBulletSpeedRatioShotgun(); //Slow bullet - bad force...*/

		if (ptr.m_pEnt && ptr.m_pEnt->IsPlayer())
		{
			info2->ScaleDamageForce(0);
		}

#ifndef CLIENT_DLL
		if (ptr.m_pEnt && ptr.m_pEnt->HasSpawnFlags(SF_BREAK_NO_BULLET_PENETRATION) == false)
		{
			TraceAttackToTriggersShotgun(*info2, ptr.startpos, ptr.endpos, m_vecDirShooting, ptr);
		}
#endif

		ptr.m_pEnt->DispatchTraceAttack(*info2, m_vecDirShootingRemember, &ptr);

		//DevMsg("bulletinfo.m_flDamageForceScale (%i)\n", bulletinfo.m_flDamageForceScale);
#ifdef GAME_DLL

		//if (info2.GetAttacker() != NULL)
			ApplyMultiDamage(); //It's requried

		if (GetAmmoDef()->Flags(GetAmmoTypeIndexShotgun()) & AMMO_FORCE_DROP_IF_CARRIED)
		{
			// Make sure if the player is holding this, he drops it
			Pickup_ForcePlayerToDropThisObject(ptr.m_pEnt);
		}
#endif
		if (ptr.m_pEnt && (
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_dog") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_combinegunship") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_helicopter") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_cscanner") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_strider") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_combineguard") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_combine_camera") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "prop_dropship_container") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_combinedropship") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_cranedriver") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_apcdriver") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "prop_vehicle_apc") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_turret_ground") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "prop_vehicle_jeep") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "prop_vehicle_airboat") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "player") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_manhack") ||
			Q_strstr(STRING(MAKE_STRING(ptr.m_pEnt->GetClassname())), "npc_rollermine")))
		{
			EndSolidShotgun(ptr);
		}
	}
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Handle shot entering water
//-----------------------------------------------------------------------------
void CSimulatedBulletShotgun::TraceAttackToTriggersShotgun(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir, trace_t &tr)
{
	Ray_t ray;
	ray.Init(tr.startpos, tr.endpos);

	CTriggerTraceEnumBulletShotgun triggerTraceEnum(&ray, info, m_vecDirShooting, MASK_SHOT);
	enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);
}
#endif

//==================================================
// Purpose:	Simulates all bullets every centisecond
//==================================================
#ifndef CLIENT_DLL
void CBulletManagerShotgun::Think(void)
#else
void CBulletManagerShotgun::ClientThink(void)
#endif
{
	unsigned short iNext = 0;
	for (unsigned short i = g_BulletsShotgun.Head(); i != g_BulletsShotgun.InvalidIndex(); i = iNext)
	{
		iNext = g_BulletsShotgun.Next(i);
		if (!g_BulletsShotgun[i]->SimulateBulletShotgun())
		{
			RemoveBulletShotgun(i);
		}
	}
	//Msg("Think\n");
	if (g_BulletsShotgun.Head() != g_BulletsShotgun.InvalidIndex())
	{
#ifdef CLIENT_DLL
		SetNextClientThink(gpGlobals->curtime + 0.01f);
#else
		SetNextThink(gpGlobals->curtime + 0.01f);
#endif
	}

}


//==================================================
// Purpose:	Called by sv_bullet_stop_speed callback to keep track of resources
//==================================================
void CBulletManagerShotgun::UpdateBulletStopSpeedShotgun(void)
{
	//m_iBulletStopSpeedShotgun = sv_bullet_stop_speed_shotgun.GetInt();
}

#ifndef CLIENT_DLL
void CBulletManagerShotgun::SendTraceAttackToTriggersShotgun(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir)
{
	TraceAttackToTriggers(info, start, end, dir);
}
#endif


//==================================================
// Purpose:	Add bullet to linked list
//			Handle lag compensation with "prebullet simulation"
// Output:	Bullet's index (-1 for error)
//==================================================
int CBulletManagerShotgun::AddBulletShotgun(CSimulatedBulletShotgun *pBullet)
{
	if (pBullet->GetAmmoTypeIndexShotgun() == -1)
	{
		Msg("ERROR: Undefined ammo type!\n");
		return -1;
	}
	unsigned short index = g_BulletsShotgun.AddToTail(pBullet);
#ifdef CLIENT_DLL
	//DevMsg("Client Bullet Created (%i)\n", index);
	if (g_BulletsShotgun.Count() == 1)
	{
		SetNextClientThink(gpGlobals->curtime + 0.01f);
	}
#else
	//DevMsg("Bullet Created (%i) LagCompensation %f\n", index, pBullet->bulletinfo.m_flLatency);
	if (pBullet->bulletinfo.m_flLatency != 0.0f)
		pBullet->SimulateBulletShotgun(); //Pre-simulation

	if (g_BulletsShotgun.Count() == 1)
	{
		SetNextThink(gpGlobals->curtime + 0.01f);
	}
#endif
	return index;
}


//==================================================
// Purpose:	Remove the bullet at index from the linked list
// Output:	Next index
//==================================================
void CBulletManagerShotgun::RemoveBulletShotgun(int index)
{
	g_BulletsShotgun.Next(index);
#ifdef CLIENT_DLL
	//DevMsg("Client ");
#endif
	//DevMsg("Bullet Removed (%i)\n", index);
	g_BulletsShotgun.Remove(index);
	if (g_BulletsShotgun.Count() == 0)
	{
#ifdef CLIENT_DLL
		SetNextClientThink(TICK_NEVER_THINK);
#else
		SetNextThink(TICK_NEVER_THINK);
#endif
	}

}