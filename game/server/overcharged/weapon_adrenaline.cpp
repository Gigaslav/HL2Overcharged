//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Adrenaline - heal weapon
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
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar sk_plr_dmg_adrenaline( "sk_plr_dmg_adrenaline", "-35" );

//-----------------------------------------------------------------------------
// CWeaponAdrenaline
//-----------------------------------------------------------------------------

class CWeaponAdrenaline : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponAdrenaline, CBaseHLCombatWeapon );

	CWeaponAdrenaline(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	//void	ItemPostFrame( void );
	void	PrimaryAttack( void );
	//void	SecondaryAttack( void );
	//bool	Reload( void );
	//bool	HealPlayer( void );

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponAdrenaline, DT_WeaponAdrenaline)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_adrenaline, CWeaponAdrenaline );
PRECACHE_WEAPON_REGISTER( weapon_adrenaline );

BEGIN_DATADESC( CWeaponAdrenaline )
END_DATADESC()

acttable_t CWeaponAdrenaline::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponAdrenaline);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponAdrenaline::CWeaponAdrenaline( void )
{

}

void CWeaponAdrenaline::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponAdrenaline::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	SendWeaponAnim(GetPrimaryAttackActivity());
	m_iClip1 = m_iClip1 - 1;

	WeaponSound( SINGLE );

	pOwner->SetAnimation( PLAYER_ATTACK1 );				// Light Kill : Forgotten anim ?

	if ( pOwner->IsAlive() && pOwner->GetHealth() < pOwner->GetMaxHealth())
	{
		pOwner->TakeHealth( 20, DMG_GENERIC );
	}
	else
	{
		pOwner->TakeHealth( -35, DMG_GENERIC ); // Light Kill : else overdose :p
	}

	color32 ShockEffect = { 255,255,255,255 };
    UTIL_ScreenFade( pOwner, ShockEffect, 0.2f, 0.0f, FFADE_IN );

	pOwner->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) );

	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}