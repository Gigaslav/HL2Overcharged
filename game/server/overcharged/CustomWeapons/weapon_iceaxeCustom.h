#ifndef	WEAPON_IAC_H
#define	WEAPON_IAC_H

#include "OverCharged/weapon_iceaxe.h"

class CWeaponIceaxeC1 : public CWeaponIceaxe
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIceaxeC1, CWeaponIceaxe);

	CWeaponIceaxeC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponIceaxeC2 : public CWeaponIceaxe
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIceaxeC2, CWeaponIceaxe);

	CWeaponIceaxeC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponIceaxeC3 : public CWeaponIceaxe
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIceaxeC3, CWeaponIceaxe);

	CWeaponIceaxeC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif