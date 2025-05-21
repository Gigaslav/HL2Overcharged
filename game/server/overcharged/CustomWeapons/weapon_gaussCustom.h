#ifndef	WEAPON_GAUSSC_H
#define	WEAPON_GAUSSC_H

#include "OverCharged/weapon_gauss.h"

class CWeaponGaussC1 : public CWeaponGauss
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGaussC1, CWeaponGauss);

	CWeaponGaussC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGaussC2 : public CWeaponGauss
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGaussC2, CWeaponGauss);

	CWeaponGaussC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGaussC3 : public CWeaponGauss
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGaussC3, CWeaponGauss);

	CWeaponGaussC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif