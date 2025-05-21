//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
//#include "weapon_airboatgun.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "AI_Memory.h"
//#include "hl2mp_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "IEffects.h"
#include "trace.h"
#include "SpriteTrail.h"
#include "steamjet.h"

#include "weapon_airboatgun.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_airboat_drying_rate("sk_airboat_drying_rate", "30");
ConVar oc_weapon_airboatgun_alt_fire_radius("oc_weapon_airboatgun_alt_fire_radius", "10");
ConVar oc_weapon_airboatgun_alt_fire_duration("oc_weapon_airboatgun_alt_fire_duration", "2");
ConVar oc_weapon_airboatgun_alt_fire_mass("oc_weapon_airboatgun_alt_fire_mass", "150");

extern ConVar hl2_walkspeed;
extern ConVar hl2_normspeed;
extern ConVar hl2_sprintspeed;




BEGIN_DATADESC(CWeaponAirboatGun)

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponAirboatGun, DT_WeaponAirboatGun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_airboatgun, CWeaponAirboatGun);
PRECACHE_WEAPON_REGISTER(weapon_airboatgun);

acttable_t	CWeaponAirboatGun::m_acttable[] =
{
		{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_CROSSBOW, false },
		{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_CROSSBOW, false },
		{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_CROSSBOW, false },
		{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_CROSSBOW, false },
		{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
		{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
		{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_CROSSBOW, false },
		//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

		{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
		{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },

		{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
		{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
		{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

		{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
		{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
		{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

		{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
		{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
		{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

		{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
		{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
		{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

		{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
		{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
		{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

		{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
		{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
		{ ACT_RUN, ACT_RUN_RIFLE, true },
		{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
		{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },

		{ ACT_IDLE, ACT_IDLE_SMG1, true },
		{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },
		{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SHOTGUN, true },
		{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
		{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
		{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
		{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
		{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
		{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
		{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, false },
		{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
		{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
		{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
		{ ACT_WALK, ACT_WALK_RIFLE, false },


	/*{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE, ACT_IDLE_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },		// FIXME: hook to AR2 unique

	{ ACT_WALK, ACT_WALK_RIFLE, true },

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
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_AR2_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_RANGE_ATTACK_AR2, false },*/

};

IMPLEMENT_ACTTABLE(CWeaponAirboatGun);

CWeaponAirboatGun::CWeaponAirboatGun()
{

}

void CWeaponAirboatGun::Precache(void)
{
	BaseClass::Precache();

	PrecacheParticleSystem("weapon_muzzle_smoke2");
	PrecacheScriptSound("Weapon_AR2.Idle");
	PrecacheScriptSound("FX_RicochetSound.Ricochet");
	PrecacheScriptSound("Bullets.DefaultNearmiss");
	UTIL_PrecacheOther("prop_combine_ball");
	UTIL_PrecacheOther("env_entity_dissolver");
	PrecacheScriptSound("Weapon_AR2.End");
	PrecacheParticleSystem("weapon_muzzle_flash_smoke_small2");
	PrecacheParticleSystem("IBullet_exhaust");
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_AR2.Draw");
	PrecacheScriptSound("Weapon_AR2.LoopFire");

	PrecacheScriptSound("Airboat.FireGunLoop");
	PrecacheScriptSound("Airboat.FireGunRevDown");
	//Ar2CreateSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponAirboatGun::DoImpactEffect(trace_t &tr, int nDamageType)
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;

	DispatchEffect("AR2Impact", data);

	/*	CPASFilter filter(tr.endpos);
	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 0, 180, 180, 4, 30, 0.1, 0);*/

	BaseClass::DoImpactEffect(tr, nDamageType);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAirboatGun::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
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

	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);

	//	FireBullets()->m_iPrimaryAmmoType -> AddAmmoType("AR2",				DMG_SONIC,					TRACER_DYNAMIC_AR2,	"sk_plr_dmg_ar2",			"sk_npc_dmg_ar2",			"sk_max_ar2",			BULLET_IMPULSE(600, 3525), 0 );
	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAirboatGun::FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	WeaponSound(WPN_DOUBLE);

	if (!GetOwner())
		return;

	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if (!pNPC)
		return;

	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecSrc, angShootDir);
		AngleVectors(angShootDir, &vecAiming);
	}
	else
	{
		vecSrc = pNPC->Weapon_ShootPosition();

		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>(pNPC);
		if (pSoldier)
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if (!pNPC->GetEnemy())
				return;

			vecTarget = pNPC->GetEnemy()->BodyTarget(vecSrc);
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize(vecAiming);
	}

	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 0.5f, oc_weapon_airboatgun_alt_fire_duration.GetFloat());
	float flRadius = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 4.0f, oc_weapon_airboatgun_alt_fire_radius.GetFloat());

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(vecSrc,
		vecVelocity,
		flRadius,
		oc_weapon_airboatgun_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAirboatGun::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	if (bSecondary)
	{
		FireNPCSecondaryAttack(pOperator, true);
	}
	else
	{
		// Ensure we have enough rounds in the clip
		m_iClip1++;

		FireNPCPrimaryAttack(pOperator, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAirboatGun::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	case EVENT_WEAPON_AR2_ALTFIRE:
	{
		FireNPCSecondaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

const WeaponProficiencyInfo_t *CWeaponAirboatGun::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
