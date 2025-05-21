//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Wrench - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_wrench.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "te_effect_dispatch.h"
#include "npc_metropolice.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    oc_weapon_wrench_tracelen("oc_weapon_wrench_tracelen", "0");
ConVar    sk_plr_dmg_wrench("sk_plr_dmg_wrench", "0");
ConVar    sk_plr_dmg_wrench_alt	( "sk_plr_dmg_wrench_alt","0");
ConVar    sk_npc_dmg_wrench		( "sk_npc_dmg_wrench","0");

//-----------------------------------------------------------------------------
// CWeaponWrench
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponWrench, DT_WeaponWrench)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_wrench, CWeaponWrench );
PRECACHE_WEAPON_REGISTER( weapon_wrench );
#endif


BEGIN_DATADESC(CWeaponWrench)

DEFINE_FIELD(m_bAltattackAnim, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAltattackChecker, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedSwing, FIELD_BOOLEAN),
DEFINE_FIELD(m_flDelayedSwingTime, FIELD_FLOAT),
DEFINE_FIELD(Fire, FIELD_BOOLEAN),
DEFINE_FIELD(Time, FIELD_INTEGER),
DEFINE_FIELD(ChangeOnce, FIELD_BOOLEAN),
DEFINE_FIELD(bIsSecondary, FIELD_BOOLEAN),
DEFINE_FIELD(secondaryGate, FIELD_BOOLEAN),
DEFINE_FIELD(m_bCanPrimaryAttack, FIELD_BOOLEAN),

END_DATADESC()

acttable_t CWeaponWrench::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponWrench);

Activity CWeaponWrench::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner)
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponWrench::CWeaponWrench( void )
{
	secondaryGate = false;
	ChangeOnce = true;
	bIsSecondary = true;
	m_bCanPrimaryAttack = true;
}

bool CWeaponWrench::Deploy( void )
{
	secondaryGate = false;
	ChangeOnce = true;
	m_bAltattackAnim	= false;
	m_bAltattackChecker = false;
	Fire = false;
	Time = 0;
	m_bDelayedSwing = false;
    m_flDelayedSwingTime = 0.0f;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponWrench::GetDamageForActivity( Activity hitActivity )
{
	if ( m_bAltattackChecker ) // Light Kill : Do check for alt attack...
	{
		if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		{
			if (!Fire)
				return sk_plr_dmg_wrench_alt.GetFloat();
			else
				return sk_plr_dmg_wrench.GetFloat();
		}
	}
	else
	{
		if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		{
			if (!Fire)
				return sk_plr_dmg_wrench_alt.GetFloat();
			else
				return sk_plr_dmg_wrench.GetFloat();
		}
	}

	return sk_npc_dmg_wrench.GetFloat();
}



//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the Wrench!)
//-----------------------------------------------------------------------------
ConVar sk_wrench_lead_time( "sk_wrench_lead_time", "0.9" );

int CWeaponWrench::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the Wrench!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_wrench_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponWrench::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange());

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = GetWpnData().animData[m_bFireMode].MeleeAttack1;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * GetHull();  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, GetMeleeHullMin(), GetMeleeHullMax(), MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				//nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? GetWpnData().animData[m_bFireMode].MeleeMiss2 : GetWpnData().animData[m_bFireMode].MeleeMiss1;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		//Make sound for the AI
		CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

		// This isn't great, but it's something for when the crowbar hits.
		pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

		CBaseEntity	*pHitEntity = traceHit.m_pEnt;

		//Apply damage to a hit target
		if (pHitEntity != NULL)
		{
			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);

			if (pPlayer && pHitEntity->IsNPC())
			{
				// If bonking an NPC, adjust damage.
				info.AdjustPlayerDamageInflictedForSkillLevel();
			}

			CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

			pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
			ApplyMultiDamage();

			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);

			if (ToBaseCombatCharacter(pHitEntity))
			{
				gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
			}
		}

		// Apply an impact effect
		ImpactEffect(traceHit);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponWrench::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
	{
		Time = 0;
		m_bAltattackAnim = false;
		if (!IsNearWall() && !GetOwnerIsRunning())
		{
			HandleAnimEventMeleeHit(pEvent, pOperator);
		}
	}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}



