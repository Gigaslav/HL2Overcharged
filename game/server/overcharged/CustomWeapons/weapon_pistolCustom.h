#ifndef	WEAPON_PISTC_H
#define	WEAPON_PISTC_H

#include "weapon_pistol.h"

class CWeaponPistolC1 : public CWeaponPistol
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPistolC1, CWeaponPistol);

	CWeaponPistolC1() : BaseClass() {};

	/*void    Drop(const Vector &vecVelocity)
	{
		return BaseClass::Drop(vecVelocity);
	};*/

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponPistolC2 : public CWeaponPistol
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPistolC2, CWeaponPistol);

	CWeaponPistolC2() : BaseClass() {};

	/*void    Drop(const Vector &vecVelocity)
	{
		return BaseClass::Drop(vecVelocity);
	};*/

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

class CWeaponPistolC3 : public CWeaponPistol
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPistolC3, CWeaponPistol);

	CWeaponPistolC3() : BaseClass() {};

	/*void    Drop(const Vector &vecVelocity)
	{
		return BaseClass::Drop(vecVelocity);
	};*/

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();
};

#endif