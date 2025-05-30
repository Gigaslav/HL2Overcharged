//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the hand barnacle weapon.
//			
//			Primary attack: Get player at the point
//			Secondary attack: Eat covards
//
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_barnacle.h"
#include "decals.h"
 
#ifdef CLIENT_DLL
	//#include "c_hl2mp_player.h"      
    #include "player.h"                
    //#include "c_te_effect_dispatch.h"
    #include "te_effect_dispatch.h"    
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

	#include "ammodef.h"		/* This is needed for the tracing done later */
	#include "gamestats.h" //
	#include "soundent.h" //
 
	#include "vphysics/constraints.h"
	#include "physics_saverestore.h"
 
#endif
 
//#include "effect_dispatch_data.h"
 
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
 
#define HOOK_MODEL			"models/props_junk/rock001a.mdl"

#define BOLT_AIR_VELOCITY	3500
#define BOLT_WATER_VELOCITY	1500

ConVar oc_weapon_barnacle_speed( "oc_weapon_barnacle_speed", "250", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Player speed on activate." );
ConVar oc_weapon_barnacle_material( "oc_weapon_barnacle_material", "sprites/physbeam.vmt", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Material of barnacle tonque" );
ConVar oc_weapon_barnacle_material_width( "oc_weapon_barnacle_material_width", "5", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Material width." );
ConVar oc_weapon_barnacle_plr_dmg( "oc_weapon_barnacle_plr_dmg", "25", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Secondary attack dmg." );
 
#ifndef CLIENT_DLL
 
LINK_ENTITY_TO_CLASS( barnacle_hook, CbarnacleHook );
 
BEGIN_DATADESC( CbarnacleHook )
	// Function Pointers
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( HookedThink ),
	DEFINE_FUNCTION( HookTouch ),

	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hBolt, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bPlayerWasStanding, FIELD_BOOLEAN ),
 
END_DATADESC()
 
CbarnacleHook *CbarnacleHook::HookCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner )
{
	// Create a new entity with CbarnacleHook private data
	CbarnacleHook *pHook = (CbarnacleHook *)CreateEntityByName( "barnacle_hook" );
	UTIL_SetOrigin( pHook, vecOrigin );
	pHook->SetAbsAngles( angAngles );
	pHook->Spawn();
 
	CWeaponbarnacle *pOwner = (CWeaponbarnacle *)pentOwner;
	pHook->m_hOwner = pOwner;
	pHook->SetOwnerEntity( pOwner->GetOwner() );
	pHook->m_hPlayer = (CBasePlayer *)pOwner->GetOwner();
 
	return pHook;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CbarnacleHook::~CbarnacleHook( void )
{ 
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
bool CbarnacleHook::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );
 
	return true;
}
 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CbarnacleHook::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}
 
//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CbarnacleHook::Spawn( void )
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
 
	SetTouch( &CbarnacleHook::HookTouch );
 
	SetThink( &CbarnacleHook::FlyThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
 
	m_pSpring		= NULL;
	m_fSpringLength = 0.0f;
	m_bPlayerWasStanding = false;
}
 
 
void CbarnacleHook::Precache( void )
{
	PrecacheModel( HOOK_MODEL );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CbarnacleHook::HookTouch( CBaseEntity *pOther )
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

		const surfacedata_t *ptexdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	// L1ght 15 : Check for extras
 
		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && ( (char) ptexdata->game.material == CHAR_TEX_ALIENFLESH ) )	//!( tr.surface.flags & SURF_SKY ) )
		{
			EmitSound( "Weapon_Barnacle.Release" ); //"Weapon_AR2.Reload_Push" );
 
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
 
		//	AddEffects( EF_NODRAW );
			SetTouch( NULL );

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
 
			//IPhysicsObject *pPhysObject = m_hPlayer->VPhysicsGetObject();
			IPhysicsObject *pRootPhysObject = VPhysicsGetObject();
			Assert( pRootPhysObject );
//			Assert( pPhysObject );
 
			pRootPhysObject->EnableMotion( false );
 
			// Root has huge mass, tip has little
			pRootPhysObject->SetMass( VPHYSICS_MAX_MASS );
		//	pPhysObject->SetMass( 100 );
		//	float damping = 3;
		//	pPhysObject->SetDamping( &damping, &damping );
 
			Vector origin = m_hPlayer->GetAbsOrigin();
			Vector rootOrigin = GetAbsOrigin();
			m_fSpringLength = (origin - rootOrigin).Length();
 
			m_bPlayerWasStanding = ( ( m_hPlayer->GetFlags() & FL_DUCKING ) == 0 );
 
			SetThink( &CbarnacleHook::HookedThink );
			SetNextThink( gpGlobals->curtime + 0.1f );
		}
		else
		{
			// Put a mark unless we've hit the sky
			/*if ( ( tr.surface.flags & SURF_SKY ) == false )		// L1ght 15: No more needed, no impacts on touch
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
			}*/
 
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
void CbarnacleHook::HookedThink( void )
{
	//set next globalthink
	SetNextThink( gpGlobals->curtime + 0.2f); //0.05f ); //0.1f

	//All of this push the player far from the hook
	Vector tempVec1 = m_hPlayer->GetAbsOrigin() - GetAbsOrigin();
	VectorNormalize(tempVec1);

	int temp_multiplier = -1;

	//m_hPlayer->SetGravity(0.0f);		// BriJee: GRAVITY not needed to change
	m_hPlayer->SetGroundEntity(NULL);


	if (m_hPlayer->m_nButtons & IN_DUCK)		// L1ght 15 : Player lowering
	{
		m_hPlayer->SetAbsVelocity(tempVec1*temp_multiplier*50);
	}
	else
	{

	/*if (m_hOwner->m_bHook)
	{
		//temp_multiplier = 1;*/	// Backwards? wut
		m_hPlayer->SetAbsVelocity(tempVec1*temp_multiplier*oc_weapon_barnacle_speed.GetFloat());	//def 400
	/*}
	else
	{
		m_hPlayer->SetAbsVelocity(tempVec1*temp_multiplier*400);//400
	}*/
	}
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CbarnacleHook::FlyThink( void )
{
	QAngle angNewAngles;
 
	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );
 
	SetNextThink( gpGlobals->curtime + 0.1f );
}
 
#endif
 
//IMPLEMENT_NETWORKCLASS_ALIASED( Weaponbarnacle, DT_Weaponbarnacle )
 
#ifdef CLIENT_DLL
void RecvProxy_HookDied( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CWeaponbarnacle *pbarnacle = ((CWeaponbarnacle*)pStruct);
 
	RecvProxy_IntToEHandle( pData, pStruct, pOut );
 
	CBaseEntity *pNewHook = pbarnacle->GetHook();
 
	if ( pNewHook == NULL )
	{
		if ( pbarnacle->GetOwner() && pbarnacle->GetOwner()->GetActiveWeapon() == pbarnacle )
		{
			pbarnacle->NotifyHookDied();
		}
	}
}
#endif
 /*
BEGIN_NETWORK_TABLE( CWeaponbarnacle, DT_Weaponbarnacle )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bInZoom ) ),
	RecvPropBool( RECVINFO( m_bMustReload ) ),
	RecvPropEHandle( RECVINFO( m_hHook ), RecvProxy_HookDied ),

	RecvPropInt ( RECVINFO (m_nBulletType)),
#else
	SendPropBool( SENDINFO( m_bInZoom ) ),
	SendPropBool( SENDINFO( m_bMustReload ) ),
	SendPropEHandle( SENDINFO( m_hHook ) ),

	SendPropInt ( SENDINFO (m_nBulletType)),
#endif
END_NETWORK_TABLE()
 
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponbarnacle )
	DEFINE_PRED_FIELD( m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bMustReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif
 
LINK_ENTITY_TO_CLASS( weapon_barnacle, CWeaponbarnacle );
 
PRECACHE_WEAPON_REGISTER( weapon_barnacle );*/

LINK_ENTITY_TO_CLASS( weapon_barnacle, CWeaponbarnacle );

PRECACHE_WEAPON_REGISTER( weapon_barnacle );

IMPLEMENT_SERVERCLASS_ST( CWeaponbarnacle, DT_Weaponbarnacle )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponbarnacle )

	DEFINE_FIELD( m_bInZoom,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMustReload,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nBulletType,	FIELD_INTEGER ),

	//DEFINE_FIELD( m_hHook,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hHook,	FIELD_VOID ),

END_DATADESC()
 
#ifndef CLIENT_DLL

acttable_t	CWeaponbarnacle::m_acttable[] = 
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
 
IMPLEMENT_ACTTABLE(CWeaponbarnacle);
 
#endif
 
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponbarnacle::CWeaponbarnacle( void )
{
	m_bReloadsSingly	= true;
	m_bInZoom			= false;
	m_bMustReload		= false;

	m_nBulletType = -1;

	
	#ifndef CLIENT_DLL
		//m_pLightGlow= NULL;
		pBeam	= NULL;
	#endif
}
  
//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponbarnacle::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "barnacle_hook" );
#endif
 
//	PrecacheScriptSound( "Weapon_Crossbow.BoltHitBody" );
//	PrecacheScriptSound( "Weapon_Crossbow.BoltHitWorld" );
//	PrecacheScriptSound( "Weapon_Crossbow.BoltSkewer" );
	PrecacheScriptSound( "Weapon_Barnacle.Release" );
 
	PrecacheModel(oc_weapon_barnacle_material.GetString());//( "sprites/physbeam.vmt" );
	//PrecacheModel( "sprites/physcannon_bluecore2b.vmt" );
 
	BaseClass::Precache();
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponbarnacle::PrimaryAttack( void )
{
	// Can't have an active hook out
	if ( m_hHook != NULL )
		return;

	PrecacheModel(oc_weapon_barnacle_material.GetString());
 
	#ifndef CLIENT_DLL
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	WeaponSound( SINGLE );
	//pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;

	//Disabled so we can shoot all the time that we want
	//m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	

	//We will not shoot bullets anymore
	//pPlayer->FireBullets( 1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );

	//pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );


	trace_t tr;
	Vector vecShootOrigin, vecShootDir, vecDir, vecEnd;

	//Gets the direction where the player is aiming
	AngleVectors (pPlayer->EyeAngles(), &vecDir);

	//Gets the position of the player
	vecShootOrigin = pPlayer->Weapon_ShootPosition();

	//Gets the position where the hook will hit
	vecEnd	= vecShootOrigin + (vecDir * MAX_TRACE_LENGTH);	
	
	//Traces a line between the two vectors
	UTIL_TraceLine( vecShootOrigin, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	//Draws the beam
	DrawBeam( vecShootOrigin, tr.endpos, 15.5 );

	if ( tr.DidHitWorld() )
	{
		UTIL_DecalTrace( &tr , "ManhackCut" );
	}

				CEffectData data;
				data.m_vStart = tr.startpos;
				data.m_vOrigin = tr.endpos;
					
				DispatchEffect( "RagdollImpact", data );

	//Creates an energy impact effect if we don't hit the sky or other places
	/*if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		m_nBulletType = GetAmmoDef()->Index("GaussEnergy");
		UTIL_ImpactTrace( &tr, m_nBulletType );

		//Makes a sprite at the end of the beam
		//m_pLightGlow = CSprite::SpriteCreate( "sprites/physcannon_bluecore2b.vmt", GetAbsOrigin(), TRUE);

		//Sets FX render and color
		//m_pLightGlow->SetTransparency( 9, 255, 255, 255, 200, kRenderFxNoDissipation );
		
		//Sets the position
		//m_pLightGlow->SetAbsOrigin(tr.endpos);
		
		//Bright
		//m_pLightGlow->SetBrightness( 255 );
		
		//Scale
		//m_pLightGlow->SetScale( 0.65 );
	}*/
	#endif

	FireHook();
 
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration( ACT_VM_PRIMARYATTACK ) );
}
 
//-----------------------------------------------------------------------------
// Purpose: Toggles the barnacle hook modes
//-----------------------------------------------------------------------------
void CWeaponbarnacle::SecondaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
		return;

	if (pPlayer->m_nButtons & IN_ATTACK)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	SendWeaponAnim(GetSecondaryAttackActivity());
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

/*	Vector	vecDirection, vecStart, vecStop, vecDir, vecEnd;
    AngleVectors( GetAbsAngles(), &vecDirection );
	VectorMA( pPlayer->Weapon_ShootPosition(), 50, vecDirection, vecEnd );

	// Set up angles
	AngleVectors( pPlayer->EyeAngles( ), &vecDir );

	// Get up vectors
	vecStart = pPlayer->EyePosition();
	vecStop = vecStart + vecDir * 70;

	CBaseEntity *pHurt = pPlayer->CheckTraceHullAttack( pPlayer->Weapon_ShootPosition(), vecEnd, 
			Vector(-1,-1,-1), Vector(0,0,0), oc_weapon_barnacle_plr_dmg.GetFloat(), DMG_CLUB, 0.75 );

	if ( pHurt )
	{
		pPlayer->ViewPunch( QAngle( -2, 1, 2 ) );
		//Msg("DAMAGED!!!!!!!!!!!! \n");
	}
	else
	{
		pPlayer->ViewPunch( QAngle( -3, 0, 1 ) );
		//Msg("NOPE!!!!!!!!!!!!!! \n");
	}*/

	// set up the vectors and traceline
	trace_t tr, m_trHit;	// Hit Target
	Vector	vecStart, vecStop, vecDir, vecEnd;

	// get the angles
	AngleVectors( pPlayer->EyeAngles( ), &vecDir );

	// get the vectors
	vecStart = pPlayer->Weapon_ShootPosition();
	vecStop = vecStart + vecDir * MAX_TRACE_LENGTH;

	// do the traceline
	UTIL_TraceLine( vecStart, vecStop, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr );

	CBaseEntity *pObject = NULL;
	pObject = tr.m_pEnt;

	if ( tr.m_pEnt )
	{
		if ((tr.endpos.DistTo(vecStart) > 10) || tr.m_pEnt->IsNPC()) // Make sure we shoot if have enough space or aim on npc
		{
			CBaseEntity *pHurt = pPlayer->CheckTraceHullAttack( 45, Vector(-16,-16,-16), Vector(16,16,16), oc_weapon_barnacle_plr_dmg.GetFloat(), DMG_CLUB, 1.0f, true );
			if (pHurt)
			{
				float X = -2.f;
				if (abs(pPlayer->LocalEyeAngles().x) + abs(X) >= 89.0f)
				{
					X = X < 0 ? (89.0f - abs(pPlayer->LocalEyeAngles().x)) * -1.f : 89.0f - abs(pPlayer->LocalEyeAngles().x);
				}
				pPlayer->ViewPunch( QAngle( X, 1, 2 ) );
				/*CBaseAnimating *pAnim;
				pAnim = dynamic_cast<CBaseAnimating*>(pObject);
				if ( pAnim )
				{
					pAnim->Ignite( 30.0f );
				}

				pObject->AddFlag(FL_ONFIRE);*/
			}
			else
			{
				float X = -3.f;
				if (abs(pPlayer->LocalEyeAngles().x) + abs(X) >= 89.0f)
				{
					X = X < 0 ? (89.0f - abs(pPlayer->LocalEyeAngles().x)) * -1.f : 89.0f - abs(pPlayer->LocalEyeAngles().x);
				}
				pPlayer->ViewPunch( QAngle( X, 0, 1 ) );
				//StopParticleEffects(pPlayer->GetViewModel()); // stop that
				//return;
			}
		}
				//Msg("Too Close \n");
	}

	WeaponSound( SPECIAL1 );
	//WeaponSound( EMPTY );

	m_flNextSecondaryAttack = gpGlobals->curtime + 1;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponbarnacle::Reload( void )
{
	if ( ( m_bMustReload ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_IDLE ); //ACT_VM_RELOAD
 
		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
 
		//Mark this as done
		m_bMustReload = false;
	}
 
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Toggles between pull and rappel mode
//-----------------------------------------------------------------------------
/*bool CWeaponbarnacle::ToggleHook( void )
{
	#ifndef CLIENT_DLL

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
 
	if ( m_bHook )
	{
		m_bHook = false;
		ClientPrint(pPlayer,HUD_PRINTCENTER, "Pull mode");
		return m_bHook;
	}
	else
	{
		m_bHook = true;
		ClientPrint(pPlayer,HUD_PRINTCENTER, "Rappel mode");
		return m_bHook;
	}
	#endif
	//return !m_bHook;
}*/
  
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponbarnacle::ItemBusyFrame( void )
{
	// Allow zoom toggling even when we're reloading
	//CheckZoomToggle();
}


	/*//Msg ("fly think \n");
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();

	if (!pWeapon)
		return;
	//pWeapon->SendViewModelAnim( ACT_VM_PRIMARYATTACK );
	pWeapon->SendWeaponAnim( ACT_VM_PRIMARYATTACK );*/
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponbarnacle::ItemPostFrame( void )
{
	//Enforces being able to use PrimaryAttack and Secondary Attack
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if ( ( pOwner->m_nButtons & IN_ATTACK ) )
	{
		pOwner->m_nButtons |= IN_USE;	// L1ght 15 : Whoah i found a fix for +USE and prop grab bug	// De use of IN_USE?
		//pOwner->m_nButtons &= ~IN_USE;	// Seems like non pressed by player keyboard method +IN_USE

		if ( m_flNextPrimaryAttack < gpGlobals->curtime )
		{

			if (!(pOwner->m_nButtons & IN_DUCK))	// L1ght 15 : Player lowering
			{////////////////


			PrimaryAttack();

			if (IsViewModelSequenceFinished())		// L1ght 15 : Loop attack anim if weapon active
			{
				SendWeaponAnim( ACT_VM_PRIMARYATTACK );
				WeaponSound( SPECIAL2 );
			}

			}////////////////

		}
	}
	else if ( m_bMustReload ) //&& HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}

	/*if ( ( pOwner->m_afButtonPressed & IN_ATTACK2 ) )
	{
		if ( m_flNextPrimaryAttack < gpGlobals->curtime )
		{
			SecondaryAttack();
		}
	}
	else if ( m_bMustReload ) //&& HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}*/
 
	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}

#ifndef CLIENT_DLL
	if ( m_hHook ) //(pOwner->m_afButtonPressed & IN_ATTACK) )
	{
		if /*(pOwner->m_afButtonReleased & IN_ATTACK || (pOwner->m_nButtons & IN_ATTACK2))*/ (!(pOwner->m_nButtons & IN_ATTACK) && !(pOwner->m_nButtons & IN_ATTACK2))
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
 
//-----------------------------------------------------------------------------
// Purpose: Fires the hook
//-----------------------------------------------------------------------------
void CWeaponbarnacle::FireHook( void )
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
 
	CbarnacleHook *pHook = CbarnacleHook::HookCreate( vecSrc, angAiming, this );
 
	if ( pOwner->GetWaterLevel() == 3 )
	{
		pHook->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	}
	else
	{
		pHook->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
	}
 
	m_hHook = pHook;
 
#endif
	float X = -2.f;
	if (abs(pOwner->LocalEyeAngles().x) + abs(X) >= 89.0f)
	{
		X = X < 0 ? (89.0f - abs(pOwner->LocalEyeAngles().x)) * -1.f : 89.0f - abs(pOwner->LocalEyeAngles().x);
	}

	pOwner->ViewPunch( QAngle( X, 0, 0 ) );
 
	//WeaponSound( SINGLE );
	//WeaponSound( SPECIAL2 );
 
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
 
	m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + 0.75;
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponbarnacle::Deploy( void )
{
	if ( m_bMustReload )
	{
		return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix() );
	}
 
	return BaseClass::Deploy();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponbarnacle::Holster( CBaseCombatWeapon *pSwitchingTo )
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
 
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponbarnacle::Drop( const Vector &vecVelocity )
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
		UTIL_Remove( this );

		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );	// L1ght 15 : Little hack to avoid multiple hooks
		if (pPlayer)
			CBaseEntity::Create( "weapon_barnacle", pPlayer->Weapon_ShootPosition(), QAngle(80,60,0), pPlayer );
	}
#endif
	else
	{
		BaseClass::Drop( vecVelocity );
	}
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponbarnacle::HasAnyAmmo( void )
{
	if ( m_hHook != NULL )
		return true;
 
	return BaseClass::HasAnyAmmo();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponbarnacle::CanHolster( void )
{
	//Can't have an active hook out
	if ( m_hHook != NULL )
		return false;
 
	return BaseClass::CanHolster();
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponbarnacle::NotifyHookDied( void )
{
	m_hHook = NULL;
 
#ifndef CLIENT_DLL
	if ( pBeam )
	{
		UTIL_Remove( pBeam ); //Kill beam
		pBeam = NULL;

		//UTIL_Remove( m_pLightGlow ); //Kill sprite
		//m_pLightGlow = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Draws a beam
// Input  : &startPos - where the beam should begin
//          &endPos - where the beam should end
//          width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeaponbarnacle::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{
#ifndef CLIENT_DLL
	//Tracer down the middle (NOT NEEDED, IT WILL FIRE A TRACER)
	//UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );
 
	trace_t tr;
	//Draw the main beam shaft
	pBeam = CBeam::BeamCreate(oc_weapon_barnacle_material.GetString(), oc_weapon_barnacle_material_width.GetFloat());//( "sprites/physbeam.vmt", 15.5 );

	// It starts at startPos
	pBeam->SetStartPos( startPos );
 
	// This sets up some things that the beam uses to figure out where
	// it should start and end
	pBeam->PointEntInit( endPos, this );
 
	// This makes it so that the beam appears to come from the muzzle of the pistol
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );
	pBeam->SetWidth( oc_weapon_barnacle_material_width.GetFloat());//width );
//	pBeam->SetEndWidth( 0.05f );
 
	// Higher brightness means less transparent
	pBeam->SetBrightness( 255 );
	//pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();

	//Sets scrollrate of the beam sprite 
	float scrollOffset = gpGlobals->curtime + 5.5;
	pBeam->SetScrollRate(scrollOffset);
 
	// The beam should only exist for a very short time
	//pBeam->LiveForTime( 0.1f );

	UpdateWaterState();
 
	SetTouch( &CbarnacleHook::HookTouch );
 
	SetThink( &CbarnacleHook::FlyThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
#endif
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - used to figure out where to do the effect
//          nDamageType - ???
//-----------------------------------------------------------------------------
/*void CWeaponbarnacle::DoImpactEffect( trace_t &tr, int nDamageType )
{
#ifndef CLIENT_DLL
	if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		m_nBulletType = GetAmmoDef()->Index("GaussEnergy");
		UTIL_ImpactTrace( &tr, m_nBulletType );
	}
#endif
}*/