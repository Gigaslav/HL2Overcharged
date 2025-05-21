//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "particle_parse.h"


#include "weapon_dual_pistols.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	DUAL_PISTOLS_FASTEST_REFIRE_TIME		0.03f
#define	DUAL_PISTOLS_FASTEST_DRY_REFIRE_TIME	0.2f

#define	DUAL_PISTOLS_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	DUAL_PISTOLS_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

#define	BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	LASER_SPRITE	"sprites/greenglow1.vmt"//"effects/greenglow1"//"sprites/redglow1.vmt"
//#define	LSPRITE	        "sprites/greenglow1.vmt"//"effects/greenglow1"
#define	LASER_SPRITE1	"sprites/redglow1.vmt"

ConVar	dual_pistols_use_new_accuracy("dual_pistols_use_new_accuracy", "1");
extern ConVar   weapon_laser_pointer;



IMPLEMENT_SERVERCLASS_ST(CWeaponDual_Pistols, DT_WeaponDual_Pistols)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_dual_pistols, CWeaponDual_Pistols);
PRECACHE_WEAPON_REGISTER(weapon_dual_pistols);

BEGIN_DATADESC(CWeaponDual_Pistols)

DEFINE_FIELD(m_flSoonestPrimaryAttack, FIELD_TIME),
DEFINE_FIELD(m_flLastAttackTime, FIELD_TIME),
DEFINE_FIELD(m_flAccuracyPenalty, FIELD_FLOAT), //NOTENOTE: This is NOT tracking game time
DEFINE_FIELD(m_nNumShotsFired, FIELD_INTEGER),

END_DATADESC()

acttable_t CWeaponDual_Pistols::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },		// L1ght 15 : MP animstate
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },	//fix
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },		// END

	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, true },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_PISTOL_LOW, true },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE/*_PISTOL*/, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_PISTOL, false },//always aims
	{ ACT_IDLE_STEALTH, ACT_IDLE_STEALTH_PISTOL, false },

	{ ACT_WALK_RELAXED, ACT_WALK, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_PISTOL, false },//always aims
	{ ACT_WALK_STEALTH, ACT_WALK_STEALTH_PISTOL, false },

	{ ACT_RUN_RELAXED, ACT_RUN, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_PISTOL, false },//always aims
	{ ACT_RUN_STEALTH, ACT_RUN_STEALTH_PISTOL, false },

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_PISTOL, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_ANGRY_PISTOL, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_PISTOL, false },//always aims
	{ ACT_IDLE_AIM_STEALTH, ACT_IDLE_STEALTH_PISTOL, false },

	{ ACT_WALK_AIM_RELAXED, ACT_WALK, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_PISTOL, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_PISTOL, false },//always aims
	{ ACT_WALK_AIM_STEALTH, ACT_WALK_AIM_STEALTH_PISTOL, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_PISTOL, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_PISTOL, false },//always aims
	{ ACT_RUN_AIM_STEALTH, ACT_RUN_AIM_STEALTH_PISTOL, false },//always aims
	//End readiness activities

	// Crouch activities
	{ ACT_CROUCHIDLE_STIMULATED, ACT_CROUCHIDLE_STIMULATED, false },
	{ ACT_CROUCHIDLE_AIM_STIMULATED, ACT_RANGE_AIM_PISTOL_LOW, false },//always aims
	{ ACT_CROUCHIDLE_AGITATED, ACT_RANGE_AIM_PISTOL_LOW, false },//always aims

	// Readiness translations
	{ ACT_READINESS_RELAXED_TO_STIMULATED, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED, false },
	{ ACT_READINESS_RELAXED_TO_STIMULATED_WALK, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK, false },
	{ ACT_READINESS_AGITATED_TO_STIMULATED, ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED, false },
	{ ACT_READINESS_STIMULATED_TO_RELAXED, ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED, false },

	/*{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, false },*/
};


IMPLEMENT_ACTTABLE(CWeaponDual_Pistols);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponDual_Pistols::CWeaponDual_Pistols(void)
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;
	Shoot2 = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDual_Pistols::Precache(void)
{
	BaseClass::Precache();

	PrecacheParticleSystem("weapon_muzzle_smoke");
	PrecacheParticleSystem("weapon_tracer");
	PrecacheParticleSystem("weapon_tracers_incendiary");
	PrecacheParticleSystem("weapon_fx");
	PrecacheParticleSystem("weapon_smoke");
	PrecacheParticleSystem("tracer_trail");
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");

	PrecacheModel("sprites/greenglow1.vmt");
	PrecacheModel(LASER_SPRITE);
	PrecacheModel(LASER_SPRITE1);
	PrecacheModel(BEAM_SPRITE);
	PrecacheScriptSound("Weapon_Pistol.Draw");
	//	PrecacheParticleSystem( "asw_tracer_fx" );
	//	PrecacheParticleSystem( "tracer_sniper_rifle" );
}

Activity CWeaponDual_Pistols::GetDrawActivity(void)
{
	iMuzzle = 2;

	if (m_iClip1 <= 0)
		return ACT_VM_PULLPIN;
	else
		return BaseClass::GetDrawActivity();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponDual_Pistols::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponDual_Pistols::PrimaryAttack(void)
{
	// If my clip is empty (and I use clips) start reload
	/*if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}*/

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	/*if ((gpGlobals->curtime - m_flLastAttackTime) > 0.3f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}*/

	//m_flLastAttackTime = gpGlobals->curtime;
	//m_flSoonestPrimaryAttack = gpGlobals->curtime + DUAL_PISTOLS_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());

	//CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	//m_flAccuracyPenalty += DUAL_PISTOLS_ACCURACY_SHOT_PENALTY_TIME;

	//m_iPrimaryAttacks++;
	//gamestats->Event_WeaponFired(pOwner, true, GetClassname());

	//MuzzleFlashTime = 0.0f;//ѕри каждом выстреле, переменна€ ставитс€ на 0.
	//Add our view kick in
	//AddViewKick();

	//Left side firing///////////////////////////////////////
	/*if ((gpGlobals->curtime - m_flLastAttackTime) > 0.3f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + DUAL_PISTOLS_FASTEST_REFIRE_TIME;*/

	pPlayer->DoMuzzleFlash();
	BaseClass::PrimaryAttack();

	if (iMuzzle == 1)
	{
		iMuzzle = 2;
		//LEFT
	}
	else if (iMuzzle == 2)
	{
		iMuzzle = 1;
	}

}

bool CWeaponDual_Pistols::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	iMuzzle = 2;
	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponDual_Pistols::Drop(const Vector &vecVelocity)
{
	if (!cvar->FindVar("oc_allow_pick_dual_pistols")->GetInt())
		BaseClass::Drop(vecVelocity);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		Vector vecForward;
		pPlayer->EyeVectors(&vecForward);

		CBaseCombatWeapon *pPistol = NULL;
		pPistol = (CBaseCombatWeapon*)CBaseEntity::Create("weapon_pistol", pPlayer->Weapon_ShootPosition(), pPlayer->GetAbsAngles(), pPlayer); //0 60 0

		pPistol->m_iClip1 = Clamp(this->Clip1() / 2, 0, pPistol->GetDefaultClip1());
		pPistol->m_iClip2 = Clamp(this->Clip2() / 2, 0, pPistol->GetDefaultClip2());

		pPistol->SetPrimaryAmmoCount(this->GetPrimaryAmmoCount());
		pPistol->SetSecondaryAmmoCount(this->GetSecondaryAmmoCount());

		if (pPistol)
		{
			//pPistol->SetAbsVelocity(vecForward * 320 + pPlayer->GetAbsVelocity());
		}

		//pPistol->SetPickableDelay(true);
		pPistol->Drop(vecForward * 320 + pPlayer->GetAbsVelocity());

		UTIL_Remove(this);
	}

	BaseClass::Drop(vecVelocity);
}

bool CWeaponDual_Pistols::Reload(void)
{
	iMuzzle = 2;

	return BaseClass::Reload();
}
