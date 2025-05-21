#ifndef	WEAPON_AR2C_H
#define	WEAPON_AR2C_H

#include "weapon_ar2.h"

class CWeaponAR2C1 : public CWeaponAR2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAR2C1, CWeaponAR2);

	CWeaponAR2C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAR2C2 : public CWeaponAR2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAR2C2, CWeaponAR2);

	CWeaponAR2C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAR2C3 : public CWeaponAR2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAR2C3, CWeaponAR2);

	CWeaponAR2C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif