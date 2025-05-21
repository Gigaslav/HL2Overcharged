#ifndef	WEAPON_CGUARDC_H
#define	WEAPON_CGUARDC_H

#include "OverCharged/weapon_cguard.h"

class CWeaponCGuardC1 : public CWeaponCGuard
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCGuardC1, CWeaponCGuard);

	CWeaponCGuardC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCGuardC2 : public CWeaponCGuard
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCGuardC2, CWeaponCGuard);

	CWeaponCGuardC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponCGuardC3 : public CWeaponCGuard
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCGuardC3, CWeaponCGuard);

	CWeaponCGuardC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif