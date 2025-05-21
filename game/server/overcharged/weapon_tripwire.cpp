//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_tripwire.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "gamestats.h"
#include "weapon_tripwire.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CWeapon_Tripwire)

END_DATADESC()

acttable_t	CWeapon_Tripwire::m_acttable[] =
{
	//{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SLAM, false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },

	{ ACT_IDLE_RELAXED, ACT_IDLE, false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED, ACT_RUN, false },
	{ ACT_WALK_RELAXED, ACT_WALK, false },
};

IMPLEMENT_ACTTABLE(CWeapon_Tripwire);

IMPLEMENT_SERVERCLASS_ST(CWeapon_Tripwire, DT_Weapon_TripWire)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_tripwire, CWeapon_Tripwire);
PRECACHE_WEAPON_REGISTER(weapon_tripwire);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeapon_Tripwire::ThrowGrenade(CBasePlayer *pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition(pPlayer, vecEye, vecSrc);
	//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;


	/*CBaseEntity *pEnt = CBaseEntity::Create("npc_tripwire", vecSrc, vec3_angle, pPlayer);
	CTripwireGrenade *pMine = (CTripwireGrenade *)pEnt;
	pMine->SetThrower(pPlayer);

	pMine->ApplyAbsVelocityImpulse(vecThrow);
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);*/

	Tripwire_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER);
	//Fraggrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER, false);

	m_bRedraw = true;

	WeaponSound(SINGLE);

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeapon_Tripwire::LobGrenade(CBasePlayer *pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 350 + Vector(0, 0, 50);

	/*CBaseEntity *pEnt = CBaseEntity::Create("npc_tripwire", vecSrc, vec3_angle, pPlayer);
	CTripwireGrenade *pMine = (CTripwireGrenade *)pEnt;
	pMine->SetThrower(pPlayer);
	pMine->ApplyAbsVelocityImpulse(vecThrow);
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);*/

	Tripwire_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, GRENADE_TIMER);
	//Fraggrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, GRENADE_TIMER, false);

	WeaponSound(WPN_DOUBLE);

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeapon_Tripwire::RollGrenade(CBasePlayer *pPlayer)
{
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &vecSrc);
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D();
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize(vecFacing);
	trace_t tr;
	UTIL_TraceLine(vecSrc, vecSrc - Vector(0, 0, 16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct(vecFacing, tr.plane.normal, tangent);
		CrossProduct(tr.plane.normal, tangent, vecFacing);
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition(pPlayer, pPlayer->WorldSpaceCenter(), vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vecFacing * 700;
	// put it on its side
	QAngle orientation(0, pPlayer->GetLocalAngles().y, -90);
	// roll it
	AngularImpulse rotSpeed(0, 0, 720);


	/*CBaseEntity *pEnt = CBaseEntity::Create("npc_tripwire", vecSrc, orientation, pPlayer);
	CTripwireGrenade *pMine = (CTripwireGrenade *)pEnt;
	pMine->SetThrower(pPlayer);
	pMine->ApplyAbsVelocityImpulse(vecThrow);
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);*/

	Tripwire_Create(vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER);
	//Fraggrenade_Create(vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER, false);

	WeaponSound(SPECIAL1);

	// Send the player 'attack' animation.
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}