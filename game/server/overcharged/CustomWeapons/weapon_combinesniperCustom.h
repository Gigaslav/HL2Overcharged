#ifndef	WEAPON_CSNIPERC_H
#define	WEAPON_CSNIPERC_H

#include "OverCharged/weapon_combinesniper.h"

class CWeaponCombineSniperC1 : public CWeaponCombineSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCombineSniperC1, CWeaponCombineSniper);

	CWeaponCombineSniperC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCombineSniperC2 : public CWeaponCombineSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCombineSniperC2, CWeaponCombineSniper);

	CWeaponCombineSniperC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCombineSniperC3 : public CWeaponCombineSniper
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCombineSniperC3, CWeaponCombineSniper);

	CWeaponCombineSniperC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif