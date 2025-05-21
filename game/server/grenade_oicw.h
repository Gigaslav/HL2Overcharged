//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADE_OICW_H
#define	GRENADE_OICW_H

#include "basegrenade_shared.h"

#define	MAX_OICW_NO_COLLIDE_TIME 0.2

class SmokeTrail;
class CWeaponOICW;

class CGrenadeOICW : public CBaseGrenade
{
public:
	DECLARE_CLASS(CGrenadeOICW, CBaseGrenade);

	CHandle< SmokeTrail > m_hSmokeTrail;
	float				 m_fSpawnTime;
	float				m_fDangerRadius;


	void		Spawn(void);
	void		Precache(void);
	void 		GrenadeOICWTouch(CBaseEntity *pOther);
	void		GrenadeOICWThink(void);
	void		Event_Killed(const CTakeDamageInfo &info);

public:
	void EXPORT				Detonate(void);
	CGrenadeOICW(void);

	DECLARE_DATADESC();
};

#endif	//GRENADEAR2_H
