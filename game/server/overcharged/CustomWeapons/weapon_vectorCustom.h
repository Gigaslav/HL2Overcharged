#ifndef	WEAPON_VECC_H
#define	WEAPON_VECC_H

#include "OverCharged/weapon_vector.h"

class CWeaponVectorC1 : public CWeaponVector
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponVectorC1, CWeaponVector);

	CWeaponVectorC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponVectorC2 : public CWeaponVector
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponVectorC2, CWeaponVector);

	CWeaponVectorC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponVectorC3 : public CWeaponVector
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponVectorC3, CWeaponVector);

	CWeaponVectorC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif