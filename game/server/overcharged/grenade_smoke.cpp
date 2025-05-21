//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
//#include "grenade_frag.h"
//#include "npc_citizen17.h"
#include "grenade_smoke.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SMOKE_GRENADE_BLIP_FREQUENCY			1.0f
#define SMOKE_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define SMOKE_GRENADE_GRACE_TIME_AFTER_PICKUP 0.5f
#define SMOKE_GRENADE_WARN_TIME 1.0f

const float SMOKE_GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

ConVar smoke_grenade_radius	( "sk_smoke_grenade_radius", "256");
ConVar smoke_grenade_damage( "sk_smoke_grenade_damage", "5" );
ConVar oc_weapon_smokegrenade_model( "oc_weapon_smokegrenade_model", "models/weapons/w_grenade.mdl", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Sets the model of smoke grenade." );
ConVar oc_weapon_smokegrenade_effect( "oc_weapon_smokegrenade_effect", "Rocket_Smoke" /*"striderbuster_smoke"*/, FCVAR_REPLICATED|FCVAR_ARCHIVE, "Sets the particle effect of smoke grenade." );

#define SMOKE_GRENADE_MODEL oc_weapon_smokegrenade_model.GetString() //"models/Weapons/w_grenade.mdl"
#define SMOKE_GRENADE_DETONATE_SOUND "HeadcrabCanister.AfterLanding"	//ambient.steam01 or HeadcrabCanister.AfterLanding
#define SMOKE_GRENADE_BREAK_SOUND "Padlock.Break"

LINK_ENTITY_TO_CLASS( npc_grenade_smoke, CGrenadeSmoke );

//DECLARE_CLASS(CGrenadeSmoke, CBaseGrenade);

BEGIN_DATADESC( CGrenadeSmoke )

	// Fields
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_punted, FIELD_BOOLEAN ),
	DEFINE_FIELD(m_bStartSmoke, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),
	DEFINE_THINKFUNC( SmokeThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimer", InputSetTimer ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CGrenadeSmoke, DT_GrenadeSmoke )
	SendPropBool( SENDINFO( m_bStartSmoke )),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeSmoke::~CGrenadeSmoke( void )
{
}

void CGrenadeSmoke::Spawn( void )
{
	Precache( );

	SetModel( SMOKE_GRENADE_MODEL );

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + SMOKE_GRENADE_BLIP_FREQUENCY;

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_punted			= false;
	m_bStartSmoke		= false;

	this->AddEffects(EF_FRAGSLIGHT);

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSmoke::OnRestore( void )
{
	// If we were primed and ready to detonate, put FX on us.
	if (m_flDetonateTime > 0)
		CreateEffects();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSmoke::CreateEffects( void )
{
	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate( "sprites/blueglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, 0, 0, 255, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 0, 0, 255, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 0.5f );
	}
}

bool CGrenadeSmoke::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

// Light Kill : Copy from original grenade
// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CGrenadeSmoke::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
	{
		if ( !m_inSolid )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -SMOKE_GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= SMOKE_GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
}


void CGrenadeSmoke::Precache( void )
{
	PrecacheModel( SMOKE_GRENADE_MODEL );

	PrecacheScriptSound( "Grenade.Blip" );
	PrecacheScriptSound( SMOKE_GRENADE_DETONATE_SOUND );
	PrecacheScriptSound( SMOKE_GRENADE_BREAK_SOUND );

	PrecacheModel( "sprites/blueglow1.vmt" );
	PrecacheModel( "sprites/bluelaser1.vmt" );

	PrecacheParticleSystem ( oc_weapon_smokegrenade_effect.GetString() ); //( "he_smokegrenade" );

	BaseClass::Precache();
}

void CGrenadeSmoke::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeSmoke::DelayThink );
	SetNextThink( gpGlobals->curtime );

	CreateEffects();
}

void CGrenadeSmoke::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	SetThrower( pPhysGunUser );

#ifdef HL2MP
	SetTimer( SMOKE_GRENADE_GRACE_TIME_AFTER_PICKUP, SMOKE_GRENADE_GRACE_TIME_AFTER_PICKUP / 2);

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + SMOKE_GRENADE_BLIP_FAST_FREQUENCY;
	m_bHasWarnedAI = true;
#else
	if( IsX360() )
	{
		// Give 'em a couple of seconds to aim and throw. 
		SetTimer( 2.0f, 1.0f);
		BlipSound();
		m_flNextBlipTime = gpGlobals->curtime + SMOKE_GRENADE_BLIP_FAST_FREQUENCY;
	}
#endif

#ifdef HL2_EPISODIC
	SetPunted( true );
#endif

	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

//extern short	g_sModelIndexSmoke;	

void CGrenadeSmoke::SmokeDetonate()
{
	m_flDetonateTime = gpGlobals->curtime + 5.0f;

	/*Vector vecAbsOrigin = GetAbsOrigin();
	CPVSFilter filter( vecAbsOrigin );
	te->Smoke( filter, 0.0, 
		&vecAbsOrigin, g_sModelIndexSmoke,
		m_DmgRadius * 0.03,
		24 );*/

	m_bStartSmoke = true;

	EmitSound( SMOKE_GRENADE_DETONATE_SOUND );

	SetThink( &CGrenadeSmoke::SmokeThink );
	SetNextThink( gpGlobals->curtime );
}

void CGrenadeSmoke::SmokeThink()
{
	if ( gpGlobals->curtime > m_flDetonateTime )
	{
		StopSound( SMOKE_GRENADE_DETONATE_SOUND );
		EmitSound( SMOKE_GRENADE_BREAK_SOUND );
		UTIL_Remove( this );
	}

	CBaseEntity*	pList[100];
	Vector			delta( smoke_grenade_radius.GetInt(), smoke_grenade_radius.GetInt(), smoke_grenade_radius.GetInt() );

	int count = UTIL_EntitiesInBox( pList, 100, GetAbsOrigin() - delta, GetAbsOrigin() + delta, 0 );
	
	// If the bugbait's been thrown, look for nearby targets to affect

	for ( int i = 0; i < count; i++ )
	{
		// If close enough, make citizens freak out when hit
		if ( UTIL_DistApprox( pList[i]->WorldSpaceCenter(), GetAbsOrigin() ) < smoke_grenade_radius.GetInt() )
		{
			if ( pList[i]->IsNPC() )
			{
				pList[i]->TakeDamage( CTakeDamageInfo( this, GetThrower(), smoke_grenade_damage.GetInt(), DMG_PARALYZE ) );
			}

			// Must be a citizen
			/*if ( FClassnameIs( pList[i], "npc_citizen") )
			{
				CNPC_Citizen *pCitizen = assert_cast<CNPC_Citizen *>(pList[i]);

				if ( pCitizen != NULL )
				{
					//pCitizen->RunAwayFromSmoke( this ); //test
				}
			}*/
		}
	}

	//TERO: damage time

	//CheckTraceHullAttack(0, -delta, delta, smoke_grenade_damage.GetInt(), DMG_POISON, 0.0f, false );

	SetNextThink( gpGlobals->curtime + 0.5f );
}

void CGrenadeSmoke::DelayThink() 
{
	if( gpGlobals->curtime > m_flDetonateTime )
	{
		SmokeDetonate();
		return;
	}

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}
	
	if( gpGlobals->curtime > m_flNextBlipTime )
	{
		BlipSound();
		
		if( m_bHasWarnedAI )
		{
			m_flNextBlipTime = gpGlobals->curtime + SMOKE_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = gpGlobals->curtime + SMOKE_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeSmoke::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeSmoke::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

void CGrenadeSmoke::InputSetTimer( inputdata_t &inputdata )
{
	SetTimer( inputdata.value.Float(), inputdata.value.Float() - SMOKE_GRENADE_WARN_TIME );
}

CBaseGrenade *SmokeGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeSmoke *pGrenade = (CGrenadeSmoke *)CBaseEntity::Create( "npc_grenade_smoke", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - SMOKE_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

	return pGrenade;
}