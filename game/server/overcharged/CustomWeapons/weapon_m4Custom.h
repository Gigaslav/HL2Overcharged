#ifndef	WEAPON_M4C_H
#define	WEAPON_M4C_H

#include "OverCharged/weapon_m4.h"

class CWeaponM4C1 : public CWeaponM4
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM4C1, CWeaponM4);

	CWeaponM4C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponM4C2 : public CWeaponM4
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM4C2, CWeaponM4);

	CWeaponM4C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponM4C3 : public CWeaponM4
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM4C3, CWeaponM4);

	CWeaponM4C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif