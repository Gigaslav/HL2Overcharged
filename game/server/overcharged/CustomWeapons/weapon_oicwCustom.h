#ifndef	WEAPON_OICWC_H
#define	WEAPON_OICWC_H

#include "OverCharged/weapon_oicw.h"

class CWeaponOICWC1 : public CWeaponOICW
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponOICWC1, CWeaponOICW);

	CWeaponOICWC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponOICWC2 : public CWeaponOICW
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponOICWC2, CWeaponOICW);

	CWeaponOICWC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponOICWC3 : public CWeaponOICW
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponOICWC3, CWeaponOICW);

	CWeaponOICWC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif