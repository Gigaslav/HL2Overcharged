#ifndef	WEAPON_PIST_S_H
#define	WEAPON_PIST_S_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponUSP
//-----------------------------------------------------------------------------

class CWeaponUSP : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponUSP, CBaseHLCombatWeapon);

	CWeaponUSP(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual void SetWeaponModelIndex(const char *pName);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
	bool m_bCheckEvent;
	float m_flCheckTimer;
};

#endif