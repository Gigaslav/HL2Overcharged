//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "weapon_pistol.h"
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
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "beam_shared.h"//For Laser
#include "Sprite.h"//For laser

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	pistol_use_new_accuracy( "pistol_use_new_accuracy", "1" );

IMPLEMENT_SERVERCLASS_ST(CWeaponPistol, DT_WeaponPistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_pistol, CWeaponPistol );
PRECACHE_WEAPON_REGISTER( weapon_pistol );

BEGIN_DATADESC( CWeaponPistol )

	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeaponPistol::m_acttable[] =
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
};


IMPLEMENT_ACTTABLE( CWeaponPistol );

CWeaponPistol::CWeaponPistol( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;
}

void CWeaponPistol::Precache(void)
{
	BaseClass::Precache();
}
void CWeaponPistol::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;


		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

/*void CWeaponPistol::Drop(const Vector &vecVelocity)
{
	if (!cvar->FindVar("oc_allow_pick_dual_pistols")->GetInt())
		BaseClass::Drop(vecVelocity);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		CBaseCombatWeapon *pDPistols = (CBaseCombatWeapon*)pPlayer->HasNamedPlayerItem("weapon_dual_pistols");
		if (pDPistols)
		{
			Vector vecForward;
			pPlayer->EyeVectors(&vecForward);

			CBaseCombatWeapon *pPistol = NULL;
			pPistol = (CBaseCombatWeapon*)CBaseEntity::Create("weapon_pistol", pPlayer->Weapon_ShootPosition(), pPlayer->GetAbsAngles(), pPlayer); //0 60 0

			pPistol->m_iClip1 = Clamp(this->Clip1() / 2, 0, pPistol->GetDefaultClip1());
			pPistol->m_iClip2 = Clamp(this->Clip2() / 2, 0, pPistol->GetDefaultClip2());

			pPistol->SetPrimaryAmmoCount(this->GetPrimaryAmmoCount());
			pPistol->SetSecondaryAmmoCount(this->GetSecondaryAmmoCount());

			pPistol->Drop(vecForward * 320 + pPlayer->GetAbsVelocity());

			UTIL_Remove(pDPistols);
		}
	}
}*/