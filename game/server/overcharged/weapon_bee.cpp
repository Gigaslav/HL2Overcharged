//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	BEES BEING UNKNOWN TO ME
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "explode.h"

#include "ai_pathfinder.h"
#include "ai_node.h"
#include "ai_default.h"
#include "ai_navigator.h"
#include "ai_network.h"
#include "ai_networkmanager.h"

#include "gib.h"
#include "decals.h"
#include "te_effect_dispatch.h"

#include "weapon_bee.h"
#include "beam_shared.h"

#include "SpriteTrail.h"

#include "npc_aliengrunt.h"

#include "tier0/memdbgon.h"

#define	BEE_SPEED_LOW 400
#define BEE_SPEED_DIE 200
#define BEE_SPEED_HIGH	500
#define BEE_SPEED random->RandomInt( BEE_SPEED_LOW, BEE_SPEED_HIGH )
#define BEE_FLY_Y_OFFSET random->RandomFloat( 0.5, 3 ) 

#define BEE_CORPSE_FADETIME 1.0f

#define BEE_FLYBEAM_SPRITE "sprites/bluelaser1.vmt" //"sprites/laserbeam.vmt"

char *BeeFlyAnimations[3] = 
{
	"fly1",
	"fly2",
	"fly3",
};

/*char *BeeIdleAnimations[3] = 
{
	"idle1",
	"idle2",
	"idle3",
};

char *BeeHiveSpots[3] = 
{
	"bee1",
	"bee2",
	"bee3",
};*/

#define BEE_FLY_ANIMATION (BeeFlyAnimations[random->RandomInt(0,2)]) 

#define BEE_IDLE_ANIMATION (BeeIdleAnimations[m_iReturnSpot])

#define BEE_HIVE_SPOT (BeeHiveSpots[m_iReturnSpot])

static ConVar sk_bee_damage("sk_bee_damage", "4");
static ConVar sk_bee_thinkdelay("sk_bee_thinkdelay", "0.4");
static ConVar sk_bee_homingspeed("sk_bee_homingspeed", "2");
static ConVar bee_gib("oc_bee_gib","1");
static ConVar bee_debug("oc_bee_debug","0");

static ConVar bee_fly_towards_enemy_time("oc_bee_fly_towards_enemy_time", "1.4");
static ConVar bee_fly_towards_random_time("oc_bee_fly_towards_random_time", "1.0");

#define BEE_FLY_TOWARDS_PLAYER_TIME bee_fly_towards_enemy_time.GetFloat()
#define BEE_FLY_TOWARDS_RANDOM_TIME bee_fly_towards_random_time.GetFloat()
#define BEE_FLY_RANDOM_MAX random->RandomFloat( -3.0f, 3.0f )

#define	BEE_HOMING_SPEED	sk_bee_homingspeed.GetFloat()

BEGIN_DATADESC( CBee )

	DEFINE_FIELD( m_hOwner,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_flBuzzTime,				FIELD_TIME ),
	DEFINE_FIELD( m_flDelayThink,			FIELD_TIME ),
	DEFINE_FIELD( m_iBeeFlyOffset,			FIELD_INTEGER ),
	DEFINE_FIELD( m_bHasNoClip,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flGracePeriodEndsAt,	FIELD_TIME ),

#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
	DEFINE_FIELD( m_flNextChangeDirection,	FIELD_TIME ),
	DEFINE_FIELD( m_bDirectionToEnemy,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_angRandomDir,			FIELD_VECTOR ),
#endif

#ifndef BEE_USE_NPC_BASE
	DEFINE_FIELD( m_vecLastTargetPosition,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_hEnemy,					FIELD_EHANDLE ),
#endif

	DEFINE_FIELD( m_pTrail,	FIELD_CLASSPTR ),
	
	// Function Pointers
	DEFINE_ENTITYFUNC( BeeTouch ),
	DEFINE_THINKFUNC( IgniteThink ),
	DEFINE_THINKFUNC( SeekThink ),


END_DATADESC()
LINK_ENTITY_TO_CLASS( bee_missile, CBee );

