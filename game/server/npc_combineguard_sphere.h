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

#ifndef	CNPC_COMBINEGUARD_SPHERE_H
#define	CNPC_COMBINEGUARD_SPHERE_H

#include "basegrenade_shared.h"

class CParticleSystem;

#define SPIT_GRAVITY 0

class CNPC_CombineGuard_Sphere : public CBaseCombatCharacter
{
	DECLARE_CLASS(CNPC_CombineGuard_Sphere, CBaseCombatCharacter);

public:
	CNPC_CombineGuard_Sphere(void);

	static CNPC_CombineGuard_Sphere *GuardSphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *Owner = NULL);
	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	//	void 				SphereTouch(CBaseEntity *pOther);
	//	void				SetSphereSize(int nSize);
	void				Detonate(void);
	void				Think(void);
	void vectors(const Vector &Origin);
private:
	DECLARE_DATADESC();

};

#endif
