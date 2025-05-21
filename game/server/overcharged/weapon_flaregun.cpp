//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose:		Flare gun
//
//				This is a custom extension of Valve's CFlaregun class.
//				Some commented-out code has been duplicated from 
//				weapon_flaregun.cpp in order to keep the mod code isolated
//				from the base game.
//
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "decals.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "props.h"
#include "ai_basenpc.h"

#include "weapon_flaregun.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	oc_wpn_flaregun_flare_lifetime("oc_wpn_flaregun_flare_lifetime", "30", FCVAR_ARCHIVE);
ConVar	oc_wpn_flaregun_flare_stick("oc_wpn_flaregun_flare_stick", "1", FCVAR_ARCHIVE);

IMPLEMENT_SERVERCLASS_ST(CFlaregun, DT_Flaregun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_flaregun, CFlaregun/*Custom*/);
PRECACHE_WEAPON_REGISTER(weapon_flaregun);

acttable_t CFlaregun::m_acttable[] =				// BriJee: Player anims
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },	//fix
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },		// END

	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, true },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_PISTOL_LOW, true },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE/*_PISTOL*/, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_PISTOL, false },//always aims
	{ ACT_IDLE_STEALTH, ACT_IDLE_STEALTH_PISTOL, false },

	{ ACT_WALK_RELAXED, ACT_WALK, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_PISTOL, false },//always aims
	{ ACT_WALK_STEALTH, ACT_WALK_STEALTH_PISTOL, false },

	{ ACT_RUN_RELAXED, ACT_RUN, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_PISTOL, false },//always aims
	{ ACT_RUN_STEALTH, ACT_RUN_STEALTH_PISTOL, false },

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_PISTOL, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_ANGRY_PISTOL, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_PISTOL, false },//always aims
	{ ACT_IDLE_AIM_STEALTH, ACT_IDLE_STEALTH_PISTOL, false },

	{ ACT_WALK_AIM_RELAXED, ACT_WALK, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_PISTOL, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_PISTOL, false },//always aims
	{ ACT_WALK_AIM_STEALTH, ACT_WALK_AIM_STEALTH_PISTOL, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_PISTOL, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_PISTOL, false },//always aims
	{ ACT_RUN_AIM_STEALTH, ACT_RUN_AIM_STEALTH_PISTOL, false },//always aims
	//End readiness activities

	// Crouch activities
	{ ACT_CROUCHIDLE_STIMULATED, ACT_CROUCHIDLE_STIMULATED, false },
	{ ACT_CROUCHIDLE_AIM_STIMULATED, ACT_RANGE_AIM_PISTOL_LOW, false },//always aims
	{ ACT_CROUCHIDLE_AGITATED, ACT_RANGE_AIM_PISTOL_LOW, false },//always aims

	// Readiness translations
	{ ACT_READINESS_RELAXED_TO_STIMULATED, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED, false },
	{ ACT_READINESS_RELAXED_TO_STIMULATED_WALK, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK, false },
	{ ACT_READINESS_AGITATED_TO_STIMULATED, ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED, false },
	{ ACT_READINESS_STIMULATED_TO_RELAXED, ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED, false },
};

IMPLEMENT_ACTTABLE(CFlaregun);

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFlaregun::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("Flare.Touch");
	//PrecacheScriptSound("Weapon_FlareGun.Burn");

	UTIL_PrecacheOther("env_flare");
}

//-----------------------------------------------------------------------------
// Purpose: Fires a flare from a given flaregun with a given velocity
//			Acts like an extension method for CFlaregun
//-----------------------------------------------------------------------------
static void AttackWithVelocity(CFlaregun * flaregun, float projectileVelocity)
{
	CBasePlayer *pOwner = ToBasePlayer(flaregun->GetOwner());

	if (pOwner == NULL)
		return;


	/*pOwner->SetAnimation(PLAYER_ATTACK1);*/
	if (flaregun->m_iClip1 <= 0)
	{
		//flaregun->SendWeaponAnim(ACT_VM_DRYFIRE);
		pOwner->m_flNextAttack = gpGlobals->curtime + flaregun->SequenceDuration();
		return;
	}

	flaregun->m_iClip1 = flaregun->m_iClip1 - 1;


				// BriJee : Player anim
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

	// Overcharged Fire from shoot pos
	Vector Muzzle;
	Muzzle = flaregun->GetClientMuzzleVector();
	/*Muzzle.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
	Muzzle.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
	Muzzle.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();*/

	CFlare *pFlare = CFlareGunProjectile::Create(Muzzle, pOwner->EyeAngles(), pOwner, FLARE_DURATION);

	if (pFlare == NULL)
		return;

	Vector forward;
	pOwner->EyeVectors(&forward);
	forward *= projectileVelocity;
	forward += pOwner->GetAbsVelocity(); // Add the player's velocity to the forward vector so that the flare follows the player's motion
	forward.Normalized();

	pFlare->SetAbsVelocity(forward);
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction(0.85f);
	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	flaregun->WeaponSound(SINGLE);
}

//-----------------------------------------------------------------------------
// Purpose: Main attack
//-----------------------------------------------------------------------------
void CFlaregun::PrimaryAttack(void)
{
	AddViewKick();
	SendWeaponAnim(GetPrimaryAttackActivity());
	
	AttackWithVelocity(this, 1500); //flaregun_primary_velocity.GetFloat());
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack - launches flares closer to the player
//-----------------------------------------------------------------------------
void CFlaregun::SecondaryAttack(void)
{
	AddViewKick();
	SendWeaponAnim(GetPrimaryAttackActivity());
	AttackWithVelocity(this, 500); //flaregun_secondary_velocity.GetFloat());
	m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();

	SecondaryAttackEffects();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*bool CFlaregun::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return fRet;
}*/

const WeaponProficiencyInfo_t *CFlaregun::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 3.0, 0.75 },
		{ 4.00, 0.75 },
		{ 10.0 / 2.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);
	return proficiencyTable;
}



//-----------------------------------------------------------------------------
// Purpose: Create function for Flare Gun projectile
// Input  : vecOrigin - 
//			vecAngles - 
//			*pOwner - 
// Output : CFlare
//-----------------------------------------------------------------------------
CFlareGunProjectile *CFlareGunProjectile::Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime)
{
	CFlareGunProjectile *pFlare = (CFlareGunProjectile *)CreateEntityByName("env_flare");

	if (pFlare == NULL)
		return NULL;

	UTIL_SetOrigin(pFlare, vecOrigin);

	pFlare->SetLocalAngles(vecAngles);
	pFlare->Spawn();
	pFlare->SetTouch(&CFlareGunProjectile::FlareGunProjectileTouch);
	pFlare->SetThink(&CFlare::FlareThink);
	pFlare->m_bLight = true; //flaregun_dynamic_lights.GetBool();	// BriJee Always light

	//Start up the flare
	pFlare->Start(lifetime);

	//Don't start sparking immediately
	pFlare->SetNextThink(gpGlobals->curtime + 0.5f);

	//Burn out time
	pFlare->m_flTimeBurnOut = gpGlobals->curtime + lifetime;

	// Time to next burn damage
	pFlare->m_flNextDamage = gpGlobals->curtime;


	pFlare->RemoveSolidFlags(FSOLID_NOT_SOLID);
	pFlare->AddSolidFlags(FSOLID_NOT_STANDABLE);

	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	pFlare->SetOwnerEntity(pOwner);
	pFlare->m_pOwner = pOwner;

	return pFlare;
}

//-----------------------------------------------------------------------------
// Purpose: Touch function for flaregun projectiles
// Input  : *pOther - The entity that the flare has collided with
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	if (!pOther->IsSolid() || FClassnameIs(pOther, "prop_vehicle_jeep") || FClassnameIs(pOther, "prop_vehicle_apc") || FClassnameIs(pOther, "prop_vehicle_airboat"))
		return;

	/*if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))	//test
	{
	// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
	if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
		return;
	}	//test*/
	//Msg("custom flare code\n");

	//UTIL_Remove(this);

	if ((m_nBounces < 10) && (GetWaterLevel() < 1))
	{
		// Throw some real chunks here
		g_pEffects->Sparks(GetAbsOrigin());
	}

	//If the flare hit a person or NPC, do damage here.
	if (pOther && pOther->m_takedamage)
	{
		// BriJee Flare touch damage is back
		int iDamage = GetAbsVelocity().Length() / 50.0f;
		iDamage = 12;	//5
		pOther->TakeDamage(CTakeDamageInfo(this, m_pOwner, iDamage, (DMG_BULLET | DMG_BURN)));
		m_flNextDamage = gpGlobals->curtime + 1.0f;
		// BriJee Flare touch damage is back

		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity *= 0.1f;
		SetAbsVelocity(vecNewVelocity);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		SetGravity(1.0f);
		Die(0.5);
		IgniteOtherIfAllowed(pOther);
		m_nBounces++;
		return;
	}
	else
	{
		// hit the world, check the material type here, see if the flare should stick.
		trace_t tr;
		tr = CBaseEntity::GetTouchTrace();

		//	brush collision fix???
		/*Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward  128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);*/
		// brush collision fix???

		//Only do this on the first bounce if the convar is set
		if (oc_wpn_flaregun_flare_stick.GetBool() && m_nBounces == 0)
		{
			const surfacedata_t *pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);

			if (pdata != NULL)
			{
				//Only embed into concrete and wood (jdw: too obscure for players?)
				//if ( ( pdata->gameMaterial == 'C' ) || ( pdata->gameMaterial == 'W' ) )
				{
					Vector	impactDir = (tr.endpos - tr.startpos);
					VectorNormalize(impactDir);

					float	surfDot = tr.plane.normal.Dot(impactDir);

					//Do not stick to ceilings or on shallow impacts
					if ((tr.plane.normal.z > -0.5f) && (surfDot < -0.9f))
					{
						RemoveSolidFlags(FSOLID_NOT_SOLID);
						AddSolidFlags(FSOLID_TRIGGER);
						UTIL_SetOrigin(this, tr.endpos + (tr.plane.normal * 2.0f));
						SetAbsVelocity(vec3_origin);
						SetMoveType(MOVETYPE_NONE);

						SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);

						int index = decalsystem->GetDecalIndexForName("SmallScorch");
						if (index >= 0)
						{
							CBroadcastRecipientFilter filter;
							te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
						}

						CPASAttenuationFilter filter2(this, "Flare.Touch");
						EmitSound(filter2, entindex(), "Flare.Touch");

						return;
					}
				}
			}
		}

		//Scorch decal
		if (GetAbsVelocity().LengthSqr() > (250 * 250))
		{
			int index = decalsystem->GetDecalIndexForName("FadingScorch");
			if (index >= 0)
			{
				CBroadcastRecipientFilter filter;
				te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
			}
		}

		// Change our flight characteristics
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		SetGravity(UTIL_ScaleForGravity(640));

		m_nBounces++;

		//After the first bounce, smacking into whoever fired the flare is fair game
		//SetOwnerEntity(this);	// BriJee Water bug fix

		// Slow down
		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity.x *= 0.8f;
		vecNewVelocity.y *= 0.8f;
		SetAbsVelocity(vecNewVelocity);
		
		//Stopped?
		if (GetAbsVelocity().Length() < 128)//flaregun_stop_velocity.GetFloat())
		{
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			AddSolidFlags(FSOLID_TRIGGER);
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
			SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileBurnTouch(CBaseEntity *pOther)
{
	if (pOther && pOther->m_takedamage && (m_flNextDamage < gpGlobals->curtime) && (!FClassnameIs(pOther, "prop_vehicle_jeep") && !FClassnameIs(pOther, "prop_vehicle_apc") && !FClassnameIs(pOther, "prop_vehicle_airboat")))
	{
		// Don't do damage - I want consistent behavior between initial collisions and after landing collisions
		// pOther->TakeDamage(CTakeDamageInfo(this, m_pOwner, 1, (DMG_BULLET | DMG_BURN)));
		m_flNextDamage = gpGlobals->curtime + 1.0f;
		IgniteOtherIfAllowed(pOther);
	}
}

void CFlareGunProjectile::IgniteOtherIfAllowed(CBaseEntity * pOther)
{
	// Don't burn the player
	if (pOther->IsPlayer())
		return;

	CAI_BaseNPC *pNPC;
	pNPC = dynamic_cast<CAI_BaseNPC*>(pOther);
	if (pNPC) {
		// Don't burn friendly NPCs
		if (pNPC->IsPlayerAlly())
			return;

		// Don't burn boss enemies
		if (FStrEq(STRING(pNPC->m_iClassname), "npc_combinegunship")		// BriJee: Check for hard npcs, dont burn them
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_combinedropship")
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_strider")
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_helicopter")
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_rollermine")
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_advisor")		
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_advisor_combat")
			|| FStrEq(STRING(pNPC->m_iClassname), "npc_advisor_fighter")
			)
			return;

		// Burn this NPC
		pNPC->IgniteLifetime(oc_wpn_flaregun_flare_lifetime.GetFloat());
	}

	// If this is a breakable prop, ignite it!
	CBreakableProp *pBreakable;
	pBreakable = dynamic_cast<CBreakableProp*>(pOther);
	if (pBreakable)
		pBreakable->IgniteLifetime(oc_wpn_flaregun_flare_lifetime.GetFloat());
}

