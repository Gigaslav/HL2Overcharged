#ifndef NPC_HORNET_H
#define NPC_HORNET_H

#include "ai_basenpc.h"

#include "sprite.h"
#include "particle_system.h"

/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//=========================================================
// Hornets
//=========================================================

//=========================================================
// Hornet Defines
//=========================================================
#define HORNET_TYPE_RED			0
#define HORNET_TYPE_ORANGE		1
#define HORNET_RED_SPEED		(float)600
#define HORNET_ORANGE_SPEED		(float)800
#define	HORNET_BUZZ_VOLUME		(float)0.8

extern int iHornetPuff;

//=========================================================
// Hornet - this is the projectile that the Alien Grunt fires.
//=========================================================
class CNPC_Hornet : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_Hornet, CAI_BaseNPC);
public:

	void Spawn(void);
	void Precache(void);
	Class_T	 Classify(void);
	Disposition_t		IRelationType(CBaseEntity *pTarget);

	void DieTouch(CBaseEntity *pOther);
	void DartTouch(CBaseEntity *pOther);
	void TrackTouch(CBaseEntity *pOther);
	void TrackTarget(void);
	void StartDart(void);
	void IgniteTrail(void);
	void StartTrack(void);
	void CreateSprites(void);
	/*	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];


	void EXPORT StartTrack ( void );

	void EXPORT TrackTarget ( void );
	void EXPORT TrackTouch ( CBaseEntity *pOther );
	void EXPORT DartTouch( CBaseEntity *pOther );


	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );*/

	float			m_flStopAttack;
	int				m_iHornetType;
	float			m_flFlySpeed;
	int				m_flDamage;

	DECLARE_DATADESC();

private:
	CHandle< CParticleSystem >	m_hParticleTrail;
};

#endif //NPC_HORNET_H