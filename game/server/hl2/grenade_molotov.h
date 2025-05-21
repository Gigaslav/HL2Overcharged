//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Molotov grenades
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADEMOLOTOV_H
#define	GRENADEMOLOTOV_H

#include "basegrenade_shared.h"
#include "smoke_trail.h"

class CGrenade_Molotov : public CBaseGrenade
{
public:
	DECLARE_CLASS( CGrenade_Molotov, CBaseGrenade );

	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	Detonate( void );
	void			MolotovTouch( CBaseEntity *pOther );
	void			MolotovThink( void );

protected:

	SmokeTrail		*m_pFireTrail;

	DECLARE_DATADESC();
};

class CParticleSystem;

#define EMBERS_GRAVITY 600

class CMolotovEmbers : public CBaseGrenade
{
	DECLARE_CLASS(CMolotovEmbers, CBaseGrenade);

public:
	CMolotovEmbers(void);

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
#endif	//GRENADEMOLOTOV_H
