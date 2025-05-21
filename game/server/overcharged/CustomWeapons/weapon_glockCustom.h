#ifndef	WEAPON_GLOCKC_H
#define	WEAPON_GLOCKC_H

#include "OverCharged/weapon_glock.h"

class CWeaponGlockC1 : public CWeaponglock
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGlockC1, CWeaponglock);

	CWeaponGlockC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGlockC2 : public CWeaponglock
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGlockC2, CWeaponglock);

	CWeaponGlockC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponGlockC3 : public CWeaponglock
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGlockC3, CWeaponglock);

	CWeaponGlockC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif