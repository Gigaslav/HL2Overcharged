//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "AR2Bullet.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "ar2_explosion.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"
#include "particle_parse.h"//OverCharged
#ifdef PORTAL
#include "portal_util_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define NO_COLLIDE_TIME 0.2
#define AR2_GRENADE_MAX_DANGER_RADIUS	300

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

// Moved to HL2_SharedGameRules because these are referenced by shared AmmoDef functions
ConVar    sk_plr_dmg_ar2_bullet("sk_plr_dmg_ar2_bullet", "90");
ConVar    sk_npc_dmg_ar2_bullet("sk_npc_dmg_ar2_bullet", "20");
ConVar    sk_max_ar2_bullet("sk_max_ar2_bullet", "2000");

ConVar	  sk_ar2_bullet_radius("sk_ar2_bullet_radius", "0");

ConVar g_CV_SmokeTrail_ar2("smoke_trail_ar2", "1", 0); // temporary dust explosion switch

BEGIN_DATADESC(CAR2Bullet)

DEFINE_FIELD(m_hSmokeTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_fSpawnTime, FIELD_TIME),
DEFINE_FIELD(m_fDangerRadius, FIELD_FLOAT),

// Function pointers
DEFINE_ENTITYFUNC(AR2BulletTouch),
DEFINE_THINKFUNC(AR2BulletThink),

END_DATADESC()

LINK_ENTITY_TO_CLASS(ar2bullet, CAR2Bullet);

void CAR2Bullet::Spawn(void)
{
	Precache();
	SetSolid(SOLID_BBOX);
//	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	// Hits everything but debris
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);

	SetModel("models/Weapons/ar2_grenade.mdl");
	UTIL_SetSize(this, Vector(-1, -1, -1), Vector(1, 1, 1));
	//	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	AddEffects(EF_NODRAW);
	SetUse(&CAR2Bullet::DetonateUse);
	SetTouch(&CAR2Bullet::AR2BulletTouch);
	SetThink(&CAR2Bullet::AR2BulletThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
	{
		m_flDamage = sk_plr_dmg_ar2_bullet.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_ar2_bullet.GetFloat();
	}

	m_DmgRadius = sk_ar2_bullet_radius.GetFloat();
	m_takedamage = DAMAGE_YES;
	m_bIsLive = true;
	m_iHealth = 1;

//	SetGravity(UTIL_ScaleForGravity(100));	// use a lower gravity for grenades to make them easier to see
	SetFriction(0.8);
	SetSequence(0);

	m_fDangerRadius = 100;

	m_fSpawnTime = gpGlobals->curtime;


	//	UTIL_Tracer(vecOrigin, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 600, true, "StriderTracer");

	// -------------//OverCharged
	// Smoke trail.
	// -------------
	if (g_CV_SmokeTrail_ar2.GetInt() && !IsXbox())
	{

		DispatchParticleEffect("shell_exhaust_smoke", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма

		/////////////////////////////Старый и никому ненужный эффект дыма//////////////////////////////
		/*		m_hSmokeTrail = SmokeTrail::CreateSmokeTrail();

		if( m_hSmokeTrail )
		{
		m_hSmokeTrail->m_SpawnRate = 48;
		m_hSmokeTrail->m_ParticleLifetime = 1;
		m_hSmokeTrail->m_StartColor.Init(0.1f, 0.1f, 0.1f);
		m_hSmokeTrail->m_EndColor.Init(0,0,0);
		m_hSmokeTrail->m_StartSize = 12;
		m_hSmokeTrail->m_EndSize = m_hSmokeTrail->m_StartSize * 4;
		m_hSmokeTrail->m_SpawnRadius = 4;
		m_hSmokeTrail->m_MinSpeed = 4;
		m_hSmokeTrail->m_MaxSpeed = 24;
		m_hSmokeTrail->m_Opacity = 0.2f;

		m_hSmokeTrail->SetLifetime(10.0f);
		m_hSmokeTrail->FollowEntity(this);
		}*/
		///////////////////////////////////////////////////////////////////////////////////////////////
	}
}

//-----------------------------------------------------------------------------
// Purpose:  The grenade has a slight delay before it goes live.  That way the
//			 person firing it can bounce it off a nearby wall.  However if it
//			 hits another character it blows up immediately
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CAR2Bullet::AR2BulletThink(void)
{
	SetNextThink(gpGlobals->curtime + 0.01f);

	if (!m_bIsLive)
	{
		// Go live after a short delay
		if (m_fSpawnTime + NO_COLLIDE_TIME < gpGlobals->curtime)
		{
			m_bIsLive = true;
		}
	}

	// If I just went solid and my velocity is zero, it means I'm resting on
	// the floor already when I went solid so blow up
	if (m_bIsLive)
	{
		if (GetAbsVelocity().Length() == 0.0 ||
			GetGroundEntity() != NULL)
		{
			Detonate();
		}
	}

	// The old way of making danger sounds would scare the crap out of EVERYONE between you and where the grenade
	// was going to hit. The radius of the danger sound now 'blossoms' over the grenade's lifetime, making it seem
	// dangerous to a larger area downrange than it does from where it was fired.
	if (m_fDangerRadius <= AR2_GRENADE_MAX_DANGER_RADIUS)
	{
		m_fDangerRadius += (AR2_GRENADE_MAX_DANGER_RADIUS * 0.05);
	}

	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 255, 95, 55, 8, 100, 0.1, 0);//OverCharged

	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, m_fDangerRadius, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER);
}

void CAR2Bullet::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

void CAR2Bullet::AR2BulletTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	if (!pOther->IsSolid())
		return;

	// If I'm live go ahead and blow up
	if (m_bIsLive)
	{
		Detonate();
	}
	else
	{
		// If I'm not live, only blow up if I'm hitting an chacter that
		// is not the owner of the weapon
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(pOther);
		if (pBCC && GetThrower() != pBCC)
		{
			m_bIsLive = true;
			Detonate();
		}
	}
}

void CAR2Bullet::Detonate(void)
{
	if (!m_bIsLive)
	{
		return;
	}
	m_bIsLive = false;
	m_takedamage = DAMAGE_NO;

	if (m_hSmokeTrail)
	{
		UTIL_Remove(m_hSmokeTrail);
		m_hSmokeTrail = NULL;
	}

	CPASFilter filter(GetAbsOrigin());

	te->Explosion(filter, 0.0,
		&GetAbsOrigin(),
		g_sModelIndexFireball,
		2.0,
		15,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		m_flDamage);

	Vector vecForward = GetAbsVelocity();
	VectorNormalize(vecForward);
	trace_t		tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + 60 * vecForward, MASK_SHOT,
		this, COLLISION_GROUP_NONE, &tr);


	if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
	{
		// non-world needs smaller decals
		if (tr.m_pEnt && !tr.m_pEnt->IsNPC())
		{
			UTIL_DecalTrace(&tr, "SmallScorch");
		}
	}
	else
	{
		UTIL_DecalTrace(&tr, "Scorch");
	}

	UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START);

	DispatchParticleEffect("explosion_turret_break", GetAbsOrigin(), GetAbsAngles());//OverCharged
	//	DispatchParticleEffect("grenade_explosion_01", GetAbsOrigin(), GetAbsAngles());//OverCharged

	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 255, 95, 55, 8, 100, 0.1, 0);//OverCharged

	RadiusDamage(CTakeDamageInfo(this, GetThrower(), m_flDamage, DMG_BLAST), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);

	UTIL_Remove(this);
}

void CAR2Bullet::Precache(void)
{
	PrecacheModel("models/Weapons/ar2_grenade.mdl");
}


CAR2Bullet::CAR2Bullet(void)
{
	m_hSmokeTrail = NULL;
}
