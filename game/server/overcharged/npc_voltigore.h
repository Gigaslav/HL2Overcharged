//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Voltigore & Voltigore baby op4 Remake
//
// 
//=============================================================================//
#ifndef NPC_voltigore_H
#define NPC_voltigore_H

#include "ai_basenpc.h"

class CNPC_voltigore : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_voltigore, CAI_BaseNPC);
	DECLARE_SERVERCLASS();	//added. cl fx declare?
public:

	void Spawn( void );
	void Precache( void );
	Class_T  Classify ( void );
	
	float MaxYawSpeed ( void );

	int MeleeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack2Conditions( float flDot, float flDist );
	int RangeAttack1Conditions( float flDot, float flDist );

	void HandleAnimEvent( animevent_t *pEvent );
	int  TranslateSchedule( int scheduleType );

	void StartTask( const Task_t *pTask );
	void RunTask ( const Task_t *pTask );

	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	int		TakeDamageFromCombineBall(const CTakeDamageInfo &info);	//added recalc damage from ar2 balls
	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
		
	void PrescheduleThink( void );
	void Event_Killed( const CTakeDamageInfo &info );

	bool ShouldGib( const CTakeDamageInfo &info );

	void	ElectricBeam(void);
	void	ElectricAttack(void);
	void	PreBigExplode(void);
	void	BigExplode(void);

	bool	IsBabyVolt() { return m_fIsBabyVolt; }	//added checking for volt baby class
	bool	m_fIsBabyVolt;		//added

	bool		CanBecomeRagdoll();	// added post death anims
	Activity	GetDeathActivity();

	int			iNumDeathAnim;
	int			iCountTillExplode;

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:
	CBaseEntity* voltigoreCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	float		m_seeTime_vo;
	float		m_flameTime_vo;
	float		m_painSoundTime_vo;
	float		m_streakTime_vo;
	
	float		m_flDmgTime_vo;

	float		m_PresetDamage;		// BJ: Phurt get slash damage from convars 
	float		m_MeleeDistRecalc;

	// BJ: Bleed added
	CNetworkVar(uint8, m_iBleedingLevel);
	unsigned char GetBleedingLevel(void) const;
};

#endif //NPC_voltigore_H