CBee::CBee()
{
#ifndef BEE_USE_NPC_BASE
	m_hEnemy = NULL;
#endif

	m_pTrail = NULL;
}

CBee::~CBee()
{

}

void CBee::Precache( void )
{
	PrecacheModel( "models/weapons/bee.mdl" );
	PrecacheScriptSound( "NPC_Bee.Buzz" );
	PrecacheScriptSound( "NPC_Bee.Hit" );

	PrecacheModel( BEE_FLYBEAM_SPRITE );
}

void CBee::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE ); //| FSOLID_TRIGGER );
	SetModel("models/weapons/bee.mdl");


	UTIL_SetSize( this, -Vector(2,2,2), Vector(2,2,2) );

	SetTouch( &CBee::BeeTouch );

	SetHullType(HULL_TINY_CENTERED);

#ifdef BEE_USE_NPC_BASE
	SetHullSizeSmall();

	SetNavType( NAV_FLY );

	CapabilitiesClear();
	CapabilitiesAdd ( bits_CAP_MOVE_FLY | bits_CAP_SKIP_NAV_GROUND_CHECK );
	CapabilitiesAdd ( bits_CAP_FRIENDLY_DMG_IMMUNE );
#endif

	SetMoveType( MOVETYPE_FLY ); //, MOVECOLLIDE_FLY_BOUNCE 
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	SetThink( &CBee::IgniteThink );
	
	AddFlag( FL_FLY );
	
	SetNextThink( gpGlobals->curtime + 0.3f );

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 3;
	m_bloodColor			= DONT_BLEED;
	 m_flDelayThink			= 0;

	m_iBeeFlyOffset		= (BEE_FLY_Y_OFFSET);

	LeaveTrail();

	//BaseClass::Spawn();

#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
	m_flNextChangeDirection = gpGlobals->curtime + (BEE_FLY_TOWARDS_PLAYER_TIME * 2.0f);
	m_bDirectionToEnemy = true;
#endif

	int nSequence = LookupSequence( BEE_FLY_ANIMATION );


	SetCycle( 0 );
	m_flAnimTime = gpGlobals->curtime;
	SetAnimatedEveryTick( true );
	ResetSequence( nSequence );
	ResetClientsideFrame();

	m_flGracePeriodEndsAt = 0;

	//NPCInit();
	//CreateVPhysics()
}

void CBee::SetGracePeriod( float flGracePeriod )
{
	m_flGracePeriodEndsAt = gpGlobals->curtime + flGracePeriod;

	// Go non-solid until the grace period ends
	AddSolidFlags( FSOLID_NOT_SOLID );
}

unsigned int CBee::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

void CBee::Explode( void )
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	/*Vector forward;

	

	GetVectors( &forward, NULL, NULL );

	trace_t tr;
	/UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	if( tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY) )
	{

		SetCollisionGroup( COLLISION_GROUP_NONE );
	
		CBaseEntity *pHurt = CheckTraceHullAttack(6, Vector(-4,-4,-4), Vector(4,4,4), 4 + sk_bee_damage.GetFloat(), DMG_POISON );
		if ( pHurt )
		{
			//pHurt->ViewPunch( QAngle(5,0,-18) );
			// Play a random attack hit sound
		}
	//}*/

	CTakeDamageInfo info(this, this, sk_bee_damage.GetFloat(), DMG_POISON ); //DMG_POISON );	// L1ght 15 : Poison is so slow vieweffect, so i had to replace
	RadiusDamage( info, GetAbsOrigin(), 6, CLASS_BEE, m_hOwner );

	m_takedamage = DAMAGE_NO;
	SetSolid( SOLID_NONE );

	EmitSound( "NPC_Bee.Hit" );

	if (bee_gib.GetBool())
	{
		CEffectData	data;
		Vector forward;

	
		data.m_vOrigin = GetAbsOrigin();
		data.m_vNormal = GetAbsVelocity();
		VectorNormalize( data.m_vNormal );
		data.m_flScale = 1;

		DispatchEffect( "BeeCorpse", data );
	}
	UTIL_BloodDrips( GetAbsOrigin(), GetAbsVelocity(), BLOOD_COLOR_YELLOW, 1 );

	if (m_pTrail)
		m_pTrail->SetLifeTime( 0.6f );

	StopSound( "NPC_Bee.Buzz" );

	RemoveDeferred();
}

void CBee::BeeTouch( CBaseEntity *pOther )
{
	Assert( pOther );

	if ( !pOther->IsSolid() || pOther == m_hOwner )
		return;
	
	// Don't touch triggers (but DO hit weapons)
	if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
		return;

	if ( pOther->Classify() == CLASS_BEE || pOther->Classify() == CLASS_ALIENGRUNT )
		return;

#ifdef BEE_USE_NOCLIP_ON_CHAINLINKS
	trace_t tr;
	tr = CBaseEntity::GetTouchTrace();

	const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	

	if ( pdata != NULL )
	{
		if ( pdata->game.material == CHAR_TEX_GRATE && pdata->physics.thickness <= 0.5 )
		{
			//DevMsg("Angry bee: woo woo, we hit a frigging fence or sumthing\n");
			m_bHasNoClip = true;
			SetMoveType( MOVETYPE_NOCLIP );
			return;
		}
	}
#endif

	Explode();
}

void CBee::IgniteThink( void )
{
	SetMoveType( MOVETYPE_FLY );

#ifdef BEE_USE_NOCLIP_ON_CHAINLINKS
	m_bHasNoClip = false;
#endif

	//TODO: Play opening sound
	EmitSound( "NPC_Bee.Buzz" );
	m_flBuzzTime = gpGlobals->curtime;

	Vector vecForward;

	AngleVectors( GetLocalAngles(), &vecForward );
	SetAbsVelocity( vecForward * BEE_SPEED );
 

	SetThink( &CBee::SeekThink );
	SetNextThink( gpGlobals->curtime );
}

#ifndef BEE_USE_NPC_BASE
void CBee::SetEnemy(CBaseEntity *pEnemy)
{
	if (pEnemy)
	{
		m_vecLastTargetPosition = pEnemy->WorldSpaceCenter();
	}

	m_hEnemy = pEnemy;
}
#endif

Vector CBee::GetShootEnemyDir( const Vector &shootOrigin, bool bNoisy )
{
	//CBaseEntity *pEnemy = m_hTarget;

#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
	if ( m_bDirectionToEnemy )
#endif

	{
		if ( GetEnemy() )
		{

			Vector vTargetDir;

			//TERO: lets do a trace to see if we can get to enemy straight
			trace_t tr;
			UTIL_TraceLine( GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), MASK_ALL,  this, COLLISION_GROUP_NPC, &tr);

			//TERO: lets see if we are going to hit a frigging fence
			const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	
			if ( pdata != NULL )
			{
				if ( pdata->game.material == CHAR_TEX_GRATE && pdata->physics.thickness <= 0.5 )
				{
					m_bHasNoClip = true;
					SetMoveType( MOVETYPE_NOCLIP );
				}
			}

			if ( tr.fraction != 1.0 && tr.m_pEnt != GetEnemy() && !m_bHasNoClip &&
				 (tr.m_pEnt == NULL || tr.m_pEnt->Classify() != CLASS_ALIENGRUNT)) //&& m_bWorkingPathFinding
			{
#ifndef BEE_USE_NPC_BASE

				vTargetDir = m_vecLastTargetPosition - shootOrigin;

				if ( vTargetDir.Length() < 64.0f)
				{
					m_vecLastTargetPosition = GetEnemy()->GetLocalOrigin();
					vTargetDir = m_vecLastTargetPosition - shootOrigin;
				}


#else

				if (GetNavigator()->IsGoalActive())
				{	
					vTargetDir = GetNavigator()->GetCurWaypointPos() - shootOrigin;

					if (!GetNavigator()->CurWaypointIsGoal())
					{
						AI_ProgressFlyPathParams_t params (MASK_NPCSOLID|CONTENTS_WATER);

						params.strictPointTolerance = 8;
						params.bTrySimplify			= false;

						if (GetNavigator()->ProgressFlyPath( params ) == AINPP_COMPLETE)
						{
							GetNavigator()->ClearGoal();
						}
					}
					else
					{
						GetNavigator()->ClearGoal();
					}
				} 
				else
				{
					vTargetDir = (GetEnemy()->BodyTarget(shootOrigin, bNoisy)-shootOrigin);
					if (!GetNavigator()->SetGoal( GetEnemy()->WorldSpaceCenter() ))
					{
	
					}
				}
#endif

			}//end found something with the trace
			else
			{
				vTargetDir = (GetEnemy()->BodyTarget(shootOrigin, bNoisy)-shootOrigin);

#ifdef BEE_USE_NPC_BASE
				if (GetNavigator()->IsGoalActive())
				{
					GetNavigator()->ClearGoal();
				}
#else
				m_vecLastTargetPosition = GetEnemy()->GetLocalOrigin();
#endif
			}

			//TERO: should we start going to random?
#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
			if (m_flNextChangeDirection < gpGlobals->curtime)
			{
				//TERO: disabled for now
				m_bDirectionToEnemy = true; //false;
				m_flNextChangeDirection = gpGlobals->curtime + BEE_FLY_TOWARDS_RANDOM_TIME;

				m_angRandomDir = QAngle( BEE_FLY_RANDOM_MAX, BEE_FLY_RANDOM_MAX, BEE_FLY_RANDOM_MAX );
			}
#endif

			return vTargetDir;
		}
	}
