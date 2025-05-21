#ifndef	WEAPON_WRENCHC_H
#define	WEAPON_WRENCHC_H

#include "OverCharged/weapon_wrench.h"

class CWeaponWrenchC1 : public CWeaponWrench
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponWrenchC1, CWeaponWrench);

	CWeaponWrenchC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponWrenchC2 : public CWeaponWrench
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponWrenchC2, CWeaponWrench);

	CWeaponWrenchC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponWrenchC3 : public CWeaponWrench
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponWrenchC3, CWeaponWrench);

	CWeaponWrenchC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif