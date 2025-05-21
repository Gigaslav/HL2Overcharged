//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_ALYXGUN_H
#define WEAPON_ALYXGUN_H

#include "basehlcombatweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponAlyxGun : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAlyxGun, CHLSelectFireMachineGun);

	CWeaponAlyxGun();
	~CWeaponAlyxGun();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	virtual float	GetMinRestTime(void);
	virtual float	GetMaxRestTime(void);
	void	SecondaryAttack(void);
	virtual void Equip(CBaseCombatCharacter *pOwner);
	float	GetFireRateSMG(void) { return 0.06f; }
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack1Condition(float flDot, float flDist);
	int		WeaponRangeAttack2Condition(float flDot, float flDist);

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);

	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

/*	virtual void SetPickupTouch(void)
	{
		// Alyx gun cannot be picked up//јликс√ан не может быть подн€т, если она умерла.
		SetTouch(NULL);
	}*/

	float m_flTooCloseTimer;

	DECLARE_ACTTABLE();
private:
	float   MuzzleFlashTime;//—чЄтчик дл€ вспышки
	bool    Switch_Mode;
	int		Shoot;
};

#endif // WEAPON_ALYXGUN_H
