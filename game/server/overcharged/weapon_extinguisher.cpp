//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"

#include "entitylist.h"
#include "fire.h"
#include "ar2_explosion.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
//#include "soundenvelope.h"

#include "soundent.h"
#include "explode.h"

#include "decals.h"
#include "particle_parse.h"

#include "weapon_extinguisher.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	fire_extinguisher_debug( "fire_extinguisher_debug", "0" );
ConVar	fire_extinguisher_radius( "fire_extinguisher_radius", "45" );
ConVar	fire_extinguisher_distance( "fire_extinguisher_distance", "200" );
ConVar	fire_extinguisher_strength( "fire_extinguisher_strength", "0.97" );	//TODO: Stub for real numbers
ConVar	fire_extinguisher_explode_radius( "fire_extinguisher_explode_radius", "256" );
ConVar	fire_extinguisher_explode_strength( "fire_extinguisher_explode_strength", "0.3" );	//TODO: Stub for real numbers

#define	EXTINGUISHER_AMMO_RATE	0.2

ConVar oc_weapon_extinguisher_effect( "oc_weapon_extinguisher_effect", "steam_jet_50", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Particle effect to extinguisher." );	//"weapon_muzzle_smoke" "human_cleanser"
ConVar oc_weapon_extinguisher_exp_dmg( "oc_weapon_extinguisher_exp_dmg", "100", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Fire extinguisher explosion damage." );
ConVar oc_weapon_extinguisher_pickup( "oc_weapon_extinguisher_pickup", "1", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Allow pickup or not." );

extern short	g_sModelIndexFireball;	// (in combatweapon.cpp) holds the index for the smoke cloud



IMPLEMENT_SERVERCLASS_ST(CWeaponExtinguisher, DT_WeaponExtinguisher)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_extinguisher, CWeaponExtinguisher );
PRECACHE_WEAPON_REGISTER( weapon_extinguisher );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponExtinguisher )

	DEFINE_FIELD( m_pJet,	FIELD_CLASSPTR ),

END_DATADESC()

acttable_t	CWeaponExtinguisher::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PISTOL,                    false },		// BJ: MP animstate
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PISTOL,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PISTOL,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PISTOL,                    false },
    //{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },		// END

	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			true },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	true },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE/*_PISTOL*/,				false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_STIMULATED,			false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_STEALTH,				ACT_IDLE_STEALTH_PISTOL,		false },

	{ ACT_WALK_RELAXED,				ACT_WALK,						false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_STIMULATED,			false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_STEALTH,				ACT_WALK_STEALTH_PISTOL,		false },

	{ ACT_RUN_RELAXED,				ACT_RUN,						false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_STIMULATED,				false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_STEALTH,				ACT_RUN_STEALTH_PISTOL,			false },

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_PISTOL,				false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_ANGRY_PISTOL,			false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_AIM_STEALTH,			ACT_IDLE_STEALTH_PISTOL,		false },

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK,						false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_PISTOL,			false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_AIM_STEALTH,			ACT_WALK_AIM_STEALTH_PISTOL,	false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN,						false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_PISTOL,				false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_AIM_STEALTH,			ACT_RUN_AIM_STEALTH_PISTOL,		false },//always aims
	//End readiness activities

	// Crouch activities
	{ ACT_CROUCHIDLE_STIMULATED,	ACT_CROUCHIDLE_STIMULATED,		false },
	{ ACT_CROUCHIDLE_AIM_STIMULATED,ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims
	{ ACT_CROUCHIDLE_AGITATED,		ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims

	// Readiness translations
	{ ACT_READINESS_RELAXED_TO_STIMULATED, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED, false },
	{ ACT_READINESS_RELAXED_TO_STIMULATED_WALK, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK, false },
	{ ACT_READINESS_AGITATED_TO_STIMULATED, ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED, false },
	{ ACT_READINESS_STIMULATED_TO_RELAXED, ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED, false },
};
IMPLEMENT_ACTTABLE( CWeaponExtinguisher );

CWeaponExtinguisher::CWeaponExtinguisher( void )
{
	m_pJet		= NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Precache( void )
{
	BaseClass::Precache();

	PrecacheParticleSystem( oc_weapon_extinguisher_effect.GetString() ); //"electrical_arc_01_parent" );
	PrecacheScriptSound( "ExtinguisherCharger.Use" );
	PrecacheScriptSound( "Weapon_Extinguisher.Single" );	//fire
	PrecacheScriptSound( "Weapon_Extinguisher.Special1" );	// end

	PrecacheScriptSound( "Weapon_Extinguisher.Explode" );
	//UTIL_PrecacheOther( "env_extinguisherjet" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Spawn( void )
{
	BaseClass::Spawn();

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 5;//FIXME: Define
	m_bFireSoundOn = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

	m_takedamage	= DAMAGE_NO;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pInflictor - 
//			*pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Event_Killed( const CTakeDamageInfo &info )
{
	//if ( info.GetDamageType() & DMG_SONIC )
		//return;
	m_takedamage	= DAMAGE_NO;	

	//TODO: Use a real effect
	if ( AR2Explosion *pExplosion = AR2Explosion::CreateAR2Explosion( GetAbsOrigin() ) )
	{
		pExplosion->SetLifetime( 10 );
	}

	// BJ: Real explosion damage
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 1024, 3.0 );
	RadiusDamage( CTakeDamageInfo( this, this, oc_weapon_extinguisher_exp_dmg.GetFloat(), DMG_SONIC ), GetAbsOrigin(), 150, CLASS_NONE, this );

	EmitSound( "Weapon_Extinguisher.Explode");	//=

	//TODO: Use a real effect
	//CPASFilter filter( GetAbsOrigin() );

	/*te->Explosion( filter, 0.0,
		&GetAbsOrigin(), 
		g_sModelIndexFireball,
		2.0, 
		15,
		TE_EXPLFLAG_NONE,
		250,
		100 );*/

	//Put out fire in a radius
	FireSystem_ExtinguishInRadius( GetAbsOrigin(), fire_extinguisher_explode_radius.GetInt(), fire_extinguisher_explode_strength.GetFloat() );

	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Turn the jet effect and noise on
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::StartJet( void )
{
	//WeaponSound( SINGLE, 30 );

	//return;
	/*
	//See if the jet needs to be created
	if ( m_pJet == NULL )
	{
		m_pJet = (CExtinguisherJet *) CreateEntityByName( "env_extinguisherjet" );

		if ( m_pJet == NULL )
		{
			Msg( "Unable to create jet for weapon_extinguisher!\n" );
			return;
		}

		//Setup the jet
		m_pJet->m_bEmit	= false;
		UTIL_SetOrigin( m_pJet, GetAbsOrigin() );
		m_pJet->SetParent( this );

		m_pJet->m_bUseMuzzlePoint = true;
		m_pJet->m_bAutoExtinguish = false;
		m_pJet->m_nLength = fire_extinguisher_distance.GetInt();

	}

	//Turn the jet on
	if ( m_pJet != NULL )
	{
		//m_pJet->TurnOn();		//test
	}*/

	if ( m_bFireSoundOn == true )
	{
		//EmitSound( "ExtinguisherCharger.Use" );
		WeaponSound( SINGLE );
		m_bFireSoundOn = false;
		//DevMsg("Fire sound function override \n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turn the jet effect and noise off
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::StopJet( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//DispatchParticleEffect( "weapon_muzzle_smoke", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);	//test
	StopParticleEffects(pOwner->GetViewModel());

	//WeaponSound( SPECIAL1 );

	/*
	//Turn the jet off
	if ( m_pJet != NULL )
	{
		//m_pJet->TurnOff();	//test
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::ItemPostFrame( void )
{	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Only shoot if we have ammo
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0) // BJ: Swith to regular Primary
	{
		if (pOwner->IsPlayer())
		{
			StopJet();
			pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 1);
			//pOwner->Weapon_SetLast(pOwner->GetActiveWeapon());
			pOwner->SwitchToNextBestWeapon(this);
		}

		return;
	}
	
	if (!m_bInReload && (IsNearWall() || GetOwnerIsRunning()))
	{
		//m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		//m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
		//if (pOwner->m_afButtonReleased & IN_ATTACK)
			//return;
		//else if (pOwner->m_nButtons & IN_ATTACK)
		//{
		if (m_bFireSoundOn == true)
		{
		StopJet();
		WeaponSound(SPECIAL1);
		//DevMsg("Run wall override? \n");
		m_bFireSoundOn = false;
		}
		//return;
	}
	//See if we should try and extinguish fires
	else if ( pOwner->m_nButtons & IN_ATTACK )
	//if ( pOwner->m_afButtonPressed & IN_ATTACK )
	{
		//Turn the jet on
		StartJet();

		Vector	vTestPos, vMuzzlePos;
		Vector	vForward, vRight, vUp;

		pOwner->EyeVectors( &vForward, &vRight, &vUp );
		
		vMuzzlePos	= pOwner->Weapon_ShootPosition( );
		
		//FIXME: Need to get the exact same muzzle point!

		//FIXME: This needs to be adjusted so the server collision matches the visuals on the client
		vMuzzlePos	+= vForward * 15.0f;
		vMuzzlePos	+= vRight * 6.0f;
		vMuzzlePos	+= vUp * -4.0f;

		QAngle aTmp;
		VectorAngles( vForward, aTmp );
		aTmp[PITCH] += 10;
		AngleVectors( aTmp, &vForward );

		vTestPos	= vMuzzlePos + ( vForward * fire_extinguisher_distance.GetInt() );

		trace_t	tr;
		UTIL_TraceLine(vMuzzlePos, vTestPos, MASK_ALL, this, COLLISION_GROUP_INTERACTIVE_DEBRIS, &tr);

		//Extinguish the fire where we hit
		FireSystem_ExtinguishInRadius( tr.endpos, fire_extinguisher_radius.GetInt(), fire_extinguisher_strength.GetFloat() );

		if ( tr.m_pEnt )	// BJ: Extinguish Props and Npc too !
		{
		if ( tr.m_pEnt->GetFlags() & FL_ONFIRE )	
		{
			UTIL_Remove( tr.m_pEnt->GetEffectEntity() );
			tr.m_pEnt->RemoveFlag( FL_ONFIRE );
		}
		}

		m_bFireSoundOn = true;
		//DevMsg("Fire sound state override \n");

		SendWeaponAnim(GetPrimaryAttackActivity());	// BJ: Player Animation
		pOwner->SetAnimation( PLAYER_ATTACK1 );		

		//Drain ammo
		if ( m_flNextPrimaryAttack < gpGlobals->curtime  )
		{
			RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo(1, m_iPrimaryAmmoType); // BJ: Swith to regular Primary
			m_flNextPrimaryAttack = gpGlobals->curtime + EXTINGUISHER_AMMO_RATE;
			DispatchParticleEffect( oc_weapon_extinguisher_effect.GetString(), PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);	//test
			UTIL_DecalTrace( &tr, "extinguish" );	// BJ: Add white decal		
		}

		//If we're just run out...
		if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 ) // BJ: Swith to regular Primary
		{
			WeaponSound( SPECIAL1 );
			StopJet();
			return;
		}

		//Debug visualization
		if ( fire_extinguisher_debug.GetInt() )
		{
			int	radius = fire_extinguisher_radius.GetInt();

			NDebugOverlay::Line( vMuzzlePos, tr.endpos, 0, 0, 128, false, 0.0f );
			
			NDebugOverlay::Box( vMuzzlePos, Vector(-1, -1, -1), Vector(1, 1, 1), 0, 0, 128, false, 0.0f );
			NDebugOverlay::Box( tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 128, false, 0.0f );
			NDebugOverlay::Box( tr.endpos, Vector(-radius, -radius, -radius), Vector(radius, radius, radius), 0, 0, 255, false, 0.0f );
		}
	}
	else if ( pOwner->m_afButtonReleased & IN_ATTACK )
	{
		WeaponSound( SPECIAL1 );
		StopJet();
		m_bFireSoundOn = false;
		//DevMsg("Ext unpress override? \n");
	}
	/*else
	{
		StopJet();
		DevMsg("Ext override? \n");
	}*/
}

void CWeaponExtinguisher::Drop( const Vector &vecVelocity )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	m_takedamage	= DAMAGE_YES;

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
	WeaponSound( SPECIAL1 );
	StopJet();
	}

	BaseClass::Drop( vecVelocity );
}

bool CWeaponExtinguisher::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	//WeaponSound( SPECIAL1 );
	//WeaponSound( EMPTY );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return false;

	if ( pOwner->m_nButtons & IN_ATTACK )
		return false;

	StopJet();

	//return true;
	return BaseClass::Holster( pSwitchingTo );
}

class CExtinguisherCharger : public CBaseToggle
{
public:
	DECLARE_CLASS( CExtinguisherCharger, CBaseToggle );

	void Spawn( void );
	bool CreateVPhysics();
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

protected:
	float	m_flNextCharge; 
	bool	m_bSoundOn;

	void	TurnOff( void );

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( func_extinguishercharger, CExtinguisherCharger );

BEGIN_DATADESC( CExtinguisherCharger )

	DEFINE_FIELD( m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( m_bSoundOn, FIELD_BOOLEAN ),

	DEFINE_FUNCTION( TurnOff ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Spawn the object
//-----------------------------------------------------------------------------
void CExtinguisherCharger::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_PUSH );

	SetModel( STRING( GetModelName() ) );

	m_bSoundOn = false;

	CreateVPhysics();
}

//-----------------------------------------------------------------------------

bool CExtinguisherCharger::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CExtinguisherCharger::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Make sure that we have a caller
	if ( pActivator == NULL )
		return;
	
	// If it's not a player, ignore
	if ( pActivator->IsPlayer() == false )
		return;

	// Turn our sound on, if it's not already
	if ( m_bSoundOn == false )
	{
		EmitSound( "ExtinguisherCharger.Use" );
		m_bSoundOn = true;
	}

	SetNextThink( gpGlobals->curtime + 0.25 );
	
	SetThink( &CExtinguisherCharger::TurnOff );

	CBasePlayer	*pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer )
	{
		//FIXME: Need a way to do this silently
		pPlayer->GiveAmmo( 1, "extinguisher" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherCharger::TurnOff( void )
{
	//Turn the sound off
	if ( m_bSoundOn )
	{
		StopSound( "ExtinguisherCharger.Use" );
		m_bSoundOn = false;
	}
}
