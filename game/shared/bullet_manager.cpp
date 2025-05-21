// (More) Reallistic simulated bullets
//
// This code is originally based from article on Valve Developer Community
// The original code you can find by this link:
// http://developer.valvesoftware.com/wiki/Simulated_Bullets

// NOTENOTE: Tested only on localhost. There maybe a latency errors and others
// NOTENOTE: The simulation might be strange.

#include "cbase.h"
#include "util_shared.h"
#include "bullet_manager.h"
#include "effect_dispatch_data.h"
#include "tier0/vprof.h"
#include "decals.h"
#include "ai_debug_shared.h"
#include "baseentity_shared.h"
#include "IEffects.h"



#ifdef CLIENT_DLL

#else
#include "ai_basenpc.h"
#include "baseentity.h"

#include "func_break.h"
#include "waterbullet.h"
#include "WaterBulletDynamic.h"
#endif

CBulletManager *g_pBulletManager;
CUtlLinkedList<CSimulatedBullet*> g_Bullets;

#ifdef CLIENT_DLL//-------------------------------------------------
#include "engine/ivdebugoverlay.h"
#include "c_te_effect_dispatch.h"
ConVar g_debug_client_bullets("g_debug_client_bullets", "0", FCVAR_CHEAT);
extern void FX_PlayerTracer(Vector& start, Vector& end);

#else//-------------------------------------------------------------
#include "vehicle_base.h"
#include "te_effect_dispatch.h"
#include "soundent.h"
#include "player_pickup.h"
#include "ilagcompensationmanager.h"
ConVar g_debug_bullets("g_debug_bullets", "0", FCVAR_CHEAT, "Debug of bullet simulation\nThe white line shows the bullet trail.\nThe red line shows not passed penetration test.\nThe green line shows passed penetration test. Turn developer mode for more information.");
#endif//------------------------------------------------------------

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_RICO_DOT_ANGLE 0.15f  //Maximum dot allowed for any ricochet
#define MIN_RICO_SPEED_PERC 0.55f //Minimum speed percent allowed for any ricochet


static void BulletSpeedModifierCallback(ConVar *var, const char *pOldString)
{
	if (cvar->FindVar("sv_bullet_speed_modifier")->GetFloat() == 0.0f)//var->GetFloat() == 0.0f) //To avoid math exception
		cvar->FindVar("sv_bullet_speed_modifier")->Revert();// var->Revert();
}
ConVar sv_bullet_speed_modifier("sv_bullet_speed_modifier", "100.000000", (FCVAR_ARCHIVE | FCVAR_REPLICATED),
	"Density/(This Value) * (Distance Penetrated) = (Change in Speed)",
	((FnChangeCallback_t)BulletSpeedModifierCallback));

static void UnrealRicochetCallback(ConVar *var, const char *pOldString)
{
	if (gpGlobals->maxClients > 1)
	{
		cvar->FindVar("sv_bullet_unrealricochet")->Revert(); //var->Revert();
		//Msg("Cannot use unreal ricochet in multiplayer game\n");
	}

	//if (cvar->FindVar("sv_bullet_unrealricochet")->GetBool())//var->GetBool()) //To avoid math exception
		//Warning("\nWarning! Enabling unreal ricochet may cause the game crash.\n\n");
}
ConVar sv_bullet_unrealricochet("sv_bullet_unrealricochet", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Unreal ricochet", ((FnChangeCallback_t)UnrealRicochetCallback));



static void BulletStopSpeedCallback(ConVar *var, const char *pOldString)
{
	int val = cvar->FindVar("sv_bullet_stop_speed")->GetInt();//var->GetInt();
	if (val<ONE_HIT_MODE)
		cvar->FindVar("sv_bullet_stop_speed")->Revert(); //var->Revert();
	else if (BulletManager())
		BulletManager()->UpdateBulletStopSpeed();
}
ConVar sv_bullet_stop_speed("sv_bullet_stop_speed", "40", FCVAR_REPLICATED,
	"Integral speed at which to remove the bullet from the bullet queue\n-1 is butter mode\n-2 is 1 hit mode",
	((FnChangeCallback_t)BulletStopSpeedCallback));



LINK_ENTITY_TO_CLASS(bullet_manager, CBulletManager);


//==================================================
// Purpose:	Constructor
//==================================================
CSimulatedBullet::CSimulatedBullet(int nEntIndex)//int nEntIndex
{
	g_pBulletManager->index = nEntIndex;
	m_vecOrigin.Init();
	m_vecDirShooting.Init();
	m_flInitialBulletSpeed = m_flBulletSpeed = 0;
	m_flEntryDensity = 0.0f;
	bStuckInWall = false;
	isUnderWater = false;
	//PrecacheParticleSystem("weapon_underwater_bubbles");
	m_iDamageType = 2;
}

//==================================================
// Purpose:	Constructor
//==================================================
CSimulatedBullet::CSimulatedBullet(FireBulletsInfo_t info, int nDamageType, int nAmmoFlags, Vector newdir, CBaseEntity *pInfictor, CBaseEntity *pAdditionalIgnoreEnt,
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

	//pFireBulletsInfo = new FireBulletsInfo_t();

	pFireBulletsInfo = info;			// Setup Fire bullets information here

	p_eInfictor = pInfictor;	// Setup inflictor

	isPlayer = p_eInfictor->IsPlayer();

	CBaseCombatCharacter * pBaseChar = ToBaseCombatCharacter(p_eInfictor);
	if (pBaseChar && pBaseChar->GetActiveWeapon())
		penetrationLength = pBaseChar->GetActiveWeapon()->GetWpnData().bulletPenetrationLength;
	else
		penetrationLength = 5.0f;

	m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
	Assert(m_pIgnoreList);

	bStuckInWall = false;

	penetrated = false;

	lifeAfterImpact = gpGlobals->curtime;

	m_pIgnoreList->AddEntityToIgnore(p_eInfictor);

	if (pAdditionalIgnoreEnt != NULL)
		m_pIgnoreList->AddEntityToIgnore(pAdditionalIgnoreEnt);
	
	m_iDamageType = nDamageType;// GetAmmoDef()->DamageType(pFireBulletsInfo.m_iAmmoType);
	TracerType = GetAmmoDef()->TracerType(pFireBulletsInfo.m_iAmmoType);

	AmmoType = pFireBulletsInfo.m_iAmmoType;

	if (p_eInfictor)
	pInfictorRemember = p_eInfictor;

	isPlayer = pInfictorRemember->IsPlayer();

	flActualForce = pFireBulletsInfo.m_flDamageForceScale;

	m_flInitialBulletSpeed = m_flBulletSpeed = GetAmmoDef()->GetAmmoOfIndex(pFireBulletsInfo.m_iAmmoType)->bulletSpeed;

	m_vecDirShooting = newdir;

	if (isPlayer)
	{
		
		if (p_eInfictor->GetWaterLevel() == 3)
		{
			CBasePlayer *pOwner = ToBasePlayer(p_eInfictor);

			if (pOwner != NULL && pOwner->GetViewModel() && pOwner->GetActiveWeapon())
			{
				m_vecOrigin = pInfictor->EyePosition();
			}

			m_flInitialBulletSpeed *= 0.2f;
		}
		else
		{

			CBasePlayer *pPlayer = ToBasePlayer(p_eInfictor);
			if (pPlayer)
			{
#ifndef CLIENT_DLL
				if (pPlayer->IsInControl)
					m_vecOrigin = pFireBulletsInfo.m_vecSrc;
				else
					m_vecOrigin = pInfictor->EyePosition();
#endif
			}
			else
				m_vecOrigin = pInfictor->EyePosition();

		}
		
	}
	else
	{
		m_vecOrigin = pFireBulletsInfo.m_vecSrc;//pFireBulletsInfo->m_vecSrc;
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

	g_pBulletManager->EndPoint = m_vecTraceRay2;

	EndPoint = m_vecTraceRay2;

	trace_t tr;
	if (m_bTraceHull)
		AI_TraceHull(m_vecOrigin, m_vecTraceRay2, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SHOT_HULL, m_pIgnoreList, &tr);
	else
		AI_TraceLine(m_vecOrigin, m_vecTraceRay2, MASK_SHOT, m_pIgnoreList, &tr);
	trEnd = tr;

	
	int iPlayerDamage = pFireBulletsInfo.m_iPlayerDamage;

	pAmmoDef = GetAmmoDef();

	flActualDamage = pFireBulletsInfo.m_flDamage;

	//if (tr.m_pEnt)
	{
		if (isPlayer)
		{
			flActualDamage = iPlayerDamage = pAmmoDef->PlrDamage(pFireBulletsInfo.m_iAmmoType);
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
			flActualDamage = iPlayerDamage = pAmmoDef->NPCDamage(pFireBulletsInfo.m_iAmmoType);
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

	m_vecDirShootingRemember = pFireBulletsInfo.m_vecDirShooting;

	info2 = new CTakeDamageInfo(pInfictorRemember, pFireBulletsInfo.m_pAttacker, flActualDamage, GetDamageType());

	//CTakeDamageInfo dmgInfo(pInfictorRemember, pFireBulletsInfo->m_pAttacker, flActualDamage, GetDamageType());
	CalculateBulletDamageForce(info2, AmmoType, m_vecDirShooting, tr.endpos);
	info2->ScaleDamageForce(flActualForce);
	info2->SetAmmoType(AmmoType);

	//info2 = dmgInfo;

	isUnderWater = false;

	//DebugDrawLine(tr.startpos, tr.endpos, 255, 255, 255, false, 15.0f);
}

//==================================================
// Purpose:	Deconstructor
//==================================================
CSimulatedBullet::~CSimulatedBullet()
{
	delete m_pIgnoreList;
	m_pIgnoreList = NULL;

	delete info2;
	info2 = NULL;

	/*delete pFireBulletsInfo;
	pFireBulletsInfo = NULL;
	delete info2;
	info2 = NULL;*/
}
//==================================================
// Purpose:	Simulates a bullet through a ray of its bullet speed
//==================================================
bool CSimulatedBullet::SimulateBullet(void)
{
	VPROF("C_SimulatedBullet::SimulateBullet");

	if (!IsFinite(m_flBulletSpeed))
		return false;		 //prevent a weird crash

	trace_t trace;
	trace = trEnd;

	if (m_flBulletSpeed <= 0) //Avoid errors;
		return false;

	if (!p_eInfictor)
	{
		p_eInfictor = pInfictorRemember;//pFireBulletsInfo->m_pAttacker;

		if (!p_eInfictor)
			return false;
	}

	m_flRayLength = m_flBulletSpeed;

	if (!isUnderWater)
	{
		m_flBulletSpeed += 0.8f * m_vecDirShooting.z; //TODO: Bullet mass
	}

	m_vecTraceRay = m_vecOrigin + m_vecDirShooting * m_flBulletSpeed;

	//DebugDrawLine(m_vecOrigin, m_vecTraceRay, 255, 255, 255, false, 15.0f);

	if (m_flBulletSpeed <= 0.f)
		return false;
	//m_vecDirShooting.z -= 0.1 / m_flBulletSpeed;// Bullet mass

#ifdef GAME_DLL
	if (pFireBulletsInfo.m_flLatency != 0)
	{
		m_vecTraceRay *= pFireBulletsInfo.m_flLatency * 100;
	}
#endif

	if (!IsInWorld())
	{
		return false;
	}

	if (bStuckInWall)
		return false;


	if ((pInfictorRemember) && (enginetrace->GetPointContents(m_vecOrigin) & (CONTENTS_WATER | CONTENTS_SLIME)) && m_flBulletSpeed != 0.f)
	{
		UTIL_Bubbles(m_vecOrigin, m_vecOrigin, 3);

		if (!isUnderWater && pInfictorRemember->GetWaterLevel() != 3)
		{
			HandleShotImpactingWater(pFireBulletsInfo, m_vecOrigin, m_vecOrigin, m_pIgnoreList, &trEnd.endpos, isPlayer);
		}
	}

	bool bulletSpeedCheck;

	bulletSpeedCheck = false;

	//MikeD UTIL_TraceLine or AI_TraceLine affects to SendTraceAttackToTriggers, so be careful
	if (m_bTraceHull)
		AI_TraceHull(m_vecOrigin, m_vecTraceRay, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SHOT_HULL, m_pIgnoreList, &trace);
	else
		AI_TraceLine(m_vecOrigin, m_vecTraceRay, MASK_SHOT, m_pIgnoreList, &trace);



#ifdef CLIENT_DLL
	if (g_debug_client_bullets.GetBool())
	{
		debugoverlay->AddLineOverlay(trace.startpos, trace.endpos, 255, 0, 0, true, 10.0f);
	}


#else
	if (g_debug_bullets.GetBool())
	{
		NDebugOverlay::Line(trace.startpos, trace.endpos, 255, 255, 255, true, 10.0f);
	}

	g_pBulletManager->EndPoint = trace.endpos;

	//if (pFireBulletsInfo.m_pAttacker != NULL)
	{
		/*CTakeDamageInfo triggerInfo(p_eInfictor, pFireBulletsInfo->m_pAttacker, pFireBulletsInfo->m_flDamage, GetDamageType());
		CalculateBulletDamageForce(&triggerInfo, pFireBulletsInfo->m_iAmmoType, m_vecDirShooting, trace.endpos);
		triggerInfo.ScaleDamageForce(pFireBulletsInfo->m_flDamageForceScale);
		triggerInfo.SetAmmoType(pFireBulletsInfo->m_iAmmoType);*/
		BulletManager()->SendTraceAttackToTriggers(*info2, trace.startpos, trace.endpos, m_vecDirShooting);
	}
#endif
	
	if (penetrated && lifeAfterImpact < gpGlobals->curtime)
	{
		return false;
	}

	if (trace.fraction == 1.0f)
	{
		m_vecOrigin += m_vecDirShooting * m_flBulletSpeed; //Do the WAY

		/*CEffectData data;
		data.m_vStart = trace.startpos;
		data.m_vOrigin = trace.endpos;
		data.m_nDamageType = GetDamageType();

		DispatchEffect("RagdollImpact", data);*/

		return true;
	}
	else
	{
		if (trace.m_pEnt == p_eInfictor) //HACK: Remove bullet if we hit self (for frag grenades)
			return false;

		if (!EntityImpact(trace))
			return false;

		if (!(trace.surface.flags & SURF_SKY))
		{

			if (trace.allsolid)//in solid
			{
				if (!AllSolid(trace))
					return false;

				m_vecOrigin += m_vecDirShooting * m_flBulletSpeed; //Do the WAY

				bulletSpeedCheck = true;
			}
			else if (trace.fraction != 1.0f)//hit solid
			{

				if (!EndSolid(trace))
					return false;

				bulletSpeedCheck = true;
			}
			else if (trace.startsolid)//exit solid
			{
				if (!StartSolid(trace))
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

	if (sv_bullet_unrealricochet.GetBool()) //Fun bullet ricochet fix
	{
		delete m_pIgnoreList; //Prevent causing of memory leak
		m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
	}

	if (bulletSpeedCheck)
	{
		if (m_flBulletSpeed <= BulletManager()->BulletStopSpeed())
		{
			return false;
		}
	}

	return true;
}


//==================================================
// Purpose:	Simulates when a solid is exited
//==================================================
bool CSimulatedBullet::StartSolid(trace_t &ptr)
{
	switch (BulletManager()->BulletStopSpeed())
	{
	case BUTTER_MODE:
	{
		//Do nothing to bullet speed
		return true;
	}
	case ONE_HIT_MODE:
	{
		return false;
	}
	default:
		{
			//float flPenetrationDistance = VectorLength(AbsEntry - AbsExit);

			//m_flBulletSpeed -= flPenetrationDistance * m_flEntryDensity / sv_bullet_speed_modifier.GetFloat();
			return true;
		}
	}
	return true;
}


//==================================================
// Purpose:	Simulates when a solid is being passed through
//==================================================
bool CSimulatedBullet::AllSolid(trace_t &ptr)
{
	switch (BulletManager()->BulletStopSpeed())
	{
	case BUTTER_MODE:
	{
		//Do nothing to bullet speed
		return true;
	}
	case ONE_HIT_MODE:
	{
		return false;
	}
	default:
		{
			//m_flBulletSpeed -= m_flBulletSpeed * m_flEntryDensity / sv_bullet_speed_modifier.GetFloat();
			return true;
		}
	}
	return true;
}


//==================================================
// Purpose:	Simulate when a surface is hit
//==================================================
bool CSimulatedBullet::EndSolid(trace_t &ptr)
{
	if (cvar->FindVar("oc_weapons_enable_dynamic_bullets_penetration")->GetInt() == 1)
	{ 
		if (TracerType != TRACER_ABGUN)
		{
			/*penetrationEntry = */m_vecEntryPosition = ptr.endpos;

#ifndef CLIENT_DLL
			int soundEntChannel = (pFireBulletsInfo.m_nFlags&FIRE_BULLETS_TEMPORARY_DANGER_SOUND) ? SOUNDENT_CHANNEL_BULLET_IMPACT : SOUNDENT_CHANNEL_UNSPECIFIED;

			CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, m_vecEntryPosition, 200, 0.5, NULL, soundEntChannel);
#endif

			if (ptr.surface.name && FStrEq(ptr.surface.name, "tools/toolsblockbullets"))
			{
				return false;
			}

			m_flEntryDensity = physprops->GetSurfaceData(ptr.surface.surfaceProps)->physics.density;

			trace_t rtr;
			Vector vecEnd = m_vecEntryPosition + m_vecDirShooting; //32 units

			if (isPlayer)
				UTIL_TraceLine(m_vecEntryPosition + m_vecDirShooting, vecEnd, MASK_SHOT, m_pIgnoreList, &rtr);
			else
				AI_TraceLine(m_vecEntryPosition + m_vecDirShooting, vecEnd, MASK_SHOT, m_pIgnoreList, &rtr);

			AbsEntry = m_vecEntryPosition;
			AbsExit = rtr.startpos;

			float flPenetrationDistance = 1.f;//VectorLength(AbsEntry - AbsExit);

			/*DesiredDistance = 0.f;//penetrationLength;//200.0f;

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
			}*/





			Vector	reflect;
			float fldot = m_vecDirShooting.Dot(ptr.plane.normal);						//Getting angles from lasttrace

			bool bMustDoRico = (fldot > -MAX_RICO_DOT_ANGLE && GetBulletSpeedRatio() > MIN_RICO_SPEED_PERC); // We can't do richochet when bullet has lowest speed

			if (sv_bullet_unrealricochet.GetBool() && isPlayer && TracerType != TRACER_ABGUN) //Cheat is only for player,yet =)
				bMustDoRico = true;

			if (bMustDoRico)
			{
				if (!sv_bullet_unrealricochet.GetBool())
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

				if (gpGlobals->maxClients == 1 && !sv_bullet_unrealricochet.GetBool()) //Use more simple for multiplayer
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
				/*if (flPenetrationDistance > DesiredDistance || ptr.IsDispSurface()) //|| !pFireBulletsInfo->m_bMustPenetrate
				{
					bStuckInWall = true;

#ifdef GAME_DLL
					if (g_debug_bullets.GetBool())
					{
						NDebugOverlay::Line(AbsEntry, AbsExit, 255, 0, 0, true, 10.0f);			
					}
#endif
				}
				else*/
				{
					trace_t tr;
					AI_TraceLine(AbsExit + m_vecDirShooting, AbsEntry, MASK_SHOT, m_pIgnoreList, &tr);

					if ((TracerType != TRACER_SNIPER) && (TracerType != TRACER_AR2) && (TracerType != TRACER_ABGUN))
						UTIL_ImpactTrace(&tr, GetDamageType()); // On exit

#ifdef GAME_DLL
					if (g_debug_bullets.GetBool())
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
					//penetrationMaxEnd = AbsExit + m_vecDirShooting * penetrationLength;
					//penetrationMaxEnd = m_vecOrigin;
				}
			}
			//m_flBulletSpeed -= flPenetrationDistance * m_flEntryDensity / sv_bullet_speed_modifier.GetFloat();
			if (!penetrated)
			{
				penetrated = true;
				lifeAfterImpact = gpGlobals->curtime + penetrationLength*0.01f;
			}
		}

	}
#ifdef GAME_DLL
	//Cancel making dust underwater:
	//if (!m_bWasInWater)
	{
		if ((TracerType == TRACER_SNIPER) || (TracerType == TRACER_AR2))
		{
			//CEffectData data;

			//data.m_vOrigin = ptr.endpos + (ptr.plane.normal * 1.0f);
			//data.m_vNormal = ptr.plane.normal;
			//DispatchEffect("AirboatGunImpact", data);

			/*float Radius = 110;
			CPASFilter filter(ptr.endpos);
			te->DynamicLight(filter, 0.0, &ptr.endpos, 0, random->RandomInt(95, 125), random->RandomInt(155, 195), 3, Radius, random->RandomInt(0.018, 0.23), Radius / 0.5);//OverCharged
			*/

			UTIL_ImpactTrace(&ptr, GetDamageType(), "AirboatGunImpact");//"AirboatGunImpact"//"BloodDripsImpact"

			//UTIL_ImpactTrace(&ptr, GetDamageType());
		}
		else if (TracerType == TRACER_ABGUN)
		{
			//pAmmoDef = GetAmmoDef();
			//int ammoType = pAmmoDef->Index("AirboatGun");

			//CEffectData data;

			//data.m_vOrigin = ptr.endpos + (ptr.plane.normal * 1.0f);
			//data.m_vNormal = ptr.plane.normal;
			//DispatchEffect("AirboatGunImpact", data);

			UTIL_ImpactTrace(&ptr, GetDamageType(), "AirboatGunImpact");//"AirboatGunImpact"//"BloodDripsImpact"
			//g_pEffects->Sparks(data.m_vOrigin);
			/*
			float Radius = 110;
			CPASFilter filter(ptr.endpos);
			te->DynamicLight(filter, 0.0, &ptr.endpos, 0, random->RandomInt(95, 125), random->RandomInt(155, 195), 3, Radius, random->RandomInt(0.018, 0.23), Radius / 0.5);//OverCharged
			*/

			//UTIL_ImpactTrace(&ptr, ammoType);
		}
		else
			UTIL_ImpactTrace(&ptr, GetDamageType());
	}
#endif
	//
	if (cvar->FindVar("oc_weapons_enable_dynamic_bullets_penetration")->GetInt() == 1)
	{
		if (TracerType != TRACER_ABGUN)
		{
			if (BulletManager()->BulletStopSpeed() == ONE_HIT_MODE)
			{
				return false;
			}
			return true;
		}
	}

return false;
}

//-----------------------------------------------------------------------------
// analog of HandleShotImpactingWater
//-----------------------------------------------------------------------------
/*bool CSimulatedBullet::WaterHit(const Vector &vecStart, const Vector &vecEnd)
{
	
	trace_t	waterTrace;
	// Trace again with water enabled
	UTIL_TraceLine(vecStart, vecEnd, (MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME), m_pIgnoreList, &waterTrace);

	// See if this is the point we entered
	if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER | CONTENTS_SLIME)) == 0)
		return false;

	int	nMinSplashSize = GetAmmoDef()->MinSplashSize(GetAmmoTypeIndex()) * (1.5 - GetBulletSpeedRatio());
	int	nMaxSplashSize = GetAmmoDef()->MaxSplashSize(GetAmmoTypeIndex()) * (1.5 - GetBulletSpeedRatio()); //High speed - small splash

	CEffectData	data;
	entIndex = data.GetEffectNameIndex();
	data.m_vOrigin = waterTrace.endpos;
	data.m_vNormal = waterTrace.plane.normal;
	data.m_flScale = random->RandomFloat(nMinSplashSize, nMaxSplashSize);
	if (waterTrace.contents & CONTENTS_SLIME)
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}
#ifndef CLIENT_DLL
	data.m_fFlags = TRACER_TYPE_WATERBULLET;
	//if (!isUnderWater)
	DispatchEffect("TracerSound", data);
#endif
	//if (!isUnderWater)
	DispatchEffect("gunshotsplash", data);

	//isUnderWater = true;
	//m_bWasInWater = true;
	return true;
}*/



void CSimulatedBullet::HandleShotImpactingWater(const FireBulletsInfo_t &info,
	const Vector &vecStart, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest, bool isPlayer)
{
	trace_t	waterTrace;
	// See if this is the point we entered
	//if ((enginetrace->GetPointContents(vecStart - Vector(0, 0, 0.1f)) & (CONTENTS_WATER | CONTENTS_SLIME)) == 0)
		//return false;
	AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME), pTraceFilter, &waterTrace);
	//if (pInfictorRemember && pInfictorRemember->GetWaterLevel() != 3) //if (ShouldDrawWaterImpacts())
	//{
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
	//}

	isUnderWater = true;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Hits triggers with raycasts
//-----------------------------------------------------------------------------
class CTriggerTraceEnumBullet : public IEntityEnumerator
{
public:
	CTriggerTraceEnumBullet(Ray_t *pRay, const CTakeDamageInfo &info, const Vector& dir, int contentsMask) :
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
bool CSimulatedBullet::EntityImpact(trace_t &ptr)
{
	if (bStuckInWall)
		return true;

	if (ptr.m_pEnt != NULL)
	{
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
					return true;
				}
			}
		}

		if (ptr.m_pEnt == m_hLastHit)
			return true;

		m_hLastHit = ptr.m_pEnt;

		if (!ptr.m_pEnt->IsPlayer() || !ptr.m_pEnt->IsNPC())
		{
			float fldot = m_vecDirShooting.Dot(ptr.plane.normal);
			//We affecting damage by angle. If we has lower angle of reflection, doing lower damage.
			flActualDamage *= -fldot;
			flActualForce *= -fldot;
		}

		flActualDamage *= GetBulletSpeedRatio(); //And also affect damage by speed modifications
		flActualForce *= GetBulletSpeedRatio(); //Slow bullet - bad force...

		if (ptr.m_pEnt && ptr.m_pEnt->IsPlayer())
		{
			info2->ScaleDamageForce(0);
		}


#ifndef CLIENT_DLL
		if (ptr.m_pEnt && ptr.m_pEnt->HasSpawnFlags(SF_BREAK_NO_BULLET_PENETRATION) == false)
		{
			TraceAttackToTriggers(*info2, ptr.startpos, ptr.endpos, m_vecDirShooting, ptr);
		}
#endif

		ptr.m_pEnt->DispatchTraceAttack(*info2, m_vecDirShootingRemember, &ptr);


#ifdef GAME_DLL
		//ptr.m_pEnt->TraceAttackToTriggers(info2, m_vecOrigin, m_vecOrigin, m_vecDirShooting);


		//if (info2.GetAttacker() != NULL)
			ApplyMultiDamage(); //It's requried

		if (GetAmmoDef()->Flags(GetAmmoTypeIndex()) & AMMO_FORCE_DROP_IF_CARRIED)
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
			EndSolid(ptr);
			//UTIL_ImpactTrace(&trace, GetDamageType());

			return false;
		}
		else
			return true;

	}

	return true;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Handle shot entering water
//-----------------------------------------------------------------------------
void CSimulatedBullet::TraceAttackToTriggers(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir, trace_t &tr)
{
	Ray_t ray;
	ray.Init(tr.startpos, tr.endpos);

	CTriggerTraceEnumBullet triggerTraceEnum(&ray, info, m_vecDirShooting, MASK_SHOT);
	enginetrace->EnumerateEntities(ray, true, &triggerTraceEnum);

}
#endif


/*IMPLEMENT_SERVERCLASS_ST(CBulletManager, DT_BulletManager)
SendPropInt(SENDINFO(index), 1, SPROP_UNSIGNED),
SendPropVector(SENDINFO(EndPoint)),
END_SEND_TABLE()*/
//==================================================
// Purpose:	Simulates all bullets every centisecond
//==================================================
#ifndef CLIENT_DLL
void CBulletManager::Think(void)
#else
void CBulletManager::ClientThink(void)
#endif
{
/*#ifdef CLIENT_DLL
	DevMsg("Think\n");
	dlight_t *el = effects->CL_AllocDlight(g_pBulletManager->index);//( index );
	el->origin = g_pBulletManager->EndPoint;

	el->color.r = 38;
	el->color.g = 151;
	el->color.b = 191;
	el->color.exponent = 5;

	el->radius = random->RandomInt(82, 118);
	el->decay = el->radius / 0.05f;
	el->die = gpGlobals->curtime + 0.17f;
#endif*/
	unsigned short iNext = 0;
	for (unsigned short i = g_Bullets.Head(); i != g_Bullets.InvalidIndex(); i = iNext)
	{
		iNext = g_Bullets.Next(i);
		if (!g_Bullets[i]->SimulateBullet())
		{
			//RemoveEffects(g_Bullets[i]->entIndex);

			RemoveBullet(i);
		}
	}

	if (g_Bullets.Head() != g_Bullets.InvalidIndex())
	{
#ifdef CLIENT_DLL
		SetNextClientThink(gpGlobals->curtime + 0.01f);
#else
		SetNextThink(gpGlobals->curtime + 0.01f);
#endif
	}
}

/*#ifdef CLIENT_DLL
void C_BulletManager::ClientThink(void)
{

	DevMsg("Think\n");
	dlight_t *el = effects->CL_AllocDlight(g_pBulletManager->index);//( index );
	el->origin = g_pBulletManager->EndPoint;

	el->color.r = 38;
	el->color.g = 151;
	el->color.b = 191;
	el->color.exponent = 5;

	el->radius = random->RandomInt(82, 118);
	el->decay = el->radius / 0.05f;
	el->die = gpGlobals->curtime + 0.17f;

}
#endif*/
//==================================================
// Purpose:	Called by sv_bullet_stop_speed callback to keep track of resources
//==================================================
void CBulletManager::UpdateBulletStopSpeed(void)
{
	m_iBulletStopSpeed = sv_bullet_stop_speed.GetInt();
}

#ifndef CLIENT_DLL
void CBulletManager::SendTraceAttackToTriggers(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir)
{
	TraceAttackToTriggers(info, start, end, dir);
}
#endif


//==================================================
// Purpose:	Add bullet to linked list
//			Handle lag compensation with "prebullet simulation"
// Output:	Bullet's index (-1 for error)
//==================================================
int CBulletManager::AddBullet(CSimulatedBullet *pBullet)
{
	if (pBullet->GetAmmoTypeIndex() == -1)
	{
		//Msg("ERROR: Undefined ammo type!\n");
		return -1;
	}
	unsigned short index = g_Bullets.AddToTail(pBullet);
#ifdef CLIENT_DLL
	//DevMsg("Client Bullet Created (%i)\n", index);
	if (g_Bullets.Count() == 1)
	{
		SetNextClientThink(gpGlobals->curtime + 0.01f);
	}
#else
	//DevMsg("Bullet Created (%i) LagCompensation %f\n", index, pBullet->pFireBulletsInfo->m_flLatency);
	if (pBullet->pFireBulletsInfo.m_flLatency != 0.0f)
		pBullet->SimulateBullet(); //Pre-simulation

	if (g_Bullets.Count() == 1)
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
void CBulletManager::RemoveBullet(int index)
{
	g_Bullets.Next(index);
#ifdef CLIENT_DLL
	//DevMsg("Client ");
#endif
	//DevMsg("Bullet Removed (%i)\n", index);
	delete g_Bullets.Element(index);
	g_Bullets.Element(index) = NULL;
	g_Bullets.Remove(index);
	if (g_Bullets.Count() == 0)
	{
#ifdef CLIENT_DLL
		SetNextClientThink(TICK_NEVER_THINK);
#else
		SetNextThink(TICK_NEVER_THINK);
#endif
	}

}

