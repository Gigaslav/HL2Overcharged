//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Gravity well device
//
//=====================================================================================//

#include "cbase.h"
#include "grenade_hopwire.h"
#include "rope.h"
#include "rope_shared.h"
#include "beam_shared.h"
#include "particle_parse.h"
#include "physics.h"
#include "physics_saverestore.h"
#include "explode.h"
#include "physics_prop_ragdoll.h"
#include "movevars_shared.h"
#include "IEffects.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar oc_weapon_hopwire_vortex_spheresize("oc_weapon_hopwire_vortex_spheresize", "8", FCVAR_ARCHIVE);
ConVar oc_weapon_hopwire_vortex_stepsize("oc_weapon_hopwire_vortex_stepsize", "0.1", FCVAR_ARCHIVE);
ConVar oc_weapon_hopwire_vortex_lifetime("oc_weapon_hopwire_vortex_lifetime", "3.0", FCVAR_ARCHIVE);
ConVar oc_weapon_hopwire_vortex("oc_weapon_hopwire_vortex", "1");
ConVar oc_weapon_hopwire_trap("oc_weapon_hopwire_trap", "1");
ConVar oc_weapon_hopwire_strider_kill_dist_h("oc_weapon_hopwire_strider_kill_dist_h", "300");
ConVar oc_weapon_hopwire_strider_kill_dist_v("oc_weapon_hopwire_strider_kill_dist_v", "256");
ConVar oc_weapon_hopwire_strider_hits("oc_weapon_hopwire_strider_hits", "1");
ConVar oc_weapon_hopwire_hopheight("oc_weapon_hopwire_hopheight", "400", FCVAR_ARCHIVE);
ConVar oc_weapon_hopwire_model("oc_weapon_hopwire_model", "models/weapons/w_grenade.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Set the model of hopwire weapon.");

ConVar g_debug_hopwire( "g_debug_hopwire", "0" );

#define	DENSE_BALL_MODEL	"models/props_junk/metal_paintcan001b.mdl"

#define	MAX_HOP_HEIGHT		(oc_weapon_hopwire_hopheight.GetFloat())		// Maximum amount the grenade will "hop" upwards when detonated

//extern ConVar oc_weapon_cguard_sphere_model;

#define VORTEX_MODEL "models/ballsphere.mdl"//oc_weapon_cguard_sphere_model.GetString()

class CGravityVortexController : public CBaseAnimating//CBaseEntity
{
	DECLARE_CLASS(CGravityVortexController, CBaseAnimating);
	DECLARE_DATADESC();

public:
	void	Precache(void);
	void	Spawn(void);
			CGravityVortexController( void ) : m_flEndTime( 0.0f ), m_flRadius( 256 ), m_flStrength( 256 ), m_flMass( 0.0f ) {}	
	float	GetConsumedMass( void ) const;

	static CGravityVortexController *Create( const Vector &origin, float radius, float strength, float duration );

private:

	void	ConsumeEntity( CBaseEntity *pEnt );
	void	PullPlayersInRange( void );
	bool	KillNPCInRange( CBaseEntity *pVictim, IPhysicsObject **pPhysObj );
	void	CreateDenseBall( void );
	void	PullThink( void );
	void	StartPull( const Vector &origin, float radius, float strength, float duration );

	float	m_flMass;		// Mass consumed by the vortex
	float	m_flEndTime;	// Time when the vortex will stop functioning
	float	m_flRadius;		// Area of effect for the vortex
	float	m_flStrength;	// Pulling strength of the vortex
};

void CGravityVortexController::Precache(void)
{
	PrecacheModel(VORTEX_MODEL);

	PrecacheParticleSystem("hopwire_vortex");

	BaseClass::Precache();
}

void CGravityVortexController::Spawn(void)
{
	Precache();

	SetModel(VORTEX_MODEL);

	//BaseClass::Spawn();

	DispatchParticleEffect("hopwire_vortex", PATTACH_ABSORIGIN_FOLLOW, this);

	float size = oc_weapon_hopwire_vortex_spheresize.GetFloat();

	size = Clamp(size, 0.1f, 50.f);

	SetModelScale(size);
}
//-----------------------------------------------------------------------------
// Purpose: Returns the amount of mass consumed by the vortex
//-----------------------------------------------------------------------------
float CGravityVortexController::GetConsumedMass( void ) const 
{
	return m_flMass;
}

//-----------------------------------------------------------------------------
// Purpose: Adds the entity's mass to the aggregate mass consumed
//-----------------------------------------------------------------------------
void CGravityVortexController::ConsumeEntity( CBaseEntity *pEnt )
{
	// Get our base physics object
	IPhysicsObject *pPhysObject = pEnt->VPhysicsGetObject();
	if ( pPhysObject == NULL )
		return;

	// Ragdolls need to report the sum of all their parts
	CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >( pEnt );
	if ( pRagdoll != NULL )
	{		
		// Find the aggregate mass of the whole ragdoll
		ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
		for ( int j = 0; j < pRagdollPhys->listCount; ++j )
		{
			m_flMass += pRagdollPhys->list[j].pObject->GetMass();
		}
	}
	else
	{
		// Otherwise we just take the normal mass
		m_flMass += pPhysObject->GetMass();
	}

	// Destroy the entity
	UTIL_Remove( pEnt );
}

//-----------------------------------------------------------------------------
// Purpose: Causes players within the radius to be sucked in
//-----------------------------------------------------------------------------
void CGravityVortexController::PullPlayersInRange( void )
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	
	Vector	vecForce = GetAbsOrigin() - pPlayer->GetAbsOrigin();//->WorldSpaceCenter();

	float	dist = VectorNormalize( vecForce );
	
	// FIXME: Need a more deterministic method here
	if ( dist < 128.0f )
	{
		// Kill the player (with falling death sound and effects)
		CTakeDamageInfo deathInfo( this, this, GetAbsOrigin(), GetAbsOrigin(), 200, DMG_FALL );
		deathInfo.SetDamageForce(vecForce);
		pPlayer->TakeDamage( deathInfo );
		
		if ( pPlayer->IsAlive() == false )
		{
			color32 black = { 0, 0, 0, 255 };
			UTIL_ScreenFade( pPlayer, black, 0.1f, 0.0f, (FFADE_OUT|FFADE_STAYOUT) );
			return;
		}
	}

	// Must be within the radius
	if ( dist > m_flRadius )
		return;

	float mass = pPlayer->VPhysicsGetObject()->GetMass();
	float playerForce = m_flStrength * 0.05f;

	// Find the pull force
	// NOTE: We might want to make this non-linear to give more of a "grace distance"
	vecForce *= ( 1.0f - ( dist / m_flRadius ) ) * playerForce * mass;
	vecForce[2] *= 0.025f;
	
	//pPlayer->SetBaseVelocity( vecForce );
	pPlayer->VelocityPunch(vecForce);
	//pPlayer->AddFlag( FL_BASEVELOCITY );
	
	// Make sure the player moves
	if ( vecForce.z > 0 && ( pPlayer->GetFlags() & FL_ONGROUND) )
	{
		pPlayer->SetGroundEntity( NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Attempts to kill an NPC if it's within range and other criteria
// Input  : *pVictim - NPC to assess
//			**pPhysObj - pointer to the ragdoll created if the NPC is killed
// Output :	bool - whether or not the NPC was killed and the returned pointer is valid
//-----------------------------------------------------------------------------
bool CGravityVortexController::KillNPCInRange( CBaseEntity *pVictim, IPhysicsObject **pPhysObj )
{
	CBaseCombatCharacter *pBCC = pVictim->MyCombatCharacterPointer();

	// See if we can ragdoll
	if ( pBCC != NULL && pBCC->CanBecomeRagdoll() )
	{
		if (!(cvar->FindVar("oc_ragdoll_serverside")->GetInt()))
		{
			if (FClassnameIs(pBCC, "npc_strider"))
				return false;
			// also if player
			if (FClassnameIs(pBCC, "player"))
				return false;

			CTakeDamageInfo info(this, this, 1.0f, DMG_GENERIC);
			CBaseEntity *pRagdoll = CreateServerRagdoll(pBCC, 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true, pBCC->BloodColor());
			pRagdoll->SetCollisionBounds(pVictim->CollisionProp()->OBBMins(), pVictim->CollisionProp()->OBBMaxs());

			CTakeDamageInfo ragdollInfo(this, this, 100000.0, DMG_GENERIC | DMG_REMOVENORAGDOLL);

			pVictim->TakeDamage(ragdollInfo);

			// Return the pointer to the ragdoll
			*pPhysObj = pRagdoll->VPhysicsGetObject();
			return true;
		}
		else
		{

			// Necessary to cause it to do the appropriate death cleanup
			CTakeDamageInfo ragdollInfo(this, this, 50.0, DMG_GENERIC);// , DMG_GENERIC | DMG_REMOVENORAGDOLL);//10000.0

			if (FClassnameIs(pBCC, "player"))
				ragdollInfo.SetDamageForce(Vector(0, 0, 0));
			else
				ragdollInfo.SetDamageForce(pVictim->GetAbsOrigin() + GetAbsOrigin());
			//ragdollInfo.ScaleDamageForce(-10.f);
			//pVictim->Event_Killed(ragdollInfo);
			pVictim->TakeDamage(ragdollInfo);

			return true;
		}
	}

	if (!(cvar->FindVar("oc_ragdoll_serverside")->GetInt()))
		*pPhysObj = NULL;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Creates a dense ball with a mass equal to the aggregate mass consumed by the vortex
//-----------------------------------------------------------------------------
void CGravityVortexController::CreateDenseBall( void )
{
	CBaseEntity *pBall = CreateEntityByName( "prop_physics" );
	
	pBall->SetModel( DENSE_BALL_MODEL );
	pBall->SetAbsOrigin( GetAbsOrigin() );
	pBall->Spawn();

	IPhysicsObject *pObj = pBall->VPhysicsGetObject();
	if ( pObj != NULL )
	{
		pObj->SetMass( GetConsumedMass() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Pulls physical objects towards the vortex center, killing them if they come too near
//-----------------------------------------------------------------------------
void CGravityVortexController::PullThink( void )
{
	float size = GetModelScale() - (oc_weapon_hopwire_vortex_stepsize.GetFloat() * gpGlobals->frametime);

	size = Clamp(size, 0.1f, 50.f);

	//DevMsg("size: %.2f \n",size);

	SetModelScale(size, 0.0f);
	
	// Pull any players close enough to us
	PullPlayersInRange();

	Vector mins, maxs;
	mins = GetAbsOrigin() - Vector( m_flRadius, m_flRadius, m_flRadius );
	maxs = GetAbsOrigin() + Vector( m_flRadius, m_flRadius, m_flRadius );

	// Draw debug information
	if ( g_debug_hopwire.GetBool() )
	{
		NDebugOverlay::Box( GetAbsOrigin(), mins - GetAbsOrigin(), maxs - GetAbsOrigin(), 0, 255, 0, 16, 4.0f );
	}
	
	CBaseEntity *pEnts[128];
	int numEnts = UTIL_EntitiesInBox( pEnts, 128, mins, maxs, 0 );

	for ( int i = 0; i < numEnts; i++ )
	{
		IPhysicsObject *pPhysObject = NULL;

		if (!(cvar->FindVar("oc_ragdoll_serverside")->GetInt()))
		{
			if (KillNPCInRange(pEnts[i], &pPhysObject) == false)
			{
				// If we didn't have a valid victim, see if we can just get the vphysics object
				pPhysObject = pEnts[i]->VPhysicsGetObject();
				if (pPhysObject == NULL)
					continue;
			}
		}
		else
		{
			if (pEnts[i] && (pEnts[i]->IsNPC() || pEnts[i]->IsPlayer()) && pEnts[i]->IsAlive())
			{
				KillNPCInRange(pEnts[i], &pPhysObject);
			}
			else
			{
				if (pEnts[i])
					pPhysObject = pEnts[i]->VPhysicsGetObject();
			}
		}

		if (pPhysObject == NULL)
			continue;


		float mass;

		CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >( pEnts[i] );
		if ( pRagdoll != NULL )
		{
			ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
			mass = 0.0f;
			
			// Find the aggregate mass of the whole ragdoll
			for ( int j = 0; j < pRagdollPhys->listCount; ++j )
			{
				mass += pRagdollPhys->list[j].pObject->GetMass();
			}
		}
		else
		{
			mass = pPhysObject->GetMass();
		}

		Vector	vecForce = GetAbsOrigin() - pEnts[i]->WorldSpaceCenter();
		Vector	vecForce2D = vecForce;
		vecForce2D[2] = 0.0f;
		float	dist2D = VectorNormalize( vecForce2D );
		float	dist = VectorNormalize( vecForce );
		
		// FIXME: Need a more deterministic method here
		if ( dist < 48.0f )
		{
			ConsumeEntity( pEnts[i] );
			continue;
		}

		// Must be within the radius
		if ( dist > m_flRadius )
			continue;

		// Find the pull force
		vecForce *= ( 1.0f - ( dist2D / m_flRadius ) ) * m_flStrength * mass;
		
		if ( pEnts[i]->VPhysicsGetObject() )
		{
			// Pull the object in
			pEnts[i]->VPhysicsTakeDamage( CTakeDamageInfo( this, this, vecForce, GetAbsOrigin(), m_flStrength, DMG_BLAST ) );
		}
	}

	// Keep going if need-be
	if ( m_flEndTime > gpGlobals->curtime )
	{
		SetThink( &CGravityVortexController::PullThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
	{
		StopSound("Weapon_HopWire.Vortex");

		StopParticleEffects(this);

		UTIL_Remove(this);

		//CreateDenseBall();
	}

	//SetModelScale(1.f);// *Clamp(m_flEndTime - gpGlobals->curtime, 0.f, 1.f));
}

//-----------------------------------------------------------------------------
// Purpose: Starts the vortex working
//-----------------------------------------------------------------------------
void CGravityVortexController::StartPull( const Vector &origin, float radius, float strength, float duration )
{
	SetAbsOrigin( origin );
	m_flEndTime	= gpGlobals->curtime + duration;
	m_flRadius	= radius;
	m_flStrength= strength;

	//PrecacheModel(VORTEX_MODEL);
	SetModel(VORTEX_MODEL);

	SetModelScale(oc_weapon_hopwire_vortex_spheresize.GetFloat(), 0.0f);

	Vector newOrigin = GetBaseAnimating()->GetLocalOrigin();
	newOrigin.z -= 30.f;
	GetBaseAnimating()->SetLocalOrigin(newOrigin);

	PrecacheScriptSound("Weapon_HopWire.Vortex");
	EmitSound("Weapon_HopWire.Vortex");

	SetThink( &CGravityVortexController::PullThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Creation utility
//-----------------------------------------------------------------------------
CGravityVortexController *CGravityVortexController::Create( const Vector &origin, float radius, float strength, float duration )
{
	// Create an instance of the vortex
	CGravityVortexController *pVortex = (CGravityVortexController *) CreateEntityByName( "vortex_controller" );
	if ( pVortex == NULL )
		return NULL;

	pVortex->Spawn();

	// Start the vortex working
	pVortex->StartPull( origin, radius, strength, duration );

	return pVortex;
}

BEGIN_DATADESC( CGravityVortexController )
	DEFINE_FIELD( m_flMass, FIELD_FLOAT ),
	DEFINE_FIELD( m_flEndTime, FIELD_TIME ),
	DEFINE_FIELD( m_flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_flStrength, FIELD_FLOAT ),

	DEFINE_THINKFUNC( PullThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( vortex_controller, CGravityVortexController );

#define HOPWIRE_MODEL "models/Weapons/w_hopwire.mdl"
//#define GRENADE_MODEL_CLOSED	aa_wpn_hopwire_model.GetString() //"models/weapons/w_hopwire.mdl"
//#define GRENADE_MODEL_OPEN		aa_wpn_hopwire_model.GetString() //"models/weapons/w_hopwire.mdl"

BEGIN_DATADESC( CGrenadeHopwire )
	DEFINE_FIELD( m_hVortexController, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( EndThink ),
	DEFINE_THINKFUNC( CombatThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_grenade_hopwire, CGrenadeHopwire );

IMPLEMENT_SERVERCLASS_ST( CGrenadeHopwire, DT_GrenadeHopwire )
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::Spawn( void )
{
	Precache();

	//SetModel( GRENADE_MODEL_CLOSED );
	SetModel(HOPWIRE_MODEL);

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	
	CreateVPhysics();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGrenadeHopwire::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::Precache( void )
{
	// FIXME: Replace
	PrecacheSound("NPC_Strider.Shoot"); // Light Kill : Why removed ? o_0
	PrecacheSound("d3_citadel.portal_open_rift"); // Light Kill : New sound
	//PrecacheSound("d3_citadel.weapon_zapper_beam_loop2");

	PrecacheModel(VORTEX_MODEL);

	PrecacheModel(HOPWIRE_MODEL);
	
	PrecacheModel( DENSE_BALL_MODEL );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : timer - 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::SetTimer( float timer )
{
	SetThink( &CBaseGrenade::PreDetonate );
	SetNextThink( gpGlobals->curtime + timer );
}

#define	MAX_STRIDER_KILL_DISTANCE_HORZ	(oc_weapon_hopwire_strider_kill_dist_h.GetFloat())		// Distance a Strider will be killed if within
#define	MAX_STRIDER_KILL_DISTANCE_VERT	(oc_weapon_hopwire_strider_kill_dist_v.GetFloat())		// Distance a Strider will be killed if within

#define MAX_STRIDER_STUN_DISTANCE_HORZ	(MAX_STRIDER_KILL_DISTANCE_HORZ*2)	// Distance a Strider will be stunned if within
#define MAX_STRIDER_STUN_DISTANCE_VERT	(MAX_STRIDER_KILL_DISTANCE_VERT*2)	// Distance a Strider will be stunned if within

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::KillStriders( void )
{
	CBaseEntity *pEnts[128];
	Vector	mins, maxs;

	ClearBounds( mins, maxs );
	AddPointToBounds( -Vector( MAX_STRIDER_STUN_DISTANCE_HORZ, MAX_STRIDER_STUN_DISTANCE_HORZ, MAX_STRIDER_STUN_DISTANCE_HORZ ), mins, maxs );
	AddPointToBounds(  Vector( MAX_STRIDER_STUN_DISTANCE_HORZ, MAX_STRIDER_STUN_DISTANCE_HORZ, MAX_STRIDER_STUN_DISTANCE_HORZ ), mins, maxs );
	AddPointToBounds( -Vector( MAX_STRIDER_STUN_DISTANCE_VERT, MAX_STRIDER_STUN_DISTANCE_VERT, MAX_STRIDER_STUN_DISTANCE_VERT ), mins, maxs );
	AddPointToBounds(  Vector( MAX_STRIDER_STUN_DISTANCE_VERT, MAX_STRIDER_STUN_DISTANCE_VERT, MAX_STRIDER_STUN_DISTANCE_VERT ), mins, maxs );

	// FIXME: It's probably much faster to simply iterate over the striders in the map, rather than any entity in the radius - jdw

	// Find any striders in range of us
	int numTargets = UTIL_EntitiesInBox( pEnts, ARRAYSIZE( pEnts ), GetAbsOrigin()+mins, GetAbsOrigin()+maxs, FL_NPC );
	float targetDistHorz, targetDistVert;

	for ( int i = 0; i < numTargets; i++ )
	{
		// Only affect striders
		if ( FClassnameIs( pEnts[i], "npc_strider" ) == false )
			continue;

		// We categorize our spatial relation to the strider in horizontal and vertical terms, so that we can specify both parameters separately
		targetDistHorz = UTIL_DistApprox2D( pEnts[i]->GetAbsOrigin(), GetAbsOrigin() );
		targetDistVert = fabs( pEnts[i]->GetAbsOrigin()[2] - GetAbsOrigin()[2] );

		if ( targetDistHorz < MAX_STRIDER_KILL_DISTANCE_HORZ && targetDistHorz < MAX_STRIDER_KILL_DISTANCE_VERT )
		{
			// Kill the strider
			float fracDamage = (pEnts[i]->GetMaxHealth() / oc_weapon_hopwire_strider_hits.GetFloat()) + 1.0f;
			CTakeDamageInfo killInfo( this, this, fracDamage, DMG_GENERIC );
			Vector	killDir = pEnts[i]->GetAbsOrigin() - GetAbsOrigin();
			VectorNormalize( killDir );

			killInfo.SetDamageForce( killDir * -1000.0f );
			killInfo.SetDamagePosition( GetAbsOrigin() );

			pEnts[i]->TakeDamage( killInfo );
		}
		else if ( targetDistHorz < MAX_STRIDER_STUN_DISTANCE_HORZ && targetDistHorz < MAX_STRIDER_STUN_DISTANCE_VERT )
		{
			// Stun the strider
			CTakeDamageInfo killInfo( this, this, 200.0f, DMG_GENERIC );
			pEnts[i]->TakeDamage( killInfo );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::EndThink( void )
{
	if (oc_weapon_hopwire_vortex.GetBool())
	{
		EntityMessageBegin( this, true );
			WRITE_BYTE( 1 );
		MessageEnd();
	}

	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::CombatThink( void )
{
	// Stop the grenade from moving
	AddEFlags( EF_NODRAW );
	AddFlag( FSOLID_NOT_SOLID );
	VPhysicsDestroyObject();
	SetAbsVelocity( vec3_origin );
	SetMoveType( MOVETYPE_NONE );

	// Do special behaviors if there are any striders in the area
	KillStriders();

	// FIXME: Replace
	EmitSound("NPC_Strider.Shoot");
	EmitSound("d3_citadel.portal_open_rift");
	//EmitSound("d3_citadel.weapon_zapper_beam_loop2");

	// Quick screen flash
	CBasePlayer *pPlayer = ToBasePlayer( GetThrower() );
	color32 white = { 255,255,255,255 };
	UTIL_ScreenFade( pPlayer, white, 0.2f, 0.0f, FFADE_IN );

	Vector Up = GetAbsOrigin();
	Up.z += 10.f;

	g_pEffects->Dust(GetAbsOrigin(), Up, 12, 50);
	UTIL_Smoke(GetAbsOrigin(), random->RandomInt(10, 15), 10);

	// Create the vortex controller to pull entities towards us
	if (oc_weapon_hopwire_vortex.GetBool())
	{
		m_hVortexController = CGravityVortexController::Create(GetAbsOrigin(), 512, 150, oc_weapon_hopwire_vortex_lifetime.GetFloat());

		// Start our client-side effect
		EntityMessageBegin( this, true );
			WRITE_BYTE( 0 );
		MessageEnd();
		
		// Begin to stop in two seconds
		SetThink( &CGrenadeHopwire::EndThink );
		SetNextThink( gpGlobals->curtime + 2.0f );
	}
	else
	{
		// Remove us immediately
		SetThink( &CBaseEntity::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopwire::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	
	if ( pPhysicsObject != NULL )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hop off the ground to start deployment
//-----------------------------------------------------------------------------
void CGrenadeHopwire::Detonate( void )
{
	AngularImpulse	hopAngle = RandomAngularImpulse( -300, 300 );

	//Find out how tall the ceiling is and always try to hop halfway
	//trace_t	tr;
	//UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, MAX_HOP_HEIGHT*2 ), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	// Jump half the height to the found ceiling
	float hopHeight = MAX_HOP_HEIGHT;//min( MAX_HOP_HEIGHT, (MAX_HOP_HEIGHT*tr.fraction) );

	//Add upwards velocity for the "hop"
	Vector hopVel( 0.0f, 0.0f, hopHeight );
	SetVelocity( hopVel, hopAngle );

	// Get the time until the apex of the hop
	float apexTime = sqrt( hopHeight / sv_gravity.GetFloat() );

	// Explode at the apex
	SetThink( &CGrenadeHopwire::CombatThink );
	SetNextThink( gpGlobals->curtime + apexTime);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseGrenade *HopWire_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	CGrenadeHopwire *pGrenade = (CGrenadeHopwire *) CBaseEntity::Create( "npc_grenade_hopwire", position, angles, pOwner );
	
	// Only set ourselves to detonate on a timer if we're not a trap hopwire
	if ( oc_weapon_hopwire_trap.GetBool() == false )
	{
		pGrenade->SetTimer( timer );
	}

	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );

	return pGrenade;
}
