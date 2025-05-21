//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npc_combineguard_sphere.h"
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar oc_combineguard_sphere_model("oc_combineguard_sphere_model", "models/props_hive/larval_essence.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");

#define GUARDSPHERE_MODEL	oc_combineguard_sphere_model.GetString() //"models/ShockRifle.mdl"


LINK_ENTITY_TO_CLASS(npc_combineguard_sphere, CNPC_CombineGuard_Sphere);

BEGIN_DATADESC(CNPC_CombineGuard_Sphere)


END_DATADESC()


CNPC_CombineGuard_Sphere *CNPC_CombineGuard_Sphere::GuardSphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *Owner)
{
	// Create a new entity with CShockRifleProjectile private data
	CNPC_CombineGuard_Sphere *GuardSphere = (CNPC_CombineGuard_Sphere *)CreateEntityByName("npc_combineguard_sphere");

	/*CBaseViewModel *vm = pentOwner->GetViewModel();

	if (!vm)
	Sphere->FollowEntity(pentOwner);
	else
	Sphere->FollowEntity(vm);*/

	//UTIL_SetOrigin(GuardSphere, vecOrigin);
	//GuardSphere->FollowEntity(Owner);
	GuardSphere->SetAbsAngles(angAngles);
	GuardSphere->Spawn();
	//GuardSphere->SetOwnerEntity(Owner);

	//Sphere->m_iDamage = iDamage;

	return GuardSphere;
}



CNPC_CombineGuard_Sphere::CNPC_CombineGuard_Sphere(void)
{
}


void CNPC_CombineGuard_Sphere::Precache(void)
{

	PrecacheModel(GUARDSPHERE_MODEL);
	PrecacheModel("models/props_hive/larval_essence.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");

	PrecacheScriptSound("GrenadeSpit.Hit");

	PrecacheParticleSystem("grenade_spit_player");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
}


float ScaleG = 0.0f;
bool RevScaleG = false;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CombineGuard_Sphere::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_BBOX);
	//SetSolidFlags(FSOLID_NOT_SOLID);
	//Vector SizeMin(0.2, 0.2, 0.2), SizeMax(1, 1, 1);
	ScaleG = 0.0f;
	RevScaleG = false;
	//SetModel("models/spitball_large.mdl");
	SetModel(GUARDSPHERE_MODEL);
	//UTIL_SetSize(this, SizeMin, SizeMax);

	SetModelScale(4.1f, 0.0f);

	//SetUse(&CBaseGrenade::DetonateUse);
	//SetTouch(&CCguard_Sphere::SphereTouch);

	SetThink(&CNPC_CombineGuard_Sphere::Think);
	SetNextThink(gpGlobals->curtime + 0.001f);

	//m_takedamage = DAMAGE_NO;
	//m_iHealth = 1;

	DevMsg("SphereInitG");
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


void CNPC_CombineGuard_Sphere::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

void CNPC_CombineGuard_Sphere::Detonate(void)
{

	UTIL_Remove(this);
}



void CNPC_CombineGuard_Sphere::Think(void)
{
//	CBaseEntity *NPC = GetBaseEntity();
	// Add a doppler effect to the balls as they travel
	//CBaseEntity *NPC = AI_GetSinglePlayer();
	//if (NPC != NULL)
	{
		if (ScaleG <= 36.0f && !RevScaleG)
		{
			ScaleG = ScaleG + 0.5f;

			if (ScaleG >= 36.0f)
				RevScaleG = true;
		}
		else if (RevScaleG)
		{

			if (ScaleG > 0.0f && RevScaleG)
				ScaleG = ScaleG - 1.1f;
			else
			{
				ScaleG = 0.0f;
				Detonate();
			}
		}

		SetModelScale(ScaleG, 0.0f);
		DevMsg("ScaleG: %.2f \n", ScaleG);


		/*CAI_BaseNPC *npc = MyNPCPointer();
		if (npc == NULL)
		{
			return;
		}*/
		
		Vector start2;
		QAngle	angShootDir;
		GetAttachment(1, start2, angShootDir);
		vectors(start2);
		//Vector vecShootDir = NPC->GetShootEnemyDir(vecShootOrigin);
		//UTIL_SetOrigin(NPC, start2);
		this->SetAbsAngles(angShootDir);
	}

	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.001f);
}

void CNPC_CombineGuard_Sphere::vectors(const Vector &Origin)
{

}

