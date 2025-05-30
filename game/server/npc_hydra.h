//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ==========
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#ifndef NPC_HYDRA_H
#define NPC_HYDRA_H

#if defined( _WIN32 )
#pragma once
#endif

#include "AI_BaseNPC.h"
#include "soundenvelope.h"

class CNPC_Hydra;

//-----------------------------------------------------------------------------
// CNPC_Hydra
//
//-----------------------------------------------------------------------------

class HydraBone
{
public:
	HydraBone(void)
	{
		vecPos = Vector(0, 0, 0);
		vecDelta = Vector(0, 0, 0);
		//vecBendDelta = Vector( 0, 0, 0 );
		//vecGoalDelta = Vector( 0, 0, 0 );
		//flBendTension = 0.0;
		flIdealLength = 1.0;
		flGoalInfluence = 0.0;
		bStuck = false;
		bOnFire = false;
	};

	Vector vecPos;
	Vector vecDelta;
	//float flBendTension;
	float flIdealLength;
	bool bStuck;
	bool bOnFire;

	float	flActualLength;
	//Vector	vecBendDelta;
	//Vector	vecGoalDelta;
	// float	flAccumLength;

	Vector	vecGoalPos;
	float	flGoalInfluence;

	DECLARE_SIMPLE_DATADESC();
};
class CHydraImpale;
class CNPC_Hydra : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_Hydra, CAI_BaseNPC);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	/*CNPC_Hydra()
	{
	}*/

	CNPC_Hydra();
	~CNPC_Hydra();



	void		Spawn(void);
	void		Precache(void);
	void		Activate(void);
	void		SendCoordinatesToClient(void);
	Class_T		Classify(void);

	void		RunAI(void);

	float		MaxYawSpeed(void);
	int			TranslateSchedule(int scheduleType);
	int			SelectSchedule(void);

	void		PrescheduleThink(void);

	void        HandleAnimEvent(animevent_t *pEvent);

	int			OnTakeDamage_Alive(const CTakeDamageInfo &info); // VXP
	void		Event_Killed(const CTakeDamageInfo &info);
	void		DeathSound(void);

	void		StartTask(const Task_t *pTask);
	void		RunTask(const Task_t *pTask);

#define			CHAIN_LINKS 64

	CNetworkArray(Vector, m_vecChain, CHAIN_LINKS);
	int			m_activeChain;

	bool		m_bHasStuckSegments;
	float		m_flCurrentLength;

	Vector		m_vecHeadGoal;
	float		m_flHeadGoalInfluence;
	CNetworkVector(m_vecHeadDir);

	CNetworkVar(float, m_flRelaxedLength);

	Vector		m_vecOutward;

	CUtlVector < HydraBone >	m_body;

	float		m_idealLength;
	float		m_idealSegmentLength;

	Vector		TestPosition(float t);

	void		CalcGoalForces(void);
	void		MoveBody(void);

	void		AdjustLength(void);
	void		CheckLength(void);

	bool		m_bExtendSoundActive;
	CSoundPatch		*m_pExtendTentacleSound;

	void		Nudge(CBaseEntity *pHitEntity, const Vector &vecContact, const Vector &vecSpeed);
	void		Stab(CBaseEntity *pHitEntity, const Vector &vecSpeed, trace_t &ptr);
	void		Kick(CBaseEntity *pHitEntity, const Vector &vecContact, const Vector &vecSpeed);
	void		Splash(const Vector &vecSplashPos);

	// float		FreeNeckLength( void );

	virtual Vector		EyePosition(void);
	virtual const QAngle &EyeAngles(void);

	virtual	Vector		BodyTarget(const Vector &posSrc, bool bNoisy);

	void		AimHeadInTravelDirection(float flInfluence);

	float		m_seed;

	// --------------------------------
	Vector		m_vecTarget;
	Vector		m_vecTargetDir;

	float		m_flLastAdjustmentTime;
	float		m_flTaskStartTime;
	float		m_flTaskEndTime;

	float		m_flLengthTime;	// time of last successful length adjustment time

	// --------------------------------

	bool		ContractFromHead(void);
	bool		ContractBetweenStuckSegments(void);
	bool		ContractFromRoot(void);

	int			VirtualRoot(void);

	bool		AddNodeBefore(int iNode);
	bool		AddNodeAfter(int iNode);

	bool		GrowFromVirtualRoot(void);
	bool		GrowFromMostStretched(void);

	void		CalcRelaxedLength(void);

	bool		IsValidConnection(int iNode0, int iNode1);

	void		AttachStabbedEntity(CBaseAnimating *pAnimating, Vector vecForce, trace_t &tr);
	void		UpdateStabbedEntity(void);
	void		DetachStabbedEntity(bool playSound);
	void		GetDesiredImpaledPosition(Vector *vecOrigin, QAngle *vecAngles);

	bool		m_bStabbedEntity;

	// VXP
	//	CBaseEntity *m_pRagdoll;
	CHydraImpale	*m_pHydraImpale;
	//	IPhysicsConstraint *m_pHIPhysConstraint;
	float		m_flDetachEntityTime;
	/*CSoundPatch *IdleSound;
	void CreateSounds();//������� ����
	void DestroySounds();//������� ����
	void StartSounds();*/
	void StopSounds();
	DEFINE_CUSTOM_AI;

private:
	CNetworkVar(bool, ChangeColor);
	CNetworkVar(bool, PainColor);
	float Inc;
	CNetworkVar(bool, blinking);
	CNetworkVar(int, blinking_type);
	bool		m_bDied;
	float		m_flDieTime;
	float		m_flNextStabTime;
};

//-----------------------------------------------------------------------------

#endif // NPC_HYDRA_H