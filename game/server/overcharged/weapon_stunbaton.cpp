//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Stun Stick- beating stick with a zappy end
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "npc_metropolice.h"
#include "weapon_stunbaton.h"
#include "IEffects.h"

#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "in_buttons.h" 
#include "gamestats.h"
#include "rumble_shared.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_stunbaton("sk_plr_dmg_stunbaton", "0");
ConVar    sk_npc_dmg_stunbaton("sk_npc_dmg_stunbaton", "0");

extern ConVar metropolice_move_and_melee;

//-----------------------------------------------------------------------------
// CWeaponStunStick
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponStunBaton, DT_WeaponStunBaton)
SendPropInt(SENDINFO(m_Active), 1, SPROP_UNSIGNED),
END_SEND_TABLE()

//#ifndef HL2MP
LINK_ENTITY_TO_CLASS(weapon_stunbaton, CWeaponStunBaton);
PRECACHE_WEAPON_REGISTER(weapon_stunbaton);
//#endif

acttable_t CWeaponStunBaton::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },

	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },

	{ ACT_IDLE_RELAXED, ACT_IDLE, false },
	{ ACT_RUN_RELAXED, ACT_RUN, false },
	{ ACT_WALK_RELAXED, ACT_WALK, false },
};

IMPLEMENT_ACTTABLE(CWeaponStunBaton);


BEGIN_DATADESC(CWeaponStunBaton)

DEFINE_FIELD(m_Active, FIELD_BOOLEAN),
DEFINE_FIELD(doOnce, FIELD_BOOLEAN),
DEFINE_FIELD(fire, FIELD_BOOLEAN),

END_DATADESC()



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponStunBaton::CWeaponStunBaton(void)
{
	// HACK:  Don't call SetStunState because this tried to Emit a sound before
	//  any players are connected which is a bug
	Fire = false;

	Time = 0;
	m_Active = false;
	doOnce = true;
	fire = false;
}
CWeaponStunBaton::~CWeaponStunBaton(void)
{
	StopParticleEffects(this);
	// HACK:  Don't call SetStunState because this tried to Emit a sound before
	//  any players are connected which is a bug
	m_Active = false;
	doOnce = false;
	fire = false;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponStunBaton::Spawn()
{
	Precache();

	BaseClass::Spawn();
	AddSolidFlags(FSOLID_NOT_STANDABLE);
}

void CWeaponStunBaton::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("Weapon_StunStick.Activate");
	PrecacheScriptSound("Weapon_StunStick.Deactivate");
	PrecacheScriptSound("Weapon_Gauss.Special4");
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponStunBaton::GetDamageForActivity(Activity hitActivity)
{
	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
	{
		if (GetOwner()->GetWaterLevel() == 3)
			return 0.f;
		else
			return sk_plr_dmg_stunbaton.GetFloat();
	}

	return sk_npc_dmg_stunbaton.GetFloat();
}

//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
extern ConVar sk_crowbar_lead_time;

int CWeaponStunBaton::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	AngularImpulse angVelocity;
	pEnemy->GetVelocity(&vecVelocity, &angVelocity);

	// Project where the enemy will be in a little while, add some randomness so he doesn't always hit
	float dt = sk_crowbar_lead_time.GetFloat();
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());

	if (pEnemy->IsPlayer())
	{
		//Vector vecDir = pEnemy->GetSmoothedVelocity();
		//float flSpeed = VectorNormalize( vecDir );

		// If player will be in front of me in one-half second, clock his arse.
		Vector vecProjectEnemy = pEnemy->GetAbsOrigin() + (pEnemy->GetAbsVelocity() * 0.35);
		Vector vecProjectMe = GetAbsOrigin();

		if ((vecProjectMe - vecProjectEnemy).Length2D() <= 48.0f)
		{
			return COND_CAN_MELEE_ATTACK1;
		}
	}

	float flTargetDist = 48.0f;
	if ((flDist > flTargetDist) && (flExtrapolatedDist > flTargetDist))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

void CWeaponStunBaton::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());


	if (pOwner != NULL)
	{
		trace_t tr, tr2;
		Vector vecDir, vecStop, vecStop2, vecSrc, vForward, vRight, vUp;

		pOwner->EyeVectors(&vForward, &vRight, &vUp);
		QAngle Angle = pOwner->EyeAngles();
		Vector res = pOwner->EyePosition();
		res += vForward;
		res += vRight;
		res += vUp;
		vecSrc = res;
		AngleVectors(Angle, &vecDir);

		vecStop2 = vecSrc + vecDir * GetMeleeRange();
		UTIL_TraceLine(vecSrc, vecStop2, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr2);

		if (pOwner->GetWaterLevel() == 3 && (GetActivity() == GetWpnData().animData[m_bFireMode].MeleeMiss1 || GetActivity() == GetWpnData().animData[m_bFireMode].MeleeAttack1))
		{
			CEffectData	data;
			data.m_nEntIndex = pOwner->GetViewModel()->entindex();
			data.m_nAttachmentIndex = 1;
			DispatchEffect("UnderWaterBubbles", data);
		}

		//CheckAdmireAnimations(pOwner);

		if (m_Active == false && pOwner->GetWaterLevel() != 3 && !doOnce)
		{
			m_Active = true;
			doOnce = true;
		}

		if (m_Active == true)
		{

			CEffectData	data;

			data.m_nEntIndex = pOwner->GetViewModel()->entindex();
			data.m_nAttachmentIndex = 1;

			if (pOwner->GetWaterLevel() == 3)
			{
				pOwner->TakeDamage(CTakeDamageInfo(this, this, 15, DMG_GAUSS | DMG_CRUSH));
				DispatchEffect("StunsSparks", data);
				m_Active = false;
			}

			DispatchEffect("StunsEffect", data);

			if (!m_bInReload && (IsNearWall() || GetOwnerIsRunning()) && pOwner)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;

			}
			if ((pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
			{

				WeaponSound(MELEE_MISS);

				//Do view kick
				AddViewKick();

				if (tr2.DidHit())
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeAttack1);
				}
				else
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeMiss1);
				}

				m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
				m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

			}
			else if ((pOwner->m_afButtonReleased & IN_ATTACK) && GetActivity() != GetWpnData().animData[m_bFireMode].MeleeIdle && GetActivity() != GetWpnData().animData[m_bFireMode].MeleeMiss1 && GetActivity() != GetWpnData().animData[m_bFireMode].MeleeAttack1)
			{
				SendWeaponAnim(GetWpnData().animData[m_bFireMode].MeleeIdle);
			}

			if (GetWpnData().MeleeStunbattonParticleEffects)
			{
				//Temp disabled

				/*DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line01", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line02", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line03", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line04", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line05", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line06", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line07", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line08", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line09", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line10", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line11", false);
				DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line12", false);*/
			}
		}
		else
		{
			//Temp disabled

			//DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line01", true);
			StopParticleEffects(this);
			StopParticleEffects(pOwner);
		}

	}

	if (doOnce)
	{
		CEffectData	data;

		data.m_nEntIndex = pOwner->GetViewModel()->entindex();
		data.m_nAttachmentIndex = 1;

		DispatchEffect("StunsSparks", data);
		EmitSound("Weapon_StunStick.Activate");
		doOnce = false;
	}

	BaseClass::ItemPostFrame();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponStunBaton::ImpactEffect(trace_t &traceHit)
{
	//FIXME: need new decals
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	if (pPlayer->GetWaterLevel() == 3)
		return;
	WeaponSound(MELEE_HIT);
	g_pEffects->Sparks(traceHit.endpos, 1, 1, &traceHit.plane.normal);
	//UTIL_ImpactTrace(&traceHit, DMG_SHOCK);
	UTIL_DecalTrace(&traceHit, "FadingScorch");
	//fire = false;
}

void CWeaponStunBaton::Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	CEffectData	data;

	data.m_nEntIndex = pPlayer->GetViewModel()->entindex();
	data.m_nAttachmentIndex = 1;
	//data.m_vOrigin = traceHit.endpos;
	if (pPlayer->GetWaterLevel() != 3)
		DispatchEffect("StunsExplode", data);

	//Apply damage to a hit target
	if (pHitEntity != NULL && pPlayer->GetWaterLevel() != 3)
	{
		//DispatchParticleEffect("weapon_stunstick_el_sparks", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "spark_middle", false);
		CPASAttenuationFilter filter(this);

		EmitSound_t ep;
		ep.m_nChannel = CHAN_WEAPON;
		ep.m_pSoundName = "Weapon_Gauss.Special4";
		ep.m_flVolume = 3;
		ep.m_SoundLevel = SNDLVL_20dB;
		ep.m_pOrigin = &traceHit.endpos;
		EmitSound(filter, entindex(), ep);

		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_GAUSS);

		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

		pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);

		if (ToBaseCombatCharacter(pHitEntity))
		{
			gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
		}
	}


	// Apply an impact effect
	if (traceHit.fraction != 1.0f)
		ImpactEffect(traceHit);
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponStunBaton::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CEffectData	data;

	data.m_nEntIndex = pOwner->GetViewModel()->entindex();
	data.m_nAttachmentIndex = 1;
	//data.m_vOrigin = traceHit.endpos;
	if (pOwner->GetWaterLevel() != 3)
		DispatchEffect("StunsExplode", data);

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = GetWpnData().animData[m_bFireMode].MeleeAttack1;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * GetHull();  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, GetMeleeHullMin(), GetMeleeHullMax(), MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				//nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	// Apply an impact effect
	if (traceHit.fraction != 1.0f)
		ImpactEffect(traceHit);

	bool bIsSecondary = false;

	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? GetWpnData().animData[m_bFireMode].MeleeMiss2 : GetWpnData().animData[m_bFireMode].MeleeMiss1;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		//Make sound for the AI
		CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

		// This isn't great, but it's something for when the crowbar hits.
		pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

		CBaseEntity	*pHitEntity = traceHit.m_pEnt;

		//Apply damage to a hit target
		if (pHitEntity != NULL && pPlayer->GetWaterLevel() != 3)
		{
			//DispatchParticleEffect("weapon_stunstick_el_sparks", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "spark_middle", false);
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_WEAPON;
			ep.m_pSoundName = "Weapon_Gauss.Special4";
			ep.m_flVolume = 3;
			ep.m_SoundLevel = SNDLVL_20dB;
			ep.m_pOrigin = &traceHit.endpos;
			EmitSound(filter, entindex(), ep);

			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);

			if (pPlayer && pHitEntity->IsNPC())
			{
				// If bonking an NPC, adjust damage.
				info.AdjustPlayerDamageInflictedForSkillLevel();
			}

			CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

			pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
			ApplyMultiDamage();

			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);

			if (ToBaseCombatCharacter(pHitEntity))
			{
				gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
			}
		}

		ImpactEffect(traceHit);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

}

void CWeaponStunBaton::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
	{
		//DevMsg("Hit \n");
		if (pOperator && pOperator->IsPlayer() && pOperator->GetWaterLevel() != 3)
		{
			Time = 0;
			CBasePlayer *pOwner = ToBasePlayer(GetOwner());

			if (pOwner && !IsNearWall() && !GetOwnerIsRunning())
			{
				/*trace_t tr;
				Vector vecDir, vecStop, vecSrc, vForward, vRight, vUp;

				pOwner->EyeVectors(&vForward, &vRight, &vUp);
				QAngle Angle = pOwner->EyeAngles();
				Vector res = pOwner->EyePosition();
				res += vForward;
				res += vRight;
				res += vUp;
				vecSrc = res;
				AngleVectors(Angle, &vecDir);
				vecStop = vecSrc + vecDir * GetMeleeRange();
				//UTIL_TraceLine(vecSrc, vecStop, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
				UTIL_TraceHull(vecSrc, vecStop, GetMeleeHullMin(), GetMeleeHullMax(), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

				Hit(tr, pOwner->GetActivity(), false);*/

				HandleAnimEventMeleeHit(pEvent, pOperator);
			}
		}
		else if (pOperator && pOperator->IsNPC())
		{
			// Trace up or down based on where the enemy is...
			// But only if we're basically facing that direction
			Vector vecDirection;
			AngleVectors(GetAbsAngles(), &vecDirection);

			CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
			if (pEnemy)
			{
				Vector vecDelta;
				VectorSubtract(pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta);
				VectorNormalize(vecDelta);

				Vector2D vecDelta2D = vecDelta.AsVector2D();
				Vector2DNormalize(vecDelta2D);
				if (DotProduct2D(vecDelta2D, vecDirection.AsVector2D()) > 0.8f)
				{
					vecDirection = vecDelta;
				}
			}

			Vector vecEnd;
			VectorMA(pOperator->Weapon_ShootPosition(), 32, vecDirection, vecEnd);
			// Stretch the swing box down to catch low level physics objects
			CBaseEntity *pHurt = pOperator->CheckTraceHullAttack(pOperator->Weapon_ShootPosition(), vecEnd,
				Vector(-16, -16, -40), Vector(16, 16, 16), GetDamageForActivity(GetActivity()), DMG_SHOCK, 0.5f, false);

			// did I hit someone?
			if (pHurt)
			{
				// play sound
				WeaponSound(MELEE_HIT);

				CBasePlayer *pPlayer = ToBasePlayer(pHurt);

				CNPC_MetroPolice *pCop = dynamic_cast<CNPC_MetroPolice *>(pOperator);
				bool bFlashed = false;

				if (pCop != NULL && pPlayer != NULL)
				{
					// See if we need to knock out this target
					if (pCop->ShouldKnockOutTarget(pHurt))
					{
						/*float yawKick = random->RandomFloat(-48, -24);

						//Kick the player angles
						pPlayer->ViewPunch(QAngle(-16, yawKick, 2));*/

						color32 white = { 255, 255, 255, 255 };
						UTIL_ScreenFade(pPlayer, white, 0.2f, 1.0f, FFADE_OUT | FFADE_PURGE | FFADE_STAYOUT);
						bFlashed = true;

						pCop->KnockOutTarget(pHurt);

						break;
					}
					else
					{
						// Notify that we've stunned a target
						pCop->StunnedTarget(pHurt);
					}
				}

				// Punch angles
				if (pPlayer != NULL && !(pPlayer->GetFlags() & FL_GODMODE))
				{
					/*float yawKick = random->RandomFloat(-48, -24);

					//Kick the player angles
					pPlayer->ViewPunch(QAngle(-16, yawKick, 2));*/

					Vector	dir = pHurt->GetAbsOrigin() - GetAbsOrigin();

					// If the player's on my head, don't knock him up
					if (pPlayer->GetGroundEntity() == pOperator)
					{
						dir = vecDirection;
						dir.z = 0;
					}

					VectorNormalize(dir);

					dir *= 500.0f;

					//If not on ground, then don't make them fly!
					if (!(pPlayer->GetFlags() & FL_ONGROUND))
						dir.z = 0.0f;

					//Push the target back
					pHurt->ApplyAbsVelocityImpulse(dir);

					if (!bFlashed)
					{
						color32 red = { 128, 0, 0, 128 };
						UTIL_ScreenFade(pPlayer, red, 0.5f, 0.1f, FFADE_IN);
					}

					// Force the player to drop anyting they were holding
					pPlayer->ForceDropOfCarriedPhysObjects();
				}

				// do effect?
			}
			else
			{
				WeaponSound(MELEE_MISS);
			}
		}
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the state of the stun stick
//-----------------------------------------------------------------------------
void CWeaponStunBaton::SetStunState(bool state)
{
	m_Active = state;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (m_Active)
	{
		//FIXME: START - Move to client-side



		if (!pOwner)
		{
			Vector vecAttachment;

			GetAttachment(1, vecAttachment);
			g_pEffects->Sparks(vecAttachment);
			EmitSound("Weapon_StunStick.Activate");
		}

		//FIXME: END - Move to client-side


	}
	else
	{
		if (pOwner == NULL)
			EmitSound("Weapon_StunStick.Deactivate");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponStunBaton::Deploy(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner)
	{
		Fire = false;
		Time = 0;
		SetStunState(true);
		doOnce = true;
		fire = false;
	}
	else
	{
		SetStunState(true);
	}
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponStunBaton::Holster(CBaseCombatWeapon *pSwitchingTo)
{

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner)
	{
		fire = false;
		doOnce = false;

		//Temp disabled

		//DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line01", true);

		EmitSound("Weapon_StunStick.Deactivate");
		StopParticleEffects(pOwner);
	}

	if (BaseClass::Holster(pSwitchingTo) == false)
		return false;

	m_Active = false;
	SetStunState(false);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecVelocity - 
//-----------------------------------------------------------------------------
void CWeaponStunBaton::Drop(const Vector &vecVelocity)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		fire = false;
		doOnce = false;

		//Temp disabled

		//DispatchParticleEffect("weapon_stunstick_rays", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "line01", true);

		StopParticleEffects(pOwner);
	}
	SetStunState(false);
	m_Active = false;
	doOnce = false;
	BaseClass::Drop(vecVelocity);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponStunBaton::GetStunState(void)
{
	return m_Active;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CWeaponStunBaton::ImpactWater(const Vector &start, const Vector &end)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (UTIL_PointContents(start) & (CONTENTS_WATER | CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if (!(UTIL_PointContents(end) & (CONTENTS_WATER | CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(start, end, (CONTENTS_WATER | CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace);

	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect("watersplash", data);
	}

	return true;
}