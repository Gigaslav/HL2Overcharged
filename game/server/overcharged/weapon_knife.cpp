//========= Copyright 2015 L1ght 15 ============================================//
//
// Purpose:		Knife - Best weapon to slash bastards
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
#include "weapon_knife.h"
#include "rumble_shared.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_knife		( "sk_plr_dmg_knife","0");
ConVar    sk_plr_dmg_knife_backstab		( "sk_plr_dmg_knife_backstab","0");
ConVar    sk_npc_dmg_knife		( "sk_npc_dmg_knife","0");
ConVar	  sk_knife_lead_time	( "sk_knife_lead_time", "0.9");
//-----------------------------------------------------------------------------
// CWeaponKnife
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponKnife, DT_WeaponKnife)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_knife, CWeaponKnife );
PRECACHE_WEAPON_REGISTER( weapon_knife );
#endif

acttable_t CWeaponKnife::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },							// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },		// END

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponKnife);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponKnife::CWeaponKnife( void )
{
	ChangeOnce = true;
	m_bBackStab	= true;
}

bool CWeaponKnife::Deploy(void)
{
	ChangeOnce = true;
	return BaseClass::Deploy();
}

Activity CWeaponKnife::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner)
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

float CWeaponKnife::GetDamageForActivity(Activity hitActivity)
{
	if (hitActivity == GetWpnData().animData[m_bFireMode].MeleeAttack1)
		return sk_plr_dmg_knife.GetFloat();

	return sk_plr_dmg_knife_backstab.GetFloat();
}

int CWeaponKnife::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the Wrench!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity();

	// Project where the enemy will be in a little while
	float dt = sk_knife_lead_time.GetFloat();
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponKnife::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{

	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

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

	if (!m_bBackStab)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !m_bBackStab, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = m_bBackStab ? GetWpnData().animData[m_bFireMode].MeleeMiss2 : GetWpnData().animData[m_bFireMode].MeleeMiss1;

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
				gamestats->Event_WeaponHit(pPlayer, !m_bBackStab, GetClassname(), info);
			}
		}

		// Apply an impact effect
		if (m_bBackStab)
			ImpactEffect(traceHit);
		else
			UTIL_DecalTrace(&traceHit, "ManhackCut");

	}

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponKnife::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
	{
		if (!IsNearWall() && 
			!GetOwnerIsRunning())
		{
			HandleAnimEventMeleeHit(pEvent, pOperator);
		}
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}



void CWeaponKnife::ItemPostFrame(void)
{
	if (!m_bInReload && (IsNearWall() || GetOwnerIsRunning()))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.45;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.45;

		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;


	trace_t traceHit;
	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange());
	Vector swingEnd = swingStart + forward * GetRange();
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



	if ((pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)//!Fire )
	{
		WeaponSound(SINGLE);
		m_bBackStab = false;

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
	if (pOwner->m_afButtonReleased & IN_ATTACK)
		m_bBackStab = true;

	if ((pOwner->m_nButtons & IN_ATTACK2)
		&& m_bBackStab
		&& m_flNextSecondaryAttack <= gpGlobals->curtime)
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
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CWeaponKnife::ImpactWater(const Vector &start, const Vector &end)
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