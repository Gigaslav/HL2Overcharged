//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_ICEAXE_H
#define WEAPON_ICEAXE_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_crowbar.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	ICEAXE_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponIceaxe
//-----------------------------------------------------------------------------
class CWeaponIceaxe : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponIceaxe, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponIceaxe();

	float			GetDamageForActivity(Activity hitActivity);
	virtual int		WeaponMeleeAttack1Condition(float flDot, float flDist);
	bool			Deploy(void);
	virtual void	ItemPostFrame(void);
	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	bool CanInspect(CBasePlayer *pOwner)
	{
		return true;
	}
private:
	bool			ImpactWater(const Vector &start, const Vector &end);
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
	void			HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	bool m_bAltattackAnim;
	bool m_bAltattackChecker;
	bool  m_bDelayedSwing;
	float m_flDelayedSwingTime;
	bool Fire;
	int Time;
	bool ChangeOnce;
	bool bIsSecondary;
};

#endif
