#ifndef	WEAPON_DISPC_H
#define	WEAPON_DISPC_H

#include "OverCharged/weapon_displacer.h"

class CWeapondisplacerC1 : public CWeapondisplacer
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapondisplacerC1, CWeapondisplacer);

	CWeapondisplacerC1() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeapondisplacerC2 : public CWeapondisplacer
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapondisplacerC2, CWeapondisplacer);

	CWeapondisplacerC2() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

class CWeapondisplacerC3 : public CWeapondisplacer
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeapondisplacerC3, CWeapondisplacer);

	CWeapondisplacerC3() : BaseClass() {};

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

};

#endif