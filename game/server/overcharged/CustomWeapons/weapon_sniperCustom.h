#ifndef	WEAPON_SNIPERC_H
#define	WEAPON_SNIPERC_H

#include "OverCharged/weapon_sniper.h"

class CWeaponSniperC1 : public CWeaponSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperC1, CWeaponSniper);

	CWeaponSniperC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSniperC2 : public CWeaponSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperC2, CWeaponSniper);

	CWeaponSniperC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSniperC3 : public CWeaponSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperC3, CWeaponSniper);

	CWeaponSniperC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif