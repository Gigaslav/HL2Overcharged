#ifndef	WEAPON_IONC_H
#define	WEAPON_IONC_H

#include "OverCharged/weapon_laser.h"

class CWeaponLaserC1 : public CWeaponLaser
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponLaserC1, CWeaponLaser);

	CWeaponLaserC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponLaserC2 : public CWeaponLaser
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponLaserC2, CWeaponLaser);

	CWeaponLaserC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponLaserC3 : public CWeaponLaser
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponLaserC3, CWeaponLaser);

	CWeaponLaserC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif