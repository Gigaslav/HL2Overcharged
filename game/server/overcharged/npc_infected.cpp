//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "ai_task.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "npc_BaseZombie.h"
#include "movevars_shared.h"
#include "IEffects.h"
#include "props.h"
#include "physics_npc_solver.h"
#include "physics_prop_ragdoll.h"

#include "gib.h"

#ifdef HL2_EPISODIC
#include "episodic/ai_behavior_passenger_zombie.h"
#endif	// HL2_EPISODIC

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define Infected_IDLE_PITCH			35
#define Infected_MIN_PITCH			70
#define Infected_MAX_PITCH			130
#define Infected_SOUND_UPDATE_FREQ	0.5

#define Infected_MAXLEAP_Z		128

#define Infected_EXCITE_DIST 480.0

#define Infected_BASE_FREQ 1.5

// If flying at an enemy, and this close or closer, start playing the maul animation!!
#define Infected_MAUL_RANGE	300

ConVar aa_npc_infected_model( "aa_npc_infected_model", "models/zombie/fast.mdl", FCVAR_REPLICATED, "Sets the model of Npc Infected." );
ConVar aa_npc_infected_health( "aa_npc_infected_health", "60", FCVAR_REPLICATED, "Npc Infected Health." );

#ifdef HL2_EPISODIC

int AE_PASSENGER_PHYSICS_PUSH1;
int AE_Infected_VEHICLE_LEAP;
int AE_Infected_VEHICLE_SS_DIE;	// Killed while doing scripted sequence on vehicle

#endif // HL2_EPISODIC

enum
{
	COND_Infected_CLIMB_TOUCH	= LAST_BASE_ZOMBIE_CONDITION,
};

envelopePoint_t envInfectedVolumeJump[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.2f,
	},
};

envelopePoint_t envInfectedVolumePain[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.0f,
	},
};

envelopePoint_t envInfectedInverseVolumePain[] =
{
	{	0.0f, 0.0f,
		0.1f, 0.1f,
	},
	{	1.0f, 1.0f,
		1.0f, 1.0f,
	},
};

envelopePoint_t envInfectedVolumeJumpPostApex[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.2f,
	},
};

envelopePoint_t envInfectedVolumeClimb[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		0.2f, 0.2f,
	},
};

envelopePoint_t envInfectedMoanVolumeFast[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		0.2f, 0.3f,
	},
};

envelopePoint_t envInfectedMoanVolume[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	1.0f, 1.0f,
		0.2f, 0.2f,
	},
	{	0.0f, 0.0f,
		1.0f, 0.4f,
	},
};

envelopePoint_t envInfectedFootstepVolume[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.7f, 0.7f,
		0.2f, 0.2f,
	},
};

envelopePoint_t envInfectedVolumeFrenzy[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		2.0f, 2.0f,
	},
};


//=========================================================
// animation events
//=========================================================
int AE_Infected_LEAP;
int AE_Infected_GALLOP_LEFT;
int AE_Infected_GALLOP_RIGHT;
int AE_Infected_CLIMB_LEFT;
int AE_Infected_CLIMB_RIGHT;

//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_Infected_DO_ATTACK = LAST_SHARED_TASK + 100,	// again, my !!!HACKHACK
	TASK_Infected_LAND_RECOVER,
	TASK_Infected_UNSTICK_JUMP,
	TASK_Infected_JUMP_BACK,
	TASK_Infected_VERIFY_ATTACK,
};

//=========================================================
// activities
//=========================================================
int ACT_Infected_LEAP_SOAR;
int ACT_Infected_LEAP_STRIKE;
int ACT_Infected_LAND_RIGHT;
int ACT_Infected_LAND_LEFT;
int ACT_Infected_FRENZY;
int ACT_Infected_BIG_SLASH;

//=========================================================
// schedules
//=========================================================
enum
{
	SCHED_Infected_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE + 100, // hack to get past the base zombie's schedules
	SCHED_Infected_UNSTICK_JUMP,
	SCHED_Infected_CLIMBING_UNSTICK_JUMP,
	SCHED_Infected_MELEE_ATTACK1,
	SCHED_Infected_TORSO_MELEE_ATTACK1,
};



//=========================================================
//=========================================================
class CInfected : public CNPC_BaseZombie
{
	DECLARE_CLASS( CInfected, CNPC_BaseZombie );

public:
	void Spawn( void );
	void Precache( void );

	void SetZombieModel( void );
	bool CanSwatPhysicsObjects( void ) { return false; }

	int	TranslateSchedule( int scheduleType );

	Activity NPC_TranslateActivity( Activity baseAct );

	void LeapAttackTouch( CBaseEntity *pOther );
	void ClimbTouch( CBaseEntity *pOther );

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	int SelectSchedule( void );
	void OnScheduleChange( void );

	void PrescheduleThink( void );

	float InnateRange1MaxRange( void );
	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );

	virtual float GetClawAttackRange() const { return 50; }

	bool ShouldPlayFootstepMoan( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	void PostNPCInit( void );

	void LeapAttack( void );
	void LeapAttackSound( void );

	//void BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce );

	bool IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;
	bool MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
	bool ShouldFailNav( bool bMovementFailed );

	int	SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	const char *GetMoanSound( int nSound );

	void OnChangeActivity( Activity NewActivity );
	void OnStateChange( NPC_STATE OldState, NPC_STATE NewState );
	void Event_Killed( const CTakeDamageInfo &info );
	//bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );

	virtual Vector GetAutoAimCenter() { return WorldSpaceCenter() - Vector( 0, 0, 12.0f ); }

	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info ); 
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void FootstepSound( bool fRightFoot );
	void FootscuffSound( bool fRightFoot ) {}; // fast guy doesn't scuff
	void StopLoopingSounds( void );

	void SoundInit( void );
	void SetIdleSoundState( void );
	void SetAngrySoundState( void );

	void BuildScheduleTestBits( void );

	void BeginNavJump( void );
	void EndNavJump( void );

	bool IsNavJumping( void ) { return m_fIsNavJumping; }
	void OnNavJumpHitApex( void );

	void BeginAttackJump( void );
	void EndAttackJump( void );

	float		MaxYawSpeed( void );

	virtual const char *GetHeadcrabClassname( void );
	virtual const char *GetHeadcrabModel( void );
	virtual const char *GetLegsModel( void );
	virtual const char *GetTorsoModel( void );

//=============================================================================
#ifdef HL2_EPISODIC

public:
	virtual bool	CreateBehaviors( void );
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual	void	UpdateEfficiency( bool bInPVS );
	virtual bool	IsInAVehicle( void );
	void			InputAttachToVehicle( inputdata_t &inputdata );
	void			VehicleLeapAttackTouch( CBaseEntity *pOther );

private:
	void			VehicleLeapAttack( void );
	bool			CanEnterVehicle( CPropJeepEpisodic *pVehicle );

	CAI_PassengerBehaviorZombie		m_PassengerBehavior;

#endif	// HL2_EPISODIC
//=============================================================================

protected:

	static const char *pMoanSounds[];

	// Sound stuff
	float			m_flDistFactor; 
	unsigned char	m_iClimbCount; // counts rungs climbed (for sound)
	bool			m_fIsNavJumping;
	bool			m_fIsAttackJumping;
	bool			m_fHitApex;
	mutable float	m_flJumpDist;

	bool			m_fHasScreamed;

private:
	float	m_flNextMeleeAttack;
	bool	m_fJustJumped;
	float	m_flJumpStartAltitude;
	float	m_flTimeUpdateSound;

	CSoundPatch	*m_pLayer2; // used for climbing ladders, and when jumping (pre apex)

public:
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( npc_Infected, CInfected );
//LINK_ENTITY_TO_CLASS( npc_Infected_torso, CInfected );


BEGIN_DATADESC( CInfected )

	DEFINE_FIELD( m_flDistFactor, FIELD_FLOAT ),
	DEFINE_FIELD( m_iClimbCount, FIELD_CHARACTER ),
	DEFINE_FIELD( m_fIsNavJumping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fIsAttackJumping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fHitApex, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flJumpDist, FIELD_FLOAT ),
	DEFINE_FIELD( m_fHasScreamed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextMeleeAttack, FIELD_TIME ),
	DEFINE_FIELD( m_fJustJumped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flJumpStartAltitude, FIELD_FLOAT ),
	DEFINE_FIELD( m_flTimeUpdateSound, FIELD_TIME ),

	// Function Pointers
	DEFINE_ENTITYFUNC( LeapAttackTouch ),
	DEFINE_ENTITYFUNC( ClimbTouch ),
	DEFINE_SOUNDPATCH( m_pLayer2 ),

#ifdef HL2_EPISODIC
	DEFINE_ENTITYFUNC( VehicleLeapAttackTouch ),
	DEFINE_INPUTFUNC( FIELD_STRING, "AttachToVehicle", InputAttachToVehicle ),
#endif	// HL2_EPISODIC

END_DATADESC()


const char *CInfected::pMoanSounds[] =
{
	"NPC_Infected.Moan1",
};

//-----------------------------------------------------------------------------
// The model we use for our legs when we get blowed up.
//-----------------------------------------------------------------------------
//static const char *s_pLegsModel = "models/zombie/classic_legs.mdl";


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CInfected::Precache( void )
{
	CBaseEntity::PrecacheModel (aa_npc_infected_model.GetString());

	PrecacheModel( "models/gibs/HGIBS_rib.mdl" );
	PrecacheModel( "models/gibs/HGIBS_scapula.mdl" );
	PrecacheModel( "models/gibs/HGIBS_spine.mdl" );
	PrecacheModel( "models/humans/Charple03.mdl" );
	PrecacheModel( "models/blackout.mdl" );

	PrecacheModel( "models/zombie/classic_torso.mdl" );
	PrecacheModel( "models/zombie/classic_legs.mdl" );
	
	PrecacheScriptSound( "NPC_Infected.LeapAttack" );
	PrecacheScriptSound( "NPC_Infected.FootstepRight" );
	PrecacheScriptSound( "NPC_Infected.FootstepLeft" );
	PrecacheScriptSound( "NPC_Infected.AttackHit" );
	PrecacheScriptSound( "NPC_Infected.AttackMiss" );
	PrecacheScriptSound( "NPC_Infected.LeapAttack" );
	PrecacheScriptSound( "NPC_Infected.Attack" );
	PrecacheScriptSound( "NPC_Infected.Idle" );
	PrecacheScriptSound( "NPC_Infected.AlertFar" );
	PrecacheScriptSound( "NPC_Infected.AlertNear" );
	PrecacheScriptSound( "NPC_Infected.GallopLeft" );
	PrecacheScriptSound( "NPC_Infected.GallopRight" );
	PrecacheScriptSound( "NPC_Infected.Scream" );
	PrecacheScriptSound( "NPC_Infected.RangeAttack" );
	PrecacheScriptSound( "NPC_Infected.Frenzy" );
	PrecacheScriptSound( "NPC_Infected.NoSound" );
	PrecacheScriptSound( "NPC_Infected.Die" );

	PrecacheScriptSound( "NPC_Infected.Gurgle" );

	PrecacheScriptSound( "NPC_Infected.Moan1" );

	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CInfected::OnScheduleChange( void )
{
	if ( m_flNextMeleeAttack > gpGlobals->curtime + 1 )
	{
		// Allow melee attacks again.
		m_flNextMeleeAttack = gpGlobals->curtime + 0.5;
	}

	BaseClass::OnScheduleChange();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CInfected::SelectSchedule ( void )
{

// ========================================================
#ifdef HL2_EPISODIC

	// Defer all decisions to the behavior if it's running
	if ( m_PassengerBehavior.CanSelectSchedule() )
	{
		DeferSchedulingToBehavior( &m_PassengerBehavior );
		return BaseClass::SelectSchedule();
	}

#endif //HL2_EPISODIC
// ========================================================

	if ( HasCondition( COND_ZOMBIE_RELEASECRAB ) )
	{
		// Death waits for no man. Or zombie. Or something.
		return SCHED_ZOMBIE_RELEASECRAB;
	}

	if ( HasCondition( COND_Infected_CLIMB_TOUCH ) )
	{
		return SCHED_Infected_UNSTICK_JUMP;
	}

	switch ( m_NPCState )
	{
	case NPC_STATE_COMBAT:
		if ( HasCondition( COND_LOST_ENEMY ) || ( HasCondition( COND_ENEMY_UNREACHABLE ) && MustCloseToAttack() ) )
		{
			// Set state to alert and recurse!
			SetState( NPC_STATE_ALERT );
			return SelectSchedule();
		}
		break;

	case NPC_STATE_ALERT:
		if ( HasCondition( COND_LOST_ENEMY ) || ( HasCondition( COND_ENEMY_UNREACHABLE ) && MustCloseToAttack() ) )
		{
			ClearCondition( COND_LOST_ENEMY );
			ClearCondition( COND_ENEMY_UNREACHABLE );
			SetEnemy( NULL );

#ifdef DEBUG_ZOMBIES
			DevMsg("Wandering\n");
#endif

			// Just lost track of our enemy. 
			// Wander around a bit so we don't look like a dingus.
			return SCHED_ZOMBIE_WANDER_MEDIUM;
		}
		break;
	}

	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CInfected::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if( GetGroundEntity() && GetGroundEntity()->Classify() == CLASS_HEADCRAB )
	{
		// Kill!
		CTakeDamageInfo info;
		info.SetDamage( GetGroundEntity()->GetHealth() );
		info.SetAttacker( this );
		info.SetInflictor( this );
		info.SetDamageType( DMG_GENERIC );
		GetGroundEntity()->TakeDamage( info );
	}

 	if( m_pMoanSound && gpGlobals->curtime > m_flTimeUpdateSound )
	{
		// Manage the snorting sound, pitch up for closer.
		float flDistNoBBox;

		if( GetEnemy() && m_NPCState == NPC_STATE_COMBAT )
		{
			flDistNoBBox = ( GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter() ).Length();
			flDistNoBBox -= WorldAlignSize().x;
		}
		else
		{
			// Calm down!
			flDistNoBBox = Infected_EXCITE_DIST;
			m_flTimeUpdateSound += 1.0;
		}

		if( flDistNoBBox >= Infected_EXCITE_DIST && m_flDistFactor != 1.0 )
		{
			// Go back to normal pitch.
			m_flDistFactor = 1.0;

			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, Infected_IDLE_PITCH, Infected_SOUND_UPDATE_FREQ );
		}
		else if( flDistNoBBox < Infected_EXCITE_DIST )
		{
			// Zombie is close! Recalculate pitch.
			int iPitch;

			m_flDistFactor = min( 1.0, 1 - flDistNoBBox / Infected_EXCITE_DIST ); 
			iPitch = Infected_MIN_PITCH + ( ( Infected_MAX_PITCH - Infected_MIN_PITCH ) * m_flDistFactor); 
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, iPitch, Infected_SOUND_UPDATE_FREQ );
		}

		m_flTimeUpdateSound = gpGlobals->curtime + Infected_SOUND_UPDATE_FREQ;
	}

	// Crudely detect the apex of our jump
	if( IsNavJumping() && !m_fHitApex && GetAbsVelocity().z <= 0.0 )
	{
		OnNavJumpHitApex();
	}

	if( IsCurSchedule(SCHED_Infected_RANGE_ATTACK1, false) )
	{
		// Think more frequently when flying quickly through the 
		// air, to update the server's location more often.
		SetNextThink(gpGlobals->curtime);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Startup all of the sound patches that the fast zombie uses.
//
//
//-----------------------------------------------------------------------------
void CInfected::SoundInit( void )
{
	if( !m_pMoanSound )
	{
		// !!!HACKHACK - kickstart the moan sound. (sjb)
		MoanSound( envInfectedMoanVolume, ARRAYSIZE( envInfectedMoanVolume ) );

		// Clear the commands that the base class gave the moaning sound channel.
		ENVELOPE_CONTROLLER.CommandClear( m_pMoanSound );
	}

	CPASAttenuationFilter filter( this );

	if( !m_pLayer2 )
	{
		// Set up layer2
		m_pLayer2 = ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_VOICE, "NPC_Infected.Gurgle", ATTN_NORM );

		// Start silent.
		ENVELOPE_CONTROLLER.Play( m_pLayer2, 0.0, 100 );
	}

	SetIdleSoundState();
}

//-----------------------------------------------------------------------------
// Purpose: Make the zombie sound calm.
//-----------------------------------------------------------------------------
void CInfected::SetIdleSoundState( void )
{
	// Main looping sound
	if ( m_pMoanSound )
	{
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, Infected_IDLE_PITCH, 1.0 );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 0.75, 1.0 );
	}

	// Second Layer
	if ( m_pLayer2 )
	{
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pLayer2, 100, 1.0 );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pLayer2, 0.0, 1.0 );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Make the zombie sound pizzled
//-----------------------------------------------------------------------------
void CInfected::SetAngrySoundState( void )
{
	if (( !m_pMoanSound ) || ( !m_pLayer2 ))
	{
		return;
	}

	EmitSound( "NPC_Infected.LeapAttack" );

	// Main looping sound
	ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, Infected_MIN_PITCH, 0.5 );
	ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 1.0, 0.5 );

	// Second Layer
	ENVELOPE_CONTROLLER.SoundChangePitch( m_pLayer2, 100, 1.0 );
	ENVELOPE_CONTROLLER.SoundChangeVolume( m_pLayer2, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CInfected::Spawn( void )
{
	Precache();

	m_fJustJumped = false;

	/*m_fIsTorso =*/ m_fIsHeadless = true;

	//if( FClassnameIs( this, "npc_Infected" ) )
	//{
		m_fIsTorso = false;
	/*}
	else
	{
		// This was placed as an npc_Infected_torso
		m_fIsTorso = true;
	}*/

#ifdef HL2_EPISODIC
	SetBloodColor( BLOOD_COLOR_RED );
#else
	SetBloodColor( BLOOD_COLOR_YELLOW );
#endif // HL2_EPISODIC

	m_iHealth			= aa_npc_infected_health.GetFloat();//50;
	m_flFieldOfView		= 0.2;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_CLIMB | bits_CAP_MOVE_JUMP | bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 /* | bits_CAP_INNATE_MELEE_ATTACK1 */);

	/*if ( m_fIsTorso == true )
	{
		CapabilitiesRemove( bits_CAP_MOVE_JUMP | bits_CAP_INNATE_RANGE_ATTACK1 );
	}*/

	m_flNextAttack = gpGlobals->curtime;

	m_pLayer2 = NULL;
	m_iClimbCount = 0;

	EndNavJump();

	m_flDistFactor = 1.0;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CInfected::PostNPCInit( void )
{
	SoundInit();

	m_flTimeUpdateSound = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the classname (ie "npc_headcrab") to spawn when our headcrab bails.
//-----------------------------------------------------------------------------
const char *CInfected::GetHeadcrabClassname( void )
{
	//return "npc_headcrab_fast";
	return ""; // "npc_citizen";
}

const char *CInfected::GetHeadcrabModel( void )
{
	return ""; //"models/headcrab.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CInfected::MaxYawSpeed( void )
{
	switch( GetActivity() )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 120;
		break;

	case ACT_RUN:
		return 160;
		break;

	case ACT_WALK:
	case ACT_IDLE:
		return 25;
		break;
		
	default:
		return 20;
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CInfected::SetZombieModel( void )
{
	Hull_t lastHull = GetHullType();

	/*if ( m_fIsTorso )
	{
		SetModel( "models/zombie/classic_torso.mdl" );
		SetHullType(HULL_TINY);
	}
	else
	{*/
		SetModel( aa_npc_infected_model.GetString() );//"models/humans/zombies/infected.mdl" );
		SetHullType(HULL_HUMAN);
	//}

	SetBodygroup( ZOMBIE_BODYGROUP_HEADCRAB, !m_fIsHeadless );

	SetHullSizeNormal( true );
	SetDefaultEyeOffset();
	SetActivity( ACT_IDLE );

	// hull changed size, notify vphysics
	// UNDONE: Solve this generally, systematically so other
	// NPCs can change size
	if ( lastHull != GetHullType() )
	{
		if ( VPhysicsGetObject() )
		{
			SetupVPhysicsHull();
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the model to use for our legs ragdoll when we are blown in twain.
//-----------------------------------------------------------------------------
const char *CInfected::GetLegsModel( void )
{
	return aa_npc_infected_model.GetString();//"models/humans/Charple03.mdl";//s_pLegsModel;
}

const char *CInfected::GetTorsoModel( void )
{
	return "models/blackout.mdl"; //"models/zombie/classic_torso.mdl";
}


//-----------------------------------------------------------------------------
// Purpose: See if I can swat the player
//
//
//-----------------------------------------------------------------------------
int CInfected::MeleeAttack1Conditions( float flDot, float flDist )
{
	if ( !GetEnemy() )
	{
		return COND_NONE;
	}

	if( !(GetFlags() & FL_ONGROUND) )
	{
		// Have to be on the ground!
		return COND_NONE;
	}

	if( gpGlobals->curtime < m_flNextMeleeAttack )
	{
		return COND_NONE;
	}
	
	int baseResult = BaseClass::MeleeAttack1Conditions( flDot, flDist );

	// @TODO (toml 07-21-04): follow up with Steve to find out why fz was explicitly not using these conditions
	if ( baseResult == COND_TOO_FAR_TO_ATTACK || baseResult == COND_NOT_FACING_ATTACK )
	{
		return COND_NONE;
	}

	return baseResult;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CInfected::GetMoanSound( int nSound )
{
	return pMoanSounds[ nSound % ARRAYSIZE( pMoanSounds ) ];
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CInfected::FootstepSound( bool fRightFoot )
{
	if( fRightFoot )
	{
		EmitSound( "NPC_Infected.FootstepRight" );
	}
	else
	{
		EmitSound( "NPC_Infected.FootstepLeft" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CInfected::AttackHitSound( void )
{
	EmitSound( "NPC_Infected.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CInfected::AttackMissSound( void )
{
	// Play a random attack miss sound
	EmitSound( "NPC_Infected.AttackMiss" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CInfected::LeapAttackSound( void )
{
	EmitSound( "NPC_Infected.LeapAttack" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CInfected::AttackSound( void )
{
	EmitSound( "NPC_Infected.Attack" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CInfected::IdleSound( void )
{
	EmitSound( "NPC_Infected.Idle" );
	MakeAISpookySound( 360.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random pain sound.
//-----------------------------------------------------------------------------
void CInfected::PainSound( const CTakeDamageInfo &info )
{
	if ( m_pLayer2 )
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pLayer2, SOUNDCTRL_CHANGE_VOLUME, envInfectedVolumePain, ARRAYSIZE(envInfectedVolumePain) );
	if ( m_pMoanSound )
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pMoanSound, SOUNDCTRL_CHANGE_VOLUME, envInfectedInverseVolumePain, ARRAYSIZE(envInfectedInverseVolumePain) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CInfected::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( "NPC_Infected.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random alert sound.
//-----------------------------------------------------------------------------
void CInfected::AlertSound( void )
{
	CBaseEntity *pPlayer = AI_GetSinglePlayer();

	if( pPlayer )
	{
		// Measure how far the player is, and play the appropriate type of alert sound. 
		// Doesn't matter if I'm getting mad at a different character, the player is the
		// one that hears the sound.
		float flDist;

		flDist = ( GetAbsOrigin() - pPlayer->GetAbsOrigin() ).Length();

		if( flDist > 512 )
		{
			EmitSound( "NPC_Infected.AlertFar" );
		}
		else
		{
			EmitSound( "NPC_Infected.AlertNear" );
		}
	}

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define Infected_MINLEAP			200
#define Infected_MAXLEAP			300
float CInfected::InnateRange1MaxRange( void ) 
{ 
	return Infected_MAXLEAP; 
}


//-----------------------------------------------------------------------------
// Purpose: See if I can make my leaping attack!!
//
//
//-----------------------------------------------------------------------------
int CInfected::RangeAttack1Conditions( float flDot, float flDist )
{

	if (GetEnemy() == NULL)
	{
		return( COND_NONE );
	}

	if( !(GetFlags() & FL_ONGROUND) )
	{
		return COND_NONE;
	}

	if( gpGlobals->curtime < m_flNextAttack )
	{
		return( COND_NONE );
	}

	// make sure the enemy isn't on a roof and I'm in the streets (Ravenholm)
	float flZDist;
	flZDist = fabs( GetEnemy()->GetLocalOrigin().z - GetLocalOrigin().z );
	if( flZDist > Infected_MAXLEAP_Z )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( flDist > InnateRange1MaxRange() )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( flDist < Infected_MINLEAP )
	{
		return COND_NONE;
	}

	if (flDot < 0.8) 
	{
		return COND_NONE;
	}

	if ( !IsMoving() )
	{
		// I Have to be running!!!
		return COND_NONE;
	}

	// Don't jump at the player unless he's facing me.
	// This allows the player to get away if he turns and sprints
	CBasePlayer *pPlayer = static_cast<CBasePlayer*>( GetEnemy() );

	if( pPlayer )
	{
		// If the enemy is a player, don't attack from behind!
		if( !pPlayer->FInViewCone( this ) )
		{
			return COND_NONE;
		}
	}

	// Drumroll please!
	// The final check! Is the path from my position to halfway between me
	// and the player clear?
	trace_t tr;
	Vector vecDirToEnemy;

	vecDirToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
	Vector vecHullMin( -16, -16, -16 );
	Vector vecHullMax( 16, 16, 16 );

	// only check half the distance. (the first part of the jump)
	vecDirToEnemy = vecDirToEnemy * 0.5;

	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + vecDirToEnemy, vecHullMin, vecHullMax, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	if( tr.fraction != 1.0 )
	{
		// There's some sort of obstacle pretty much right in front of me.
		return COND_NONE;
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose:
//
//
//-----------------------------------------------------------------------------
void CInfected::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_Infected_CLIMB_LEFT || pEvent->event == AE_Infected_CLIMB_RIGHT )
	{
		if( ++m_iClimbCount % 3 == 0 )
		{
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pLayer2, random->RandomFloat( 100, 150 ), 0.0 );
			ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pLayer2, SOUNDCTRL_CHANGE_VOLUME, envInfectedVolumeClimb, ARRAYSIZE(envInfectedVolumeClimb) );
		}

		return;
	}

	if ( pEvent->event == AE_Infected_LEAP )
	{
		LeapAttack();
		return;
	}
	
	if ( pEvent->event == AE_Infected_GALLOP_LEFT )
	{
		EmitSound( "NPC_Infected.GallopLeft" );
		return;
	}

	if ( pEvent->event == AE_Infected_GALLOP_RIGHT )
	{
		EmitSound( "NPC_Infected.GallopRight" );
		return;
	}
	
	if ( pEvent->event == AE_ZOMBIE_ATTACK_RIGHT )
	{
		Vector right;
		AngleVectors( GetLocalAngles(), NULL, &right, NULL );
		right = right * -50;

		ClawAttack( GetClawAttackRange(), 3, QAngle( -3, -5, -3 ), right, ZOMBIE_BLOOD_RIGHT_HAND );
		return;
	}

	if ( pEvent->event == AE_ZOMBIE_ATTACK_LEFT )
	{
		Vector right;
		AngleVectors( GetLocalAngles(), NULL, &right, NULL );
		right = right * 50;
		ClawAttack( GetClawAttackRange(), 3, QAngle( -3, 5, -3 ), right, ZOMBIE_BLOOD_LEFT_HAND );
		return;
	}

//=============================================================================
#ifdef HL2_EPISODIC

	// Do the leap attack
	if ( pEvent->event == AE_Infected_VEHICLE_LEAP )
	{
		VehicleLeapAttack();
		return;
	}

	// Die while doing an SS in a vehicle
	if ( pEvent->event == AE_Infected_VEHICLE_SS_DIE )
	{
		if ( IsInAVehicle() )
		{
			// Get the vehicle's present speed as a baseline
			Vector vecVelocity = vec3_origin;
			CBaseEntity *pVehicle = m_PassengerBehavior.GetTargetVehicle();
			if ( pVehicle )
			{
				pVehicle->GetVelocity( &vecVelocity, NULL );
			}

			// TODO: We need to make this content driven -- jdw
			Vector vecForward, vecRight, vecUp;
			GetVectors( &vecForward, &vecRight, &vecUp );

			vecVelocity += ( vecForward * -2500.0f ) + ( vecRight * 200.0f ) + ( vecUp * 300 );
			
			// Always kill
			float flDamage = GetMaxHealth() + 10;

			// Take the damage and die
			CTakeDamageInfo info( this, this, vecVelocity * 25.0f, WorldSpaceCenter(), flDamage, (DMG_CRUSH|DMG_VEHICLE) );
			TakeDamage( info );
		}
		return;
	}

#endif // HL2_EPISODIC
//=============================================================================

	BaseClass::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose: Jump at the enemy!! (stole this from the headcrab)
//
//
//-----------------------------------------------------------------------------
void CInfected::LeapAttack( void )
{
	SetGroundEntity( NULL );

	BeginAttackJump();

	LeapAttackSound();

	//
	// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
	//
	UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

	Vector vecJumpDir;
	CBaseEntity *pEnemy = GetEnemy();

	if ( pEnemy )
	{
		Vector vecEnemyPos = pEnemy->WorldSpaceCenter();

		float gravity = sv_gravity.GetFloat();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		//
		// How fast does the zombie need to travel to reach my enemy's eyes given gravity?
		//
		float height = ( vecEnemyPos.z - GetAbsOrigin().z );

		if ( height < 16 )
		{
			height = 16;
		}
		else if ( height > 120 )
		{
			height = 120;
		}
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		//
		// Scale the sideways velocity to get there at the right time
		//
		vecJumpDir = vecEnemyPos - GetAbsOrigin();
		vecJumpDir = vecJumpDir / time;

		//
		// Speed to offset gravity at the desired height.
		//
		vecJumpDir.z = speed;

		//
		// Don't jump too far/fast.
		//
#define CLAMP 1000.0
		float distance = vecJumpDir.Length();
		if ( distance > CLAMP )
		{
			vecJumpDir = vecJumpDir * ( CLAMP / distance );
		}

		// try speeding up a bit.
		SetAbsVelocity( vecJumpDir );
		m_flNextAttack = gpGlobals->curtime + 2;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfected::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_Infected_VERIFY_ATTACK:
		// Simply ensure that the zombie still has a valid melee attack
		if( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		{
			TaskComplete();
		}
		else
		{
			TaskFail("");
		}
		break;

	case TASK_Infected_JUMP_BACK:
		{
			SetActivity( ACT_IDLE );

			SetGroundEntity( NULL );

			BeginAttackJump();

			Vector forward;
			AngleVectors( GetLocalAngles(), &forward );

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			ApplyAbsVelocityImpulse( forward * -200 + Vector( 0, 0, 200 ) );
		}
		break;

	case TASK_Infected_UNSTICK_JUMP:
		{
			SetGroundEntity( NULL );

			// Call begin attack jump. A little bit later if we fail to pathfind, we check
			// this value to see if we just jumped. If so, we assume we've jumped 
			// to someplace that's not pathing friendly, and so must jump again to get out.
			BeginAttackJump();

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			CBaseEntity *pEnemy = GetEnemy();
			Vector vecJumpDir;

			if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN )
			{
				// Jump off the pipe backwards!
				Vector forward;

				GetVectors( &forward, NULL, NULL );

				ApplyAbsVelocityImpulse( forward * -200 );
			}
			else if( pEnemy )
			{
				vecJumpDir = pEnemy->GetLocalOrigin() - GetLocalOrigin();
				VectorNormalize( vecJumpDir );
				vecJumpDir.z = 0;

				ApplyAbsVelocityImpulse( vecJumpDir * 300 + Vector( 0, 0, 200 ) );
			}
			else
			{
				DevMsg("UNHANDLED CASE! Stuck Fast Zombie with no enemy!\n");
			}
		}
		break;

	case TASK_WAIT_FOR_MOVEMENT:
		// If we're waiting for movement, that means that pathfinding succeeded, and
		// we're about to be moving. So we aren't stuck. So clear this flag. 
		m_fJustJumped = false;

		BaseClass::StartTask( pTask );
		break;

	case TASK_FACE_ENEMY:
		{
			// We don't use the base class implementation of this, because GetTurnActivity
			// stomps our landing scrabble animations (sjb)
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTarget( flEnemyLKP );
		}
		break;

	case TASK_Infected_LAND_RECOVER:
		{
			// Set the ideal yaw
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTarget( flEnemyLKP );

			// figure out which way to turn.
			float flDeltaYaw = GetMotor()->DeltaIdealYaw();

			if( flDeltaYaw < 0 )
			{
				SetIdealActivity( (Activity)ACT_Infected_LAND_RIGHT );
			}
			else
			{
				SetIdealActivity( (Activity)ACT_Infected_LAND_LEFT );
			}


			TaskComplete();
		}
		break;

	case TASK_RANGE_ATTACK1:

		// Make melee attacks impossible until we land!
		m_flNextMeleeAttack = gpGlobals->curtime + 60;

		SetTouch( &CInfected::LeapAttackTouch );
		break;

	case TASK_Infected_DO_ATTACK:
		SetActivity( (Activity)ACT_Infected_LEAP_SOAR );
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfected::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_Infected_JUMP_BACK:
	case TASK_Infected_UNSTICK_JUMP:
		if( GetFlags() & FL_ONGROUND )
		{
			TaskComplete();
		}
		break;

	case TASK_RANGE_ATTACK1:
		if( ( GetFlags() & FL_ONGROUND ) || ( m_pfnTouch == NULL ) )
		{
			// All done when you touch the ground, or if our touch function has somehow cleared.
			TaskComplete();

			// Allow melee attacks again.
			m_flNextMeleeAttack = gpGlobals->curtime + 0.5;
			return;
		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}


//---------------------------------------------------------
//---------------------------------------------------------
int CInfected::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		{
			// Scream right now, cause in half a second, we're gonna jump!!
	
			if( !m_fHasScreamed )
			{
				// Only play that over-the-top attack scream once per combat state.
				EmitSound( "NPC_Infected.Scream" );
				m_fHasScreamed = true;
			}
			else
			{
				EmitSound( "NPC_Infected.RangeAttack" );
			}

			return SCHED_Infected_RANGE_ATTACK1;
		}
		break;

	case SCHED_MELEE_ATTACK1:
		/*if ( m_fIsTorso == true )
		{
			return SCHED_Infected_TORSO_MELEE_ATTACK1;
		}
		else
		{*/
			return SCHED_Infected_MELEE_ATTACK1;
		//}
		break;

	case SCHED_Infected_UNSTICK_JUMP:
		if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN || GetActivity() == ACT_CLIMB_DISMOUNT )
		{
			return SCHED_Infected_CLIMBING_UNSTICK_JUMP;
		}
		else
		{
			return SCHED_Infected_UNSTICK_JUMP;
		}
		break;
	case SCHED_MOVE_TO_WEAPON_RANGE:
		{
			float flZDist = fabs( GetEnemy()->GetLocalOrigin().z - GetLocalOrigin().z );
			if ( flZDist > Infected_MAXLEAP_Z )
				return SCHED_CHASE_ENEMY;
			else // fall through to default
				return BaseClass::TranslateSchedule( scheduleType );
			break;
		}

	default:
		return BaseClass::TranslateSchedule( scheduleType );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Activity CInfected::NPC_TranslateActivity( Activity baseAct )
{
	if ( baseAct == ACT_CLIMB_DOWN )
		return ACT_CLIMB_UP;
	
	return BaseClass::NPC_TranslateActivity( baseAct );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CInfected::LeapAttackTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() )
	{
		// Touching a trigger or something.
		return;
	}

	// Stop the zombie and knock the player back
	Vector vecNewVelocity( 0, 0, GetAbsVelocity().z );
	SetAbsVelocity( vecNewVelocity );

	Vector forward;
	AngleVectors( GetLocalAngles(), &forward );
	QAngle qaPunch( 15, random->RandomInt(-5,5), random->RandomInt(-5,5) );
	
	ClawAttack( GetClawAttackRange(), 5, qaPunch, forward * 500, ZOMBIE_BLOOD_BOTH_HANDS );

	SetTouch( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Lets us know if we touch the player while we're climbing.
//-----------------------------------------------------------------------------
void CInfected::ClimbTouch( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() )
	{
		// If I hit the player, shove him aside.
		Vector vecDir = pOther->WorldSpaceCenter() - WorldSpaceCenter();
		vecDir.z = 0.0; // planar
		VectorNormalize( vecDir );

		if( IsXbox() )
		{
			vecDir *= 400.0f;
		}
		else
		{
			vecDir *= 200.0f;
		}

		pOther->VelocityPunch( vecDir );

		if ( GetActivity() != ACT_CLIMB_DISMOUNT || 
			 ( pOther->GetGroundEntity() == NULL &&
			   GetNavigator()->IsGoalActive() &&
			   pOther->GetAbsOrigin().z - GetNavigator()->GetCurWaypointPos().z < -1.0 ) )
		{
			SetCondition( COND_Infected_CLIMB_TOUCH );
		}

		SetTouch( NULL );
	}
	else if ( dynamic_cast<CPhysicsProp *>(pOther) )
	{
		NPCPhysics_CreateSolver( this, pOther, true, 5.0 );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Shuts down our looping sounds.
//-----------------------------------------------------------------------------
void CInfected::StopLoopingSounds( void )
{
	if ( m_pMoanSound )
	{
		ENVELOPE_CONTROLLER.SoundDestroy( m_pMoanSound );
		m_pMoanSound = NULL;
	}

	if ( m_pLayer2 )
	{
		ENVELOPE_CONTROLLER.SoundDestroy( m_pLayer2 );
		m_pLayer2 = NULL;
	}

	BaseClass::StopLoopingSounds();
}


//-----------------------------------------------------------------------------
// Purpose: Fast zombie cannot range attack when he's a torso!
//-----------------------------------------------------------------------------
/*void CInfected::BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce )
{
	CapabilitiesRemove( bits_CAP_INNATE_RANGE_ATTACK1 );
	CapabilitiesRemove( bits_CAP_MOVE_JUMP );
	CapabilitiesRemove( bits_CAP_MOVE_CLIMB );

	//ReleaseHeadcrab( EyePosition(), vecLegsForce * 0.5, true, true, true );

	BaseClass::BecomeTorso( vecTorsoForce, vecLegsForce );
}*/

//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CInfected::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	const float MAX_JUMP_RISE		= 220.0f;
	const float MAX_JUMP_DISTANCE	= 512.0f;
	const float MAX_JUMP_DROP		= 384.0f;

	if ( BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE ) )
	{
		// Hang onto the jump distance. The AI is going to want it.
		m_flJumpDist = (startPos - endPos).Length();

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CInfected::MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost )
{
	float delta = vecEnd.z - vecStart.z;

	float multiplier = 1;
	if ( moveType == bits_CAP_MOVE_JUMP )
	{
		multiplier = ( delta < 0 ) ? 0.5 : 1.5;
	}
	else if ( moveType == bits_CAP_MOVE_CLIMB )
	{
		multiplier = ( delta > 0 ) ? 0.5 : 4.0;
	}

	*pCost *= multiplier;

	return ( multiplier != 1 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CInfected::ShouldFailNav( bool bMovementFailed )
{
	if ( !BaseClass::ShouldFailNav( bMovementFailed ) )
	{
		DevMsg( 2, "Fast zombie in scripted sequence probably hit bad node configuration at %s\n", VecToString( GetAbsOrigin() ) );
		
		if ( GetNavigator()->GetPath()->CurWaypointNavType() == NAV_JUMP && GetNavigator()->RefindPathToGoal( false ) )
		{
			return false;
		}
		DevMsg( 2, "Fast zombie failed to get to scripted sequence\n" );
	}

	return true;
}


//---------------------------------------------------------
// Purpose: Notifier that lets us know when the fast
//			zombie has hit the apex of a navigational jump.
//---------------------------------------------------------
void CInfected::OnNavJumpHitApex( void )
{
	m_fHitApex = true;	// stop subsequent notifications
}

//---------------------------------------------------------
// Purpose: Overridden to detect when the zombie goes into
//			and out of his climb state and his navigation
//			jump state.
//---------------------------------------------------------
void CInfected::OnChangeActivity( Activity NewActivity )
{
	if ( NewActivity == ACT_Infected_FRENZY )
	{
		// Scream!!!!
		EmitSound( "NPC_Infected.Frenzy" );
		SetPlaybackRate( random->RandomFloat( .9, 1.1 ) );	
	}

	if( NewActivity == ACT_JUMP )
	{
		BeginNavJump();
	}
	else if( GetActivity() == ACT_JUMP )
	{
		EndNavJump();
	}

	if ( NewActivity == ACT_LAND )
	{
		m_flNextAttack = gpGlobals->curtime + 1.0;
	}

	if ( NewActivity == ACT_GLIDE )
	{
		// Started a jump.
		BeginNavJump();
	}
	else if ( GetActivity() == ACT_GLIDE )
	{
		// Landed a jump
		EndNavJump();

		if ( m_pMoanSound )
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, Infected_MIN_PITCH, 0.3 );
	}

	if ( NewActivity == ACT_CLIMB_UP )
	{
		// Started a climb!
		if ( m_pMoanSound )
			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 0.0, 0.2 );

		SetTouch( &CInfected::ClimbTouch );
	}
	else if ( GetActivity() == ACT_CLIMB_DISMOUNT || ( GetActivity() == ACT_CLIMB_UP && NewActivity != ACT_CLIMB_DISMOUNT ) )
	{
		// Ended a climb
		if ( m_pMoanSound )
			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 1.0, 0.2 );

		SetTouch( NULL );
	}

	BaseClass::OnChangeActivity( NewActivity );
}


//=========================================================
// 
//=========================================================
int CInfected::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( m_fJustJumped )
	{
		// Assume we failed cause we jumped to a bad place.
		m_fJustJumped = false;
		return SCHED_Infected_UNSTICK_JUMP;
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//=========================================================
// Purpose: Do some record keeping for jumps made for 
//			navigational purposes (i.e., not attack jumps)
//=========================================================
void CInfected::BeginNavJump( void )
{
	m_fIsNavJumping = true;
	m_fHitApex = false;

	ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pLayer2, SOUNDCTRL_CHANGE_VOLUME, envInfectedVolumeJump, ARRAYSIZE(envInfectedVolumeJump) );
}

//=========================================================
// 
//=========================================================
void CInfected::EndNavJump( void )
{
	m_fIsNavJumping = false;
	m_fHitApex = false;
}

//=========================================================
// 
//=========================================================
void CInfected::BeginAttackJump( void )
{
	// Set this to true. A little bit later if we fail to pathfind, we check
	// this value to see if we just jumped. If so, we assume we've jumped 
	// to someplace that's not pathing friendly, and so must jump again to get out.
	m_fJustJumped = true;

	m_flJumpStartAltitude = GetLocalOrigin().z;
}

//=========================================================
// 
//=========================================================
void CInfected::EndAttackJump( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfected::BuildScheduleTestBits( void )
{
	// FIXME: This is probably the desired call to make, but it opts into an untested base class path, we'll need to
	//		  revisit this and figure out if we want that. -- jdw
	// BaseClass::BuildScheduleTestBits();
	//
	// For now, make sure our active behavior gets a chance to add its own bits
	if ( GetRunningBehavior() )
		GetRunningBehavior()->BridgeBuildScheduleTestBits(); 

#ifdef HL2_EPISODIC
	SetCustomInterruptCondition( COND_PROVOKED );
#endif	// HL2_EPISODIC

	// Any schedule that makes us climb should break if we touch player
	if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN || GetActivity() == ACT_CLIMB_DISMOUNT)
	{
		SetCustomInterruptCondition( COND_Infected_CLIMB_TOUCH );
	}
	else
	{
		ClearCustomInterruptCondition( COND_Infected_CLIMB_TOUCH );
	}
}

//=========================================================
// 
//=========================================================
void CInfected::OnStateChange( NPC_STATE OldState, NPC_STATE NewState )
{
	if( NewState == NPC_STATE_COMBAT )
	{
		SetAngrySoundState();
	}
	else if( (m_pMoanSound) && ( NewState == NPC_STATE_IDLE || NewState == NPC_STATE_ALERT ) ) ///!!!HACKHACK - sjb
	{
		// Don't make this sound while we're slumped
		if ( IsSlumped() == false )
		{
			// Set it up so that if the zombie goes into combat state sometime down the road
			// that he'll be able to scream.
			m_fHasScreamed = false;

			SetIdleSoundState();
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CInfected::Event_Killed( const CTakeDamageInfo &info )
{
	// Shut up my screaming sounds.
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Infected.NoSound" );

	CTakeDamageInfo dInfo = info;

	if ((info.GetDamageType() & (DMG_BLAST)) && (cvar->FindVar("aa_more_gibs")->GetInt() == 1))	// L1ght 15 : Gibs time!!!
	{
		Vector vecLegsForce;
		vecLegsForce.x = random->RandomFloat( -400, 400 );
		vecLegsForce.y = random->RandomFloat( -400, 400 );
		vecLegsForce.z = random->RandomFloat( 0, 250 );
		Vector vecDamageDir = info.GetDamageForce();

		UTIL_BloodSpray( WorldSpaceCenter(), vecDamageDir, BLOOD_COLOR_RED, 18, FX_BLOODSPRAY_ALL );
		SetModel( "models/humans/Charple03.mdl" );

		//CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/Gibs/HGIBS.mdl", 5 );
		CGib::SpawnSpecificGibs( this, 1, 150, 500, "models/Gibs/HGIBS_scapula.mdl", 5 );
		CGib::SpawnSpecificGibs( this, 1, 150, 500, "models/Gibs/HGIBS_spine.mdl", 5 );
		CGib::SpawnSpecificGibs( this, 1, 150, 500, "models/Gibs/HGIBS_rib.mdl", 5 );
	}

#if 0

	// Become a server-side ragdoll and create a constraint at the hand
	if ( m_PassengerBehavior.GetPassengerState() == PASSENGER_STATE_INSIDE )
	{
		IPhysicsObject *pVehiclePhys = m_PassengerBehavior.GetTargetVehicle()->GetServerVehicle()->GetVehicleEnt()->VPhysicsGetObject();
		CBaseAnimating *pVehicleAnimating = m_PassengerBehavior.GetTargetVehicle()->GetServerVehicle()->GetVehicleEnt()->GetBaseAnimating();
		int nRightHandBone = 31;//GetBaseAnimating()->LookupBone( "ValveBiped.Bip01_R_Finger2" );
		Vector vecRightHandPos;
		QAngle vecRightHandAngle;
		GetAttachment( LookupAttachment( "Blood_Right" ), vecRightHandPos, vecRightHandAngle );
		//CTakeDamageInfo dInfo( GetEnemy(), GetEnemy(), RandomVector( -200, 200 ), WorldSpaceCenter(), 50.0f, DMG_CRUSH );
		dInfo.SetDamageType( info.GetDamageType() | DMG_REMOVENORAGDOLL );
		dInfo.ScaleDamageForce( 10.0f );
		CBaseEntity *pRagdoll = CreateServerRagdoll( GetBaseAnimating(), 0, info, COLLISION_GROUP_DEBRIS );

		/*
		GetBaseAnimating()->GetBonePosition( nRightHandBone, vecRightHandPos, vecRightHandAngle );

		CBaseEntity *pRagdoll = CreateServerRagdollAttached(	GetBaseAnimating(), 
																vec3_origin, 
																-1, 
																COLLISION_GROUP_DEBRIS, 
																pVehiclePhys,
																pVehicleAnimating, 
																0, 
																vecRightHandPos,
																nRightHandBone,	
																vec3_origin );*/

	}
#endif

	BaseClass::Event_Killed( dInfo );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*bool CInfected::ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold )
{
	/*if( m_fIsTorso )
	{
		// Already split.
		return false;
	}*/

	// Break in half IF:
	// 
	// Take half or more of max health in DMG_BLAST
	/*if( (info.GetDamageType() & DMG_BLAST) && m_iHealth <= 0 )
	{
		return true;
	}*/
/*
	return false;
}*/

//=============================================================================
#ifdef HL2_EPISODIC

//-----------------------------------------------------------------------------
// Purpose: Add the passenger behavior to our repertoire
//-----------------------------------------------------------------------------
bool CInfected::CreateBehaviors( void )
{
	AddBehavior( &m_PassengerBehavior );

	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
// Purpose: Get on the vehicle!
//-----------------------------------------------------------------------------
void CInfected::InputAttachToVehicle( inputdata_t &inputdata )
{
	// Interrupt us
	SetCondition( COND_PROVOKED );

	// Find the target vehicle
	CBaseEntity *pEntity = FindNamedEntity( inputdata.value.String() );
	CPropJeepEpisodic *pVehicle = dynamic_cast<CPropJeepEpisodic *>(pEntity);

	// Get in the car if it's valid
	if ( pVehicle && CanEnterVehicle( pVehicle ) )
	{
		// Set her into a "passenger" behavior
		m_PassengerBehavior.Enable( pVehicle );
		m_PassengerBehavior.AttachToVehicle();
	}

	RemoveSpawnFlags( SF_NPC_GAG );
}

//-----------------------------------------------------------------------------
// Purpose: Passed along from the vehicle's callback list
//-----------------------------------------------------------------------------
void CInfected::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	// Only do the override while riding on a vehicle
	if ( m_PassengerBehavior.CanSelectSchedule() && m_PassengerBehavior.GetPassengerState() != PASSENGER_STATE_OUTSIDE )
	{
		int damageType = 0;
		float flDamage = CalculatePhysicsImpactDamage( index, pEvent, gZombiePassengerImpactDamageTable, 1.0, true, damageType );

		if ( flDamage > 0  )
		{
			Vector damagePos;
			pEvent->pInternalData->GetContactPoint( damagePos );
			Vector damageForce = pEvent->postVelocity[index] * pEvent->pObjects[index]->GetMass();
			CTakeDamageInfo info( this, this, damageForce, damagePos, flDamage, (damageType|DMG_VEHICLE) );
			TakeDamage( info );
		}
		return;
	}

	BaseClass::VPhysicsCollision( index, pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: FIXME: Fold this into LeapAttack using different jump targets!
//-----------------------------------------------------------------------------
void CInfected::VehicleLeapAttack( void )
{
	CBaseEntity *pEnemy = GetEnemy();
	if ( pEnemy == NULL )
		return;

	Vector vecEnemyPos;
	UTIL_PredictedPosition( pEnemy, 1.0f, &vecEnemyPos );

	// Move
	SetGroundEntity( NULL );
	BeginAttackJump();
	LeapAttackSound();

	// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
	UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

	// FIXME: This should be the exact position we'll enter at, but this approximates it generally
	//vecEnemyPos[2] += 16;

	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	Vector vecJumpDir = VecCheckToss( this, GetAbsOrigin(), vecEnemyPos, 0.1f, 1.0f, false, &vecMins, &vecMaxs );

	SetAbsVelocity( vecJumpDir );
	m_flNextAttack = gpGlobals->curtime + 2.0f;
	SetTouch( &CInfected::VehicleLeapAttackTouch );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CInfected::CanEnterVehicle( CPropJeepEpisodic *pVehicle )
{
	if ( pVehicle == NULL )
		return false;

	return pVehicle->NPC_CanEnterVehicle( this, false );
}

//-----------------------------------------------------------------------------
// Purpose: FIXME: Move into behavior?
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CInfected::VehicleLeapAttackTouch( CBaseEntity *pOther )
{
	if ( pOther->GetServerVehicle() )
	{
		m_PassengerBehavior.AttachToVehicle();

		// HACK: Stop us cold
		SetLocalVelocity( vec3_origin );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determine whether we're in a vehicle or not
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CInfected::IsInAVehicle( void )
{
	// Must be active and getting in/out of vehicle
	if ( m_PassengerBehavior.IsEnabled() && m_PassengerBehavior.GetPassengerState() != PASSENGER_STATE_OUTSIDE )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Override our efficiency so that we don't jitter when we're in the middle
//			of our enter/exit animations.
// Input  : bInPVS - Whether we're in the PVS or not
//-----------------------------------------------------------------------------
void CInfected::UpdateEfficiency( bool bInPVS )
{ 
	// If we're transitioning and in the PVS, we override our efficiency
	if ( IsInAVehicle() && bInPVS )
	{
		PassengerState_e nState = m_PassengerBehavior.GetPassengerState();
		if ( nState == PASSENGER_STATE_ENTERING || nState == PASSENGER_STATE_EXITING )
		{
			SetEfficiency( AIE_NORMAL );
			return;
		}
	}

	// Do the default behavior
	BaseClass::UpdateEfficiency( bInPVS );
}

#endif	// HL2_EPISODIC
//=============================================================================

//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_Infected, CInfected )

	DECLARE_ACTIVITY( ACT_Infected_LEAP_SOAR )
	DECLARE_ACTIVITY( ACT_Infected_LEAP_STRIKE )
	DECLARE_ACTIVITY( ACT_Infected_LAND_RIGHT )
	DECLARE_ACTIVITY( ACT_Infected_LAND_LEFT )
	DECLARE_ACTIVITY( ACT_Infected_FRENZY )
	DECLARE_ACTIVITY( ACT_Infected_BIG_SLASH )
	
	DECLARE_TASK( TASK_Infected_DO_ATTACK )
	DECLARE_TASK( TASK_Infected_LAND_RECOVER )
	DECLARE_TASK( TASK_Infected_UNSTICK_JUMP )
	DECLARE_TASK( TASK_Infected_JUMP_BACK )
	DECLARE_TASK( TASK_Infected_VERIFY_ATTACK )

	DECLARE_CONDITION( COND_Infected_CLIMB_TOUCH )

	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_Infected_LEAP )
	DECLARE_ANIMEVENT( AE_Infected_GALLOP_LEFT )
	DECLARE_ANIMEVENT( AE_Infected_GALLOP_RIGHT )
	DECLARE_ANIMEVENT( AE_Infected_CLIMB_LEFT )
	DECLARE_ANIMEVENT( AE_Infected_CLIMB_RIGHT )

#ifdef HL2_EPISODIC
	// FIXME: Move!
	DECLARE_ANIMEVENT( AE_PASSENGER_PHYSICS_PUSH1 )
	DECLARE_ANIMEVENT( AE_Infected_VEHICLE_LEAP )
	DECLARE_ANIMEVENT( AE_Infected_VEHICLE_SS_DIE )
#endif	// HL2_EPISODIC

	//=========================================================
	// 
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_Infected_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_RANGE_ATTACK1"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_Infected_LEAP_STRIKE"
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_WAIT						0.1"
		"		TASK_Infected_LAND_RECOVER	0" // essentially just figure out which way to turn.
		"		TASK_FACE_ENEMY					0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	// I have landed somewhere that's pathfinding-unfriendly
	// just try to jump out.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_Infected_UNSTICK_JUMP,

		"	Tasks"
		"		TASK_Infected_UNSTICK_JUMP	0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_Infected_CLIMBING_UNSTICK_JUMP,

		"	Tasks"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
		"		TASK_Infected_UNSTICK_JUMP	0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	// > Melee_Attack1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_Infected_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_Infected_FRENZY"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_Infected_VERIFY_ATTACK	0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_Infected_BIG_SLASH"

		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
	);

	//=========================================================
	// > Melee_Attack1
	//=========================================================
	DEFINE_SCHEDULE
		(
		SCHED_Infected_TORSO_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_Infected_VERIFY_ATTACK	0"

		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		);

AI_END_CUSTOM_NPC()


