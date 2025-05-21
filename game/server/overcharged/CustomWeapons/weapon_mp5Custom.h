#ifndef	WEAPON_MP5C_H
#define	WEAPON_MP5C_H

#include "OverCharged/weapon_mp5.h"

class CWeaponMp5C1 : public CWeaponmp5
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMp5C1, CWeaponmp5);

	CWeaponMp5C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponMp5C2 : public CWeaponmp5
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMp5C2, CWeaponmp5);

	CWeaponMp5C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponMp5C3 : public CWeaponmp5
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMp5C3, CWeaponmp5);

	CWeaponMp5C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif