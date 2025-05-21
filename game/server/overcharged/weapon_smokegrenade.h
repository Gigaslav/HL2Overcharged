#ifndef	WEAPON_SMGRENADE_H
#define	WEAPON_SMGRENADE_H

#include "basehlcombatweapon.h"
#include "grenade_frag.h"
#include "grenade_smoke.h"
#include "weapon_frag.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponSmokeGrenade : public CWeaponFrag
{
	DECLARE_CLASS(CWeaponSmokeGrenade, CWeaponFrag);
public:
	DECLARE_SERVERCLASS();

public:
	void	GrenChangeVisible(void);
	void	GrenSetVisible(void);
	void	ThrowGrenade(CBasePlayer *pPlayer);
	void	RollGrenade(CBasePlayer *pPlayer);
	void	LobGrenade(CBasePlayer *pPlayer);

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};

#endif