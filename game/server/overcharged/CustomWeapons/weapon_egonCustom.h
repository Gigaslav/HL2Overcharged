#ifndef	WEAPON_EGONC_H
#define	WEAPON_EGONC_H

#include "OverCharged/weapon_egon.h"

class CWeaponEgonC1 : public CWeaponEgon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponEgonC1, CWeaponEgon);

	CWeaponEgonC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponEgonC2 : public CWeaponEgon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponEgonC2, CWeaponEgon);

	CWeaponEgonC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeaponEgonC3 : public CWeaponEgon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponEgonC3, CWeaponEgon);

	CWeaponEgonC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif