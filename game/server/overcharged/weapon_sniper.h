#ifndef	WEAPON_SNIPER_H
#define	WEAPON_SNIPER_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponSniper
//-----------------------------------------------------------------------------

class CWeaponSniper : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponSniper, CBaseHLCombatWeapon);
public:

	CWeaponSniper(void);
	void	Precache(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	float	WeaponAutoAimScale()	{ return 0.6f; }
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
};

#endif