
#ifndef ENTITY_WEAPONDETACH_H
#define ENTITY_WEAPONDETACH_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"


class CEntityWeaponDetach : public CBaseEntity
{
	DECLARE_CLASS(CEntityWeaponDetach, CBaseEntity);
	//DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
public:
	CEntityWeaponDetach(void);
	CEntityWeaponDetach(CBaseEntity *pParent, const char *attachment, const Vector &min, const Vector &max);

	CEntityWeaponDetach *entCreate(CBaseEntity *pOwner, const Vector &position, const Vector &min, const Vector &max);// , const Vector &min, const Vector &max);
	void LegacyThink(CBaseEntity *pVehicle, const char *pWeapon, const char *attachment, const char *textOn, const char *textOff, float radius, bool alreadyHaveIt);

	bool		foundedPlayer;
	int			count;
	bool		canTakeOffTheGun;
};
#endif