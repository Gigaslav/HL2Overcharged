#ifndef	WEAPON_SMG2C_H
#define	WEAPON_SMG2C_H

#include "OverCharged/weapon_smg2.h"

class CWeaponSMG2C1 : public CWeaponSMG2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG2C1, CWeaponSMG2);

	CWeaponSMG2C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSMG2C2 : public CWeaponSMG2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG2C2, CWeaponSMG2);

	CWeaponSMG2C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSMG2C3 : public CWeaponSMG2
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG2C3, CWeaponSMG2);

	CWeaponSMG2C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif