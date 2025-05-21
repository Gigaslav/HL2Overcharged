#ifndef	WEAPON_SRC_H
#define	WEAPON_SRC_H

#include "OverCharged/weapon_shockrifle.h"

class CWeaponShockRifleC1 : public CWeaponShockRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShockRifleC1, CWeaponShockRifle);

	CWeaponShockRifleC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponShockRifleC2 : public CWeaponShockRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShockRifleC2, CWeaponShockRifle);

	CWeaponShockRifleC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponShockRifleC3 : public CWeaponShockRifle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShockRifleC3, CWeaponShockRifle);

	CWeaponShockRifleC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif