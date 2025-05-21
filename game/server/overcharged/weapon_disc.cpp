//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "sprite.h"
#include "spritetrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"

#include "effect_dispatch_data.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar aa_wpn_disc_model( "aa_wpn_disc_model", "models/weapons/w_missile_closed.mdl", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Sets the model of weapon disc." );

#define BOLT_MODEL	aa_wpn_disc_model.GetString() //"models/disc.mdl"

#define BOLT_AIR_VELOCITY	500
#define BOLT_WATER_VELOCITY	500
#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1

//extern ConVar sk_plr_dmg_disc;
//extern ConVar sk_npc_dmg_disc;
ConVar	sk_plr_dmg_disc( "sk_plr_dmg_disc", "50" );
ConVar	sk_npc_dmg_disc( "sk_plr_dmg_disc", "50" );

#ifndef CLIENT_DLL

void TE_StickyBolt( IRecipientFilter& filter, float delay,	Vector vecDirection, const Vector *origin );

//-----------------------------------------------------------------------------
// Disc
//-----------------------------------------------------------------------------
class CDiscProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS( CDiscProjectile, CBaseCombatCharacter );

public:
	CDiscProjectile() { };
	~CDiscProjectile();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CDiscProjectile *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL );

protected:

	bool	CreateSprites( void );

	CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;
	
	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS( disc_projectile, CDiscProjectile );

BEGIN_DATADESC( CDiscProjectile )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),

	// These are recreated on reload, they don't need storage
	DEFINE_FIELD( m_pGlowSprite, FIELD_EHANDLE ),
	//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CDiscProjectile, DT_DiscProjectile )
END_SEND_TABLE()

CDiscProjectile *CDiscProjectile::BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner )
{
	// Create a new entity with CDiscProjectile private data
	CDiscProjectile *pDiscProjectile = (CDiscProjectile *)CreateEntityByName( "disc_projectile" );
	UTIL_SetOrigin( pDiscProjectile, vecOrigin );
	pDiscProjectile->SetAbsAngles( angAngles );
	pDiscProjectile->Spawn();
	pDiscProjectile->SetOwnerEntity( pentOwner );

	pDiscProjectile->m_iDamage = iDamage;

	return pDiscProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CDiscProjectile::~CDiscProjectile( void )
{
	if ( m_pGlowSprite )
	{
		UTIL_Remove( m_pGlowSprite );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CDiscProjectile::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CDiscProjectile::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CDiscProjectile::CreateSprites( void )
{
	// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate( "sprites/light_glow02_noz.vmt", GetLocalOrigin(), false );

	if ( m_pGlowSprite != NULL )
	{
		m_pGlowSprite->FollowEntity( this );
		m_pGlowSprite->SetTransparency( kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation );
		m_pGlowSprite->SetScale( 0.2f );
		m_pGlowSprite->TurnOff();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDiscProjectile::Spawn( void )
{
	Precache( );

	SetModel( BOLT_MODEL );
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(10,10,10), Vector(10,10,10) );
	SetSolid( SOLID_BBOX );
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CDiscProjectile::BoltTouch );

	SetThink( &CDiscProjectile::BubbleThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	
	CreateSprites();

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW;
}


void CDiscProjectile::Precache( void )
{
	PrecacheModel( BOLT_MODEL );

	PrecacheModel( "sprites/light_glow02_noz.vmt" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CDiscProjectile::BoltTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC() )
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_disc.GetFloat(), DMG_NEVERGIB | DMG_DISSOLVE ); // Light Kill : added DMG_DISSOLVE to make better resuilt
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}
		else
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_disc.GetFloat(), DMG_BULLET | DMG_NEVERGIB | DMG_DISSOLVE ); // Light Kill : Same...
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Weapon_Crossbow.BoltHitBody" );

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if ( tr2.fraction != 1.0f )
		{
//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if ( tr2.m_pEnt == NULL || ( tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;
			
				DispatchEffect( "BoltImpact", data );
			}
		}
		
		SetTouch( NULL );
		SetThink( NULL );

		UTIL_Remove( this );
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// Reflect off of anything that isnt the sky
		if (tr.surface.flags & SURF_SKY)
		{
			UTIL_Remove( this );
		}
		else
		{
			EmitSound( "Weapon_Crossbow.BoltHitWorld" );

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize( vecDir );

			float hitDot = DotProduct( tr.plane.normal, -vecDir );

			Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;
				
			QAngle reflectAngles;

			VectorAngles( vReflection, reflectAngles );

			SetLocalAngles( reflectAngles );

			SetAbsVelocity( vReflection * speed * 0.75f );
			
			// Shoot some sparks
			if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
			{
				g_pEffects->Sparks( GetAbsOrigin() );
			}
		}
	}

	if ( g_pGameRules->IsMultiplayer() )
	{
//		SetThink( &CDiscProjectile::ExplodeThink );
//		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDiscProjectile::BubbleThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( GetWaterLevel()  == 0 )
		return;

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

#endif

//-----------------------------------------------------------------------------
// CWeaponDisc
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponDisc C_WeaponDisc
#endif

class CWeaponDisc : public CBaseHLCombatWeapon //CWeaponSDKBase
{
	DECLARE_CLASS( CWeaponDisc, CBaseHLCombatWeapon );
public:

	CWeaponDisc( void );
	
	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	//virtual void	SecondaryAttack( void );
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool	Reload( void );
	virtual void	ItemPostFrame( void );
	//virtual void	ItemBusyFrame( void );
	virtual bool	SendWeaponAnim( int iActivity );

#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	//DECLARE_NETWORKCLASS(); 
	//DECLARE_PREDICTABLE();
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	
	//void	SetSkin( int skinNum );		// Light Kill : Do not use custom skins for weapon disc
	//void	CheckZoomToggle( void );
	void	FireBolt( void );
	//void	ToggleZoom( void );
	
	// Various states for the crossbow's charger
	enum ChargerState_t
	{
		CHARGER_STATE_START_LOAD,
		CHARGER_STATE_START_CHARGE,
		CHARGER_STATE_READY,
		CHARGER_STATE_DISCHARGE,
		CHARGER_STATE_OFF,
	};

	void	CreateChargerEffects( void );
	void	SetChargerState( ChargerState_t state );
	void	DoLoadEffect( void );

	DECLARE_ACTTABLE();

private:
	
	/*// Charger effects
	ChargerState_t		m_nChargeState;

#ifndef CLIENT_DLL
	CHandle<CSprite>	m_hChargerSprite;
#endif

	CNetworkVar( bool,	m_bInZoom );
	CNetworkVar( bool,	m_bMustReload );

	CWeaponDisc( const CWeaponDisc & );*/

	// Charger effects
	ChargerState_t		m_nChargeState;
	CHandle<CSprite>	m_hChargerSprite;

	bool				m_bInZoom;
	bool				m_bMustReload;
};

/*IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDisc, DT_WeaponDisc )

BEGIN_NETWORK_TABLE( CWeaponDisc, DT_WeaponDisc )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bInZoom ) ),
	RecvPropBool( RECVINFO( m_bMustReload ) ),
#else
	SendPropBool( SENDINFO( m_bInZoom ) ),
	SendPropBool( SENDINFO( m_bMustReload ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponDisc )
	DEFINE_PRED_FIELD( m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bMustReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_disc, CWeaponDisc );

PRECACHE_WEAPON_REGISTER( weapon_disc );

acttable_t	CWeaponDisc::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_CROSSBOW,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_CROSSBOW,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_CROSSBOW,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_CROSSBOW,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_CROSSBOW,					false },
};

IMPLEMENT_ACTTABLE(CWeaponDisc);*/

LINK_ENTITY_TO_CLASS( weapon_disc, CWeaponDisc );

PRECACHE_WEAPON_REGISTER( weapon_disc );

IMPLEMENT_SERVERCLASS_ST( CWeaponDisc, DT_WeaponDisc )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponDisc )

	DEFINE_FIELD( m_bInZoom,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMustReload,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nChargeState,	FIELD_INTEGER ),
	DEFINE_FIELD( m_hChargerSprite,	FIELD_EHANDLE ),

END_DATADESC()

acttable_t	CWeaponDisc::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PHYSGUN,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PHYSGUN,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,                    false },
    { ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM,                false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims

	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,			true },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SHOTGUN,			true },
	//{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,		true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		false },
	{ ACT_WALK,						ACT_WALK_RIFLE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponDisc);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponDisc::CWeaponDisc( void )
{
	m_bReloadsSingly	= true;
	m_bFiresUnderwater	= true;
	m_bInZoom			= false;
	m_bMustReload		= false;
}

#define	CROSSBOW_GLOW_SPRITE	"sprites/light_glow02_noz.vmt"
#define	CROSSBOW_GLOW_SPRITE2	"sprites/blueflare1.vmt"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDisc::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "disc_projectile" );
#endif

	PrecacheScriptSound( "Weapon_Crossbow.BoltHitBody" );
	PrecacheScriptSound( "Weapon_Crossbow.BoltHitWorld" );
	PrecacheScriptSound( "Weapon_Crossbow.BoltSkewer" );

	PrecacheModel( CROSSBOW_GLOW_SPRITE );
	PrecacheModel( CROSSBOW_GLOW_SPRITE2 );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponDisc::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	pPlayer->SetAnimation( PLAYER_ATTACK1 );				// Light Kill : Forgotten anim ?

	/*if ( m_bInZoom && g_pGameRules->IsMultiplayer() )
	{
//		FireSniperBolt();
		FireBolt();
	}
	else
	{*/
		FireBolt();
	//}

	// Signal a reload
	m_bMustReload = true;

	//SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration( ACT_VM_PRIMARYATTACK ) );
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;	// Light Kill : little delay between shoots
	//PrepareHitmarker();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*void CWeaponDisc::SecondaryAttack( void )
{
	//NOTENOTE: The zooming is handled by the post/busy frames
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDisc::Reload( void )
{
	if ( BaseClass::Reload() )
	{
		m_bMustReload = false;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDisc::CheckZoomToggle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer->m_afButtonPressed & IN_ATTACK2 )
	{
		ToggleZoom();
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDisc::ItemBusyFrame( void )
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDisc::ItemPostFrame( void )
{
	// Allow zoom toggling
	//CheckZoomToggle();

	if ( m_bMustReload && HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDisc::FireBolt( void )
{
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

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

#ifndef CLIENT_DLL
	Vector vecAiming	= pOwner->GetAutoaimVector( 0 );	
	Vector vecSrc		= pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );

	//CDiscProjectile *pDiscProjectile = CDiscProjectile::BoltCreate( vecSrc, angAiming, GetSDKWpnData().m_iDamage, pOwner );
	CDiscProjectile *pDiscProjectile = CDiscProjectile::BoltCreate( vecSrc, angAiming, 0, pOwner ); // Light Kill : Test

	if ( pOwner->GetWaterLevel() == 3 )
	{
		pDiscProjectile->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	}
	else
	{
		pDiscProjectile->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
	}

#endif

	m_iClip1--;

	pOwner->ViewPunch( QAngle( -2, 0, 0 ) );

	WeaponSound( SINGLE );
	WeaponSound( SPECIAL2 );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if ( !m_iClip1 && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + 0.75;

	DoLoadEffect();
	SetChargerState( CHARGER_STATE_DISCHARGE );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDisc::Deploy( void )
{
	/*if ( m_iClip1 <= 0 )
	{
		return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix() );
	}*/

	//SetSkin( BOLT_SKIN_GLOW );

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDisc::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	/*if ( m_bInZoom )
	{
		ToggleZoom();
	}*/

	SetChargerState( CHARGER_STATE_OFF );

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDisc::ToggleZoom( void )
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
}*/

#define	BOLT_TIP_ATTACHMENT	2

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDisc::CreateChargerEffects( void )
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
/*void CWeaponDisc::SetSkin( int skinNum )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if ( pViewModel == NULL )
		return;

	pViewModel->m_nSkin = skinNum;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDisc::DoLoadEffect( void )
{
	//SetSkin( BOLT_SKIN_GLOW );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Tony; change this up a bit; on the server, dispatch an effect but don't send it to the client who fires
	//on the client, create an effect either in the view model, or on the world model if first person.
	CEffectData	data;

	data.m_nAttachmentIndex = 1;

#ifdef GAME_DLL
	data.m_nEntIndex = entindex();

	CPASFilter filter( data.m_vOrigin );
	filter.RemoveRecipient( pOwner );
	te->DispatchEffect( filter, 0.0, data.m_vOrigin, "CrossbowLoad", data );
#else
	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if ( pViewModel != NULL )
	{

		if ( ::input->CAM_IsThirdPerson() )
			data.m_hEntity = pViewModel->GetRefEHandle();
		else
			data.m_hEntity = GetRefEHandle();
		DispatchEffect( "CrossbowLoad", data );
	}
#endif

	//Tony; switched this up, always attach it to the weapon, not the view model!!
#ifndef CLIENT_DLL
	CSprite *pBlast = CSprite::SpriteCreate( CROSSBOW_GLOW_SPRITE2, GetAbsOrigin(), false );

	if ( pBlast )
	{
		pBlast->SetAttachment( this, 1 );
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
void CWeaponDisc::SetChargerState( ChargerState_t state )
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
			//SetSkin( BOLT_SKIN_NORMAL );
			
			if ( m_hChargerSprite == NULL )
				break;
			
			m_hChargerSprite->SetBrightness( 0 );
			m_hChargerSprite->TurnOff();
		}

		break;
#endif
	case CHARGER_STATE_OFF:
		{
			//SetSkin( BOLT_SKIN_NORMAL );

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
void CWeaponDisc::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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
bool CWeaponDisc::SendWeaponAnim( int iActivity )
{
	int newActivity = iActivity;

	// The last shot needs a non-loaded activity
	if ( ( newActivity == ACT_VM_IDLE ) && ( m_iClip1 <= 0 ) )
	{
		newActivity = ACT_VM_FIDGET;
	}

	//For now, just set the ideal activity and be done with it
	return BaseClass::SendWeaponAnim( newActivity );
}