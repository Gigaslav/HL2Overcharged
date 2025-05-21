#ifndef	WEAPON_KNC_H
#define	WEAPON_KNC_H

#include "OverCharged/weapon_knife.h"

class CWeaponKnifeC1 : public CWeaponKnife
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponKnifeC1, CWeaponKnife);

	CWeaponKnifeC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponKnifeC2 : public CWeaponKnife
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponKnifeC2, CWeaponKnife);

	CWeaponKnifeC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponKnifeC3 : public CWeaponKnife
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponKnifeC3, CWeaponKnife);

	CWeaponKnifeC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif