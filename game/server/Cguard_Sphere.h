//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot by bullsquid 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	CGUARD_SPHERE_H
#define	CGUARD_SPHERE_H

#include "basegrenade_shared.h"

class CParticleSystem;

#define SPIT_GRAVITY 0

class CCguard_Sphere : public CBaseCombatCharacter
{
	DECLARE_CLASS(CCguard_Sphere, CBaseCombatCharacter);

public:
	CCguard_Sphere(void);

	static CCguard_Sphere *SphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBaseViewModel *pentOwner = NULL);
	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void				Detonate(void);
	void				Think(void);

	float				Scale;
	bool				RevScale;
private:
	DECLARE_DATADESC();

};

#endif
