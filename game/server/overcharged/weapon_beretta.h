#ifndef	WEAPON_BERETTA_H
#define	WEAPON_BERETTA_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponBeretta
//-----------------------------------------------------------------------------

class CWeaponBeretta : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponBeretta, CBaseHLCombatWeapon);

	CWeaponBeretta(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void) { return; };
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
};

#endif