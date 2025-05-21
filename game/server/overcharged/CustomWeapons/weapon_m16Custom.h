#ifndef	WEAPON_M16C_H
#define	WEAPON_M16C_H

#include "OverCharged/weapon_m16.h"

class CWeaponM16C1 : public CWeaponM16
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM16C1, CWeaponM16);

	CWeaponM16C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponM16C2 : public CWeaponM16
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM16C2, CWeaponM16);

	CWeaponM16C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponM16C3 : public CWeaponM16
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM16C3, CWeaponM16);

	CWeaponM16C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif