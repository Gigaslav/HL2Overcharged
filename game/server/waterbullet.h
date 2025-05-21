//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that simulates bullets that are underwater.
//
//=============================================================================//

#ifndef WEAPON_WATERBULLET_H
#define WEAPON_WATERBULLET_H
#ifdef _WIN32
#pragma once
#endif

#define WATER_BULLET_BUBBLES_PER_INCH 0.05f

//=========================================================
//=========================================================
class CWaterBullet : public CBaseAnimating
{
	DECLARE_CLASS( CWaterBullet, CBaseAnimating );

public:
	void Precache();
	void Spawn( const Vector &vecOrigin, const Vector &vecDir );
	void Touch( CBaseEntity *pOther );
	void BulletThink();
public:
	/*float fldot;
	float flActualDamage;
	float flActualForce;
	CTakeDamageInfo info;
	int AmmoType;
	Vector m_vecDirShooting;
	trace_t tr0;*/
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

#endif // WEAPON_WATERBULLET_H
