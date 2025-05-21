#ifndef	WEAPONSG552C_H
#define	WEAPONSG552C_H

#include "OverCharged/weapon_sg552.h"

class CWeaponSG552C1 : public CWeaponSG552
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSG552C1, CWeaponSG552);

	CWeaponSG552C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSG552C2 : public CWeaponSG552
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSG552C2, CWeaponSG552);

	CWeaponSG552C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSG552C3 : public CWeaponSG552
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSG552C3, CWeaponSG552);

	CWeaponSG552C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif