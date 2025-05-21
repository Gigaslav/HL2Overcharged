//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:	Charger from HL Alpha remake
//
//=============================================================================//

#include "cbase.h"
#include "ai_hint.h"
#include "ai_localnavigator.h"
#include "ai_memory.h"
#include "ai_moveprobe.h"
#include "npcevent.h"
#include "IEffects.h"
#include "ndebugoverlay.h"
#include "soundent.h"
#include "soundenvelope.h"
#include "ai_squad.h"
#include "ai_network.h"
#include "ai_pathfinder.h"
#include "ai_navigator.h"
#include "ai_senses.h"
#include "npc_rollermine.h"
#include "ai_blended_movement.h"
#include "physics_prop_ragdoll.h"
#include "iservervehicle.h"
#include "player_pickup.h"
#include "props.h"
#include "decals.h"
#include "prop_combine_ball.h"
#include "eventqueue.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "grenade_spit.h"	// spit added
//#include "grenade_energy.h"
#include "energy_wave.h"	// energy wave added
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

inline void TraceHull_SkipPhysics( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr, float minMass );

ConVar	g_debug_charger( "g_debug_charger", "0" );
ConVar	sk_charger_dmg_charge( "sk_charger_dmg_charge", "0" );
ConVar	sk_charger_dmg_shove( "sk_charger_dmg_shove", "0" );

ConVar  sk_charger_spit_speed_regular("sk_charger_spit_speed_regular", "0", FCVAR_NONE, "Regular speed at which an charger spit grenade travels.");
ConVar  sk_charger_spit_speed_special("sk_charger_spit_speed_special", "0", FCVAR_NONE, "Special speed at which an charger spit grenade travels.");
ConVar  sk_charger_dmg_waveroar("sk_charger_dmg_waveroar", "0", FCVAR_NONE, "Roar wave damage.");

// When enabled, add code to have the antlion bleed profusely as it is badly injured.
#define charger_BLOOD_EFFECTS 2
ConVar	g_charger_hemorrhage( "g_charger_hemorrhage", "1", FCVAR_NONE, "If 1, charger will emit a bleeding particle effect when wounded." );

// Spawnflags 
#define	SF_charger_SERVERSIDE_RAGDOLL	( 1 << 16 )
#define SF_charger_INSIDE_FOOTSTEPS	( 1 << 17 )

#define	ENVELOPE_CONTROLLER		(CSoundEnvelopeController::GetController())

#define	charger_MODEL		"models/charger_xen.mdl"

#define	MIN_BLAST_DAMAGE		25.0f
#define MIN_CRUSH_DAMAGE		20.0f

//==================================================
//
// charger
//
//==================================================

#define charger_MAX_OBJECTS				128
#define	charger_MIN_OBJECT_MASS			8
#define	charger_MAX_OBJECT_MASS			750
#define	charger_FARTHEST_PHYSICS_OBJECT	350
#define charger_OBJECTFINDING_FOV			DOT_45DEGREE // 1/sqrt(2)

//Melee definitions

#define	charger_MELEE1_RANGE		86.0f
#define	charger_MELEE1_CONE		0.7f

// Sight
#define	charger_FOV_NORMAL			-0.4f

#define	charger_CHARGE_MIN			86
#define	charger_CHARGE_MAX			5048 //2048

ConVar	sk_charger_health( "sk_charger_health", "0" );

int	g_interactionchargerFoundPhysicsObject = 0;	// We're moving to a physics object to shove it, don't all choose the same object
int	g_interactionchargerShovedPhysicsObject = 0;	// We've punted an object, it is now clear to be chosen by others

//==================================================
// chargerSchedules
//==================================================

enum
{
	SCHED_charger_CHARGE = LAST_SHARED_SCHEDULE,
	SCHED_charger_CHARGE_CRASH,
	SCHED_charger_CHARGE_CANCEL,
	SCHED_charger_PHYSICS_ATTACK,
	SCHED_charger_PHYSICS_DAMAGE_HEAVY,
	SCHED_charger_CHARGE_TARGET,
	SCHED_charger_FIND_CHARGE_POSITION,
	SCHED_charger_MELEE_ATTACK1,
	SCHED_charger_PATROL_RUN,
	SCHED_charger_ROAR,
	SCHED_charger_CHASE_ENEMY_TOLERANCE,
	SCHED_FORCE_charger_PHYSICS_ATTACK,
	SCHED_charger_CANT_ATTACK,
	SCHED_charger_TAKE_COVER_FROM_ENEMY,
	SCHED_charger_CHASE_ENEMY
};


//==================================================
// chargerTasks
//==================================================

enum
{
	TASK_charger_CHARGE = LAST_SHARED_TASK,
	TASK_charger_GET_PATH_TO_PHYSOBJECT,
	TASK_charger_SHOVE_PHYSOBJECT,
	TASK_charger_SET_FLINCH_ACTIVITY,
	TASK_charger_GET_PATH_TO_CHARGE_POSITION,
	TASK_charger_GET_PATH_TO_NEAREST_NODE,
	TASK_charger_GET_CHASE_PATH_ENEMY_TOLERANCE,
	TASK_charger_FIND_PHYSOBJECT,
};

//==================================================
// chargerConditions
//==================================================

enum
{
	COND_charger_PHYSICS_TARGET = LAST_SHARED_CONDITION,
	COND_charger_PHYSICS_TARGET_INVALID,
	COND_charger_HAS_CHARGE_TARGET,
	COND_charger_CAN_CHARGE
};

enum
{	
	SQUAD_SLOT_charger_CHARGE = LAST_SHARED_SQUADSLOT,
};

//==================================================
// charger Activities
//==================================================

Activity ACT_charger_SEARCH;
Activity ACT_charger_BARK;
Activity ACT_charger_SHOVE_PHYSOBJECT;
Activity ACT_charger_FLINCH_LIGHT;
Activity ACT_charger_ROAR;
Activity ACT_charger_RUN_HURT;

// Flinches
Activity ACT_charger_PHYSHIT_FR;
Activity ACT_charger_PHYSHIT_FL;
Activity ACT_charger_PHYSHIT_RR;
Activity ACT_charger_PHYSHIT_RL;

// Charge
Activity ACT_charger_CHARGE_START;
Activity ACT_charger_CHARGE_CANCEL;
Activity ACT_charger_CHARGE_RUN;
Activity ACT_charger_CHARGE_CRASH;
Activity ACT_charger_CHARGE_STOP;
Activity ACT_charger_CHARGE_HIT;
//Activity ACT_charger_CHARGE_ANTICIPATION;

// Anim events
int AE_charger_CHARGE_HIT;
int AE_charger_SHOVE_PHYSOBJECT;
int AE_charger_SHOVE;
int AE_charger_FOOTSTEP_LIGHT;
int AE_charger_FOOTSTEP_HEAVY;
int AE_charger_VOICE_GROWL;
int AE_charger_VOICE_BARK;
int AE_charger_VOICE_PAIN;
int AE_charger_VOICE_SQUEEZE;
int AE_charger_VOICE_SCRATCH;
int AE_charger_VOICE_GRUNT;
int AE_charger_VOICE_ROAR;
//int AE_charger_FOOTPRINT_LEFT;
//int AE_charger_FOOTPRINT_RIGHT;
//int AE_charger_FOOTPRINT_BACK;

struct PhysicsObjectCriteria_t
{
	CBaseEntity *pTarget;
	Vector	vecCenter;		// Center point to look around
	float	flRadius;		// Radius to search within
	float	flTargetCone;
	bool	bPreferObjectsAlongTargetVector;	// Prefer objects that we can strike easily as we move towards our target
	float	flNearRadius;						// If we won't hit the player with the object, but get this close, throw anyway
};

#define MAX_FAILED_PHYSOBJECTS 8

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_charger : public CAI_BlendedNPC
{
public:
	DECLARE_CLASS( CNPC_charger, CAI_BlendedNPC );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNPC_charger( void );

	Class_T	Classify( void ) { return CLASS_ANTLION; }
	virtual int		GetSoundInterests( void ) { return (SOUND_WORLD|SOUND_COMBAT|SOUND_PLAYER|SOUND_DANGER); }
	virtual bool	QueryHearSound( CSound *pSound );

	const impactdamagetable_t &GetPhysicsImpactDamageTable( void );

	virtual int		MeleeAttack1Conditions( float flDot, float flDist );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	virtual int		TranslateSchedule( int scheduleType );
	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void	DeathSound( const CTakeDamageInfo &info );
	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		SelectSchedule( void );

	virtual float GetAutoAimRadius() { return 36.0f; }
	
	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	HandleAnimEvent( animevent_t *pEvent );
	virtual void	UpdateEfficiency( bool bInPVS )	{ SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); SetMoveEfficiency( AIME_NORMAL ); }
	virtual void	PrescheduleThink( void );
	virtual void	GatherConditions( void );
	virtual void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	virtual void	StopLoopingSounds();
	virtual bool	HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender );

	CEnergyWave*	m_pEnergyWaveChar;	//added
	void		WaveRoarAttack(void);	//added
	void		Touch(CBaseEntity *pOther);	//added
	
	// footprint
	//void SnowFootPrint( void );
	//void SnowFootPrint( bool IsLeft );
	//int FootPrintDecal( void ){ int fpdid = decalsystem->GetDecalIndexForName( "FootPrintcharger" ); return fpdid; };

	// Input handlers.
	void	InputSetShoveTarget( inputdata_t &inputdata );
	void	InputSetChargeTarget( inputdata_t &inputdata );
	void	InputClearChargeTarget( inputdata_t &inputdata );
	void	InputRagdoll( inputdata_t &inputdata );
	void	InputEnableBark( inputdata_t &inputdata );
	void	InputDisableBark( inputdata_t &inputdata );
	void	InputEnablePreferPhysicsAttack( inputdata_t &inputdata );
	void	InputDisablePreferPhysicsAttack( inputdata_t &inputdata );

	virtual bool	IsLightDamage( const CTakeDamageInfo &info );
	virtual bool	IsHeavyDamage( const CTakeDamageInfo &info );
	virtual bool	OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual bool	BecomeRagdollOnClient( const Vector &force );
	virtual void UpdateOnRemove( void );
	virtual bool		IsUnreachable( CBaseEntity* pEntity );			// Is entity is unreachable?

	virtual float	MaxYawSpeed( void );
	virtual bool	OverrideMove( float flInterval );
	virtual bool	CanBecomeRagdoll( void );

	virtual bool	ShouldProbeCollideAgainstEntity( CBaseEntity *pEntity );

	virtual Activity	NPC_TranslateActivity( Activity baseAct );

	//---------------------------------
	// Navigation & Movement -- prevent stopping paths for the guard
	//---------------------------------
	class CNavigator : public CAI_ComponentWithOuter<CNPC_charger, CAI_Navigator>
	{
		typedef CAI_ComponentWithOuter<CNPC_charger, CAI_Navigator> BaseClass;
	public:
		CNavigator( CNPC_charger *pOuter )
			:	BaseClass( pOuter )
		{
		}

		bool GetStoppingPath( CAI_WaypointList *pClippedWaypoints );
	};
	CAI_Navigator *	CreateNavigator()	{ return new CNavigator( this );	}

	DEFINE_CUSTOM_AI;

private:

	inline bool CanStandAtPoint( const Vector &vecPos, Vector *pOut );
	bool	RememberFailedPhysicsTarget( CBaseEntity *pTarget );
	void	GetPhysicsShoveDir( CBaseEntity *pObject, float flMass, Vector *pOut );
	void	CreateGlow( CSprite **pSprite, const char *pAttachName );
	void	Footstep( bool bHeavy );
	int		SelectCombatSchedule( void );
	int		SelectUnreachableSchedule( void );
					
	void	ChargeLookAhead( void );
	bool	EnemyIsRightInFrontOfMe( CBaseEntity **pEntity );
	bool	HandleChargeImpact( Vector vecImpact, CBaseEntity *pEntity );
	bool	ShouldCharge( const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel );
	bool	ShouldWatchEnemy( void );
					
	void	ImpactShock( const Vector &origin, float radius, float magnitude, CBaseEntity *pIgnored = NULL );
	void	BuildScheduleTestBits( void );
	void	Shove( void );
	void	SpitFire(void);	// BJ: Added
	virtual bool GetSpitVectorRegular(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut);	// added
	virtual bool GetSpitVectorUnder(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut);	// added
	Vector	m_vecSaveSpitVelocity;
	bool bChargeWaveRoarActivate;
	bool bChargeWaveOnce;


	void	FoundEnemy( void );
	void	LostEnemy( void );
	void	UpdateHead( void );
	void	UpdatePhysicsTarget( bool bPreferObjectsAlongTargetVector, float flRadius = charger_FARTHEST_PHYSICS_OBJECT );
	void	MaintainPhysicsTarget( void );
	void	ChargeDamage( CBaseEntity *pTarget );
	void	StartSounds( void );
	void	SetHeavyDamageAnim( const Vector &vecSource );
	float	ChargeSteer( void );
	CBaseEntity *FindPhysicsObjectTarget( const PhysicsObjectCriteria_t &criteria );
	Vector	GetPhysicsHitPosition( CBaseEntity *pObject, CBaseEntity *pTarget, Vector *vecTrajectory, float *flClearDistance );
	bool	CanStandAtShoveTarget( CBaseEntity *pShoveObject, CBaseEntity *pTarget, Vector *pOut );
	CBaseEntity *GetNextShoveTarget( CBaseEntity *pLastEntity, AISightIter_t &iter );

	int				m_nFlinchActivity;

	bool			m_bStopped;
	bool			m_bBarkEnabled;
							
	float			m_flSearchNoiseTime;
	float			m_flAngerNoiseTime;
	float			m_flBreathTime;
	float			m_flChargeTime;
	float			m_flPhysicsCheckTime;
	float			m_flNextHeavyFlinchTime;
	float			m_flNextRoarTime;
	int				m_iChargeMisses;
	bool			m_bDecidedNotToStop;
	bool			m_bPreferPhysicsAttack;
					
	Vector			m_vecPhysicsTargetStartPos;
	Vector			m_vecPhysicsHitPosition;
					
	EHANDLE			m_hShoveTarget;
	EHANDLE			m_hChargeTarget;
	EHANDLE			m_hChargeTargetPosition;
	EHANDLE			m_hOldTarget;
	EHANDLE			m_hPhysicsTarget;
					
	CUtlVectorFixed<EHANDLE, MAX_FAILED_PHYSOBJECTS>		m_FailedPhysicsTargets;

	CSoundPatch		*m_pGrowlHighSound;
	CSoundPatch		*m_pGrowlLowSound;
	CSoundPatch		*m_pGrowlIdleSound;
	CSoundPatch		*m_pBreathSound;
	CSoundPatch		*m_pConfusedSound;

	string_t		m_iszPhysicsPropClass;
	string_t		m_strShoveTargets;

	CSprite			*m_hCaveGlow[2];

#if charger_BLOOD_EFFECTS
	CNetworkVar( uint8, m_iBleedingLevel );

	unsigned char GetBleedingLevel( void ) const;
#endif
protected:

	int m_poseThrow;
	int m_poseHead_Yaw, m_poseHead_Pitch;
	virtual void	PopulatePoseParameters( void );
};

//==================================================
// CNPC_charger::m_DataDesc
//==================================================

BEGIN_DATADESC( CNPC_charger )

	DEFINE_FIELD(m_vecSaveSpitVelocity, FIELD_VECTOR),	//added
	DEFINE_FIELD( m_nFlinchActivity,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bStopped,			FIELD_BOOLEAN ), 	
	DEFINE_KEYFIELD( m_bBarkEnabled,	FIELD_BOOLEAN, "allowbark" ),

	DEFINE_FIELD( m_flSearchNoiseTime,	FIELD_TIME ), 
	DEFINE_FIELD( m_flAngerNoiseTime,	FIELD_TIME ), 
	DEFINE_FIELD( m_flBreathTime,		FIELD_TIME ), 
	DEFINE_FIELD( m_flChargeTime,		FIELD_TIME ),

	DEFINE_FIELD( m_hShoveTarget,				FIELD_EHANDLE ), 
	DEFINE_FIELD( m_hChargeTarget,				FIELD_EHANDLE ), 
	DEFINE_FIELD( m_hChargeTargetPosition,		FIELD_EHANDLE ), 
	DEFINE_FIELD( m_hOldTarget,					FIELD_EHANDLE ),
	// m_FailedPhysicsTargets	// We do not save/load these

	DEFINE_FIELD( m_hPhysicsTarget,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecPhysicsTargetStartPos,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecPhysicsHitPosition,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flPhysicsCheckTime,			FIELD_TIME ),
	DEFINE_FIELD( m_flNextHeavyFlinchTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flNextRoarTime,				FIELD_TIME ),
	DEFINE_FIELD( m_iChargeMisses,				FIELD_INTEGER ),
	DEFINE_FIELD( m_bDecidedNotToStop,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPreferPhysicsAttack,		FIELD_BOOLEAN ),

	DEFINE_FIELD(m_pEnergyWaveChar, FIELD_CLASSPTR),	//added
	
#if charger_BLOOD_EFFECTS
	DEFINE_FIELD( m_iBleedingLevel,				FIELD_CHARACTER ),
#endif

	DEFINE_KEYFIELD( m_strShoveTargets,	FIELD_STRING, "shovetargets" ),

	DEFINE_AUTO_ARRAY( m_hCaveGlow, FIELD_CLASSPTR ),

	DEFINE_SOUNDPATCH( m_pGrowlHighSound ),
	DEFINE_SOUNDPATCH( m_pGrowlLowSound ),
	DEFINE_SOUNDPATCH( m_pGrowlIdleSound ),
	DEFINE_SOUNDPATCH( m_pBreathSound ),
	DEFINE_SOUNDPATCH( m_pConfusedSound ),

	DEFINE_INPUTFUNC( FIELD_STRING,	"SetShoveTarget", InputSetShoveTarget ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"SetChargeTarget", InputSetChargeTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ClearChargeTarget", InputClearChargeTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Ragdoll", InputRagdoll ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableBark", InputEnableBark ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableBark", InputDisableBark ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnablePreferPhysicsAttack", InputEnablePreferPhysicsAttack ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisablePreferPhysicsAttack", InputDisablePreferPhysicsAttack ),

END_DATADESC()


//Fast Growl (Growl High)
envelopePoint_t envchargerFastGrowl[] =
{
	{	1.0f, 1.0f,
		0.2f, 0.4f,
	},
	{	0.1f, 0.1f,
		0.8f, 1.0f,
	},
	{	0.0f, 0.0f,
		0.4f, 0.8f,
	},
};


//Bark 1 (Growl High)
envelopePoint_t envchargerBark1[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.4f, 0.6f,
	},
};

//Bark 2 (Confused)
envelopePoint_t envchargerBark2[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.2f,
	},
	{	0.2f, 0.3f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.4f, 0.6f,
	},
};

//Pain
envelopePoint_t envchargerPain1[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.2f,
	},
	{	-1.0f, -1.0f,
		0.5f, 0.8f,
	},
	{	0.1f, 0.2f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.5f, 0.75f,
	},
};

//Squeeze (High Growl)
envelopePoint_t envchargerSqueeze[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.5f,
	},
};

//Scratch (Low Growl)
envelopePoint_t envchargerScratch[] =
{
	{	1.0f, 1.0f,
		0.4f, 0.6f,
	},
	{	0.5f, 0.5f,
		0.4f, 0.6f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.5f,
	},
};

//Grunt
envelopePoint_t envchargerGrunt[] =
{
	{	0.6f, 1.0f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.1f, 0.2f,
	},
};

envelopePoint_t envchargerGrunt2[] =
{
	{	0.2f, 0.4f,
		0.1f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.4f, 0.6f,
	},
};

//==============================================================================================
// ANTLION GUARD PHYSICS DAMAGE TABLE
//==============================================================================================
static impactentry_t chargerLinearTable[] =
{
	{ 100*100,	10 },
	{ 250*250,	25 },
	{ 350*350,	50 },
	{ 500*500,	75 },
	{ 1000*1000,100 },
};

static impactentry_t chargerAngularTable[] =
{
	{  50* 50, 10 },
	{ 100*100, 25 },
	{ 150*150, 50 },
	{ 200*200, 75 },
};

impactdamagetable_t gchargerImpactDamageTable =
{
	chargerLinearTable,
	chargerAngularTable,
	
	ARRAYSIZE(chargerLinearTable),
	ARRAYSIZE(chargerAngularTable),

	200*200,// minimum linear speed squared
	180*180,// minimum angular speed squared (360 deg/s to cause spin/slice damage)
	15,		// can't take damage from anything under 15kg

	10,		// anything less than 10kg is "small"
	5,		// never take more than 1 pt of damage from anything under 15kg
	128*128,// <15kg objects must go faster than 36 in/s to do damage

	45,		// large mass in kg 
	2,		// large mass scale (anything over 500kg does 4X as much energy to read from damage table)
	1,		// large mass falling scale
	0,		// my min velocity
};

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const impactdamagetable_t
//-----------------------------------------------------------------------------
const impactdamagetable_t &CNPC_charger::GetPhysicsImpactDamageTable( void )
{
	return gchargerImpactDamageTable;
}

//==================================================
// CNPC_charger
//==================================================

CNPC_charger::CNPC_charger( void )
{
	m_iszPhysicsPropClass = AllocPooledString( "prop_physics" );
}

LINK_ENTITY_TO_CLASS( npc_charger, CNPC_charger );		// BJ: Charger class

IMPLEMENT_SERVERCLASS_ST(CNPC_charger, DT_NPC_charger)

#if charger_BLOOD_EFFECTS
	SendPropInt(  SENDINFO( m_iBleedingLevel ), 2, SPROP_UNSIGNED ),
#endif
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::UpdateOnRemove( void )
{
	// Chain to the base class
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::Precache( void )
{
	PrecacheModel( charger_MODEL );

	UTIL_PrecacheOther("grenade_spit");
	//UTIL_PrecacheOther("grenade_energy");

	PrecacheScriptSound( "NPC_Charger.ChargeHitObject" );	//shove
	PrecacheScriptSound( "NPC_Charger.ChargeHitWall" );	//hithard

	PrecacheScriptSound("NPC_Charger.AttackWave"); //added wave attack

	PrecacheScriptSound("NPC_Charger.Spit"); //added spit attack

	if ( HasSpawnFlags(SF_charger_INSIDE_FOOTSTEPS) )
	{
		PrecacheScriptSound( "NPC_AntlionGuard.Inside.StepLight" );
		PrecacheScriptSound( "NPC_AntlionGuard.Inside.StepHeavy" );
	}
	else
	{
		PrecacheScriptSound( "NPC_Charger.StepLight" );
		PrecacheScriptSound( "NPC_Charger.StepHeavy" );
	}

	PrecacheScriptSound( "NPC_Charger.BreatheLoop" );	// unused?
	PrecacheScriptSound( "NPC_Charger.ShellCrack" );
	PrecacheScriptSound( "NPC_Charger.Hurt" );

	PrecacheScriptSound( "NPC_Charger.ChargeEnemy" );
	PrecacheScriptSound( "NPC_Charger.Roar" );
	PrecacheScriptSound( "NPC_Charger.Death" );

	PrecacheScriptSound("NPC_charger.FrustratedRoar");	// re-enabled
	PrecacheScriptSound( "NPC_Charger.GrowlHigh" );	// run for enemy
	PrecacheScriptSound( "NPC_Charger.Idle" );
	PrecacheScriptSound( "NPC_Charger.BreathSound" );
	PrecacheScriptSound( "NPC_Charger.Confused" );		// unknown
	PrecacheScriptSound( "NPC_Charger.Fallover" );

	PrecacheParticleSystem( "blood_antlionguard_injured_light" );
	PrecacheParticleSystem( "blood_antlionguard_injured_heavy" );

	PrecacheScriptSound("NPC_Antlion.PoisonBall");
	PrecacheParticleSystem("blood_impact_yellow_01");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::CreateGlow( CSprite **pSprite, const char *pAttachName )
{
	if ( pSprite == NULL )
		return;

	// Create the glow sprite
	*pSprite = CSprite::SpriteCreate( "sprites/grubflare1.vmt", GetLocalOrigin(), false );
	Assert( *pSprite );
	if ( *pSprite == NULL )
		return;

	(*pSprite)->TurnOn();
	(*pSprite)->SetTransparency( kRenderWorldGlow, 156, 169, 121, 164, kRenderFxNoDissipation );
	(*pSprite)->SetScale( 1.0f );
	(*pSprite)->SetGlowProxySize( 16.0f );
	int nAttachment = LookupAttachment( pAttachName );
	(*pSprite)->SetParent( this, nAttachment );
	(*pSprite)->SetLocalOrigin( vec3_origin );

	// Don't uselessly animate, we're a static sprite!
	(*pSprite)->SetThink( NULL );
	(*pSprite)->SetNextThink( TICK_NEVER_THINK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::Spawn( void )
{
	Precache();

	SetModel( charger_MODEL );
	SetHullType(HULL_MEDIUM);	// BJL Antlion hull perfect 4 us	//TALL was

	SetHullSizeNormal();
	SetDefaultEyeOffset();
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );

	SetNavType( NAV_GROUND );
	SetBloodColor( BLOOD_COLOR_YELLOW );

	m_iHealth = sk_charger_health.GetFloat();
	m_iMaxHealth = m_iHealth;
	m_flFieldOfView	= charger_FOV_NORMAL;
	
	m_flPhysicsCheckTime = 0;
	m_flChargeTime = 0;
	m_flNextRoarTime = 0;
	m_iChargeMisses = 0;
	m_flNextHeavyFlinchTime = 0;
	m_bDecidedNotToStop = false;

	ClearHintGroup();
	
	m_bStopped = false;

	m_hShoveTarget = NULL;
	m_hChargeTarget = NULL;
	m_hChargeTargetPosition = NULL;
	m_hPhysicsTarget = NULL;
	
	m_HackedGunPos.x		= 10;
	m_HackedGunPos.y		= 0;
	m_HackedGunPos.z		= 30;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	m_pEnergyWaveChar = NULL;	//added
	bChargeWaveRoarActivate = false;
	bChargeWaveOnce = false;

	NPCInit();

	BaseClass::Spawn();

	// Do not dissolve
	AddEFlags( EFL_NO_DISSOLVE );

	// We get a minute of free knowledge about the target
	GetEnemies()->SetEnemyDiscardTime( 120.0f );
	GetEnemies()->SetFreeKnowledgeDuration( 60.0f );

	// We need to bloat the absbox to encompass all the hitboxes
	Vector absMin = -Vector(64,64, 35);
	Vector absMax = Vector(64,64,128);
	CollisionProp()->SetSurroundingBoundsType( USE_SPECIFIED_BOUNDS, &absMin, &absMax );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::Activate( void )
{
	BaseClass::Activate();

	// Find all nearby physics objects and add them to the list of objects we will sense
	CBaseEntity *pObject = NULL;
	while ( ( pObject = gEntList.FindEntityInSphere( pObject, WorldSpaceCenter(), 2500 ) ) != NULL )
	{
		// Can't throw around debris
		if ( pObject->GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
			continue;

		// We can only throw a few types of things
		if ( !FClassnameIs( pObject, "prop_physics" ) && !FClassnameIs( pObject, "func_physbox" ) )
			continue;

		// Ensure it's mass is within range
		IPhysicsObject *pPhysObj = pObject->VPhysicsGetObject();
		if( ( pPhysObj == NULL ) || ( pPhysObj->GetMass() > charger_MAX_OBJECT_MASS ) || ( pPhysObj->GetMass() < charger_MIN_OBJECT_MASS ) )
			continue;

		// Tell the AI sensing list that we want to consider this
		g_AI_SensedObjectsManager.AddEntity( pObject );

		if ( g_debug_charger.GetInt() == 5 )
		{
			pObject->m_debugOverlays |= OVERLAY_BBOX_BIT;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Our enemy is unreachable. Select a schedule.
//-----------------------------------------------------------------------------
int CNPC_charger::SelectUnreachableSchedule( void )
{

	// First, look for objects near ourselves
	PhysicsObjectCriteria_t	criteria;
	criteria.bPreferObjectsAlongTargetVector = false;
	criteria.flNearRadius = (40*12);
	criteria.flRadius = (250*12);
	criteria.flTargetCone = -1.0f;
	criteria.pTarget = GetEnemy();
	criteria.vecCenter = GetAbsOrigin();

	CBaseEntity *pTarget = FindPhysicsObjectTarget( criteria );
	if ( pTarget == NULL && GetEnemy() )
	{
		// Use the same criteria, but search near the target instead of us
		criteria.vecCenter = GetEnemy()->GetAbsOrigin();
		pTarget = FindPhysicsObjectTarget( criteria );
	}

	// If we found one, we'll want to attack it
	if ( pTarget )
	{
		m_hPhysicsTarget = pTarget;
		SetCondition( COND_charger_PHYSICS_TARGET );
		m_vecPhysicsTargetStartPos = m_hPhysicsTarget->WorldSpaceCenter();
		
		// Tell any squadmates I'm going for this item so they don't as well
		if ( GetSquad() != NULL )
		{
			GetSquad()->BroadcastInteraction( g_interactionchargerFoundPhysicsObject, (void *)((CBaseEntity *)m_hPhysicsTarget), this );
		}

		return SCHED_charger_PHYSICS_ATTACK;
	}

	// Deal with a visible player
	if ( HasCondition( COND_SEE_ENEMY ) )
	{
		// Roar at the player as show of frustration
		if ( m_flNextRoarTime < gpGlobals->curtime )
		{
			m_flNextRoarTime = gpGlobals->curtime + RandomFloat( 20,40 );
			return SCHED_charger_ROAR;
		}

		// If we're under attack, then let's leave for a bit
		//if ( GetEnemy() && HasCondition( COND_HEAVY_DAMAGE ) )
		//{
			//Vector dir = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
			//VectorNormalize(dir);

			//GetMotor()->SetIdealYaw( -dir );
			
			//return SCHED_charger_TAKE_COVER_FROM_ENEMY;
		//}
	}

	// If we're too far away, try to close distance to our target first
	float flDistToEnemySqr = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();
	if ( flDistToEnemySqr > Square( 100*12 ) )
		return SCHED_charger_CHASE_ENEMY_TOLERANCE;

	// Fire that we're unable to reach our target!
	if ( GetEnemy() && GetEnemy()->IsPlayer() )
	{
		m_OnLostPlayer.FireOutput( this, this );
	}

	m_OnLostEnemy.FireOutput( this, this );
	GetEnemies()->MarkAsEluded( GetEnemy() );

	// Move randomly for the moment
	return SCHED_charger_CANT_ATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_charger::SelectCombatSchedule( void )
{
	ClearHintGroup();

	// Attack if we can
	if ( HasCondition(COND_CAN_MELEE_ATTACK1) )
		return SCHED_MELEE_ATTACK1;

	// See if we can bark
	if ( HasCondition( COND_ENEMY_UNREACHABLE ) )
		return SelectUnreachableSchedule();

	//Physics target
	if ( HasCondition( COND_charger_PHYSICS_TARGET ) )
		return SCHED_charger_PHYSICS_ATTACK;

	// Charging
	if ( HasCondition( COND_charger_CAN_CHARGE ) )
	{
		// Don't let other squad members charge while we're doing it
		// OccupyStrategySlot( SQUAD_SLOT_charger_CHARGE );
		return SCHED_charger_CHARGE;
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::ShouldCharge( const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel )
{

	// Must have a target
	if ( !GetEnemy() )
		return false;

	// No one else in the squad can be charging already
	if ( IsStrategySlotRangeOccupied( SQUAD_SLOT_charger_CHARGE, SQUAD_SLOT_charger_CHARGE ) )
		return false;

	// Don't check the distance once we start charging
	if ( !bCheckForCancel )
	{
		// Don't allow use to charge again if it's been too soon
		if ( useTime && ( m_flChargeTime > gpGlobals->curtime ) )
			return false;

		float distance = UTIL_DistApprox2D( startPos, endPos );

		// Must be within our tolerance range
		if ( ( distance < charger_CHARGE_MIN ) || ( distance > charger_CHARGE_MAX ) )
			return false;
	}

	if ( GetSquad() )
	{
		// If someone in our squad is closer to the enemy, then don't charge (we end up hitting them more often than not!)
		float flOurDistToEnemySqr = ( GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).LengthSqr();
		AISquadIter_t iter;
		for ( CAI_BaseNPC *pSquadMember = GetSquad()->GetFirstMember( &iter ); pSquadMember; pSquadMember = GetSquad()->GetNextMember( &iter ) )
		{
			if ( pSquadMember->IsAlive() == false || pSquadMember == this )
				continue;

			if ( ( pSquadMember->GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).LengthSqr() < flOurDistToEnemySqr )
				return false;
		}
	}

	//FIXME: We'd like to exclude small physics objects from this check!

	// We only need to hit the endpos with the edge of our bounding box
	Vector vecDir = endPos - startPos;
	VectorNormalize( vecDir );
	float flWidth = WorldAlignSize().x * 0.5;
	Vector vecTargetPos = endPos - (vecDir * flWidth);

	// See if we can directly move there
	AIMoveTrace_t moveTrace;
	GetMoveProbe()->MoveLimit( NAV_GROUND, startPos, vecTargetPos, MASK_NPCSOLID_BRUSHONLY, GetEnemy(), &moveTrace );
	
	// Draw the probe
	if ( g_debug_charger.GetInt() == 1 )
	{
		Vector	enemyDir	= (vecTargetPos - startPos);
		float	enemyDist	= VectorNormalize( enemyDir );

		NDebugOverlay::BoxDirection( startPos, GetHullMins(), GetHullMaxs() + Vector(enemyDist,0,0), enemyDir, 0, 255, 0, 8, 1.0f );
	}

	// If we're not blocked, charge
	if ( IsMoveBlocked( moveTrace ) )
	{
		// Don't allow it if it's too close to us
		if ( UTIL_DistApprox( WorldSpaceCenter(), moveTrace.vEndPosition ) < charger_CHARGE_MIN )
			return false;

		// Allow some special cases to not block us
		if ( moveTrace.pObstruction != NULL )
		{
			// If we've hit the world, see if it's a cliff
			if ( moveTrace.pObstruction == GetContainingEntity( INDEXENT(0) ) )
			{	
				// Can't be too far above/below the target
				if ( fabs( moveTrace.vEndPosition.z - vecTargetPos.z ) > StepHeight() )
					return false;

				// Allow it if we got pretty close
				if ( UTIL_DistApprox( moveTrace.vEndPosition, vecTargetPos ) < 64 )
					return true;
			}

			// Hit things that will take damage
			if ( moveTrace.pObstruction->m_takedamage != DAMAGE_NO )
				return true;

			// Hit things that will move
			if ( moveTrace.pObstruction->GetMoveType() == MOVETYPE_VPHYSICS )
				return true;
		}

		return false;
	}

	// Only update this if we've requested it
	if ( useTime )
	{
		m_flChargeTime = gpGlobals->curtime + 4.0f;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_charger::SelectSchedule( void )
{

#if 0
	// Debug physics object finding
	if ( 0 ) //g_debug_charger.GetInt() == 3 )
	{
		m_flPhysicsCheckTime = 0;
		UpdatePhysicsTarget( false );
		return SCHED_IDLE_STAND;
	}
#endif

	// Flinch on heavy damage, but not if we've flinched too recently.
	// This is done to prevent stun-locks from grenades.
	if ( HasCondition( COND_HEAVY_DAMAGE ) && m_flNextHeavyFlinchTime < gpGlobals->curtime )
	{
		m_flNextHeavyFlinchTime = gpGlobals->curtime + 8.0f;
		return SCHED_charger_PHYSICS_DAMAGE_HEAVY;
	}

	// Prefer to use physics, in this case
	if ( m_bPreferPhysicsAttack )
	{
		// If we have a target, try to go for it
		if ( HasCondition( COND_charger_PHYSICS_TARGET ) )
			return SCHED_charger_PHYSICS_ATTACK;
	}

	// Charge after a target if it's set
	if ( m_hChargeTarget && m_hChargeTargetPosition )
	{
		ClearCondition( COND_charger_HAS_CHARGE_TARGET );
		ClearHintGroup();
		
		if ( m_hChargeTarget->IsAlive() == false )
		{
			m_hChargeTarget	= NULL;
			m_hChargeTargetPosition = NULL;
			SetEnemy( m_hOldTarget );

			if ( m_hOldTarget == NULL )
			{
				m_NPCState = NPC_STATE_ALERT;
			}
		}
		else
		{
			m_hOldTarget = GetEnemy();
			SetEnemy( m_hChargeTarget );
			UpdateEnemyMemory( m_hChargeTarget, m_hChargeTarget->GetAbsOrigin() );

			//If we can't see the target, run to somewhere we can
			if ( ShouldCharge( GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), false, false ) == false )
				return SCHED_charger_FIND_CHARGE_POSITION;

			return SCHED_charger_CHARGE_TARGET;
		}
	}

	// See if we need to clear a path to our enemy
	if ( HasCondition( COND_ENEMY_OCCLUDED ) || HasCondition( COND_ENEMY_UNREACHABLE ) )
	{
		CBaseEntity *pBlocker = GetEnemyOccluder();

		if ( ( pBlocker != NULL ) && FClassnameIs( pBlocker, "prop_physics" ) )
		{
			m_hPhysicsTarget = pBlocker;
			return SCHED_charger_PHYSICS_ATTACK;
		}
	}

	//Only do these in combat states
	if ( m_NPCState == NPC_STATE_COMBAT && GetEnemy() )
		return SelectCombatSchedule();

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_charger::MeleeAttack1Conditions( float flDot, float flDist )
{
	// Don't attack again too soon
	if ( GetNextAttack() > gpGlobals->curtime )
		return 0;

	// While charging, we can't melee attack
	if ( IsCurSchedule( SCHED_charger_CHARGE ) )
		return 0;

	// Must be close enough
	if ( flDist > charger_MELEE1_RANGE )
		return COND_TOO_FAR_TO_ATTACK;
		

	// Must be within a viable cone
	if ( flDot < charger_MELEE1_CONE )
		return COND_NOT_FACING_ATTACK;

	// If the enemy is on top of me, I'm allowed to hit the sucker
	if ( GetEnemy()->GetGroundEntity() == this )
		return COND_CAN_MELEE_ATTACK1;

	trace_t	tr;
	TraceHull_SkipPhysics( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), Vector(-10,-10,-10), Vector(10,10,10), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5 );

	// If we hit anything, go for it
	if ( tr.fraction < 1.0f )
		return COND_CAN_MELEE_ATTACK1;

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CNPC_charger::MaxYawSpeed( void )
{
	Activity eActivity = GetActivity();

	// Stay still
	if (( eActivity == ACT_charger_SEARCH ) || 
		( eActivity == ACT_charger_BARK ) || 
		( eActivity == ACT_MELEE_ATTACK1 ) )
		return 0.0f;

	CBaseEntity *pEnemy = GetEnemy();

	if ( pEnemy != NULL && pEnemy->IsPlayer() == false )
		return 16.0f;

	// Turn slowly when you're charging
	if ( eActivity == ACT_charger_CHARGE_START )
		return 12.0f; //4

	// Allow a better turning rate when moving quickly but not charging the player
	if ( ( eActivity == ACT_charger_CHARGE_RUN ) && IsCurSchedule( SCHED_charger_CHARGE ) == false )
		return 16.0f;

	// Turn more slowly as we close in on our target
	if ( eActivity == ACT_charger_CHARGE_RUN )
	{
		if ( pEnemy == NULL )
			return 9.0f; //2

		float dist = UTIL_DistApprox2D( GetEnemy()->WorldSpaceCenter(), WorldSpaceCenter() );

		if ( dist > 512 )
			return 16.0f;

		//FIXME: Alter by skill level
		float yawSpeed = RemapVal( dist, 0, 512, 1.0f, 2.0f );
		yawSpeed = clamp( yawSpeed, 1.0f, 2.0f );

		return yawSpeed;
	}

	if ( eActivity == ACT_charger_CHARGE_STOP )
		return 8.0f;

	switch( eActivity )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 40.0f;
		break;
	
	case ACT_RUN:
	default:
		return 20.0f;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::OverrideMove( float flInterval )
{
	// If the guard's charging, we're handling the movement
	if ( IsCurSchedule( SCHED_charger_CHARGE ) )
		return true;

	return BaseClass::OverrideMove( flInterval );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::Shove( void )
{
	if ( GetNextAttack() > gpGlobals->curtime )
		return;

	CBaseEntity *pHurt = NULL;
	CBaseEntity *pTarget;

	pTarget = ( m_hShoveTarget == NULL ) ? GetEnemy() : m_hShoveTarget.Get();

	if ( pTarget == NULL )
	{
		m_hShoveTarget = NULL;
		return;
	}

	//Always damage bullseyes if we're scripted to hit them
	if ( pTarget->Classify() == CLASS_BULLSEYE )
	{
		pTarget->TakeDamage( CTakeDamageInfo( this, this, 1.0f, DMG_CRUSH ) );
		m_hShoveTarget = NULL;
		return;
	}

	float damage = ( pTarget->IsPlayer() ) ? sk_charger_dmg_shove.GetFloat() : 250;

	// If the target's still inside the shove cone, ensure we hit him	
	Vector vecForward, vecEnd;
	AngleVectors( GetAbsAngles(), &vecForward );
  	float flDistSqr = ( pTarget->WorldSpaceCenter() - WorldSpaceCenter() ).LengthSqr();
  	Vector2D v2LOS = ( pTarget->WorldSpaceCenter() - WorldSpaceCenter() ).AsVector2D();
  	Vector2DNormalize(v2LOS);
  	float flDot	= DotProduct2D (v2LOS, vecForward.AsVector2D() );
	if ( flDistSqr < (charger_MELEE1_RANGE*charger_MELEE1_RANGE) && flDot >= charger_MELEE1_CONE )
	{
		vecEnd = pTarget->WorldSpaceCenter();
	}
	else
	{
		vecEnd = WorldSpaceCenter() + ( BodyDirection3D() * charger_MELEE1_RANGE );
	}

	// Use the melee trace to ensure we hit everything there
	trace_t tr;
	CTakeDamageInfo	dmgInfo( this, this, damage, DMG_SLASH );
	CTraceFilterMelee traceFilter( this, COLLISION_GROUP_NONE, &dmgInfo, 1.0, true );
	Ray_t ray;
	ray.Init( WorldSpaceCenter(), vecEnd, Vector(-16,-16,-16), Vector(16,16,16) );
	enginetrace->TraceRay( ray, MASK_SHOT_HULL, &traceFilter, &tr );
	pHurt = tr.m_pEnt;

	// Knock things around
	ImpactShock( tr.endpos, 100.0f, 250.0f );

	if ( pHurt )
	{
		Vector	traceDir = ( tr.endpos - tr.startpos );
		VectorNormalize( traceDir );

		// Generate enough force to make a 75kg guy move away at 600 in/sec
		Vector vecForce = traceDir * ImpulseScale( 75, 100 );
		CTakeDamageInfo info( this, this, vecForce, tr.endpos, damage, DMG_CLUB );
		pHurt->TakeDamage( info );

		m_hShoveTarget = NULL;

		EmitSound( "NPC_Charger.ChargeHitObject" );

		// If the player, throw him around
		if ( pHurt->IsPlayer() )
		{
			//Punch the view
			pHurt->ViewPunch( QAngle(20,0,-20) );
			
			//Shake the screen
			UTIL_ScreenShake( pHurt->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START );

			//Red damage indicator
			color32 red = {128,0,0,128};
			UTIL_ScreenFade( pHurt, red, 1.0f, 0.1f, FFADE_IN );

			Vector forward, up;
			AngleVectors( GetLocalAngles(), &forward, NULL, &up );
			pHurt->ApplyAbsVelocityImpulse( forward * 150 + up * 65 ); //400 150 BJ THROW

		}	
		else
		{
			CBaseCombatCharacter *pVictim = ToBaseCombatCharacter( pHurt );
			
			if ( pVictim )
			{
				if ( NPC_Rollermine_IsRollermine( pVictim ) )
				{
					Pickup_OnPhysGunDrop( pVictim, NULL, LAUNCHED_BY_CANNON );
				}

				// FIXME: This causes NPCs that are not physically motivated to hop into the air strangely -- jdw
				// pVictim->ApplyAbsVelocityImpulse( BodyDirection2D() * 400 + Vector( 0, 0, 250 ) );
			}

			m_hShoveTarget = NULL;
		}
	}
}

// BJ: ADDED
void CNPC_charger::WaveRoarAttack(void)
{

	if (m_pEnergyWaveChar)
	{
		UTIL_Remove(m_pEnergyWaveChar);
	}
	Vector vFacingDir = EyeDirection3D();
	m_pEnergyWaveChar = (CEnergyWave*)Create("energy_wave", EyePosition(), GetLocalAngles());
	//m_flEndEnergyWaveTime = gpGlobals->curtime + 1;
	m_pEnergyWaveChar->SetAbsVelocity(100 * vFacingDir);

	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityInSphere(pEntity, GetAbsOrigin(), 300)) != NULL)	//500
	{
		if (pEntity->Classify() == CLASS_ANTLION)
		{
			continue;
		}

		if (pEntity->GetFlags() & FL_NOTARGET)
		{
			continue;
		}

		IPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();

		if (pEntity->m_takedamage != DAMAGE_NO || pPhysicsObject)
		{
			// --------------------------
			// Adjust damage by distance
			// --------------------------
			float flDist = (pEntity->WorldSpaceCenter() - GetAbsOrigin()).Length();
			float flDamageAdjuster = 1 - (flDist / 300);

			// --------------------------
			// Adjust damage by direction
			// --------------------------
			Vector forward;
			AngleVectors(GetAbsAngles(), &forward);
			Vector vEntDir = (pEntity->GetAbsOrigin() - GetAbsOrigin());
			VectorNormalize(vEntDir);
			float flDotPr = DotProduct(forward, vEntDir);
			flDamageAdjuster *= flDotPr;

			if (flDamageAdjuster < 0)
			{
				continue;
			}

			// --------------------------
			// Adjust damage by visibility
			// --------------------------
			if (!FVisible(pEntity))
			{
				if (pEntity->IsPlayer())
				{
					flDamageAdjuster *= 0.5;
				}
				else if (!FClassnameIs(pEntity, "func_breakable") && !FClassnameIs(pEntity, "func_pushable"))
				{
					// do not hurt nonclients through walls, but allow damage to be done to breakables
					continue;
				}
			}

			// ------------------------------
			//  Apply the damage
			// ------------------------------
			if (pEntity->m_takedamage != DAMAGE_NO)
			{
				CTakeDamageInfo info(this, this, flDamageAdjuster * sk_charger_dmg_waveroar.GetFloat(), DMG_SONIC | DMG_ALWAYSGIB);
				CalculateExplosiveDamageForce(&info, (pEntity->GetAbsOrigin() - GetAbsOrigin()), pEntity->GetAbsOrigin());

				pEntity->TakeDamage(info);

				// Throw the player
				if (pEntity->IsPlayer())
				{
					Vector forward;
					AngleVectors(GetLocalAngles(), &forward);

					Vector vecVelocity = pEntity->GetAbsVelocity();
					vecVelocity += forward * 250 * flDamageAdjuster;
					vecVelocity.z = 300 * flDamageAdjuster;
					pEntity->SetAbsVelocity(vecVelocity);
					pEntity->ViewPunch(QAngle(random->RandomInt(-20, 20), 0, random->RandomInt(-20, 20)));
				}
			}
			// ------------------------------
			//  Apply physics foces
			// ------------------------------
			IPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();
			if (pPhysicsObject)
			{
				float flForce = flDamageAdjuster * 8000;
				pPhysicsObject->ApplyForceCenter((vEntDir + Vector(0, 0, 0.2)) * flForce);
				pPhysicsObject->ApplyTorqueCenter(vEntDir * flForce);
			}
		}
	}


	EmitSound("NPC_Charger.AttackWave");

}

void CNPC_charger::Touch(CBaseEntity *pOther)
{
	DevMsg("Touched something \n");
	//CBaseEntity *pHurt = NULL;
	//trace_t tr;
	//pHurt = tr.m_pEnt;
	CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), 0, 0);

	// If the player, throw him around
	if (pHurt)
	{
		if (pHurt->IsPlayer())
		{
			DevMsg("Touched player \n");
		
		//Punch the view
		pHurt->ViewPunch(QAngle(20, 0, -20));

		//Shake the screen
		UTIL_ScreenShake(pHurt->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START);

		//Red damage indicator
		color32 red = { 128, 0, 0, 128 };
		UTIL_ScreenFade(pHurt, red, 1.0f, 0.1f, FFADE_IN);

		Vector forward, up;
		AngleVectors(GetLocalAngles(), &forward, NULL, &up);
		pHurt->ApplyAbsVelocityImpulse(forward * 400 + up * 150);

		if (m_iBleedingLevel == 2)
		{
			pHurt->TakeDamage(CTakeDamageInfo(this, this, /*pHurt->m_iHealth -*/ 12, DMG_POISON));
		}
		else
			pHurt->TakeDamage(CTakeDamageInfo(this, this, 5, DMG_SLASH));

		//#endif
		}
		if (pHurt->IsNPC())
		{
			DevMsg("Touched NPC \n");

			Vector forward, up;
			AngleVectors(GetLocalAngles(), &forward, NULL, &up);
			pHurt->ApplyAbsVelocityImpulse(forward * 100 + up * 40);

			pHurt->TakeDamage(CTakeDamageInfo(this, this, /*pHurt->m_iHealth -*/ 12, DMG_SLASH));
		}

	}

	BaseClass::Touch(pOther);
}

Vector VecCheckThrowToleranceCharge(CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance)
{
	flSpeed = MAX(1.0f, flSpeed);

	float flGravity = GetCurrentGravity();

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);


	trace_t tr;
	UTIL_TraceLine(vecSpot1, vecApex, MASK_SOLID, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		// fail!
		/*if (g_debug_antlion_worker.GetBool())
		{
			NDebugOverlay::Line(vecSpot1, vecApex, 255, 0, 0, true, 5.0);
		}*/

		return vec3_origin;
	}

	/*if (g_debug_antlion_worker.GetBool())
	{
		NDebugOverlay::Line(vecSpot1, vecApex, 0, 255, 0, true, 5.0);
	}*/

	UTIL_TraceLine(vecApex, vecSpot2, MASK_SOLID_BRUSHONLY, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		bool bFail = true;

		// Didn't make it all the way there, but check if we're within our tolerance range
		if (flTolerance > 0.0f)
		{
			float flNearness = (tr.endpos - vecSpot2).LengthSqr();
			if (flNearness < Square(flTolerance))
			{
				/*if (g_debug_antlion_worker.GetBool())
				{
					NDebugOverlay::Sphere(tr.endpos, vec3_angle, flTolerance, 0, 255, 0, 0, true, 5.0);
				}*/

				bFail = false;
			}
		}

		if (bFail)
		{
			/*if (g_debug_antlion_worker.GetBool())
			{
				NDebugOverlay::Line(vecApex, vecSpot2, 255, 0, 0, true, 5.0);
				NDebugOverlay::Sphere(tr.endpos, vec3_angle, flTolerance, 255, 0, 0, 0, true, 5.0);
			}*/
			return vec3_origin;
		}
	}

	/*if (g_debug_antlion_worker.GetBool())
	{
		NDebugOverlay::Line(vecApex, vecSpot2, 0, 255, 0, true, 5.0);
	}*/

	return vecGrenadeVel;
}

bool CNPC_charger::GetSpitVectorRegular(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut)
{
	// antlion workers exist only in episodic.
//#if HL2_EPISODIC
	// Try the most direct route
	Vector vecToss = VecCheckThrowToleranceCharge(this, vecStartPos, vecTarget, sk_charger_spit_speed_regular.GetFloat(), (10.0f*12.0f));

	// If this failed then try a little faster (flattens the arc)
	if (vecToss == vec3_origin)
	{
		vecToss = VecCheckThrowToleranceCharge(this, vecStartPos, vecTarget, sk_charger_spit_speed_regular.GetFloat() * 1.5f, (10.0f*12.0f));
		if (vecToss == vec3_origin)
			return false;
	}

	// Save out the result
	if (vecOut)
	{
		*vecOut = vecToss;
	}

	return true;
//#else
	//return false;
//#endif
}

bool CNPC_charger::GetSpitVectorUnder(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut)
{
	// antlion workers exist only in episodic.
	//#if HL2_EPISODIC
	// Try the most direct route
	Vector vecToss = VecCheckThrowToleranceCharge(this, vecStartPos, vecTarget, sk_charger_spit_speed_special.GetFloat(), (10.0f*12.0f));

	// If this failed then try a little faster (flattens the arc)
	if (vecToss == vec3_origin)
	{
		vecToss = VecCheckThrowToleranceCharge(this, vecStartPos, vecTarget, sk_charger_spit_speed_special.GetFloat() * 1.5f, (10.0f*12.0f));
		if (vecToss == vec3_origin)
			return false;
	}

	// Save out the result
	if (vecOut)
	{
		*vecOut = vecToss;
	}

	return true;
	//#else
	//return false;
	//#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::SpitFire(void)	// BJ: Separate spit function
{
	if (GetEnemy())
	{
		Vector vSpitPos;
		GetAttachment("mouth", vSpitPos);

		Vector	vTarget;

		// If our enemy is looking at us and far enough away, lead him
		if (HasCondition(COND_ENEMY_FACING_ME) && UTIL_DistApprox(GetAbsOrigin(), GetEnemy()->GetAbsOrigin()) > (40 * 12))
		{
			UTIL_PredictedPosition(GetEnemy(), 0.5f, &vTarget);
			vTarget.z = GetEnemy()->GetAbsOrigin().z;
		}
		else
		{
			// Otherwise he can't see us and he won't be able to dodge
			vTarget = GetEnemy()->BodyTarget(vSpitPos, true);
		}

		vTarget[2] += random->RandomFloat(0.0f, 32.0f);

		Vector	vecToss;

		if (m_iBleedingLevel == 2)
		{
			DevMsg("Bleed level 2, activate spit specials \n");
			
			if (GetSpitVectorUnder(vSpitPos, vTarget, &vecToss) == false)
			{
				// Now try where they were
				if (GetSpitVectorUnder(vSpitPos, m_vSavePosition, &vecToss) == false)
				{
					// Failing that, just shoot with the old velocity we calculated initially!
					vecToss = m_vecSaveSpitVelocity;
				}
			}
		}
		else
		{

			// Try and spit at our target
			//Vector	vecToss;
			if (GetSpitVectorRegular(vSpitPos, vTarget, &vecToss) == false)
			{
				// Now try where they were
				if (GetSpitVectorRegular(vSpitPos, m_vSavePosition, &vecToss) == false)
				{
					// Failing that, just shoot with the old velocity we calculated initially!
					vecToss = m_vecSaveSpitVelocity;
				}
			}
		}

		// Find what our vertical theta is to estimate the time we'll impact the ground
		Vector vecToTarget = (vTarget - vSpitPos);
		VectorNormalize(vecToTarget);
		float flVelocity = VectorNormalize(vecToss);
		float flCosTheta = DotProduct(vecToTarget, vecToss);
		float flTime = (vSpitPos - vTarget).Length2D() / (flVelocity * flCosTheta);

		// Emit a sound where this is going to hit so that targets get a chance to act correctly
		CSoundEnt::InsertSound(SOUND_DANGER, vTarget, (15 * 12), flTime, this);

		// Don't fire again until this volley would have hit the ground (with some lag behind it)
		SetNextAttack(gpGlobals->curtime + flTime + random->RandomFloat(0.5f, 2.0f));

		// Tell any squadmates not to fire for some portion of the time this volley will be in the air (except on hard)
		//if (g_pGameRules->IsSkillLevel(SKILL_HARD) == false)
			//DelaySquadAttack(flTime);

		for (int i = 0; i < 6; i++)
		{
			CGrenadeSpit *pGrenade = (CGrenadeSpit*)CreateEntityByName("grenade_spit");
			pGrenade->SetAbsOrigin(vSpitPos);
			pGrenade->SetAbsAngles(vec3_angle);
			DispatchSpawn(pGrenade);
			pGrenade->SetThrower(this);
			pGrenade->SetOwnerEntity(this);

			if (i == 0)
			{
				pGrenade->SetSpitSize(SPIT_LARGE);
				pGrenade->SetAbsVelocity(vecToss * flVelocity);
			}
			else
			{
				pGrenade->SetAbsVelocity((vecToss + RandomVector(-0.035f, 0.035f)) * flVelocity);
				pGrenade->SetSpitSize(random->RandomInt(SPIT_SMALL, SPIT_MEDIUM));
			}

			// Tumble through the air
			pGrenade->SetLocalAngularVelocity(
				QAngle(random->RandomFloat(-250, -500),
				random->RandomFloat(-250, -500),
				random->RandomFloat(-250, -500)));
		}

		for (int i = 0; i < 8; i++)
		{
			DispatchParticleEffect("blood_impact_yellow_01", vSpitPos + RandomVector(-12.0f, 12.0f), RandomAngle(0, 360));
		}

		EmitSound("NPC_Charger.Spit");
	}
	//return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTraceFilterCharge : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCharge );
	
	CTraceFilterCharge( const IHandleEntity *passentity, int collisionGroup, CNPC_charger *pAttacker )
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_pAttacker(pAttacker)
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		
		if ( pEntity )
		{
			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;
			
			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			if ( pEntity->m_takedamage == DAMAGE_NO )
				return false;

			// Translate the vehicle into its driver for damage
			if ( pEntity->GetServerVehicle() != NULL )
			{
				CBaseEntity *pDriver = pEntity->GetServerVehicle()->GetPassenger();

				if ( pDriver != NULL )
				{
					pEntity = pDriver;
				}
			}
	
			Vector	attackDir = pEntity->WorldSpaceCenter() - m_pAttacker->WorldSpaceCenter();
			VectorNormalize( attackDir );

			float	flDamage = ( pEntity->IsPlayer() ) ? sk_charger_dmg_shove.GetFloat() : 250;//;

			CTakeDamageInfo info( m_pAttacker, m_pAttacker, flDamage, DMG_CRUSH );
			CalculateMeleeDamageForce( &info, attackDir, info.GetAttacker()->WorldSpaceCenter(), 4.0f );

			CBaseCombatCharacter *pVictimBCC = pEntity->MyCombatCharacterPointer();

			// Only do these comparisons between NPCs
			if ( pVictimBCC )
			{
				// Can only damage other NPCs that we hate
				if ( m_pAttacker->IRelationType( pEntity ) == D_HT )
				{
					pEntity->TakeDamage( info );
					return true;
				}
			}
			else
			{
				// Otherwise just damage passive objects in our way
				pEntity->TakeDamage( info );
				Pickup_ForcePlayerToDropThisObject( pEntity );
			}
		}

		return false;
	}

public:
	const IHandleEntity *m_pPassEnt;
	int					m_collisionGroup;
	CNPC_charger	*m_pAttacker;
};

#define	MIN_FOOTSTEP_NEAR_DIST	Square( 80*12.0f )// ft

//-----------------------------------------------------------------------------
// Purpose: Plays a footstep sound with temporary distance fades
// Input  : bHeavy - Larger back hoof is considered a "heavy" step
//-----------------------------------------------------------------------------
void CNPC_charger::Footstep( bool bHeavy )
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	Assert( pPlayer != NULL );
	if ( pPlayer == NULL )
		return;

	float flDistanceToPlayerSqr = ( pPlayer->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();
	float flNearVolume = RemapValClamped( flDistanceToPlayerSqr, Square(10*12.0f), MIN_FOOTSTEP_NEAR_DIST, VOL_NORM, 0.0f );

	EmitSound_t soundParams;
	CPASAttenuationFilter filter( this );

	if ( bHeavy )
	{
		if ( flNearVolume > 0.0f )
		{
			soundParams.m_pSoundName = "NPC_Charger.StepHeavy";
			soundParams.m_flVolume = flNearVolume;
			soundParams.m_nFlags = SND_CHANGE_VOL;
			EmitSound( filter, entindex(), soundParams );
		}

		//EmitSound( "NPC_Charger.StepHeavy" );
	}
	else
	{
		if ( flNearVolume > 0.0f )
		{
			soundParams.m_pSoundName = "NPC_Charger.StepLight";
			soundParams.m_flVolume = flNearVolume;
			soundParams.m_nFlags = SND_CHANGE_VOL;
			EmitSound( filter, entindex(), soundParams );
		}

		//EmitSound( "NPC_Charger.StepLight" );
	}
}

extern ConVar sv_gravity;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pObject - 
//			*pOut - 
//-----------------------------------------------------------------------------
void CNPC_charger::GetPhysicsShoveDir( CBaseEntity *pObject, float flMass, Vector *pOut )
{
	const Vector vecStart = pObject->WorldSpaceCenter();
	const Vector vecTarget = GetEnemy()->WorldSpaceCenter();
	
	const float flBaseSpeed = 800.0f;
	float flSpeed = RemapValClamped( flMass, 0.0f, 150.0f, flBaseSpeed * 2.0f, flBaseSpeed );

	// Try the most direct route
	Vector vecToss = VecCheckThrow( this, vecStart, vecTarget, flSpeed, 1.0f );

	// If this failed then try a little faster (flattens the arc)
	if ( vecToss == vec3_origin )
	{
		vecToss = VecCheckThrow( this, vecStart, vecTarget, flSpeed * 1.25f, 1.0f );
		if ( vecToss == vec3_origin )
		{
			const float flGravity = sv_gravity.GetFloat();

			vecToss = (vecTarget - vecStart);

			// throw at a constant time
			float time = vecToss.Length( ) / flSpeed;
			vecToss = vecToss * (1.0f / time);

			// adjust upward toss to compensate for gravity loss
			vecToss.z += flGravity * time * 0.5f;
		}
	}

	// Save out the result
	if ( pOut )
	{
		*pOut = vecToss;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CNPC_charger::HandleAnimEvent( animevent_t *pEvent )
{

	// Don't handle anim events after death
	if ( m_NPCState == NPC_STATE_DEAD )
	{
		BaseClass::HandleAnimEvent( pEvent );
		return;
	}

	/*
	// Footprints in da snow
	if ( pEvent->event == AE_charger_FOOTPRINT_LEFT )
	{
		SnowFootPrint( true );
#if 1
		return;
#endif
	}
	if ( pEvent->event == AE_charger_FOOTPRINT_RIGHT )
	{
		SnowFootPrint( false );
#if 1
		return;
#endif
	}
	if ( pEvent->event == AE_charger_FOOTPRINT_BACK )
	{
		SnowFootPrint();
#if 1
		return;
#endif
	}
	*/

	if ( pEvent->event == AE_charger_SHOVE_PHYSOBJECT )
	{
		if ( m_hPhysicsTarget == NULL )
		{
			// Disrupt other objects near us anyway
			ImpactShock( WorldSpaceCenter(), 150, 150.0f );		//BJ: 250 last
			return;
		}

		// If we have no enemy, we don't really have a direction to throw the object
		// in. But, we still want to clobber it so the animevent doesn't happen fruitlessly,
		// and we want to clear the condition flags and other state regarding the m_hPhysicsTarget.
		// So, skip the code relevant to computing a launch vector specific to the object I'm
		// striking, but use the ImpactShock call further below to punch everything in the neighborhood.
		CBaseEntity *pEnemy = GetEnemy();
		if ( pEnemy != NULL )
		{
			//Setup the throw velocity
			IPhysicsObject *physObj = m_hPhysicsTarget->VPhysicsGetObject();
			Vector vecShoveVel = ( pEnemy->GetAbsOrigin() - m_hPhysicsTarget->WorldSpaceCenter() );
			VectorNormalize( vecShoveVel );

			// Must still be close enough to our target
			Vector shoveDir = m_hPhysicsTarget->WorldSpaceCenter() - WorldSpaceCenter();
			float shoveDist = VectorNormalize( shoveDir );
			if ( shoveDist > 300.0f )
			{
				// Pick a new target next time (this one foiled us!)
				RememberFailedPhysicsTarget( m_hPhysicsTarget );
				m_hPhysicsTarget = NULL;
				return;
			}

	
			Vector vecTargetDir = vecShoveVel;

			// Get our shove direction
			GetPhysicsShoveDir( m_hPhysicsTarget, physObj->GetMass(), &vecShoveVel );

			// If the player isn't looking at me, and isn't reachable, be more forgiving about hitting them
			if ( HasCondition( COND_ENEMY_UNREACHABLE ) && HasCondition( COND_ENEMY_FACING_ME ) == false )
			{
				// Build an arc around the top of the target that we'll offset our aim by
				Vector vecOffset;
				float flSin, flCos;
				float flRad = random->RandomFloat( 0, M_PI / 6.0f ); // +/- 30 deg
				if ( random->RandomInt( 0, 1 ) )
					flRad *= -1.0f;

				SinCos( flRad, &flSin, &flCos );

				// Rotate the 2-d circle to be "facing" along our shot direction
				Vector vecArc;
				QAngle vecAngles;
				VectorAngles( vecTargetDir, vecAngles );
				VectorRotate( Vector( 0.0f, flCos, flSin ), vecAngles, vecArc );

				// Find the radius by which to avoid the player
				float flOffsetRadius = ( m_hPhysicsTarget->CollisionProp()->BoundingRadius() + GetEnemy()->CollisionProp()->BoundingRadius() ) * 1.5f;

				// Add this to our velocity to offset it
				vecShoveVel += ( vecArc * flOffsetRadius );
			}

			// Factor out mass
			vecShoveVel *= physObj->GetMass();
				
			// FIXME: We need to restore this on the object at some point if we do this!
			float flDragCoef = 0.0f;
			physObj->SetDragCoefficient( &flDragCoef, &flDragCoef );

			//Send it flying
			AngularImpulse angVel( random->RandomFloat(-180, 180), 100, random->RandomFloat(-360, 360) );
			physObj->ApplyForceCenter( vecShoveVel );
			physObj->AddVelocity( NULL, &angVel );
		}
						
		//Display dust
		Vector vecRandom = RandomVector( -1, 1);
		VectorNormalize( vecRandom );
		g_pEffects->Dust( m_hPhysicsTarget->WorldSpaceCenter(), vecRandom, 64.0f, 32 );

		// If it's being held by the player, break that bond
		Pickup_ForcePlayerToDropThisObject( m_hPhysicsTarget );

		EmitSound( "NPC_Charger.ChargeHitWall" );

		//Clear the state information, we're done
		ClearCondition( COND_charger_PHYSICS_TARGET );
		ClearCondition( COND_charger_PHYSICS_TARGET_INVALID );

		// Disrupt other objects near it, including the m_hPhysicsTarget if we had no valid enemy
		ImpactShock( m_hPhysicsTarget->WorldSpaceCenter(), 150, 150.0f, pEnemy != NULL ? m_hPhysicsTarget : NULL );		// BJ: 250.0f was

		// Notify any squad members that we're no longer monopolizing this object
		if ( GetSquad() != NULL )
		{
			GetSquad()->BroadcastInteraction( g_interactionchargerShovedPhysicsObject, (void *)((CBaseEntity *)m_hPhysicsTarget), this );
		}

		m_hPhysicsTarget = NULL;
		m_FailedPhysicsTargets.RemoveAll();

		return;
	}
	
	if ( pEvent->event == AE_charger_CHARGE_HIT )
	{
		UTIL_ScreenShake( GetAbsOrigin(), 32.0f, 4.0f, 1.0f, 512, SHAKE_START );
		EmitSound( "NPC_Charger.ChargeHitWall" );

		Vector	startPos = GetAbsOrigin();
		float	checkSize = ( CollisionProp()->BoundingRadius() + 8.0f );
		Vector	endPos = startPos + ( BodyDirection3D() * checkSize );

		CTraceFilterCharge traceFilter( this, COLLISION_GROUP_NONE, this );

		Ray_t ray;
		ray.Init( startPos, endPos, GetHullMins(), GetHullMaxs() );

		trace_t tr;
		enginetrace->TraceRay( ray, MASK_SHOT, &traceFilter, &tr );

		if ( g_debug_charger.GetInt() == 1 )
		{
			Vector hullMaxs = GetHullMaxs();
			hullMaxs.x += checkSize;

			NDebugOverlay::BoxDirection( startPos, GetHullMins(), hullMaxs, BodyDirection2D(), 100, 255, 255, 20, 1.0f );
		}

		//NDebugOverlay::Box3D( startPos, endPos, BodyDirection2D(), 
		if ( m_hChargeTarget && m_hChargeTarget->IsAlive() == false )
		{
			m_hChargeTarget = NULL;
			m_hChargeTargetPosition = NULL;
		}

		// Cause a shock wave from this point which will distrupt nearby physics objects
		ImpactShock( tr.endpos, 200, 500 );
		return;
	}

	if ( pEvent->event == AE_charger_SHOVE )
	{
		EmitSound("NPC_Charger.StepLight", pEvent->eventtime );
		Shove();
		return;
	}

	if ( pEvent->event == AE_charger_FOOTSTEP_LIGHT )
	{
		if ( HasSpawnFlags(SF_charger_INSIDE_FOOTSTEPS) )
		{
			Footstep( false );
		}
		else
		{
			Footstep( false );
		}
		// BJ: Spit here?	// Down???
		//DevMsg("SpitFire Event Light \n");
		//SpitFire();
		return;
	}

	if ( pEvent->event == AE_charger_FOOTSTEP_HEAVY )
	{
		static bool bChargeActivateSpecials;

		if ( HasSpawnFlags(SF_charger_INSIDE_FOOTSTEPS) )
		{
			Footstep( true );
		}
		else
		{
			Footstep( true );
		}

		if (m_iBleedingLevel == 2)
		{
			if (bChargeActivateSpecials)
			{
				// BJ: Spit here?	// UP
				DevMsg("SpitFire Special\n");
				SpitFire();
			}

			bChargeActivateSpecials = !bChargeActivateSpecials;
		}

		return;
	}
	
	if ( pEvent->event == AE_charger_VOICE_GROWL )
	{
		StartSounds();

		if (m_iBleedingLevel != 2)
		{
			// BJ: Spit here?
			DevMsg("SpitFire VOICE_GROWL \n");
			SpitFire();
		}

		float duration = 0.0f;

		if ( random->RandomInt( 0, 10 ) < 6 )
		{
			duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerFastGrowl, ARRAYSIZE(envchargerFastGrowl) );
		}
		else
		{
			duration = 1.0f;
			EmitSound( "NPC_charger.FrustratedRoar" );
			ENVELOPE_CONTROLLER.SoundFadeOut( m_pGrowlHighSound, 0.5f, false );
		}
		
		m_flAngerNoiseTime = gpGlobals->curtime + duration + random->RandomFloat( 2.0f, 4.0f );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, 0.1f );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );

		m_flBreathTime = gpGlobals->curtime + duration - 0.2f;

		EmitSound( "NPC_Charger.ChargeEnemy" );
		return;
	}
		

	if ( pEvent->event == AE_charger_VOICE_BARK )
	{
		StartSounds();

		// BJ: Spit here?
		//DevMsg("SpitFire VOICE_BARK \n");

		float duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerBark1, ARRAYSIZE(envchargerBark1) );
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pConfusedSound, SOUNDCTRL_CHANGE_VOLUME, envchargerBark2, ARRAYSIZE(envchargerBark2) );
		
		m_flAngerNoiseTime = gpGlobals->curtime + duration + random->RandomFloat( 2.0f, 4.0f );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, 0.1f );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );
		
		m_flBreathTime = gpGlobals->curtime + duration - 0.2f;
		return;
	}
	
	if ( pEvent->event == AE_charger_VOICE_ROAR )
	{
		StartSounds();

		if (random->RandomInt(0, 4) != 4)	// BJ: 1 to 4 chance to NOT spit, only roar
		{
			// BJ: Spit here?
			DevMsg("SpitFire VOICE_ROAR \n");
			SpitFire();
		}

		float duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerFastGrowl, ARRAYSIZE(envchargerFastGrowl) );
		
		m_flAngerNoiseTime = gpGlobals->curtime + duration + random->RandomFloat( 2.0f, 4.0f );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, 0.1f );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );

		m_flBreathTime = gpGlobals->curtime + duration - 0.2f;

		EmitSound( "NPC_Charger.Roar" );
		return;
	}

	if ( pEvent->event == AE_charger_VOICE_PAIN )
	{
		StartSounds();

		float duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pConfusedSound, SOUNDCTRL_CHANGE_VOLUME, envchargerPain1, ARRAYSIZE(envchargerPain1) );
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerBark2, ARRAYSIZE(envchargerBark2) );
		
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, 0.1f );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );
		
		m_flBreathTime = gpGlobals->curtime + duration - 0.2f;
		return;
	}

	if ( pEvent->event == AE_charger_VOICE_SQUEEZE )
	{	
		StartSounds();

		float duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerSqueeze, ARRAYSIZE(envchargerSqueeze) );
		
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.6f, random->RandomFloat( 2.0f, 4.0f ) );
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pBreathSound, random->RandomInt( 60, 80 ), random->RandomFloat( 2.0f, 4.0f ) );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );

		m_flBreathTime = gpGlobals->curtime + ( duration * 0.5f );

		EmitSound( "NPC_Charger.ChargeEnemy" );
		return;
	}

	if ( pEvent->event == AE_charger_VOICE_SCRATCH )
	{	
		StartSounds();

		float duration = random->RandomFloat( 2.0f, 4.0f );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.6f, duration );
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pBreathSound, random->RandomInt( 60, 80 ), duration );

		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );

		m_flBreathTime = gpGlobals->curtime + duration;

		EmitSound( "NPC_Charger.ChargeEnemy" );
		return;
	}
		
	if ( pEvent->event == AE_charger_VOICE_GRUNT )
	{	
		StartSounds();

		float duration = ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pConfusedSound, SOUNDCTRL_CHANGE_VOLUME, envchargerGrunt, ARRAYSIZE(envchargerGrunt) );
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pGrowlHighSound, SOUNDCTRL_CHANGE_VOLUME, envchargerGrunt2, ARRAYSIZE(envchargerGrunt2) );
		
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, 0.1f );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 0.1f );

		m_flBreathTime = gpGlobals->curtime + duration;
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::SetHeavyDamageAnim( const Vector &vecSource )
{
	Vector vFacing = BodyDirection2D();
	Vector vDamageDir = ( vecSource - WorldSpaceCenter() );

	vDamageDir.z = 0.0f;

	VectorNormalize( vDamageDir );

	Vector vDamageRight, vDamageUp;
	VectorVectors( vDamageDir, vDamageRight, vDamageUp );

	float damageDot = DotProduct( vFacing, vDamageDir );
	float damageRightDot = DotProduct( vFacing, vDamageRight );

	// See if it's in front
	if ( damageDot > 0.0f )
	{
		// See if it's right
		if ( damageRightDot > 0.0f )
		{
			m_nFlinchActivity = ACT_charger_PHYSHIT_FR;
		}
		else
		{
			m_nFlinchActivity = ACT_charger_PHYSHIT_FL;
		}
	}
	else
	{
		// Otherwise it's from behind
		if ( damageRightDot < 0.0f )
		{
			m_nFlinchActivity = ACT_charger_PHYSHIT_RR;
		}
		else
		{
			m_nFlinchActivity = ACT_charger_PHYSHIT_RL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
int CNPC_charger::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo dInfo = info;

	// BJ: Added wave logic
	if ((m_iBleedingLevel == 1) && (!bChargeWaveRoarActivate))
	{
		DevMsg("First Wave Roar Activate \n");
		WaveRoarAttack();
		bChargeWaveRoarActivate = true;
	}
	else if ((m_iBleedingLevel == 2) && (!bChargeWaveOnce))
	{
		DevMsg("Second Wave Roar Activate \n");
		WaveRoarAttack();
		bChargeWaveOnce = true;
	}	//end


	// Don't take damage from another charger.
	if ( dInfo.GetAttacker() && dInfo.GetAttacker() != this && FClassnameIs( dInfo.GetAttacker(), "npc_charger" ) )
		return 0;

	if ( ( dInfo.GetDamageType() & DMG_CRUSH ) && !( dInfo.GetDamageType() & DMG_VEHICLE ) )
	{
		// Don't take damage from physics objects that weren't thrown by the player.
		CBaseEntity *pInflictor = dInfo.GetInflictor();

		IPhysicsObject *pObj = pInflictor->VPhysicsGetObject();
		if ( !pObj || !( pObj->GetGameFlags() & FVPHYSICS_WAS_THROWN ) )
		{
			return 0;
		}
	}

	// Hack to make antlion guard harder in HARD
	if ( g_pGameRules->IsSkillLevel(SKILL_HARD) && !(info.GetDamageType() & DMG_CRUSH) )
	{
		dInfo.SetDamage( dInfo.GetDamage() * 0.75 );
	}

	// Cap damage taken by crushing (otherwise we can get crushed oddly)
	if ( ( dInfo.GetDamageType() & DMG_CRUSH ) && dInfo.GetDamage() > 100 )
	{
		dInfo.SetDamage( 100 );
	}

	// Only take damage from what we classify as heavy damages (explosions, refrigerators, etc)
	if ( IsHeavyDamage( dInfo ) )
	{
		// Always take a set amount of damage from a combine ball
		if ( info.GetInflictor() && UTIL_IsCombineBall( info.GetInflictor() ) )
		{
			dInfo.SetDamage( 50 );
		}

		UTIL_ScreenShake( GetAbsOrigin(), 32.0f, 8.0f, 0.5f, 512, SHAKE_START );

		// Set our response animation
		SetHeavyDamageAnim( dInfo.GetDamagePosition() );

		// Explosive barrels don't carry through their attacker, so this 
		// condition isn't set, and we still want to flinch. So we set it.
		SetCondition( COND_HEAVY_DAMAGE );

		// TODO: Make this its own effect!
		CEffectData data;
		data.m_vOrigin = dInfo.GetDamagePosition();
		data.m_vNormal = -dInfo.GetDamageForce();
		VectorNormalize( data.m_vNormal );
		DispatchEffect( "HunterDamage", data );

		// Play a sound for a physics impact
		if ( dInfo.GetDamageType() & DMG_CRUSH )
		{
			EmitSound("NPC_Charger.ShellCrack");
		}

		// Roar in pain
		EmitSound( "NPC_Charger.Hurt" );
	}
	
	int nPreHealth = GetHealth();
	int nDamageTaken = BaseClass::OnTakeDamage_Alive( dInfo );

	// See if we've crossed a measured phase in our health and flinch from that to show we do take damage
	if ( HasCondition( COND_HEAVY_DAMAGE ) == false && ( info.GetDamageType() & DMG_BULLET ) )
	{
		bool bTakeHeavyDamage = false;

		// Do an early flinch so that players understand the guard can be hurt by 
		if ( ( (float) GetHealth() / (float) GetMaxHealth() ) > 0.9f )
		{
			float flPrePerc = (float) nPreHealth / (float) GetMaxHealth();
			float flPostPerc = (float) GetHealth() / (float) GetMaxHealth();
			if ( flPrePerc >= 0.95f && flPostPerc < 0.95f )
			{
				bTakeHeavyDamage = true;
			}
		}

		// Otherwise see if we've passed a measured point in our health
		if ( bTakeHeavyDamage == false )
		{
			const float flNumDamagePhases =	5.0f;

			float flDenom = ( (float) GetMaxHealth() / flNumDamagePhases );
			int nPreDamagePhase = ceil( (float) nPreHealth / flDenom );
			int nPostDamagePhase = ceil( (float) GetHealth() / flDenom );
			if ( nPreDamagePhase != nPostDamagePhase )
			{
				bTakeHeavyDamage = true;
			}
		}

		// Flinch if we should
		if ( bTakeHeavyDamage )
		{
			// Set our response animation
			SetHeavyDamageAnim( dInfo.GetDamagePosition() );

			// Roar in pain
			EmitSound( "NPC_Charger.Hurt" );

			// Flinch!
			SetCondition( COND_HEAVY_DAMAGE );
		}
	}

	return nDamageTaken;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pAttacker - 
//			flDamage - 
//			&vecDir - 
//			*ptr - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CNPC_charger::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	CTakeDamageInfo info = inputInfo;

	// Bullets are weak against us, buckshot less so
	if ( info.GetDamageType() & DMG_BUCKSHOT )
	{
		info.ScaleDamage( 0.5f );
	}
	else if ( info.GetDamageType() & DMG_BULLET )
	{
		info.ScaleDamage( 0.25f );
	}

	// Make sure we haven't rounded down to a minimal amount
	if ( info.GetDamage() < 1.0f )
	{
		info.SetDamage( 1.0f );
	}

	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_charger::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_charger_SET_FLINCH_ACTIVITY:
		SetIdealActivity( (Activity) m_nFlinchActivity );
		break;

	case TASK_charger_SHOVE_PHYSOBJECT:
		{
			if ( ( m_hPhysicsTarget == NULL ) || ( GetEnemy() == NULL ) )
			{
				TaskFail( "Tried to shove a NULL physics target!\n" );
				break;
			}
			
			//Get the direction and distance to our thrown object
			Vector	dirToTarget = ( m_hPhysicsTarget->WorldSpaceCenter() - WorldSpaceCenter() );
			float	distToTarget = VectorNormalize( dirToTarget );
			dirToTarget.z = 0;

			//Validate it's still close enough to shove
			//FIXME: Real numbers
			if ( distToTarget > 256.0f )
			{
				RememberFailedPhysicsTarget( m_hPhysicsTarget );
				m_hPhysicsTarget = NULL;

				TaskFail( "Shove target moved\n" );
				break;
			}

			//Validate its offset from our facing
			float targetYaw = UTIL_VecToYaw( dirToTarget );
			float offset = UTIL_AngleDiff( targetYaw, UTIL_AngleMod( GetLocalAngles().y ) );

			if ( fabs( offset ) > 55 )
			{
				RememberFailedPhysicsTarget( m_hPhysicsTarget );
				m_hPhysicsTarget = NULL;

				TaskFail( "Shove target off-center\n" );
				break;
			}

			//Blend properly
			SetPoseParameter( m_poseThrow, offset );

			//Start playing the animation
			SetActivity( ACT_charger_SHOVE_PHYSOBJECT );
		}
		break;

	case TASK_charger_FIND_PHYSOBJECT:
		{

			// Force the antlion guard to find a physobject
			m_flPhysicsCheckTime = 0;
			UpdatePhysicsTarget( false, (100*12) );
			if ( m_hPhysicsTarget )
			{
				TaskComplete();
			}
			else
			{
				TaskFail( "Failed to find a physobject.\n" );
			}
		}
		break;

	case TASK_charger_GET_PATH_TO_PHYSOBJECT:
		{
			if ( ( m_hPhysicsTarget == NULL ) || ( GetEnemy() == NULL ) )
			{
				TaskFail( "Tried to find a path to NULL physics target!\n" );
				break;
			}
			
			Vector vecGoalPos = m_vecPhysicsHitPosition; 
			AI_NavGoal_t goal( GOALTYPE_LOCATION, vecGoalPos, ACT_RUN );

			if ( GetNavigator()->SetGoal( goal ) )
			{
				if ( g_debug_charger.GetInt() == 1 )
				{
					NDebugOverlay::Cross3D( vecGoalPos, Vector(8,8,8), -Vector(8,8,8), 0, 255, 0, true, 2.0f );
					NDebugOverlay::Line( vecGoalPos, m_hPhysicsTarget->WorldSpaceCenter(), 0, 255, 0, true, 2.0f );
					NDebugOverlay::Line( m_hPhysicsTarget->WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), 0, 255, 0, true, 2.0f );
				}

				// Face the enemy
				GetNavigator()->SetArrivalDirection( GetEnemy() );
				TaskComplete();
			}
			else
			{
				if ( g_debug_charger.GetInt() == 1 )
				{
					NDebugOverlay::Cross3D( vecGoalPos, Vector(8,8,8), -Vector(8,8,8), 255, 0, 0, true, 2.0f );
					NDebugOverlay::Line( vecGoalPos, m_hPhysicsTarget->WorldSpaceCenter(), 255, 0, 0, true, 2.0f );
					NDebugOverlay::Line( m_hPhysicsTarget->WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), 255, 0, 0, true, 2.0f );
				}

				RememberFailedPhysicsTarget( m_hPhysicsTarget );
				m_hPhysicsTarget = NULL;
				TaskFail( "Unable to navigate to physics attack target!\n" );
				break;
			}

			//!!!HACKHACK - this is a hack that covers a bug in antlion guard physics target selection! (Tracker #77601)
			// This piece of code (combined with some code in GatherConditions) COVERS THE BUG by escaping the schedule
			// if 30 seconds have passed (it should never take this long for the guard to get to an object and hit it).
			// It's too scary to figure out why this particular antlion guard can't get to its object, but we're shipping
			// like, tomorrow. (sjb) 8/22/2007
			m_flWaitFinished = gpGlobals->curtime + 30.0f;
		}
		break;

	case TASK_charger_CHARGE:
		{
			// HACK: Because the guard stops running his normal blended movement 
			//		 here, he also needs to remove his blended movement layers!
			GetMotor()->MoveStop();

			SetActivity( ACT_charger_CHARGE_START );
			m_bDecidedNotToStop = false;
		}
		break;


	case TASK_charger_GET_PATH_TO_CHARGE_POSITION:
		{
			if ( !m_hChargeTargetPosition )
			{
				TaskFail( "Tried to find a charge position without one specified.\n" );
				break;
			}

			// Move to the charge position
			AI_NavGoal_t goal( GOALTYPE_LOCATION, m_hChargeTargetPosition->GetAbsOrigin(), ACT_RUN );
			if ( GetNavigator()->SetGoal( goal ) )
			{
				// We want to face towards the charge target
				Vector vecDir = m_hChargeTarget->GetAbsOrigin() - m_hChargeTargetPosition->GetAbsOrigin();
				VectorNormalize( vecDir );
				vecDir.z = 0;
				GetNavigator()->SetArrivalDirection( vecDir );
				TaskComplete();
			}
			else
			{
				m_hChargeTarget = NULL;
				m_hChargeTargetPosition = NULL;
				TaskFail( FAIL_NO_ROUTE );
			}
		}
		break;

	case TASK_charger_GET_PATH_TO_NEAREST_NODE:
		{
			if ( !GetEnemy() )
			{
				TaskFail( FAIL_NO_ENEMY );
				break;
			}

			// Find the nearest node to the enemy
			int node = GetNavigator()->GetNetwork()->NearestNodeToPoint( this, GetEnemy()->GetAbsOrigin(), false );
			CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode( node );
			if( pNode == NULL )
			{
				TaskFail( FAIL_NO_ROUTE );
				break;
			}

			Vector vecNodePos = pNode->GetPosition( GetHullType() );
			AI_NavGoal_t goal( GOALTYPE_LOCATION, vecNodePos, ACT_RUN );
			if ( GetNavigator()->SetGoal( goal ) )
			{
				GetNavigator()->SetArrivalDirection( GetEnemy() );
				TaskComplete();
				break;
			}


			TaskFail( FAIL_NO_ROUTE );
			break;
		}
		break;

	case TASK_charger_GET_CHASE_PATH_ENEMY_TOLERANCE:
		{
			// Chase the enemy, but allow local navigation to succeed if it gets within the goal tolerance
			GetNavigator()->SetLocalSucceedOnWithinTolerance( true );

			if ( GetNavigator()->SetGoal( GOALTYPE_ENEMY ) )
			{
				TaskComplete();
			}
			else
			{
				RememberUnreachable(GetEnemy());
				TaskFail(FAIL_NO_ROUTE);
			}

			GetNavigator()->SetLocalSucceedOnWithinTolerance( false );
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate & apply damage & force for a charge to a target.
//			Done outside of the guard because we need to do this inside a trace filter.
//-----------------------------------------------------------------------------
void ApplyDogChargeDamage( CBaseEntity *pcharger, CBaseEntity *pTarget, float flDamage )
{
	Vector attackDir = ( pTarget->WorldSpaceCenter() - pcharger->WorldSpaceCenter() );
	VectorNormalize( attackDir );
	Vector offset = RandomVector( -32, 32 ) + pTarget->WorldSpaceCenter();

	// Generate enough force to make a 75kg guy move away at 700 in/sec
	Vector vecForce = attackDir * ImpulseScale( 75, 700 );

	// Deal the damage
	CTakeDamageInfo	info( pcharger, pcharger, vecForce, offset, flDamage, DMG_CLUB );
	pTarget->TakeDamage( info );

}

//-----------------------------------------------------------------------------
// Purpose: A simple trace filter class to skip small moveable physics objects
//-----------------------------------------------------------------------------
class CTraceFilterSkipPhysics : public CTraceFilter
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterSkipPhysics );
	
	CTraceFilterSkipPhysics( const IHandleEntity *passentity, int collisionGroup, float minMass )
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_minMass(minMass)
	{
	}
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		if ( pEntity )
		{
			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;
			
			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			// don't test small moveable physics objects (unless it's an NPC)
			if ( !pEntity->IsNPC() && pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
				if ( pPhysics != NULL && pPhysics->IsMoveable() && pPhysics->GetMass() < m_minMass)
					return false;
			}

			// If we hit an antlion, don't stop, but kill it
			if ( pEntity->Classify() == CLASS_ANTLION )
			{
				CBaseEntity *pGuard = (CBaseEntity*)EntityFromEntityHandle( m_pPassEnt );
				ApplyDogChargeDamage( pGuard, pEntity, pEntity->GetHealth() );
				return false;
			}
		}

		return true;
	}



private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
	float m_minMass;
};

inline void TraceHull_SkipPhysics( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr, float minMass )
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd, hullMin, hullMax );
	CTraceFilterSkipPhysics traceFilter( ignore, collisionGroup, minMass );
	enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if our charge target is right in front of the guard
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::EnemyIsRightInFrontOfMe( CBaseEntity **pEntity )
{
	if ( !GetEnemy() )
		return false;

	if ( (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).LengthSqr() < (156*156) )
	{
		Vector vecLOS = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
		vecLOS.z = 0;
		VectorNormalize( vecLOS );
		Vector vBodyDir = BodyDirection2D();
		if ( DotProduct( vecLOS, vBodyDir ) > 0.8 )
		{
			// He's in front of me, and close. Make sure he's not behind a wall.
			trace_t tr;
			UTIL_TraceLine( WorldSpaceCenter(), GetEnemy()->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
			if ( tr.m_pEnt == GetEnemy() )
			{
				*pEntity = tr.m_pEnt;
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: While charging, look ahead and see if we're going to run into anything.
//			If we are, start the gesture so it looks like we're anticipating the hit.
//-----------------------------------------------------------------------------
void CNPC_charger::ChargeLookAhead( void )
{
	trace_t	tr;
	Vector vecForward;
	GetVectors( &vecForward, NULL, NULL );
	Vector vecTestPos = GetAbsOrigin() + ( vecForward * m_flGroundSpeed * 0.75 );
	Vector testHullMins = GetHullMins();
	testHullMins.z += (StepHeight() * 2);
	TraceHull_SkipPhysics( GetAbsOrigin(), vecTestPos, testHullMins, GetHullMaxs(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5 );

	//NDebugOverlay::Box( tr.startpos, testHullMins, GetHullMaxs(), 0, 255, 0, true, 0.1f );
	//NDebugOverlay::Box( vecTestPos, testHullMins, GetHullMaxs(), 255, 0, 0, true, 0.1f );

	if ( tr.fraction != 1.0 )
	{
		// Start playing the hit animation
		//AddGesture( ACT_charger_CHARGE_ANTICIPATION );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles the guard charging into something. Returns true if it hit the world.
//-----------------------------------------------------------------------------
bool CNPC_charger::HandleChargeImpact( Vector vecImpact, CBaseEntity *pEntity )
{
	// Cause a shock wave from this point which will disrupt nearby physics objects
	ImpactShock( vecImpact, 128, 350 );

	// Did we hit anything interesting?
	if ( !pEntity || pEntity->IsWorld() )
	{
		// Robin: Due to some of the finicky details in the motor, the guard will hit
		//		  the world when it is blocked by our enemy when trying to step up 
		//		  during a moveprobe. To get around this, we see if the enemy's within
		//		  a volume in front of the guard when we hit the world, and if he is,
		//		  we hit him anyway.
		EnemyIsRightInFrontOfMe( &pEntity );

		// Did we manage to find him? If not, increment our charge miss count and abort.
		if ( pEntity->IsWorld() )
		{
			m_iChargeMisses++;
			return true;
		}
	}

	// Hit anything we don't like
	if ( IRelationType( pEntity ) == D_HT && ( GetNextAttack() < gpGlobals->curtime ) )
	{
		EmitSound( "NPC_Charger.ChargeHitObject" );

		if ( !IsPlayingGesture( ACT_charger_CHARGE_HIT ) )
		{
			RestartGesture( ACT_charger_CHARGE_HIT );
		}
		
		ChargeDamage( pEntity );
		
		pEntity->ApplyAbsVelocityImpulse( ( BodyDirection2D() * 400 ) + Vector( 0, 0, 200 ) );

		if ( !pEntity->IsAlive() && GetEnemy() == pEntity )
		{
			SetEnemy( NULL );
		}

		SetNextAttack( gpGlobals->curtime + 2.0f );
		SetActivity( ACT_charger_CHARGE_STOP );

		// We've hit something, so clear our miss count
		m_iChargeMisses = 0;
		return false;
	}

	// Hit something we don't hate. If it's not moveable, crash into it.
	if ( pEntity->GetMoveType() == MOVETYPE_NONE || pEntity->GetMoveType() == MOVETYPE_PUSH )
		return true;

	// If it's a vphysics object that's too heavy, crash into it too.
	if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics )
		{
			// If the object is being held by the player, knock it out of his hands
			if ( pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
			{
				Pickup_ForcePlayerToDropThisObject( pEntity );
				return false;
			}

			if ( (!pPhysics->IsMoveable() || pPhysics->GetMass() > VPhysicsGetObject()->GetMass() * 0.5f ) )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_charger::ChargeSteer( void )
{
	trace_t	tr;
	Vector	testPos, steer, forward, right;
	QAngle	angles;
	const float	testLength = m_flGroundSpeed * 0.15f;

	//Get our facing
	GetVectors( &forward, &right, NULL );

	steer = forward;

	const float faceYaw	= UTIL_VecToYaw( forward );

	//Offset right
	VectorAngles( forward, angles );
	angles[YAW] += 45.0f;
	AngleVectors( angles, &forward );

	//Probe out
	testPos = GetAbsOrigin() + ( forward * testLength );

	//Offset by step height
	Vector testHullMins = GetHullMins();
	testHullMins.z += (StepHeight() * 2);

	//Probe
	TraceHull_SkipPhysics( GetAbsOrigin(), testPos, testHullMins, GetHullMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5f );

	//Debug info
	if ( g_debug_charger.GetInt() == 1 )
	{
		if ( tr.fraction == 1.0f )
		{
  			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 0, 255, 0, 8, 0.1f );
   		}
   		else
   		{
  			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 255, 0, 0, 8, 0.1f );
		}
	}

	//Add in this component
	steer += ( right * 0.5f ) * ( 1.0f - tr.fraction );

	//Offset left
	angles[YAW] -= 90.0f;
	AngleVectors( angles, &forward );

	//Probe out
	testPos = GetAbsOrigin() + ( forward * testLength );

	// Probe
	TraceHull_SkipPhysics( GetAbsOrigin(), testPos, testHullMins, GetHullMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr, VPhysicsGetObject()->GetMass() * 0.5f );

	//Debug
	if ( g_debug_charger.GetInt() == 1 )
	{
		if ( tr.fraction == 1.0f )
		{
			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 0, 255, 0, 8, 0.1f );
		}
		else
		{
			NDebugOverlay::BoxDirection( GetAbsOrigin(), testHullMins, GetHullMaxs() + Vector(testLength,0,0), forward, 255, 0, 0, 8, 0.1f );
		}
	}

	//Add in this component
	steer -= ( right * 0.5f ) * ( 1.0f - tr.fraction );

	//Debug
	if ( g_debug_charger.GetInt() == 1 )
	{
		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + ( steer * 512.0f ), 255, 255, 0, true, 0.1f );
		NDebugOverlay::Cross3D( GetAbsOrigin() + ( steer * 512.0f ), Vector(2,2,2), -Vector(2,2,2), 255, 255, 0, true, 0.1f );

		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + ( BodyDirection3D() * 256.0f ), 255, 0, 255, true, 0.1f );
		NDebugOverlay::Cross3D( GetAbsOrigin() + ( BodyDirection3D() * 256.0f ), Vector(2,2,2), -Vector(2,2,2), 255, 0, 255, true, 0.1f );
	}

	return UTIL_AngleDiff( UTIL_VecToYaw( steer ), faceYaw );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_charger::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
	case TASK_charger_SET_FLINCH_ACTIVITY:
		
		AutoMovement( );
		
		if ( IsActivityFinished() )
		{
			TaskComplete();
		}
		break;

	case TASK_charger_SHOVE_PHYSOBJECT:
	
		if ( IsActivityFinished() )
		{
			TaskComplete();
		}

		break;

	case TASK_charger_CHARGE:
		{
			Activity eActivity = GetActivity();

			// See if we're trying to stop after hitting/missing our target
			if ( eActivity == ACT_charger_CHARGE_STOP || eActivity == ACT_charger_CHARGE_CRASH ) 
			{
				if ( IsActivityFinished() )
				{
					TaskComplete();
					return;
				}

				// Still in the process of slowing down. Run movement until it's done.
				AutoMovement();
				return;
			}

			// Check for manual transition
			if ( ( eActivity == ACT_charger_CHARGE_START ) && ( IsActivityFinished() ) )
			{
				SetActivity( ACT_charger_CHARGE_RUN );
			}

			// See if we're still running
			if ( eActivity == ACT_charger_CHARGE_RUN || eActivity == ACT_charger_CHARGE_START ) 
			{
				if ( HasCondition( COND_NEW_ENEMY ) || HasCondition( COND_LOST_ENEMY ) || HasCondition( COND_ENEMY_DEAD ) )
				{
					SetActivity( ACT_charger_CHARGE_STOP );
					return;
				}
				else 
				{
					if ( GetEnemy() != NULL )
					{
						Vector	goalDir = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
						VectorNormalize( goalDir );

						if ( DotProduct( BodyDirection2D(), goalDir ) < 0.25f )
						{
							if ( !m_bDecidedNotToStop )
							{
								// We've missed the target. Randomly decide not to stop, which will cause
								// the guard to just try and swing around for another pass.
								m_bDecidedNotToStop = true;
								if ( RandomFloat(0,1) > 0.3 )
								{
									m_iChargeMisses++;
									SetActivity( ACT_charger_CHARGE_STOP );
								}
							}
						}
						else
						{
							m_bDecidedNotToStop = false;
						}
					}
				}
			}

			// Steer towards our target
			float idealYaw;
			if ( GetEnemy() == NULL )
			{
				idealYaw = GetMotor()->GetIdealYaw();
			}
			else
			{
				idealYaw = CalcIdealYaw( GetEnemy()->GetAbsOrigin() );
			}

			// Add in our steering offset
			idealYaw += ChargeSteer();
			
			// Turn to face
			GetMotor()->SetIdealYawAndUpdate( idealYaw );

			// See if we're going to run into anything soon
			ChargeLookAhead();

			// Let our animations simply move us forward. Keep the result
			// of the movement so we know whether we've hit our target.
			AIMoveTrace_t moveTrace;
			if ( AutoMovement( GetEnemy(), &moveTrace ) == false )
			{
				// Only stop if we hit the world
				if ( HandleChargeImpact( moveTrace.vEndPosition, moveTrace.pObstruction ) )
				{
					// If we're starting up, this is an error
					if ( eActivity == ACT_charger_CHARGE_START )
					{
						TaskFail( "Unable to make initial movement of charge\n" );
						return;
					}

					// Crash unless we're trying to stop already
					if ( eActivity != ACT_charger_CHARGE_STOP )
					{
						if ( moveTrace.fStatus == AIMR_BLOCKED_WORLD && moveTrace.vHitNormal == vec3_origin )
						{
							SetActivity( ACT_charger_CHARGE_STOP );
						}
						else
						{
							SetActivity( ACT_charger_CHARGE_CRASH );
						}
					}
				}
				else if ( moveTrace.pObstruction )
				{
					// If we hit an antlion, don't stop, but kill it
					if ( moveTrace.pObstruction->Classify() == CLASS_ANTLION )
					{
						if ( FClassnameIs( moveTrace.pObstruction, "npc_charger" ) )
						{
							// Crash unless we're trying to stop already
							if ( eActivity != ACT_charger_CHARGE_STOP )
							{
								SetActivity( ACT_charger_CHARGE_STOP );
							}
						}
						else
						{
							ApplyDogChargeDamage( this, moveTrace.pObstruction, moveTrace.pObstruction->GetHealth() );
						}
					}
				}
			}
		}
		break;

	case TASK_WAIT_FOR_MOVEMENT:
	{

			// See if we're going to run into anything soon
			ChargeLookAhead();

			if ( HasCondition(COND_CAN_MELEE_ATTACK1) )
			{


				CBaseEntity *pEntity = GetEnemy();

				if (pEntity && pEntity->IsPlayer())
				{
					EmitSound( "NPC_Charger.ChargeHitObject" );

					if ( !IsPlayingGesture( ACT_charger_CHARGE_HIT ) )
					{
						RestartGesture( ACT_charger_CHARGE_HIT );
					}
					
					ChargeDamage( pEntity );
					
					pEntity->ApplyAbsVelocityImpulse( ( BodyDirection2D() * 400 ) + Vector( 0, 0, 200 ) );

					if ( !pEntity->IsAlive() && GetEnemy() == pEntity )
					{
						SetEnemy( NULL );
					}

					SetNextAttack( gpGlobals->curtime + 2.0f );
					SetActivity( ACT_charger_CHARGE_STOP );

					// We've hit something, so clear our miss count
					m_iChargeMisses = 0;

					AutoMovement();
					TaskComplete();
					return;
				}
			}
		//}

		BaseClass::RunTask( pTask );

	}
	break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::FoundEnemy( void )
{
	m_flAngerNoiseTime = gpGlobals->curtime + 2.0f;
	SetState( NPC_STATE_COMBAT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::LostEnemy( void )
{
	m_flSearchNoiseTime = gpGlobals->curtime + 2.0f;
	SetState( NPC_STATE_ALERT );

	m_OnLostPlayer.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::InputSetShoveTarget( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );

	if ( pTarget == NULL )
	{
		Warning( "**Guard %s cannot find shove target %s\n", GetClassname(), inputdata.value.String() );
		m_hShoveTarget = NULL;
		return;
	}

	m_hShoveTarget = pTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::InputSetChargeTarget( inputdata_t &inputdata )
{
	if ( !IsAlive() )
		return;

	// Pull the target & position out of the string
	char parseString[255];
	Q_strncpy(parseString, inputdata.value.String(), sizeof(parseString));

	// Get charge target name
	char *pszParam = strtok(parseString," ");
	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, pszParam, NULL, inputdata.pActivator, inputdata.pCaller );
	if ( !pTarget )
	{
		Warning( "ERROR: Guard %s cannot find charge target '%s'\n", STRING(GetEntityName()), pszParam );
		return;
	}

	// Get the charge position name
	pszParam = strtok(NULL," ");
	CBaseEntity *pPosition = gEntList.FindEntityByName( NULL, pszParam, NULL, inputdata.pActivator, inputdata.pCaller );
	if ( !pPosition )
	{
		Warning( "ERROR: Guard %s cannot find charge position '%s'\nMake sure you've specified the parameters as [target start]!\n", STRING(GetEntityName()), pszParam );
		return;
	}

	// Make sure we don't stack charge targets
	if ( m_hChargeTarget )
	{
		if ( GetEnemy() == m_hChargeTarget )
		{
			SetEnemy( NULL );
		}
	}

	SetCondition( COND_charger_HAS_CHARGE_TARGET );
	m_hChargeTarget = pTarget;
	m_hChargeTargetPosition = pPosition;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_charger::InputClearChargeTarget( inputdata_t &inputdata )
{
	m_hChargeTarget = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : baseAct - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CNPC_charger::NPC_TranslateActivity( Activity baseAct )
{
	//See which run to use
	if ( ( baseAct == ACT_RUN ) && IsCurSchedule( SCHED_charger_CHARGE ) )
		return (Activity) ACT_charger_CHARGE_RUN;

	if ( ( baseAct == ACT_RUN ) && ( m_iHealth <= (m_iMaxHealth/4) ) )
		return (Activity) ACT_charger_RUN_HURT;

	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::ShouldWatchEnemy( void )
{
	Activity nActivity = GetActivity();

	if ( ( nActivity == ACT_charger_SEARCH ) || 
		 ( nActivity == ACT_charger_SHOVE_PHYSOBJECT ) || 
		 ( nActivity == ACT_charger_PHYSHIT_FR ) || 
		 ( nActivity == ACT_charger_PHYSHIT_FL ) || 
		 ( nActivity == ACT_charger_PHYSHIT_RR ) || 
		 ( nActivity == ACT_charger_PHYSHIT_RL ) || 
		 ( nActivity == ACT_charger_CHARGE_CRASH ) || 
		 ( nActivity == ACT_charger_CHARGE_HIT ) /*||
		 ( nActivity == ACT_charger_CHARGE_ANTICIPATION )*/ )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::UpdateHead( void )
{
	float yaw = GetPoseParameter( m_poseHead_Yaw );
	float pitch = GetPoseParameter( m_poseHead_Pitch );

	// If we should be watching our enemy, turn our head
	if ( ShouldWatchEnemy() && ( GetEnemy() != NULL ) )
	{
		Vector	enemyDir = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( enemyDir );
		
		float angle = VecToYaw( BodyDirection3D() );
		float angleDiff = VecToYaw( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + yaw );

		SetPoseParameter( m_poseHead_Yaw, UTIL_Approach( yaw + angleDiff, yaw, 50 ) );

		angle = UTIL_VecToPitch( BodyDirection3D() );
		angleDiff = UTIL_VecToPitch( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + pitch );

		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( pitch + angleDiff, pitch, 50 ) );
	}
	else
	{
		// Otherwise turn the head back to its normal position
		SetPoseParameter( m_poseHead_Yaw,	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( 0, pitch, 10 ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::MaintainPhysicsTarget( void )
{
	if ( m_hPhysicsTarget == NULL || GetEnemy() == NULL )
		return;

	// Update our current target to make sure it's still valid
	float flTargetDistSqr = ( m_hPhysicsTarget->WorldSpaceCenter() - m_vecPhysicsTargetStartPos ).LengthSqr();
	bool bTargetMoved = ( flTargetDistSqr > Square(30*12.0f) );
	bool bEnemyCloser = ( ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr() <= flTargetDistSqr );

	// Make sure this hasn't moved too far or that the player is now closer
	if ( bTargetMoved || bEnemyCloser )
	{
		ClearCondition( COND_charger_PHYSICS_TARGET );
		SetCondition( COND_charger_PHYSICS_TARGET_INVALID );
		m_hPhysicsTarget = NULL;
		return;
	}
	else
	{
		SetCondition( COND_charger_PHYSICS_TARGET );
		ClearCondition( COND_charger_PHYSICS_TARGET_INVALID );
	}

	if ( g_debug_charger.GetInt() == 3 )
	{
		NDebugOverlay::Cross3D( m_hPhysicsTarget->WorldSpaceCenter(), -Vector(32,32,32), Vector(32,32,32), 255, 255, 255, true, 1.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::UpdatePhysicsTarget( bool bPreferObjectsAlongTargetVector, float flRadius )
{
	if ( GetEnemy() == NULL )
		return;

	// Already have a target, don't bother looking
	if ( m_hPhysicsTarget != NULL )
		return;

	// Too soon to check again
	if ( m_flPhysicsCheckTime > gpGlobals->curtime )
		return;

	// Attempt to find a valid shove target
	PhysicsObjectCriteria_t criteria;
	criteria.pTarget = GetEnemy();
	criteria.vecCenter = GetEnemy()->GetAbsOrigin();
	criteria.flRadius = flRadius;
	criteria.flTargetCone = charger_OBJECTFINDING_FOV;
	criteria.bPreferObjectsAlongTargetVector = bPreferObjectsAlongTargetVector;
	criteria.flNearRadius = (20*12); // TODO: It may preferable to disable this for legacy products as well -- jdw

	m_hPhysicsTarget = FindPhysicsObjectTarget( criteria );
		
	// Found one, so interrupt us if we care
	if ( m_hPhysicsTarget != NULL )
	{
		SetCondition( COND_charger_PHYSICS_TARGET );
		m_vecPhysicsTargetStartPos = m_hPhysicsTarget->WorldSpaceCenter();
	}

	// Don't search again for another second
	m_flPhysicsCheckTime = gpGlobals->curtime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Let the probe know I can run through small debris
//-----------------------------------------------------------------------------
bool CNPC_charger::ShouldProbeCollideAgainstEntity( CBaseEntity *pEntity )
{
	if ( m_iszPhysicsPropClass != pEntity->m_iClassname )
		return BaseClass::ShouldProbeCollideAgainstEntity( pEntity );

	if ( m_hPhysicsTarget == pEntity )
		return false;

	if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
	{
		IPhysicsObject *pPhysObj = pEntity->VPhysicsGetObject();

		if( pPhysObj && pPhysObj->GetMass() <= charger_MAX_OBJECT_MASS )
		{
			return false;
		}
	}

	return BaseClass::ShouldProbeCollideAgainstEntity( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// Don't do anything after death
	if ( m_NPCState == NPC_STATE_DEAD )
		return;

	// Automatically update our physics target when chasing enemies
	if ( IsCurSchedule( SCHED_charger_CHASE_ENEMY ) || 
		 IsCurSchedule( SCHED_charger_PATROL_RUN ) ||
		 IsCurSchedule( SCHED_charger_CANT_ATTACK ) ||
		 IsCurSchedule( SCHED_charger_CHASE_ENEMY_TOLERANCE ) )
	{
		bool bCheckAlongLine = ( IsCurSchedule( SCHED_charger_CHASE_ENEMY ) || IsCurSchedule( SCHED_charger_CHASE_ENEMY_TOLERANCE ) );
		UpdatePhysicsTarget( bCheckAlongLine );
	}
	else if ( !IsCurSchedule( SCHED_charger_PHYSICS_ATTACK ) )
	{
		ClearCondition( COND_charger_PHYSICS_TARGET );
		m_hPhysicsTarget = NULL;
	}

	UpdateHead();

	if ( ( m_flGroundSpeed <= 0.0f ) )
	{
		if ( m_bStopped == false )
		{
			StartSounds();

			float duration = random->RandomFloat( 2.0f, 8.0f );

			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.0f, duration );
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pBreathSound, random->RandomInt( 40, 60 ), duration );
			
			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, duration );
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pGrowlIdleSound, random->RandomInt( 120, 140 ), duration );

			m_flBreathTime = gpGlobals->curtime + duration - (duration*0.75f);
		}
		
		m_bStopped = true;

		if ( m_flBreathTime < gpGlobals->curtime )
		{
			StartSounds();

			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, random->RandomFloat( 0.2f, 0.3f ), random->RandomFloat( 0.5f, 1.0f ) );
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pGrowlIdleSound, random->RandomInt( 80, 120 ), random->RandomFloat( 0.5f, 1.0f ) );

			m_flBreathTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 8.0f );
		}
	}
	else
	{
		if ( m_bStopped ) 
		{
			StartSounds();

			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pBreathSound, 0.6f, random->RandomFloat( 2.0f, 4.0f ) );
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pBreathSound, random->RandomInt( 140, 160 ), random->RandomFloat( 2.0f, 4.0f ) );

			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pGrowlIdleSound, 0.0f, 1.0f );
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pGrowlIdleSound, random->RandomInt( 90, 110 ), 0.2f );
		}


		m_bStopped = false;
	}

	// Put danger sounds out in front of us
	for ( int i = 0; i < 3; i++ )
	{
		CSoundEnt::InsertSound( SOUND_DANGER, WorldSpaceCenter() + ( BodyDirection3D() * 128 * (i+1) ), 128, 0.1f, this );		
	}

#if charger_BLOOD_EFFECTS
	// compute and if necessary transmit the bleeding level for the particle effect
	m_iBleedingLevel = GetBleedingLevel();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::GatherConditions( void )
{
	BaseClass::GatherConditions();

	// Make sure our physics target is still valid
	MaintainPhysicsTarget();

	if( IsCurSchedule(SCHED_charger_PHYSICS_ATTACK) )
	{
		if( gpGlobals->curtime > m_flWaitFinished )
		{
			ClearCondition( COND_charger_PHYSICS_TARGET );
			SetCondition( COND_charger_PHYSICS_TARGET_INVALID );
			m_hPhysicsTarget = NULL;
		}
	}

	// See if we can charge the target
	if ( GetEnemy() )
	{
		if ( ShouldCharge( GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), true, false ) )
		{
			SetCondition( COND_charger_CAN_CHARGE );
		}
		else
		{
			ClearCondition( COND_charger_CAN_CHARGE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::StopLoopingSounds()
{
	//Stop all sounds
	ENVELOPE_CONTROLLER.SoundDestroy( m_pGrowlHighSound );
	ENVELOPE_CONTROLLER.SoundDestroy( m_pGrowlIdleSound );
	ENVELOPE_CONTROLLER.SoundDestroy( m_pBreathSound );
	ENVELOPE_CONTROLLER.SoundDestroy( m_pConfusedSound );
	
	
	m_pGrowlHighSound	= NULL;
	m_pGrowlIdleSound	= NULL;
	m_pBreathSound		= NULL;
	m_pConfusedSound	= NULL;

	BaseClass::StopLoopingSounds();
}

//------------------------------------------------------------------------------
// Purpose : Returns true is entity was remembered as unreachable.
//			 After a time delay reachability is checked
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CNPC_charger::IsUnreachable(CBaseEntity *pEntity)
{
	float UNREACHABLE_DIST_TOLERANCE_SQ = (240 * 240);

	// Note that it's ok to remove elements while I'm iterating
	// as long as I iterate backwards and remove them using FastRemove
	for (int i=m_UnreachableEnts.Size()-1;i>=0;i--)
	{
		// Remove any dead elements
		if (m_UnreachableEnts[i].hUnreachableEnt == NULL)
		{
			m_UnreachableEnts.FastRemove(i);
		}
		else if (pEntity == m_UnreachableEnts[i].hUnreachableEnt)
		{
			// Test for reachability on any elements that have timed out
			if ( gpGlobals->curtime > m_UnreachableEnts[i].fExpireTime ||
				pEntity->GetAbsOrigin().DistToSqr(m_UnreachableEnts[i].vLocationWhenUnreachable) > UNREACHABLE_DIST_TOLERANCE_SQ)
			{
				m_UnreachableEnts.FastRemove(i);
				return false;
			}
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Return the point at which the guard wants to stand on to knock the physics object at the target entity
// Input  : *pObject - Object to be shoved.
//			*pTarget - Target to be shoved at.
//			*vecTrajectory - Trajectory to our target
//			*flClearDistance - Distance behind the entity we're clear to use
// Output : Position at which to attempt to strike the object
//-----------------------------------------------------------------------------
Vector CNPC_charger::GetPhysicsHitPosition( CBaseEntity *pObject, CBaseEntity *pTarget, Vector *vecTrajectory, float *flClearDistance )
{
	// Get the trajectory we want to knock the object along
	Vector vecToTarget = pTarget->WorldSpaceCenter() - pObject->WorldSpaceCenter();
	VectorNormalize( vecToTarget );
	vecToTarget.z = 0;

	// Get the distance we want to be from the object when we hit it
	IPhysicsObject *pPhys = pObject->VPhysicsGetObject();
	Vector extent = physcollision->CollideGetExtent( pPhys->GetCollide(), pObject->GetAbsOrigin(), pObject->GetAbsAngles(), -vecToTarget );
	float flDist = ( extent - pObject->WorldSpaceCenter() ).Length() + CollisionProp()->BoundingRadius() + 32.0f;
	
	if ( vecTrajectory != NULL )
	{
		*vecTrajectory = vecToTarget;
	}

	if ( flClearDistance != NULL )
	{
		*flClearDistance = flDist;
	}

	// Position at which we'd like to be
	return (pObject->WorldSpaceCenter() + ( vecToTarget * -flDist ));
}

//-----------------------------------------------------------------------------
// Purpose: See if we're able to stand on the ground at this point
// Input  : &vecPos - Position to try
//			*pOut - Result position (only valid if we return true)
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CNPC_charger::CanStandAtPoint( const Vector &vecPos, Vector *pOut )
{
	Vector vecStart = vecPos + Vector( 0, 0, (4*12) );
	Vector vecEnd = vecPos - Vector( 0, 0, (4*12) );
	trace_t	tr;
	bool bTraceCleared = false;

	// Start high and try to go lower, looking for the ground between here and there
	// We do this first because it's more likely to succeed in the typical guard arenas (with open terrain)
	UTIL_TraceHull( vecStart, vecEnd, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.startsolid && !tr.allsolid )
	{
		// We started in solid but didn't end up there, see if we can stand where we ended up
		UTIL_TraceHull( tr.endpos, tr.endpos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		
		// Must not be in solid
		bTraceCleared = ( !tr.allsolid && !tr.startsolid );
	}
	else
	{
		// Must not be in solid and must have found a floor (otherwise we're potentially hanging over a ledge)
		bTraceCleared = ( tr.allsolid == false && tr.fraction < 1.0f );
	}

	// Either we're clear or we're still unlucky
	if ( bTraceCleared == false )
	{
		if ( g_debug_charger.GetInt() == 3 )
		{
			NDebugOverlay::Box( vecPos, GetHullMins(), GetHullMaxs(), 255, 0, 0, 0, 15.0f );
		}
		return false;
	}

	if ( pOut )
	{
		*pOut = tr.endpos;
	}

	if ( g_debug_charger.GetInt() == 3 )
	{
		NDebugOverlay::Box( (*pOut), GetHullMins(), GetHullMaxs(), 0, 255, 0, 0, 15.0f );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Determines whether or not the guard can stand in a position to strike a specified object
// Input  : *pShoveObject - Object being shoved
//			*pTarget - Target we're shoving the object at
//			*pOut - The position we decide to stand at
// Output : Returns true if the guard can stand and deliver.
//-----------------------------------------------------------------------------
bool CNPC_charger::CanStandAtShoveTarget( CBaseEntity *pShoveObject, CBaseEntity *pTarget, Vector *pOut )
{
	// Get the position we want to be at to swing at the object
	float flClearDistance;
	Vector vecTrajectory;
	Vector vecHitPosition = GetPhysicsHitPosition( pShoveObject, pTarget, &vecTrajectory, &flClearDistance );
	Vector vecStandPosition;

	if ( g_debug_charger.GetInt() == 3 )
	{
		NDebugOverlay::HorzArrow( pShoveObject->WorldSpaceCenter(), pShoveObject->WorldSpaceCenter() + vecTrajectory * 64.0f, 16.0f, 255, 255, 0, 16, true, 15.0f );
	}

	// If we failed, try around the sides
	if ( CanStandAtPoint( vecHitPosition, &vecStandPosition ) == false )
	{
		// Get the angle (in reverse) to the target
		float flRad = atan2( -vecTrajectory.y, -vecTrajectory.x );
		float flRadOffset = DEG2RAD( 45.0f );

		// Build an offset vector, rotating around the base
		Vector vecSkewTrajectory;
		SinCos( flRad + flRadOffset, &vecSkewTrajectory.y, &vecSkewTrajectory.x );
		vecSkewTrajectory.z = 0.0f;
		
		// Try to the side
		if ( CanStandAtPoint( ( pShoveObject->WorldSpaceCenter() + ( vecSkewTrajectory * flClearDistance ) ), &vecStandPosition ) == false )
		{
			// Try the other side
			SinCos( flRad - flRadOffset, &vecSkewTrajectory.y, &vecSkewTrajectory.x );
			vecSkewTrajectory.z = 0.0f;			
			if ( CanStandAtPoint( ( pShoveObject->WorldSpaceCenter() + ( vecSkewTrajectory * flClearDistance ) ), &vecStandPosition ) == false )
				return false;
		}
	}

	// Found it, report it
	if ( pOut != NULL )
	{
		*pOut = vecStandPosition;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Iterate through a number of lists depending on our criteria
//-----------------------------------------------------------------------------
CBaseEntity *CNPC_charger::GetNextShoveTarget( CBaseEntity *pLastEntity, AISightIter_t &iter )
{
	// Try to find scripted items first
	if ( m_strShoveTargets != NULL_STRING )
	{
		CBaseEntity *pFound = gEntList.FindEntityByName( pLastEntity, m_strShoveTargets );
		if ( pFound )
			return pFound;
	}

	// Failing that, use our senses
	if ( iter != (AISightIter_t)(-1) )
		return GetSenses()->GetNextSeenEntity( &iter );

    return GetSenses()->GetFirstSeenEntity( &iter, SEEN_MISC );
}

//-----------------------------------------------------------------------------
// Purpose: Search for a physics item to swat at the player
// Output : Returns the object we're going to swat.
//-----------------------------------------------------------------------------
CBaseEntity *CNPC_charger::FindPhysicsObjectTarget( const PhysicsObjectCriteria_t &criteria )
{
	// Must have a valid target entity
 	if ( criteria.pTarget == NULL )
		return NULL;

	if ( g_debug_charger.GetInt() == 3 )
	{
		NDebugOverlay::Circle( GetAbsOrigin(), QAngle( -90, 0, 0 ), criteria.flRadius, 255, 0, 0, 8, true, 2.0f );
	}

	// Get the vector to our target, from ourself
	Vector vecDirToTarget = criteria.pTarget->GetAbsOrigin() - GetAbsOrigin();
	VectorNormalize( vecDirToTarget );
	vecDirToTarget.z = 0;

	// Cost is determined by distance to the object, modified by how "in line" it is with our target direction of travel
	// Use the distance to the player as the base cost for throwing an object (avoids pushing things too close to the player)
	float flLeastCost = ( criteria.bPreferObjectsAlongTargetVector ) ? ( criteria.pTarget->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr() : Square( criteria.flRadius );
	float flCost;

	AISightIter_t iter = (AISightIter_t)(-1);
	CBaseEntity *pObject = NULL;
	CBaseEntity	*pNearest = NULL;
	Vector vecBestHitPosition = vec3_origin;

	// Look through the list of sensed objects for possible targets
	while( ( pObject = GetNextShoveTarget( pObject, iter ) ) != NULL )
	{
		// If we couldn't shove this object last time, don't try again
		if ( m_FailedPhysicsTargets.Find( pObject ) != m_FailedPhysicsTargets.InvalidIndex() )
			continue;

		// Ignore things less than half a foot in diameter
		if ( pObject->CollisionProp()->BoundingRadius() < 6.0f )
			continue;

		IPhysicsObject *pPhysObj = pObject->VPhysicsGetObject();
		if ( pPhysObj == NULL )
			continue;

		// Ignore motion disabled props
		if ( pPhysObj->IsMoveable() == false )
			continue;

		// Ignore things lighter than 5kg
		if ( pPhysObj->GetMass() < 5.0f )
			continue;

		// Ignore objects moving too quickly (they'll be too hard to catch otherwise)
		Vector	velocity;
		pPhysObj->GetVelocity( &velocity, NULL );
		if ( velocity.LengthSqr() > (16*16) )
			continue;

		// Get the direction from us to the physics object
		Vector vecDirToObject = pObject->WorldSpaceCenter() - GetAbsOrigin();
		VectorNormalize( vecDirToObject );
		vecDirToObject.z = 0;
		
		Vector vecObjCenter = pObject->WorldSpaceCenter();
		float flDistSqr = 0.0f;
		float flDot = 0.0f;

		// If we want to find things along the vector to the target, do so
		if ( criteria.bPreferObjectsAlongTargetVector )
		{
			// Object must be closer than our target
			if ( ( GetAbsOrigin() - vecObjCenter ).LengthSqr() > ( GetAbsOrigin() - criteria.pTarget->GetAbsOrigin() ).LengthSqr() )
				continue;

			// Calculate a "cost" to this object
			flDistSqr = ( GetAbsOrigin() - vecObjCenter ).LengthSqr();
			flDot = DotProduct( vecDirToTarget, vecDirToObject );
			
			// Ignore things outside our allowed cone
			if ( flDot < criteria.flTargetCone )
				continue;

			// The more perpendicular we are, the higher the cost
			float flCostScale = RemapValClamped( flDot, 1.0f, criteria.flTargetCone, 1.0f, 4.0f );
			flCost = flDistSqr * flCostScale;
		}
		else
		{
			// Straight distance cost
			flCost = ( criteria.vecCenter - vecObjCenter ).LengthSqr();
		}

		// This must be a less costly object to use
		if ( flCost >= flLeastCost )
		{
			if ( g_debug_charger.GetInt() == 3 )
			{
				NDebugOverlay::Box( vecObjCenter, -Vector(16,16,16), Vector(16,16,16), 255, 0, 0, 0, 2.0f );
			}

			continue;
		}

		// Check for a (roughly) clear trajectory path from the object to target
		trace_t	tr;
		UTIL_TraceLine( vecObjCenter, criteria.pTarget->BodyTarget( vecObjCenter ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
		
		// See how close to our target we got (we still look good hurling things that won't necessarily hit)
		if ( ( tr.endpos - criteria.pTarget->WorldSpaceCenter() ).LengthSqr() > Square(criteria.flNearRadius) )
			continue;

		// Must be able to stand at a position to hit the object
		Vector vecHitPosition;
		if ( CanStandAtShoveTarget( pObject, criteria.pTarget, &vecHitPosition ) == false )
		{
			if ( g_debug_charger.GetInt() == 3 )
			{
				NDebugOverlay::HorzArrow( GetAbsOrigin(), pObject->WorldSpaceCenter(), 32.0f, 255, 0, 0, 64, true, 2.0f );
			}
			continue;
		}

		// Take this as the best object so far
		pNearest = pObject;
		flLeastCost = flCost;
		vecBestHitPosition = vecHitPosition;
		
		if ( g_debug_charger.GetInt() == 3 )
		{
			NDebugOverlay::HorzArrow( GetAbsOrigin(), pObject->WorldSpaceCenter(), 16.0f, 255, 255, 0, 0, true, 2.0f );
		}
	}

	// Set extra info if we've succeeded
	if ( pNearest != NULL )
	{
		m_vecPhysicsHitPosition = vecBestHitPosition;
	
		if ( g_debug_charger.GetInt() == 3 )
		{
			NDebugOverlay::HorzArrow( GetAbsOrigin(), pNearest->WorldSpaceCenter(), 32.0f, 0, 255, 0, 128, true, 2.0f );
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_charger::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	// Interrupt if we can shove something
	if ( IsCurSchedule( SCHED_charger_CHASE_ENEMY ) )
	{
		SetCustomInterruptCondition( COND_charger_PHYSICS_TARGET );
	}

	// Interrupt if we've been given a charge target
	if ( IsCurSchedule( SCHED_charger_CHARGE ) == false )
	{
		SetCustomInterruptCondition( COND_charger_HAS_CHARGE_TARGET );
	}

	// Once we commit to doing this, just do it!
	if ( IsCurSchedule( SCHED_MELEE_ATTACK1 ) )
	{
		ClearCustomInterruptCondition( COND_ENEMY_OCCLUDED );
	}

	// Always take heavy damage
	SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			radius - 
//			magnitude - 
//-----------------------------------------------------------------------------
void CNPC_charger::ImpactShock( const Vector &origin, float radius, float magnitude, CBaseEntity *pIgnored )
{
	// Also do a local physics explosion to push objects away
	float	adjustedDamage, flDist;
	Vector	vecSpot;
	float	falloff = 1.0f / 2.5f;

	CBaseEntity *pEntity = NULL;

	// Find anything within our radius
	while ( ( pEntity = gEntList.FindEntityInSphere( pEntity, origin, radius ) ) != NULL )
	{
		// Don't affect the ignored target
		if ( pEntity == pIgnored )
			continue;
		if ( pEntity == this )
			continue;

		// UNDONE: Ask the object if it should get force if it's not MOVETYPE_VPHYSICS?
		if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS || ( pEntity->VPhysicsGetObject() && pEntity->IsPlayer() == false ) )
		{
			vecSpot = pEntity->BodyTarget( GetAbsOrigin() );
			
			// decrease damage for an ent that's farther from the bomb.
			flDist = ( GetAbsOrigin() - vecSpot ).Length();

			if ( radius == 0 || flDist <= radius )
			{
				adjustedDamage = flDist * falloff;
				adjustedDamage = magnitude - adjustedDamage;
		
				if ( adjustedDamage < 1 )
				{
					adjustedDamage = 1;
				}

				CTakeDamageInfo info( this, this, adjustedDamage, DMG_BLAST );
				CalculateExplosiveDamageForce( &info, (vecSpot - GetAbsOrigin()), GetAbsOrigin() );

				pEntity->VPhysicsTakeDamage( info );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CNPC_charger::ChargeDamage( CBaseEntity *pTarget )
{
	if ( pTarget == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pTarget );

	if ( pPlayer != NULL )
	{
		//Kick the player angles
		pPlayer->ViewPunch( QAngle( 20, 20, -30 ) );	

		Vector	dir = pPlayer->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( dir );
		dir.z = 0.0f;
		
		Vector vecNewVelocity = dir * 250.0f;
		vecNewVelocity[2] += 128.0f;
		pPlayer->SetAbsVelocity( vecNewVelocity );

		color32 red = {128,0,0,128};
		UTIL_ScreenFade( pPlayer, red, 1.0f, 0.1f, FFADE_IN );
	}
	
	// Player takes less damage
	float flDamage = 0;
	flDamage = ( pPlayer == NULL ) ? 250 : sk_charger_dmg_charge.GetFloat();
	
	// If it's being held by the player, break that bond
	Pickup_ForcePlayerToDropThisObject( pTarget );

	// Calculate the physics force
	ApplyDogChargeDamage( this, pTarget, flDamage );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_charger::InputRagdoll( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	//Set us to nearly dead so the velocity from death is minimal
	SetHealth( 1 );

	CTakeDamageInfo info( this, this, GetHealth(), DMG_CRUSH );
	BaseClass::TakeDamage( info );
}



//-----------------------------------------------------------------------------
// Purpose: make m_bPreferPhysicsAttack true
//-----------------------------------------------------------------------------
void CNPC_charger::InputEnablePreferPhysicsAttack( inputdata_t &inputdata )
{
	m_bPreferPhysicsAttack = true;
}

//-----------------------------------------------------------------------------
// Purpose: make m_bPreferPhysicsAttack false
//-----------------------------------------------------------------------------
void CNPC_charger::InputDisablePreferPhysicsAttack( inputdata_t &inputdata )
{
	m_bPreferPhysicsAttack = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_charger::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	// Figure out what to do next
	if ( failedSchedule == SCHED_charger_CHASE_ENEMY && HasCondition( COND_ENEMY_UNREACHABLE ) )
		return SelectUnreachableSchedule();

	return BaseClass::SelectFailSchedule( failedSchedule,failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_charger::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_CHASE_ENEMY:
		return SCHED_charger_CHASE_ENEMY;
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::StartSounds( void )
{
	//Initialize the additive sound channels
	CPASAttenuationFilter filter( this );

	if ( m_pGrowlHighSound == NULL )
	{
		m_pGrowlHighSound = ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_VOICE, "NPC_Charger.GrowlHigh",	ATTN_NORM );
		
		if ( m_pGrowlHighSound )
		{
			ENVELOPE_CONTROLLER.Play( m_pGrowlHighSound,0.0f, 100 );
		}
	}

	if ( m_pGrowlIdleSound == NULL )
	{
		m_pGrowlIdleSound = ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_STATIC, "NPC_Charger.Idle",	ATTN_NORM );

		if ( m_pGrowlIdleSound )
		{
			ENVELOPE_CONTROLLER.Play( m_pGrowlIdleSound,0.0f, 100 );
		}
	}

	if ( m_pBreathSound == NULL )
	{
		m_pBreathSound	= ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_ITEM, "NPC_Charger.BreathSound",		ATTN_NORM );
		
		if ( m_pBreathSound )
		{
			ENVELOPE_CONTROLLER.Play( m_pBreathSound,	0.0f, 100 );
		}
	}

	if ( m_pConfusedSound == NULL )
	{
		m_pConfusedSound = ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_WEAPON,"NPC_Charger.Confused",	ATTN_NORM );

		if ( m_pConfusedSound )
		{
			ENVELOPE_CONTROLLER.Play( m_pConfusedSound,	0.0f, 100 );
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::InputEnableBark( inputdata_t &inputdata )
{
	m_bBarkEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::InputDisableBark( inputdata_t &inputdata )
{
	m_bBarkEnabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_charger::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Charger.Death" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CNPC_charger::Event_Killed( const CTakeDamageInfo &info )
{
	if (m_pEnergyWaveChar)
	{
			DevMsg("Clear wave entities \n");
			UTIL_Remove(m_pEnergyWaveChar);
			//m_pEnergyWaveChar = NULL;
	}

	BaseClass::Event_Killed( info );

	// If I'm bleeding, stop due to decreased pressure of hemolymph after
	// cessation of aortic contraction
#if charger_BLOOD_EFFECTS
	m_iBleedingLevel = 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Don't become a ragdoll until we've finished our death anim
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::CanBecomeRagdoll( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &force - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::BecomeRagdollOnClient( const Vector &force )
{
	if ( !CanBecomeRagdoll() ) 
		return false;

	EmitSound( "NPC_Charger.Fallover" );

	// Become server-side ragdoll if we're flagged to do it
	if ( m_spawnflags & SF_charger_SERVERSIDE_RAGDOLL )
	{
		CTakeDamageInfo	info;

		// Fake the info
		info.SetDamageType( DMG_GENERIC );
		info.SetDamageForce( force );
		info.SetDamagePosition( WorldSpaceCenter() );

		CBaseEntity *pRagdoll = CreateServerRagdoll( this, 0, info, COLLISION_GROUP_NONE );

		// Transfer our name to the new ragdoll
		pRagdoll->SetName( GetEntityName() );
		pRagdoll->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
		
		// Get rid of our old body
		UTIL_Remove(this);

		return true;
	}

	return BaseClass::BecomeRagdollOnClient( force );
}

//-----------------------------------------------------------------------------
// Purpose: Override how we face our target as we move
// Output :
//-----------------------------------------------------------------------------
bool CNPC_charger::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{
  	Vector		vecFacePosition = vec3_origin;
	CBaseEntity	*pFaceTarget = NULL;
	bool		bFaceTarget = false;

	// FIXME: this will break scripted sequences that walk when they have an enemy
	if ( m_hChargeTarget )
	{
		vecFacePosition = m_hChargeTarget->GetAbsOrigin();
		pFaceTarget = m_hChargeTarget;
		bFaceTarget = true;
	}
	else if ( GetEnemy() && IsCurSchedule( SCHED_charger_CANT_ATTACK ) )
	{
		// Always face our enemy when randomly patrolling around
		vecFacePosition = GetEnemy()->EyePosition();
		pFaceTarget = GetEnemy();
		bFaceTarget = true;
	}
	else if ( GetEnemy() && GetNavigator()->GetMovementActivity() == ACT_RUN )
  	{
		Vector vecEnemyLKP = GetEnemyLKP();
		
		// Only start facing when we're close enough
		if ( ( UTIL_DistApprox( vecEnemyLKP, GetAbsOrigin() ) < 512 ) || IsCurSchedule( SCHED_charger_PATROL_RUN ) )
		{
			vecFacePosition = vecEnemyLKP;
			pFaceTarget = GetEnemy();
			bFaceTarget = true;
		}
	}

	// Face
	if ( bFaceTarget )
	{
		AddFacingTarget( pFaceTarget, vecFacePosition, 1.0, 0.2 );
	}

	return BaseClass::OverrideMoveFacing( move, flInterval );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::IsHeavyDamage( const CTakeDamageInfo &info )
{
	// Struck by blast
	if ( info.GetDamageType() & DMG_BLAST )
	{
		if ( info.GetDamage() > MIN_BLAST_DAMAGE )
			return true;
	}

	// Struck by large object
	if ( info.GetDamageType() & DMG_CRUSH )
	{
		IPhysicsObject *pPhysObject = info.GetInflictor()->VPhysicsGetObject();

		if ( ( pPhysObject != NULL ) && ( pPhysObject->GetGameFlags() & FVPHYSICS_WAS_THROWN ) )
		{
			// Always take hits from a combine ball
			if ( UTIL_IsAR2CombineBall( info.GetInflictor() ) )
				return true;

			// If we're under half health, stop being interrupted by heavy damage
			if ( GetHealth() < (GetMaxHealth() * 0.25) )
				return false;

			// Ignore physics damages that don't do much damage
			if ( info.GetDamage() < MIN_CRUSH_DAMAGE )
				return false;

			return true;
		}

		return false;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::IsLightDamage( const CTakeDamageInfo &info )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Filter out sounds we don't care about
// Input  : *pSound - sound to test against
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::QueryHearSound( CSound *pSound )
{
	// Don't bother with danger sounds from antlions or other guards
	if ( pSound->SoundType() == SOUND_DANGER && ( pSound->m_hOwner != NULL && pSound->m_hOwner->Classify() == CLASS_ANTLION ) )
		return false;

	return BaseClass::QueryHearSound( pSound );
}

//---------------------------------------------------------
// Prevent the guard from using stopping paths, as it occasionally forces him off the navmesh.
//---------------------------------------------------------
bool CNPC_charger::CNavigator::GetStoppingPath( CAI_WaypointList *pClippedWaypoints )
{
	return BaseClass::GetStoppingPath( pClippedWaypoints );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::RememberFailedPhysicsTarget( CBaseEntity *pTarget )
{
	// Already in the list?
	if ( m_FailedPhysicsTargets.Find( pTarget ) != m_FailedPhysicsTargets.InvalidIndex() )
		return true;

	// We're not holding on to any more
	if ( ( m_FailedPhysicsTargets.Count() + 1 ) > MAX_FAILED_PHYSOBJECTS )
		return false;

	m_FailedPhysicsTargets.AddToTail( pTarget );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handle squad or NPC interactions
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_charger::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender )
{
	// Don't chase targets that other guards in our squad may be going after!
	if ( interactionType == g_interactionchargerFoundPhysicsObject )
	{
		RememberFailedPhysicsTarget( (CBaseEntity *) data );
		return true;
	}

	// Mark a shoved object as being free to pursue again
	if ( interactionType == g_interactionchargerShovedPhysicsObject )
	{
		CBaseEntity *pObject = (CBaseEntity *) data;
		m_FailedPhysicsTargets.FindAndRemove( pObject );
		return true;
	}

	return BaseClass::HandleInteraction( interactionType, data, sender );
}

/*
//-----------------------------------------------------------------------------
// Purpose: Footprint rear
// Output :
//-----------------------------------------------------------------------------
void CNPC_charger::SnowFootPrint()
{
	trace_t tr;
	Vector traceStart;
	QAngle angles;

	int attachment;

	//!!!PERF - These string lookups here aren't the swiftest, but
	// this doesn't get called very frequently unless a lot of NPCs
	// are using this code.

	attachment = this->LookupAttachment( "RearFoot" );

	if( attachment == -1 )
	{
		// Exit if this NPC doesn't have the proper attachments.
		return;
	}

	this->GetAttachment( attachment, traceStart, angles );

	UTIL_TraceLine( traceStart, traceStart - Vector( 0, 0, 48.0f), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
	if( tr.fraction < 1.0 && tr.m_pEnt )
	{
		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );
		if( psurf )
		{
			// Can't plant footprints on fake materials (ladders, wading)
			if ( psurf->game.material != 'X' )
			{
				int footprintDecal = -1;
	   
				// NOTE: We could add in snow, mud, others here
				switch(psurf->game.material)
				{
				case 'J': // ICEMOD snow, defind in decals.h
					footprintDecal = 1;
					break;
				}

				if (footprintDecal != -1)
				{
					Vector right;
					AngleVectors( angles, 0, &right, 0 );//pev->angles, 0, &right, 0 );

					// Figure out where the top of the stepping leg is 
					//trace_t tr;
					Vector hipOrigin;

					VectorMA( this->GetAbsOrigin(), 
						0,
						right, hipOrigin );

					// Find where that leg hits the ground
					UTIL_TraceLine( hipOrigin, hipOrigin + Vector(0, 0, -COORD_EXTENT * 1.74), 
						MASK_SOLID_BRUSHONLY,
						this,
						COLLISION_GROUP_NONE,
						&tr );

					// plant the decal
					CPVSFilter filter( tr.endpos );

					// get decal index number by name
					int fpindex = FootPrintDecal(); // get the decal by script name
					te->FootprintDecal( filter, 0.0f, &tr.endpos, &right, 0, //0 = COLLISION_GROUP_NONE?
										fpindex, 'J' ); // J can be something else if this has any issues, 0 = fpindex
				}
			} 
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Footprints left and right
// Output :
//-----------------------------------------------------------------------------
void CNPC_charger::SnowFootPrint( bool IsLeft )
{
	trace_t tr;
	Vector traceStart;
	QAngle angles;

	int attachment;

	//!!!PERF - These string lookups here aren't the swiftest, but
	// this doesn't get called very frequently unless a lot of NPCs
	// are using this code.

	if( IsLeft )
	{
		attachment = this->LookupAttachment( "LeftFoot" );
	}
	else
	{
		attachment = this->LookupAttachment( "RightFoot" );
	}

	if( attachment == -1 )
	{
		// Exit if this NPC doesn't have the proper attachments.
		return;
	}

	this->GetAttachment( attachment, traceStart, angles );

	UTIL_TraceLine( traceStart, traceStart - Vector( 0, 0, 48.0f), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
	if( tr.fraction < 1.0 && tr.m_pEnt )
	{
		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );
		if( psurf )
		{
			// Can't plant footprints on fake materials (ladders, wading)
			if ( psurf->game.material != 'X' )
			{
				int footprintDecal = -1;
	   
				// NOTE: We could add in snow, mud, others here
				switch(psurf->game.material)
				{
				case 'J': // ICEMOD snow, defind in decals.h
					footprintDecal = 1;
					break;
				}

				if (footprintDecal != -1)
				{
					Vector right;
					AngleVectors( angles, 0, &right, 0 );//pev->angles, 0, &right, 0 );

					// Figure out where the top of the stepping leg is 
					//trace_t tr;
					Vector hipOrigin;

					VectorMA( this->GetAbsOrigin(), 
						IsLeft ? -12 : 12, // 12 = half width
						right, hipOrigin );

					// Find where that leg hits the ground
					UTIL_TraceLine( hipOrigin, hipOrigin + Vector(0, 0, -COORD_EXTENT * 1.74), 
						MASK_SOLID_BRUSHONLY,
						this,
						COLLISION_GROUP_NONE,
						&tr );

					// plant the decal
					CPVSFilter filter( tr.endpos );

					// get decal index number by name
					int fpindex = FootPrintDecal(); // get the decal by script name
					te->FootprintDecal( filter, 0.0f, &tr.endpos, &right, 0, //0 = COLLISION_GROUP_NONE?
										fpindex, 'J' ); // J can be something else if this has any issues, 0 = fpindex
				}
			} 
		}
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: Cache whatever pose parameters we intend to use
//-----------------------------------------------------------------------------
void	CNPC_charger::PopulatePoseParameters( void )
{
	m_poseThrow = LookupPoseParameter("throw");
	m_poseHead_Pitch = LookupPoseParameter("head_pitch");
	m_poseHead_Yaw   = LookupPoseParameter("head_yaw" );

	BaseClass::PopulatePoseParameters();
}

#if charger_BLOOD_EFFECTS
//-----------------------------------------------------------------------------
// Purpose: Return desired level for the continuous bleeding effect (not the 
//				individual blood spurts you see per bullet hit)
//          Return 0 for don't bleed, 
//				   1 for mild bleeding
//                 2 for severe bleeding
//-----------------------------------------------------------------------------
unsigned char CNPC_charger::GetBleedingLevel( void ) const
{
	if ( m_iHealth > ( m_iMaxHealth >> 1 ) )
	{	// greater than 50%
		return 0;
	}
	else if ( m_iHealth > ( m_iMaxHealth >> 2 ) )
	{	// less than 50% but greater than 25%
		return 1;
	}
	else
	{
		return 2;
	}
}
#endif

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_charger, CNPC_charger )

	// Interactions	
	DECLARE_INTERACTION( g_interactionchargerFoundPhysicsObject )
	DECLARE_INTERACTION( g_interactionchargerShovedPhysicsObject )

	// Squadslots	
	DECLARE_SQUADSLOT( SQUAD_SLOT_charger_CHARGE )

	//Tasks
	DECLARE_TASK( TASK_charger_CHARGE )
	DECLARE_TASK( TASK_charger_GET_PATH_TO_PHYSOBJECT )
	DECLARE_TASK( TASK_charger_SHOVE_PHYSOBJECT )
	DECLARE_TASK( TASK_charger_SET_FLINCH_ACTIVITY )
	DECLARE_TASK( TASK_charger_GET_PATH_TO_CHARGE_POSITION )
	DECLARE_TASK( TASK_charger_GET_PATH_TO_NEAREST_NODE )
	DECLARE_TASK( TASK_charger_GET_CHASE_PATH_ENEMY_TOLERANCE )
	DECLARE_TASK( TASK_charger_FIND_PHYSOBJECT )

	//Activities
	DECLARE_ACTIVITY( ACT_charger_SEARCH )
	DECLARE_ACTIVITY( ACT_charger_BARK )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_START )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_CANCEL )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_RUN )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_CRASH )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_STOP )
	DECLARE_ACTIVITY( ACT_charger_CHARGE_HIT )
	//DECLARE_ACTIVITY( ACT_charger_CHARGE_ANTICIPATION )
	DECLARE_ACTIVITY( ACT_charger_SHOVE_PHYSOBJECT )
	DECLARE_ACTIVITY( ACT_charger_FLINCH_LIGHT )
	DECLARE_ACTIVITY( ACT_charger_ROAR )
	DECLARE_ACTIVITY( ACT_charger_PHYSHIT_FR )
	DECLARE_ACTIVITY( ACT_charger_PHYSHIT_FL )
	DECLARE_ACTIVITY( ACT_charger_PHYSHIT_RR )	
	DECLARE_ACTIVITY( ACT_charger_PHYSHIT_RL )		
	
	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_charger_CHARGE_HIT )
	DECLARE_ANIMEVENT( AE_charger_SHOVE_PHYSOBJECT )
	DECLARE_ANIMEVENT( AE_charger_SHOVE )
	DECLARE_ANIMEVENT( AE_charger_FOOTSTEP_LIGHT )
	DECLARE_ANIMEVENT( AE_charger_FOOTSTEP_HEAVY )
	DECLARE_ANIMEVENT( AE_charger_VOICE_GROWL )
	DECLARE_ANIMEVENT( AE_charger_VOICE_BARK )
	DECLARE_ANIMEVENT( AE_charger_VOICE_PAIN )
	DECLARE_ANIMEVENT( AE_charger_VOICE_SQUEEZE )
	DECLARE_ANIMEVENT( AE_charger_VOICE_SCRATCH )
	DECLARE_ANIMEVENT( AE_charger_VOICE_GRUNT )
	DECLARE_ANIMEVENT( AE_charger_VOICE_ROAR )
	//DECLARE_ANIMEVENT( AE_charger_FOOTPRINT_LEFT )
	//DECLARE_ANIMEVENT( AE_charger_FOOTPRINT_RIGHT )
	//DECLARE_ANIMEVENT( AE_charger_FOOTPRINT_BACK )

	DECLARE_CONDITION( COND_charger_PHYSICS_TARGET )
	DECLARE_CONDITION( COND_charger_PHYSICS_TARGET_INVALID )
	DECLARE_CONDITION( COND_charger_HAS_CHARGE_TARGET )
	DECLARE_CONDITION( COND_charger_CAN_CHARGE )

	//==================================================
	// SCHED_charger_CHARGE
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_CHARGE,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_charger_CHASE_ENEMY"
		"		TASK_FACE_ENEMY						0"
		"		TASK_charger_CHARGE			0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_HEAVY_DAMAGE"

		// These are deliberately left out so they can be detected during the 
		// charge Task and correctly play the charge stop animation.
		//"		COND_NEW_ENEMY"
		//"		COND_ENEMY_DEAD"
		//"		COND_LOST_ENEMY"
	)

	//==================================================
	// SCHED_charger_CHARGE_TARGET
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_CHARGE_TARGET,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_ENEMY						0"
		"		TASK_charger_CHARGE			0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_charger_CHARGE_SMASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_CHARGE_CRASH,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_charger_CHARGE_CRASH"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_charger_PHYSICS_ATTACK
	//==================================================

	DEFINE_SCHEDULE
	( 
		SCHED_charger_PHYSICS_ATTACK,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_charger_CHASE_ENEMY"
		"		TASK_charger_GET_PATH_TO_PHYSOBJECT	0"
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		"		TASK_FACE_ENEMY								0"
		"		TASK_charger_SHOVE_PHYSOBJECT			0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
		"		COND_NEW_ENEMY"
		"		COND_charger_PHYSICS_TARGET_INVALID"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_FORCE_charger_PHYSICS_ATTACK
	//==================================================

	DEFINE_SCHEDULE
	( 
		SCHED_FORCE_charger_PHYSICS_ATTACK,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_charger_CANT_ATTACK"
		"		TASK_charger_FIND_PHYSOBJECT			0"
		"		TASK_SET_SCHEDULE							SCHEDULE:SCHED_charger_PHYSICS_ATTACK"
		""
		"	Interrupts"
		"		COND_charger_PHYSICS_TARGET"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_charger_CANT_ATTACK
	//		If we're here, the guard can't chase enemy, can't find a physobject to attack with
	//==================================================

	DEFINE_SCHEDULE
	( 
		SCHED_charger_CANT_ATTACK,

		"	Tasks"
		"		TASK_SET_ROUTE_SEARCH_TIME		2"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	1024"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_WAIT_PVS					0"
		""
		"	Interrupts"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_charger_PHYSICS_TARGET"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_charger_PHYSICS_DAMAGE_HEAVY
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_PHYSICS_DAMAGE_HEAVY,

		"	Tasks"
		"		TASK_STOP_MOVING						0"
		"		TASK_RESET_ACTIVITY						0"
		"		TASK_charger_SET_FLINCH_ACTIVITY	0"
		""
		"	Interrupts"
	)

	//==================================================
	// SCHED_charger_CHARGE_CANCEL
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_CHARGE_CANCEL,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_charger_CHARGE_CANCEL"
		""
		"	Interrupts"
	)
	
	//==================================================
	// SCHED_charger_FIND_CHARGE_POSITION
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_FIND_CHARGE_POSITION,

		"	Tasks"
		"		TASK_charger_GET_PATH_TO_CHARGE_POSITION	0"
		"		TASK_RUN_PATH									0"
		"		TASK_WAIT_FOR_MOVEMENT							0"
		"	"
		"	Interrupts"
		"		COND_ENEMY_DEAD"
		"		COND_GIVE_WAY"
		"		COND_TASK_FAILED"
		"		COND_HEAVY_DAMAGE"
	)

	//=========================================================
	// > SCHED_charger_CHASE_ENEMY_TOLERANCE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_charger_CHASE_ENEMY_TOLERANCE,

		"	Tasks"
		"		TASK_STOP_MOVING									0"
		"		TASK_SET_FAIL_SCHEDULE								SCHEDULE:SCHED_charger_PATROL_RUN"
		"		TASK_charger_GET_PATH_TO_NEAREST_NODE			2400" //500
		"		TASK_RUN_PATH										0"
		"		TASK_WAIT_FOR_MOVEMENT								0"
		"		TASK_FACE_ENEMY										0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_charger_PHYSICS_TARGET"
		"		COND_HEAVY_DAMAGE"
		"		COND_charger_CAN_CHARGE"
	);

	//=========================================================
	// > PATROL_RUN
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_charger_PATROL_RUN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_charger_CANT_ATTACK"
		"		TASK_SET_ROUTE_SEARCH_TIME					1"	// Spend 3 seconds trying to build a path if stuck
		"		TASK_charger_GET_PATH_TO_NEAREST_NODE	2400" //500
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_charger_PHYSICS_TARGET"
		"		COND_HEAVY_DAMAGE"
		"		COND_charger_CAN_CHARGE"
	);

	//==================================================
	// SCHED_charger_ROAR
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_charger_ROAR,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_charger_ROAR"
		"	"
		"	Interrupts"
		"		COND_HEAVY_DAMAGE"
	)

	//==================================================
	// SCHED_charger_TAKE_COVER_FROM_ENEMY
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_charger_TAKE_COVER_FROM_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_charger_CANT_ATTACK"
		"		TASK_FIND_COVER_FROM_ENEMY		0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_charger_PHYSICS_TARGET"
		"		COND_HEAVY_DAMAGE"
	)

	//=========================================================
	// SCHED_charger_CHASE_ENEMY
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_charger_CHASE_ENEMY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	2400" //300
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_RANGE_ATTACK1"
		// "		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TASK_FAILED"
		"		COND_LOST_ENEMY"
		"		COND_HEAVY_DAMAGE"
		"		COND_charger_CAN_CHARGE"
	)

AI_END_CUSTOM_NPC()
