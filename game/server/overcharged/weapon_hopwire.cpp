//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "grenade_hopwire.h"

#include "weapon_hopwire.h"
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CWeaponHopwire )
	DEFINE_FIELD( m_hActiveHopWire, FIELD_EHANDLE ),
END_DATADESC()

acttable_t	CWeaponHopwire::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_GRENADE,                    false },	// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_GRENADE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_GRENADE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_GRENADE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_GRENADE,        false },
    { ACT_HL2MP_JUMP,	ACT_HL2MP_JUMP_SLAM,                    false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponHopwire);

IMPLEMENT_SERVERCLASS_ST(CWeaponHopwire, DT_WeaponHopwire)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_hopwire, CWeaponHopwire );
PRECACHE_WEAPON_REGISTER(weapon_hopwire);


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CWeaponHopwire::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_hActiveHopWire != NULL )
		return false;

	return BaseClass::Holster( pSwitchingTo );
}*/


//-----------------------------------------------------------------------------
// Purpose: Override the ammo behavior so we never disallow pulling the weapon out
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHopwire::HasAnyAmmo( void )
{
	/*if ( m_hActiveHopWire != NULL )
		return true;*/

	return BaseClass::HasAnyAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponHopwire::ItemPostFrame( void )
{

	if( m_fDrawbackFinished )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if 
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
		}
		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) )
				{
					SendWeaponAnim( ACT_VM_THROW );
					m_fDrawbackFinished = false;
				}
				break;

			case GRENADE_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & (IN_ATTACK|IN_ATTACK2)) )
				{
					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}

					m_fDrawbackFinished = false;
				}
				break;

			default:
				break;
			}
		}
	}

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopwire::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 1200;
//#ifdef HL2_EPISODIC//OverCharged
	m_hActiveHopWire = static_cast<CGrenadeHopwire *> (HopWire_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, GRENADE_TIMER ));
	m_hActiveHopWire->SetTimer(5.f);
//#endif
	//CGrenadeHopwire *hopwire = static_cast<CGrenadeHopwire *> (HopWire_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER));
	//CGravityVortexController *pGravGren = (CGravityVortexController*)Create("grenade_molotov", /*vecSrc*/ vecSrc, vec3_angle, GetOwner());
	//hopwire->SetTimer(5.f);

	m_bRedraw = true;

	WeaponSound( SINGLE );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopwire::LobGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	
	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 350 + Vector( 0, 0, 50 );
//#ifdef HL2_EPISODIC//OverCharged
	m_hActiveHopWire = static_cast<CGrenadeHopwire *> (HopWire_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(200,random->RandomInt(-600,600),0), pPlayer, GRENADE_TIMER ));
	m_hActiveHopWire->SetTimer(5.f);
//#endif
	WeaponSound( WPN_DOUBLE );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopwire::RollGrenade( CBasePlayer *pPlayer )
{
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vecFacing * 700;
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	AngularImpulse rotSpeed(0,0,720);
//#ifdef HL2_EPISODIC//OverCharged
	m_hActiveHopWire = static_cast<CGrenadeHopwire *> (HopWire_Create( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER ));
	m_hActiveHopWire->SetTimer(5.f);
//#endif
	WeaponSound( SPECIAL1 );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;
}
