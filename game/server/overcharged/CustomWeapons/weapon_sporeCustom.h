#ifndef	WEAPON_SPOREC_H
#define	WEAPON_SPOREC_H

#include "OverCharged/weapon_spore_launcher.h"

class CWeaponSporeLauncherC1 : public CWeaponSporeLauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSporeLauncherC1, CWeaponSporeLauncher);

	CWeaponSporeLauncherC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSporeLauncherC2 : public CWeaponSporeLauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSporeLauncherC2, CWeaponSporeLauncher);

	CWeaponSporeLauncherC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSporeLauncherC3 : public CWeaponSporeLauncher
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSporeLauncherC3, CWeaponSporeLauncher);

	CWeaponSporeLauncherC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif