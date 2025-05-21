//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This is the molotov weapon
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "AI_Memory.h"
#include "player.h"
#include "gamerules.h"		// For g_pGameRules
#include "weapon_molotov.h"
#include "grenade_molotov.h"
#include "in_buttons.h"
#include "game.h"			
#include "vstdlib/random.h"
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CWeaponMolotov )

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponMolotov, DT_WeaponMolotov)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_molotov, CWeaponMolotov );
PRECACHE_WEAPON_REGISTER(weapon_molotov);

acttable_t	CWeaponMolotov::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_GRENADE,                    false },	// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_GRENADE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_GRENADE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_GRENADE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_GRENADE,        false },
    { ACT_HL2MP_JUMP,	ACT_HL2MP_JUMP_SLAM,                    false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};
IMPLEMENT_ACTTABLE(CWeaponMolotov);


void CWeaponMolotov::Precache( void )
{
	PrecacheParticleSystem("weapon_molotov_flame");
	PrecacheModel ( cvar->FindVar("oc_weapon_molotov_model")->GetString() );
	BaseClass::Precache();
}

void CWeaponMolotov::LobGrenade(CBasePlayer *pPlayer)
{
	ThrowGrenade(pPlayer);
}

void CWeaponMolotov::RollGrenade(CBasePlayer *pPlayer)
{
	ThrowGrenade(pPlayer);
}

void CWeaponMolotov::ThrowGrenade(CBasePlayer *pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);

	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;

	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	vForward[2] += 0.1f;

	Vector vecThrow;

	pPlayer->GetVelocity(&vecThrow, NULL);

	vecThrow += vForward * 1200;

	CGrenade_Molotov *pMolotov = (CGrenade_Molotov*)Create("grenade_molotov", vecSrc, vec3_angle, GetOwner());

	if (!pMolotov)
	{
		Msg("Couldn't make molotov!\n");
		return;
	}

	pMolotov->SetAbsVelocity(vecThrow);

	// Tumble through the air
	QAngle angVel( random->RandomFloat ( -100, -500 ), random->RandomFloat ( -100, -500 ), random->RandomFloat ( -100, -500 ) ); 

	pMolotov->SetLocalAngularVelocity( angVel );

	pMolotov->SetThrower( GetOwner() );

	pMolotov->SetOwnerEntity( ((CBaseEntity*)GetOwner()) );
}