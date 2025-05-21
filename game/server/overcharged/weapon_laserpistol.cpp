//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Laser pistol - pew,pew :D
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
#include "beam_shared.h"	/* We will use some of the functions declared this later... */
#include "ammodef.h"		/* This is needed for the tracing done later */
 
// This determines the sprite used by the laser beam
#define PHYSCANNON_BEAM_SPRITE "sprites/orangelight1.vmt"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	Laserpistol_FASTEST_REFIRE_TIME		0.1f
#define	Laserpistol_FASTEST_DRY_REFIRE_TIME	0.2f

#define	Laserpistol_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	Laserpistol_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//ConVar	laserpistol_use_new_accuracy( "laserpistol_use_new_accuracy", "1" );
ConVar oc_wpn_laserpistol_shoot_effect( "oc_wpn_laserpistol_shoot_effect", "0", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Shoot effects for laser pistol. var 1-2." );

//-----------------------------------------------------------------------------
// CWeaponLaserpistol
//-----------------------------------------------------------------------------

class CWeaponLaserpistol : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponLaserpistol, CBaseHLCombatWeapon );

	CWeaponLaserpistol(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( trace_t &tr, int nDamageType, CBaseCombatCharacter *pOperator );
	void	SecondaryAttack( void );	// L1ght 15: Change effects by mouse clicks too

	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void   	DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	void	DoImpactEffect( trace_t &tr, int nDamageType );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	//Activity	GetPrimaryAttackActivity( void );

	virtual bool Reload( void );
	
	virtual int	GetMinBurst() 
	{ 
		return 1; 
	}

	virtual int	GetMaxBurst() 
	{ 
		return 3; 
	}

	/*virtual float GetFireRate( void ) 
	{
		return 0.5f; 
	}*/

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
	int	m_nBulletType;
};


IMPLEMENT_SERVERCLASS_ST(CWeaponLaserpistol, DT_WeaponLaserpistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_laserpistol, CWeaponLaserpistol );
PRECACHE_WEAPON_REGISTER( weapon_laserpistol );

BEGIN_DATADESC( CWeaponLaserpistol )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),
	DEFINE_FIELD( m_nBulletType,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeaponLaserpistol::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PISTOL,                    false },		// L1ght 15 : MP animstate
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PISTOL,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
    { ACT_HL2MP_GESTURE_RELOAD,          ACT_GESTURE_RELOAD_SMG1,					false },	//fix
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


IMPLEMENT_ACTTABLE( CWeaponLaserpistol );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponLaserpistol::CWeaponLaserpistol( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_nBulletType = -1;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::Precache( void )
{
	//PrecacheParticleSystem( "weapon_muzzle_smoke_long" );
	//if (cvar->FindVar("laserpistol_shoot_effect")->GetInt() == 2)
	//{
	PrecacheParticleSystem( "electrical_arc_01_parent" );
	//}
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + Laserpistol_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::PrimaryAttack( trace_t &tr, int nDamageType, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if ((cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1) || (cvar->FindVar("oc_state_is_running")->GetInt() == 1))
	{
		return;
	}

	if ( ( gpGlobals->curtime - m_flLastAttackTime ) > 0.5f )
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + Laserpistol_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner() );


	/*if(m_nNumShotsFired >= 5)
	{
		DispatchParticleEffect( "weapon_muzzle_smoke_long", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
	}*/

	BaseClass::PrimaryAttack();

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += Laserpistol_ACCURACY_SHOT_PENALTY_TIME;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pOwner, true, GetClassname() );

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	DrawBeam( vecShootOrigin, tr.endpos, 15.5 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, Laserpistol_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::ItemPostFrame( void )
{
	if (!m_bInReload && ((cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1) || (cvar->FindVar("oc_state_is_running")->GetInt() == 1)))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
	}
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
/*Activity CWeaponLaserpistol::GetPrimaryAttackActivity( void )
{
	if ( m_nNumShotsFired < 1 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nNumShotsFired < 2 )
		return ACT_VM_RECOIL1;

	if ( m_nNumShotsFired < 3 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponLaserpistol::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - where the beam should begin
//          &endPos - where the beam should end
//          width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{
	//Tracer down the middle
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );

	if (cvar->FindVar("oc_wpn_laserpistol_shoot_effect")->GetInt() == 2)
	{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	DispatchParticleEffect( "electrical_arc_01_parent", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
	}

	if (cvar->FindVar("oc_wpn_laserpistol_shoot_effect")->GetInt() == 1)
	{
	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( PHYSCANNON_BEAM_SPRITE, 15.5 );
 
	// It starts at startPos
	pBeam->SetStartPos( startPos );
 
	// This sets up some things that the beam uses to figure out where
	// it should start and end
	pBeam->PointEntInit( endPos, this );
 
	// This makes it so that the laser appears to come from the muzzle of the pistol
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );
	pBeam->SetWidth( width );
//	pBeam->SetEndWidth( 0.05f );
 
	// Higher brightness means less transparent
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
 
	// The beam should only exist for a very short time
	pBeam->LiveForTime( 0.1f );
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - used to figure out where to do the effect
//          nDamageType - ???
//-----------------------------------------------------------------------------
void CWeaponLaserpistol::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw our beam
	DrawBeam( tr.startpos, tr.endpos, 15.5 );
	if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		m_nBulletType = GetAmmoDef()->Index("GaussEnergy");
		UTIL_ImpactTrace( &tr, m_nBulletType );
	}
}

void CWeaponLaserpistol::SecondaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	if ((cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1) || (cvar->FindVar("oc_state_is_running")->GetInt() == 1))
	{
		return;
	}

	if (cvar->FindVar("oc_wpn_laserpistol_shoot_effect")->GetInt() == 0)
	{
		cvar->FindVar("oc_wpn_laserpistol_shoot_effect")->SetValue(1);
	}
	else
	{
		cvar->FindVar("oc_wpn_laserpistol_shoot_effect")->SetValue(0);
	}

	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flNextSecondaryAttack = gpGlobals->curtime + 2.0f;
}