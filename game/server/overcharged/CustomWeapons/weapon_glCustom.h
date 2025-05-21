#ifndef	WEAPON_GLC_H
#define	WEAPON_GLC_H

#include "OverCharged/weapon_grenadelauncher.h"

class CWeaponGLC1 : public CWeaponGrenadelauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGLC1, CWeaponGrenadelauncher);

	CWeaponGLC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGLC2 : public CWeaponGrenadelauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGLC2, CWeaponGrenadelauncher);

	CWeaponGLC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGLC3 : public CWeaponGrenadelauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGLC3, CWeaponGrenadelauncher);

	CWeaponGLC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif