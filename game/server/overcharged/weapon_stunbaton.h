
//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_STUNBATON_H
#define WEAPON_STUNBATON_H
#ifdef _WIN32
#pragma once
#endif

#include "basebludgeonweapon.h"

#define	STUNSTICK_REFIRE	0.6f

class CWeaponStunBaton : public CBaseHLBludgeonWeapon
{
	DECLARE_CLASS(CWeaponStunBaton, CBaseHLBludgeonWeapon);
	DECLARE_DATADESC();

public:

	CWeaponStunBaton();
	~CWeaponStunBaton();
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	virtual void Precache();

	void		Spawn();

	int			WeaponMeleeAttack1Condition(float flDot, float flDist);

	void		Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary);
	void		HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	bool		Deploy(void);
	bool		Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual void ItemPostFrame(void);
	void		Drop(const Vector &vecVelocity);
	void		ImpactEffect(trace_t &traceHit);
	void		SecondaryAttack(void)	{}
	void		SetStunState(bool state);
	bool		GetStunState(void);
	virtual void		Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	float		GetDamageForActivity(Activity hitActivity);

	bool		CanBePickedUpByNPCs(void) { return false; }

	bool CanInspect(CBasePlayer *pOwner)
	{
		return true;
	}

private:
	bool			ImpactWater(const Vector &start, const Vector &end);
	bool Fire;
	int Time;
	bool fire;
	bool doOnce;
	CNetworkVar(bool, m_Active);
};

#endif