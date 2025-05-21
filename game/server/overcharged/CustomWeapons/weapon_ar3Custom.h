#ifndef	WEAPONAR3C_H
#define	WEAPONAR3C_H

#include "OverCharged/weapon_ar3.h"

class CWeaponAr3C1 : public CWeaponAr3
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAr3C1, CWeaponAr3);

	CWeaponAr3C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponAr3C2 : public CWeaponAr3
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAr3C2, CWeaponAr3);

	CWeaponAr3C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponAr3C3 : public CWeaponAr3
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAr3C3, CWeaponAr3);

	CWeaponAr3C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif