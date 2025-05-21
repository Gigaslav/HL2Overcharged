#ifndef	WEAPON_FTH_H
#define	WEAPON_FTH_H

#include "basehlcombatweapon.h"

class CWeaponflamethrower : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponflamethrower, CHLSelectFireMachineGun);

	CWeaponflamethrower();

	DECLARE_SERVERCLASS();

	void	ItemPostFrame(void);
	void	Precache(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void) { return; }
	void	Drop(const Vector &vecVelocity);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	bool	Reload(void);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition();
	const WeaponProficiencyInfo_t *GetProficiencyValues();
	void	StartLoopSound();

	DECLARE_ACTTABLE();

private:
	bool DoOnce;
	bool bFlameSndLoop;
	bool bRemAmmoReducer;
	bool bRemAmmoReducer2;
};
#endif