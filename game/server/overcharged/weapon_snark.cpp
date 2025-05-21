//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		HL1 snark monster remake in modern form
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
#include "engine/IEngineSound.h"
#include "npc_snark.h"
#include "beam_shared.h"

#include "weapon_snark.h"
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( weapon_snark, CWeaponSnark );
PRECACHE_WEAPON_REGISTER( weapon_snark );

IMPLEMENT_SERVERCLASS_ST( CWeaponSnark, DT_WeaponSnark )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponSnark )
	DEFINE_FIELD( m_bThrowState, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t	CWeaponSnark::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },	// BJ : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SLAM, false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },

	{ ACT_IDLE_RELAXED, ACT_IDLE, false },
	{ ACT_RUN_RELAXED, ACT_RUN, false },
	{ ACT_WALK_RELAXED, ACT_WALK, false },
};

IMPLEMENT_ACTTABLE(CWeaponSnark);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSnark::CWeaponSnark( void )
{
	m_bReloadsSingly	= false;
	m_bThrowState		= false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSnark::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther("npc_snark");
	PrecacheScriptSound("WpnSnark.PrimaryAttack");
	PrecacheScriptSound("WpnSnark.Deploy");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSnark::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	Vector vecForward;
	pPlayer->EyeVectors( &vecForward );

	// find place to toss monster
	// Does this need to consider a crouched player?
	Vector vecStart	= pPlayer->WorldSpaceCenter() + (vecForward * 20);
	Vector vecEnd	= vecStart + (vecForward * 44);
	trace_t tr;
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.allsolid || tr.startsolid || tr.fraction <= 0.25 )
		return;

	// player "shoot" animation
	//SendWeaponAnim( ACT_VM_THROW );
	SendWeaponAnim(GetWpnData().animData[0].ThrowPrimary);
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	QAngle spawnAng;
	VectorAngles(pPlayer->BodyDirection2D(), spawnAng);

	Vector Origin;
	Origin = GetClientMuzzleVector();//pPlayer->Weapon_ShootPosition()

	CNPC_snark *pSnark = (CNPC_snark*)Create("npc_snark", Origin, spawnAng, pPlayer);

	if (pSnark)
	{
		pSnark->SetGroundEntity(NULL);
		pSnark->AddSpawnFlags(SF_NPC_FALL_TO_GROUND);
		pSnark->SetAbsVelocity(vecForward * 340 + pPlayer->GetAbsVelocity());

		pSnark->AddClassRelationship(CLASS_PLAYER, D_HT, 9999);
	}

	// play hunt sound
	CPASAttenuationFilter filter( this );
	EmitSound(filter, entindex(), SNARK_THROW_SOUND);

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 200, 0.2 );

	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

	m_bThrowState = true;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
	SetWeaponIdleTime(m_flNextPrimaryAttack);
	
	//m_flNextPrimaryAttack = gpGlobals->curtime + 0.6;	//0.3
	//SetWeaponIdleTime( gpGlobals->curtime + 1.0 );
}

void CWeaponSnark::SecondaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);

	// find place to toss monster
	// Does this need to consider a crouched player?
	Vector vecStart = pPlayer->WorldSpaceCenter() + (vecForward * 20);
	Vector vecEnd = vecStart + (vecForward * 44);
	trace_t tr;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
	if (tr.allsolid || tr.startsolid || tr.fraction <= 0.25)
		return;

	// player "shoot" animation
	//SendWeaponAnim( ACT_VM_THROW );
	SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);
	pPlayer->SetAnimation(PLAYER_ATTACK1);
	
	QAngle spawnAng;
	VectorAngles(pPlayer->BodyDirection2D(), spawnAng);

	Vector Origin;
	Origin = GetClientMuzzleVector();//pPlayer->Weapon_ShootPosition()

	CNPC_snark *pSnark = (CNPC_snark*)Create("npc_snark", Origin, spawnAng, pPlayer);

	if (pSnark)
	{
		pSnark->SetGroundEntity(NULL);
		pSnark->AddSpawnFlags(SF_NPC_FALL_TO_GROUND);
		pSnark->SetAbsVelocity(vecForward * 150 + pPlayer->GetAbsVelocity());

		pSnark->AddClassRelationship(CLASS_PLAYER, D_HT, 9999);
	}

	// play hunt sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), SNARK_THROW_SOUND);

	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 200, 0.2);

	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

	m_bThrowState = true;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
	SetWeaponIdleTime(m_flNextPrimaryAttack);

	//m_flNextSecondaryAttack = gpGlobals->curtime + 1.3;		//0.3
	//SetWeaponIdleTime(gpGlobals->curtime + 1.0);
}

void CWeaponSnark::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_bThrowState )
	{
		m_bThrowState = false;

		if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			if ( !pPlayer->SwitchToNextBestWeapon( pPlayer->GetActiveWeapon() ) )
				Holster();
		}
		else
		{
			SendWeaponAnim( ACT_VM_DRAW );
			SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
		}
	}
	else
	{
		//if ( random->RandomFloat( 0, 1 ) <= 0.75 )
		//{
			SendWeaponAnim( ACT_VM_IDLE );
		//}
		//else
		//{
		//	SendWeaponAnim( ACT_VM_FIDGET );
		//}
	}
}

bool CWeaponSnark::Deploy( void )
{
	CPASAttenuationFilter filter( this );
	EmitSound(filter, entindex(), SNARK_DRAW_SOUND);

	return BaseClass::Deploy();
}

bool CWeaponSnark::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
	{
		return false;
	}

	if ( !BaseClass::Holster( pSwitchingTo ) )
	{
		return false;
	}

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		SetThink( &CWeaponSnark::DestroyItem );
		SetNextThink( gpGlobals->curtime + 0.1 );
	}

	pPlayer->SetNextAttack( gpGlobals->curtime + 0.5 );

	return true;
}
