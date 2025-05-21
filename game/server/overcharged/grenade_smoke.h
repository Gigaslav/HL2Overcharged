//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_BUGBAIT_H
#define GRENADE_BUGBAIT_H
#ifdef _WIN32
#pragma once
#endif

#include "smoke_trail.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"

//Radius of the bugbait's effect on other creatures
extern ConVar smoke_grenade_radius;


class CGrenadeSmoke : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeSmoke, CBaseGrenade );
	DECLARE_SERVERCLASS();

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeSmoke( void );

public:
	void	SmokeDetonate();
	void	SmokeThink();

	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	CreateEffects( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	BlipSound() { EmitSound("Grenade.Blip");  		CPASFilter filter(GetAbsOrigin());	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 0, 0, 255, 4.5, 30, 0.1, 1.1); }
	void	DelayThink();
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	void	SetPunted( bool punt ) { m_punted = punt; }
	bool	WasPunted( void ) const { return m_punted; }

	void	InputSetTimer( inputdata_t &inputdata );


private:

	CNetworkVar( bool, m_bStartSmoke );

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
};

extern CBaseGrenade *SmokeGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

#endif // GRENADE_BUGBAIT_H