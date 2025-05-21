#ifndef	WEAPON_DPISTC_H
#define	WEAPON_DPISTC_H

#include "OverCharged/weapon_dual_pistols.h"

class CWeaponDual_PistolsC1 : public CWeaponDual_Pistols
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDual_PistolsC1, CWeaponDual_Pistols);

	CWeaponDual_PistolsC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponDual_PistolsC2 : public CWeaponDual_Pistols
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDual_PistolsC2, CWeaponDual_Pistols);

	CWeaponDual_PistolsC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponDual_PistolsC3 : public CWeaponDual_Pistols
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDual_PistolsC3, CWeaponDual_Pistols);

	CWeaponDual_PistolsC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif