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

#ifndef	BLOODDRIPS_H
#define	BLOODDRIPS_H

#include "basegrenade_shared.h"

class CParticleSystem;

enum RedSpitSize_e
{
	REDSPIT_SMALL,
	REDSPIT_MEDIUM,
	REDSPIT_LARGE,
};

#define DRIPS_GRAVITY 600

class CBloodDrips : public CBaseGrenade
{
	DECLARE_CLASS(CBloodDrips, CBaseGrenade);

public:
	CBloodDrips(void);

	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void 				BloodDripsTouch(CBaseEntity *pOther);
	void				SetBloodDripsSize(int nSize);
	void				Detonate(void);
	void				Think(void);

private:
	DECLARE_DATADESC();

	void	InitHissSound(void);

	CHandle< CParticleSystem >	m_hSpitEffect;
	CSoundPatch		*m_pHissSound;
	bool			m_bPlaySound;
};

#endif	//GRENADESPIT_H
