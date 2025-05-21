//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "soundent.h"
#include "te_particlesystem.h"
#include "ndebugoverlay.h"
#include "in_buttons.h"
#include "ai_basenpc.h"
#include "ai_memory.h"
#include "shake.h"
#include "gamerules.h"
#include "te_effect_dispatch.h"
#include "decals.h"
#include "IEffects.h"
#include "AmmoDef.h"
#include "soundenvelope.h"
#include "particle_parse.h"
#include "game.h"
#include "NPCevent.h"
#include "weapon_laser.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define LASER_AMMOUSING_TIME		0.1
#define LASER_DAMAGE_TIME			0.02
#define LASER_BEAM_TEXTURE			"effects/Laser/laser.vmt"
#define LASER_HALO_BEAM_TEXTURE		"effects/Laser/laserhalo.vmt"
#define ADDITIONAL_SPRITE			"effects/Laser/aquaDot.vmt"
#define MAX_BURN_RADIUS				256
#define RADIUS_GROW_RATE			50.0
#define LASER_TARGET_INVALID		Vector( FLT_MAX, FLT_MAX, FLT_MAX )

extern ConVar						sk_plr_dmg_laser_wide("sk_plr_dmg_laser_wide", "7");
ConVar								oc_weapon_laser_firewater_frequency("oc_weapon_laser_firewater_frequency", "0", (FCVAR_REPLICATED), "Egon damage.");

IMPLEMENT_SERVERCLASS_ST(CWeaponLaser, DT_WeaponLaser)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(info_target_laser, CPointEntity);
LINK_ENTITY_TO_CLASS(weapon_laser, CWeaponLaser);
PRECACHE_WEAPON_REGISTER(weapon_laser);

BEGIN_DATADESC(CWeaponLaser)

	DEFINE_FIELD(m_bFirstPlay, FIELD_BOOLEAN),
	DEFINE_FIELD(m_flBurnRadius, FIELD_FLOAT),
	DEFINE_FIELD(m_vecLaserTarget, FIELD_VECTOR),
	DEFINE_FIELD(m_BeamFireState, FIELD_INTEGER),
	DEFINE_FIELD(m_SSprite, FIELD_EHANDLE),
	DEFINE_FIELD(m_flChargeTime, FIELD_TIME),
	DEFINE_FIELD(m_flDmgTime, FIELD_TIME),
	DEFINE_FIELD(m_flAmmoTime, FIELD_TIME),
	DEFINE_FIELD(m_hBeam, FIELD_EHANDLE),
	DEFINE_FIELD(m_hHaloBeam, FIELD_EHANDLE),

END_DATADESC()

//-----------------------------------------------------------------------------
// Maps base activities to weapons-specific ones so our characters do the right things.
//-----------------------------------------------------------------------------
acttable_t CWeaponLaser::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },

	{ ACT_IDLE, ACT_IDLE_SMG1, true },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },

	//{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SNIPER_RIFLE, true }
};

IMPLEMENT_ACTTABLE(CWeaponLaser);

CWeaponLaser::CWeaponLaser(void)
{
	m_BeamFireState = FIRE_NONE;
	m_bFirstPlay =			true;
	m_flChargeTime =		gpGlobals->curtime;
	m_flDmgTime =			gpGlobals->curtime;
	m_flAmmoTime =			gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLaser::Precache(void)
{
	PrecacheMaterial(LASER_BEAM_TEXTURE);
	PrecacheMaterial(LASER_HALO_BEAM_TEXTURE);
	PrecacheMaterial(ADDITIONAL_SPRITE);
	BaseClass::Precache();
}

void CWeaponLaser::UseAmmo(int count)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) >= count)
		RemoveAmmo(GetPrimaryAmmoType(), count); //pPlayer->RemoveAmmo(count, m_iPrimaryAmmoType);
	else
		RemoveAmmo(GetPrimaryAmmoType(), pPlayer->GetAmmoCount(m_iPrimaryAmmoType)); //pPlayer->RemoveAmmo(pPlayer->GetAmmoCount(m_iPrimaryAmmoType), m_iPrimaryAmmoType);
}

Activity CWeaponLaser::GetPrimaryAttackActivity(void)
{
	if (m_BeamFireState == FIRE_STARTUP)
		return BaseClass::GetPrimaryAttackActivity();
	else if (m_BeamFireState == FIRE_LOOP)
		return GetWpnData().animData[m_bFireMode].FirePrimarySpecial;

	return BaseClass::GetPrimaryAttackActivity();
}

void CWeaponLaser::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		return;
	}

	if (GetActivity() != GetPrimaryAttackActivity())
		SendWeaponAnim(GetPrimaryAttackActivity());

	switch (m_BeamFireState)
	{
		case FIRE_NONE:
		{
			m_BeamFireState = FIRE_STARTUP;
		}
		break;
		case FIRE_STARTUP:
		{
			if (m_flNextPrimaryAttack <= gpGlobals->curtime)
			{
				if (m_BeamFireState == FIRE_STARTUP)
					Charge();
				WeaponSound(SPECIAL1);
				m_BeamFireState = FIRE_LOOP;
				m_bFirstPlay = true;
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flDmgTime = m_flAmmoTime = gpGlobals->curtime + GetWpnData().mode_fire_rate_automatic_startup_delay;
			}
		}
		break;
		case FIRE_LOOP:
		{
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime;// +GetFireRate();

			if (pPlayer->GetWaterLevel() != 3)
			{
				if (m_bFirstPlay)
				{
					/*CEffectData Data;
					Data.m_nEntIndex = pPlayer->GetViewModel()->entindex();
					data.m_vOrigin = 
					DispatchEffect("LaserSparksMuzzle", Data);*/
					m_flDmgTime = m_flAmmoTime = gpGlobals->curtime + GetFireRate();
					WeaponSound(SINGLE);
					StopWeaponSound(SPECIAL1);
					m_bFirstPlay = false;
				}
				Vector vecAiming = pPlayer->GetAutoaimVector(0);
				Vector vecSrc = pPlayer->EyePosition();

				Fire(vecSrc, vecAiming, pPlayer);

				/*data.m_nEntIndex = pPlayer->GetViewModel()->entindex();
				DispatchEffect("MuzzleLight", data);*/
					
				FireClientPrimaryAttack();
			}
			else
				DryFire();

				if (GetWpnData().allowLoopSound)
					StartLoopSound();
		}
		break;
		case FIRE_END:
		{
			StopWeaponSound(SPECIAL1);
			DestroyBeam();
			StopLoopSound();
		}
		break;
		default:
			break;
	}

	/*if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}*/
}

void CWeaponLaser::ApplyDamage(const Vector &vecDir, trace_t &tr, CBasePlayer *pPlayer)
{
	if (m_flDmgTime <= gpGlobals->curtime)
	{
		// wide mode does damage to the ent, and radius damage
		if (tr.m_pEnt && tr.m_pEnt->m_takedamage != DAMAGE_NO)
		{
			ClearMultiDamage();

			auto dmgType = tr.m_pEnt->IsCombatCharacter() ? DMG_BURN : DMG_BULLET;

			if (tr.m_pEnt && tr.m_pEnt->IsAlive() && (tr.m_pEnt->GetFlags() & FL_ONFIRE) == 0)
			{
				CTakeDamageInfo info(this, pPlayer, sk_plr_dmg_laser_wide.GetFloat(), dmgType);
				CalculateMeleeDamageForce(&info, vecDir, tr.endpos);
				tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);
				ApplyMultiDamage();
			}
			if (tr.m_pEnt && tr.m_pEnt->IsAlive() && (tr.m_pEnt->GetFlags() & FL_ONFIRE) != 0)
			{
				CTakeDamageInfo info(this, pPlayer, sk_plr_dmg_laser_wide.GetFloat(), DMG_BULLET);
				CalculateMeleeDamageForce(&info, vecDir, tr.endpos);
				tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);
				ApplyMultiDamage();
			}

			RadiusDamage(CTakeDamageInfo(this, this, sk_plr_dmg_laser_wide.GetFloat() / 4, dmgType), tr.endpos, 10.0f, CLASS_NONE, NULL);
		}

		//UTIL_Smoke(tr.endpos, random->RandomInt(5, 10), 10);//ƒым от луча
		//UTIL_DecalTrace(&tr, "RedGlowFade");
		UTIL_DecalTrace(&tr, "FadingScorch"); //√арь от луча
		//m_flDmgTime = gpGlobals->curtime + LASER_DAMAGE_TIME;

		ShouldDrawWaterImpacts(tr);

		m_flDmgTime = gpGlobals->curtime + GetFireRate();
	}

}

void CWeaponLaser::AmmoUsing()
{
	// Wide mode uses 10 charges per second in single player
	if (m_flAmmoTime <= gpGlobals->curtime)
	{
		UseAmmo(1);
		m_flAmmoTime = gpGlobals->curtime + LASER_AMMOUSING_TIME;
	}
}

void CWeaponLaser::Fire(const Vector &vecOrigSrc, const Vector &vecDir, CBasePlayer *pPlayer)
{
	Vector vecDest = vecOrigSrc + (vecDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine(vecOrigSrc, vecDest, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (tr.allsolid)
		return;

	/*CEffectData data;
	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;
	DispatchEffect("LaserSparksImpact", data);*/

	UTIL_ImpactTrace(&tr, GetDamageType(), "LaserSparksImpact");

	ApplyDamage(vecDir, tr, pPlayer); 

	AmmoUsing();

	AddViewKick();

	Vector vecUp, vecRight;
	QAngle angDir;

	VectorAngles(vecDir, angDir);
	AngleVectors(angDir, NULL, &vecRight, &vecUp);

	Vector tmpSrc = vecOrigSrc + (vecUp * -8) + (vecRight * 3);
	UpdateBeam(tmpSrc, tr.endpos, pPlayer);

	if (pPlayer)
	{
		CEffectData data;

		data.m_nEntIndex = pPlayer->entindex();

		data.m_vOrigin = tr.endpos;

		data.m_vStart.x = GetWpnData().iMuzzleFlashLightR;
		data.m_vStart.y = GetWpnData().iMuzzleFlashLightG;
		data.m_vStart.z = GetWpnData().iMuzzleFlashLightB;

		DispatchEffect("LaserWeaponLight", data);
	}
	/*CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &tr.endpos, 0, 213, 255, 0, 250, 0.09, 0);*/

    //DispatchParticleEffect("ball", tmpSrc, tr.endpos, vec3_angle, NULL);
	//UpdateEffect( tr.endpos );
	//UTIL_ImpactTrace( &tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss" ); // след на стене		
}

void CWeaponLaser::UpdateBeam(const Vector &startPoint, const Vector &endPoint, CBasePlayer *pPlayer)
{
	if (!m_hBeam)
	{
		CreateBeam(pPlayer);
	}
	else
	{
		m_hBeam->SetStartPos(endPoint);
	}

	if (m_hHaloBeam)
	{
		m_hHaloBeam->SetStartPos(endPoint);
	}

	if (m_SSprite)
	{
		m_SSprite->SetAbsOrigin(endPoint);
		m_SSprite->SetBrightness(random->RandomFloat(180.f, 255.f));
		m_SSprite->SetScale(random->RandomFloat(1.0f, 2.0f));
	}
}

void CWeaponLaser::CreateBeam(CBasePlayer *pPlayer)
{
	if (!m_hBeam)
	{
		int width = 2.0f;
		m_hBeam = CBeam::BeamCreate(LASER_BEAM_TEXTURE, width*0.5);
		m_hBeam->SetWidth(width);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hBeam->PointEntInit(this->GetAbsOrigin(), this);
		else
			m_hBeam->PointEntInit(pPlayer->GetViewModel()->GetAbsOrigin(), pPlayer->GetViewModel());

		m_hBeam->SetEndAttachment(1);
		m_hBeam->SetScrollRate(950);
		m_hBeam->AddSpawnFlags(SF_BEAM_TEMPORARY);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hBeam->SetOwnerEntity(this);
		else
			m_hBeam->SetOwnerEntity(pPlayer->GetViewModel());

		m_hBeam->SetBrightness(2255);
		m_hBeam->SetColor(0, 213, 255);
		//m_hBeam->SetHaloTexture(PrecacheModel(LASER_HALO_BEAM_TEXTURE));
	}
	else
	{
		Vector	vForward, vRight, vUp, vThrowPos;
		pPlayer->EyeVectors(&vForward, &vRight, &vUp);
		vThrowPos = pPlayer->EyePosition();

		/*if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		{
			vThrowPos += vForward * 22.0f;
			vThrowPos += vRight * 0.7f;// *1.0f;
			vThrowPos += vUp * -3.4f;
			CPASFilter filter(GetAbsOrigin());
			te->DynamicLight(filter, 0.0, &vThrowPos, 0, 213, 255, 0, 170, 0.12, 0);
		}
		else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
		{
			vThrowPos += vForward * 22.0f;
			vThrowPos += vRight * 5.4f;
			vThrowPos += vUp * -3.4f;
			CPASFilter filter(GetAbsOrigin());
			te->DynamicLight(filter, 0.0, &vThrowPos, 0, 213, 255, 0, 170, 0.12, 0);
		}*/
	}

	if (!m_hHaloBeam)
	{
		int width = 4.0f;
		m_hHaloBeam = CBeam::BeamCreate(LASER_HALO_BEAM_TEXTURE, width*0.5);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hHaloBeam->PointEntInit(this->GetAbsOrigin(), this);
		else
			m_hHaloBeam->PointEntInit(pPlayer->GetViewModel()->GetAbsOrigin(), pPlayer->GetViewModel());

		m_hHaloBeam->SetEndWidth(width);
		m_hHaloBeam->SetEndAttachment(1);
		m_hHaloBeam->SetScrollRate(950);
		m_hHaloBeam->AddSpawnFlags(SF_BEAM_TEMPORARY);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hHaloBeam->SetOwnerEntity(this);
		else
			m_hHaloBeam->SetOwnerEntity(pPlayer);

		m_hHaloBeam->SetBrightness(50);
		m_hHaloBeam->SetColor(0, 213, 255);
	}

	if (!m_SSprite)
	{
		m_SSprite = CSprite::SpriteCreate(ADDITIONAL_SPRITE, m_hBeam->GetAbsEndPos(), false);
		m_SSprite->SetParent(m_hBeam);
		m_SSprite->SetScale(1.0);
		m_SSprite->SetColor(0, 213, 255);
		m_SSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_SSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	}
}

void CWeaponLaser::DestroyBeam(void)
{
	m_BeamFireState = FIRE_NONE;

	if (m_bFirstPlay)
		m_bFirstPlay = false;

	m_flNextSecondaryAttack = m_flDmgTime = m_flChargeTime = m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

	StopLoopSound();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		//GetParticleSystemIndex("")
		StopParticleEffects(pPlayer->GetViewModel());

		if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
	}

	if (m_hBeam)
	{
		UTIL_Remove(m_hBeam);
		m_hBeam = NULL;
	}

	if (m_hHaloBeam)
	{
		UTIL_Remove(m_hHaloBeam);
		m_hHaloBeam = NULL;
	}

	if (m_SSprite)
	{
		m_SSprite->Expand(10, 500);
		m_SSprite = NULL;
	}
}

bool CWeaponLaser::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool bRet;
	pSwitchingTo = NULL;
	bRet = BaseClass::Holster(pSwitchingTo);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return false;

	StopWeaponSound(SINGLE);

	StopWeaponSound(SPECIAL1);

	m_BeamFireState = FIRE_NONE;
	DestroyBeam();
	
	if (bRet)
	{
		pSwitchingTo = NULL;
	}

	return bRet;
}

bool CWeaponLaser::WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions)
{
	CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

	if (!npcOwner)
	{
		return false;
	}

	m_vecLaserTarget = targetPos;
	return true;
}


int CWeaponLaser::WeaponRangeAttack1Condition(float flDot, float flDist)
{
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
	{
		return COND_NONE;
	}

	if (m_vecLaserTarget == LASER_TARGET_INVALID)
	{
		// No target!
		return COND_NONE;
	}

	if (flDist > m_fMaxRange1)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.5f)	// UNDONE: Why check this here? Isn't the AI checking this already?
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_RANGE_ATTACK1;
}

void CWeaponLaser::Charge(void)
{


}

void CWeaponLaser::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (!m_bReloadComplete &&
		!m_bInReload &&
		(IsNearWall() ||
		GetOwnerIsRunning()))
	{
		DestroyBeam();
		StopWeaponSound(SPECIAL1);
		return;
	}

	if (pPlayer->m_nButtons & IN_ZOOM)
	{
		if (m_BeamFireState != FIRE_END)
			m_BeamFireState = FIRE_END;
	}

	if (pPlayer->GetWaterLevel() == 3)
	{
		if (m_BeamFireState != FIRE_END)
			m_BeamFireState = FIRE_END;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && (m_hBeam || m_hHaloBeam || m_SSprite))
	{
		DestroyBeam();
		StopWeaponSound(SPECIAL1);
		return;
	}

	switch (m_BeamFireState)
	{
		case FIRE_STARTUP:
		{
			if (pPlayer->m_afButtonReleased & IN_ATTACK)
			{
				StopWeaponSound(SPECIAL1);
				StopLoopSound();
				m_BeamFireState = FIRE_END;
			}
		}
		break;
		case FIRE_LOOP:
		{
			if (pPlayer->m_afButtonReleased & IN_ATTACK)
			{
				StopWeaponSound(SPECIAL1);
				StopLoopSound();
				m_BeamFireState = FIRE_END;
			}
		}
		break;
		case FIRE_END:
		{
			StopWeaponSound(SPECIAL1);
			DestroyBeam();
		}
		break;
		default:
			break;
	}

	/*if (!(pPlayer->m_nButtons & IN_DUCK))
		pPlayer->SetMaxSpeed(hl2_normspeed.GetFloat()*0.5f);
	else if (pPlayer->m_nButtons & IN_DUCK)
		pPlayer->SetMaxSpeed(hl2_normspeed.GetFloat());
	else if (pPlayer->GetFlags() & FL_DUCKING)
		pPlayer->SetMaxSpeed(hl2_normspeed.GetFloat());

	if (pPlayer->m_nButtons & IN_SPEED)
		pPlayer->SetMaxSpeed(hl2_normspeed.GetFloat());*/
}

//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponLaser::ShouldDrawWaterImpacts(const trace_t &shot_trace)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (WaterContents(shot_trace.startpos))
		return false;

	// We must end inside of water
	if (!WaterContents(shot_trace.endpos))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(shot_trace.startpos, shot_trace.endpos, (CONTENTS_WATER | CONTENTS_SLIME), UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &waterTrace);


	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(2.0, 4.0f);	// Water effect scale

		// See if we hit slime
		if (FBitSet(waterTrace.contents, CONTENTS_SLIME))
		{
			FSetBit(data.m_fFlags, FX_WATER_IN_SLIME);
		}

		CPASFilter filter(data.m_vOrigin);
		te->DispatchEffect(filter, 0.0, data.m_vOrigin, "watersplash", data);
	}
	return true;
}