//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef NPC_BASESCANNER_H
#define NPC_BASESCANNER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player_pickup.h"
#include "weapon_physcannon.h"
#include "hl2_player.h"
#include "smoke_trail.h"
#include "ai_basenpc_physicsflyer.h"
#include "filesystem.h"
//-----------------------------------------------------------------------------
// States for the scanner's sound.
//-----------------------------------------------------------------------------
enum MSynthFlyMode_t
{
	MSYNTH_FLY_PHOTO = 0,		// Fly close to photograph entity
	MSYNTH_FLY_PATROL,			// Fly slowly around the enviroment
	MSYNTH_FLY_FAST,			// Fly quickly around the enviroment
	MSYNTH_FLY_CHASE,			// Fly quickly around the enviroment
	MSYNTH_FLY_SPOT,			// Fly above enity in spotlight position
	MSYNTH_FLY_ATTACK,			// Get in my enemies face for spray or flash
	MSYNTH_FLY_DIVE,			// Divebomb - only done when dead
	MSYNTH_FLY_FOLLOW,			// Following a target
};

enum MSynthInspectAct_t
{
	MSYNTH_INSACT_HANDS_UP,
	MSYNTH_INSACT_SHOWARMBAND,
};

// Sentences
#define MSYNTH_SENTENCE_ATTENTION	0
#define MSYNTH_SENTENCE_HANDSUP	1
#define MSYNTH_SENTENCE_PROCEED	2
#define MSYNTH_SENTENCE_CURIOUS	3

#define	MSYNTH_ATTACK_MIN_DELAY		3		// Min time between attacks
#define	MSYNTH_ATTACK_MAX_DELAY		5		// Max time between attacks
#define MSYNTH_ATTACK_NEAR_DIST		350		// Fly attack min distance
#define MSYNTH_ATTACK_FAR_DIST			800		// Fly attack max distance
#define MSYNTH_ATTACK_RANGE			1050		// Attack max distance

// Scanner movement vars
#define	MSYNTH_BANK_RATE				30
#define	MSYNTH_MAX_SPEED				250
#define	MSYNTH_MAX_DIVE_BOMB_SPEED		2500
#define MSYNTH_SQUAD_FLY_DIST			500		// How far to scanners stay apart
#define MSYNTH_SQUAD_HELP_DIST			4000	// How far will I fly to help

//#define MSYNTH_PARTICLE_TRAIL "mortar_synth_trail"
//#define MSYNTH_PARTICLE_GLOW "mortar_synth_projectile"
//#define MSYNTH_PARTICLE_EXPLOSION "mortar_projectile_explosion"


#define MSYNTH_IDLE_SOUND "NPC_MortarSynth.Idle"
#define MSYNTH_IDLE2_SOUND "NPC_MortarSynth.Idle"
#define MSYNTH_ENGINE_SOUND "NPC_MortarSynth.Engine"
#define MSYNTH_PAIN_SOUND "NPC_MortarSynth.Pain"
#define MSYNTH_ALERT_SOUND "NPC_MortarSynth.Alert"
#define MSYNTH_DIE_SOUND "NPC_MortarSynth.Explode"
#define MSYNTH_ATTACK_SOUND "NPC_MortarSynth.Fire"
#define MSYNTH_SHOCK_IMPACT_SOUND "NPC_MortarSynth.Death"
#define MSYNTH_DEPLOY_SOUND "NPC_MortarSynth.Deploy"
#define MSYNTH_PREATTACK_SOUND "NPC_MortarSynth.Charge"
#define MSYNTH_SPEECH1_SOUND "NPC_MortarSynth.Speech1"
#define MSYNTH_SPEECH2_SOUND "NPC_MortarSynth.Speech2"
#define MSYNTH_SPEECH3_SOUND "NPC_MortarSynth.Speech3"
#define MSYNTH_SPEECH4_SOUND "NPC_MortarSynth.Speech4"
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_BaseMSynth : public CAI_BasePhysicsFlyingBot, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS(CNPC_BaseMSynth, CAI_BasePhysicsFlyingBot);

public:
	CNPC_BaseMSynth();

	void			ReadDataFromFile(IFileSystem *filesystem, CBaseEntity *pCaller);

	void			Spawn(void);

	virtual void	UpdateEfficiency(bool bInPVS);

	Class_T			Classify(void) { return(CLASS_SCANNER); }
	virtual float	GetAutoAimRadius();

	void			Event_Killed(const CTakeDamageInfo &info);
	int				OnTakeDamage_Alive(const CTakeDamageInfo &info);
	int				OnTakeDamage_Dying(const CTakeDamageInfo &info);
	void			TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void			Gib(void);

	void			OnStateChange(NPC_STATE eOldState, NPC_STATE eNewState);
	void			ClampMotorForces(Vector &linear, AngularImpulse &angular);

	int				DrawDebugTextOverlays(void);

	virtual float	GetHeadTurnRate(void);

	virtual void	VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

	// 	CDefaultPlayerPickupVPhysics
	void			OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);
	void			OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason);

	bool			ShouldPlayIdleSound(void);
	void			IdleSound(void);
	void			DeathSound(const CTakeDamageInfo &info);
	void			AlertSound(void);
	void			PainSound(const CTakeDamageInfo &info);
	virtual char	*GetScannerSoundPrefix(void) { return ""; }
	void			ScannerEmitSound(const char *pszSoundName);

	int				MeleeAttack1Conditions(float flDot, float flDist);

	int				SelectSchedule(void);
	void			StartTask(const Task_t *pTask);
	void			OnScheduleChange(void);
	void			UpdateOnRemove(void);
	virtual float	GetMaxSpeed(void);

	void			PostRunStopMoving()	{} // scanner can use "movement" activities but not be moving

	virtual bool	CanBecomeServerRagdoll(void) { return false; }

	void			SpeakSentence(int sentenceType);

	bool			IsHeldByPhyscannon(void);

	// Inputs
	void			InputSetFlightSpeed(inputdata_t &inputdata);
	void			InputSetDistanceOverride(inputdata_t &inputdata);
	char MSYNTH_PARTICLE_TRAIL[64];
	char MSYNTH_PARTICLE_GLOW[64];
	char MSYNTH_PARTICLE_EXPLOSION[64];
protected:


	virtual char		*GetEngineSound(void) { return NULL; }
	void				PlayFlySound(void);

	void				SetBanking(float flInterval);
	void				UpdateHead(float flInterval);
	inline CBaseEntity *EntityToWatch(void);

	bool				IsEnemyPlayerInSuit(void);

	// Movement
	virtual bool		OverridePathMove(CBaseEntity *pMoveTarget, float flInterval);
	virtual bool		OverrideMove(float flInterval);
	Vector				IdealGoalForMovement(const Vector &goalPos, const Vector &startPos, float idealRange, float idealHeight);
	virtual void		AdjustScannerVelocity(void) { return; }
	virtual void		MoveToAttack(float flInterval);
	virtual void		MoveToTarget(float flInterval, const Vector &vecMoveTarget);
	virtual void		MoveExecute_Alive(float flInterval);
	virtual float		MinGroundDist(void) { return 64; }
	Vector				VelocityToEvade(CBaseCombatCharacter *pEnemy);
	virtual float		GetGoalDistance(void);

	// Divebombing
	virtual void		AttackDivebomb(void);
	void				DiveBombSoundThink();
	void				AttackDivebombCollide(float flInterval);
	void				MoveToDivebomb(float flInterval);
	void				BlendPhyscannonLaunchSpeed();

private:
	bool	GetGoalDirection(Vector *vOut);

	void	StartSmokeTrail(void);

	// Take damage from being thrown by a physcannon 
	void TakeDamageFromPhyscannon(CBasePlayer *pPlayer);

	// Take damage from physics impacts
	void TakeDamageFromPhysicsImpact(int index, gamevcollisionevent_t *pEvent);

	// Do we have a physics attacker?
	CBasePlayer *HasPhysicsAttacker(float dt);

	virtual void		StopLoopingSounds(void);

public:
	// ------------------------
	//  Death Cleanup
	// ------------------------
	CTakeDamageInfo		m_KilledInfo;

protected:
	MSynthFlyMode_t	m_nFlyMode;

	// Pose parameters
	int					m_nPoseTail;
	int					m_nPoseDynamo;
	int					m_nPoseFlare;
	int					m_nPoseFaceVert;
	int					m_nPoseFaceHoriz;

	bool				m_bHasSpoken;

	// Movement
	float				m_flFlyNoiseBase;
	float				m_flEngineStallTime;
	float				m_fNextFlySoundTime;
	Vector				m_vecDiveBombDirection;		// The direction we dive bomb. Calculated at the moment of death.
	float				m_flDiveBombRollForce;		// Used for roll while dive bombing.

	float				m_flGoalOverrideDistance;

	// Deriver scanner variables
	float				m_flAttackNearDist;
	float				m_flAttackFarDist;
	float				m_flAttackRange;

private:
	CSoundPatch			*m_pEngineSound;

	// physics influence
	CHandle<CBasePlayer>	m_hPhysicsAttacker;
	float					m_flLastPhysicsInfluenceTime;

	// Attacks
	SmokeTrail			*m_pSmokeTrail;

protected:
	DEFINE_CUSTOM_AI;

	// Custom interrupt conditions
	enum
	{
		COND_SCANNER_FLY_CLEAR = BaseClass::NEXT_CONDITION,
		COND_SCANNER_FLY_BLOCKED,
		COND_SCANNER_GRABBED_BY_PHYSCANNON,
		COND_SCANNER_RELEASED_FROM_PHYSCANNON,

		NEXT_CONDITION,
	};

	// Custom schedules
	enum
	{
		SCHED_SCANNER_PATROL = BaseClass::NEXT_SCHEDULE,
		SCHED_SCANNER_ATTACK,
		SCHED_SCANNER_ATTACK_HOVER,
		SCHED_SCANNER_ATTACK_DIVEBOMB,
		SCHED_SCANNER_CHASE_ENEMY,
		SCHED_SCANNER_CHASE_TARGET,
		SCHED_SCANNER_FOLLOW_HOVER,
		SCHED_SCANNER_HELD_BY_PHYSCANNON,

		NEXT_SCHEDULE,
	};

	// Custom tasks
	enum
	{
		TASK_SCANNER_SET_FLY_PATROL = BaseClass::NEXT_TASK,
		TASK_SCANNER_SET_FLY_CHASE,
		TASK_SCANNER_SET_FLY_ATTACK,
		TASK_SCANNER_SET_FLY_DIVE,

		NEXT_TASK,
	};

	DECLARE_DATADESC();
};

#endif // NPC_BASESCANNER_H
