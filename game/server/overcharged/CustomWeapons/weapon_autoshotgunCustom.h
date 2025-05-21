#ifndef	WEAPON_AUTOSHOTGUNC_H
#define	WEAPON_AUTOSHOTGUNC_H

#include "OverCharged/weapon_autoshotgun.h"

class CWeaponAutoShotgunC1 : public CWeaponAutoShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAutoShotgunC1, CWeaponAutoShotgun);

	CWeaponAutoShotgunC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAutoShotgunC2 : public CWeaponAutoShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAutoShotgunC2, CWeaponAutoShotgun);

	CWeaponAutoShotgunC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAutoShotgunC3 : public CWeaponAutoShotgun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAutoShotgunC3, CWeaponAutoShotgun);

	CWeaponAutoShotgunC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif