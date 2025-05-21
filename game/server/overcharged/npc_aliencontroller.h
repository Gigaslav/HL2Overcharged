//=================== Half-Life 2: Short Stories Mod 2009 =====================//
//
// Purpose:	Alien Controllers from HL1 now in updated form
//
//=============================================================================//


#ifndef NPC_ALIENCONTROLLER
#define NPC_ALIENCONTROLLER

#include "cbase.h"
#include "npc_talker.h"
#include "ai_basenpc.h"
#include "IEffects.h"
#include "sprite.h"
//#include "ai_basenpc_physicsflyer.h"
#include "ai_basenpc.h"
#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"

//#define CONTROLLER_PATH_FINDING_DEBUG_MESSAGES
//#define CONTROLLER_MOVE_DEBUG_MESSAGES 1
#define HLSS_CONTROLLER_TELEKINESIS 1 // Light Kill : Enable telekinesis for Alien Controller
//----------------------------------------------------------------
// PHYS OBJECTS
//----------------------------------------------------------------

#define ALIENCONTROLLER_MAX_PHYSOBJ_MASS				490
#define ALIENCONTROLLER_MIN_PHYSOBJ_MASS				 30
#define ALIENCONTROLLER_PLAYER_MAX_SWAT_DIST		   1536
#define ALIENCONTROLLER_PHYSOBJ_PULLDIST			   1024
#define ALIENCONTROLLER_PHYSOBJ_MOVE_TO_DIST			900
#define ALIENCONTROLLER_SWAT_DELAY						5
#define ALIENCONTROLLER_FARTHEST_PHYSICS_OBJECT		   1024 //40.0*12.0 //40.0*12.0
#define ALIENCONTROLLER_PHYSICS_SEARCH_DEPTH			100

#define ALIENCONTROLLER_CATCH_DISTANCE					48
#define	ALIENCONTROLLER_MOVE_UP_DISTANCE				24
#define ALIENCONTROLLER_PULL_VELOCITY_MOD				0.1f
#define ALIENCONTROLLER_PULL_ANGULARIMP_MOD				0.8f
#define ALIENCONTROLLER_PULL_TO_GUN_VEL_MOD				2.0f

#define ALIENCONTROLLER_CANCEL_OBJECT					20

//----------------------------------------------------------------
// TELEKINESIS
//----------------------------------------------------------------

#define ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS		 3

#define ALIENCONTROLLER_TELEKINESIS_INITIAL_DELAY			 2.0f
#define ALIENCONTROLLER_TELEKINESIS_DELAY					 1.5f
#define ALIENCONTROLLER_TELEKINESIS_INITIAL_CANCEL_DELAY	10.0f
#define ALIENCONTROLLER_TELEKINESIS_CANCEL_DELAY			 7.0f

#define ALIENCONTROLLER_TELEKINESIS_NO_BEST_OBJECT_DELAY	 0.3f

#define ALIENCONTROLLER_TELEKINESIS_SCAN_DELAY				10.0f
#define ALIENCONTROLLER_TELEKINESIS_INITIAL_SCAN_DELAY		 3.0f

#define ALIENCONTROLLER_TELEKINESIS_PULL_VECTOR				Vector(0,0,64)

#define ALIENCONTROLLER_TELEKINESIS_MAX_HEIGHT 256
#define ALIENCONTROLLER_TELEKINESIS_MIN_HEIGHT  64

#define ALIENCONTROLLER_HULL_MINS Vector( -16, -16, 0 )
#define ALIENCONTROLLER_HULL_MAXS Vector( 16, 16, 70 )


//----------------------------------------------------------------
// MOVEMENT
//----------------------------------------------------------------

#define ALIENCONTROLLER_FACE_ENEMY_DISTANCE					512.0f
#define ALIENCONTROLLER_XY_DISTANCE_NOT_FACING				8.0f
#define ALIENCONTROLLER_Z_DISTANCE_NOT_FACING				4.0f
#define ALIENCONTROLLER_XY_DISTANCE_FACING					4.0f
#define ALIENCONTROLLER_Z_DISTANCE_FACING					2.0f

#define CONTROLLER_BODY_CENTER_Z 35
#define CONTROLLER_BODY_CENTER Vector(0,0,CONTROLLER_BODY_CENTER_Z) //-35

//----------------------------------------------------------------
// PARTICLE MESSAGES
//----------------------------------------------------------------

#define MESSAGE_START_FIREBALL 0
#define MESSAGE_STOP_FIREBALL 1
#define MESSAGE_START_BEAM 2
#define MESSAGE_STOP_BEAM 3
#define MESSAGE_STOP_ALL 4


//----------------------------------------------------------------

#define ALIENCONTROLLER_OVERRIDE_SCHEDULE_CHANGE_FUNCTIONS_TO_FIX_GOAL_FROM_CLEARING 1

class CNPC_AlienController : public CAI_PlayerAlly 
{
DECLARE_CLASS( CNPC_AlienController, CAI_PlayerAlly );

public:
	CNPC_AlienController();

	Class_T			Classify( void ) { return( CLASS_ALIENCONTROLLER ); }

	unsigned int	PhysicsSolidMaskForEntity() { return (CONTENTS_TEAM1 | BaseClass::PhysicsSolidMaskForEntity()); }

	void			DeathSound( const CTakeDamageInfo &info ) ;
	void			PainSound( const CTakeDamageInfo &info );
	void			AlertSound( void );
	void			AttackSound( void );
	void			IdleSound( void );

	void			Precache(void);

	void			Spawn(void);

	bool			CanBecomeRagdoll(const CTakeDamageInfo &info);//OVR
	Activity		GetDeathActivity();//OVR

	void			Activate();

	void			OnRestore();
	int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void			PrescheduleThink( void );

#ifdef ALIENCONTROLLER_OVERRIDE_SCHEDULE_CHANGE_FUNCTIONS_TO_FIX_GOAL_FROM_CLEARING
	virtual void	OnScheduleChange ( void );
	virtual void	SetSchedule( CAI_Schedule *pNewSchedule, bool DontClearGoal = false );
#endif

	bool			ControllerReachedPoint(Vector vecOrigin);

	//virtual bool	FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker );

	virtual float	GetReactionDelay( CBaseEntity *pEnemy ) { return 0.0; }

	virtual float	MaxYawSpeed( void ) { return 100.0f; }

	virtual void	UpdateEfficiency( bool bInPVS );

	virtual void	HandleAnimEvent( animevent_t *pEvent );
	virtual int		GetSoundInterests( void );
	virtual int		SelectSchedule( void );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );

	virtual	Activity NPC_TranslateActivity( Activity baseAct );
	virtual	int		TranslateSchedule( int scheduleType );

	virtual bool	OverrideMove(float flInterval);
	virtual	bool	OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	bool			Probe( const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect );
	bool			ControllerProgressFlyPath();

	int				RangeAttack1Conditions( float flDot, float flDist );
	int				MeleeAttack1Conditions( float flDot, float flDist );

	void			Event_Killed( const CTakeDamageInfo &info );

	virtual float	InnateRange1MaxRange();
	virtual float	InnateRange1MinRange();

	Disposition_t	IRelationType( CBaseEntity *pTarget );

	//Controller custom

	void			ShootFireBall();

	void			ParticleMessages(int message);

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	void			CombineBallCheck();
#endif

	Vector			GetPreferredAttackPoint(CBaseEntity *moveTarget);
	void			ControllerFly(float flInterval, Vector vMoveTargetPos, Vector vTargetPos );
	void			ControllerLand(float flInterval );
	float			CheckLandingSpace(Vector origin);

	// Pulling and throwing physics objects

	int				GetSwatActivity( void );

#ifdef HLSS_CONTROLLER_TELEKINESIS
	void			NPCThink(void);	//for physobject pull

	virtual void	BuildScheduleTestBits( void );
	void			GatherConditions( void );
	bool			FindNearestPhysicsObjects( int iMaxMass );

	bool			ThrowObject( void );
	void			CancelObject( void );
	void			PullObjects();
	void			StartObjectSounds();
#endif

	void			UpdateHead( void );


	// Inputs
	void			InputEnableLanding( inputdata_t &inputdata );
	void			InputDisableLanding( inputdata_t &inputdata );

	void			InputStopScriptingAndGesture( inputdata_t &inputdata );

private:

	//FIREBALL

	int				m_iFireBallAttachment;
	int				m_iLeftHandAttachment;
	int				m_iBrainsAttachment;

	CNetworkVar( bool, m_bFireballEffects );

	EHANDLE			m_hFireBallTarget;

	int				m_iLastOffsetAngle;
	float			m_fLastOffsetSwithcTime;
	float			m_flPreferredHeight;
	Vector			m_vecPreferedAttackPoint;
	float			m_flNextPreferredAttackTime;

	float			m_flNextAttackTime;
	int				m_iNumberOfAttacks;

	//SOUNDS:
	float			m_flNextPainSoundTime;

	//TELEKINESIS:
#ifdef HLSS_CONTROLLER_TELEKINESIS
	CNetworkArray( EHANDLE, m_hPhysicsEnt, ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS );

	Vector			m_vecPhysicsEntOrigin[ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS];
	int				m_iNumberOfPhysiscsEnts;

	float			m_flNextTelekinesisCancel;
	float			m_flNextTelekinesisThrow;
	float			m_flNextTelekinesisScan;
#endif

	bool			m_bIsFlying;
	bool			m_bIsLanding;
	bool			m_bCanLand;
	float			m_flNextLandingTime;
	//int				m_iNodeToLand;
	Vector			m_vecLandPosition;

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	float			m_flNextCombineBallScan;
	EHANDLE			m_hCombineBall;
#endif

	//TERO: hint node releated shit
	Vector			m_vOldGoal;
//	Vector			m_vLastFlyPosition;

//  -------------------------------------------------------

	//OUTPUTS:
	COutputEvent		m_OnLand;
	COutputEvent		m_OnLiftOff;

//  -------------------------------------------------------

	bool			CheckLanding();
	void			Land(bool bIsFlying);
	void			Claw();

private:

	
	DEFINE_CUSTOM_AI;

	
	// Custom interrupt conditions
	enum
	{
		COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL = BaseClass::NEXT_CONDITION,
#ifdef HLSS_CONTROLLER_TELEKINESIS
		COND_ALIENCONTROLLER_CAN_PHYS_ATTACK,
#endif
		COND_ALIENCONTROLLER_SHOULD_LAND,
		COND_ALIENCONTROLLER_FLY_BLOCKED,
	};

	// Custom schedules
	enum
	{
		SCHED_ALIENCONTROLLER_LEVITATE = BaseClass::NEXT_SCHEDULE,
		SCHED_ALIENCONTROLLER_SHOOT_FIREBALL,
		SCHED_ALIENCONTROLLER_FIREBALL_EXTINGUISH,
		SCHED_ALIENCONTROLLER_LAND,
		SCHED_ALIENCONTROLLER_PRESS_ATTACK,

#ifdef HLSS_CONTROLLER_TELEKINESIS
		SCHED_ALIENCONTROLLER_TELEKINESIS,
#endif
	};

	
	// Custom tasks
	enum
	{
		TASK_ALIENCONTROLLER_FIREBALL_EXTINGUISH = BaseClass::NEXT_TASK,
		TASK_ALIENCONTROLLER_LAND,

#ifdef HLSS_CONTROLLER_TELEKINESIS
		TASK_ALIENCONTROLLER_TELEKINESIS,
#endif
	};
	

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};


#endif //NPC_ALIENCONTROLLER