#ifndef	WEAPON_USPC_H
#define	WEAPON_USPC_H

#include "OverCharged/weapon_usp.h"

class CWeaponUSPC1 : public CWeaponUSP
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponUSPC1, CWeaponUSP);

	CWeaponUSPC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponUSPC2 : public CWeaponUSP
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponUSPC2, CWeaponUSP);

	CWeaponUSPC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponUSPC3 : public CWeaponUSP
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponUSPC3, CWeaponUSP);

	CWeaponUSPC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif