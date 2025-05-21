//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "weapon_frag.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CWeaponFrag )
	DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_AttackPaused, FIELD_INTEGER ),
	DEFINE_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t	CWeaponFrag::m_acttable[] = 
{
	//{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SLAM, false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },

	{ ACT_IDLE_RELAXED, ACT_IDLE, false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED, ACT_RUN, false },
	{ ACT_WALK_RELAXED, ACT_WALK, false },
};

IMPLEMENT_ACTTABLE(CWeaponFrag);

IMPLEMENT_SERVERCLASS_ST(CWeaponFrag, DT_WeaponFrag)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_frag, CWeaponFrag );
PRECACHE_WEAPON_REGISTER(weapon_frag);



/*CWeaponFrag::CWeaponFrag() :
	CBaseHLCombatWeapon(),
	m_bRedraw( false )
{
	NULL;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "npc_grenade_frag" );

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponFrag::Deploy( void )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		m_bRedraw = false;
		m_fDrawbackFinished = false;
	}

	if (!HasPrimaryAmmo())
	{
		return false;
	}

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFrag::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		m_bRedraw = false;
		m_fDrawbackFinished = false;
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponFrag::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			break;

		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			m_bRedraw = true;
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW2:
			RollGrenade( pOwner );
			m_bRedraw = true;
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW3:
			LobGrenade( pOwner );
			m_bRedraw = true;
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	0.5
	if( fThrewGrenade )
	{
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!

		// Make a sound designed to scare snipers back into their holes!
		CBaseCombatCharacter *pOwner = GetOwner();

		if( pOwner )
		{
			Vector vecSrc = pOwner->Weapon_ShootPosition();
			Vector	vecDir;

			AngleVectors( pOwner->EyeAngles(), &vecDir );

			trace_t tr;

			UTIL_TraceLine( vecSrc, vecSrc + vecDir * 1024, MASK_SOLID_BRUSHONLY, pOwner, COLLISION_GROUP_NONE, &tr );

			CSoundEnt::InsertSound( SOUND_DANGER_SNIPERONLY, tr.endpos, 384, 0.2, pOwner );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFrag::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		//Redraw the weapon
		SendWeaponAnim(ACT_VM_DRAW);

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::SecondaryAttack( void )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		if (m_bRedraw)
			return;

		if (!HasPrimaryAmmo())
			return;

		CBaseCombatCharacter *pOwner = GetOwner();

		if (pOwner == NULL)
			return;

		CBasePlayer *pPlayer = ToBasePlayer(pOwner);

		if (pPlayer == NULL)
			return;

		if (IsNearWall() || GetOwnerIsRunning())
		{
			return;
		}

		m_bIsFiring = true;

		// Note that this is a secondary attack and prepare the grenade attack to pause.
		m_AttackPaused = GRENADE_PAUSED_SECONDARY;

		//SendWeaponAnim(ACT_VM_PULLBACK_LOW);

		SendWeaponAnim(GetWpnData().animData[0].ThrowPullDown);

		//m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();

		// Don't let weapon idle interfere in the middle of a throw!
		m_flTimeWeaponIdle = FLT_MAX;
		m_flNextSecondaryAttack = FLT_MAX;

		// If I'm now out of ammo, switch away
		if (!HasPrimaryAmmo())
		{
			pPlayer->SwitchToNextBestWeapon(this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::PrimaryAttack( void )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		if (m_bRedraw)
			return;

		CBaseCombatCharacter *pOwner = GetOwner();

		if (pOwner == NULL)
		{
			return;
		}

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (!pPlayer)
			return;

		if (/*(pPlayer->m_nButtons & IN_RUN) || */(pPlayer->m_nButtons & IN_SPEED) && ((pPlayer->m_nButtons & IN_FORWARD) || (pPlayer->m_nButtons & IN_BACK) || (pPlayer->m_nButtons & IN_MOVELEFT) || (pPlayer->m_nButtons & IN_MOVERIGHT)))
		{
			return;
		}

		m_bIsFiring = true;

		// Note that this is a primary attack and prepare the grenade attack to pause.
		m_AttackPaused = GRENADE_PAUSED_PRIMARY;
		//SendWeaponAnim(ACT_VM_PULLBACK_HIGH);

		SendWeaponAnim(GetWpnData().animData[0].ThrowPullUp);

		//m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();

		// Put both of these off indefinitely. We do not know how long
		// the player will hold the grenade.
		m_flTimeWeaponIdle = FLT_MAX;
		m_flNextPrimaryAttack = FLT_MAX;

		// If I'm now out of ammo, switch away
		if (!HasPrimaryAmmo())
		{
			pPlayer->SwitchToNextBestWeapon(this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponFrag::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::ItemPostFrame( void )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		//if (m_bIsFiring && m_flTimeWeaponIdle <= gpGlobals->curtime)
		if (m_fDrawbackFinished)
		{
			CBasePlayer *pOwner = ToBasePlayer(GetOwner());

			if (pOwner)
			{
				switch (m_AttackPaused)
				{
				case GRENADE_PAUSED_PRIMARY:
					if (!(pOwner->m_nButtons & IN_ATTACK))
					{
						//SendWeaponAnim(ACT_VM_THROW);
						SendWeaponAnim(GetWpnData().animData[0].ThrowPrimary);
						m_fDrawbackFinished = false;
					}
					break;

				case GRENADE_PAUSED_SECONDARY:
					if (!(pOwner->m_nButtons & IN_ATTACK2))
					{
						//See if we're ducking
						if (pOwner->m_nButtons & IN_DUCK)
						{
							//Send the weapon animation
							//SendWeaponAnim(ACT_VM_SECONDARYATTACK);
							SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);
						}
						else
						{
							//Send the weapon animation
							//SendWeaponAnim(ACT_VM_HAULBACK);
							SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);
						}

						m_fDrawbackFinished = false;
					}
					break;

				default:
					break;
				}
			}
		}
	}

	BaseClass::ItemPostFrame();

	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
	{
		if (m_bRedraw)
		{
			if (IsViewModelSequenceFinished())
			{
				Reload();
			}
		}

		if (m_bIsFiring && (m_flTimeWeaponIdle < gpGlobals->curtime))
			m_bIsFiring = false;
	}
}

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponFrag::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 1200;
	Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, GRENADE_TIMER, false );

	WeaponSound( SINGLE );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::LobGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	
	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 350 + Vector( 0, 0, 50 );
	Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(200,random->RandomInt(-600,600),0), pPlayer, GRENADE_TIMER, false );

	WeaponSound( WPN_DOUBLE );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::RollGrenade( CBasePlayer *pPlayer )
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
	Fraggrenade_Create( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER, false );

	WeaponSound( SPECIAL1 );

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

