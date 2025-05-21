#ifndef	WEAPON_CROSSBOWC_H
#define	WEAPON_CROSSBOWC_H

#include "weapon_crossbow.h"

class CWeaponCrossbowC1 : public CWeaponCrossbow
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCrossbowC1, CWeaponCrossbow);

	CWeaponCrossbowC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCrossbowC2 : public CWeaponCrossbow
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCrossbowC2, CWeaponCrossbow);

	CWeaponCrossbowC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCrossbowC3 : public CWeaponCrossbow
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCrossbowC3, CWeaponCrossbow);

	CWeaponCrossbowC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif