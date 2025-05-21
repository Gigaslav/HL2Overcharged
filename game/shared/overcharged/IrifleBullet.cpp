//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "IrifleBullet.h"
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
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_irifle_bullet_dmg("sk_irifle_bullet_dmg", "20", FCVAR_NONE, "Total damage done by an individual antlion worker loogie.");
ConVar    oc_IrifleBulletSpeed("oc_IrifleBulletSpeed", "3000", FCVAR_NONE, "Total damage done by an individual antlion worker loogie.");

LINK_ENTITY_TO_CLASS(irifle_bullet, CIrifleBullet);

BEGIN_DATADESC(CIrifleBullet)

//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

//IMPLEMENT_SERVERCLASS_ST(CShotgunBullet, DT_ShotgunBullet)
//END_SEND_TABLE()

CIrifleBullet *CIrifleBullet::BoltCreate(/*const Vector &vecOrigin, const QAngle &angAngles, */CBasePlayer *pentOwner)
{
	// Create a new entity with CCrossbowBolt private data
	CIrifleBullet *pBolt = (CIrifleBullet *)CreateEntityByName("irifle_bullet");
	//UTIL_SetOrigin(pBolt, vecOrigin);
	//pBolt->SetAbsAngles(angAngles);
	pBolt->Spawn(pentOwner);
	//pBolt->SetOwnerEntity(pentOwner);
	return pBolt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CIrifleBullet::~CIrifleBullet(void)
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CShotgunBullet::CreateVPhysics(void)
{
// Create the object in the physics system
VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

return true;
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*unsigned int CShotgunBullet::PhysicsSolidMaskForEntity() const
{
return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool DoOnceMS = true;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CIrifleBullet::Spawn(CBasePlayer *pPlayer)
{
	Precache();
	//AddEffects(EF_NODRAW);


	//CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	//SetModel("models/weapons/bullets/ar2.mdl");
	//SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	//UTIL_SetSize(this, -Vector(0.3f, 0.3f, 0.3f), Vector(0.3f, 0.3f, 0.3f));
	//SetSolid(SOLID_CUSTOM);
	//SetGravity(0.05f);

	// Make sure we're updated if we're underwater
	//UpdateWaterState();

	if (cvar->FindVar("oc_event_magout")->GetInt() == 1)
	{
		//DevMsg("Mag dropped");
		Vector attachOrigin;
		QAngle attachAngles;

		if (pPlayer->GetAttachment("muzzle", attachOrigin, attachAngles))
		{
			/*DevMsg("Finded attachment MAG");
			CBaseEntity *m_add = CreateEntityByName("prop_dynamic");

			m_add->SetSolid(SOLID_BBOX);
			m_add->SetCollisionGroup(COLLISION_GROUP_WEAPON);
			m_add->SetModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");
			m_add->SetAbsOrigin(attachOrigin);
			m_add->SetAbsAngles(attachAngles);
			m_add->Spawn();
			m_add->SetMoveType(MOVETYPE_VPHYSICS);*/
			Vector Origin;
			QAngle Ang;

			Origin.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
			Origin.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
			Origin.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();

			Ang.x = cvar->FindVar("oc_event_magout_ang_x")->GetFloat();
			Ang.y = cvar->FindVar("oc_event_magout_ang_y")->GetFloat();
			Ang.z = cvar->FindVar("oc_event_magout_ang_z")->GetFloat();


			if (cvar->FindVar("oc_mag_test_pos_enabled")->GetInt() == 1)
			{
				Vector forward, right, up;
				pPlayer->EyeVectors(&forward, &right, &up);
				AngleVectors(Ang, &forward, &right, &up);

				if (DoOnceMS)
				{
					cvar->FindVar("oc_mag_test_pos_x")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.x);
					cvar->FindVar("oc_mag_test_pos_y")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.y);
					cvar->FindVar("oc_mag_test_pos_z")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.z);

					cvar->FindVar("oc_mag_test_ang_x")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.x);
					cvar->FindVar("oc_mag_test_ang_y")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.y);
					cvar->FindVar("oc_mag_test_ang_z")->SetValue(pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.z);
					DoOnceMS = false;
				}
				Origin += forward * cvar->FindVar("oc_mag_test_pos_x")->GetFloat();
				Origin += right * cvar->FindVar("oc_mag_test_pos_y")->GetFloat();
				Origin += up * cvar->FindVar("oc_mag_test_pos_z")->GetFloat();

				Ang.x += cvar->FindVar("oc_mag_test_ang_x")->GetFloat();
				Ang.y += cvar->FindVar("oc_mag_test_ang_y")->GetFloat();
				Ang.z += cvar->FindVar("oc_mag_test_ang_z")->GetFloat();
			}
			else if (cvar->FindVar("oc_mag_test_pos_enabled")->GetInt() == 0)
			{
				DoOnceMS = true;
				Vector forward, right, up;
				pPlayer->EyeVectors(&forward, &right, &up);
				AngleVectors(Ang, &forward, &right, &up);

				Origin += forward * pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.x;
				Origin += right * pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.y;
				Origin += up * pPlayer->GetActiveWeapon()->GetWpnData().Mag_Offset.z;

				Ang.x += pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.x;
				Ang.y += pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.y;
				Ang.z += pPlayer->GetActiveWeapon()->GetWpnData().Mag_Angle.z;

			}

			int MagNumber = pPlayer->GetActiveWeapon()->GetWpnData().MagType;
			CBaseEntity *pVial = NULL; // Null
			pVial = CBaseEntity::Create("item_empty_mag", Origin, Ang, pPlayer); // Creates



			switch (MagNumber)
			{
			case 0:
			{
				pVial->PrecacheModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");
				pVial->SetModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");
			}
			break;
			case 1:
			{
				pVial->PrecacheModel("models/weapons/MAGS/w_pist_p250_mag.mdl");
				pVial->SetModel("models/weapons/MAGS/w_pist_p250_mag.mdl");
			}
			break;
			case 2:
			{
				pVial->PrecacheModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");
				pVial->SetModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");
			}
			break;
			case 3:
			{
				pVial->PrecacheModel("models/weapons/MAGS/w_mach_m249_mag.mdl");
				pVial->SetModel("models/weapons/MAGS/w_mach_m249_mag.mdl");
			}
			break;
			default:
				break;
			}


			pVial->SetSolid(SOLID_BBOX);
			pVial->SetCollisionGroup(COLLISION_GROUP_WEAPON);
			pVial->SetMoveType(MOVETYPE_VPHYSICS);
			pVial->AddSpawnFlags(SF_NORESPAWN); // Doesn't allow respawning
			pVial->SetLocalOrigin(Origin);
			pVial->SetLocalAngles(Ang);
			pVial->Spawn();
			/*CBaseEntity *pGib;
			pGib = CreateRagGib("models/cremator_headprop.mdl", Origin, Ang, Vector(0, 0, 0), 10, false);
			pGib->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
			pGib->SetMoveType(MOVETYPE_VPHYSICS);*/
		}
		cvar->FindVar("oc_event_magout")->SetValue(0);
	}
}


void CIrifleBullet::Precache(void)
{
	//PrecacheModel(BULLET_MODEL);

	// This is used by C_TEStickyBolt, despte being different from above!!!
	PrecacheModel("models/weapons/bullets/ar2.mdl");
	PrecacheParticleSystem("IBullet_exhaust");
	PrecacheParticleSystem("AR2_tracer01");
	PrecacheParticleSystem("AR2_Impact");
	PrecacheModel("effects/gunshiptracer.vmt");
	PrecacheModel("effects/spark.vmt");
}


