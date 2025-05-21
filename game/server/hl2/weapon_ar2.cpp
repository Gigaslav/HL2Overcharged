//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "weapon_ar2.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "AI_Memory.h"
#include "soundent.h"
//#include "hl2mp_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "IEffects.h"
#include "trace.h"
#include "SpriteTrail.h"
#include "steamjet.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar sk_plr_dmg_ar2;

ConVar sk_weapon_ar2_alt_fire_radius("sk_weapon_ar2_alt_fire_radius", "10");
ConVar sk_weapon_ar2_alt_fire_duration("sk_weapon_ar2_alt_fire_duration", "2");
ConVar sk_weapon_ar2_alt_fire_mass("sk_weapon_ar2_alt_fire_mass", "150");
//ConVar  cl_tracer_type("cl_tracer_type", "2", FCVAR_CHEAT);

//=========================================================
//=========================================================

BEGIN_DATADESC(CWeaponAR2)

DEFINE_FIELD(m_flDelayedFire, FIELD_TIME),
DEFINE_FIELD(m_bShotDelayed, FIELD_BOOLEAN),

DEFINE_FIELD(m_iFirstRound, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondRound, FIELD_INTEGER),
DEFINE_FIELD(m_iThirdRound, FIELD_INTEGER),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponAR2, DT_WeaponAR2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_ar2, CWeaponAR2);
PRECACHE_WEAPON_REGISTER(weapon_ar2);

acttable_t	CWeaponAR2::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE, ACT_IDLE_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },		// FIXME: hook to AR2 unique

	{ ACT_WALK, ACT_WALK_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_AR2_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_RANGE_ATTACK_AR2, false },

	/*{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },

	//{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },//false
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },	//END

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },*/
};

IMPLEMENT_ACTTABLE(CWeaponAR2);

CWeaponAR2::CWeaponAR2()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;

	m_iFirstRound = 0;
	m_iSecondRound = 0;
	m_iThirdRound = 0;
}

void CWeaponAR2::Precache(void)
{
	BaseClass::Precache();

	PrecacheParticleSystem("weapon_muzzle_smoke");
	PrecacheScriptSound("Weapon_AR2.Idle");
	PrecacheScriptSound("FX_RicochetSound.Ricochet");
	PrecacheScriptSound("Bullets.DefaultNearmiss");
	UTIL_PrecacheOther("prop_combine_ball");
	UTIL_PrecacheOther("env_entity_dissolver");
	PrecacheScriptSound("Weapon_AR2.End");
	PrecacheParticleSystem("weapon_muzzle_flash_smoke_small2");
	PrecacheParticleSystem("IBullet_exhaust");
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_AR2.Draw");
	PrecacheScriptSound("Weapon_AR2.LoopFire");

	PrecacheScriptSound("Weapon_Ar2.Loop");
	PrecacheScriptSound("Weapon_Ar2.Stop");

}

void CWeaponAR2::SetRoundsVisibility()
{
	if (GetWpnData().animData[m_bFireMode].UseHalfFullMagAnimations)
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer)
		{
			CBaseViewModel *pVM = pPlayer->GetViewModel();
			if (pVM)
			{
				//int ammo = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);

				//bool hasAmmo = UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo();

				switch (Clip1())
				{
				case 0:
				{
					m_iFirstRound = 1;
					m_iSecondRound = 1;
					m_iThirdRound = 1;
				}
				break;
				case 1:
				{
					m_iFirstRound = 1;
					m_iSecondRound = 1;
					m_iThirdRound = 0;
				}
				break;
				case 2:
				{
					m_iFirstRound = 1;
					m_iSecondRound = 0;
					m_iThirdRound = 0;
				}
				break;
				default:
				{
					m_iFirstRound = 0;
					m_iSecondRound = 0;
					m_iThirdRound = 0;
				}
				break;
				}

				pVM->SetBodygroup(2, m_iFirstRound);
				pVM->SetBodygroup(3, m_iSecondRound);
				pVM->SetBodygroup(4, m_iThirdRound);
			}
		}
	}
}

void CWeaponAR2::SetRoundVisibility(animevent_t *pEvent, int iRound)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (GetWpnData().animData[m_bFireMode].UseHalfFullMagAnimations)
	{
		if (pPlayer)
		{
			CBaseViewModel *pVM = pPlayer->GetViewModel();

			switch (iRound)
			{
				case 2:
				{
					m_iFirstRound = atoi(pEvent->options);

					if (pVM)
						pVM->SetBodygroup(iRound, m_iFirstRound);
				}
				break;
				case 3:
				{
					m_iSecondRound = atoi(pEvent->options);

					if (pVM)
						pVM->SetBodygroup(iRound, m_iSecondRound);
				}
				break;
				case 4:
				{
					m_iThirdRound = atoi(pEvent->options);

					if (pVM)
						pVM->SetBodygroup(iRound, m_iThirdRound);
				}
				break;
				default:
				{
					int iVal = atoi(pEvent->options);

					if (pVM)
						pVM->SetBodygroup(iRound, iVal);
				}
				break;
			}
		}
	}
}

Activity CWeaponAR2::GetDrawActivity()
{
	SetRoundsVisibility();

	return BaseClass::GetDrawActivity();
}

Activity CWeaponAR2::GetHolsterActivity()
{
	SetRoundsVisibility();

	return BaseClass::GetHolsterActivity();
}

void CWeaponAR2::PrimaryAttack()
{
	BaseClass::PrimaryAttack();

	SetRoundsVisibility();
}


void CWeaponAR2::ItemPostFrame(void)
{
	// See if we need to fire off our secondary round
	if (m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire)
	{
		DelayedAttack();
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}


	/*if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].UseHalfFullMagAnimations)
	{
		CBaseViewModel *pVM = pPlayer->GetViewModel();

		if (pVM)
		{
			if (!HasPrimaryAmmo() && !m_bInReload)
			{
				pVM->SetBodygroup(2, 1);
				pVM->SetBodygroup(3, 1);
				pVM->SetBodygroup(4, 1);
			}
			else if (m_iClip1 == 1 && HasPrimaryAmmo() && !m_bInReload)
			{
				pVM->SetBodygroup(2, 0);
				pVM->SetBodygroup(3, 1);
				pVM->SetBodygroup(4, 1);
			}
			else if (m_iClip1 > 1 && HasPrimaryAmmo() && !m_bInReload)
			{
				pVM->SetBodygroup(2, 0);
				pVM->SetBodygroup(3, 0);
				pVM->SetBodygroup(4, 0);
			}
		}
	}*/

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::DelayedAttack(void)
{
	if (IsNearWall() || GetOwnerIsRunning())
	{
		m_bShotDelayed = false;
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	m_bShotDelayed = false;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Deplete the clip completely
	SendWeaponAnim(GetSecondaryAttackActivity());
	m_flNextSecondaryAttack = pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

	WeaponSound(WPN_DOUBLE);

	pOwner->RumbleEffect(RUMBLE_SHOTGUN_DOUBLE, 0, RUMBLE_FLAG_RESTART);

	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;

	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	vThrowPos = pPlayer->EyePosition();


	pPlayer->GetVelocity(&vThrowVel, NULL);
	vThrowVel += vForward * 1000;


	//BriJee: Overcharged Server-side ironsight state?
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		vThrowPos += vForward * 22.0f;
		vThrowPos += vRight * 0.7f;// *1.0f;
		vThrowPos += vUp * -3.4f;
		//DevMsg("ISight enabled \n");
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		vThrowPos += vForward * 22.0f;
		vThrowPos += vRight * 5.4f;
		vThrowPos += vUp * -3.4f;
		//DevMsg("ISight disabled \n");
	}

	// Fire the combine ball
	CreateCombineBall(vThrowPos,
		vThrowVel,
		sk_weapon_ar2_alt_fire_radius.GetFloat(),
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		sk_weapon_ar2_alt_fire_duration.GetFloat(),
		pPlayer);

	m_bIsFiring = false;
	// View effects
	color32 white = { 255, 255, 255, 64 };
	UTIL_ScreenFade(pOwner, white, 0.1, 0, FFADE_IN);

	//Disorient the player
	QAngle angles = pOwner->GetLocalAngles();

	angles.x += random->RandomInt(-4, 4);
	angles.y += random->RandomInt(-4, 4);
	angles.z = 0;

	pOwner->SnapEyeAngles(angles);

	pOwner->ViewPunch(QAngle(random->RandomInt(-8, -12), random->RandomInt(1, 2), 0));

	// Decrease ammo
	RemoveAmmo(GetSecondaryAmmoType(), 1); //pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	SecondaryAttackEffects();

	/*if (pPlayer->GetActiveWeapon() && pPlayer->GetActiveWeapon()->GetWpnData().iMuzzleFlashTypeS != NULL)
	{
		if (pPlayer->GetActiveWeapon()->GetWpnData().iMuzzleFlashDelayed != AllocPooledString("") &&
			pPlayer->GetActiveWeapon()->GetWpnData().iSecondaryAttachment != AllocPooledString(""))
		{
			char prclName2[256];
			Q_snprintf(prclName2, sizeof(prclName2), "%s", pPlayer->GetActiveWeapon()->GetWpnData().iMuzzleFlashDelayed);
			const char *prclName = prclName2;
			char prclAName2[256];
			Q_snprintf(prclAName2, sizeof(prclAName2), "%s", pPlayer->GetActiveWeapon()->GetWpnData().iSecondaryAttachment);
			const char *prclAName = prclAName2;
			DispatchParticleEffect(prclName, PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), prclAName, false);
		}
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::SecondaryAttack(void)
{

	if (m_bShotDelayed)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	/*if ((pPlayer->GetAmmoCount(GetSecondaryAmmoType()) <= 0))
	{
		BaseClass::WeaponSound(EMPTY);
		return;
	}

	// Cannot fire underwater
	if (GetOwner() && GetOwner()->GetWaterLevel() == 3)
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack;
		return;
	}*/

	m_bShotDelayed = true;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;

	if (pPlayer)
	{
		pPlayer->RumbleEffect(RUMBLE_AR2_ALT_FIRE, 0, RUMBLE_FLAG_RESTART);
	}

	//SendWeaponAnim(ACT_VM_FIDGET); 
	SendWeaponAnim(GetWpnData().animData[m_bFireMode].FireSecondaryCharging);

	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();// +0.5f;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();//SequenceDuration();

	WeaponSound(SPECIAL1);
	m_bIsFiring = true;
	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

	DelayedAttackEffects();
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponAR2::CanHolster(void)
{
	if (m_bShotDelayed)
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);

	//	FireBullets()->m_iPrimaryAmmoType -> AddAmmoType("AR2",				DMG_SONIC,					TRACER_DYNAMIC_AR2,	"sk_plr_dmg_ar2",			"sk_npc_dmg_ar2",			"sk_max_ar2",			BULLET_IMPULSE(600, 3525), 0 );
	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	WeaponSound(WPN_DOUBLE);

	if (!GetOwner())
		return;

	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if (!pNPC)
		return;

	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecSrc, angShootDir);
		AngleVectors(angShootDir, &vecAiming);
	}
	else
	{
		vecSrc = pNPC->Weapon_ShootPosition();

		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>(pNPC);
		if (pSoldier)
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if (!pNPC->GetEnemy())
				return;

			vecTarget = pNPC->GetEnemy()->BodyTarget(vecSrc);
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize(vecAiming);
	}

	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 0.5f, sk_weapon_ar2_alt_fire_duration.GetFloat());
	float flRadius = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 4.0f, sk_weapon_ar2_alt_fire_radius.GetFloat());

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(vecSrc,
		vecVelocity,
		flRadius,
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	if (bSecondary)
	{
		FireNPCSecondaryAttack(pOperator, true);
	}
	else
	{
		// Ensure we have enough rounds in the clip
		m_iClip1++;

		FireNPCPrimaryAttack(pOperator, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	//3015 - First round
	//4000 - Second round
	//3018 - Third round

	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
	{
		SetRoundVisibility(pEvent, 2);
	}
	break;
	case EVENT_WEAPON_RELOAD_START:
	{
		SetRoundVisibility(pEvent, 3);
	}
	break;
	case EVENT_WEAPON_RELOAD_FILL_CLIP:
	{
		SetRoundVisibility(pEvent, 4);
	}
	break;

	case EVENT_WEAPON_AR2:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	case EVENT_WEAPON_AR2_ALTFIRE:
	{
		FireNPCSecondaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

const WeaponProficiencyInfo_t *CWeaponAR2::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
