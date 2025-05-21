//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Snark remake
//
//=============================================================================//

#ifndef NPC_snark_H
#define NPC_snark_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_squadslot.h"
#include "ai_basenpc.h"
#include "soundent.h"

class CNPC_snark : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_snark, CAI_BaseNPC);

public:
	void Spawn( void );
	void Precache( void );
	void RunTask( const Task_t *pTask );
	void StartTask( const Task_t *pTask );

	void OnChangeActivity( Activity NewActivity );
	Activity NPC_TranslateActivity(Activity eNewActivity);

	bool	CanBecomeRagdoll()
	{
		return false;
	}

	void	BiteSound(void);
	void	PainSound(const CTakeDamageInfo &info);
	void	DeathSound(const CTakeDamageInfo &info);
	void	IdleSound(void);
	void	AlertSound(void);
	void	AttackSound(void);
	void	TelegraphSound(void);
	//void	PreExplodeSound(const CTakeDamageInfo &info);

	bool IsFirmlyOnGround();
	void MoveOrigin( const Vector &vecDelta );
	void ThrowAt( const Vector &vecPos );
	void ThrowThink( void );
	virtual void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin, bool bThrown = false );
	void JumpToBurrowHint( CAI_Hint *pHint );

	bool	HasHeadroom();
	void	LeapTouch ( CBaseEntity *pOther );
	virtual void TouchDamage( CBaseEntity *pOther );
	bool	CorpseGib( const CTakeDamageInfo &info );
	void	Touch( CBaseEntity *pOther );
	Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	float	GetAutoAimRadius();
	void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void	Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false );

	float	MaxYawSpeed( void );
	void	GatherConditions( void );
	void	PrescheduleThink( void );
	Class_T Classify( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		RangeAttack1Conditions ( float flDot, float flDist );
	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut );
	void	Event_Killed( const CTakeDamageInfo &info );
	void	BuildScheduleTestBits( void );
	bool	FValidateHintType( CAI_Hint *pHint );

	bool	IsJumping( void ) { return m_bMidJump; }

	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual int		TranslateSchedule( int scheduleType );

	virtual float	GetReactionDelay( CBaseEntity *pEnemy ) { return 0.0; }

	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void	CrawlFromCanister();

	virtual	bool		AllowedToIgnite( void ) { return true; }

	virtual bool CanBeAnEnemyOf( CBaseEntity *pEnemy );

	bool	IsHangingFromCeiling( void ) 
	{ 
		return m_bHangingFromCeiling;	
	}

	virtual void PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot );

	void DropFromCeiling( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

protected:
	void snarkInit();

	void Leap( const Vector &vecVel );

	void GrabHintNode( CAI_Hint *pHint );
	bool FindBurrow( const Vector &origin, float distance, bool excludeNear );
	bool ValidBurrowPoint( const Vector &point );
	void ClearBurrowPoint( const Vector &origin );
	void Burrow( void );
	void Unburrow( void );
	void SetBurrowed( bool bBurrowed );
	void JumpFromCanister();

	// Begins the climb from the canister
	void BeginClimbFromCanister();

	void InputBurrow( inputdata_t &inputdata );
	void InputBurrowImmediate( inputdata_t &inputdata );
	void InputUnburrow( inputdata_t &inputdata );

	void InputStartHangingFromCeiling( inputdata_t &inputdata );
	void InputDropFromCeiling( inputdata_t &inputdata );

	int CalcDamageInfo( CTakeDamageInfo *pInfo );
	void CreateDust( bool placeDecal = true );

	// Eliminates roll + pitch potentially in the snark at canister jump time
	void EliminateRollAndPitch();

	float InnateRange1MinRange( void );
	float InnateRange1MaxRange( void );

protected:
	int		m_nGibCount;
	float	m_flTimeDrown;
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.

	float	m_flTimeAlive;	// BriJee OVR: Overall live time.
	float	m_flTimeToForgetOwner;

	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;

	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bCrawlFromCanister;
	bool	m_bStartBurrowed;
	bool	m_bBurrowed;
	bool	m_bHidden;
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.

	bool bDoPreExplodeOnce;	// BJ: Do jump explode once

	float	m_flBurrowTime;
	int		m_nContext;			// for FValidateHintType context
	int		m_nJumpFromCanisterDir;

	bool	m_bHangingFromCeiling;
	float	m_flIlluminatedTime;
};

#endif // NPC_snark_H