#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
	else
	{
		Vector forward;
		AngleVectors( GetLocalAngles() + m_angRandomDir, &forward );
		
		//TERO: should we start going to enemy?
		if (m_flNextChangeDirection < gpGlobals->curtime)
		{
			m_bDirectionToEnemy = true;
			m_flNextChangeDirection = gpGlobals->curtime + BEE_FLY_TOWARDS_PLAYER_TIME;
		}

		return forward;
	}
#endif

	//TERO: no enemy
	Vector forward;
	AngleVectors( GetLocalAngles(), &forward );
	return forward;
}

void CBee::SeekThink( void )
{

	if (m_flBuzzTime < gpGlobals->curtime)
	{
		m_flBuzzTime = gpGlobals->curtime + 1.43;
		EmitSound( "NPC_Bee.Buzz" );
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

	if ( m_flDelayThink < gpGlobals->curtime ) 
	{

		if (m_bHasNoClip)
		{
			SetMoveType( MOVETYPE_FLY );
			m_bHasNoClip = false;
		}

		if (GetEnemy() != NULL)
		{
			if (GetEnemy()->m_iHealth <= 0)
			{
				//DevMsg("Bee: target dead\n");
				SetEnemy( NULL );
				//m_hTarget = NULL;
			}
		}

		if ( !GetEnemy() )
		{
			CNPC_AlienGrunt *pAGrunt= dynamic_cast<CNPC_AlienGrunt*>((CBaseEntity*)m_hOwner);
			if (pAGrunt)
				SetEnemy( pAGrunt->GetEnemy() );
			else m_hOwner = NULL;
		} 

		FlyToEnemy();
		

		if (m_flDelayThink + BEE_THINK_INTERVAL < gpGlobals->curtime)
		{
			//DevMsg("Stopped thinking, %f < %f\n", m_flDelayThink + BEE_THINK_INTERVAL, gpGlobals->curtime);
			m_flDelayThink = gpGlobals->curtime + sk_bee_thinkdelay.GetFloat();
		}
		/*else
		{
			DevMsg("Thinking %f\n", (m_flDelayThink + BEE_THINK_INTERVAL - gpGlobals->curtime) / BEE_THINK_INTERVAL);
		}*/



	}//End DelayThink
	else 
	{
		LeaveTrail(); // 0.01f );
	}


	SetSimulationTime( gpGlobals->curtime );

	StudioFrameAdvance();

	// Think as soon as possible
	SetNextThink( gpGlobals->curtime); //TÄHÄN JOKU PITEMPI AIKA
}

void CBee::LeaveTrail() //float flTrailTime)
{
	if (!m_pTrail)
	{
		m_pTrail	= CSpriteTrail::SpriteTrailCreate( BEE_FLYBEAM_SPRITE, GetLocalOrigin(), false );

		if ( m_pTrail != NULL )
		{
			int attachment = LookupAttachment( "trail" );
			m_pTrail->SetAttachment( this, attachment );
			//m_pTrail->SetTransparency( kRenderTransAdd, 255, 185+random->RandomInt( -16, 16 ), 40, 128, kRenderFxNone );		//BJ: Bee trail color
			m_pTrail->SetTransparency(kRenderTransAdd, random->RandomInt(220, 255), random->RandomInt(0, 80), random->RandomInt(0, 60), 128, kRenderFxNone);	//hl1 colors
			m_pTrail->SetStartWidth( 4.0f );
			m_pTrail->SetLifeTime( 1.0f );
		}
	}

}

CBee *CBee::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	CBee *pBee = (CBee *) CBaseEntity::Create("bee_missile", vecOrigin, vecAngles, pOwner );

	//pBee->SetOwnerEntity( Instance( pentOwner ) );
	if (pBee)
	{
		pBee->Spawn();
		pBee->AddEffects( EF_NOSHADOW );
	
		Vector vecForward;
		AngleVectors( vecAngles, &vecForward );

		pBee->SetAbsVelocity( vecForward * 300 + Vector( 0,0, 128 ) );
	}

	return pBee;
}


void CBee::FlyToEnemy()
{
	float flHomingSpeed = BEE_HOMING_SPEED; 

	float flScale = 1.0f - clamp(((m_flDelayThink + BEE_THINK_INTERVAL - gpGlobals->curtime) / BEE_THINK_INTERVAL), 0.0f, 1.0f);
	flHomingSpeed *= (flScale * 0.75f) + 0.25f;
	
	//DevMsg("Flying interval percentage %f, ", flScale);
	//DevMsg("homing speed %f/%f\n", flHomingSpeed, BEE_HOMING_SPEED);

	//TERO: Here we should check if we can go straight to the enemy

	Vector vTargetDir;


	/*if (!GetEnemy())
	{
		int nSequence = LookupSequence( BEE_FLY_ANIMATION );

		SetCycle( 0 );
		m_flAnimTime = gpGlobals->curtime;
		SetAnimatedEveryTick( true );
		ResetSequence( nSequence );
		ResetClientsideFrame();
	
		SetParent( NULL );
		SetGracePeriod( 2 );
		SetMoveType( MOVETYPE_FLY );


		vTargetDir= GetShootEnemyDir( GetLocalOrigin() );
		VectorNormalize( vTargetDir );

		SetAbsVelocity( vTargetDir * 300 );
	}*/

	vTargetDir = GetShootEnemyDir( GetLocalOrigin() );

	float flDist = VectorNormalize( vTargetDir );

	Vector	vDir	= GetAbsVelocity();
	float	flSpeed	= VectorNormalize( vDir );
	Vector	vNewVelocity = vDir;

	if ( gpGlobals->frametime > 0.0f )
	{
		if ( flSpeed != 0 )
		{
			vNewVelocity = ( flHomingSpeed * vTargetDir ) +  vDir;
			//vNewVelocity = ( flHomingSpeed * vTargetDir ) + ( ( 1 - flHomingSpeed ) * vDir );

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
	}

	QAngle	finalAngles;
	VectorAngles( vNewVelocity, finalAngles );
	SetAbsAngles( finalAngles );

	vNewVelocity *= flSpeed;

	//TERO: Lets add some noise on y
	vNewVelocity.y += m_iBeeFlyOffset;
	m_iBeeFlyOffset = m_iBeeFlyOffset * (-1);


	SetAbsVelocity( vNewVelocity );

	if( GetAbsVelocity() == vec3_origin || flSpeed < BEE_SPEED_DIE )
	{
		// Strange circumstances have brought this missile to halt. Just blow it up.
		Explode();
		return;
	}


	//TERO: lets draw a small trail
	LeaveTrail(); // 0.6f );
}

void CBee::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;

	//if (info.GetInflictor())
	//	DevMsg("weapon_bee: killed by %s\n", info.GetInflictor()->GetClassname());

	Explode();
}

