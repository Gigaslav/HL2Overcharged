#ifndef	WEAPON_ALYXGUNC_H
#define	WEAPON_ALYXGUNC_H

#include "weapon_alyxgun.h"

class CWeaponAlyxGunC1 : public CWeaponAlyxGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAlyxGunC1, CWeaponAlyxGun);

	CWeaponAlyxGunC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAlyxGunC2 : public CWeaponAlyxGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAlyxGunC2, CWeaponAlyxGun);

	CWeaponAlyxGunC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAlyxGunC3 : public CWeaponAlyxGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAlyxGunC3, CWeaponAlyxGun);

	CWeaponAlyxGunC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif