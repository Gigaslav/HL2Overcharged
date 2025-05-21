#ifndef	WEAPON_357C_H
#define	WEAPON_357C_H

#include "weapon_357.h"

class CWeapon357C1 : public CWeapon357
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapon357C1, CWeapon357);

	CWeapon357C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeapon357C2 : public CWeapon357
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapon357C2, CWeapon357);

	CWeapon357C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeapon357C3 : public CWeapon357
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapon357C3, CWeapon357);

	CWeapon357C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif