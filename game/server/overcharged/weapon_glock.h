#ifndef	WEAPON_GLOCK_H
#define	WEAPON_GLOCK_H

#include "basehlcombatweapon.h"

class CWeaponglock : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponglock, CBaseHLCombatWeapon);

	CWeaponglock(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();

private:
	int		m_nNumShotsFired;
	int		m_nBulletsFired;
};

#endif