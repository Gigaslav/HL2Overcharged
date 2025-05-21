#ifndef	WEAPON_SHOTGUNC_H
#define	WEAPON_SHOTGUNC_H

#include "weapon_shotgun.h"

class CWeaponShotgunC1 : public CWeaponShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShotgunC1, CWeaponShotgun);

	CWeaponShotgunC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponShotgunC2 : public CWeaponShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShotgunC2, CWeaponShotgun);

	CWeaponShotgunC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponShotgunC3 : public CWeaponShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShotgunC3, CWeaponShotgun);

	CWeaponShotgunC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif