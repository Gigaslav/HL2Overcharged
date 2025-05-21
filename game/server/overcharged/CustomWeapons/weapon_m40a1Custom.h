#ifndef	WEAPON_M40A1C_H
#define	WEAPON_M40A1C_H

#include "OverCharged/weapon_sniper_m40a1.h"

class CWeaponSniperM40A1C1 : public CWeaponSniperM40A1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperM40A1C1, CWeaponSniperM40A1);

	CWeaponSniperM40A1C1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSniperM40A1C2 : public CWeaponSniperM40A1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperM40A1C2, CWeaponSniperM40A1);

	CWeaponSniperM40A1C2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponSniperM40A1C3 : public CWeaponSniperM40A1
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSniperM40A1C3, CWeaponSniperM40A1);

	CWeaponSniperM40A1C3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif