//========================= Overcharged 2018 custom npc's =======================//
//
// Purpose: Shock roach from op4 race X
//
//=============================================================================//
//

#include "cbase.h"
#include "game.h"
#include "antlion_dust.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hint.h"
#include "ai_hull.h"
#include "ai_navigator.h"
#include "ai_moveprobe.h"
#include "ai_memory.h"
#include "bitstring.h"
#include "hl2_shareddefs.h"
#include "npcevent.h"
#include "soundent.h"
#include "overcharged/npc_shockroach.h"
#include "gib.h"
#include "ai_interactions.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "world.h"
#include "npc_bullseye.h"
#include "physics_npc_solver.h"
#include "hl2_gamerules.h"
#include "decals.h"

#include "BloodDripsGreen.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ROACH_ATTN_IDLE				(float)1.5
#define shockroach_GUTS_GIB_COUNT		1
#define shockroach_LEGS_GIB_COUNT		3
#define shockroach_ALL_GIB_COUNT		5

#define shockroach_RUNMODE_ACCELERATE		1
#define shockroach_RUNMODE_IDLE			2
#define shockroach_RUNMODE_DECELERATE		3
#define shockroach_RUNMODE_FULLSPEED		4
#define shockroach_RUNMODE_PAUSE			5

#define shockroach_RUN_MINSPEED	0.5
#define shockroach_RUN_MAXSPEED	1.0

const float shockroach_BURROWED_FOV = -1.0f;
const float shockroach_UNBURROWED_FOV = 0.5f;

#define shockroach_IGNORE_WORLD_COLLISION_TIME 0.5

const int shockroach_MIN_JUMP_DIST = 48;
const int shockroach_MAX_JUMP_DIST = 256;

#define shockroach_BURROW_POINT_SEARCH_RADIUS 256.0

// Debugging
#define	shockroach_DEBUG_HIDING		1

#define shockroach_BURN_SOUND_FREQUENCY 10

//-----------------------------------------------------------------------------
// Skill settings.
//-----------------------------------------------------------------------------
ConVar	sk_shockroach_health("sk_shockroach_health", "0");
ConVar	sk_shockroach_melee_dmg("sk_shockroach_melee_dmg", "0");
ConVar	sk_shockroach_livetime("sk_shockroach_livetime", "20");
ConVar g_debug_shockroach( "g_debug_shockroach", "0", FCVAR_CHEAT );

//------------------------------------
// Spawnflags
//------------------------------------
#define SF_shockroach_START_HIDDEN		(1 << 16)
#define SF_shockroach_START_HANGING		(1 << 17)


//-----------------------------------------------------------------------------
// Think contexts.
//-----------------------------------------------------------------------------
static const char *s_pPitchContext = "PitchContext";


//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
int AE_shockroach_JUMPATTACK;
int AE_shockroach_JUMP_TELEGRAPH;
int AE_POISONshockroach_FLINCH_HOP;
int AE_POISONshockroach_FOOTSTEP;
int AE_POISONshockroach_THREAT_SOUND;
int AE_shockroach_BURROW_IN;
int AE_shockroach_BURROW_IN_FINISH;
int AE_shockroach_BURROW_OUT;
int AE_shockroach_CEILING_DETACH;


//-----------------------------------------------------------------------------
// Custom schedules.
//-----------------------------------------------------------------------------
enum
{
	SCHED_shockroach_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE,
	SCHED_shockroach_WAKE_ANGRY,
	SCHED_shockroach_WAKE_ANGRY_NO_DISPLAY,
	SCHED_shockroach_DROWN,
	SCHED_shockroach_FAIL_DROWN,
	SCHED_shockroach_AMBUSH,
	SCHED_shockroach_HOP_RANDOMLY, // get off something you're not supposed to be on.
	SCHED_shockroach_BARNACLED,
	SCHED_shockroach_UNHIDE,
	SCHED_shockroach_HARASS_ENEMY,
	SCHED_shockroach_FALL_TO_GROUND,
	SCHED_shockroach_RUN_TO_BURROW_IN,
	SCHED_shockroach_RUN_TO_SPECIFIC_BURROW,
	SCHED_shockroach_BURROW_IN,
	SCHED_shockroach_BURROW_WAIT,
	SCHED_shockroach_BURROW_OUT,
	SCHED_shockroach_WAIT_FOR_CLEAR_UNBURROW,
	SCHED_shockroach_CRAWL_FROM_CANISTER,

	SCHED_FAST_shockroach_RANGE_ATTACK1,

	SCHED_shockroach_CEILING_WAIT,
	SCHED_shockroach_CEILING_DROP,
};


//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_shockroach_HOP_ASIDE = LAST_SHARED_TASK,
	TASK_shockroach_HOP_OFF_NPC,
	TASK_shockroach_DROWN,
	TASK_shockroach_WAIT_FOR_BARNACLE_KILL,
	TASK_shockroach_UNHIDE,
	TASK_shockroach_HARASS_HOP,
	TASK_shockroach_FIND_BURROW_IN_POINT,
	TASK_shockroach_BURROW,
	TASK_shockroach_UNBURROW,
	TASK_shockroach_BURROW_WAIT,
	TASK_shockroach_CHECK_FOR_UNBURROW,
	TASK_shockroach_JUMP_FROM_CANISTER,
	TASK_shockroach_CLIMB_FROM_CANISTER,

	TASK_shockroach_CEILING_WAIT,
	TASK_shockroach_CEILING_POSITION,
	TASK_shockroach_CEILING_DETACH,
	TASK_shockroach_CEILING_FALL,
	TASK_shockroach_CEILING_LAND,
};


//=========================================================
// conditions 
//=========================================================
enum
{
	COND_shockroach_IN_WATER = LAST_SHARED_CONDITION,
	COND_shockroach_ILLEGAL_GROUNDENT,
	COND_shockroach_BARNACLED,
	COND_shockroach_UNHIDE,
};

//=========================================================
// private activities
//=========================================================
int ACT_shockroach_THREAT_DISPLAY;
int ACT_shockroach_HOP_LEFT;
int ACT_shockroach_HOP_RIGHT;
int ACT_shockroach_DROWN;
int ACT_shockroach_BURROW_IN;
int ACT_shockroach_BURROW_OUT;
int ACT_shockroach_BURROW_IDLE;
int ACT_shockroach_CRAWL_FROM_CANISTER_LEFT;
int ACT_shockroach_CRAWL_FROM_CANISTER_CENTER;
int ACT_shockroach_CRAWL_FROM_CANISTER_RIGHT;
int ACT_shockroach_CEILING_IDLE;
int ACT_shockroach_CEILING_DETACH;
int ACT_shockroach_CEILING_FALL;
int ACT_shockroach_CEILING_LAND;

BEGIN_DATADESC( CBaseshockroach )

	// m_nGibCount - don't save
	DEFINE_FIELD( m_bHidden, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimeDrown, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeLive, FIELD_TIME),			// BriJee OVR: Overall roach live time.
	DEFINE_FIELD( m_bCommittedToJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),
	DEFINE_FIELD( m_flIgnoreWorldCollisionTime, FIELD_TIME ),

	DEFINE_KEYFIELD( m_bStartBurrowed, FIELD_BOOLEAN, "startburrowed" ),
	DEFINE_FIELD( m_bBurrowed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flBurrowTime, FIELD_TIME ),
	DEFINE_FIELD( m_nContext, FIELD_INTEGER ),
	DEFINE_FIELD( m_bCrawlFromCanister, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nJumpFromCanisterDir, FIELD_INTEGER ),

	DEFINE_FIELD( m_bHangingFromCeiling, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flIlluminatedTime, FIELD_TIME ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Burrow", InputBurrow ),
	DEFINE_INPUTFUNC( FIELD_VOID, "BurrowImmediate", InputBurrowImmediate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Unburrow", InputUnburrow ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StartHangingFromCeiling", InputStartHangingFromCeiling ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DropFromCeiling", InputDropFromCeiling ),

	// Function Pointers
	DEFINE_THINKFUNC( EliminateRollAndPitch ),
	DEFINE_THINKFUNC( ThrowThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::Spawn( void )
{
	
#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags( SF_NPC_FADE_CORPSE );
#endif // _XBOX

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );

	SetCollisionGroup( HL2COLLISION_GROUP_HEADCRAB );	// BriJee OVR: Stil use headcrab collision...

	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.

	SetBloodColor( BLOOD_COLOR_GREEN );
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;
	m_nGibCount			= shockroach_ALL_GIB_COUNT;

	// Are we starting hidden?
	if ( m_spawnflags & SF_shockroach_START_HIDDEN )
	{
		m_bHidden = true;
		AddSolidFlags( FSOLID_NOT_SOLID );
		SetRenderColorA( 0 );
		m_nRenderMode = kRenderTransTexture;
		AddEffects( EF_NODRAW );
	}
	else
	{
		m_bHidden = false;
	}

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 );
	CapabilitiesAdd( bits_CAP_SQUAD );

	// shockroachs get to cheat for 5 seconds (sjb)
	GetEnemies()->SetFreeKnowledgeDuration( 5.0 );

	m_bHangingFromCeiling = false;
	m_flIlluminatedTime = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Stuff that must happen after NPCInit is called.
//-----------------------------------------------------------------------------
void CBaseshockroach::shockroachInit()
{
	// See if we're supposed to start burrowed
	if ( m_bStartBurrowed )
	{
		SetBurrowed( true );
		SetSchedule( SCHED_shockroach_BURROW_WAIT );
	}

	if ( GetSpawnFlags() & SF_shockroach_START_HANGING )
	{
		SetSchedule( SCHED_shockroach_CEILING_WAIT );
		m_flIlluminatedTime = -1;
	}
}	


//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CBaseshockroach::Precache( void )
{
	BaseClass::Precache();
}	


//-----------------------------------------------------------------------------
// The shockroach will crawl from the cannister, then jump to a burrow point
//-----------------------------------------------------------------------------
void CBaseshockroach::CrawlFromCanister()
{
	// This is necessary to prevent ground computations, etc. from happening
	// while the crawling animation is occuring
	AddFlag( FL_FLY );
	m_bCrawlFromCanister = true;
	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : NewActivity - 
//-----------------------------------------------------------------------------
void CBaseshockroach::OnChangeActivity( Activity NewActivity )
{
	bool fRandomize = false;
	float flRandomRange = 0.0;

	// If this crab is starting to walk or idle, pick a random point within
	// the animation to begin. This prevents lots of crabs being in lockstep.
	if ( NewActivity == ACT_IDLE )
	{
		flRandomRange = 0.75;
		fRandomize = true;
	}
	else if ( NewActivity == ACT_RUN )
	{
		flRandomRange = 0.25;
		fRandomize = true;
	}

	BaseClass::OnChangeActivity( NewActivity );

	if( fRandomize )
	{
		SetCycle( random->RandomFloat( 0.0, flRandomRange ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CBaseshockroach::Classify( void )
{
	if( m_bHidden )
	{
		// Effectively invisible to other AI's while hidden.
		return( CLASS_NONE ); 
	}
	else
	{
		return( CLASS_RACEX );	//BriJee OVR: new class
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &posSrc - 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CBaseshockroach::BodyTarget( const Vector &posSrc, bool bNoisy ) 
{ 
	Vector vecResult;
	vecResult = GetAbsOrigin();
	vecResult.z += 6;
	return vecResult;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CBaseshockroach::GetAutoAimRadius()
{ 
	if( g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE )
	{
		return 24.0f;
	}

	return 12.0f;
}


//-----------------------------------------------------------------------------
// Purpose: Allows each sequence to have a different turn rate associated with it.
// Output : float
//-----------------------------------------------------------------------------
float CBaseshockroach::MaxYawSpeed( void )
{
	return BaseClass::MaxYawSpeed();
}

//-----------------------------------------------------------------------------
// Because the AI code does a tracehull to find the ground under an NPC, shockroachs
// can often be seen standing with one edge of their box perched on a ledge and
// 80% or more of their body hanging out over nothing. This is often a case
// where a shockroach will be unable to pathfind out of its location. This heuristic
// very crudely tries to determine if this is the case by casting a simple ray 
// down from the center of the shockroach.
//-----------------------------------------------------------------------------
#define shockroach_MAX_LEDGE_HEIGHT	12.0f
bool CBaseshockroach::IsFirmlyOnGround()
{
	if( !(GetFlags()&FL_ONGROUND) )
		return false;

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() - Vector( 0, 0, shockroach_MAX_LEDGE_HEIGHT ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );
	return tr.fraction != 1.0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseshockroach::MoveOrigin( const Vector &vecDelta )
{
	UTIL_SetOrigin( this, GetLocalOrigin() + vecDelta );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecPos - 
//-----------------------------------------------------------------------------
void CBaseshockroach::ThrowAt( const Vector &vecPos )
{
	JumpAttack( false, vecPos, true );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecPos - 
//-----------------------------------------------------------------------------
void CBaseshockroach::JumpToBurrowHint( CAI_Hint *pHint )
{
	Vector vecVel = VecCheckToss( this, GetAbsOrigin(), pHint->GetAbsOrigin(), 0.5f, 1.0f, false, NULL, NULL );

	// Undershoot by a little because it looks bad if we overshoot and turn around to burrow.
	vecVel *= 0.9f;
	Leap( vecVel );

	GrabHintNode( pHint );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecVel - 
//-----------------------------------------------------------------------------
void CBaseshockroach::Leap( const Vector &vecVel )
{
	SetTouch( &CBaseshockroach::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + shockroach_IGNORE_WORLD_COLLISION_TIME;

	if( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the shockroach where he actually is...
	m_bMidJump = true;
	SetThink( &CBaseshockroach::ThrowThink );
	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::ThrowThink( void )
{
	if (gpGlobals->curtime > m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CBaseshockroach::CallNPCThink );
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: Does a jump attack at the given position.
// Input  : bRandomJump - Just hop in a random direction.
//			vecPos - Position to jump at, ignored if bRandom is set to true.
//			bThrown - 
//-----------------------------------------------------------------------------
void CBaseshockroach::JumpAttack( bool bRandomJump, const Vector &vecPos, bool bThrown )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
		float gravity = GetCurrentGravity();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		// How fast does the shockroach need to travel to reach the position given gravity?
		float flActualHeight = vecPos.z - GetAbsOrigin().z;
		float height = flActualHeight;
		if ( height < 16 )
		{
			height = 16;
		}
		else
		{
			float flMaxHeight = bThrown ? 400 : 120;
			if ( height > flMaxHeight )
			{
				height = flMaxHeight;
			}
		}

		// overshoot the jump by an additional 8 inches
		// NOTE: This calculation jumps at a position INSIDE the box of the enemy (player)
		// so if you make the additional height too high, the crab can land on top of the
		// enemy's head.  If we want to jump high, we'll need to move vecPos to the surface/outside
		// of the enemy's box.
		
		float additionalHeight = 0;
		if ( height < 32 )
		{
			additionalHeight = 8;
		}

		height += additionalHeight;

		// NOTE: This equation here is from vf^2 = vi^2 + 2*a*d
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		// add in the time it takes to fall the additional height
		// So the impact takes place on the downward slope at the original height
		time += sqrt( (2 * additionalHeight) / gravity );

		// Scale the sideways velocity to get there at the right time
		VectorSubtract( vecPos, GetAbsOrigin(), vecJumpVel );
		vecJumpVel /= time;

		// Speed to offset gravity at the desired height.
		vecJumpVel.z = speed;

		// Don't jump too far/fast.
		float flJumpSpeed = vecJumpVel.Length();
		float flMaxSpeed = bThrown ? 1000.0f : 650.0f;
		if ( flJumpSpeed > flMaxSpeed )
		{
			vecJumpVel *= flMaxSpeed / flJumpSpeed;
		}
	}
	else
	{
		//
		// Jump hop, don't care where.
		//
		Vector forward, up;
		AngleVectors( GetLocalAngles(), &forward, NULL, &up );
		vecJumpVel = Vector( forward.x, forward.y, up.z ) * 350;
	}

	AttackSound();
	Leap( vecJumpVel );
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CBaseshockroach::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_shockroach_JUMPATTACK )
	{
		// Ignore if we're in mid air
		if ( m_bMidJump )
			return;

		CBaseEntity *pEnemy = GetEnemy();
			
		if ( pEnemy )
		{
			if ( m_bCommittedToJump )
			{
				JumpAttack( false, m_vecCommittedJumpPos );
			}
			else
			{
				// Jump at my enemy's eyes.
				JumpAttack( false, pEnemy->EyePosition() );
			}

			m_bCommittedToJump = false;
			
		}
		else
		{
			// Jump hop, don't care where.
			JumpAttack( true );
		}

		return;
	}
	
	if ( pEvent->event == AE_shockroach_CEILING_DETACH )
	{
		SetMoveType( MOVETYPE_STEP );
		RemoveFlag( FL_ONGROUND );
		RemoveFlag( FL_FLY );

		SetAbsVelocity( Vector ( 0, 0, -128 ) );
		return;
	}
	if ( pEvent->event == AE_shockroach_JUMP_TELEGRAPH )
	{
		TelegraphSound();

		CBaseEntity *pEnemy = GetEnemy();
		
		if ( pEnemy )
		{
			// Once we telegraph, we MUST jump. This is also when commit to what point
			// we jump at. Jump at our enemy's eyes.
			m_vecCommittedJumpPos = pEnemy->EyePosition();
			m_bCommittedToJump = true;
		}

		return;
	}

	if ( pEvent->event == AE_shockroach_BURROW_IN )
	{
		EmitSound( "NPC_shockroach.BurrowIn" );
		CreateDust();

		return;
	}

	if ( pEvent->event == AE_shockroach_BURROW_IN_FINISH )
	{
		SetBurrowed( true );
		return;
	}

	if ( pEvent->event == AE_shockroach_BURROW_OUT )
	{
		Assert( m_bBurrowed );
		if ( m_bBurrowed )
		{
			EmitSound( "NPC_shockroach.BurrowOut" );
			CreateDust();
			SetBurrowed( false );

			// We're done with this burrow hint node. It might be NULL here
			// because we may have started burrowed (no hint node in that case).
			GrabHintNode( NULL );
		}

		return;
	}

	CAI_BaseNPC::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose: Does all the fixup for going to/from the burrowed state.
//-----------------------------------------------------------------------------
void CBaseshockroach::SetBurrowed( bool bBurrowed )
{
	if ( bBurrowed )
	{
		AddEffects( EF_NODRAW );
		AddFlag( FL_NOTARGET );
		m_spawnflags |= SF_NPC_GAG;
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_NO;
		m_flFieldOfView = shockroach_BURROWED_FOV;

		SetState( NPC_STATE_IDLE );
		SetActivity( (Activity) ACT_shockroach_BURROW_IDLE );
	}
	else
	{
		RemoveEffects( EF_NODRAW );
		RemoveFlag( FL_NOTARGET );
		m_spawnflags &= ~SF_NPC_GAG;
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_YES;
		m_flFieldOfView	= shockroach_UNBURROWED_FOV;
	}

	m_bBurrowed = bBurrowed;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CBaseshockroach::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_shockroach_CLIMB_FROM_CANISTER:
			AutoMovement( );
			if ( IsActivityFinished() )
			{
				TaskComplete();
			}
			break;

		case TASK_shockroach_JUMP_FROM_CANISTER:
			GetMotor()->UpdateYaw();
			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;

		case TASK_shockroach_WAIT_FOR_BARNACLE_KILL:
			if ( m_flNextFlinchTime < gpGlobals->curtime )
			{
				m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 2.0f );
				CTakeDamageInfo info;
				PainSound( info );
			}
			break;

		case TASK_shockroach_HOP_OFF_NPC:
			if( GetFlags() & FL_ONGROUND )
			{
				TaskComplete();
			}
			else
			{
				// Face the direction I've been forced to jump.
				GetMotor()->SetIdealYawToTargetAndUpdate( GetAbsOrigin() + GetAbsVelocity() );
			}
			break;

		case TASK_shockroach_DROWN:
			if( gpGlobals->curtime > m_flTimeDrown )
			{
				OnTakeDamage( CTakeDamageInfo( this, this, m_iHealth * 2, DMG_DROWN ) );
			}
			break;

		case TASK_RANGE_ATTACK1:
		case TASK_RANGE_ATTACK2:
		case TASK_shockroach_HARASS_HOP:
		{
			if ( IsActivityFinished() )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CBaseshockroach::CallNPCThink );
				SetIdealActivity( ACT_IDLE );

				if ( m_bAttackFailed )
				{
					// our attack failed because we just ran into something solid.
					// delay attacking for a while so we don't just repeatedly leap
					// at the enemy from a bad location.
					m_bAttackFailed = false;
					m_flNextAttack = gpGlobals->curtime + 1.2f;
				}
			}
			break;
		}

		case TASK_shockroach_CHECK_FOR_UNBURROW:
		{			
			// Must wait for our next check time
			if ( m_flBurrowTime > gpGlobals->curtime )
				return;

			// See if we can pop up
			if ( ValidBurrowPoint( GetAbsOrigin() ) )
			{
				m_spawnflags &= ~SF_NPC_GAG;
				RemoveSolidFlags( FSOLID_NOT_SOLID );

				TaskComplete();
				return;
			}

			// Try again in a couple of seconds
			m_flBurrowTime = gpGlobals->curtime + random->RandomFloat( 0.5f, 1.0f );

			break;
		}

		case TASK_shockroach_BURROW_WAIT:
		{	
			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_CAN_RANGE_ATTACK2 ) )
			{
				TaskComplete();
			}
			
			break;
		}

		case TASK_shockroach_CEILING_WAIT:
			{	
#ifdef HL2_EPISODIC
				if ( DarknessLightSourceWithinRadius( this, DARKNESS_LIGHTSOURCE_SIZE ) )
				{
					DropFromCeiling();
				}
#endif

				if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_CAN_RANGE_ATTACK2 ) )
				{
					TaskComplete();
				}

				break;
			}

		case TASK_shockroach_CEILING_DETACH:
			{
				if ( IsActivityFinished() )
				{
					ClearCondition( COND_CAN_RANGE_ATTACK1 );
					RemoveFlag(FL_FLY);
					TaskComplete();
				}
			}
			break;

		case TASK_shockroach_CEILING_FALL:
			{
				Vector vecPrPos;
				trace_t tr;

				//Figure out where the shockroach is going to be in quarter of a second.
				vecPrPos = GetAbsOrigin() + ( GetAbsVelocity() * 0.25f );
				UTIL_TraceHull( vecPrPos, vecPrPos, GetHullMins(), GetHullMaxs(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
				
				if ( tr.startsolid == true || GetFlags() & FL_ONGROUND )
				{
					RemoveSolidFlags( FSOLID_NOT_SOLID );
					TaskComplete();
				}
			}
			break;

		case TASK_shockroach_CEILING_LAND:
			{
				if ( IsActivityFinished() )
				{
					RemoveSolidFlags( FSOLID_NOT_SOLID ); //double-dog verify that we're solid.
					TaskComplete();
					m_bHangingFromCeiling = false;
				}
			}
			break;
		default:
		{
			if (gpGlobals->curtime > m_flTimeLive)			//=========== BriJee OVR: Live time
			{
				//DevMsg("Shock roach live time ended. \n");
				OnTakeDamage(CTakeDamageInfo(this, this, m_iHealth * 2, DMG_DROWN));
			}

			BaseClass::RunTask( pTask );
		}
	}
}


//-----------------------------------------------------------------------------
// Before jumping, shockroachs usually use SetOrigin() to lift themselves off the 
// ground. If the shockroach doesn't have the clearance to so, they'll be stuck
// in the world. So this function makes sure there's headroom first.
//-----------------------------------------------------------------------------
bool CBaseshockroach::HasHeadroom()
{
	trace_t tr;
	UTIL_TraceEntity( this, GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 1 ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

#if 0
	if( tr.fraction == 1.0f )
	{
		Msg("Headroom\n");
	}
	else
	{
		Msg("NO Headroom\n");
	}
#endif

	return (tr.fraction == 1.0);
}

//-----------------------------------------------------------------------------
// Purpose: LeapTouch - this is the shockroach's touch function when it is in the air.
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBaseshockroach::LeapTouch( CBaseEntity *pOther )
{
	m_bMidJump = false;

	if ( IRelationType( pOther ) == D_HT )
	{
		// Don't hit if back on ground
		if ( !( GetFlags() & FL_ONGROUND ) )
		{
	 		if ( pOther->m_takedamage != DAMAGE_NO )
			{
				BiteSound();
				TouchDamage( pOther );

				// attack succeeded, so don't delay our next attack if we previously thought we failed
				m_bAttackFailed = false;
			}
			else
			{
				ImpactSound();
			}
		}
		else
		{
			ImpactSound();
		}
	}
	else if( !(GetFlags() & FL_ONGROUND) )
	{
		// Still in the air...
		if( !pOther->IsSolid() )
		{
			// Touching a trigger or something.
			return;
		}

		// just ran into something solid, so the attack probably failed.  make a note of it
		// so that when the attack is done, we'll delay attacking for a while so we don't
		// just repeatedly leap at the enemy from a bad location.
		m_bAttackFailed = true;

		if( gpGlobals->curtime < m_flIgnoreWorldCollisionTime )
		{
			// shockroachs try to ignore the world, static props, and friends for a 
			// fraction of a second after they jump. This is because they often brush
			// doorframes or props as they leap, and touching those objects turns off
			// this touch function, which can cause them to hit the player and not bite.
			// A timer probably isn't the best way to fix this, but it's one of our 
			// safer options at this point (sjb).
			return;
		}
	}

	// Shut off the touch function.
	SetTouch( NULL );
	SetThink ( &CBaseshockroach::CallNPCThink );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CBaseshockroach::CalcDamageInfo( CTakeDamageInfo *pInfo )
{
	pInfo->Set( this, this, sk_shockroach_melee_dmg.GetFloat(), DMG_SLASH );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: Deal the damage from the shockroach's touch attack.
//-----------------------------------------------------------------------------
void CBaseshockroach::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcDamageInfo( &info );
	pOther->TakeDamage( info  );
}


//---------------------------------------------------------
//---------------------------------------------------------
void CBaseshockroach::GatherConditions( void )
{
	// If we're hidden, just check to see if we should unhide
	if ( m_bHidden )
	{
		// See if there's enough room for our hull to fit here. If so, unhide.
		trace_t tr;
		AI_TraceHull( GetAbsOrigin(), GetAbsOrigin(),GetHullMins(), GetHullMaxs(), MASK_SHOT, this, GetCollisionGroup(), &tr );
		if ( tr.fraction == 1.0 )
		{
			SetCondition( COND_PROVOKED );
			SetCondition( COND_shockroach_UNHIDE );

			if ( g_debug_shockroach.GetInt() == shockroach_DEBUG_HIDING )
			{
				NDebugOverlay::Box( GetAbsOrigin(), GetHullMins(), GetHullMaxs(), 0,255,0, true, 1.0 );
			}
		}
		else if ( g_debug_shockroach.GetInt() == shockroach_DEBUG_HIDING )
		{
			NDebugOverlay::Box( GetAbsOrigin(), GetHullMins(), GetHullMaxs(), 255,0,0, true, 0.1 );
		}

		// Prevent baseclass thinking, so we don't respond to enemy fire, etc.
		return;
	}

	BaseClass::GatherConditions();

	if( m_lifeState == LIFE_ALIVE && GetWaterLevel() > 1 )
	{
		// Start Drowning!
		SetCondition( COND_shockroach_IN_WATER );
	}

	// See if I've landed on an NPC or player or something else illegal
	ClearCondition( COND_shockroach_ILLEGAL_GROUNDENT );
	CBaseEntity *ground = GetGroundEntity();
	if( (GetFlags() & FL_ONGROUND) && ground && !ground->IsWorld() )
	{
		if ( IsHangingFromCeiling() == false )
		{
			if( ( ground->IsNPC() || ground->IsPlayer() ) )
			{
				SetCondition( COND_shockroach_ILLEGAL_GROUNDENT );
			}
			else if( ground->VPhysicsGetObject() && (ground->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD) )
			{
				SetCondition( COND_shockroach_ILLEGAL_GROUNDENT );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
	
	// Are we fading in after being hidden?
	if ( !m_bHidden && (m_nRenderMode != kRenderNormal) )
	{
		int iNewAlpha = MIN( 255, GetRenderColor().a + 120 );
		if ( iNewAlpha >= 255 )
		{
			m_nRenderMode = kRenderNormal;
			SetRenderColorA( 0 );
		}
		else
		{
			SetRenderColorA( iNewAlpha );
		}
	}

	//
	// Make the crab coo a little bit in combat state.
	//
	if (( m_NPCState == NPC_STATE_COMBAT ) && ( random->RandomFloat( 0, 5 ) < 0.1 ))
	{
		IdleSound();
	}

	// Make sure we've turned off our burrow state if we're not in it
	Activity eActivity = GetActivity();
	if ( m_bBurrowed &&
		 ( eActivity != ACT_shockroach_BURROW_IDLE ) &&
		 ( eActivity != ACT_shockroach_BURROW_OUT ) &&
		 ( eActivity != ACT_shockroach_BURROW_IN) )
	{
		DevMsg( "shockroach failed to unburrow properly!\n" );
		Assert( 0 );
		SetBurrowed( false );
	}

}


//-----------------------------------------------------------------------------
// Eliminates roll + pitch from the shockroach
//-----------------------------------------------------------------------------
#define shockroach_ROLL_ELIMINATION_TIME 0.3f
#define shockroach_PITCH_ELIMINATION_TIME 0.3f

//-----------------------------------------------------------------------------
// Eliminates roll + pitch potentially in the shockroach at canister jump time
//-----------------------------------------------------------------------------
void CBaseshockroach::EliminateRollAndPitch()
{
	QAngle angles = GetAbsAngles();
	angles.x = AngleNormalize( angles.x );
	angles.z = AngleNormalize( angles.z );
	if ( ( angles.x == 0.0f ) && ( angles.z == 0.0f ) )
		return;

	float flPitchRate = 90.0f / shockroach_PITCH_ELIMINATION_TIME;
	float flPitchDelta = flPitchRate * TICK_INTERVAL;
	if ( fabs( angles.x ) <= flPitchDelta )
	{
		angles.x = 0.0f;
	}
	else
	{
		flPitchDelta *= (angles.x > 0.0f) ? -1.0f : 1.0f;
		angles.x += flPitchDelta;
	}

	float flRollRate = 180.0f / shockroach_ROLL_ELIMINATION_TIME;
	float flRollDelta = flRollRate * TICK_INTERVAL;
	if ( fabs( angles.z ) <= flRollDelta )
	{
		angles.z = 0.0f;
	}
	else
	{
		flRollDelta *= (angles.z > 0.0f) ? -1.0f : 1.0f;
		angles.z += flRollDelta;
	}

	SetAbsAngles( angles );

	SetContextThink( &CBaseshockroach::EliminateRollAndPitch, gpGlobals->curtime + TICK_INTERVAL, s_pPitchContext );
}


//-----------------------------------------------------------------------------
// Begins the climb from the canister
//-----------------------------------------------------------------------------
void CBaseshockroach::BeginClimbFromCanister()
{
	Assert( GetMoveParent() );
	// Compute a desired position or hint
	Vector vecForward, vecActualForward;
	AngleVectors( GetMoveParent()->GetAbsAngles(), &vecActualForward );
	vecForward = vecActualForward;
	vecForward.z = 0.0f;
	VectorNormalize( vecForward );

	Vector vecSearchCenter = GetAbsOrigin();
	CAI_Hint *pHint = CAI_HintManager::FindHint( this, HINT_HEADCRAB_BURROW_POINT, 0, shockroach_BURROW_POINT_SEARCH_RADIUS, &vecSearchCenter );

	if( !pHint && hl2_episodic.GetBool() )
	{
		// Look for exit points within 10 feet.
		pHint = CAI_HintManager::FindHint( this, HINT_HEADCRAB_EXIT_POD_POINT, 0, 120.0f, &vecSearchCenter );
	}

	if ( pHint && ( !pHint->IsLocked() ) )
	{
		// Claim the hint node so other shockroachs don't try to take it!
		GrabHintNode( pHint );

		// Compute relative yaw..
		Vector vecDelta;
		VectorSubtract( pHint->GetAbsOrigin(), vecSearchCenter, vecDelta );
		vecDelta.z = 0.0f;
		VectorNormalize( vecDelta );

		float flAngle = DotProduct( vecDelta, vecForward );
		if ( flAngle >= 0.707f )
		{
			m_nJumpFromCanisterDir = 1;
		}
		else
		{
			// Check the cross product to see if it's on the left or right.
			// All we care about is the sign of the z component. If it's +, the hint is on the left.
			// If it's -, then the hint is on the right.
			float flCrossZ = vecForward.x * vecDelta.y - vecDelta.x * vecForward.y;
			m_nJumpFromCanisterDir = ( flCrossZ > 0 ) ? 0 : 2;
		}
	}
	else
	{
		// Choose a random direction (forward, left, or right)
		m_nJumpFromCanisterDir = random->RandomInt( 0, 2 );
	}

	Activity act;
	switch( m_nJumpFromCanisterDir )
	{
	case 0:	
		act = (Activity)ACT_shockroach_CRAWL_FROM_CANISTER_LEFT; 
		break;

	default:
	case 1:
		act = (Activity)ACT_shockroach_CRAWL_FROM_CANISTER_CENTER; 
		break;

	case 2:	
		act = (Activity)ACT_shockroach_CRAWL_FROM_CANISTER_RIGHT; 
		break;
	}

	SetIdealActivity( act );
}


//-----------------------------------------------------------------------------
// Jumps from the canister
//-----------------------------------------------------------------------------
#define shockroach_ATTACK_PLAYER_FROM_CANISTER_DIST 250.0f
#define shockroach_ATTACK_PLAYER_FROM_CANISTER_COSANGLE 0.866f

void CBaseshockroach::JumpFromCanister()
{
	Assert( GetMoveParent() );

	Vector vecForward, vecActualForward, vecActualRight;
	AngleVectors( GetMoveParent()->GetAbsAngles(), &vecActualForward, &vecActualRight, NULL );

	switch( m_nJumpFromCanisterDir )
	{
	case 0:
		VectorMultiply( vecActualRight, -1.0f, vecForward );
		break;
	case 1:
		vecForward = vecActualForward;
		break;
	case 2:
		vecForward = vecActualRight;
		break;
	}

	vecForward.z = 0.0f;
	VectorNormalize( vecForward );
	QAngle shockroachAngles;
	VectorAngles( vecForward, shockroachAngles );

	SetActivity( ACT_RANGE_ATTACK1 );
	StudioFrameAdvanceManual( 0.0 );
	SetParent( NULL );
	RemoveFlag( FL_FLY );
	IncrementInterpolationFrame();

	GetMotor()->SetIdealYaw( shockroachAngles.y );
	
	// Check to see if the player is within jump range. If so, jump at him!
	bool bJumpedAtEnemy = false;

	// FIXME: Can't use GetEnemy() here because enemy only updates during
	// schedules which are interruptible by COND_NEW_ENEMY or COND_LOST_ENEMY
	CBaseEntity *pEnemy = BestEnemy();
	if ( pEnemy )
	{
		Vector vecDirToEnemy;
		VectorSubtract( pEnemy->GetAbsOrigin(), GetAbsOrigin(), vecDirToEnemy );
		vecDirToEnemy.z = 0.0f;
		float flDist = VectorNormalize( vecDirToEnemy );
		if ( ( flDist < shockroach_ATTACK_PLAYER_FROM_CANISTER_DIST ) && 
			( DotProduct( vecDirToEnemy, vecForward ) >= shockroach_ATTACK_PLAYER_FROM_CANISTER_COSANGLE ) )
		{
			GrabHintNode( NULL );
			JumpAttack( false, pEnemy->EyePosition(), false );
			bJumpedAtEnemy = true;
		}
	}

	if ( !bJumpedAtEnemy )
	{
		if ( GetHintNode() )
		{
			JumpToBurrowHint( GetHintNode() );
		}
		else
		{
			vecForward *= 100.0f;
			vecForward += GetAbsOrigin();
			JumpAttack( false, vecForward, false );
		}
	}

	EliminateRollAndPitch();
}

#define shockroach_ILLUMINATED_TIME 0.15f

void CBaseshockroach::DropFromCeiling( void )
{
#ifdef HL2_EPISODIC
	if ( HL2GameRules()->IsAlyxInDarknessMode() )
	{
		if ( IsHangingFromCeiling() )
		{
			if ( m_flIlluminatedTime == -1 )
			{
				m_flIlluminatedTime = gpGlobals->curtime + shockroach_ILLUMINATED_TIME;
				return;
			}

			if ( m_flIlluminatedTime <= gpGlobals->curtime )
			{
				if ( IsCurSchedule( SCHED_shockroach_CEILING_DROP ) == false )
				{
					SetSchedule( SCHED_shockroach_CEILING_DROP );

					CBaseEntity *pPlayer = AI_GetSinglePlayer();

					if ( pPlayer )
					{
						SetEnemy( pPlayer ); //Is this a bad thing to do?
						UpdateEnemyMemory( pPlayer, pPlayer->GetAbsOrigin());
					}
				}
			}
		}
	}
#endif // HL2_EPISODIC
}

//-----------------------------------------------------------------------------
// Purpose: Player has illuminated this NPC with the flashlight
//-----------------------------------------------------------------------------
void CBaseshockroach::PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot )
{
	if ( flDot < 0.97387f )
		return;

	DropFromCeiling();
}

bool CBaseshockroach::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
#ifdef HL2_EPISODIC
	if ( IsHangingFromCeiling() )
		return false;
#endif

	return BaseClass::CanBeAnEnemyOf( pEnemy );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CBaseshockroach::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_shockroach_WAIT_FOR_BARNACLE_KILL:
		break;

	case TASK_shockroach_BURROW_WAIT:
		break;

	case TASK_shockroach_CLIMB_FROM_CANISTER:
		BeginClimbFromCanister();
		break;

	case TASK_shockroach_JUMP_FROM_CANISTER:
		JumpFromCanister();
		break;

	case TASK_shockroach_CEILING_POSITION:
		{
			trace_t tr;
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 512 ), NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			// SetMoveType( MOVETYPE_NONE );
			AddFlag(FL_FLY);
			m_bHangingFromCeiling = true;

			//Don't need this anymore
			RemoveSpawnFlags( SF_shockroach_START_HANGING );

			SetAbsOrigin( tr.endpos );

			TaskComplete();
		}
		break;

	case TASK_shockroach_CEILING_WAIT:
		break;

	case TASK_shockroach_CEILING_DETACH:
		{
			SetIdealActivity( (Activity)ACT_shockroach_CEILING_DETACH );
		}
		break;

	case TASK_shockroach_CEILING_FALL:
		{
			SetIdealActivity( (Activity)ACT_shockroach_CEILING_FALL );
		}
		break;

	case TASK_shockroach_CEILING_LAND:
		{
			SetIdealActivity( (Activity)ACT_shockroach_CEILING_LAND );
		}
		break;

	case TASK_shockroach_HARASS_HOP:
		{
			// Just pop up into the air like you're trying to get at the
			// enemy, even though it's known you can't reach them.
			if ( GetEnemy() )
			{
				Vector forward, up;

				GetVectors( &forward, NULL, &up );

				m_vecCommittedJumpPos = GetAbsOrigin();
				m_vecCommittedJumpPos += up * random->RandomFloat( 80, 150 );
				m_vecCommittedJumpPos += forward * random->RandomFloat( 32, 80 );

				m_bCommittedToJump = true;

				SetIdealActivity( ACT_RANGE_ATTACK1 );
			}
			else
			{
				TaskFail( "No enemy" );
			}
		}
		break;

	case TASK_shockroach_HOP_OFF_NPC:
		{
			CBaseEntity *ground = GetGroundEntity();
			if( ground )
			{
				// If jumping off of a physics object that the player is holding, create a 
				// solver to prevent the shockroach from colliding with that object for a 
				// short time.
				if( ground && ground->VPhysicsGetObject() )
				{
					if( ground->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
					{
						NPCPhysics_CreateSolver( this, ground, true, 0.5 );
					}
				}


				Vector vecJumpDir; 

				// Jump in some random direction. This way if the person I'm standing on is
				// against a wall, I'll eventually get away.
				
				vecJumpDir.z = 0;
				vecJumpDir.x = 0;
				vecJumpDir.y = 0;
				
				while( vecJumpDir.x == 0 && vecJumpDir.y == 0 )
				{
					vecJumpDir.x = random->RandomInt( -1, 1 ); 
					vecJumpDir.y = random->RandomInt( -1, 1 );
				}

				vecJumpDir.NormalizeInPlace();

				SetGroundEntity( NULL );
				
				if( HasHeadroom() )
				{
					// Bump up
					MoveOrigin( Vector( 0, 0, 1 ) );
				}
				
				SetAbsVelocity( vecJumpDir * 200 + Vector( 0, 0, 200 ) );
			}
			else
			{
				// *shrug* I guess they're gone now. Or dead.
				TaskComplete();
			}
		}
		break;

		case TASK_shockroach_DROWN:
		{
			// Set the gravity really low here! Sink slowly
			SetGravity( UTIL_ScaleForGravity( 80 ) );
			m_flTimeDrown = gpGlobals->curtime + 4;
			break;
		}

		case TASK_RANGE_ATTACK1:
		{
#ifdef WEDGE_FIX_THIS
			CPASAttenuationFilter filter( this, ATTN_IDLE );
			EmitSound( filter, entindex(), CHAN_WEAPON, pAttackSounds[0], GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch() );
#endif
			SetIdealActivity( ACT_RANGE_ATTACK1 );
			break;
		}

		case TASK_shockroach_UNHIDE:
		{
			m_bHidden = false;
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			RemoveEffects( EF_NODRAW );

			TaskComplete();
			break;
		}

		case TASK_shockroach_CHECK_FOR_UNBURROW:
		{
			if ( ValidBurrowPoint( GetAbsOrigin() ) )
			{
				m_spawnflags &= ~SF_NPC_GAG;
				RemoveSolidFlags( FSOLID_NOT_SOLID );
				TaskComplete();
			}
			break;
		}

		case TASK_shockroach_FIND_BURROW_IN_POINT:
		{	
			if ( FindBurrow( GetAbsOrigin(), pTask->flTaskData, true ) == false )
			{
				TaskFail( "TASK_shockroach_FIND_BURROW_IN_POINT: Unable to find burrow in position\n" );
			}
			else
			{
				TaskComplete();
			}
			break;
		}

		case TASK_shockroach_BURROW:
		{
			Burrow();
			TaskComplete();
			break;
		}

		case TASK_shockroach_UNBURROW:
		{
			Unburrow();
			TaskComplete();
			break;
		}

		default:
		{
			BaseClass::StartTask( pTask );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CBaseshockroach::InnateRange1MinRange( void )
{
	return shockroach_MIN_JUMP_DIST;
}

float CBaseshockroach::InnateRange1MaxRange( void )
{
	return shockroach_MAX_JUMP_DIST;
}

int CBaseshockroach::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( gpGlobals->curtime < m_flNextAttack )
		return 0;

	if ( ( GetFlags() & FL_ONGROUND ) == false )
		return 0;

	// When we're burrowed ignore facing, because when we unburrow we'll cheat and face our enemy.
	if ( !m_bBurrowed && ( flDot < 0.65 ) )
		return COND_NOT_FACING_ATTACK;

	// This code stops lots of shockroachs swarming you and blocking you
	// whilst jumping up and down in your face over and over. It forces
	// them to back up a bit. If this causes problems, consider using it
	// for the fast shockroachs only, rather than just removing it.(sjb)
	if ( flDist < shockroach_MIN_JUMP_DIST )
		return COND_TOO_CLOSE_TO_ATTACK;

	if ( flDist > shockroach_MAX_JUMP_DIST )
		return COND_TOO_FAR_TO_ATTACK;

	// Make sure the way is clear!
	CBaseEntity *pEnemy = GetEnemy();
	if( pEnemy )
	{
		bool bEnemyIsBullseye = ( dynamic_cast<CNPC_Bullseye *>(pEnemy) != NULL );

		trace_t tr;
		AI_TraceLine( EyePosition(), pEnemy->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.m_pEnt != GetEnemy() )
		{
			if ( !bEnemyIsBullseye || tr.m_pEnt != NULL )
				return COND_NONE;
		}

		if( GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z )
		{
			// Only run this test if trying to jump at a player who is higher up than me, else this 
			// code will always prevent a shockroach from jumping down at an enemy, and sometimes prevent it
			// jumping just slightly up at an enemy.
			Vector vStartHullTrace = GetAbsOrigin();
			vStartHullTrace.z += 1.0;

			Vector vEndHullTrace = GetEnemy()->EyePosition() - GetAbsOrigin();
			vEndHullTrace.NormalizeInPlace();
			vEndHullTrace *= 8.0;
			vEndHullTrace += GetAbsOrigin();

			AI_TraceHull( vStartHullTrace, vEndHullTrace,GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

			if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
			{
				return COND_TOO_CLOSE_TO_ATTACK;
			}
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}


//------------------------------------------------------------------------------
// Purpose: Override to do shockroach specific gibs
// Output :
//------------------------------------------------------------------------------
bool CBaseshockroach::CorpseGib( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_shockroach.Gib" );	

	return BaseClass::CorpseGib( info );
}

//------------------------------------------------------------------------------
// Purpose:
// Input  :
//------------------------------------------------------------------------------
void CBaseshockroach::Touch( CBaseEntity *pOther )
{ 
	// If someone has smacked me into a wall then gib!
	if (m_NPCState == NPC_STATE_DEAD) 
	{
		if (GetAbsVelocity().Length() > 250)
		{
			trace_t tr;
			Vector vecDir = GetAbsVelocity();
			VectorNormalize(vecDir);
			AI_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecDir * 100, 
				MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr); 
			float dotPr = DotProduct(vecDir,tr.plane.normal);
			if ((tr.fraction						!= 1.0) && 
				(dotPr <  -0.8) )
			{
				CTakeDamageInfo	info( GetWorldEntity(), GetWorldEntity(), 100.0f, DMG_CRUSH );

				info.SetDamagePosition( tr.endpos );

				Event_Gibbed( info );
			}
		
		}
	}

	// BriJee OVR: Give player shock weapon on touch once!

	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;*/

	//CBaseEntity *pPlayer = AI_GetSinglePlayer();
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

	//if (!pPlayer)
		//return;


	if (pOther->IsPlayer())
	{
		if (!pPlayer->Weapon_OwnsThisType("weapon_shockrifle"))
		{
			pPlayer->GiveNamedItem("weapon_shockrifle");
			UTIL_Remove(this);
			//DevMsg("Player already own shock roach weapon \n");
		}
	}

	BaseClass::Touch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pevAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : 
//-----------------------------------------------------------------------------
int CBaseshockroach::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	//
	// Don't take any acid damage.
	//
	if ( info.GetDamageType() & DMG_ACID )
	{
		info.SetDamage( 0 );
	}

	//
	// Certain death from melee bludgeon weapons!
	//
	if ( info.GetDamageType() & DMG_CLUB )
	{
		info.SetDamage( m_iHealth );
	}

	if( info.GetDamageType() & DMG_BLAST )
	{
		if( random->RandomInt( 0 , 1 ) == 0 )
		{
			// Catch on fire randomly if damaged in a blast.
			Ignite( 30 );
		}
	}

	if( info.GetDamageType() & DMG_BURN )
	{
		// Slow down burn damage so that shockroachs live longer while on fire.
		info.ScaleDamage( 0.25 );

#define shockroach_SCORCH_RATE	5
#define shockroach_SCORCH_FLOOR	30

		if( IsOnFire() )
		{
			Scorch( shockroach_SCORCH_RATE, shockroach_SCORCH_FLOOR );

			if( m_iHealth <= 1 && (entindex() % 2) )
			{
				// Some shockroachs leap at you with their dying breath
				if( !IsCurSchedule( SCHED_shockroach_RANGE_ATTACK1 ) && !IsRunningDynamicInteraction() )
				{
					SetSchedule( SCHED_shockroach_RANGE_ATTACK1 );
				}
			}
		}

		Ignite( 30 );
	}

	/*if (cvar->FindVar("oc_ragdoll_enable_blooddrips")->GetInt())
	{
		int MaxDripsGreen = RandomInt(0, 5);
		Vector	vecTraceDir;
		if (info.GetDamageType() & (DMG_BULLET | DMG_SHOCK))
		{
			for (int i = 0; i < MaxDripsGreen; i++)
			{
				vecTraceDir = WorldSpaceCenter();
				if (LastHitGroup() == HITGROUP_CHEST)
				{
					vecTraceDir.x += random->RandomFloat(-0.1, 0.1);
					vecTraceDir.y += random->RandomFloat(-0.1, 0.1);
					vecTraceDir.z += random->RandomFloat(-0.1, 0.1);
				}
				if (LastHitGroup() == HITGROUP_HEAD)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.4);
					vecTraceDir.y += random->RandomFloat(0.1, 0.4);
					vecTraceDir.z += random->RandomFloat(12.1, 12.4);
				}
				if (LastHitGroup() == HITGROUP_LEFTARM)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
					vecTraceDir.z += random->RandomFloat(0.1, 0.2);
				}
				if (LastHitGroup() == HITGROUP_RIGHTARM)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(12.1, 12.4);
					vecTraceDir.z += random->RandomFloat(0.1, 0.2);
				}
				if (LastHitGroup() == HITGROUP_LEFTLEG)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
					vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
				}
				if (LastHitGroup() == HITGROUP_RIGHTLEG)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(12.1, 12.4);
					vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
				}

				// Set the velocity
				Vector vecVelocity;
				AngularImpulse angImpulse;

				QAngle angles;
				angles.x = random->RandomFloat(-70, 20);
				angles.y = random->RandomFloat(0, 360);
				angles.z = 0.0f;
				AngleVectors(angles, &vecVelocity);

				vecVelocity *= random->RandomFloat(150, 300);
				vecVelocity += GetAbsVelocity();

				angImpulse = RandomAngularImpulse(-280, 280);



				CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
				//pGrenade->SetAbsOrigin(vecTraceDir);
				pGrenade->SetLocalOrigin(vecTraceDir);
				pGrenade->SetAbsAngles(RandomAngle(0, 360));
				DispatchSpawn(pGrenade);
				pGrenade->SetThrower(this);
				pGrenade->SetOwnerEntity(this);
				pGrenade->SetAbsVelocity(vecVelocity);
			}
		}
	}*/
	return CAI_BaseNPC::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseshockroach::ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut )
{
	// Assumes the shockroach mass is 5kg (100 feet per second)
	float MAX_shockroach_RAGDOLL_SPEED = 100.0f * 12.0f * 5.0f;

	Vector vecClampedForce; 
	BaseClass::ClampRagdollForce( vecForceIn, &vecClampedForce );

	// Copy the force to vecForceOut, in case we don't change it.
	*vecForceOut = vecClampedForce;

	float speed = VectorNormalize( vecClampedForce );
	if( speed > MAX_shockroach_RAGDOLL_SPEED )
	{
		// Don't let the ragdoll go as fast as it was going to.
		vecClampedForce *= MAX_shockroach_RAGDOLL_SPEED;
		*vecForceOut = vecClampedForce;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseshockroach::Event_Killed( const CTakeDamageInfo &info )
{
	// Create a little decal underneath the shockroach
	// This type of damage combination happens from dynamic scripted sequences
	if ( info.GetDamageType() & (DMG_GENERIC | DMG_PREVENT_PHYSICS_FORCE) )
	{
		trace_t	tr;
		AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		UTIL_DecalTrace( &tr, "YellowBlood" );
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Type - 
//-----------------------------------------------------------------------------
int CBaseshockroach::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
		case SCHED_FALL_TO_GROUND:
			return SCHED_shockroach_FALL_TO_GROUND;

		case SCHED_WAKE_ANGRY:
		{
			if ( HaveSequenceForActivity((Activity)ACT_shockroach_THREAT_DISPLAY) )
				return SCHED_shockroach_WAKE_ANGRY;
			else
				return SCHED_shockroach_WAKE_ANGRY_NO_DISPLAY;
		}
		
		case SCHED_RANGE_ATTACK1:
			return SCHED_shockroach_RANGE_ATTACK1;

		case SCHED_FAIL_TAKE_COVER:
			return SCHED_ALERT_FACE;

		case SCHED_CHASE_ENEMY_FAILED:
			{
				if( !GetEnemy() )
					break;

				if( !HasCondition( COND_SEE_ENEMY ) )
					break;

				float flZDiff;
				flZDiff = GetEnemy()->GetAbsOrigin().z - GetAbsOrigin().z;

				// Make sure the enemy isn't so high above me that this would look silly.
				if( flZDiff < 128.0f || flZDiff > 512.0f )
					return SCHED_COMBAT_PATROL;

				float flDist;
				flDist = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).Length2D();

				// Maybe a patrol will bring me closer.
				if( flDist > 384.0f )
				{
					return SCHED_COMBAT_PATROL;
				}

				return SCHED_shockroach_HARASS_ENEMY;
			}
			break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseshockroach::SelectSchedule( void )
{
	if ( m_bCrawlFromCanister )
	{
		m_bCrawlFromCanister = false;
		return SCHED_shockroach_CRAWL_FROM_CANISTER;
	}

	// If we're hidden or waiting until seen, don't do much at all
	if ( m_bHidden || HasSpawnFlags(SF_NPC_WAIT_TILL_SEEN) )
	{
		if( HasCondition( COND_shockroach_UNHIDE ) )
		{
			// We've decided to unhide 
			return SCHED_shockroach_UNHIDE;
		}

		return m_bBurrowed ? ( int )SCHED_shockroach_BURROW_WAIT : ( int )SCHED_IDLE_STAND;
	}

	if ( GetSpawnFlags() & SF_shockroach_START_HANGING && IsHangingFromCeiling() == false )
	{
		return SCHED_shockroach_CEILING_WAIT;
	}

	if ( IsHangingFromCeiling() )
	{
		bool bIsAlyxInDarknessMode = false;
#ifdef HL2_EPISODIC
		bIsAlyxInDarknessMode = HL2GameRules()->IsAlyxInDarknessMode();
#endif // HL2_EPISODIC

		if ( bIsAlyxInDarknessMode == false && ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_NEW_ENEMY ) ) )
			return SCHED_shockroach_CEILING_DROP;

		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
			return SCHED_shockroach_CEILING_DROP;

		return SCHED_shockroach_CEILING_WAIT;
	}

	if ( m_bBurrowed )
	{
		if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			return SCHED_shockroach_BURROW_OUT;

		return SCHED_shockroach_BURROW_WAIT;
	}

	if( HasCondition( COND_shockroach_IN_WATER ) )
	{
		// No matter what, drown in water
		return SCHED_shockroach_DROWN;
	}

	if( HasCondition( COND_shockroach_ILLEGAL_GROUNDENT ) )
	{
		// You're on an NPC's head. Get off.
		return SCHED_shockroach_HOP_RANDOMLY;
	}

	if ( HasCondition( COND_shockroach_BARNACLED ) )
	{
		// Caught by a barnacle!
		return SCHED_shockroach_BARNACLED;
	}

	switch ( m_NPCState )
	{
		case NPC_STATE_ALERT:
		{
			if (HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ))
			{
				if ( fabs( GetMotor()->DeltaIdealYaw() ) < ( 1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return SCHED_TAKE_COVER_FROM_ORIGIN;
				}
				else if ( SelectWeightedSequence( ACT_SMALL_FLINCH ) != -1 )
				{
					m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
					return SCHED_SMALL_FLINCH;
				}
			}
			else if (HasCondition( COND_HEAR_DANGER ) ||
					 HasCondition( COND_HEAR_PLAYER ) ||
					 HasCondition( COND_HEAR_WORLD  ) ||
					 HasCondition( COND_HEAR_COMBAT ))
			{
				return SCHED_ALERT_FACE_BESTSOUND;
			}
			else
			{
				return SCHED_PATROL_WALK;
			}
			break;
		}
	}

	if ( HasCondition( COND_FLOATING_OFF_GROUND ) )
	{
		SetGravity( 1.0 );
		SetGroundEntity( NULL );
		return SCHED_FALL_TO_GROUND;
	}

	if ( GetHintNode() && GetHintNode()->HintType() == HINT_HEADCRAB_BURROW_POINT )
	{
		// Only burrow if we're not within leap attack distance of our enemy.
		if ( ( GetEnemy() == NULL ) || ( ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).Length() > shockroach_MAX_JUMP_DIST ) )
		{
			return SCHED_shockroach_RUN_TO_SPECIFIC_BURROW;
		}
		else
		{
			// Forget about burrowing, we've got folks to leap at!
			GrabHintNode( NULL );
		}
	}

	int nSchedule = BaseClass::SelectSchedule();
	if ( nSchedule == SCHED_SMALL_FLINCH )
	{
		 m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
	}

	return nSchedule;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CBaseshockroach::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY && failedTask == TASK_FIND_BACKAWAY_FROM_SAVEPOSITION )
	{
		if ( HasCondition( COND_SEE_ENEMY ) )
		{
			return SCHED_RANGE_ATTACK1;
		}
	}

	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY || failedSchedule == SCHED_PATROL_WALK || failedSchedule == SCHED_COMBAT_PATROL )
	{
		if( !IsFirmlyOnGround() )
		{
			return SCHED_shockroach_HOP_RANDOMLY;
		}
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//			&vecDir - 
//			*ptr - 
//-----------------------------------------------------------------------------
void CBaseshockroach::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo	newInfo = info;

	// Ignore if we're in a dynamic scripted sequence
	if ( info.GetDamageType() & DMG_PHYSGUN && !IsRunningDynamicInteraction() )
	{
		Vector	puntDir = ( info.GetDamageForce() * 1000.0f );

		newInfo.SetDamage( m_iMaxHealth / 3.0f );

		if( info.GetDamage() >= GetHealth() )
		{
			// This blow will be fatal, so scale the damage force
			// (it's a unit vector) so that the ragdoll will be 
			// affected.
			newInfo.SetDamageForce( info.GetDamageForce() * 3000.0f );
		}

		PainSound( newInfo );
		SetGroundEntity( NULL );
		ApplyAbsVelocityImpulse( puntDir );
	}

	BaseClass::TraceAttack( newInfo, vecDir, ptr, pAccumulator );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseshockroach::Ignite( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner )
{
	// Can't start on fire if we're burrowed
	if ( m_bBurrowed )
		return;

	bool bWasOnFire = IsOnFire();

#ifdef HL2_EPISODIC
	if( GetHealth() > flFlameLifetime )
	{
		// Add some burn time to very healthy shockroachs to fix a bug where
		// black shockroachs would sometimes spontaneously extinguish (and survive)
		flFlameLifetime += 10.0f;
	}
#endif// HL2_EPISODIC

 	BaseClass::Ignite( flFlameLifetime, bNPCOnly, flSize, bCalledByLevelDesigner );

	if( !bWasOnFire )
	{
#ifdef HL2_EPISODIC
		if ( HL2GameRules()->IsAlyxInDarknessMode() == true )
		{
			GetEffectEntity()->AddEffects( EF_DIMLIGHT );
		}
#endif // HL2_EPISODIC

		// For the poison shockroach, who runs around when ignited
		SetActivity( TranslateActivity(GetIdealActivity()) );
	}
}


//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CBaseshockroach::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
	{
		// Die instantly
		return false;
	}
	else if (interactionType ==	g_interactionVortigauntStomp)
	{
		SetIdealState( NPC_STATE_PRONE );
		return true;
	}
	else if (interactionType ==	g_interactionVortigauntStompFail)
	{
		SetIdealState( NPC_STATE_COMBAT );
		return true;
	}
	else if (interactionType ==	g_interactionVortigauntStompHit)
	{
		// Gib the existing guy, but only with legs and guts
		m_nGibCount = shockroach_LEGS_GIB_COUNT;
		OnTakeDamage ( CTakeDamageInfo( sourceEnt, sourceEnt, m_iHealth, DMG_CRUSH|DMG_ALWAYSGIB ) );
		
		// Create dead shockroach in its place
		CBaseshockroach *pEntity = (CBaseshockroach*) CreateEntityByName( "npc_shockroach" );
		pEntity->Spawn();
		pEntity->SetLocalOrigin( GetLocalOrigin() );
		pEntity->SetLocalAngles( GetLocalAngles() );
		pEntity->m_NPCState = NPC_STATE_DEAD;
		return true;
	}
	else if (	interactionType ==	g_interactionVortigauntKick
				/* || (interactionType ==	g_interactionBullsquidThrow) */
				)
	{
		SetIdealState( NPC_STATE_PRONE );
		
		if( HasHeadroom() )
		{
			MoveOrigin( Vector( 0, 0, 1 ) );
		}

		Vector vHitDir = GetLocalOrigin() - sourceEnt->GetLocalOrigin();
		VectorNormalize(vHitDir);

		CTakeDamageInfo info( sourceEnt, sourceEnt, m_iHealth+1, DMG_CLUB );
		CalculateMeleeDamageForce( &info, vHitDir, GetAbsOrigin() );

		TakeDamage( info );

		return true;
	}

	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseshockroach::FValidateHintType( CAI_Hint *pHint )
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//-----------------------------------------------------------------------------
void CBaseshockroach::ClearBurrowPoint( const Vector &origin )
{
	CBaseEntity *pEntity = NULL;
	float		flDist;
	Vector		vecSpot, vecCenter, vecForce;

	//Cause a ruckus
	UTIL_ScreenShake( origin, 1.0f, 80.0f, 1.0f, 256.0f, SHAKE_START );

	//Iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( origin, 128 ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity->m_takedamage != DAMAGE_NO && pEntity->Classify() != CLASS_PLAYER && pEntity->VPhysicsGetObject() )
		{
			vecSpot	 = pEntity->BodyTarget( origin );
			vecForce = ( vecSpot - origin ) + Vector( 0, 0, 16 );

			// decrease damage for an ent that's farther from the bomb.
			flDist = VectorNormalize( vecForce );

			//float mass = pEntity->VPhysicsGetObject()->GetMass();
			CollisionProp()->RandomPointInBounds( vec3_origin, Vector( 1.0f, 1.0f, 1.0f ), &vecCenter );

			if ( flDist <= 128.0f )
			{
				pEntity->VPhysicsGetObject()->Wake();
				pEntity->VPhysicsGetObject()->ApplyForceOffset( vecForce * 250.0f, vecCenter );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Determine whether a point is valid or not for burrowing up into
// Input  : &point - point to test for validity
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseshockroach::ValidBurrowPoint( const Vector &point )
{
	trace_t	tr;

	AI_TraceHull( point, point+Vector(0,0,1), GetHullMins(), GetHullMaxs(), 
		MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	// See if we were able to get there
	if ( ( tr.startsolid ) || ( tr.allsolid ) || ( tr.fraction < 1.0f ) )
	{
		CBaseEntity *pEntity = tr.m_pEnt;

		//If it's a physics object, attempt to knock is away, unless it's a car
		if ( ( pEntity ) && ( pEntity->VPhysicsGetObject() ) && ( pEntity->GetServerVehicle() == NULL ) )
		{
			ClearBurrowPoint( point );
		}

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::GrabHintNode( CAI_Hint *pHint )
{
	// Free up the node for use
	ClearHintNode();

	if ( pHint )
	{
		SetHintNode( pHint );
		pHint->Lock( this );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Finds a point where the shockroach can burrow underground.
// Input  : distance - radius to search for burrow spot in
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseshockroach::FindBurrow( const Vector &origin, float distance, bool excludeNear )
{
	// Attempt to find a burrowing point
	CHintCriteria	hintCriteria;

	hintCriteria.SetHintType( HINT_HEADCRAB_BURROW_POINT );
	hintCriteria.SetFlag( bits_HINT_NODE_NEAREST );

	hintCriteria.AddIncludePosition( origin, distance );
	
	if ( excludeNear )
	{
		hintCriteria.AddExcludePosition( origin, 128 );
	}

	CAI_Hint *pHint = CAI_HintManager::FindHint( this, hintCriteria );

	if ( pHint == NULL )
		return false;

	GrabHintNode( pHint );

	// Setup our path and attempt to run there
	Vector vHintPos;
	pHint->GetPosition( this, &vHintPos );

	AI_NavGoal_t goal( vHintPos, ACT_RUN );

	return GetNavigator()->SetGoal( goal );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::Burrow( void )
{
	// Stop us from taking damage and being solid
	m_spawnflags |= SF_NPC_GAG;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::Unburrow( void )
{
	// Become solid again and visible
	m_spawnflags &= ~SF_NPC_GAG;
	RemoveSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_YES;

	SetGroundEntity( NULL );

	// If we have an enemy, come out facing them
	if ( GetEnemy() )
	{
		Vector dir = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
		VectorNormalize(dir);

		GetMotor()->SetIdealYaw( dir );

		QAngle angles = GetLocalAngles();
		angles[YAW] = UTIL_VecToYaw( dir );
		SetLocalAngles( angles );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Tells the shockroach to unburrow as soon the space is clear.
//-----------------------------------------------------------------------------
void CBaseshockroach::InputUnburrow( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_shockroach_WAIT_FOR_CLEAR_UNBURROW );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the shockroach to run to a nearby burrow point and burrow.
//-----------------------------------------------------------------------------
void CBaseshockroach::InputBurrow( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_shockroach_RUN_TO_BURROW_IN );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the shockroach to burrow right where he is.
//-----------------------------------------------------------------------------
void CBaseshockroach::InputBurrowImmediate( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_shockroach_BURROW_IN );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::InputStartHangingFromCeiling( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_shockroach_CEILING_WAIT );
	m_flIlluminatedTime = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::InputDropFromCeiling( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	if ( IsHangingFromCeiling() == false )
		return;

	SetSchedule( SCHED_shockroach_CEILING_DROP );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseshockroach::CreateDust( bool placeDecal )
{
	trace_t	tr;
	AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );

		if ( ( (char) pdata->game.material == CHAR_TEX_CONCRETE ) || ( (char) pdata->game.material == CHAR_TEX_DIRT ) )
		{
			UTIL_CreateAntlionDust( tr.endpos + Vector(0, 0, 24), GetLocalAngles() );

			//CEffectData data;
			//data.m_vOrigin = GetAbsOrigin();
			//data.m_vNormal = tr.plane.normal;
			//DispatchEffect( "shockroachdust", data );
			
			if ( placeDecal )
			{
				UTIL_DecalTrace( &tr, "shockroach.Unburrow" );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::Precache( void )
{
	PrecacheModel( "models/RaceX/shockroach.mdl" );	// BriJee OVR: Roach model

	PrecacheScriptSound( "NPC_shockroach.Gib" );
	PrecacheScriptSound( "NPC_shockroach.Idle" );
	PrecacheScriptSound( "NPC_shockroach.Alert" );
	PrecacheScriptSound( "NPC_shockroach.Pain" );
	PrecacheScriptSound( "NPC_shockroach.Die" );
	PrecacheScriptSound( "NPC_shockroach.Attack" );
	PrecacheScriptSound( "NPC_shockroach.Bite" );
	PrecacheScriptSound( "NPC_shockroach.BurrowIn" );
	PrecacheScriptSound( "NPC_shockroach.BurrowOut" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::Spawn( void )
{
	Precache();
	SetModel( "models/RaceX/shockroach.mdl" );

	BaseClass::Spawn();

	m_flTimeLive = gpGlobals->curtime + sk_shockroach_livetime.GetFloat();

	m_iHealth = sk_shockroach_health.GetFloat();
	m_flBurrowTime = 0.0f;
	m_bCrawlFromCanister = false;
	m_bMidJump = false;

	NPCInit();
	shockroachInit();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity Cshockroach::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_WALK )
		return ACT_RUN;

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::IdleSound( void )
{
	EmitSound( "NPC_shockroach.Idle" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::AlertSound( void )
{
	EmitSound( "NPC_shockroach.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::PainSound( const CTakeDamageInfo &info )
{
	if( IsOnFire() && random->RandomInt( 0, shockroach_BURN_SOUND_FREQUENCY ) > 0 )
	{
		// Don't squeak every think when burning.
		return;
	}

	EmitSound( "NPC_shockroach.Pain" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_shockroach.Die" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::TelegraphSound( void )
{
	//FIXME: Need a real one
	EmitSound( "NPC_shockroach.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::AttackSound( void )
{
	EmitSound( "NPC_shockroach.Attack" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Cshockroach::BiteSound( void )
{
	EmitSound( "NPC_shockroach.Bite" );
}


LINK_ENTITY_TO_CLASS( npc_shockroach, Cshockroach );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
// Output : 
//-----------------------------------------------------------------------------
float Cshockroach::MaxYawSpeed ( void )
{
	switch ( GetActivity() )
	{
	case ACT_IDLE:			
		return 30;

	case ACT_RUN:			
	case ACT_WALK:			
		return 20;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 15;

	case ACT_RANGE_ATTACK1:
		{
			const Task_t *pCurTask = GetTask();
			if ( pCurTask && pCurTask->iTask == TASK_shockroach_JUMP_FROM_CANISTER )
				return 15;
		}
		return 30;

	default:
		return 30;
	}

	return BaseClass::MaxYawSpeed();
}


//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CBaseshockroach::BuildScheduleTestBits( void )
{
	if ( !IsCurSchedule(SCHED_shockroach_DROWN) )
	{
		// Interrupt any schedule unless already drowning.
		SetCustomInterruptCondition( COND_shockroach_IN_WATER );
	}
	else
	{
		// Don't stop drowning just because you're in water!
		ClearCustomInterruptCondition( COND_shockroach_IN_WATER );
	}

	if( !IsCurSchedule(SCHED_shockroach_HOP_RANDOMLY) )
	{
		SetCustomInterruptCondition( COND_shockroach_ILLEGAL_GROUNDENT );
	}
	else
	{
		ClearCustomInterruptCondition( COND_shockroach_ILLEGAL_GROUNDENT );
	}
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_shockroach, CBaseshockroach )

	DECLARE_TASK( TASK_shockroach_HOP_ASIDE )
	DECLARE_TASK( TASK_shockroach_DROWN )
	DECLARE_TASK( TASK_shockroach_HOP_OFF_NPC )
	DECLARE_TASK( TASK_shockroach_WAIT_FOR_BARNACLE_KILL )
	DECLARE_TASK( TASK_shockroach_UNHIDE )
	DECLARE_TASK( TASK_shockroach_HARASS_HOP )
	DECLARE_TASK( TASK_shockroach_BURROW )
	DECLARE_TASK( TASK_shockroach_UNBURROW )
	DECLARE_TASK( TASK_shockroach_FIND_BURROW_IN_POINT )
	DECLARE_TASK( TASK_shockroach_BURROW_WAIT )
	DECLARE_TASK( TASK_shockroach_CHECK_FOR_UNBURROW )
	DECLARE_TASK( TASK_shockroach_JUMP_FROM_CANISTER )
	DECLARE_TASK( TASK_shockroach_CLIMB_FROM_CANISTER )

	DECLARE_TASK( TASK_shockroach_CEILING_POSITION )
	DECLARE_TASK( TASK_shockroach_CEILING_WAIT )
	DECLARE_TASK( TASK_shockroach_CEILING_DETACH )
	DECLARE_TASK( TASK_shockroach_CEILING_FALL )
	DECLARE_TASK( TASK_shockroach_CEILING_LAND )

	DECLARE_ACTIVITY( ACT_shockroach_THREAT_DISPLAY )
	DECLARE_ACTIVITY( ACT_shockroach_HOP_LEFT )
	DECLARE_ACTIVITY( ACT_shockroach_HOP_RIGHT )
	DECLARE_ACTIVITY( ACT_shockroach_DROWN )
	DECLARE_ACTIVITY( ACT_shockroach_BURROW_IN )
	DECLARE_ACTIVITY( ACT_shockroach_BURROW_OUT )
	DECLARE_ACTIVITY( ACT_shockroach_BURROW_IDLE )
	DECLARE_ACTIVITY( ACT_shockroach_CRAWL_FROM_CANISTER_LEFT )
	DECLARE_ACTIVITY( ACT_shockroach_CRAWL_FROM_CANISTER_CENTER )
	DECLARE_ACTIVITY( ACT_shockroach_CRAWL_FROM_CANISTER_RIGHT )
	DECLARE_ACTIVITY( ACT_shockroach_CEILING_FALL )

	DECLARE_ACTIVITY( ACT_shockroach_CEILING_IDLE )
	DECLARE_ACTIVITY( ACT_shockroach_CEILING_DETACH )
	DECLARE_ACTIVITY( ACT_shockroach_CEILING_LAND )

	DECLARE_CONDITION( COND_shockroach_IN_WATER )
	DECLARE_CONDITION( COND_shockroach_ILLEGAL_GROUNDENT )
	DECLARE_CONDITION( COND_shockroach_BARNACLED )
	DECLARE_CONDITION( COND_shockroach_UNHIDE )

	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_shockroach_JUMPATTACK )
	DECLARE_ANIMEVENT( AE_shockroach_JUMP_TELEGRAPH )
	DECLARE_ANIMEVENT( AE_shockroach_BURROW_IN )
	DECLARE_ANIMEVENT( AE_shockroach_BURROW_IN_FINISH )
	DECLARE_ANIMEVENT( AE_shockroach_BURROW_OUT )
	DECLARE_ANIMEVENT( AE_shockroach_CEILING_DETACH )
	
	//=========================================================
	// > SCHED_shockroach_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
		"		COND_NO_PRIMARY_AMMO"
	)

	//=========================================================
	//
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_WAKE_ANGRY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE "
		"		TASK_FACE_IDEAL					0"
		"		TASK_SOUND_WAKE					0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_shockroach_THREAT_DISPLAY"
		""
		"	Interrupts"
	)

	//=========================================================
	//
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_WAKE_ANGRY_NO_DISPLAY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE "
		"		TASK_FACE_IDEAL					0"
		"		TASK_SOUND_WAKE					0"
		"		TASK_FACE_ENEMY					0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_FAST_shockroach_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_FAST_shockroach_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
	)

	//=========================================================
	// The irreversible process of drowning
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_DROWN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_shockroach_FAIL_DROWN"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_shockroach_DROWN"
		"		TASK_shockroach_DROWN			0"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_shockroach_FAIL_DROWN,

		"	Tasks"
		"		TASK_shockroach_DROWN			0"
		""
		"	Interrupts"
	)


	//=========================================================
	// shockroach lurks in place and waits for a chance to jump on
	// some unfortunate soul.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_AMBUSH,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_WAIT_INDEFINITE		0"

		"	Interrupts"
		"		COND_SEE_ENEMY"
		"		COND_SEE_HATE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_PROVOKED"
	)

	//=========================================================
	// shockroach has landed atop another NPC or has landed on 
	// a ledge. Get down!
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_HOP_RANDOMLY,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_shockroach_HOP_OFF_NPC	0"

		"	Interrupts"
	)

	//=========================================================
	// shockroach is in the clutches of a barnacle
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_BARNACLED,

		"	Tasks"
		"		TASK_STOP_MOVING						0"
		"		TASK_SET_ACTIVITY						ACTIVITY:ACT_shockroach_DROWN"
		"		TASK_shockroach_WAIT_FOR_BARNACLE_KILL	0"

		"	Interrupts"
	)

	//=========================================================
	// shockroach is unhiding
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_UNHIDE,

		"	Tasks"
		"		TASK_shockroach_UNHIDE			0"

		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_shockroach_HARASS_ENEMY,

		"	Tasks"
		"		TASK_FACE_ENEMY					0"
		"		TASK_shockroach_HARASS_HOP		0"
		"		TASK_WAIT_FACE_ENEMY			1"
		"		TASK_SET_ROUTE_SEARCH_TIME		2"	// Spend 2 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	300"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"	Interrupts"
		"		COND_NEW_ENEMY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_shockroach_FALL_TO_GROUND,

		"	Tasks"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_shockroach_DROWN"
		"		TASK_FALL_TO_GROUND				0"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_shockroach_CRAWL_FROM_CANISTER,
		"	Tasks"
		"		TASK_shockroach_CLIMB_FROM_CANISTER	0"
		"		TASK_shockroach_JUMP_FROM_CANISTER	0"
		""
		"	Interrupts"
	)

	//==================================================
	// Burrow In
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_BURROW_IN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_shockroach_BURROW				0"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_shockroach_BURROW_IN"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_shockroach_BURROW_IDLE"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_shockroach_BURROW_WAIT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Run to a nearby burrow hint and burrow there
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_RUN_TO_BURROW_IN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_shockroach_FIND_BURROW_IN_POINT	512"
		"		TASK_SET_TOLERANCE_DISTANCE			8"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_shockroach_BURROW_IN"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_GIVE_WAY"
		"		COND_CAN_RANGE_ATTACK1"
	)

	//==================================================
	// Run to m_pHintNode and burrow there
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_RUN_TO_SPECIFIC_BURROW,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_SET_TOLERANCE_DISTANCE			8"
		"		TASK_GET_PATH_TO_HINTNODE			0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_shockroach_BURROW_IN"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_GIVE_WAY"
	)

	//==================================================
	// Wait until we can unburrow and attack something
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_BURROW_WAIT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_shockroach_BURROW_WAIT"
		"		TASK_shockroach_BURROW_WAIT			1"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_NEW_ENEMY"				// HACK: We don't actually choose a new schedule on new enemy, but
											// we need this interrupt so that the shockroach actually acquires
											// new enemies while burrowed. (look in ai_basenpc.cpp for "DO NOT mess")
		"		COND_CAN_RANGE_ATTACK1"
	)

	//==================================================
	// Burrow Out
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_BURROW_OUT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_shockroach_BURROW_WAIT"
		"		TASK_shockroach_UNBURROW			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_shockroach_BURROW_OUT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Wait for it to be clear for unburrowing
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_shockroach_WAIT_FOR_CLEAR_UNBURROW,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_shockroach_BURROW_WAIT"
		"		TASK_shockroach_CHECK_FOR_UNBURROW		1"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_shockroach_BURROW_OUT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Wait until we can drop.
	//==================================================
	DEFINE_SCHEDULE
	(
	SCHED_shockroach_CEILING_WAIT,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_shockroach_CEILING_DROP"
	"		TASK_SET_ACTIVITY					ACTIVITY:ACT_shockroach_CEILING_IDLE"
	"		TASK_shockroach_CEILING_POSITION		0"
	"		TASK_shockroach_CEILING_WAIT			1"
	""
	"	Interrupts"
	"		COND_TASK_FAILED"
	"		COND_NEW_ENEMY"	
	"		COND_CAN_RANGE_ATTACK1"
	)

	//==================================================
	// Deatch from ceiling.
	//==================================================
	DEFINE_SCHEDULE
	(
	SCHED_shockroach_CEILING_DROP,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_shockroach_CEILING_WAIT"
	"		TASK_shockroach_CEILING_DETACH		0"
	"		TASK_shockroach_CEILING_FALL			0"
	"		TASK_shockroach_CEILING_LAND			0"
	""
	"	Interrupts"
	"		COND_TASK_FAILED"
	)

AI_END_CUSTOM_NPC()
