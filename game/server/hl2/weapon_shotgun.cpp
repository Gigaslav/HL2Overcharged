//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A shotgun.
//
//			Primary attack: single barrel shot.
//			Secondary attack: double barrel shot.
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "weapon_shotgun.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"		// For g_pGameRules
#include "in_buttons.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "gamestats.h"
/////////NEW///////////////
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "beam_shared.h"//For Laser
#include "Sprite.h"//For laser
#include "baseviewmodel_shared.h"
#include "OverCharged/ShotgunBullet.h"
/////////////////////////////////
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
extern ConVar weapon_laser_pointer;

extern ConVar sk_plr_num_shotgun_pellets;
extern ConVar sk_plr_num_shotgun_pellets2;

ConVar   oc_weapon_shotgun_animation("oc_weapon_shotgun_animation", "0");
ConVar   oc_weapon_shotgun_animation_pump("oc_weapon_shotgun_animation_pump", "0");

#define	BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	LASER_SPRITE	"sprites/greenglow1.vmt"//"effects/greenglow1"//"sprites/redglow1.vmt"
#define	LSPRITE	        "sprites/greenglow1.vmt"

IMPLEMENT_SERVERCLASS_ST(CWeaponShotgun, DT_WeaponShotgun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_shotgun, CWeaponShotgun );
PRECACHE_WEAPON_REGISTER(weapon_shotgun);

BEGIN_DATADESC( CWeaponShotgun )
DEFINE_FIELD(PumpAfterTwoBarrels, FIELD_BOOLEAN),
	DEFINE_FIELD( m_bNeedPump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDelayedFire1, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDelayedFire2, FIELD_BOOLEAN ),
	DEFINE_FIELD(ChangeOnce, FIELD_BOOLEAN),
	DEFINE_FIELD(wasAnimated, FIELD_BOOLEAN),
END_DATADESC()

acttable_t	CWeaponShotgun::m_acttable[] =
{
	/*{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, false },*/

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeaponShotgun);

void CWeaponShotgun::Precache( void )
{
	CBaseCombatWeapon::Precache();
	PrecacheParticleSystem("weapon_muzzle_smoke");
	PrecacheScriptSound("FX_RicochetSound.Ricochet");
	PrecacheScriptSound("Bullets.DefaultNearmiss");
	PrecacheModel("sprites/greenglow1.vmt");
	PrecacheModel(LASER_SPRITE);
	PrecacheModel(LSPRITE);
	PrecacheModel(BEAM_SPRITE);
	PrecacheScriptSound("Weapon_Shotgun.Draw");
	PumpAfterTwoBarrels = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponShotgun::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
	FireBulletsInfo_t info;

	info.m_iTracerFreq = 1;
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
	info.m_vecSpread = pOperator->GetAttackSpread(this);
	if (((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 0)) || ((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 2)))
	{
		pOperator->FireBullets(9, vecShootOrigin, vecShootDir, info.m_vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
	else if ((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 1))
	{
		pOperator->FireBulletsShotgun(9, vecShootOrigin, vecShootDir, info.m_vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack( pOperator, true );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			FireNPCPrimaryAttack( pOperator, false );
		}
		break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:	When we shipped HL2, the shotgun weapon did not override the
//			BaseCombatWeapon default rest time of 0.3 to 0.6 seconds. When
//			NPC's fight from a stationary position, their animation events
//			govern when they fire so the rate of fire is specified by the
//			animation. When NPC's move-and-shoot, the rate of fire is 
//			specifically controlled by the shot regulator, so it's imporant
//			that GetMinRestTime and GetMaxRestTime are implemented and provide
//			reasonable defaults for the weapon. To address difficulty concerns,
//			we are going to fix the combine's rate of shotgun fire in episodic.
//			This change will not affect Alyx using a shotgun in EP1. (sjb)
//-----------------------------------------------------------------------------
float CWeaponShotgun::GetMinRestTime()
{
	if( hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
	{
		return 1.2f;
	}
	
	return BaseClass::GetMinRestTime();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CWeaponShotgun::GetMaxRestTime()
{
	if( hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
	{
		return 1.5f;
	}

	return BaseClass::GetMaxRestTime();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

bool CWeaponShotgun::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool rRet;
	pSwitchingTo = NULL;
	rRet = BaseClass::Holster(pSwitchingTo);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
		m_bReloadComplete = 0;

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponShotgun::CWeaponShotgun( void )
{
	m_bReloadsSingly = true;
	ChangeOnce = true;

	m_bNeedPump		= false;
	m_bDelayedFire1 = false;
	m_bDelayedFire2 = false;

	m_fMinRange1		= 0.0;
	m_fMaxRange1		= 500;
	m_fMinRange2		= 0.0;
	m_fMaxRange2		= 200;

	wasAnimated = false;
}
