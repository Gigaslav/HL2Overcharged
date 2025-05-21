#ifndef	WEAPON_STUNC_H
#define	WEAPON_STUNC_H

#include "OverCharged/weapon_stunbaton.h"

class CWeaponStunBatonC1 : public CWeaponStunBaton
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponStunBatonC1, CWeaponStunBaton);

	CWeaponStunBatonC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponStunBatonC2 : public CWeaponStunBaton
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponStunBatonC2, CWeaponStunBaton);

	CWeaponStunBatonC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponStunBatonC3 : public CWeaponStunBaton
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponStunBatonC3, CWeaponStunBaton);

	CWeaponStunBatonC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif