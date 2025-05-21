#ifndef	WEAPON_IMMC_H
#define	WEAPON_IMMC_H

#include "OverCharged/weapon_immolator.h"

class CWeaponImmolatorC1 : public CWeaponImmolator
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponImmolatorC1, CWeaponImmolator);

	CWeaponImmolatorC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponImmolatorC2 : public CWeaponImmolator
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponImmolatorC2, CWeaponImmolator);

	CWeaponImmolatorC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponImmolatorC3 : public CWeaponImmolator
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponImmolatorC3, CWeaponImmolator);

	CWeaponImmolatorC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif