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

#ifndef	BLOODDRIPSGREEN_H
#define	BLOODDRIPSGREEN_H

#include "basegrenade_shared.h"

class CParticleSystem;

enum GreenSpitSize_e
{
	GREENSPIT_SMALL,
	GREENSPIT_MEDIUM,
	GREENSPIT_LARGE,
};

#define DRIPS_GRAVITY 600

class CBloodDripsGreen : public CBaseGrenade
{
	DECLARE_CLASS(CBloodDripsGreen, CBaseGrenade);

public:
	CBloodDripsGreen(void);
	~CBloodDripsGreen(void);
	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void 				BloodDripsGreenTouch(CBaseEntity *pOther);
	void				SetBloodDripsGreenSize(int nSize);
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
