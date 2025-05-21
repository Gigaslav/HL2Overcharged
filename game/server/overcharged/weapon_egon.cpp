//========= Copyright © 1996-2018, Valve & Overcharged ============
//
// Purpose:		Egon (enhanced & modern)
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "player.h"
#include "shake.h"
#include "AmmoDef.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "weapon_egon.h"


//#define EGON_CHARGE_TIME			0.2f
#define EGON_AMMOUSING_TIME			0.1
#define EGON_DAMAGE_TIME			0.02
#define EGON_BEAM_TEXTURE			"effects/egon/xbeam3.vmt"
#define MAX_BURN_RADIUS				256
#define RADIUS_GROW_RATE			50.0
#define EGON_TARGET_INVALID			Vector( FLT_MAX, FLT_MAX, FLT_MAX )
#define ADDITIONAL_SPRITE			"effects/egon/HitSprite.vmt"

//extern ConVar sk_plr_dmg_egon_wide;
ConVar sk_plr_dmg_egon_wide("sk_plr_dmg_egon_wide", "0", (FCVAR_REPLICATED), "Egon damage.");
ConVar oc_weapon_egon_beams_life_time("oc_weapon_egon_beams_life_time", "2", (FCVAR_REPLICATED), "Egon damage.");
ConVar oc_weapon_egon_firewater_frequency("oc_weapon_egon_firewater_frequency", "0", (FCVAR_REPLICATED), "Egon damage.");

LINK_ENTITY_TO_CLASS(weapon_egon, CWeaponEgon);

PRECACHE_WEAPON_REGISTER(weapon_egon);

IMPLEMENT_SERVERCLASS_ST(CWeaponEgon, DT_WeaponEgon)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponEgon)
	DEFINE_FIELD(m_bFirstPlay, FIELD_BOOLEAN),
	DEFINE_FIELD(m_beamIndex, FIELD_INTEGER),
	DEFINE_FIELD(m_flBurnRadius, FIELD_FLOAT),
	DEFINE_FIELD(m_vecLaserTarget, FIELD_VECTOR),
	DEFINE_FIELD(m_BeamFireState, FIELD_INTEGER),
	DEFINE_FIELD(m_SSprite, FIELD_EHANDLE),
	DEFINE_FIELD(m_flChargeTime, FIELD_TIME),
	DEFINE_FIELD(m_flDmgTime, FIELD_TIME),
	DEFINE_FIELD(m_flAmmoTime, FIELD_TIME),
	DEFINE_FIELD(m_hBeam, FIELD_EHANDLE),
	DEFINE_FIELD(m_hCurveBeam, FIELD_EHANDLE),
END_DATADESC()


acttable_t CWeaponEgon::m_acttable[] =					// BriJee: Egon act's over here
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
};

IMPLEMENT_ACTTABLE(CWeaponEgon);
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponEgon::CWeaponEgon(void)
{
	m_BeamFireState = FIRE_NONE;
	m_bFirstPlay = true;
	m_flChargeTime = gpGlobals->curtime;
	m_flDmgTime = gpGlobals->curtime;
	m_flAmmoTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEgon::Precache(void)
{
	m_beamIndex = PrecacheModel(EGON_BEAM_TEXTURE);
	PrecacheMaterial(EGON_BEAM_TEXTURE);
	PrecacheMaterial(ADDITIONAL_SPRITE);
	BaseClass::Precache();
}

void CWeaponEgon::UseAmmo(int count)
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

Activity CWeaponEgon::GetPrimaryAttackActivity(void)
{
	if (m_BeamFireState == FIRE_STARTUP)
		return BaseClass::GetPrimaryAttackActivity();
	else if (m_BeamFireState == FIRE_LOOP)
		return GetWpnData().animData[m_bFireMode].FirePrimarySpecial;

	return BaseClass::GetPrimaryAttackActivity();
}

void CWeaponEgon::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

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
					m_flDmgTime = m_flAmmoTime = gpGlobals->curtime + GetFireRate();
					WeaponSound(SINGLE);
					StopWeaponSound(SPECIAL1);
					m_bFirstPlay = false;
					//DispatchParticleEffect("gluon_beam_startup", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);
					//DispatchParticleEffect("gluon_beam_burst", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);
					DispatchParticleEffect(STRING(GetWpnData().iMuzzleFlashShortBurst), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), GetWpnData().iAttachment, false);
				}
				Vector vecAiming = pPlayer->GetAutoaimVector(0);
				Vector vecSrc = pPlayer->EyePosition();

				Fire(vecSrc, vecAiming, pPlayer);

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