void CWeaponWrench::ItemPostFrame( void )
{
	if (!m_bInReload && (IsNearWall() || GetOwnerIsRunning()))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.45;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.45;

		/*if (GetActivity() != GetWpnData().MeleeIdle
			&& IsViewModelSequenceFinished() && !m_bAltattackAnim)
		{
			SendWeaponAnim(GetWpnData().MeleeIdle);
		}*/
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;


	trace_t traceHit;
	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange());
	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit);

	//CheckAdmireAnimations(pOwner);

	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD) || (pOwner->m_nButtons & IN_FIREMODE)))	// BriJee OVR: IN_FIREMODE added
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false) && m_bReloadComplete == 0 && pOwner->GetActiveWeapon())
		{
			if (pOwner->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].ZeroIdleAnim == 0 && pOwner->GetActiveWeapon()->thisType != TYPE_GRENADE)
				WeaponIdle();
		}
	}

	/*if (pOwner->m_nButtons & IN_ZOOM)	// L1ght 15 : Zoom bug fixed
	{
		m_bAltattackAnim = false;
		return;
	}*/

	// Light Kill : Do delay between attack
	if (m_bDelayedSwing && gpGlobals->curtime > m_flDelayedSwingTime)
    {
		m_bDelayedSwing = false;
    }

	if ((pOwner->m_nButtons & IN_ATTACK) && m_bCanPrimaryAttack && m_flNextPrimaryAttack <= gpGlobals->curtime)//!Fire )
	{
		//m_bAltattackChecker = false;
		//bIsSecondary = true;
		bIsSecondary = false;
		secondaryGate = false;
		WeaponSound(SINGLE);

		//Do view kick
		AddViewKick();

		if (traceHit.DidHit())
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeAttack1);
			m_flNextPrimaryAttack = gpGlobals->curtime + GetWpnData().fireRate;
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate*1.5f;
		}
		else
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeMiss1);
			m_flNextPrimaryAttack = gpGlobals->curtime + GetWpnData().fireRate * 1.25f;
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate*1.2f;
		}

	}
	if ((pOwner->m_afButtonReleased & IN_ATTACK) && (GetActivity() != GetWpnData().animData[m_bFireMode].MeleeSwing2))
	{
		bIsSecondary = true;
		secondaryGate = false;
	}


	if ((pOwner->m_nButtons & IN_ATTACK2) && 
		bIsSecondary &&
		m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		//Do view kick
		AddViewKick();

		m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate * 2.f;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack;
		SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeSwing2);
		secondaryGate = false;
		bIsSecondary = false;
		m_bCanPrimaryAttack = false;
	}
	if ((pOwner->m_afButtonReleased & IN_ATTACK2))//&& m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		secondaryGate = true;
		m_bCanPrimaryAttack = true;
	}

	if (!bIsSecondary && secondaryGate)
	{
		WeaponSound(SINGLE);

		//Do view kick
		AddViewKick();

		if (traceHit.DidHit())
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeAttack2);
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate * 1.2f;
		}
		else
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeMiss2);
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate * 1.4f;
		}
		secondaryGate = false;
		bIsSecondary = true;
	}

	/*if ( (pOwner->m_afButtonPressed & IN_ATTACK2)
		&& !m_bAltattackAnim 
		&& gpGlobals->curtime > m_flNextSecondaryAttack)
	{
		m_bAltattackChecker = true;
		bIsSecondary = false;

		m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate + 0.2;

		SendWeaponAnim(GetWpnData().MeleeSwing2);
		m_bAltattackAnim = true;

		Fire = true;
	}


	if (Fire)
	{
		Time++;
		if (Time > 28)
		{
			Fire = false;
			Time = 0;
		}
	}

	if ((pOwner->m_afButtonReleased & IN_ATTACK2) 
		&& m_bAltattackAnim 
		&& GetActivity() != GetWpnData().MeleeAttack2
		&& GetActivity() != GetWpnData().MeleeMiss2)
	{
		WeaponSound(SINGLE);

		if (traceHit.DidHit())
		{
			SendWeaponAnim(GetWpnData().MeleeAttack2);
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate + 0.2f;
		}
		else
		{
			SendWeaponAnim(GetWpnData().MeleeMiss2);
			m_flNextSecondaryAttack = gpGlobals->curtime + GetWpnData().fireRate + 0.4f;
		}

		m_bAltattackAnim = false;
	}*/
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CWeaponWrench::ImpactWater(const Vector &start, const Vector &end)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (UTIL_PointContents(start) & (CONTENTS_WATER | CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if (!(UTIL_PointContents(end) & (CONTENTS_WATER | CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(start, end, (CONTENTS_WATER | CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace);

	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect("watersplash", data);
	}

	return true;
}
//#endif