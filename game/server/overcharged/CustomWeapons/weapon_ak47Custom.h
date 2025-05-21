#ifndef	WEAPONAK47C1_H
#define	WEAPONAK47C1_H

#include "OverCharged/weapon_ak47.h"

class CWeaponAK47C1 : public CWeaponAK47
{
	DECLARE_DATADESC();
public:

	DECLARE_CLASS(CWeaponAK47C1, CWeaponAK47);

	CWeaponAK47C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

protected:

	DECLARE_ACTTABLE();
};

class CWeaponAK47C2 : public CWeaponAK47
{
	DECLARE_DATADESC();
public:

	DECLARE_CLASS(CWeaponAK47C2, CWeaponAK47);

	CWeaponAK47C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

protected:

	DECLARE_ACTTABLE();
};

class CWeaponAK47C3 : public CWeaponAK47
{
	DECLARE_DATADESC();
public:

	DECLARE_CLASS(CWeaponAK47C3, CWeaponAK47);

	CWeaponAK47C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

protected:

	DECLARE_ACTTABLE();
};

#endif