void CWeaponEgon::ApplyDamage(const Vector &vecDir, trace_t &tr, CBasePlayer *pPlayer)
{

	if (m_flDmgTime <= gpGlobals->curtime)
	{
		// wide mode does damage to the ent, and radius damage
		if (tr.m_pEnt && tr.m_pEnt->m_takedamage != DAMAGE_NO)
		{
			ClearMultiDamage();

			if (tr.m_pEnt && tr.m_pEnt->IsAlive() && (tr.m_pEnt->GetFlags() & FL_ONFIRE) == 0)
			{
				CTakeDamageInfo info(this, pPlayer, sk_plr_dmg_egon_wide.GetFloat(), DMG_ENERGYBEAM | /*DMG_REMOVENORAGDOLL | */ DMG_DISSOLVE_EGON_ELECTRICAL | DMG_DISSOLVE_EGON);
				CalculateMeleeDamageForce(&info, vecDir, tr.endpos);
				tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);
				ApplyMultiDamage();
			}
			if (tr.m_pEnt && tr.m_pEnt->IsAlive() && (tr.m_pEnt->GetFlags() & FL_ONFIRE) != 0)
			{
				CTakeDamageInfo info(this, pPlayer, sk_plr_dmg_egon_wide.GetFloat(), DMG_ENERGYBEAM |/* DMG_REMOVENORAGDOLL | */ DMG_DISSOLVE_EGON_ELECTRICAL | DMG_DISSOLVE_EGON);
				CalculateMeleeDamageForce(&info, vecDir, tr.endpos);
				tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);
				ApplyMultiDamage();
			}

		}

		RadiusDamage(CTakeDamageInfo(this, this, sk_plr_dmg_egon_wide.GetFloat() / 4, DMG_BURN), tr.endpos, 10.0f, CLASS_NONE, NULL);

		UTIL_DecalTrace(&tr, "FadingScorch"); //√арь от луча

		ShouldDrawWaterImpacts(tr);

		m_flDmgTime = gpGlobals->curtime + GetFireRate();
	}
}

void CWeaponEgon::AmmoUsing()
{
	// Wide mode uses 10 charges per second in single player
	if (m_flAmmoTime <= gpGlobals->curtime)
	{
		UseAmmo(1);
		m_flAmmoTime = gpGlobals->curtime + EGON_AMMOUSING_TIME;
	}
}

void CWeaponEgon::Fire(const Vector &vecOrigSrc, const Vector &vecDir, CBasePlayer *pPlayer)
{
	Vector vecDest = vecOrigSrc + (vecDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine(vecOrigSrc, vecDest, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);


	ApplyDamage(vecDir, tr, pPlayer);

	AmmoUsing();
	AddViewKick();


	Vector vecUp, vecRight;
	QAngle angDir;

	VectorAngles(vecDir, angDir);
	AngleVectors(angDir, NULL, &vecRight, &vecUp);

	Vector tmpSrc = vecOrigSrc + (vecUp * -8) + (vecRight * 3);
	UpdateBeam(tmpSrc, tr.endpos, pPlayer);

	if (pPlayer->GetViewModel())
	{
		CEffectData	data;
		data.m_nEntIndex = pPlayer->GetViewModel()->entindex();
		data.m_nAttachmentIndex = 1;
		DispatchEffect("EgonLightning", data);
	}

	//DispatchParticleEffect("gluon_beam_startup", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged
	//DispatchParticleEffect("gluon_beam_burst", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged

	/*CPASFilter filter(endPoint);
	te->DynamicLight(filter, 0.0, &endPoint, 80, 22, 255, 2, 40, 0.07, 0);*/

	if (pPlayer)
	{
		CEffectData data;

		data.m_nEntIndex = pPlayer->entindex();

		data.m_vOrigin = tr.endpos;

		data.m_vStart.x = GetWpnData().iMuzzleFlashLightR;
		data.m_vStart.y = GetWpnData().iMuzzleFlashLightG;
		data.m_vStart.z = GetWpnData().iMuzzleFlashLightB;

		DispatchEffect("EgonWeaponLight", data);
	}

	if (tr.DidHit())
	{
		//int beams;
		/*CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &tr.endpos, 80, 22, 255, 2, 40, 0.07, 0);*/

		/*for (beams = 0; beams < 6; beams++)
		{
			//		Vector vecDest;

			// Random unit vector
			vecDest.x = random->RandomFloat(-1, 1);
			vecDest.y = random->RandomFloat(-1, 1);
			vecDest.z = random->RandomFloat(-1, 1);

			// Push out to radius dist.
			vecDest = tr.endpos + vecDest * 35.0f;

			DispatchParticleEffect("gluon_beam_end_inst", tr.endpos, vec3_angle);

			UTIL_Beam(tr.endpos,
				vecDest,
				m_beamIndex,
				0,		//halo index
				0,		//frame start
				2.0f,	//framerate
				0.08f,	//life
				1.3,		// width
				0.75,	// endwidth
				0.75,	// fadelength,
				2,		// noise

				138,		// red
				112,	// green
				234,		// blue,

				155, // bright
				150  // speed
				);
		}*/
	}
	if (tr.m_pEnt != NULL && !tr.m_pEnt->IsWorld())
	{
		CSingleUserRecipientFilter filter(pPlayer);

		const char *shootsound = GetShootSound(SPECIAL2);
		if (shootsound && shootsound[0] &&
			random->RandomInt(0, 3) == 3)
		{
			EmitSound(filter, pPlayer->entindex(), shootsound, &tr.endpos);
		}
	}

}

void CWeaponEgon::UpdateBeam(const Vector &startPoint, const Vector &endPoint, CBasePlayer *pPlayer)
{
	if (!m_hBeam && !m_hCurveBeam)
	{
		CreateBeam(pPlayer);
	}

	if (m_hBeam)
	{
		m_hBeam->SetStartPos(endPoint);
	}
	if (m_hCurveBeam)
	{
		m_hCurveBeam->SetStartPos(endPoint);
	}
	if (m_SSprite)
	{
		m_SSprite->SetAbsOrigin(endPoint);
		m_SSprite->SetBrightness(random->RandomFloat(120.f, 255.f));
		m_SSprite->SetScale(random->RandomFloat(0.35f, 0.55f));
	}
}


void CWeaponEgon::CreateBeam(CBasePlayer *pPlayer)
{
	if (!m_hBeam)
	{
		m_hBeam = CBeam::BeamCreate(EGON_BEAM_TEXTURE, 7.0);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hBeam->PointEntInit(this->GetAbsOrigin(), this);
		else
			m_hBeam->PointEntInit(pPlayer->GetViewModel()->GetAbsOrigin(), pPlayer->GetViewModel());

		m_hBeam->SetEndAttachment(1);
		m_hBeam->SetScrollRate(950);
		m_hBeam->SetBrightness(200);
		m_hBeam->SetNoise(0.4);
		m_hBeam->AddSpawnFlags(SF_BEAM_TEMPORARY);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hBeam->SetOwnerEntity(this);
		else
			m_hBeam->SetOwnerEntity(pPlayer->GetViewModel());

		m_hBeam->SetBrightness(2255);
		m_hBeam->SetColor(138, 112, 234);
	}
	if (!m_hCurveBeam)
	{
		float width = 3.2f;
		m_hCurveBeam = CBeam::BeamCreate(EGON_BEAM_TEXTURE, width);
		m_hCurveBeam->SetBeamFlags(FBEAM_SINENOISE);
		m_hCurveBeam->SetWidth(width*1.5);
		m_hCurveBeam->SetEndWidth(width / 3.0f);
		m_hCurveBeam->SetScrollRate(2000);
		m_hCurveBeam->SetNoise(RandomFloat(0.1f, 0.12f));

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hCurveBeam->PointEntInit(this->GetAbsOrigin(), this);
		else
			m_hCurveBeam->PointEntInit(pPlayer->GetViewModel()->GetAbsOrigin(), pPlayer->GetViewModel());

		m_hCurveBeam->SetEndAttachment(1);
		m_hCurveBeam->AddSpawnFlags(SF_BEAM_TEMPORARY);

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			m_hCurveBeam->SetOwnerEntity(this);
		else
			m_hCurveBeam->SetOwnerEntity(pPlayer->GetViewModel());

		m_hCurveBeam->SetBrightness(2255);
		m_hCurveBeam->SetColor(30, 30, 255);
	}
	if (!m_SSprite)
	{
		m_SSprite = CSprite::SpriteCreate(ADDITIONAL_SPRITE, m_hBeam->GetAbsEndPos(), false);
		m_SSprite->SetParent(m_hBeam);
		//m_SSprite->SetScale(0.25);
		m_SSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_SSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	}
}


void CWeaponEgon::DestroyBeam(void)
{
	m_BeamFireState = FIRE_NONE;

	if (m_bFirstPlay)
		m_bFirstPlay = false;

	m_flNextSecondaryAttack = m_flDmgTime = m_flChargeTime = m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

	StopLoopSound();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		StopParticleEffects(pPlayer->GetViewModel());

		if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
	}

	if (m_hBeam)
	{
		UTIL_Remove(m_hBeam);
		m_hBeam = NULL;
	}
	if (m_hCurveBeam)
	{
		UTIL_Remove(m_hCurveBeam);
		m_hCurveBeam = NULL;
	}
	if (m_SSprite)
	{
		m_SSprite->Expand(10, 500);
		m_SSprite = NULL;
	}
}

bool CWeaponEgon::Holster(CBaseCombatWeapon *pSwitchingTo)
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

bool CWeaponEgon::WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions)
{
	CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

	if (!npcOwner)
	{
		return false;
	}

	m_vecLaserTarget = targetPos;
	return true;
}


int CWeaponEgon::WeaponRangeAttack1Condition(float flDot, float flDist)
{
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
	{
		return COND_NONE;
	}

	if (m_vecLaserTarget == EGON_TARGET_INVALID)
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

void CWeaponEgon::Charge(void)
{

}

void CWeaponEgon::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (!m_bReloadComplete &&
		!m_bInReload &&
		(IsNearWall() || GetOwnerIsRunning()))
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

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && (m_hBeam || m_hCurveBeam || m_SSprite))
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
}


//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponEgon::ShouldDrawWaterImpacts(const trace_t &shot_trace)
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

