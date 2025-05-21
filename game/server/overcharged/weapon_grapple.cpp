//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Grapple h00k
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_grapple.h"
 
#ifdef CLIENT_DLL
	//#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "game.h"
	//#include "hl2mp_player.h"
	#include "player.h"
	#include "te_effect_dispatch.h"
	#include "IEffects.h"
	#include "Sprite.h"
	#include "SpriteTrail.h"
	#include "beam_shared.h"
	#include "explode.h"
 
	#include "vphysics/constraints.h"
	#include "physics_saverestore.h"
 
#endif

//#include "effect_dispatch_data.h"
 
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
 
#define HOOK_MODEL			"models/props_junk/rock001a.mdl"
#define BOLT_MODEL			"models/crossbow_bolt.mdl"
 
#define BOLT_AIR_VELOCITY	3500
#define BOLT_WATER_VELOCITY	1500
#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1
 
 
#ifndef CLIENT_DLL
 
LINK_ENTITY_TO_CLASS( grapple_hook, CGrappleHook );
 
BEGIN_DATADESC( CGrappleHook )
	// Function Pointers
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( HookedThink ),
	DEFINE_FUNCTION( HookTouch ),
 
	DEFINE_PHYSPTR( m_pSpring ),
	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hBolt, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bPlayerWasStanding, FIELD_BOOLEAN ),
 
END_DATADESC()
 
CGrappleHook *CGrappleHook::HookCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner )
{
	// Create a new entity with CGrappleHook private data
	CGrappleHook *pHook = (CGrappleHook *)CreateEntityByName( "grapple_hook" );
	UTIL_SetOrigin( pHook, vecOrigin );
	pHook->SetAbsAngles( angAngles );
	pHook->Spawn();
 
	CWeaponGrapple *pOwner = (CWeaponGrapple *)pentOwner;
	pHook->m_hOwner = pOwner;
	pHook->SetOwnerEntity( pOwner->GetOwner() );
	pHook->m_hPlayer = (CBasePlayer *)pOwner->GetOwner();
 
	return pHook;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrappleHook::~CGrappleHook( void )
{
	if ( m_pSpring )
	{
		physenv->DestroySpring( m_pSpring );
		m_pSpring = NULL;
	}
 
	if ( m_hBolt )
	{
		UTIL_Remove( m_hBolt );
		m_hBolt = NULL;
	}
 
	// Revert Jay's gai flag
	if ( m_hPlayer )
		m_hPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, false );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGrappleHook::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );
 
	return true;
}
 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CGrappleHook::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrappleHook::Spawn( void )
{
	Precache( );
 
	SetModel( HOOK_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(1,1,1), Vector(1,1,1) );
	SetSolid( SOLID_BBOX );
	SetGravity( 0.05f );
 
	// The rock is invisible, the crossbow bolt is the visual representation
	AddEffects( EF_NODRAW );
 
	// Make sure we're updated if we're underwater
	UpdateWaterState();
 
	SetTouch( &CGrappleHook::HookTouch );
 
	SetThink( &CGrappleHook::FlyThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
 
	m_pSpring		= NULL;
	m_fSpringLength = 0.0f;
	m_bPlayerWasStanding = false;
 
	// Create bolt model and parent it
	CBaseEntity *pBolt = CBaseEntity::CreateNoSpawn( "prop_dynamic", GetAbsOrigin(), GetAbsAngles(), this );
	pBolt->SetModelName( MAKE_STRING( BOLT_MODEL ) );
	pBolt->SetModel( BOLT_MODEL );
	DispatchSpawn( pBolt );
	pBolt->SetParent( this );
}
 
 
void CGrappleHook::Precache( void )
{
	PrecacheModel( HOOK_MODEL );
	PrecacheModel( BOLT_MODEL );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CGrappleHook::HookTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;
 
	if ( (pOther != m_hOwner) && (pOther->m_takedamage != DAMAGE_NO) )
	{
		m_hOwner->NotifyHookDied();
 
		SetTouch( NULL );
		SetThink( NULL );
 
		UTIL_Remove( this );
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();
 
		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			EmitSound( "Weapon_Crossbow.BoltHitWorld" );
 
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
 
			//FIXME: We actually want to stick (with hierarchy) to what we've hit
			SetMoveType( MOVETYPE_NONE );
 
			Vector vForward;
 
			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize ( vForward );
 
			CEffectData	data;
 
			data.m_vOrigin = tr.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = 0;
 
		//	DispatchEffect( "Impact", data );
 
			UTIL_ImpactTrace( &tr, DMG_BULLET );
 
		//	AddEffects( EF_NODRAW );
			SetTouch( NULL );
 
			// Shoot some sparks
			if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
			{
				g_pEffects->Sparks( GetAbsOrigin() );
			}
 
			VPhysicsDestroyObject();
			VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_NOT_STANDABLE, false );
			AddSolidFlags( FSOLID_NOT_SOLID );
		//	SetMoveType( MOVETYPE_NONE );
 
			if ( !m_hPlayer )
			{
				Assert( 0 );
				return;
			}
 
			// Set Jay's gai flag
			m_hPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, true );
 
			IPhysicsObject *pPhysObject = m_hPlayer->VPhysicsGetObject();
			IPhysicsObject *pRootPhysObject = VPhysicsGetObject();
			Assert( pRootPhysObject );
			Assert( pPhysObject );
 
			pRootPhysObject->EnableMotion( false );
 
			// Root has huge mass, tip has little
			pRootPhysObject->SetMass( VPHYSICS_MAX_MASS );
		//	pPhysObject->SetMass( 100 );
		//	float damping = 3;
		//	pPhysObject->SetDamping( &damping, &damping );
 
			Vector origin = m_hPlayer->GetAbsOrigin();
			Vector rootOrigin = GetAbsOrigin();
			m_fSpringLength = (origin - rootOrigin).Length();
 
			springparams_t spring;
			spring.constant = 8000;
			spring.damping = 400;
			spring.naturalLength = m_fSpringLength;
			spring.relativeDamping = 2;
			spring.startPosition = origin;
			spring.endPosition = rootOrigin;
			spring.useLocalPositions = false;
			spring.onlyStretch = false;		
			m_pSpring = physenv->CreateSpring( pPhysObject, pRootPhysObject, &spring );
 
			m_bPlayerWasStanding = ( ( m_hPlayer->GetFlags() & FL_DUCKING ) == 0 );
 
			SetThink( &CGrappleHook::HookedThink );
			SetNextThink( gpGlobals->curtime + 0.1f );
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ( ( tr.surface.flags & SURF_SKY ) == false )
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
			}
 
			SetTouch( NULL );
			SetThink( NULL );
 
			m_hOwner->NotifyHookDied();
			UTIL_Remove( this );
		}
	}
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrappleHook::HookedThink( void )
{
	UpdatePlayerConstraint();
 
	//set next globalthink
	SetNextThink( gpGlobals->curtime + 0.1f );
 
	m_fSpringLength -= 20;
 
	if ( m_fSpringLength < 1 )
		return;
 
	if ( m_pSpring )
		m_pSpring->SetSpringLength( m_fSpringLength );
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CGrappleHook::UpdatePlayerConstraint( void )
{
	// Check to see if the player's standing/ducking state has changed.
	bool bStanding = ( ( m_hPlayer->GetFlags() & FL_DUCKING ) == 0 );
	if ( bStanding == m_bPlayerWasStanding )
		return;
 
	// Destroy the current spring.
	physenv->DestroySpring( m_pSpring );
	m_pSpring = NULL;
 
	// Set Jay's gai flag
//	m_hPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, true );
 
	// Create the new constraint for the standing/ducking player physics object.
	IPhysicsObject *pPhysObject = m_hPlayer->VPhysicsGetObject();
	IPhysicsObject *pRootPhysObject = VPhysicsGetObject();
	Assert( pRootPhysObject );
	Assert( pPhysObject );
 
	Vector origin = m_hPlayer->GetAbsOrigin();
	Vector rootOrigin = GetAbsOrigin();
	m_fSpringLength = (origin - rootOrigin).Length();
 
	springparams_t spring;
	spring.constant = 6000; //400
	spring.damping = 100;
	spring.naturalLength = m_fSpringLength;
	spring.relativeDamping = 2;
	spring.startPosition = origin;
	spring.endPosition = rootOrigin;
	spring.useLocalPositions = false;
	spring.onlyStretch = false;		
	m_pSpring = physenv->CreateSpring( pPhysObject, pRootPhysObject, &spring );
 
	// Save state for the next check.
	m_bPlayerWasStanding = bStanding;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrappleHook::FlyThink( void )
{
	QAngle angNewAngles;
 
	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );
 
	SetNextThink( gpGlobals->curtime + 0.1f );
}
 
#endif
 
//IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrapple, DT_WeaponGrapple )
 
#ifdef CLIENT_DLL
void RecvProxy_HookDied( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CWeaponGrapple *pGrapple = ((CWeaponGrapple*)pStruct);
 
	RecvProxy_IntToEHandle( pData, pStruct, pOut );
 
	CBaseEntity *pNewHook = pGrapple->GetHook();
 
	if ( pNewHook == NULL )
	{
		if ( pGrapple->GetOwner() && pGrapple->GetOwner()->GetActiveWeapon() == pGrapple )
		{
			pGrapple->NotifyHookDied();
		}
	}
}
#endif
/*
BEGIN_NETWORK_TABLE( CWeaponGrapple, DT_WeaponGrapple )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bInZoom ) ),
	RecvPropBool( RECVINFO( m_bMustReload ) ),
	RecvPropEHandle( RECVINFO( m_hHook ), RecvProxy_HookDied ),
#else
	SendPropBool( SENDINFO( m_bInZoom ) ),
	SendPropBool( SENDINFO( m_bMustReload ) ),
	SendPropEHandle( SENDINFO( m_hHook ) ),
#endif
END_NETWORK_TABLE()
 
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponGrapple )
	DEFINE_PRED_FIELD( m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bMustReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif
 
LINK_ENTITY_TO_CLASS( weapon_grapple, CWeaponGrapple );
 
PRECACHE_WEAPON_REGISTER( weapon_grapple );*/

LINK_ENTITY_TO_CLASS( weapon_grapple, CWeaponGrapple );

PRECACHE_WEAPON_REGISTER( weapon_grapple );

IMPLEMENT_SERVERCLASS_ST( CWeaponGrapple, DT_WeaponGrapple )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponGrapple )

	DEFINE_FIELD( m_bInZoom,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMustReload,	FIELD_BOOLEAN ),

	//DEFINE_FIELD( m_hHook,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hHook,	FIELD_VOID ),

END_DATADESC()
 
#ifndef CLIENT_DLL
 
acttable_t	CWeaponGrapple::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_CROSSBOW,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_CROSSBOW,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_CROSSBOW,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_CROSSBOW,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_CROSSBOW,                    false },
    //{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};
 
IMPLEMENT_ACTTABLE(CWeaponGrapple);
 
#endif
 
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponGrapple::CWeaponGrapple( void )
{
	m_bReloadsSingly	= true;
	m_bInZoom			= false;
	m_bMustReload		= false;
#ifndef CLIENT_DLL
	m_hRope				= NULL;
#endif
}
 
#define	CROSSBOW_GLOW_SPRITE	"sprites/light_glow02_noz.vmt"
#define	CROSSBOW_GLOW_SPRITE2	"sprites/blueflare1.vmt"
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grapple_hook" );
#endif
 
//	PrecacheScriptSound( "Weapon_Crossbow.BoltHitBody" );
	PrecacheScriptSound( "Weapon_Crossbow.BoltHitWorld" );
//	PrecacheScriptSound( "Weapon_Crossbow.BoltSkewer" );
 
	PrecacheModel( CROSSBOW_GLOW_SPRITE );
	PrecacheModel( CROSSBOW_GLOW_SPRITE2 );
 
	BaseClass::Precache();
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponGrapple::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// Can't have an active hook out
	if ( m_hHook != NULL )
		return;
 
	FireHook();
	pPlayer->SetAnimation( PLAYER_ATTACK1 );				// Light Kill : Forgotten anim ?
 
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration( ACT_VM_PRIMARYATTACK ) );
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponGrapple::SecondaryAttack( void )
{
	//NOTENOTE: The zooming is handled by the post/busy frames
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrapple::Reload( void )
{

	if ( ( m_bMustReload ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_RELOAD );
 
		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
 
		//Mark this as done
		m_bMustReload = false;
	}

	return true;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::CheckZoomToggle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();	// L1ght 15 : Check to avoid problems

	if (pWeapon == NULL || (cvar->FindVar("oc_state_IRsight_on")->GetInt())) //pWeapon->IsIronsighted() )
	{
		m_bInZoom = false;
		return;
	}
 
	if ( pPlayer->m_afButtonPressed & IN_ATTACK2 )
	{
		ToggleZoom();
	}

	if ( pPlayer->m_afButtonPressed & IN_SPEED )	// L1ght 15: Disable zoom on the run
	{
		if ( m_bInZoom )
		{
			ToggleZoom();
		}
	}
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::ItemBusyFrame( void )
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::ItemPostFrame( void )
{
	// Allow zoom toggling
	CheckZoomToggle();
 
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( ( pOwner->m_nButtons & IN_ATTACK ) )
	{
		if ( m_flNextPrimaryAttack < gpGlobals->curtime )
		{
			PrimaryAttack();
		}
	}
	else if ( m_bMustReload ) //&& HasWeaponIdleTimeElapsed() )
	{
		Reload();
		pOwner->SetAnimation( PLAYER_RELOAD );
	}
 
#ifndef CLIENT_DLL
	if ( m_hHook )
	{
		if ( m_hRope )
			m_hRope->RecalculateLength();
 
		if ( !(pOwner->m_nButtons & IN_ATTACK) )
		{
			m_hHook->SetTouch( NULL );
			m_hHook->SetThink( NULL );
 
			UTIL_Remove( m_hHook );
			m_hHook = NULL;
 
			NotifyHookDied();
 
			m_bMustReload = true;
		}
	}
#endif
 
	BaseClass::ItemPostFrame();
}
 
#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrapple::CreateRope( void )
{
	if ( !m_hHook )
	{
		Assert( 0 );
		return false;
	}
 
	m_hRope = CRopeKeyframe::Create( this, m_hHook, 1, 0 );
 
	if ( m_hRope )
	{
		m_hRope->m_Width = 2;
		m_hRope->m_nSegments = ROPE_MAX_SEGMENTS / 2;
		m_hRope->EnableWind( false );
	//	m_hRope->EnableCollision(); // Collision looks worse than no collision
		m_hRope->SetupHangDistance( 0 );
	}
 
	return true;
}
#endif
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::FireHook( void )
{
	if ( m_bMustReload )
		return;
 
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( pOwner == NULL )
		return;
 
#ifndef CLIENT_DLL
	Vector vecAiming	= pOwner->GetAutoaimVector( 0 );	
	Vector vecSrc		= pOwner->Weapon_ShootPosition();
 
	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );
 
	CGrappleHook *pHook = CGrappleHook::HookCreate( vecSrc, angAiming, this );
 
	if ( pOwner->GetWaterLevel() == 3 )
	{
		pHook->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	}
	else
	{
		pHook->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
	}
 
	m_hHook = pHook;
	CreateRope();
 
#endif
 
	pOwner->ViewPunch( QAngle( -2, 0, 0 ) );
 
	WeaponSound( SINGLE );
	WeaponSound( SPECIAL2 );
 
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
 
	m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + 0.75;
 
	DoLoadEffect();
	SetChargerState( CHARGER_STATE_DISCHARGE );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrapple::Deploy( void )
{
	if ( m_bMustReload )
	{
		return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix() );
	}
 
	SetSkin( BOLT_SKIN_GLOW );
 
	return BaseClass::Deploy();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrapple::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifndef CLIENT_DLL
	if ( m_hHook )
	{
		m_hHook->SetTouch( NULL );
		m_hHook->SetThink( NULL );
 
		UTIL_Remove( m_hHook );
		m_hHook = NULL;
 
		NotifyHookDied();
 
		m_bMustReload = true;
	}
#endif
 
	if ( m_bInZoom )
	{
		ToggleZoom();
	}
 
	SetChargerState( CHARGER_STATE_OFF );
 
	return BaseClass::Holster( pSwitchingTo );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::ToggleZoom( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
 
	if ( pPlayer == NULL )
		return;
 
#ifndef CLIENT_DLL
 
	if ( m_bInZoom )
	{
		if ( pPlayer->SetFOV( this, 0, 0.2f ) )
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if ( pPlayer->SetFOV( this, 20, 0.1f ) )
		{
			m_bInZoom = true;
		}
	}
#endif
}
 
#define	BOLT_TIP_ATTACHMENT	2
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::CreateChargerEffects( void )
{
#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( m_hChargerSprite != NULL )
		return;
 
	m_hChargerSprite = CSprite::SpriteCreate( CROSSBOW_GLOW_SPRITE, GetAbsOrigin(), false );
 
	if ( m_hChargerSprite )
	{
		m_hChargerSprite->SetAttachment( pOwner->GetViewModel(), BOLT_TIP_ATTACHMENT );
		m_hChargerSprite->SetTransparency( kRenderTransAdd, 255, 128, 0, 255, kRenderFxNoDissipation );
		m_hChargerSprite->SetBrightness( 0 );
		m_hChargerSprite->SetScale( 0.1f );
		m_hChargerSprite->TurnOff();
	}
#endif
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : skinNum - 
//-----------------------------------------------------------------------------
void CWeaponGrapple::SetSkin( int skinNum )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( pOwner == NULL )
		return;
 
	CBaseViewModel *pViewModel = pOwner->GetViewModel();
 
	if ( pViewModel == NULL )
		return;
 
	pViewModel->m_nSkin = skinNum;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::DoLoadEffect( void )
{
	SetSkin( BOLT_SKIN_GLOW );
 
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( pOwner == NULL )
		return;
 
	CBaseViewModel *pViewModel = pOwner->GetViewModel();
 
	if ( pViewModel == NULL )
		return;
 
	CEffectData	data;
 
#ifdef CLIENT_DLL
	data.m_hEntity = pViewModel->GetRefEHandle();
#else
	data.m_nEntIndex = pViewModel->entindex();
#endif
	data.m_nAttachmentIndex = 1;
 
	DispatchEffect( "CrossbowLoad", data );
 
#ifndef CLIENT_DLL
 
	CSprite *pBlast = CSprite::SpriteCreate( CROSSBOW_GLOW_SPRITE2, GetAbsOrigin(), false );
 
	if ( pBlast )
	{
		pBlast->SetAttachment( pOwner->GetViewModel(), 1 );
		pBlast->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
		pBlast->SetBrightness( 128 );
		pBlast->SetScale( 0.2f );
		pBlast->FadeOutFromSpawn();
	}
#endif
 
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void CWeaponGrapple::SetChargerState( ChargerState_t state )
{
	// Make sure we're setup
	CreateChargerEffects();
 
	// Don't do this twice
	if ( state == m_nChargeState )
		return;
 
	m_nChargeState = state;
 
	switch( m_nChargeState )
	{
	case CHARGER_STATE_START_LOAD:
 
		WeaponSound( SPECIAL1 );
 
		// Shoot some sparks and draw a beam between the two outer points
		DoLoadEffect();
 
		break;
#ifndef CLIENT_DLL
	case CHARGER_STATE_START_CHARGE:
		{
			if ( m_hChargerSprite == NULL )
				break;
 
			m_hChargerSprite->SetBrightness( 32, 0.5f );
			m_hChargerSprite->SetScale( 0.025f, 0.5f );
			m_hChargerSprite->TurnOn();
		}
 
		break;
 
	case CHARGER_STATE_READY:
		{
			// Get fully charged
			if ( m_hChargerSprite == NULL )
				break;
 
			m_hChargerSprite->SetBrightness( 80, 1.0f );
			m_hChargerSprite->SetScale( 0.1f, 0.5f );
			m_hChargerSprite->TurnOn();
		}
 
		break;
 
	case CHARGER_STATE_DISCHARGE:
		{
			SetSkin( BOLT_SKIN_NORMAL );
 
			if ( m_hChargerSprite == NULL )
				break;
 
			m_hChargerSprite->SetBrightness( 0 );
			m_hChargerSprite->TurnOff();
		}
 
		break;
#endif
	case CHARGER_STATE_OFF:
		{
			SetSkin( BOLT_SKIN_NORMAL );
 
#ifndef CLIENT_DLL
			if ( m_hChargerSprite == NULL )
				break;
 
			m_hChargerSprite->SetBrightness( 0 );
			m_hChargerSprite->TurnOff();
#endif
		}
		break;
 
	default:
		break;
	}
}
 
#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponGrapple::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_THROW:
		SetChargerState( CHARGER_STATE_START_LOAD );
		break;
 
	case EVENT_WEAPON_THROW2:
		SetChargerState( CHARGER_STATE_START_CHARGE );
		break;
 
	case EVENT_WEAPON_THROW3:
		SetChargerState( CHARGER_STATE_READY );
		break;
 
	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}
 
#endif
 
//-----------------------------------------------------------------------------
// Purpose: Set the desired activity for the weapon and its viewmodel counterpart
// Input  : iActivity - activity to play
//-----------------------------------------------------------------------------
bool CWeaponGrapple::SendWeaponAnim( int iActivity )
{
	int newActivity = iActivity;
 
	// The last shot needs a non-loaded activity
//	if ( ( newActivity == ACT_VM_IDLE ) && ( m_iClip1 <= 0 ) )
//	{
//		newActivity = ACT_VM_FIDGET;
//	}
 
	//For now, just set the ideal activity and be done with it
	return BaseClass::SendWeaponAnim( newActivity );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	if ( m_hHook )
	{
		m_hHook->SetTouch( NULL );
		m_hHook->SetThink( NULL );
 
		UTIL_Remove( m_hHook );
		m_hHook = NULL;
 
		NotifyHookDied();
 
		m_bMustReload = true;
	}
#endif
 
	if ( m_bInZoom )
	{
		ToggleZoom();
	}
 
	SetChargerState( CHARGER_STATE_OFF );
 
	BaseClass::Drop( vecVelocity );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponGrapple::HasAnyAmmo( void )
{
	if ( m_hHook != NULL )
		return true;
 
	return BaseClass::HasAnyAmmo();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponGrapple::CanHolster( void )
{
	//Can't have an active hook out
	if ( m_hHook != NULL )
		return false;
 
	return BaseClass::CanHolster();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrapple::NotifyHookDied( void )
{
	m_hHook = NULL;
 
#ifndef CLIENT_DLL
	if ( m_hRope )
	{
		UTIL_Remove( m_hRope );
		m_hRope = NULL;
	}
#endif
}