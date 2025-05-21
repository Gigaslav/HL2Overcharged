//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_WRENCH_H
#define WEAPON_WRENCH_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_wrench.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif


//-----------------------------------------------------------------------------
// CWeaponWrench
//-----------------------------------------------------------------------------

class CWeaponWrench : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponWrench, CBaseHLBludgeonWeapon );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponWrench();

	float		GetDamageForActivity( Activity hitActivity );
	//void	PrimaryAttack(void);

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	//void		SecondaryAttack( void )	{	return;	}
	bool	Deploy( void );

	bool CanInspect(CBasePlayer *pOwner)
	{
		return true;
	}

	virtual void	ItemPostFrame( void );

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	bool			ImpactWater(const Vector &start, const Vector &end);
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	bool m_bAltattackAnim;
	bool m_bAltattackChecker;
	// Light Kill : Delay between attack
	bool  m_bDelayedSwing;
	float m_flDelayedSwingTime;
	bool Fire;
	int Time;
	bool ChangeOnce;
	bool bIsSecondary;
	bool secondaryGate;
	bool m_bCanPrimaryAttack;
//#endif
};

#endif // WEAPON_WRENCH_H
