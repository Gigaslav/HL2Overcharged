#ifndef	WEAPON_FlashB_H
#define	WEAPON_FlashB_H

#include "basehlcombatweapon.h"
#include "weapon_frag.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFlashbang : public CWeaponFrag
{
	DECLARE_CLASS(CWeaponFlashbang, CWeaponFrag);
public:
	DECLARE_SERVERCLASS();

	void GrenChangeVisible(void);
	void GrenSetVisible(void);

	void ThrowGrenade(CBasePlayer *pPlayer);
	void RollGrenade(CBasePlayer *pPlayer);
	void LobGrenade(CBasePlayer *pPlayer);

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};

#endif