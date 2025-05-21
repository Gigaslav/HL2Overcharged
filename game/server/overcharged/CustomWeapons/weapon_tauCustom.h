#ifndef	WEAPON_TAUC_H
#define	WEAPON_TAUC_H

#include "OverCharged/weapon_tau.h"

class CWeaponTauC1 : public CWeaponTau
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponTauC1, CWeaponTau);

	CWeaponTauC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponTauC2 : public CWeaponTau
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponTauC2, CWeaponTau);

	CWeaponTauC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponTauC3 : public CWeaponTau
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponTauC3, CWeaponTau);

	CWeaponTauC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif