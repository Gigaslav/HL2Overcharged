//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pocket Teleport
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
//#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar aa_wpn_teleport_target( "aa_wpn_teleport_target", "teleport_special", FCVAR_REPLICATED, "Teleport to the named entity in cvar. Usualy made to work with info_teleport_destination" );

//-----------------------------------------------------------------------------
// CWeaponTeleport
//-----------------------------------------------------------------------------

class CWeaponTeleport : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponTeleport, CBaseHLCombatWeapon );

	CWeaponTeleport(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );
	void	PrimaryAttack( void );
	void	DelayedAttack( void );

	string_t m_iLandmark;	// teleport

protected:

	float					m_flDelayedFire;
	bool					m_bShotDelayed;

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponTeleport, DT_WeaponTeleport)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_Teleport, CWeaponTeleport );
PRECACHE_WEAPON_REGISTER( weapon_Teleport );

BEGIN_DATADESC( CWeaponTeleport )
	DEFINE_KEYFIELD( m_iLandmark, FIELD_STRING, "landmark" ),	// teleport
END_DATADESC()

acttable_t CWeaponTeleport::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_GRENADE,                    false },	// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_GRENADE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_GRENADE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_GRENADE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_GESTURE_RELOAD_SMG1,        false },
    { ACT_HL2MP_JUMP,	ACT_HL2MP_JUMP_SLAM,                    false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponTeleport);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponTeleport::CWeaponTeleport( void )
{
	m_bFiresUnderwater	= false;
}

void CWeaponTeleport::Precache( void )
{
	BaseClass::Precache();

}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTeleport::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ((cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1) || (cvar->FindVar("oc_state_is_running")->GetInt() == 1))
	{
		return;
	}

	if ( m_bShotDelayed )
		return;

	// Cannot fire underwater
	if ( GetOwner() && GetOwner()->GetWaterLevel() == 3 )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	m_bShotDelayed = true;
	m_flDelayedFire = gpGlobals->curtime + 3.0f;

	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

void CWeaponTeleport::ItemPostFrame( void )
{
	// See if we need to fire off our secondary round
	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
	{
		DelayedAttack();
	}

	BaseClass::ItemPostFrame();
}

void CWeaponTeleport::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	CBaseEntity	*pentTarget = NULL;
	// The activator and caller are the same
	pentTarget = gEntList.FindEntityByName( pentTarget, /*m_target*/ /*"tp_dark_room"*/ aa_wpn_teleport_target.GetString(), NULL, pOwner, pOwner );
	if (!pentTarget)
	{
		//ClientPrint( pOwner, HUD_PRINTCENTER, "No teleports found" );
		return;
	}

	//ClientPrint( pOwner, HUD_PRINTCENTER, "Teleport Found" );

	CBaseEntity	*pentLandmark = NULL;
	Vector vecLandmarkOffset(0, 0, 0);
	if (m_iLandmark != NULL_STRING)
	{
		// The activator and caller are the same
		pentLandmark = gEntList.FindEntityByName(pentLandmark, m_iLandmark, NULL, pOwner, pOwner );
		if (pentLandmark)
		{
			vecLandmarkOffset = pOwner->GetAbsOrigin() - pentLandmark->GetAbsOrigin();
		}
	}


	pOwner->SetGroundEntity( NULL );
	
	Vector tmp = pentTarget->GetAbsOrigin();

	if (!pentLandmark && pOwner->IsPlayer())
	{
		// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
		tmp.z -= pOwner->WorldAlignMins().z;
	}

	//
	// Only modify the toucher's angles and zero their velocity if no landmark was specified.
	//
	const QAngle *pAngles = NULL;
	Vector *pVelocity = NULL;

	if (!pentLandmark && !HasSpawnFlags(0x20) )	//SF_TELEPORT_PRESERVE_ANGLES) )
	{
		pAngles = &pentTarget->GetAbsAngles();

		pVelocity = NULL;	//BUGBUG - This does not set the player's velocity to zero!!!
	}

	tmp += vecLandmarkOffset;
	pOwner->Teleport( &tmp, pAngles, pVelocity );

	//==
	
	WeaponSound( SINGLE );

	m_iClip1 = m_iClip1 - 1;

	color32 ShockEffect = { 255,255,255,255 };
    UTIL_ScreenFade( pOwner, ShockEffect, 0.2f, 1.0f, FFADE_IN );
}
