#ifndef	WEAPON_AnnC_H
#define	WEAPON_AnnC_H

#include "weapon_annabelle.h"

class CWeaponAnnabelleC1 : public CWeaponAnnabelle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAnnabelleC1, CWeaponAnnabelle);

	CWeaponAnnabelleC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAnnabelleC2 : public CWeaponAnnabelle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAnnabelleC2, CWeaponAnnabelle);

	CWeaponAnnabelleC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponAnnabelleC3 : public CWeaponAnnabelle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAnnabelleC3, CWeaponAnnabelle);

	CWeaponAnnabelleC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif