//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONAIRBOATGUN_H
#define	WEAPONAIRBOATGUN_H

#include "basecombatweapon.h"
#include "basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"

class CWeaponAirboatGun : public CHLMachineGun
{
public:
	DECLARE_CLASS(CWeaponAirboatGun, CHLMachineGun);

	CWeaponAirboatGun();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	const char *GetTracerType(void) { return "AR2Tracer"; }
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual void	SecondaryAttack(void) {};
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void	DoImpactEffect(trace_t &tr, int nDamageType);
	const WeaponProficiencyInfo_t *GetProficiencyValues();

	/*virtual void            SetPickupTouch(void)
	{

	if (!touch)
	{
	SetTouch(NULL);
	}
	else if (touch)
	{
	SetTouch(&CBaseCombatWeapon::DefaultTouch);
	}

	}*/

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


#endif
