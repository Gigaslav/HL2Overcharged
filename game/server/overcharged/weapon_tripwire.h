//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		TRIPWIRE 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONTRIPWIRE_H
#define	WEAPONTRIPWIRE_H

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"
#include "weapon_frag.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeapon_Tripwire : public CWeaponFrag
{
	DECLARE_CLASS(CWeapon_Tripwire, CWeaponFrag);

	DECLARE_SERVERCLASS();

public:

	void	ThrowGrenade(CBasePlayer *pPlayer);
	void	RollGrenade(CBasePlayer *pPlayer);
	void	LobGrenade(CBasePlayer *pPlayer);

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};
#endif	//WEAPONTRIPWIRE_H