#ifndef	WEAPON_HOPWIRE_H
#define	WEAPON_HOPWIRE_H

#include "basehlcombatweapon.h"
#include "grenade_frag.h"
#include "weapon_frag.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponHopwire : public CWeaponFrag
{
	DECLARE_CLASS(CWeaponHopwire, CWeaponFrag);
public:
	DECLARE_SERVERCLASS();

	bool	HasAnyAmmo(void);
	void	ThrowGrenade(CBasePlayer *pPlayer);
	void	RollGrenade(CBasePlayer *pPlayer);
	void	LobGrenade(CBasePlayer *pPlayer);

	CHandle<CGrenadeHopwire>	m_hActiveHopWire;

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};

#endif