#ifndef	WEAPON_DEAGLEC_H
#define	WEAPON_DEAGLEC_H

#include "OverCharged/weapon_deagle.h"

class CWeaponDeagleC1 : public CWeaponDeagle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDeagleC1, CWeaponDeagle);

	CWeaponDeagleC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponDeagleC2 : public CWeaponDeagle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDeagleC2, CWeaponDeagle);

	CWeaponDeagleC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponDeagleC3 : public CWeaponDeagle
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDeagleC3, CWeaponDeagle);

	CWeaponDeagleC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif