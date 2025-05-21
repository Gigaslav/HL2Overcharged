//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Molotov weapon
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "basehlcombatweapon.h"
#include "weapon_frag.h"
#ifndef	WEAPON_MOLOTOV_H
#define	WEAPON_MOLOTOV_H

class CGrenade_Molotov;

#define MOLOTOV_RADIUS	4.0f // inches

class CWeaponMolotov : public CWeaponFrag
{
public:
	DECLARE_CLASS(CWeaponMolotov, CWeaponFrag);

	DECLARE_SERVERCLASS();

public:

	void				Precache( void );
	//void				ThrowMolotov( const Vector &vecSrc, const Vector &vecVelocity);
	void				ThrowGrenade(CBasePlayer *pPlayer);
	void				RollGrenade(CBasePlayer *pPlayer);
	void				LobGrenade(CBasePlayer *pPlayer);

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};

#endif	//WEAPON_MOLOTOV_H
