#ifndef	WEAPON_BERETTAC_H
#define	WEAPON_BERETTAC_H

#include "OverCharged/weapon_beretta.h"

class CWeaponBerettaC1 : public CWeaponBeretta
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponBerettaC1, CWeaponBeretta);

	CWeaponBerettaC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponBerettaC2 : public CWeaponBeretta
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponBerettaC2, CWeaponBeretta);

	CWeaponBerettaC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponBerettaC3 : public CWeaponBeretta
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponBerettaC3, CWeaponBeretta);

	CWeaponBerettaC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif