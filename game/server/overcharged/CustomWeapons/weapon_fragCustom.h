#ifndef	WEAPON_FRAGC_H
#define	WEAPON_FRAGC_H

#include "weapon_frag.h"

class CWeaponFragC1 : public CWeaponFrag
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponFragC1, CWeaponFrag);

	CWeaponFragC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponFragC2 : public CWeaponFrag
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponFragC2, CWeaponFrag);

	CWeaponFragC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponFragC3 : public CWeaponFrag
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponFragC3, CWeaponFrag);

	CWeaponFragC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif