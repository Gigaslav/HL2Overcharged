#ifndef	WEAPON_RPGC_H
#define	WEAPON_RPGC_H

#include "weapon_rpg.h"

class CWeaponRPGC1 : public CWeaponRPG
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponRPGC1, CWeaponRPG);

	CWeaponRPGC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponRPGC2 : public CWeaponRPG
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponRPGC2, CWeaponRPG);

	CWeaponRPGC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponRPGC3 : public CWeaponRPG
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponRPGC3, CWeaponRPG);

	CWeaponRPGC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif