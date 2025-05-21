#ifndef	WEAPON_PISTOL_H
#define	WEAPON_PISTOL_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponPistol
//-----------------------------------------------------------------------------

class CWeaponPistol : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponPistol, CBaseHLCombatWeapon);

	CWeaponPistol(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	//virtual void    Drop(const Vector &vecVelocity);

	DECLARE_ACTTABLE();

private:

	int     Shoot1;
	float   MuzzleFlashTime;//—чЄтчик дл€ вспышки
	int		m_nNumShotsFired;

};

#endif