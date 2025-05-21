//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	BEES BEING UNKNOWN TO ME
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "explode.h"
#include "gib.h"

#include "decals.h"

#include "ai_basenpc.h"
#include "ai_pathfinder.h"
#include "ai_node.h"
#include "ai_default.h"
#include "ai_navigator.h"
#include "ai_network.h"
#include "ai_networkmanager.h"

#include "gib.h"
#include "EntityFlame.h"

#include "smoke_trail.h"
#include "beam_shared.h"
#include "sprite.h"
#include "fire.h"
#include "te_effect_dispatch.h"

#include "particle_parse.h"
#include "particle_system.h"

#include "weapon_fireball.h"
#include "npc_aliencontroller.h"

#include "tier0/memdbgon.h"


#define	FIREBALL_SPEED random->RandomInt(1000, 1200)

static ConVar sk_fireball_damage("sk_fireball_damage", "10");


BEGIN_DATADESC( CFireBall )

	DEFINE_FIELD( m_hOwner,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_flDelayThink,			FIELD_TIME ),
	DEFINE_FIELD( m_flGracePeriodEndsAt,	FIELD_TIME ),
	DEFINE_FIELD( m_hRocketTrail,			FIELD_EHANDLE ),

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	DEFINE_FIELD( m_bHasNoClip,				FIELD_BOOLEAN ),
#endif

//	DEFINE_FIELD( m_nHaloSprite,			FIELD_INTEGER ),

	// Function Pointers
	DEFINE_FUNCTION( FireBallTouch ),
	DEFINE_FUNCTION( IgniteThink ),
	DEFINE_FUNCTION( FireThink ),
	DEFINE_FUNCTION( FlyThink ),


END_DATADESC()
LINK_ENTITY_TO_CLASS( fireball_missile, CFireBall );

CFireBall::CFireBall()
{
	m_hRocketTrail = NULL;
}

CFireBall::~CFireBall()
{
}

void CFireBall::Precache( void )
{
	PrecacheModel( "models/weapons/fireball.mdl" );

	PrecacheModel( "models/gibs/hgibs.mdl" );

	PrecacheScriptSound( "d1_town.FlameTrapIgnite" );
	PrecacheScriptSound( "NPC_Bee.Hit" );

	PrecacheScriptSound( "streetwar.fire_tiny_loop" );

	PrecacheParticleSystem( "controller_fireball_explode" );
}

void CFireBall::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	//AddSolidFlags( FSOLID_NOT_SOLID ); //| FSOLID_TRIGGER ); // 
	SetModel("models/weapons/fireball.mdl");

	SetRenderMode(kRenderTransColor );
	SetRenderColorA(0);

	UTIL_SetSize( this, -Vector(8,8,8), Vector(8,8,8) );

	SetTouch( &CFireBall::FireBallTouch );

	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE); //MOVECOLLIDE_FLY_CUSTOM 
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	SetThink( &CFireBall::IgniteThink );
	
	SetNextThink( gpGlobals->curtime );

	//m_takedamage = DAMAGE_YES;
	m_takedamage = DAMAGE_NO;
	m_iHealth = m_iMaxHealth = 3;
	m_bloodColor			= DONT_BLEED;

	m_flDelayThink = 0.1f;
	m_flGracePeriodEndsAt = 0;


	// Start our animation cycle. Use the random to avoid everything thinking the same frame
	//SetContextThink( &CPropCombineBall::AnimThink, gpGlobals->curtime + random->RandomFloat( 0.0f, 0.1f), s_pAnimThinkContext );


	AddFlag( FL_OBJECT );
}

unsigned int CFireBall::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

void CFireBall::Explode( void )
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	Vector forward; // = GetAbsVelocity();
	GetVectors(&forward, NULL, NULL);
	VectorNormalize(forward);

	//GetVectors( &forward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + (forward * 16), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_takedamage = DAMAGE_NO;
	SetSolid( SOLID_NONE );
	AddEffects( EF_NODRAW );

	bool bMadeFire = false; 

	if( tr.fraction > 0.5 || !(tr.surface.flags & SURF_SKY) )
	{
		bMadeFire = FireSystem_StartFire( tr.endpos, 64, 4, 6, SF_FIRE_START_ON | SF_FIRE_START_FULL | SF_FIRE_SMOKELESS, NULL, FIRE_NATURAL);

		float damage = sk_fireball_damage.GetFloat();
		if ( GetWaterLevel() != 0 )
			damage = damage * 0.5f;
		
		CTakeDamageInfo info( this, m_hOwner, damage, DMG_BURN );
		g_pGameRules->RadiusDamage( info, GetAbsOrigin(), 20, CLASS_ALIENCONTROLLER, NULL );
	}

	if (tr.m_pEnt && tr.m_pEnt->IsAlive() && !tr.m_pEnt->IsPlayer())	// L1ght 15 : Don't ignite player.
	{
		variant_t Variant;
		Variant.SetInt(2);
		tr.m_pEnt->AcceptInput("ignitelifetime", this, this, Variant, 0); //AcceptInput( ignitelifetime 2
	}

	if( m_hRocketTrail )
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	//we want to leave a scroch mark

	trace_t		pTrace;
	Vector vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &pTrace);
	UTIL_DecalTrace( &pTrace, "Scorch" );

	//TERO: re-enable later
	CreateFireBlast();

	if (bMadeFire)
	{
		AddEffects( EF_NODRAW );
		SetSolid( SOLID_NONE );
		SetMoveType( MOVETYPE_NONE );

		EmitSound("streetwar.fire_tiny_loop");
		SetThink( &CFireBall::FireThink );
		SetNextThink( gpGlobals->curtime + 6.0f );
	}
	else
	{
		UTIL_Remove( this );
	}
}

void CFireBall::FireThink( void )
{
	StopSound("streetwar.fire_tiny_loop");
	UTIL_Remove( this );
}

void CFireBall::FireBallTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	//DevMsg("pOther classname: %s\n", pOther->GetClassname());

	if ( !pOther->IsSolid() || pOther == m_hOwner )
		return;
	
	// Don't touch triggers (but DO hit weapons)
	if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) && 
		pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON &&
		pOther->GetCollisionGroup() != COLLISION_GROUP_VEHICLE_CLIP )
		return;

	if ( pOther->Classify() == CLASS_BEE || pOther->Classify() == CLASS_ALIENCONTROLLER )
		return;

	trace_t tr;
	tr = CBaseEntity::GetTouchTrace();

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	

	if ( pdata != NULL )
	{
		if ( pdata->game.material == CHAR_TEX_GRATE && pdata->physics.thickness <= 0.5 )
		{
			m_bHasNoClip = true;
			SetMoveType( MOVETYPE_NOCLIP );
		//	SteerToTarget(2);
			return;
		}
	}
#endif

	Explode();
}

void CFireBall::IgniteThink( void )
{
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE ); //MOVECOLLIDE_FLY_CUSTOM 

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	m_bHasNoClip = false;
#endif

	//SetModel("models/weapons/bee.mdl");
	//UTIL_SetSize( this, vec3_origin, vec3_origin );

	//TODO: Play opening sound

	Vector vecForward;

	EmitSound( "d1_town.FlameTrapIgnite" );

	/*if (m_hTarget && m_hTarget->Classify() == CLASS_MANHACK)
	{
		SetAbsVelocity( Vector(0,0,-500) );
	}
	else
	{*/
		AngleVectors( GetAbsAngles(), &vecForward );
		SetAbsVelocity( vecForward * FIREBALL_SPEED );
	//}
 
	SetThink( &CFireBall::FlyThink );
	SetNextThink( gpGlobals->curtime );

	CreateFireTrail();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFireBall::CreateFireTrail( void )
{
	if ( m_hRocketTrail )
		return;

	// Smoke trail.
	if ( (m_hRocketTrail = CFireTrail::CreateFireTrail()) != NULL ) //RocketTrail::CreateRocketTrail
	{	
		m_hRocketTrail->SetLifetime( 666 );
		m_hRocketTrail->FollowEntity( this, "0" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFireBall::CreateFireBlast( void )
{
	DispatchParticleEffect( "controller_fireball_explode", GetAbsOrigin(), RandomAngle( 0, 360 ) );

	//TERO: COMMENTED OUT BECAUSE THE FIRE DOESN'T KNOW HOW TO GO OUT!
	/*CEffectData	data;
	Vector forward;

	
	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = GetAbsVelocity();
	VectorNormalize( data.m_vNormal );
	data.m_flScale = 1;

	DispatchEffect( "FireBallChunk", data );*/


	//Vector vecBallVelocity = GetAbsVelocity();
	//VectorNormalize(vecBallVelocity);

#define NUMBER_OF_FIRE_CHUNKS 3
	/*for (int i = 0; i< NUMBER_OF_FIRE_CHUNKS; i++)
	{
		CGib *pChunk = CREATE_ENTITY( CGib, "gib" );

		if (!pChunk)
		{
			DevMsg("weapon_fireball: failed to create fire gibs\n");
			break;
		}

		pChunk->Spawn( "models/gibs/hgibs.mdl" );
		pChunk->SetBloodColor( DONT_BLEED );
		//pChunk->AddEffects( EF_NODRAW );

		QAngle vecSpawnAngles;
		vecSpawnAngles.Random( -90, 90 );
		pChunk->SetAbsOrigin( GetAbsOrigin() );
		pChunk->SetAbsAngles( GetAbsAngles() );

		pChunk->SetOwnerEntity( this ); //used to say "this" and after that m_hOwner, both might have been releated to crashes
		pChunk->m_lifeTime = random->RandomFloat( 1.0f, 2.0f );
		pChunk->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
		IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal( SOLID_VPHYSICS, pChunk->GetSolidFlags(), false );
	
		// Set the velocity
		if ( pPhysicsObject )
		{
			pPhysicsObject->EnableMotion( true );
			Vector vecVelocity;

			QAngle angles;
			angles.x = random->RandomFloat( -40, 0 );
			angles.y = random->RandomFloat( 0, 360 );
			angles.z = 0.0f;
			AngleVectors( angles, &vecVelocity );
		
			vecVelocity *= random->RandomFloat( 300, 600 );
			vecVelocity -= vecBallVelocity;

			AngularImpulse angImpulse;
			angImpulse = RandomAngularImpulse( -180, 180 );

			pChunk->SetAbsVelocity( vecVelocity );
			pPhysicsObject->SetVelocity(&vecVelocity, &angImpulse );
		}

		CEntityFlame *pFlame = CEntityFlame::Create( pChunk, false );
		if ( pFlame != NULL )
		{
			pFlame->SetLifetime( pChunk->m_lifeTime );
		}

		pChunk = NULL;
	}*/
}

Vector CFireBall::GetShootEnemyDir( const Vector &shootOrigin, bool bNoisy )
{
	CBaseEntity *pEnemy = m_hTarget;

	if ( pEnemy )
	{
		return (pEnemy->BodyTarget(shootOrigin, bNoisy)-shootOrigin);
	}
	else
	{
		Vector forward;
		AngleVectors( GetLocalAngles(), &forward );
		return forward;
	}
}

void CFireBall::SteerToTarget(float flHomingSpeed)
{
	if (m_hTarget != NULL) 
	{
		//TERO: Here we should check if we can go straight to the enemy

		Vector vTargetDir= GetShootEnemyDir( GetLocalOrigin() );

		float flDist = VectorNormalize( vTargetDir );

		Vector	vDir	= GetAbsVelocity();
		float	flSpeed	= VectorNormalize( vDir );
		Vector	vNewVelocity = vDir;

		if ( flSpeed < 300.0f )
		{	
			DevMsg("weapon_fireball, speed: %f\n", flSpeed);
			Explode();
		}
		else if ( flSpeed != 0 )
		{
			flHomingSpeed = flHomingSpeed * (flDist/1024);

			CBaseEntity *pEnemy = m_hTarget;
			if (pEnemy && pEnemy->Classify() == CLASS_MANHACK)
				flHomingSpeed *= 2.5;

			vNewVelocity = ( flHomingSpeed * vTargetDir ) +  vDir;

			// This computation may happen to cancel itself out exactly. If so, slam to targetdir.
			if ( VectorNormalize( vNewVelocity ) < 1e-3 )
			{
					vNewVelocity = (flDist != 0) ? vTargetDir : vDir;
			}
		}
		else
		{
			vNewVelocity = vTargetDir;
		}
	
		QAngle	finalAngles;
		VectorAngles( vNewVelocity, finalAngles );
		SetAbsAngles( finalAngles );

		vNewVelocity *= flSpeed;

		SetAbsVelocity( vNewVelocity );
	}
}

void CFireBall::SetGracePeriod( float flGracePeriod )
{
	m_flGracePeriodEndsAt = gpGlobals->curtime + flGracePeriod;

	// Go non-solid until the grace period ends
	AddSolidFlags( FSOLID_NOT_SOLID );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFireBall::FlyThink( void )
{
	if( GetAbsVelocity() == vec3_origin )
	{
		// Strange circumstances have brought this missile to halt. Just blow it up.
		Explode();
		return;
	}

	// If we have a grace period, go solid when it ends
	if ( m_flGracePeriodEndsAt )
	{
		if ( m_flGracePeriodEndsAt < gpGlobals->curtime )
		{
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			m_flGracePeriodEndsAt = 0;
		}
	}

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	// If we have a grace period, go solid when it ends
	if ( m_bHasNoClip )
	{
		SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE ); //MOVECOLLIDE_FLY_CUSTOM 
		m_bHasNoClip = false;
	}
#endif

	if ( m_flDelayThink < gpGlobals->curtime) 
	{
		 m_flDelayThink = gpGlobals->curtime + 0.1f;

		 //TERO: maybe a bit too smart for a fireball
		/*	if (m_hTarget == NULL)
		{
			CNPC_AlienController *pAController = dynamic_cast<CNPC_AlienController*>((CBaseEntity*)m_hOwner);
			if (pAController)
				m_hTarget = pAController->GetEnemy();
		}*/

		SteerToTarget(1.0f);
	}

	// Think as soon as possible
	SetNextThink( gpGlobals->curtime );

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.5, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	

	if ( pdata != NULL )
	{
		if ( pdata->game.material == CHAR_TEX_GRATE && pdata->physics.thickness <= 0.5 )
		{
			m_bHasNoClip = true;
			SetMoveType( MOVETYPE_NOCLIP );
			//DevMsg("Fireball: woo woo, we'are going a frigging fence or sumthing\n");
			return;
		}
	}
#endif

//TERO: commented this out for now
//	CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, 100, 0.2, m_hOwner, SOUNDENT_CHANNEL_REPEATED_DANGER );
}

CFireBall *CFireBall::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner)
{
	CFireBall *pFireBall = (CFireBall*) CBaseEntity::Create( "fireball_missile", vecOrigin, vecAngles, pOwner );
	//pFireBall->SetOwnerEntity( Instance( pentOwner ) );
	pFireBall->Spawn();
	pFireBall->AddEffects( EF_NOSHADOW );

	pFireBall->m_hOwner = pOwner;

	return pFireBall;
}
