#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_flash.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include "particle_smokegrenade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FLASH_GRENADE_BLIP_FREQUENCY 1.0f
#define FLASH_GRENADE_BLIP_FAST_FREQUENCY 0.3f

#define FLASH_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

//ConVar sk_flashgrenade_radius ( "sk_flashgrenade_radius", "0");
ConVar oc_weapon_flashgrenade_model( "oc_weapon_flashgrenade_model", "models/weapons/w_grenade.mdl", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Sets the model of flash grenade." );

#define FLASHGRENADE_MODEL oc_weapon_flashgrenade_model.GetString() //"models/Weapons/w_grenade.mdl"

class CGrenadeFlash : public CBaseGrenade
{
DECLARE_CLASS( CGrenadeFlash, CBaseGrenade );

#if !defined( CLIENT_DLL )
DECLARE_DATADESC();
#endif

~CGrenadeFlash( void );

public:
void Spawn( void );
void OnRestore( void );
void Precache( void );
bool CreateVPhysics( void );
void CreateEffects( void );
void SetTimer( float detonateDelay, float warnDelay );
void SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
int OnTakeDamage( const CTakeDamageInfo &inputInfo );
void BlipSound() { EmitSound("Flashbang.Explode");  CPASFilter filter(GetAbsOrigin());	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 0, 255, 0, 6.5, 35, 0.1, 1.1); }
void DelayThink();
void CreateSmokeTrail( void );
void VPhysicsUpdate( IPhysicsObject *pPhysics );
void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
protected:
//CHandle m_pMainGlow;
//CHandle m_pGlowTrail;
CHandle<CSprite>		m_pMainGlow;
CHandle<CSpriteTrail>	m_pGlowTrail;

float m_flNextBlipTime;
bool m_inSolid;
};

LINK_ENTITY_TO_CLASS( npc_grenade_flash, CGrenadeFlash );

BEGIN_DATADESC( CGrenadeFlash )

// Fields
DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),

// Function Pointers
DEFINE_THINKFUNC( DelayThink ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeFlash::~CGrenadeFlash( void )
{
}


void RadiusFlash( Vector vecSrc, CBaseEntity *pevInflictor, CBaseEntity *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity *pEntity = NULL;
	trace_t tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;
	float flRadius = 1500;


	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
	pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ((pEntity = gEntList.FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		// get the heck out of here if it aint a player.
		if (pEntity->IsPlayer() == FALSE)
		continue;

		if (( pEntity->m_takedamage != DAMAGE_NO ) && (pEntity->m_lifeState != LIFE_DEAD))
		{
			// blast's don't tavel into or out of water
			if (bInWater && pEntity->GetWaterLevel() == 0)
			continue;
			if (!bInWater && pEntity->GetWaterLevel() == 3)
			continue;

			vecSpot = pEntity->BodyTarget( vecSrc );

			UTIL_TraceLine( vecSrc, vecSpot, MASK_SHOT, pevInflictor, COLLISION_GROUP_NONE, &tr );

			if ( tr.fraction == 1.0 || tr.m_pEnt == pEntity )
			{
				// the explosion can 'see' this entity, so hurt them!
				if (tr.startsolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.endpos = vecSrc;
					tr.fraction = 0.0;
				}

				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.endpos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;

				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}

				Vector vecLOS;
				float flDot;
				Vector vForward;
				AngleVectors( pEntity->EyeAngles(), &vForward );

				vecLOS = ( vecSrc - (pEntity->EyePosition()));

				flDot = DotProduct (vecLOS, vForward);


				float fadeTime, fadeHold;
				int alpha;

				// if target is facing the bomb, the effect lasts longer
				if (flDot >= 0.0)
				{
					fadeTime = flAdjustedDamage * 3.0f;
					fadeHold = flAdjustedDamage / 1.5f;
					alpha = 255;
				}
				else
				{
					fadeTime = flAdjustedDamage * 1.75f;
					fadeHold = flAdjustedDamage / 3.5f;
					alpha = 200;
				}

				color32 clr = { 255, 255, 255, 255 };
				UTIL_ScreenFade( pEntity, clr, fadeTime, fadeHold, FFADE_IN );
			}
		}
	}
}


void CGrenadeFlash::Spawn( void )
{
Precache( );

SetModel( FLASHGRENADE_MODEL );

/*if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
{
//m_flDamage = sk_plr_dmg_fraggrenade.GetFloat();
m_DmgRadius = 10; //sk_flashgrenade_radius.GetFloat();
}
else
{
//m_flDamage = sk_npc_dmg_fraggrenade.GetFloat();
m_DmgRadius = 10; //sk_flashgrenade_radius.GetFloat();
}*/

m_DmgRadius = 10;

m_takedamage = DAMAGE_NO;	// was YES
m_iHealth = 1;

SetSize( -Vector(4,4,4), Vector(4,4,4) );
SetCollisionGroup( COLLISION_GROUP_WEAPON );
CreateVPhysics();

CreateEffects();

//BlipSound();
m_flNextBlipTime = gpGlobals->curtime + FLASH_GRENADE_BLIP_FREQUENCY;

AddSolidFlags( FSOLID_NOT_STANDABLE );

this->AddEffects(EF_FRAGFLIGHT);

BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFlash::OnRestore( void )
{
CreateEffects();

BaseClass::OnRestore();
} 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFlash::CreateEffects( void )
{
	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate( "sprites/greenglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, 255, 228, 0, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 228, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 0.5f );
	}
}

bool CGrenadeFlash::CreateVPhysics()
{
// Create the object in the physics system
VPhysicsInitNormal( SOLID_BBOX, 0, false );
return true;
}

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
int m_collisionGroupAlreadyChecked;
int m_newCollisionGroup;
};

void CGrenadeFlash::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
BaseClass::VPhysicsUpdate( pPhysics );
Vector vel;
AngularImpulse angVel;
pPhysics->GetVelocity( &vel, &angVel );

Vector start = GetAbsOrigin();
// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
trace_t tr;

// UNDONE: Hull won't work with hitboxes - hits outer hull. But the whole point of this test is to hit hitboxes.
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
vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
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
vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
angVel *= -0.5f;
pPhysics->SetVelocity( &vel, &angVel );
}
}


void CGrenadeFlash::Precache( void )
{
PrecacheModel( FLASHGRENADE_MODEL );
//PrecacheScriptSound( "Grenade.Blip" );

PrecacheModel( "sprites/greenglow1.vmt" );
PrecacheModel( "sprites/bluelaser1.vmt" );

PrecacheScriptSound( "Flashbang.Explode" ); // Light Kill : Flash grenade explosion sound
BaseClass::Precache();
}

void CGrenadeFlash::SetTimer( float detonateDelay, float warnDelay )
{
m_flDetonateTime = gpGlobals->curtime + detonateDelay;
m_flWarnAITime = gpGlobals->curtime + warnDelay;
SetThink( &CGrenadeFlash::DelayThink );
SetNextThink( gpGlobals->curtime );
}

void CGrenadeFlash::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
#ifdef HL2MP
SetTimer( FLASH_GRENADE_GRACE_TIME_AFTER_PICKUP, FLASH_GRENADE_GRACE_TIME_AFTER_PICKUP / 2);
SetThrower( pPhysGunUser );

BlipSound();
m_flNextBlipTime = gpGlobals->curtime + FLASH_GRENADE_BLIP_FAST_FREQUENCY;
m_bHasWarnedAI = true;
#endif

BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

void CGrenadeFlash::DelayThink() 
{
RadiusFlash ( GetAbsOrigin(), this, false, 4, CLASS_NONE, DMG_BLAST );

if( gpGlobals->curtime > m_flDetonateTime )
{
//AddEffects( EF_NODRAW );
	// Light Kill : Oh dammit remove this crap after flash effect appears
	Remove();
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
m_flNextBlipTime = gpGlobals->curtime + FLASH_GRENADE_BLIP_FAST_FREQUENCY;
}
else
{
m_flNextBlipTime = gpGlobals->curtime + FLASH_GRENADE_BLIP_FREQUENCY;
}
}

SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeFlash::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
if ( pPhysicsObject )
{
pPhysicsObject->AddVelocity( &velocity, &angVelocity );
}
}


int CGrenadeFlash::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
VPhysicsTakeDamage( inputInfo );

// Grenades only suffer blast damage and burn damage.
if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
return 0;

return BaseClass::OnTakeDamage( inputInfo );
}


CBaseGrenade *Flashgrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, float timer )
{
CGrenadeFlash *pGrenade = (CGrenadeFlash *)CBaseEntity::Create( "npc_grenade_flash", position, angles, pOwner );

// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
// one second before detonation.
pGrenade->SetTimer( timer, timer - 1.5f );
pGrenade->SetVelocity( velocity, angVelocity );
pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

pGrenade->SetThink( &CGrenadeFlash::DelayThink );
pGrenade->SetNextThink( gpGlobals->curtime + timer );

return pGrenade;
}