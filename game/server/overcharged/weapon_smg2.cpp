//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "basecombatcharacter.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "globalstate.h"
#include "particle_parse.h"
#include "gamestats.h"
#include "in_buttons.h"

#include "weapon_smg2.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar   weapon_laser_pointer;

IMPLEMENT_SERVERCLASS_ST(CWeaponSMG2, DT_WeaponSMG2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_smg2, CWeaponSMG2);
PRECACHE_WEAPON_REGISTER(weapon_smg2);

BEGIN_DATADESC(CWeaponSMG2)
END_DATADESC()

acttable_t	CWeaponSMG2::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SMG1,                false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },

	//	{ ACT_ARM,				ACT_ARM_PISTOL,					true },
	//	{ ACT_DISARM,			ACT_DISARM_PISTOL,				true },
};

IMPLEMENT_ACTTABLE(CWeaponSMG2);

#define TOOCLOSETIMER_OFF	0.0f
#define ALYX_TOOCLOSETIMER	1.0f		// Time an enemy must be tooclose before Alyx is allowed to shoot it.

//=========================================================
CWeaponSMG2::CWeaponSMG2()
{
	m_fMinRange1 = 1;
	m_fMaxRange1 = 5000;
	Shoot = 0;
	m_flTooCloseTimer = TOOCLOSETIMER_OFF;
#ifdef HL2_EPISODIC
	m_fMinRange1 = 60;
	m_fMaxRange1 = 2048;
#endif//HL2_EPISODIC
}

CWeaponSMG2::~CWeaponSMG2()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG2::Precache(void)
{
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_SMG2.Draw");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSMG2::Equip(CBaseCombatCharacter *pOwner)
{
	BaseClass::Equip(pOwner);
}

//-----------------------------------------------------------------------------
// Purpose: Try to encourage Alyx not to use her weapon at point blank range,
//			but don't prevent her from defending herself if cornered.
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponSMG2::WeaponRangeAttack1Condition(float flDot, float flDist)
{
#ifdef HL2_EPISODIC

	if (flDist < m_fMinRange1)
	{
		// If Alyx is not able to fire because an enemy is too close, start a timer.
		// If the condition persists, allow her to ignore it and defend herself. The idea
		// is to stop Alyx being content to fire point blank at enemies if she's able to move
		// away, without making her defenseless if she's not able to move.
		float flTime;

		if (m_flTooCloseTimer == TOOCLOSETIMER_OFF)
		{
			m_flTooCloseTimer = gpGlobals->curtime;
		}

		flTime = gpGlobals->curtime - m_flTooCloseTimer;

		if (flTime > ALYX_TOOCLOSETIMER)
		{
			// Fake the range to allow Alyx to shoot.
			flDist = m_fMinRange1 + 1.0f;
		}
	}
	else
	{
		m_flTooCloseTimer = TOOCLOSETIMER_OFF;
	}

	int nBaseCondition = BaseClass::WeaponRangeAttack1Condition(flDot, flDist);

	// While in a vehicle, we extend our aiming cone (this relies on COND_NOT_FACING_ATTACK 
	// TODO: This needs to be rolled in at the animation level
	if (GetOwner()->IsInAVehicle())
	{
		Vector vecRoughDirection = (GetOwner()->GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter());
		Vector vecRight;
		GetVectors(NULL, &vecRight, NULL);
		bool bRightSide = (DotProduct(vecRoughDirection, vecRight) > 0.0f);
		float flTargetDot = (bRightSide) ? -0.7f : 0.0f;

		if (nBaseCondition == COND_NOT_FACING_ATTACK && flDot >= flTargetDot)
		{
			nBaseCondition = COND_CAN_RANGE_ATTACK1;
		}
	}

	return nBaseCondition;

#else 

	return BaseClass::WeaponRangeAttack1Condition(flDot, flDist);

#endif//HL2_EPISODIC
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponSMG2::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	return COND_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponSMG2::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	WeaponSound(SINGLE_NPC);

	if (hl2_episodic.GetBool())
	{
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
	else
	{
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	}

	pOperator->DoMuzzleFlash();

	if (hl2_episodic.GetBool())
	{
		// Never fire Alyx's last bullet just in case there's an emergency
		// and she needs to be able to shoot without reloading.
		if (m_iClip1 > 1)
		{
			m_iClip1 = m_iClip1 - 1;
		}
	}
	else
	{
		m_iClip1 = m_iClip1 - 1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG2::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	// HACK: We need the gun to fire its muzzle flash
	if (bSecondary == false)
	{
		SetActivity(ACT_RANGE_ATTACK_PISTOL, 0.0f);
	}

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
void CWeaponSMG2::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;
	/*case EVENT_WEAPON_PISTOL_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
		break;
	}*/

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not Alyx is in the injured mode
//-----------------------------------------------------------------------------
/*bool IsAlyxInInjuredMode(void)
{
	if (hl2_episodic.GetBool() == false)
		return false;

	return (GlobalEntity_GetState("ep2_alyx_injured") == GLOBAL_ON);
}*/

//-----------------------------------------------------------------------------
/*const Vector& CWeaponSMG2::GetBulletSpread(void)
{
	static const Vector cone = VECTOR_CONE_2DEGREES;
	static const Vector injuredCone = VECTOR_CONE_6DEGREES;

/*	if (IsAlyxInInjuredMode())
		return injuredCone;*/

	/*return cone;
}*/
/*
//-----------------------------------------------------------------------------
float CWeaponSMG2::GetMinRestTime(void)
{
	if (IsAlyxInInjuredMode())
		return 1.5f;

	return BaseClass::GetMinRestTime();
}
*//*
//-----------------------------------------------------------------------------
float CWeaponSMG2::GetMaxRestTime(void)
{
	if (IsAlyxInInjuredMode())
		return 3.0f;

	return BaseClass::GetMaxRestTime();
}*/
