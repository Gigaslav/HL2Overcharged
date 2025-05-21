#ifndef	WEAPON_IONC_H
#define	WEAPON_IONC_H

#include "OverCharged/weapon_ionrifle.h"

class CWeaponIonrifleC1 : public CWeaponIonRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIonrifleC1, CWeaponIonRifle);

	CWeaponIonrifleC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponIonrifleC2 : public CWeaponIonRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIonrifleC2, CWeaponIonRifle);

	CWeaponIonrifleC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponIonrifleC3 : public CWeaponIonRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIonrifleC3, CWeaponIonRifle);

	CWeaponIonrifleC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif