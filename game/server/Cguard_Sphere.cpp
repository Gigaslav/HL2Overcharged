//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "Cguard_Sphere.h"
#include "soundent.h"
#include "decals.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"
#include "OverCharged/weapon_cguard.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar oc_weapon_cguard_sphere_model("oc_weapon_cguard_sphere_model", "models/ballsphere.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");
ConVar oc_weapon_cguard_minscale("oc_weapon_cguard_minscale", "2", FCVAR_ARCHIVE);

#define SPHERE_MODEL	oc_weapon_cguard_sphere_model.GetString() //"models/ShockRifle.mdl"


LINK_ENTITY_TO_CLASS(cguard_sphere, CCguard_Sphere);

BEGIN_DATADESC(CCguard_Sphere)

	DEFINE_FIELD(Scale, FIELD_TIME),
	DEFINE_FIELD(RevScale, FIELD_BOOLEAN),
	DEFINE_THINKFUNC(Think),

END_DATADESC()


CCguard_Sphere *CCguard_Sphere::SphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBaseViewModel *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CCguard_Sphere *Sphere = (CCguard_Sphere *)CreateEntityByName("cguard_sphere");

	/*CBaseViewModel *vm = pentOwner->GetViewModel();

	if (!vm)
		Sphere->FollowEntity(pentOwner);
	else
	    Sphere->FollowEntity(vm);*/

	UTIL_SetOrigin(Sphere, vecOrigin);
	Sphere->SetAbsAngles(angAngles);
	Sphere->Spawn();
	Sphere->SetOwnerEntity(pentOwner);

	//Sphere->FollowEntity(pentOwner);
	//Sphere->m_iDamage = iDamage;

	return Sphere;
}



CCguard_Sphere::CCguard_Sphere(void)
{
	Scale = 0.0f;
	RevScale = false;
}


void CCguard_Sphere::Precache(void)
{
	// m_nSquidSpitSprite = PrecacheModel("sprites/greenglow1.vmt");// client side spittle.

	PrecacheModel(SPHERE_MODEL);
	PrecacheModel("models/ballsphere.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");

	PrecacheScriptSound("GrenadeSpit.Hit");

	PrecacheParticleSystem("grenade_spit_player");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCguard_Sphere::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_NONE);
	//SetSolidFlags(FSOLID_NOT_SOLID);
	//Vector SizeMin(0.2, 0.2, 0.2), SizeMax(1, 1, 1);
	Scale = 0.0f;
	RevScale = false;
	//SetModel("models/spitball_large.mdl");
	SetModel(SPHERE_MODEL);
	//UTIL_SetSize(this, SizeMin, SizeMax);

	SetModelScale(cvar->FindVar("oc_weapon_cguard_minscale")->GetFloat(), 0.0f);

	//SetUse(&CBaseGrenade::DetonateUse);
	//SetTouch(&CCguard_Sphere::SphereTouch);

	SetThink(&CCguard_Sphere::Think);
	SetNextThink(gpGlobals->curtime + 0.001f);

	//m_takedamage = DAMAGE_NO;
	//m_iHealth = 1;

	//DevMsg("SphereInit");
	//SetGravity(UTIL_ScaleForGravity(SPIT_GRAVITY));
	//SetFriction(0.8f);

	//SetCollisionGroup(HL2COLLISION_GROUP_SPIT);

	//AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	// We're self-illuminating, so we don't take or give shadows
	//AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	/*DispatchParticleEffect("grenade_spit_trail", PATTACH_ABSORIGIN_FOLLOW, this);

	// Create the dust effect in place
	m_hSphereEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSphereEffect != NULL)
	{
		// Setup our basic parameters
		m_hSphereEffect->KeyValue("start_active", "1");
		m_hSphereEffect->KeyValue("effect_name", "antlion_spit_trail");
		m_hSphereEffect->SetParent(this);
		m_hSphereEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSphereEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSphereEffect->Activate();
	}*/
}


void CCguard_Sphere::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Handle spitting
//-----------------------------------------------------------------------------
/*void CCguard_Sphere::SphereTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
		return;

	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ((pTrace->contents & CONTENTS_WATER) != 0);
	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if (bHitWater)
	{
		// Splash!
		CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector(0, 0, 1);
		data.m_flScale = 8.0f;

		DispatchEffect("watersplash", data);
	}
	else
	{
		// Make a splat decal
		trace_t *pNewTrace = const_cast<trace_t*>(pTrace);
		UTIL_DecalTrace(pNewTrace, "SpitSplat");
	}



	QAngle vecAngles;
	VectorAngles(tracePlaneNormal, vecAngles);

	if (pOther->IsPlayer() || bHitWater)
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect("grenade_spit_player", GetAbsOrigin(), vecAngles);
	}
	else
	{
		DispatchParticleEffect("grenade_spit", GetAbsOrigin(), vecAngles);
	}

	Detonate();
}*/

void CCguard_Sphere::Detonate(void)
{
	/*m_takedamage = DAMAGE_NO;

	EmitSound("GrenadeSpit.Hit");

	// Stop our hissing sound
	if (m_pHissSound != NULL)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pHissSound);
		m_pHissSound = NULL;
	}
	*/

	UTIL_Remove(this);
}


ConVar oc_weapon_cguard_maxscale("oc_weapon_cguard_maxscale", "6", FCVAR_ARCHIVE);
ConVar oc_weapon_cguard_increment_scale("oc_weapon_cguard_increment_scale", "0.1", FCVAR_ARCHIVE);
ConVar oc_weapon_cguard_decrement_scale("oc_weapon_cguard_decrement_scale", "0.9", FCVAR_ARCHIVE);

ConVar oc_weapon_cguard_sphere_pos_x("oc_weapon_cguard_sphere_pos_x", "5", FCVAR_ARCHIVE);
ConVar oc_weapon_cguard_sphere_pos_y("oc_weapon_cguard_sphere_pos_y", "0.1", FCVAR_ARCHIVE);
ConVar oc_weapon_cguard_sphere_pos_z("oc_weapon_cguard_sphere_pos_z", "0.9", FCVAR_ARCHIVE);
void CCguard_Sphere::Think(void)
{

	// Add a doppler effect to the balls as they travel
	CBaseEntity *pPlayer = AI_GetSinglePlayer();
	if (pPlayer != NULL)
	{
		if (Scale <= cvar->FindVar("oc_weapon_cguard_maxscale")->GetFloat() && !RevScale)
		{
			Scale = Scale + (cvar->FindVar("oc_weapon_cguard_increment_scale")->GetFloat() * gpGlobals->frametime);

			if (Scale >= cvar->FindVar("oc_weapon_cguard_maxscale")->GetFloat())
				RevScale = true;
		}
		else if (RevScale)
		{
			Scale = 0.0f;
			Detonate();

			/*if (Scale > 0.0f && RevScale)
				Scale = Scale - (cvar->FindVar("oc_weapon_cguard_decrement_scale")->GetFloat() * gpGlobals->frametime);
				else
				{
				Scale = 0.0f;
				Detonate();
				}*/
		}
		//SetParent(pPlayer, 1);
		SetModelScale(Scale, 0.0f);
		//DevMsg("Scale: %.2f \n", Scale);

		CWeaponCGuard *pParentWeapon = (CWeaponCGuard*)GetParent();
		if (pParentWeapon)
		{
			Vector MuzzleS;
			MuzzleS = pParentWeapon->GetClientMuzzleVector();

			QAngle MuzzleAngle;
			MuzzleAngle = pParentWeapon->GetClientMuzzleAngles();
			/*MuzzleAngle.x = cvar->FindVar("oc_muzzle_angle_x")->GetFloat();
			MuzzleAngle.y = cvar->FindVar("oc_muzzle_angle_y")->GetFloat();
			MuzzleAngle.z = cvar->FindVar("oc_muzzle_angle_z")->GetFloat();*/

			MuzzleAngle.x = MuzzleAngle.x - 90.0f;

			Vector forward(1, 0, 0), right(0, 1, 0), up(0, 0, 1);

			/*MuzzleS.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
			MuzzleS.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
			MuzzleS.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();*/

			AngleVectors(MuzzleAngle, &forward, &right, &up);
			MuzzleS += forward * oc_weapon_cguard_sphere_pos_x.GetFloat();
			MuzzleS += right * oc_weapon_cguard_sphere_pos_y.GetFloat();
			MuzzleS += up * oc_weapon_cguard_sphere_pos_z.GetFloat();


			UTIL_SetOrigin(this, MuzzleS);
			this->SetAbsAngles(MuzzleAngle);
		}
	}
	else
	{
		Detonate();
	}

	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.001f);
}


