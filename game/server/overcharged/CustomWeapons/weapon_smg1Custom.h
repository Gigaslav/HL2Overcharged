#ifndef	WEAPON_SMG1C_H
#define	WEAPON_SMG1C_H

#include "weapon_smg1.h"

class CWeaponSMG1C1 : public CWeaponSMG1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG1C1, CWeaponSMG1);

	CWeaponSMG1C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSMG1C2 : public CWeaponSMG1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG1C2, CWeaponSMG1);

	CWeaponSMG1C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSMG1C3 : public CWeaponSMG1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG1C3, CWeaponSMG1);

	CWeaponSMG1C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif