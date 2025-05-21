#ifndef	WEAPON_MGUN_H
#define	WEAPON_MGUN_H

#include "OverCharged/weapon_machinegun.h"

class CWeaponMachineGunC1 : public CWeaponMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMachineGunC1, CWeaponMachineGun);

	CWeaponMachineGunC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponMachineGunC2 : public CWeaponMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMachineGunC2, CWeaponMachineGun);

	CWeaponMachineGunC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponMachineGunC3 : public CWeaponMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMachineGunC3, CWeaponMachineGun);

	CWeaponMachineGunC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif