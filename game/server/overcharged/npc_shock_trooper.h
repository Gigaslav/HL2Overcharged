//========= Overcharged 2021. ============//
//
// Purpose: Shock trooper NPC
//
//
// 9 september 2021 BJ grenade throw fix 
// 26 oct melee sounds fix, cleanup hl1 unused
//=============================================================================//
#ifndef NPC_shock_trooper_H
#define NPC_shock_trooper_H

#include "ai_squad.h"
#include "ai_baseNPC.h"
#include "ai_hint.h"
#include "ai_link.h"
#include "ai_motor.h"
#include "ai_node.h"
#include "ai_senses.h"
#include "ai_utils.h"
#include "ai_waypoint.h"
#include "ai_baseactor.h"
#include "npcevent.h"
#include "datamap.h"
#include "basehlcombatweapon.h"

#include "IEffects.h"
#include "soundenvelope.h"
#include "soundent.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"

#include "props.h"
#include "gib.h"

// for determing whether or not to be passive to the player
#include "globalstate.h"

//#include "grenade_homer.h" // for test attack (disabled) // obsolete
#include "particle_parse.h" // for particle attack and for oil sprays
#include "te_particlesystem.h" // for particle attack and for oil sprays
#include "te_effect_dispatch.h" // for particle attack and for oil sprays

// for the beam attack
#include "fire.h"
#include "beam_shared.h"
#include "beam_flags.h"

// for the grenade attack
#include "basegrenade_shared.h"
#include "fire.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "world.h"

// for finding and handling corpses to burn
#include "physics_prop_ragdoll.h"
#include "EntityFlame.h"

// tracing ammo
#include "ammodef.h" // included for ammo-related damage table

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CNPC_shock_trooper : public CAI_BaseActor
{
	DECLARE_CLASS(CNPC_shock_trooper, CAI_BaseActor);
public:
	CNPC_shock_trooper();

	void	Precache(void);
	void	Spawn(void);

	void	JustSpoke(void);
	void	SpeakSentence(void);
	void	PrescheduleThink(void);

	bool	FOkToSpeak(void);
	bool	NoFriendlyFire(CBaseEntity *pTarget);

	Class_T	Classify(void);
	int     RangeAttack1Conditions(float flDot, float flDist);
	int		MeleeAttack1Conditions(float flDot, float flDist);
	int     RangeAttack2Conditions(float flDot, float flDist);

	Activity NPC_TranslateActivity(Activity eNewActivity);

	void	ClearAttackConditions(void);

	int		IRelationPriority(CBaseEntity *pTarget);

	int     GetGrenadeConditions(float flDot, float flDist);

	bool	FCanCheckAttacks(void);
	virtual void RunAI(void);
	int     GetSoundInterests(void);

	void    TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	int		OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

	float	MaxYawSpeed(void);

	void    IdleSound(void);

	void	CheckAmmo(void);

	CBaseEntity *Kick(void);

	Vector	Weapon_ShootPosition(void);
	void	HandleAnimEvent(animevent_t *pEvent);

	void	Shoot(void);
	void	Shotgun(void);

	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	int		SelectSchedule(void);
	int		TranslateSchedule(int scheduleType);


	void	PainSound(const CTakeDamageInfo &info);
	void	DeathSound(const CTakeDamageInfo &info);
	void	SetAim(const Vector &aimDir);

	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void	StartNPC(void);

	int		SquadRecruit(int searchRadius, int maxMembers);

	void	Event_Killed(const CTakeDamageInfo &info);


	static const char *pGruntSentences[];

	bool	m_bInBarnacleMouth;

public:
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

private:

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;
	float m_flTalkWaitTime;

	Vector	m_vecTossVelocity;

	int		m_iLastGrenadeCondition;
	bool	m_fStanding;
	bool	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_iClipSize;

	int		m_voicePitch;

	int		m_iSentence;

	float	m_flCheckAttackTime;

	int		m_iAmmoType;

	int		m_iWeapons;

	virtual bool GetNadeVector(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut);	// BJ nade calc added
	Vector	m_vecSaveNadeVelocity;
};

#endif