#ifndef	WEAPON_CROWBARC_H
#define	WEAPON_CROWBARC_H

#include "weapon_crowbar.h"

class CWeaponCrowbarC1 : public CWeaponCrowbar
{
public:
	DECLARE_CLASS(CWeaponCrowbarC1, CWeaponCrowbar);

	CWeaponCrowbarC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCrowbarC2 : public CWeaponCrowbar
{
public:
	DECLARE_CLASS(CWeaponCrowbarC2, CWeaponCrowbar);

	CWeaponCrowbarC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCrowbarC3 : public CWeaponCrowbar
{
public:
	DECLARE_CLASS(CWeaponCrowbarC3, CWeaponCrowbar);

	CWeaponCrowbarC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif