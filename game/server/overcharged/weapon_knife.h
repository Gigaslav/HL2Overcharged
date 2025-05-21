//========= Copyright 2015 L1ght 15 ============================================//
//
// Purpose:		Knife - Best weapon to slash bastards
//
//=============================================================================//

#ifndef WEAPON_Knife_H
#define WEAPON_Knife_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_Knife.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	KNIFE_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponKnife
//-----------------------------------------------------------------------------

class CWeaponKnife : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponKnife, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponKnife();

	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );

	bool		Deploy( void );

	bool CanInspect(CBasePlayer *pOwner)
	{
		return true;
	}

	virtual void	ItemPostFrame(void);
	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	bool			ImpactWater(const Vector &start, const Vector &end);
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	bool ChangeOnce;
	bool m_bBackStab;
};

#endif // WEAPON_Knife_H
