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

#ifndef	AR2_BULLET_H
#define	AR2_BULLET_H

#include "basegrenade_shared.h"

#define	MAX_AR2_NO_COLLIDE_TIME 0.2

class SmokeTrail;
class CWeaponSMG1;

class CAR2Bullet : public CBaseGrenade
{
public:
	DECLARE_CLASS(CAR2Bullet, CBaseGrenade);

	CHandle< SmokeTrail > m_hSmokeTrail;
	float				 m_fSpawnTime;
	float				m_fDangerRadius;


	void		Spawn(void);
	void		Precache(void);
	void 		AR2BulletTouch(CBaseEntity *pOther);
	void		AR2BulletThink(void);
	void		Event_Killed(const CTakeDamageInfo &info);

public:
	void EXPORT				Detonate(void);
	CAR2Bullet(void);

	DECLARE_DATADESC();
};

#endif
