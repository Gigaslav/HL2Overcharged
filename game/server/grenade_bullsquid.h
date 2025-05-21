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

#ifndef	GRENADEBULLSQUID_H
#define	GRENADEBULLSQUID_H

#include "basegrenade_shared.h"

class CParticleSystem;
class CPropVehicleDriveable;

enum SpitSize_e
{
	SPIT_SMALL,
	SPIT_MEDIUM,
	SPIT_LARGE,
};

#define SPIT_GRAVITY 600

class CGrenadeBullsquid : public CBaseGrenade
{
	DECLARE_CLASS(CGrenadeBullsquid, CBaseGrenade);

public:
	CGrenadeBullsquid(void);

	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void 				GrenadeBullsquidTouch(CBaseEntity *pOther);
	void				SetSpitSize(int nSize);
	void				Detonate(void);
	void				Think(void);
	void				SetCanMakeDecal(int index){ m_bCanDecal = (index == 0) ? true : false; };

private:
	DECLARE_DATADESC();

	void	InitHissSound(void);

	CHandle< CParticleSystem >	m_hSpitEffect;
	CSoundPatch		*m_pHissSound;
	bool			m_bPlaySound;
	bool			m_bCanDecal;
};

#endif	//GRENADESPIT_H
