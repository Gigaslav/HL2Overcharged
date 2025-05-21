//========= Overcharged 2019. ============//
//
// Purpose: Shock trooper bounce grenade
//
//=============================================================================//

#ifndef strooper_bounceGRENADE_H
#define strooper_bounceGRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"

class Cgrenade_strooper_bounce : public CBaseGrenade
{
	DECLARE_CLASS( Cgrenade_strooper_bounce, CBaseGrenade );
public:
	DECLARE_DATADESC();
	virtual void Precache();

	void Explode( trace_t *pTrace, int bitsDamageType );
	unsigned int	PhysicsSolidMaskForEntity( void ) const;
};

class CHandGrenade : public Cgrenade_strooper_bounce
{
public:
	DECLARE_CLASS( CHandGrenade, Cgrenade_strooper_bounce );
	DECLARE_DATADESC();

	void	Spawn( void );
	void	Precache( void );
	void	BounceSound( void );
	void	BounceTouch( CBaseEntity *pOther );

	void ShootTimed( CBaseCombatCharacter *pOwner, Vector vecVelocity, float flTime );
};

#endif
