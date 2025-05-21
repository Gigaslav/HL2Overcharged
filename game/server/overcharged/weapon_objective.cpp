//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		
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

ConVar aa_wpn_objective_use( "aa_wpn_objective_use", "0", FCVAR_REPLICATED, "Use only in marked place or by map script." );

//-----------------------------------------------------------------------------
// CWeaponObjective
//-----------------------------------------------------------------------------

class CWeaponObjective : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponObjective, CBaseHLCombatWeapon );

	CWeaponObjective(void);

	DECLARE_SERVERCLASS();

	//void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponObjective, DT_WeaponObjective)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_Objective, CWeaponObjective );
PRECACHE_WEAPON_REGISTER( weapon_Objective );

BEGIN_DATADESC( CWeaponObjective )
END_DATADESC()

acttable_t CWeaponObjective::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE,					false },
	{ ACT_WALK,						ACT_WALK,					false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },

	{ ACT_HL2MP_IDLE,               ACT_HL2MP_IDLE_SLAM,                  false },	//new
    { ACT_HL2MP_RUN,                ACT_HL2MP_RUN_SLAM,                    false },

    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,       false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,       false },

    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,             false },
};

IMPLEMENT_ACTTABLE(CWeaponObjective);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponObjective::CWeaponObjective( void )
{
	m_bFiresUnderwater	= false;
}

/*void CWeaponObjective::Precache( void )
{
	BaseClass::Precache();
}*/


void CWeaponObjective::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if (cvar->FindVar("aa_wpn_objective_use")->GetInt() == 0)
	{
		ClientPrint( pOwner, HUD_PRINTCENTER, "Map specific item. Useable only in marked position" );
		m_flNextPrimaryAttack = gpGlobals->curtime + 6;
	}
	else
	{
		/*#ifdef CLIENT_DLL
		engine->ClientCmd( "aa_weapondrop" );
		#endif*/
		//engine->ClientCmd( "aa_weapondrop" );
		engine->ClientCommand( pOwner->edict(), "aa_weapondrop" );
		m_flNextPrimaryAttack = gpGlobals->curtime + 1;
	}
}

void CWeaponObjective::SecondaryAttack( void )
{
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );	// L1ght 15 : Inspect animation
	//SendWeaponAnim( ACT_VM_DRAW );
	m_flNextSecondaryAttack	= gpGlobals->curtime + 10;
}


/*

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


//-----------------------------------------------------------------------------
// CWeaponUseableItem
//-----------------------------------------------------------------------------

class CWeaponUseableItem : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponUseableItem, CBaseHLCombatWeapon );

	CWeaponUseableItem(void);

	DECLARE_SERVERCLASS();

	void	PrimaryAttack( void );
	void	SecondaryAttack( void );

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponUseableItem, DT_WeaponUseableItem)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_useableitem, CWeaponUseableItem );
PRECACHE_WEAPON_REGISTER( weapon_useableitem );

BEGIN_DATADESC( CWeaponUseableItem )
END_DATADESC()

acttable_t CWeaponUseableItem::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE,					false },
	{ ACT_WALK,						ACT_WALK,					false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },

	{ ACT_HL2MP_IDLE,               ACT_HL2MP_IDLE_SLAM,                  false },	//new
    { ACT_HL2MP_RUN,                ACT_HL2MP_RUN_SLAM,                    false },

    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,       false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,       false },

    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,             false },
};

IMPLEMENT_ACTTABLE(CWeaponUseableItem);

CWeaponUseableItem::CWeaponUseableItem( void )
{
	m_bFiresUnderwater	= false;
}


void CWeaponUseableItem::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	ClientPrint( pOwner, HUD_PRINTCENTER, "Map specific item. Useable only in marked position" );
	//SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	m_flNextPrimaryAttack = gpGlobals->curtime + 4;
}

void CWeaponUseableItem::SecondaryAttack( void )
{
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );	// L1ght 15 : Inspect animation
	m_flNextSecondaryAttack	= gpGlobals->curtime + 10;
}
*/