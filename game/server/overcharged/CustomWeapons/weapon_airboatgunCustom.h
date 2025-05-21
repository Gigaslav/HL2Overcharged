#ifndef	WEAPONAIRBOATGUNC_H
#define	WEAPONAIRBOATGUNC_H

#include "OverCharged/weapon_airboatgun.h"

class CWeaponAirboatGunC1 : public CWeaponAirboatGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAirboatGunC1, CWeaponAirboatGun);

	CWeaponAirboatGunC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponAirboatGunC2 : public CWeaponAirboatGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAirboatGunC2, CWeaponAirboatGun);

	CWeaponAirboatGunC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponAirboatGunC3 : public CWeaponAirboatGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAirboatGunC3, CWeaponAirboatGun);

	CWeaponAirboatGunC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif