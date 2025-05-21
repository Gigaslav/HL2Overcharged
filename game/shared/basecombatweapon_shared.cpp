//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "physics_saverestore.h"
#include "datacache/imdlcache.h"
#include "activitylist.h"
#include "particle_parse.h"
// NVNT start extra includes
#include "haptics/haptic_utils.h"
#if !defined( CLIENT_DLL )
#include "OverCharged/item_empty_magazines.h"
#endif
#ifdef CLIENT_DLL
#include "prediction.h"
#endif

#include "saverestoretypes.h"
#include "saverestore_utlvector.h"

// NVNT end extra includes

#if defined ( TF_DLL ) || defined ( TF_CLIENT_DLL )
#include "tf_shareddefs.h"
#endif

#if !defined( CLIENT_DLL )

// Game DLL Headers
#include "weapon_physcannon.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "eventlist.h"
#include "soundent.h"
#include "eventqueue.h"
#include "fmtstr.h"
#include "gameweaponmanager.h"
#include "te_effect_dispatch.h"
#include "OverCharged/ShotgunBullet.h"
#ifdef HL2MP
#include "hl2mp_gamerules.h"
#endif

#endif

#include "hl2_player_shared.h"
#include "in_buttons.h"
#include <algorithm>
#include <iterator>
#if !defined( CLIENT_DLL )
#include "globalstate.h"
#endif
#include "IEffects.h"//дым от ствола
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The minimum time a hud hint for a weapon should be on screen. If we switch away before
// this, then teh hud hint counter will be deremented so the hint will be shown again, as
// if it had never been seen. The total display time for a hud hint is specified in client
// script HudAnimations.txt (which I can't read here). 
#define MIN_HUDHINT_DISPLAY_TIME 7.0f
#define	TIME_AFTER_ANIMSTATE 0.15f
#define HIDEWEAPON_THINK_CONTEXT			"BaseCombatWeapon_HideThink"

extern bool UTIL_ItemCanBeTouchedByPlayer(CBaseEntity *pItem, CBasePlayer *pPlayer);

#if defined ( TF_CLIENT_DLL ) || defined ( TF_DLL )
#ifdef _DEBUG
ConVar tf_weapon_criticals_force_random("tf_weapon_criticals_force_random", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
#endif // _DEBUG
ConVar tf_weapon_criticals_bucket_cap("tf_weapon_criticals_bucket_cap", "1000.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar tf_weapon_criticals_bucket_bottom("tf_weapon_criticals_bucket_bottom", "-250.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar tf_weapon_criticals_bucket_default("tf_weapon_criticals_bucket_default", "300.0", FCVAR_REPLICATED | FCVAR_CHEAT);
#endif // TF

ConVar oc_weapon_357_enable_scope("oc_weapon_357_enable_scope", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Player reload state.");

static ConVarRef HostPitchLoopingFire("host_timescale");


//===============================GLOBAL=========
//BriJee OVR: Weapon holster anim. main function.
#ifdef CLIENT_DLL
#define CShowWeapon C_ShowWeapon
#endif
class CShowWeapon : public CAutoGameSystemPerFrame
{
public:
	bool Init()
	{
		ClearShowWeapon();
		return true;
	}
	void UpdateWeaponActivites(CBaseCombatWeapon *m_pWeapon, bool fillArray = false)
	{
		if (!m_pWeapon || m_pWeapon == NULL || m_pWeapon == nullptr)
			return;

		Assert(m_pWeapon);
		if (m_pWeapon->GetOwner())
		{
			const int arrCount = 115;
			int attackActivites2[arrCount] =
			{
				ACT_RESET,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].ThrowPrimary,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].ThrowSecondary,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimaryTwoRounds,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimaryAlmostEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimaryCompleteEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedTwoRounds,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedAlmostEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedCompleteEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedTwoRounds,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedAlmostEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedCompleteEmpty,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimary1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimary2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimary3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimary4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimarySilenced1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimarySilenced2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimarySilenced3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FirePrimarySilenced4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsighted1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsighted2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsighted3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsighted4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedSilenced1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedSilenced2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedSilenced3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireIronsightedSilenced4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScoped1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScoped2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScoped3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScoped4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedSilenced1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedSilenced2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedSilenced3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireScopedSilenced4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondary1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondary2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondary3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondary4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryLast,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryCharging,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryIronsighted1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryIronsighted2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryIronsighted3,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].FireSecondaryIronsighted4,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeAttack1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeAttack2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeMiss1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeMiss2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeSwing1,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].MeleeSwing2,
				m_pWeapon->GetWpnData().animData[m_pWeapon->m_bFireMode].WeaponSwitchAnim,
				ACT_VM_PRIMARYATTACK,
				ACT_VM_SECONDARYATTACK,
				//m_pWeapon->GetSecondaryAttackActivity(),
				ACT_VM_SECONDARYATTACK_SPECIAL,
				ACT_VM_PRIMARYATTACK_SPECIAL,
				ACT_VM_PRIMARYATTACK_MIDEMPTY,
				ACT_VM_SHOOTLAST,
				ACT_VM_SHOOTLAST_IRONSIGHTED,
				ACT_VM_DRYFIRE,
				ACT_VM_DRYFIRE_LEFT,
				ACT_VM_DRYFIRE_SILENCED,
				ACT_VM_EMPTY_FIRE,
				ACT_VM_PRIMARYATTACK_1,
				ACT_VM_PRIMARYATTACK_2,
				ACT_VM_PRIMARYATTACK_3,
				ACT_VM_PRIMARYATTACK_4,
				ACT_VM_PRIMARYATTACK_5,
				ACT_VM_PRIMARYATTACK_6,
				ACT_VM_PRIMARYATTACK_7,
				ACT_VM_PRIMARYATTACK_8,
				ACT_VM_PRIMARYATTACK_DEPLOYED,
				ACT_VM_PRIMARYATTACK_DEPLOYED_1,
				ACT_VM_PRIMARYATTACK_DEPLOYED_2,
				ACT_VM_PRIMARYATTACK_DEPLOYED_3,
				ACT_VM_PRIMARYATTACK_DEPLOYED_4,
				ACT_VM_PRIMARYATTACK_DEPLOYED_5,
				ACT_VM_PRIMARYATTACK_DEPLOYED_6,
				ACT_VM_PRIMARYATTACK_DEPLOYED_7,
				ACT_VM_PRIMARYATTACK_DEPLOYED_8,
				ACT_VM_PRIMARYATTACK_DEPLOYED_EMPTY,
				ACT_VM_PRIMARYATTACK_EMPTY,
				ACT_VM_PRIMARYATTACK_IRONSIGHTED1,
				ACT_VM_PRIMARYATTACK_IRONSIGHTED2,
				ACT_VM_PRIMARYATTACK_IRONSIGHTED3,
				ACT_VM_PRIMARYATTACK_MIDEMPTY,
				ACT_VM_PRIMARYATTACK_MIDEMPTY_IRONSIGHTED,
				ACT_VM_PRIMARYATTACK_QRL,
				ACT_VM_PRIMARYATTACK_SILENCED,
				ACT_VM_RECOIL1,
				ACT_VM_RECOIL2,
				ACT_VM_RECOIL3,
				ACT_VM_THROW,
				ACT_VM_MELEE,
				ACT_VM_HITCENTER,
				ACT_VM_HITCENTER2,
				ACT_VM_HITCENTER_SPECIAL,
				ACT_VM_HITLEFT,
				ACT_VM_HITLEFT2,
				ACT_VM_HITRIGHT,
				ACT_VM_HITRIGHT2,
				ACT_VM_MISSCENTER,
				ACT_VM_MISSCENTER2,
				ACT_VM_MISSLEFT,
				ACT_VM_MISSLEFT2,
				ACT_VM_MISSRIGHT,
				ACT_VM_MISSRIGHT2,
				ACT_VM_SWINGHARD,
				ACT_VM_SWINGMISS,
				ACT_VM_SWINGHIT,
				ACT_VM_ATTACH_SILENCER,
				ACT_VM_DETACH_SILENCER,
				ACT_VM_PULLBACK
			};

			if (fillArray)
			{
				m_pWeapon->attackActivites.RemoveAll();

				for (int i = 0; i < arrCount; i++)
				{
					m_pWeapon->attackActivites.AddToHead((Activity)attackActivites2[i]);
				}
			}
		}
	}
	void FrameUpdatePreEntityThink()
	{
		if (m_pWeapon&&m_flTime<gpGlobals->curtime)
		{
			ShowWeapon();
		}
	}
	void Update(float frametime)
	{
		FrameUpdatePreEntityThink();
	}
	void SetShowWeapon(CBaseCombatWeapon *pWeapon, int iActivity, float delta)
	{
		m_pWeapon = pWeapon;
		m_iActivity = iActivity;
		if (delta == 0)
		{
			ShowWeapon();
		}
		else
		{
			m_flTime = gpGlobals->curtime + delta;
		}
	}
	void ClearShowWeapon()
	{
		m_pWeapon = NULL;
	}
private:
	void ShowWeapon()
	{
		if (!m_pWeapon || m_pWeapon == NULL || m_pWeapon == nullptr)
			return;

		Assert(m_pWeapon);
		m_pWeapon->SetWeaponVisible(true);
		if (m_pWeapon->GetOwner())
		{
			CBaseCombatWeapon *pLastWeapon = m_pWeapon->GetOwner()->GetActiveWeapon();
			m_pWeapon->GetOwner()->m_hActiveWeapon = m_pWeapon;
			CBasePlayer *pOwner = ToBasePlayer(m_pWeapon->GetOwner());
			if (pOwner && pOwner->IsPlayer())
			{
				m_pWeapon->SetViewModel();
				m_pWeapon->SendWeaponAnim(m_iActivity);

				m_pWeapon->letViewModelBob = true;
				//m_pWeapon->thisType = m_pWeapon->GetWpnData().weaponType;

				//std::array<Activity, 102> attackActivites2 =
				if (m_pWeapon->m_bWeaponBlockWall)
					m_pWeapon->m_bWeaponBlockWall = false;

				UpdateWeaponActivites(m_pWeapon, true);

				//attackActivites = attackActivites2;

				if (m_pWeapon->GetOwner() && m_pWeapon->GetOwner()->IsPlayer())
					m_pWeapon->AddMuzzleFlash();

				m_pWeapon->SetupAmmo();

				//#ifndef CLIENT_DLL
				if (m_pWeapon->GetWpnData().allowLoopSound && m_pWeapon->m_bIsFiring)
				{
					m_pWeapon->StopLoopSound();
					m_pWeapon->DestroyLoopSound();
				}
				else if (m_pWeapon->GetWpnData().allowLoopSound)
					m_pWeapon->DestroyLoopSound();

				if (m_pWeapon->GetWpnData().allowLoopSound)
				{
					m_pWeapon->PrecacheScriptSound(m_pWeapon->GetWpnData().ShootLoopingSound);
					m_pWeapon->PrecacheScriptSound(m_pWeapon->GetWpnData().EndLoopingSound);

					if (m_pWeapon->GetWpnData().allowLoopSilencerSound)
					{
						m_pWeapon->PrecacheScriptSound(m_pWeapon->GetWpnData().ShootLoopingSilencerSound);
						m_pWeapon->PrecacheScriptSound(m_pWeapon->GetWpnData().EndLoopingSilencerSound);
					}
				}

				if (m_pWeapon->m_bIsInPrimaryAttack && m_pWeapon->m_flNextPrimaryAttack < gpGlobals->curtime)
					m_pWeapon->m_bIsInPrimaryAttack = false;

				if (pOwner && m_pWeapon->GetWpnData().WeaponUsingSilencer)
				{
					CBaseViewModel *pViewModel = pOwner->GetViewModel();
					m_pWeapon->bg = pViewModel->FindBodygroupByName(m_pWeapon->GetWpnData().silencerBodyGroup);
					if (m_pWeapon->m_bSilenced && !pViewModel->GetBodygroup(m_pWeapon->bg))
					{
						m_pWeapon->bgV = m_pWeapon->GetWpnData().silencerBGEnableValue;

						pViewModel->SetBodygroup(m_pWeapon->bg, m_pWeapon->bgV);

						m_pWeapon->ReinitializeLoopSound(m_pWeapon->GetWpnData().ShootLoopingSilencerSound);
					}
					else if (!m_pWeapon->m_bSilenced && pViewModel->GetBodygroup(m_pWeapon->bg))
					{
						m_pWeapon->bgV = m_pWeapon->GetWpnData().silencerBGDisableValue;

						pViewModel->SetBodygroup(m_pWeapon->bg, m_pWeapon->bgV);

						m_pWeapon->ReinitializeLoopSound(m_pWeapon->GetWpnData().ShootLoopingSound);
					}
				}
				else
					m_pWeapon->ReinitializeLoopSound(m_pWeapon->GetWpnData().ShootLoopingSound);

				//if (Q_strcmp(m_pWeapon->GetWpnData().WeaponType, "shotgun") == 0)
				if (m_pWeapon->thisType == TYPE_SHOTGUN)
					m_pWeapon->m_bReloadsSingly = true;
				//#endif
				pOwner->SetNextAttack(gpGlobals->curtime + m_pWeapon->GetViewModelSequenceDuration());

				if (pLastWeapon && pOwner->Weapon_ShouldSetLast(pLastWeapon, m_pWeapon))
				{
					pOwner->Weapon_SetLast(pLastWeapon->GetLastWeapon());
				}

				CBaseViewModel *pViewModel = pOwner->GetViewModel();
				Assert(pViewModel);
				if (pViewModel)
					pViewModel->RemoveEffects(EF_NODRAW);
				pOwner->ResetAutoaim();
			}
			/*else if (pOwner && !pOwner->IsPlayer())
			{
			pOwner->SetNextAttack(gpGlobals->curtime + m_pWeapon->GetViewModelSequenceDuration());

			if (pLastWeapon && pOwner->Weapon_ShouldSetLast(pLastWeapon, m_pWeapon))
			{
			pOwner->Weapon_SetLast(pLastWeapon->GetLastWeapon());
			}

			m_pWeapon->RemoveEffects(EF_NODRAW);
			}*/
		}

		// Can't shoot again until we've finished deploying
		m_pWeapon->m_flNextSecondaryAttack = m_pWeapon->m_flNextPrimaryAttack = gpGlobals->curtime + m_pWeapon->GetViewModelSequenceDuration();

		ClearShowWeapon();
	}
	CBaseCombatWeapon *m_pWeapon;
	int m_iActivity;
	float m_flTime;
};
static CShowWeapon g_ShowWeapon;


void CBaseCombatWeapon::OnIronSight(void)
{
	MDLCACHE_CRITICAL_SECTION();
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

#ifndef CLIENT_DLL
	CHL2_Player *pHl2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (pHl2Player)
	{
		if (!pHl2Player->IsZooming())
		{
			if (pHl2Player->CanZoom(this))
				pHl2Player->StartZooming(true, GetWpnData().flIronsightFOVOffset);
		}
		else
		{
			OffIronSight();
			return;
		}
	}
#endif


	if (pPlayer)
	{
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel)
			pViewModel->m_bExpSighted = true;
	}

	IronSightState = 1;

	m_flNextInspectAnimation = 0;

	cvar->FindVar("oc_state_IRsight_on")->SetValue(IronSightState);
}

void CBaseCombatWeapon::OffIronSight(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

#ifndef CLIENT_DLL
	CHL2_Player *pHl2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (pHl2Player)
	{
		if (pHl2Player->IsZooming())
		{
			pHl2Player->StopZooming(true);
		}
	}
#endif

	if (pPlayer)
	{
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel)
			pViewModel->m_bExpSighted = false;
	}

	IronSightState = 0;

	cvar->FindVar("oc_state_IRsight_on")->SetValue(IronSightState);

}


bool CBaseCombatWeapon::IsIronSighted()
{
	MDLCACHE_CRITICAL_SECTION();

	bool state = false;

	state = IronSightState > 0 ? true : false;

	return state;
}

void CBaseCombatWeapon::OnScopeSight(void)
{
	MDLCACHE_CRITICAL_SECTION();
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel)
			pViewModel->m_bExpScope = true;
	}

	ScopeSightState = true;

	ToggleZoom(pPlayer, ScopeSightState);

	m_flNextInspectAnimation = 0;

	cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
}
void CBaseCombatWeapon::OffScopeSight(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel)
			pViewModel->m_bExpScope = false;
	}

	ScopeSightState = false;

	ToggleZoom(pPlayer, ScopeSightState);

	cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
}

bool CBaseCombatWeapon::IsScopeSighted()
{
	MDLCACHE_CRITICAL_SECTION();

	//bool state = false;

	//state = (ScopeSightState > 0) ? true : false;

	return ScopeSightState;
}

void CBaseCombatWeapon::UploadDataScript()
{
	//string_t name = m_iClassname;
	//if (!gEntList.FindEntityByClassname(this, GetClassname()))
	/*#ifndef CLIENT_DLL
	const char* ClassName = m_iClassname.ToCStr();

	PrecacheIndividualScriptInfoData(ClassName, filesystem, g_pGameRules->GetEncryptionKey());
	#endif*/
}

CBaseCombatWeapon::CBaseCombatWeapon()
{
	// Constructor must call this
	// CONSTRUCT_PREDICTABLE( CBaseCombatWeapon );

	m_vTracerMuzzleVector = Vector(0, 0, 0);
	m_vMuzzleVector = Vector(0, 0, 0);
	m_vMuzzleAngle = QAngle(0, 0, 0);

	iVMSkin = 0;

#ifndef CLIENT_DLL
	IronSightState = false;
	ScopeSightState = false;

#endif

	m_bShouldDrawWeaponBloodOverlay = false;
	m_iWeaponBloodOverlayDetailFrame = 0;
	m_flNextWallClipTime = 0.f;
	m_flNextLowerTime = 0.f;
	//m_flNextRemoveAmmo = 0.f;
	//m_flAmmoRemoveDelay = 0.f;
	m_bShouldBeLowered = false;
	HostPitchLoopingFire = cvar->FindVar("host_timescale");
	m_iMuzzleFlashState = 0;
	holdingFire = false;
#if defined( CLIENT_DLL )
	ScopeSightState = false;
	IronSightState = false;
	m_flNextMuzzleFlashTime = gpGlobals->curtime;
	IsMuzzleFlashActive = false;
	IsMuzzleFlashComplete = false;
	ReinitializeMuzzleFlash = false;
	m_flMuzzleFlashRoll = 0.0f;
#endif

#if !defined( CLIENT_DLL )
	soundInitialVolume = 0.0f;
	soundInitialPitch = 0.0f;

	m_pLoopSound = NULL;
	m_pLoopSoundController = NULL;
#endif
	m_BeamFireState = FIRE_NONE;
	letViewModelBob = true;
	m_bIsFiring = false;
	m_bIsInPrimaryAttack = false;
	thisType = GetWpnData().weaponType;
	PumpAfterTwoBarrels = false;
	reloadFromEmpty = false;
	m_bEnableMainLaser = false;
	IronSightState = 0;
	ScopeSightState = false;
	m_bWeaponBlockWall = false;
	_enabledLaser = false;
	EnableLaser = false;
	EnableLaserInterrupt = false;
	clipNearWall = false;
	distanceToWall = 0.f;
	tr_endPos = Vector(0, 0, 0);
	iMuzzle = 1;
	m_bFirstEquip = true;
	m_bFireMode = 0;
	m_bFireModeCounter = 1;
	burstFireCount = 0;
	//m_bFireModes = NULL;
	m_bFireModesSize = 0;
	switched = false;
	wasburstFire = false;
	m_flAccuracyPenalty = 0.0f;
	m_flNextInspectAnimation = 0;
	m_flNextLaserSwitchTime = gpGlobals->curtime;
	m_flNextSwitchTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flNextShotgunReload = gpGlobals->curtime;
	m_flNextAmmoRegen = gpGlobals->curtime;
	m_flNextAmmoRemove = gpGlobals->curtime;
	m_iShotgunReloadState = 0;
	m_bZoomLevel = 0;
	m_bZoomUpped = false;
	blockToAmmoRegen = false;
	scoped = false;
	// BriJee OVR: New functions.
	//m_bSilenced = false;
	m_bReloadComplete = 0;
	m_wSequence = 0;
	m_wSequence2 = 0;
	ChangeOnce = true;
	// Some default values.  There should be set in the particular weapon classes
	m_fMinRange1 = 65;
	m_fMinRange2 = 65;
	m_fMaxRange1 = 1024;
	m_fMaxRange2 = 1024;
	m_bInReload = false;
	m_bReloadsSingly = false;

	// Defaults to zero
	m_nViewModelIndex = 0;

	m_bFlipViewModel = false;
	/*m_bWeaponIsLoweredWall = false;
	m_bWeaponIsLoweredSpeed = false;
	TimerSpeed = 0;
	TimerWall = 0;*/
#if defined( CLIENT_DLL )
	m_iState = m_iOldState = WEAPON_NOT_CARRIED;
	m_iClip1 = -1;
	m_iClip2 = -1;
	m_iPrimaryAmmoType = -1;
	m_iSecondaryAmmoType = -1;
#endif

#if !defined( CLIENT_DLL )
	m_pConstraint = NULL;
	OnBaseCombatWeaponCreated(this);
#endif

	m_hWeaponFileInfo = GetInvalidWeaponInfoHandle();

#if defined( TF_DLL )
	UseClientSideAnimation();
#endif

#if defined ( TF_CLIENT_DLL ) || defined ( TF_DLL )
	m_flCritTokenBucket = tf_weapon_criticals_bucket_default.GetFloat();
	m_nCritChecks = 1;
	m_nCritSeedRequests = 0;
#endif // TF
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseCombatWeapon::~CBaseCombatWeapon(void)
{
	Pickable = false;
	m_flNextPick = 0.f;
	m_iMuzzleFlashState = 0;
	holdingFire = false;

#if defined( CLIENT_DLL )
	if (m_M)
	{
		delete m_M;
		m_M = NULL;
	}
	if (m_Ar2_MM)
	{
		delete m_Ar2_MM;
		m_Ar2_MM = NULL;
	}
	m_flNextMuzzleFlashTime = 0.f;
	IsMuzzleFlashActive = false;
	IsMuzzleFlashComplete = false;
	ReinitializeMuzzleFlash = false;
#endif

	DestroyLoopSound();
	m_wSequence = 0;
	m_wSequence2 = 0;
#if !defined( CLIENT_DLL )
	if (m_hLaserMuzzleSprite)
	{
		UTIL_Remove(m_hLaserMuzzleSprite);
	}
#endif
	m_bReloadComplete = 0;
#if !defined( CLIENT_DLL )
	//Remove our constraint, if we have one
	if (m_pConstraint != NULL)
	{
		physenv->DestroyConstraint(m_pConstraint);
		m_pConstraint = NULL;
	}
	OnBaseCombatWeaponDestroyed(this);
#endif
#ifndef CLIENT_DLL
	if (m_pLoopSound)
		DestroyLoopSound();
#endif
}

void CBaseCombatWeapon::Activate(void)
{

	BaseClass::Activate();

	SetSkin(iVMSkin);

#ifndef CLIENT_DLL
	if (GetOwnerEntity())
		return;

	if (g_pGameRules->IsAllowedToSpawn(this) == false)
	{
		UTIL_Remove(this);
		return;
	}
#endif

}

void CBaseCombatWeapon::SetupWeapon(CBasePlayer *pOwner)
{
	MDLCACHE_CRITICAL_SECTION();

	//CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	//if (pOwner && pOwner->IsPlayer())
	{
		//pLastWeapon->letViewModelBob = true;

		//UpdateWeaponActivites(pLastWeapon, true);

		m_bFiresUnderwater = GetWpnData().m_bFiresUnderwater;
		m_bAltFiresUnderwater = GetWpnData().m_bAltFiresUnderwater;

		//MikeD Set VM skin
		SetSkin(iVMSkin, !pOwner->IsSuitEquipped());

		//#ifndef CLIENT_DLL
		/*if (pLastWeapon->GetWpnData().allowLoopSound && pLastWeapon->m_bIsFiring)
		{
		pLastWeapon->StopLoopSound();
		pLastWeapon->DestroyLoopSound();
		}
		else if (pLastWeapon->GetWpnData().allowLoopSound)
		pLastWeapon->DestroyLoopSound();

		if (pLastWeapon->GetWpnData().allowLoopSound)
		{
		pLastWeapon->PrecacheScriptSound(pLastWeapon->GetWpnData().ShootLoopingSound);
		pLastWeapon->PrecacheScriptSound(pLastWeapon->GetWpnData().EndLoopingSound);

		if (pLastWeapon->GetWpnData().allowLoopSilencerSound)
		{
		pLastWeapon->PrecacheScriptSound(pLastWeapon->GetWpnData().ShootLoopingSilencerSound);
		pLastWeapon->PrecacheScriptSound(pLastWeapon->GetWpnData().EndLoopingSilencerSound);
		}
		}

		if (pLastWeapon->m_bIsInPrimaryAttack && pLastWeapon->m_flNextPrimaryAttack < gpGlobals->curtime)
		pLastWeapon->m_bIsInPrimaryAttack = false;

		if (pLastWeapon->GetWpnData().WeaponUsingSilencer)
		{
		CBaseViewModel *pViewModel = pOwner->GetViewModel();
		if (pViewModel)
		{
		pLastWeapon->bg = pViewModel->FindBodygroupByName(pLastWeapon->GetWpnData().silencerBodyGroup);

		if (pViewModel->GetBodygroup(bg))
		{
		if (pLastWeapon->m_bSilenced)
		{
		pLastWeapon->bgV = pLastWeapon->GetWpnData().silencerBGEnableValue;

		pViewModel->SetBodygroup(pLastWeapon->bg, pLastWeapon->bgV);

		pLastWeapon->ReinitializeLoopSound(pLastWeapon->GetWpnData().ShootLoopingSilencerSound);
		}
		else if (!pLastWeapon->m_bSilenced)
		{
		pLastWeapon->bgV = pLastWeapon->GetWpnData().silencerBGDisableValue;

		pViewModel->SetBodygroup(pLastWeapon->bg, pLastWeapon->bgV);

		pLastWeapon->ReinitializeLoopSound(pLastWeapon->GetWpnData().ShootLoopingSound);
		}
		}
		}
		}*/
		//else
		//pLastWeapon->ReinitializeLoopSound(pLastWeapon->GetWpnData().ShootLoopingSound);


		//if (pLastWeapon->thisType == TYPE_SHOTGUN)
		//pLastWeapon->m_bReloadsSingly = true;

		//pOwner->SetNextAttack(gpGlobals->curtime + pLastWeapon->GetViewModelSequenceDuration());

		//if (pLastWeapon && pOwner->Weapon_ShouldSetLast(pLastWeapon, pLastWeapon))
		{
			//pOwner->Weapon_SetLast(pLastWeapon->GetLastWeapon());
		}

		/*CBaseViewModel *pViewModel = pOwner->GetViewModel();
		Assert(pViewModel);
		if (pViewModel)
		pViewModel->RemoveEffects(EF_NODRAW);
		pOwner->ResetAutoaim();*/
	}
}

void CBaseCombatWeapon::SetupAmmo(void)
{
	MDLCACHE_CRITICAL_SECTION();

	if (!GetWpnData().BulletOverride)
		return;

	CAmmoDef *pAmmoType = GetAmmoDef();
	if (pAmmoType)
	{
		Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex(m_iPrimaryAmmoType);
		//Ammo_t pAmmo = pAmmoType->m_AmmoType[pAmmoType->m_nAmmoIndex];
		if (pAmmo)
		{
			pAmmo->physicsForceImpulse = GetWpnData().BulletImpulseForce;
			pAmmo->bulletSpeed = GetWpnData().BulletSpeed;
		}
	}
}

void CBaseCombatWeapon::AddMuzzleFlash(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner && pOwner->GetActiveWeapon() && GetWpnData().AllowProjectedMuzzleFlashes)
	{
#ifndef CLIENT_DLL
		if (pOwner)
		{
			CEffectData	MFCallBack;
			MFCallBack.m_nEntIndex = entindex();
			DispatchEffect("SetMuzzleFlashActive", MFCallBack);
		}
#endif
		m_iMuzzleFlashState = 1;

	}
}

void CBaseCombatWeapon::RemoveMuzzleFlash(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner && pOwner->GetActiveWeapon() && m_iMuzzleFlashState > 0 ? true : GetWpnData().AllowProjectedMuzzleFlashes)
	{
#ifndef CLIENT_DLL
		if (pOwner)
		{
			CEffectData	MFCallBack;
			MFCallBack.m_nEntIndex = entindex();
			DispatchEffect("SetMuzzleFlashDeActive", MFCallBack);
		}
#endif
		m_iMuzzleFlashState = 0;
	}
}

bool CBaseCombatWeapon::IsClippedAmmo(void)
{
	//true - clip_size; false - default_clip
	return GetWpnData().iMaxClip1 >= 0 ? true : false;// GetWpnData().iDefaultClip1 >= 0;
}

void CBaseCombatWeapon::GiveDefaultAmmo(void)
{
	// If I use clips, set my clips to the default
	if (UsesClipsForAmmo1())
	{
		m_iClip1 = AutoFiresFullClip() ? 0 : GetDefaultClip1();
	}
	else
	{
		SetPrimaryAmmoCount(GetDefaultClip1());
		m_iClip1 = WEAPON_NOCLIP;
	}
	if (UsesClipsForAmmo2())
	{
		m_iClip2 = GetDefaultClip2();
	}
	else
	{
		SetSecondaryAmmoCount(GetDefaultClip2());
		m_iClip2 = WEAPON_NOCLIP;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set mode to world model and start falling to the ground
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Spawn(void)
{
	Pickable = true;

	Precache();

	BaseClass::Spawn();

	SetSolid(SOLID_BBOX);
	m_flNextEmptySoundTime = 0.0f;

	// Weapons won't show up in trace calls if they are being carried...
	RemoveEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);

	m_iState = WEAPON_NOT_CARRIED;
	// Assume 
	m_nViewModelIndex = 0;

	//if (!IsEFlagSet(FL_GRAPHED))
	GiveDefaultAmmo();

	if (cvar->FindVar("oc_player_draw_body")->GetInt())
	{
		/*if (!cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
		{

		AddEffects(EF_NODRAW);
		}
		else
		{*/
		if (!cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			GetBaseAnimating()->SetRenderMode(kRenderNone);

		if (GetWorldModel())
			SetModel(GetWorldModel());

		//}
	}
	else
	{
		if (GetWorldModel())
		{
			SetModel(GetWorldModel());
		}
	}

#if !defined( CLIENT_DLL )
	if (IsX360())
	{
		AddEffects(EF_ITEM_BLINK);
	}

	FallInit();
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetBlocksLOS(false);

	// Default to non-removeable, because we don't want the
	// game_weapon_manager entity to remove weapons that have
	// been hand-placed by level designers. We only want to remove
	// weapons that have been dropped by NPC's.
	SetRemoveable(false);
#endif

	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds(true, 36);

	// Use more efficient bbox culling on the client. Otherwise, it'll setup bones for most
	// characters even when they're not in the frustum.
	AddEffects(EF_BONEMERGE_FASTCULL);

	m_iReloadHudHintCount = 0;
	m_iAltFireHudHintCount = 0;
	m_flHudHintMinDisplayTime = 0;

	m_bZoomUpped = false;
	scoped = false;
	m_bFirstEquip = true;

#ifndef CLIENT_DLL

	//SetSkin(iVMSkin);

	thisType = GetWpnData().weaponType;

#endif
}

//-----------------------------------------------------------------------------
// Purpose: get this game's encryption key for decoding weapon kv files
// Output : virtual const unsigned char
//-----------------------------------------------------------------------------
const unsigned char *CBaseCombatWeapon::GetEncryptionKey(void)
{
	return g_pGameRules->GetEncryptionKey();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Precache(void)
{
	PrecacheScriptSound("Weapon_low_ammo.low_ammo");

	//if (GetAdditionalViewModel() != NULL)
	//PrecacheModel(GetAdditionalViewModel());
#if defined( CLIENT_DLL )
	Assert(Q_strlen(GetClassname()) > 0);
	// Msg( "Client got %s\n", GetClassname() );
#endif
	m_iPrimaryAmmoType = m_iSecondaryAmmoType = -1;

	/*#ifndef CLIENT_DLL
	if (GetWpnData().allowLoopSound)
	{
	PrecacheScriptSound(GetWpnData().ShootLoopingSound);
	PrecacheScriptSound(GetWpnData().EndLoopingSound);

	CreateLoopSound(GetWpnData().ShootLoopingSound);
	if (GetWpnData().allowLoopSilencerSound)
	{
	PrecacheScriptSound(GetWpnData().ShootLoopingSound);
	PrecacheScriptSound(GetWpnData().EndLoopingSilencerSound);
	}
	}

	#endif*/

	// Add this weapon to the weapon registry, and get our index into it
	// Get weapon data from script file
	if (ReadWeaponDataFromFileForSlot(filesystem, GetClassname(), &m_hWeaponFileInfo, GetEncryptionKey()))
	{

		if (GetWpnData().m_bHasMag)
		{
			PrecacheModel(GetWpnData().iMagName);
			PrecacheScriptSound(GetWpnData().magHitSound);
		}


		// Get the ammo indexes for the ammo's specified in the data file
		if (GetWpnData().szAmmo1[0])
		{
			m_iPrimaryAmmoType = GetAmmoDef()->Index(GetWpnData().szAmmo1);
			if (m_iPrimaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined primary ammo type (%s)\n", GetClassname(), GetWpnData().szAmmo1);
			}
#if defined ( TF_DLL ) || defined ( TF_CLIENT_DLL )
			// Ammo override
			int iModUseMetalOverride = 0;
			CALL_ATTRIB_HOOK_INT(iModUseMetalOverride, mod_use_metal_ammo_type);
			if (iModUseMetalOverride)
			{
				m_iPrimaryAmmoType = (int)TF_AMMO_METAL;
			}
#endif
		}
		if (GetWpnData().szAmmo2[0])
		{
			m_iSecondaryAmmoType = GetAmmoDef()->Index(GetWpnData().szAmmo2);
			if (m_iSecondaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined secondary ammo type (%s)\n", GetClassname(), GetWpnData().szAmmo2);
			}

		}
#if defined( CLIENT_DLL )
		gWR.LoadWeaponSprites(GetWeaponFileInfoHandle());
#endif
		// Precache models (preload to avoid hitch)
		m_iViewModelIndex = 0;
		m_iWorldModelIndex = 0;
		if (GetViewModel() && GetViewModel()[0])
		{
			m_iViewModelIndex = CBaseEntity::PrecacheModel(GetViewModel());
		}
		if (GetWorldModel() && GetWorldModel()[0])
		{
			m_iWorldModelIndex = CBaseEntity::PrecacheModel(GetWorldModel());
		}

		// Precache sounds, too
		for (int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i)
		{
			const char *shootsound = GetShootSound(i);
			if (shootsound && shootsound[0])
			{
				CBaseEntity::PrecacheScriptSound(shootsound);
			}
		}
	}
	else
	{
		// Couldn't read data file, remove myself
		Warning("Error reading weapon data file for: %s\n", GetClassname());
		//	Remove( );	//don't remove, this gets released soon!
	}
}

int CBaseCombatWeapon::GetMinBurst()
{
	return GetWpnData().npcMinBurst;
}

int CBaseCombatWeapon::GetMaxBurst()
{
	return GetWpnData().npcMaxBurst;
}

//-----------------------------------------------------------------------------
// Purpose: Get my data in the file weapon info array
//-----------------------------------------------------------------------------
const FileWeaponInfo_t &CBaseCombatWeapon::GetWpnData(void) const
{
	return *GetFileWeaponInfoFromHandle(m_hWeaponFileInfo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetViewModel(int /*viewmodelindex = 0 -- this is ignored in the base class here*/) const
{
	return GetWpnData().szViewModel;
}

const char *CBaseCombatWeapon::GetAdditionalViewModel(int /*viewmodelindex = 0 -- this is ignored in the base class here*/) const
{
	return GetWpnData().szAdditionalViewModel;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetWorldModel(void) const
{
	/*if (m_bSilenced)
	{
	return (GetWpnData().szSilentModel);
	}
	else
	{
	return GetWpnData().szWorldModel;
	}*/
	return GetWpnData().szWorldModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetAnimPrefix(void) const
{
	return GetWpnData().szAnimationPrefix;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetPrintName(void) const
{
	return GetWpnData().szPrintName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip1(void) const
{
#if defined ( TF_DLL ) || defined ( TF_CLIENT_DLL )
	int iModMaxClipOverride = 0;
	CALL_ATTRIB_HOOK_INT(iModMaxClipOverride, mod_max_primary_clip_override);
	if (iModMaxClipOverride != 0)
		return iModMaxClipOverride;
#endif

	return GetWpnData().iMaxClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip2(void) const
{
	return GetWpnData().iMaxClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip1(void) const
{
	return GetWpnData().iDefaultClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip2(void) const
{
	return GetWpnData().iDefaultClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo1(void) const
{
	return (GetMaxClip1() != WEAPON_NOCLIP);
}

bool CBaseCombatWeapon::IsMeleeWeapon() const
{
	return GetWpnData().m_bMeleeWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo2(void) const
{
	return (GetMaxClip2() > WEAPON_NOCLIP && GetMaxClip2() > 0);
}


float CBaseCombatWeapon::FlashLifeTime(void) const//OverCharged
{
	return GetWpnData().iFlashLifeTime;
}

//-----------------------------------------------------------------------------
// Purpose:  Тут все новые события
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxFOV(void) const//OverCharged
{
	/*if (m_bZoomLevel != 0)
	return m_bZoomLevel;
	else
	return GetWpnData().rtFov;*/

	return GetWpnData().rtFov;
}

int CBaseCombatWeapon::FlashType(void) const//OverCharged
{
	return GetWpnData().iFlashType;
}

int CBaseCombatWeapon::MuzzleFlashTypeIndex(void) const//OverCharged
{
	return GetWpnData().externalIndexLight;
}
int CBaseCombatWeapon::MuzzleSecondaryFlashTypeIndex(void) const//OverCharged
{
	return GetWpnData().externalIndexSLight;
}
int CBaseCombatWeapon::MuzzleTracerTypeNpcIndex(void) const//OverCharged
{
	return GetWpnData().externalIndexTN;
}
string_t CBaseCombatWeapon::MuzzleTracerType(int index) const//OverCharged
{
	return GetWpnData().iMuzzleTracerType[index];
}
string_t CBaseCombatWeapon::MuzzleTracerNpcType(int index) const//OverCharged
{
	return GetWpnData().iMuzzleTracerNpcType[index];
}
string_t CBaseCombatWeapon::MuzzleFlashType(int index) const//OverCharged
{
	return GetWpnData().iMuzzleLightType[index];
}
string_t CBaseCombatWeapon::MuzzleSecondaryFlashType(int index) const//OverCharged
{
	return GetWpnData().iMuzzleSecondaryLightType[index];
}
string_t CBaseCombatWeapon::MuzzleFlashType(void) const//OverCharged
{
	return GetWpnData().iMuzzleLightType[0];
}
/*string_t CBaseCombatWeapon::MuzzleFlashTransparentType(void) const//OverCharged
{
return GetWpnData().iMuzzleFlashTransparentType;
}*/
string_t CBaseCombatWeapon::MuzzleSecondaryFlashType(void) const//OverCharged
{
	return GetWpnData().iMuzzleSecondaryLightType[0];
}
float CBaseCombatWeapon::GetFireRate(void) const//OverCharged
{
	if (cvar->FindVar("oc_test_weapon_firerate")->GetFloat() > 0)
		return	cvar->FindVar("oc_test_weapon_firerate")->GetFloat();

	//return GetWpnData().fireRate;
	return GetFireRate();
}
int CBaseCombatWeapon::NumShots(void) const
{
	if (GetWpnData().NumShots != NULL)
		return GetWpnData().NumShots;

	return 1;
}

bool CBaseCombatWeapon::UsesSecondaryAmmoType(void) const
{
	return GetWpnData().iAmmo2Type > 0;
}

int CBaseCombatWeapon::GetSecondaryAttachment()
{
	return LookupAttachment(GetWpnData().iSecondaryAttachment);
}

string_t CBaseCombatWeapon::WeaponShellModel(void) const//OverCharged
{
	return GetWpnData().iWeaponShellModel;
}
string_t CBaseCombatWeapon::WeaponShellBounceSound(void) const//OverCharged
{
	return GetWpnData().iWeaponShellBounceSound;
}
string_t CBaseCombatWeapon::WeaponShellHitSound(void) const//OverCharged
{
	return GetWpnData().iWeaponShellHitSound;
}
string_t CBaseCombatWeapon::WeaponShellParticle(void) const//OverCharged
{
	return GetWpnData().iWeaponShellParticle;
}

string_t CBaseCombatWeapon::WeaponZoomUpSound(void) const//OverCharged
{
	return GetWpnData().iWeaponZoomUpSound;
}
/*int CBaseCombatWeapon::FlashLightType(void) const//OverCharged
{
return GetWpnData().iFlashLightType;
}

int CBaseCombatWeapon::FlashLightTypeNPC(void) const//OverCharged
{
return GetWpnData().iFlashLightTypeNPC;
}

int CBaseCombatWeapon::TracerType(void) const//OverCharged
{
return GetWpnData().iTracerType;
}*/

int CBaseCombatWeapon::WeaponGetWaterLevel(void)//OverCharged
{
	//DevMsg("CBaseCombatWeapon GetWaterLevel %i \n", enginetrace->GetPointContents(this->GetAbsOrigin()) & (CONTENTS_WATER | CONTENTS_SLIME));
	Vector WeaponPos, WeaponPos2;
	this->GetAttachment("muzzle", WeaponPos);
	this->GetAttachment(1, WeaponPos2);

	if (enginetrace->GetPointContents(WeaponPos) & (CONTENTS_WATER | CONTENTS_SLIME) || enginetrace->GetPointContents(WeaponPos2) & (CONTENTS_WATER | CONTENTS_SLIME))
		return 3;
	else
		return 0;
	return 0;// CBaseEntity::m_nWaterLevel;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeight(void) const
{
	return GetWpnData().iWeight;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched to when the player runs out
//			of ammo in their current weapon or they pick this weapon up.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchTo(void) const
{
	return GetWpnData().bAutoSwitchTo;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched away from when the player
//			runs out of ammo in this weapon or picks up another weapon or ammo.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchFrom(void) const
{
	return GetWpnData().bAutoSwitchFrom;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeaponFlags(void) const
{
	return GetWpnData().iFlags;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetSlot(void) const
{
	return GetWpnData().iSlot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetPosition(void) const
{
	return GetWpnData().iPosition;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetName(void) const
{
	return GetWpnData().szClassName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteActive(void) const
{
	return GetWpnData().iconActive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteInactive(void) const
{
	return GetWpnData().iconInactive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo(void) const
{
	return GetWpnData().iconAmmo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo2(void) const
{
	return GetWpnData().iconAmmo2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteCrosshair(void) const
{
	return GetWpnData().iconCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAutoaim(void) const
{
	return GetWpnData().iconAutoaim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedCrosshair(void) const
{
	return GetWpnData().iconZoomedCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedAutoaim(void) const
{
	return GetWpnData().iconZoomedAutoaim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetShootSound(int iIndex) const
{
	return GetWpnData().aShootSounds[iIndex];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetRumbleEffect() const
{
	return GetWpnData().iRumbleEffect;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseCombatCharacter	*CBaseCombatWeapon::GetOwner() const
{
	return ToBaseCombatCharacter(m_hOwner.Get());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : BaseCombatCharacter - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetOwner(CBaseCombatCharacter *owner)
{
	if (!owner)
	{
#ifndef CLIENT_DLL
		// Make sure the weapon updates its state when it's removed from the player
		// We have to force an active state change, because it's being dropped and won't call UpdateClientData()
		int iOldState = m_iState;
		m_iState = WEAPON_NOT_CARRIED;
		OnActiveStateChanged(iOldState);
#endif

		// make sure we clear out our HideThink if we have one pending
		SetContextThink(NULL, 0, HIDEWEAPON_THINK_CONTEXT);
	}

	m_hOwner = owner;

#ifndef CLIENT_DLL
	DispatchUpdateTransmitState();
#else
	UpdateVisibility();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return false if this weapon won't let the player switch away from it
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsAllowedToSwitch(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon can be selected via the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanBeSelected(void)
{
	if (!VisibleInWeaponSelection())
		return false;

	return HasAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon has some ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAmmo(void)
{
	// Weapons with no ammo types can always be selected
	if (m_iPrimaryAmmoType == -1 && m_iSecondaryAmmoType == -1)
		return true;
	if (GetWeaponFlags() & ITEM_FLAG_SELECTONEMPTY)
		return true;

	CBasePlayer *player = ToBasePlayer(GetOwner());
	if (!player)
		return false;
	return (m_iClip1 > 0 || player->GetAmmoCount(m_iPrimaryAmmoType) || m_iClip2 > 0 || player->GetAmmoCount(m_iSecondaryAmmoType));
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon should be seen, and hence be selectable, in the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::VisibleInWeaponSelection(void)
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasWeaponIdleTimeElapsed(void)
{
	if (gpGlobals->curtime > m_flTimeWeaponIdle)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : time - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponIdleTime(float time)
{
	m_flTimeWeaponIdle = time;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetWeaponIdleTime(void)
{
	return m_flTimeWeaponIdle;
}

//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Drop(const Vector &vecVelocity)
{

#if !defined( CLIENT_DLL )
	/*if (GetOwner() && GetOwner()->IsPlayer() && GetOwner()->IsAlive())
	{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return;

	if ((pPlayer->m_nButtons & IN_ATTACK) || (pPlayer->m_nButtons & IN_ATTACK2))
	return;
	}*/

	bool ownerIsPlayer = false;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer && pPlayer->IsPlayer())
	{
		ownerIsPlayer = true;

		if (IsGrenade())//(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? thisType == TYPE_GRENADE : false)
			return;

		CHL2_Player *pHLPlayer = static_cast<CHL2_Player*>(pPlayer);
		if (pHLPlayer)
		{
			if (!IsGrenade())
				pHLPlayer->SetHandsVisible(true);
		}
	}

	/*if (m_bSilenced)
	{
	const char* wModel = GetWorldModel();
	PrecacheModel(wModel);
	SetModel(wModel);
	}*/

	m_bWeaponBlockWall = false;
	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	m_flAccuracyPenalty = 0.0f;
	m_flNextInspectAnimation = 0;
	m_flNextLaserSwitchTime = gpGlobals->curtime;
	m_flNextSwitchTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flNextShotgunReload = gpGlobals->curtime;
	m_flNextAmmoRegen = gpGlobals->curtime;
	m_flNextAmmoRemove = gpGlobals->curtime;
	m_iShotgunReloadState = 0;

	if (GetWpnData().allowLoopSound)
	{
		StopLoopSound();
		DestroyLoopSound();
	}

	SetRemoveable(true);

	CBaseEntity *pOwner = GetOwnerEntity();
	if (pOwner && pOwner->IsNPC())
		WeaponManager_AmmoMod(this);
	else if (pOwner && !pOwner->IsNPC())
	{
		if (!Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_frag")
			&& !Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_hopwire")
			&& !Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_tripwire")
			&& !Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_molotov")
			&& !Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_smokegrenade")
			&& !Q_strstr(STRING(MAKE_STRING(this->GetClassname())), "weapon_flashbang"))
		{
			RemoveMuzzleFlash();
		}
	}

	//If non weapons - make hands visible
	if (pOwner && pOwner->IsPlayer())
	{
		if (GetWpnData().allowTwoDScope && IsScopeSighted())
		{
			OffScopeSight();//ToggleZoom(ToBasePlayer(GetOwner()), IsScopeSighted());
		}

		/*CHL2_Player *pPlayer = (CHL2_Player*)pOwner;
		if (pPlayer)
		pPlayer->ChangeBodyGroupState(true, false, false, true);*/
	}
	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags(SF_NORESPAWN);

	StopAnimation();
	StopFollowingEntity();
	SetMoveType(MOVETYPE_FLYGRAVITY);
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects(EF_NODRAW);
	FallInit();
	SetGroundEntity(NULL);

	SetPickableDelay(ownerIsPlayer);
	SetThink(&CBaseCombatWeapon::SetPickupTouch);
	SetTouch(NULL);

	if (hl2_episodic.GetBool())
	{
		RemoveSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP);
	}

	IPhysicsObject *pObj = VPhysicsGetObject();
	if (pObj != NULL)
	{
		AngularImpulse	angImp(200, 200, 200);
		pObj->AddVelocity(&vecVelocity, &angImp);
	}
	else
	{
		SetAbsVelocity(vecVelocity);
	}



	if (pOwner && pOwner->IsPlayer())
	{

		if (IsIronSighted())
			OffIronSight();

		if (IsScopeSighted())
			OffScopeSight();

		if (pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->m_hLaserMuzzleSprite != NULL)
		{
			StopLaserEffects((CBasePlayer*)pOwner);
		}

		if (pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->GetWpnData().allowLoopSound && m_bIsFiring)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;
	}

	SetNextThink(gpGlobals->curtime + 1.0f);
	SetOwnerEntity(NULL);
	SetOwner(NULL);


	// If we're not allowing to spawn due to the gamerules,
	// remove myself when I'm dropped by an NPC.
	if (pOwner && pOwner->IsNPC())
	{
		if (g_pGameRules->IsAllowedToSpawn(this) == false)
		{
			UTIL_Remove(this);
			return;
		}
	}

#endif
}

bool CBaseCombatWeapon::CanBePickedUpByNPCs()
{
	return GetWpnData().m_bCanBePickedUpByNPCs;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
	/*if (EnabledHoldingFire())
	return;*/

#if !defined( CLIENT_DLL )
	RemoveEffects(EF_ITEM_BLINK);

	if (pNewOwner->IsPlayer())
	{
		m_OnPlayerPickup.FireOutput(pNewOwner, this);

		// Play the pickup sound for 1st-person observers
		CRecipientFilter filter;
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *player = UTIL_PlayerByIndex(i);
			if (player && !player->IsAlive() && player->GetObserverMode() == OBS_MODE_IN_EYE)
			{
				filter.AddRecipient(player);
			}
		}
		if (filter.GetRecipientCount())
		{
			CBaseEntity::EmitSound(filter, pNewOwner->entindex(), "Player.PickupWeapon");
		}

		if (GetWpnData().m_bPickHint)
			UTIL_HudHintText(pNewOwner, GetWpnData().hintText);

		// Robin: We don't want to delete weapons the player has picked up, so 
		// clear the name of the weapon. This prevents wildcards that are meant 
		// to find NPCs finding weapons dropped by the NPCs as well.
		SetName(NULL_STRING);
	}
	else
	{
		m_OnNPCPickup.FireOutput(pNewOwner, this);
	}

#ifdef HL2MP
	HL2MPRules()->RemoveLevelDesignerPlacedObject(this);
#endif

	// Someone picked me up, so make it so that I can't be removed.
	SetRemoveable(false);
#endif
}

void CBaseCombatWeapon::MakeTracerHolded(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType)
{
	Vector vNewSrc = vecTracerSrc;
	int iEntIndex = entindex();

	if (g_pGameRules->IsMultiplayer())
	{
		iEntIndex = entindex();
	}

	int iAttachment = LookupAttachment(GetWpnData().MuzzleAttachmentNPC);

	if (iAttachment == -1)
		iAttachment = 1;

	tr_endPos = tr.endpos;

	AmmoTracer_t aT_t = (AmmoTracer_t)iTracerType;

	if (cvar->FindVar("oc_particle_tracers")->GetInt())
	{
		indexTN = random->RandomInt(0, GetWpnData().externalIndexTN);
		Q_snprintf(nameTN2, sizeof(nameTN2), "%s", GetWpnData().iMuzzleTracerNpcType[indexTN]);
		nameTN = nameTN2;
		UTIL_ParticleTracer(nameTN, vNewSrc, tr.endpos, iEntIndex, iAttachment, aT_t == TRACER_NONE);
	}
	else
	{
		float flVelocity = random->RandomFloat(GetWpnData().iOldMuzzleTracerVelocityMinNPC, GetWpnData().iOldMuzzleTracerVelocityMaxNPC);
		UTIL_Tracer(vNewSrc, tr.endpos, iEntIndex, iAttachment, flVelocity, true, "CommonTracer");
	}
}

static ConVar oc_test_bone_number("oc_test_bone_number", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

float PosX11;
float PosY11;
float PosZ11;

float DynX11;
float DynY11;
float DynZ11;


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecTracerSrc - 
//			&tr - 
//			iTracerType - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType)
{
	CBaseEntity *pOwner = GetOwner();

	if (pOwner == NULL)
	{
		if (EnabledHoldingFire())
			MakeTracerHolded(vecTracerSrc, tr, iTracerType);
		else
			BaseClass::MakeTracer(vecTracerSrc, tr, iTracerType);

		return;
	}

	//	const char *pszTracerName = GetTracerType();

	Vector vNewSrc = vecTracerSrc;

	int iEntIndex = pOwner->entindex();

	if (g_pGameRules->IsMultiplayer())
	{
		iEntIndex = entindex();
	}

	int iAttachment = GetTracerAttachment();

	tr_endPos = tr.endpos;

	if (pOwner->IsPlayer())
	{
		switch (iTracerType)
		{
		case TRACER_NONE:
		{
			return;
		}
		break;

		default:
		{

#ifndef CLIENT_DLL

			CEffectData	data;

			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

			int plrAttachment = GetWpnData().iAttachment;

			if (pPlayer && (thisType == TYPE_DUAL_PISTOLS || thisType == TYPE_DUAL_MACHINEGUNS))
			{
				if (iMuzzle == 2)
				{
					plrAttachment = GetWpnData().iAttachment;
				}
				else
				{
					if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
						plrAttachment = LookupAttachment(GetWpnData().MuzzleAttachment2);
					else
						plrAttachment = pPlayer->GetViewModel()->LookupAttachment(GetWpnData().MuzzleAttachment2);//pPlayer->GetViewModel()->LookupAttachment("muzzle1");
				}
			}

			if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
				data.m_nEntIndex = entindex();
			else
				data.m_nEntIndex = pPlayer->GetViewModel()->entindex();

			data.m_vOrigin = tr.endpos;

			data.m_nAttachmentIndex = plrAttachment;

			data.m_nDamageType = iTracerType;

			DispatchEffect("TracerEffect", data);
#endif
		}
		break;
		}
	}
	else
	{
		if (cvar->FindVar("oc_particle_tracers")->GetInt())
		{
			switch (iTracerType)
			{
			case TRACER_NONE:
			{
				return;
			}
			break;

			default:
			{
				indexTN = random->RandomInt(0, pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->GetWpnData().externalIndexTN);
				Q_snprintf(nameTN2, sizeof(nameTN2), "%s", pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->GetWpnData().iMuzzleTracerNpcType[indexTN]);
				nameTN = nameTN2;
				UTIL_ParticleTracer(nameTN, vNewSrc, tr.endpos, iEntIndex, iAttachment, iTracerType == iTracerType);
			}
			break;
			}
		}
		else
		{
			float flVelocity = random->RandomFloat(pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->GetWpnData().iOldMuzzleTracerVelocityMinNPC, pOwner->MyCombatCharacterPointer()->GetActiveWeapon()->GetWpnData().iOldMuzzleTracerVelocityMaxNPC);
			UTIL_Tracer(vNewSrc, tr.endpos, iEntIndex, iAttachment, flVelocity, true, "CommonTracer");
		}
	}
	//UTIL_Tracer(Muzzle, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 7000, false, "GaussTracer");
}

void CBaseCombatWeapon::GiveTo(CBaseEntity *pOther)
{
	DefaultTouch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: Default Touch function for player picking up a weapon (not AI)
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DefaultTouch(CBaseEntity *pOther)
{
	/*if (EnabledHoldingFire())
	return;*/

	EnableHoldingFire(false);

	if (cvar->FindVar("oc_allow_weapon_autopickup")->GetInt() == 0)	// BriJee OVR: Auto pick up weapon convar
	{
#if !defined( CLIENT_DLL )
		CBasePlayer *pPlayer = ToBasePlayer(pOther);
		if (!pPlayer)
			return;

		UTIL_HudHintText(pPlayer, "Valve_Hint_PickUp");
#endif
	}
	//else
	{


#if !defined( CLIENT_DLL )
		// Can't pick up dissolving weapons
		if (IsDissolving())
			return;

		if (!Pickable)
			return;

		// if it's not a player, ignore
		CBasePlayer *pPlayer = ToBasePlayer(pOther);
		if (!pPlayer)
			return;

		/*if (GetSpawnFlags() & SF_NPC_NO_PLAYER_PUSHAWAY)
		return;*/

		if (HasSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY))
			return;

		if (UTIL_ItemCanBeTouchedByPlayer(this, pPlayer))
		{
			// This makes sure the player could potentially take the object
			// before firing the cache interaction output. That doesn't mean
			// the player WILL end up taking the object, but cache interactions
			// are fired as soon as you prove you have found the object, not
			// when you finally acquire it.
			m_OnCacheInteraction.FireOutput(pOther, this);
		}

		if (HasSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP))
			return;

		if (pPlayer->BumpWeapon(this))
		{
			OnPickedUp(pPlayer);
			//Pickable = false;
		}
#endif
	}
}

//---------------------------------------------------------
// It's OK for base classes to override this completely 
// without calling up. (sjb)
//---------------------------------------------------------
bool CBaseCombatWeapon::ShouldDisplayAltFireHUDHint()
{
	if (m_iAltFireHudHintCount >= WEAPON_RELOAD_HUD_HINT_COUNT)
		return false;

	if (UsesSecondaryAmmo() && HasSecondaryAmmo())
	{
		return true;
	}

	if (!UsesSecondaryAmmo() && HasPrimaryAmmo())
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DisplayAltFireHudHint()
{
#if !defined( CLIENT_DLL )
	CFmtStr hint;
	hint.sprintf("#valve_hint_alt_%s", GetClassname());
	UTIL_HudHintText(GetOwner(), hint.Access());
	m_iAltFireHudHintCount++;
	m_bAltFireHudHintDisplayed = true;
	m_flHudHintMinDisplayTime = gpGlobals->curtime + MIN_HUDHINT_DISPLAY_TIME;
#endif//CLIENT_DLL
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::RescindAltFireHudHint()
{
#if !defined( CLIENT_DLL )
	Assert(m_bAltFireHudHintDisplayed);

	UTIL_HudHintText(GetOwner(), "");
	--m_iAltFireHudHintCount;
	m_bAltFireHudHintDisplayed = false;
#endif//CLIENT_DLL
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::ShouldDisplayReloadHUDHint()
{
	if (m_iReloadHudHintCount >= WEAPON_RELOAD_HUD_HINT_COUNT)
		return false;

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner != NULL && pOwner->IsPlayer() && UsesClipsForAmmo1() && m_iClip1 < (GetMaxClip1() / 2))
	{
		// I'm owned by a player, I use clips, I have less then half a clip loaded. Now, does the player have more ammo?
		if (pOwner)
		{
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
				return true;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DisplayReloadHudHint()
{
#if !defined( CLIENT_DLL )
	UTIL_HudHintText(GetOwner(), "valve_hint_reload");
	m_iReloadHudHintCount++;
	m_bReloadHudHintDisplayed = true;
	m_flHudHintMinDisplayTime = gpGlobals->curtime + MIN_HUDHINT_DISPLAY_TIME;
#endif//CLIENT_DLL
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::RescindReloadHudHint()
{
#if !defined( CLIENT_DLL )
	Assert(m_bReloadHudHintDisplayed);

	UTIL_HudHintText(GetOwner(), "");
	--m_iReloadHudHintCount;
	m_bReloadHudHintDisplayed = false;
#endif//CLIENT_DLL
}


void CBaseCombatWeapon::SetPickupTouch(void)
{
#if !defined( CLIENT_DLL )

	if (Pickable)
		SetTouch(&CBaseCombatWeapon::DefaultTouch);

	//auto classname = this->GetClassname();

	if (gpGlobals->maxClients > 1)
	{
		if (GetSpawnFlags() & SF_NORESPAWN)
		{
			SetThink(&CBaseEntity::SUB_Remove);
			SetNextThink(gpGlobals->curtime + 30.0f);
		}
	}
	else
	{
		if (!HasAnyAmmo())
		{
			Pickable = false;
			//SetSolidFlags
			//SetCollisionGroup(COLLISION_GROUP_DEBRIS);
			AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);
			SUB_StartFadeOut(15, false);
		}
		else
		{
			if (!Pickable && m_flNextPick < gpGlobals->curtime)
			{
				Pickable = true;
				SetTouch(&CBaseCombatWeapon::DefaultTouch);
			}
		}

		if (HasAnyAmmo() && m_flNextPick > gpGlobals->curtime)
			SetNextThink(gpGlobals->curtime + 1.0f);
	}

#endif
}

void CBaseCombatWeapon::SetPickableDelay(bool ownerIsPlayer)
{
	if (ownerIsPlayer)
	{
		Pickable = false;
		m_flNextPick = gpGlobals->curtime + 3.f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Become a child of the owner (MOVETYPE_FOLLOW)
//			disables collisions, touch functions, thinking
// Input  : *pOwner - new owner/operator
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Equip(CBaseCombatCharacter *pOwner)
{
	// Attach the weapon to an owner
	SetAbsVelocity(vec3_origin);
	RemoveSolidFlags(FSOLID_TRIGGER);

	bool drawBody = false;
	if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
	{
		CBasePlayer *pPlayer = ToBasePlayer(pOwner);
		if (pPlayer)
		{
			CBaseViewModel *vm = pPlayer->GetViewModel(0);

			if (vm)
			{
				if (vm)//->pPlayerModel)
				{
					drawBody = true;
					SetOwner(pOwner);
					SetOwnerEntity(vm);
					FollowEntity(vm);
					SetParent(vm, LookupAttachment("anim_attachment_RH"));
					/*SetOwner(pOwner);
					SetOwnerEntity(vm->pVial);
					FollowEntity(vm->pVial);
					SetParent(vm->pVial, LookupAttachment("anim_attachment_RH"));*/
					/*drawBody = true;
					SetOwner(pOwner);
					SetOwnerEntity(pOwner->GetBaseAnimating());
					FollowEntity(pOwner->GetBaseAnimating());
					SetParent(pOwner->GetBaseAnimating(), LookupAttachment("anim_attachment_RH"));*/
				}
			}
			else
			{
				FollowEntity(pOwner);
				SetOwner(pOwner);
				SetOwnerEntity(pOwner);
			}
		}
		else
		{
			FollowEntity(pOwner);
			SetOwner(pOwner);
			SetOwnerEntity(pOwner);
		}
	}
	else
	{
		FollowEntity(pOwner);
		SetOwner(pOwner);
		SetOwnerEntity(pOwner);
	}

	// Break any constraint I might have to the world.
	RemoveEffects(EF_ITEM_BLINK);

#if !defined( CLIENT_DLL )
	if (m_pConstraint != NULL)
	{
		RemoveSpawnFlags(SF_WEAPON_START_CONSTRAINED);
		physenv->DestroyConstraint(m_pConstraint);
		m_pConstraint = NULL;
	}
#endif


	m_flNextPrimaryAttack = gpGlobals->curtime;
	m_flNextSecondaryAttack = gpGlobals->curtime;
	SetTouch(NULL);
	SetThink(NULL);
#if !defined( CLIENT_DLL )
	VPhysicsDestroyObject();
#endif

	if (pOwner->IsPlayer())
	{
		if (drawBody)
		{
			PrecacheModel(cvar->FindVar("oc_playermodel_suit")->GetString());
			SetModel(cvar->FindVar("oc_playermodel_suit")->GetString());
			int bg_rl = FindBodygroupByName("right_leg");
			int bg_ll = FindBodygroupByName("left_leg");
			int bg_t = FindBodygroupByName("torso");
			int bg_h = FindBodygroupByName("head");
			SetBodygroup(bg_rl, 1);
			SetBodygroup(bg_ll, 1);
			SetBodygroup(bg_t, 1);
			SetBodygroup(bg_h, 1);
		}
		else
			SetModel(GetViewModel());
	}
	else
	{
		// Make the weapon ready as soon as any NPC picks it up.
		m_flNextPrimaryAttack = gpGlobals->curtime;
		m_flNextSecondaryAttack = gpGlobals->curtime;
		SetModel(GetWorldModel());
	}
}

void CBaseCombatWeapon::SetActivity(Activity act, float duration)
{
	//Adrian: Oh man...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel(GetWorldModel());
#endif

	int sequence = SelectWeightedSequence(act);

	// FORCE IDLE on sequences we don't have (which should be many)
	if (sequence == ACTIVITY_NOT_AVAILABLE)
		sequence = SelectWeightedSequence(ACT_VM_IDLE);

	//Adrian: Oh man again...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel(GetViewModel());
#endif

	if (sequence != ACTIVITY_NOT_AVAILABLE)
	{
		SetSequence(sequence);

		SetActivity(act);
		SetCycle(0);
		ResetSequenceInfo();

		if (duration > 0)
		{
			// FIXME: does this even make sense in non-shoot animations?
			m_flPlaybackRate = SequenceDuration(sequence) / duration;
			m_flPlaybackRate = MIN(m_flPlaybackRate, 12.0);  // FIXME; magic number!, network encoding range
		}
		else
		{
			m_flPlaybackRate = 1.0;
		}
	}
}

//====================================================================================
// WEAPON CLIENT HANDLING
//====================================================================================
int CBaseCombatWeapon::UpdateClientData(CBasePlayer *pPlayer)
{
	int iNewState = WEAPON_IS_CARRIED_BY_PLAYER;

	if (pPlayer->GetActiveWeapon() == this)
	{
		if (pPlayer->m_fOnTarget)
		{
			iNewState = WEAPON_IS_ONTARGET;
		}
		else
		{
			iNewState = WEAPON_IS_ACTIVE;
		}
	}
	else
	{
		iNewState = WEAPON_IS_CARRIED_BY_PLAYER;
	}

	if (m_iState != iNewState)
	{
		int iOldState = m_iState;
		m_iState = iNewState;
		OnActiveStateChanged(iOldState);
	}
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModelIndex(int index)
{
	Assert(index >= 0 && index < MAX_VIEWMODELS);
	m_nViewModelIndex = index;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iActivity - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SendViewModelAnim(int nSequence)
{
#if defined( CLIENT_DLL )
	if (!IsPredicted())
		return;
#endif

	if (nSequence < 0)
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *vm = pOwner->GetViewModel(m_nViewModelIndex, false);

	if (vm == NULL)
		return;

	SetViewModel();
	Assert(vm->ViewModelIndex() == m_nViewModelIndex);
	vm->SendViewModelMatchingSequence(nSequence);
}

float CBaseCombatWeapon::GetViewModelSequenceDuration()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
	{
		Assert(false);
		return 0;
	}

	CBaseViewModel *vm = pOwner->GetViewModel(m_nViewModelIndex);
	if (vm == NULL)
	{
		Assert(false);
		return 0;
	}

	SetViewModel();
	Assert(vm->ViewModelIndex() == m_nViewModelIndex);
	return vm->SequenceDuration();
}

bool CBaseCombatWeapon::IsViewModelSequenceFinished(void)
{
	// These are not valid activities and always complete immediately
	if (GetActivity() == ACT_RESET || GetActivity() == ACT_INVALID)
		return true;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
	{
		Assert(false);
		return false;
	}

	CBaseViewModel *vm = pOwner->GetViewModel(m_nViewModelIndex);
	if (vm == NULL)
	{
		Assert(false);
		return false;
	}

	return vm->IsSequenceFinished();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModel()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;
	CBaseViewModel *vm = pOwner->GetViewModel(m_nViewModelIndex, false);
	if (vm == NULL)
		return;
	Assert(vm->ViewModelIndex() == m_nViewModelIndex);
	vm->SetWeaponModel(GetViewModel(m_nViewModelIndex), this);
}

//-----------------------------------------------------------------------------
// Purpose: Set the desired activity for the weapon and its viewmodel counterpart
// Input  : iActivity - activity to play
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SendWeaponAnim(int iActivity)
{
#ifdef USES_ECON_ITEMS
	iActivity = TranslateViewmodelHandActivity((Activity)iActivity);
#endif		
	// NVNT notify the haptics system of this weapons new activity
#ifdef WIN32
#ifdef CLIENT_DLL
	if (prediction->InPrediction() && prediction->IsFirstTimePredicted())
#endif
#ifndef _X360
		HapticSendWeaponAnim(this, iActivity);
#endif
#endif
	//For now, just set the ideal activity and be done with it
	return SetIdealActivity((Activity)iActivity);
}

//====================================================================================
// WEAPON SELECTION
//====================================================================================

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAnyAmmo(void)
{

	/*if (cvar->FindVar("oc_weapon_disable_autoswitch")->GetInt() == 1)
	{
	return true;
	}
	else*/
	{

		// If I don't use ammo of any kind, I can always fire
		if (!UsesPrimaryAmmo() && !UsesSecondaryAmmo())
			return true;

		// Otherwise, I need ammo of either type
		return (HasPrimaryAmmo() || HasSecondaryAmmo());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasPrimaryAmmo(void)
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if (UsesClipsForAmmo1())
	{
		if (m_iClip1 > 0)
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if (pOwner)
	{
		if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
			return true;
	}
	else
	{
		// No owner, so return how much primary ammo I have along with me.
		if (GetPrimaryAmmoCount() > 0)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasSecondaryAmmo(void)
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if (UsesClipsForAmmo2())
	{
		if (m_iClip2 > 0)
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if (pOwner)
	{
		if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses primary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesPrimaryAmmo(void)
{
	if (m_iPrimaryAmmoType < 0)
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses secondary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesSecondaryAmmo(void)
{
	if (m_iSecondaryAmmoType < 0)
		return false;
	return true;
}

bool CBaseCombatWeapon::IsGrenade()
{
	if (thisType == TYPE_GRENADE)
	{
		if (cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt())
			return true;
	}
	return false;// IsEffectActive(EF_NODRAW);
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide weapon and corresponding view model if any
// Input  : visible - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponVisible(bool visible)
{
	CBaseViewModel *vm = NULL;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		vm = pOwner->GetViewModel(m_nViewModelIndex);
	}

	if (visible)
	{
		RemoveEffects(EF_NODRAW);
		if (vm)
		{
			vm->RemoveEffects(EF_NODRAW);
		}
	}
	else
	{
		AddEffects(EF_NODRAW);
		if (vm)
		{
			vm->AddEffects(EF_NODRAW);
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsWeaponVisible(void)
{
	CBaseViewModel *vm = NULL;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		vm = pOwner->GetViewModel(m_nViewModelIndex);
		if (vm)
			return (!vm->IsEffectActive(EF_NODRAW));
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::ReloadOrSwitchWeapons(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	Assert(pOwner);

	//m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if (!GetWpnData().allowAmmoRegen && !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime)
	{
		// weapon isn't useable, switch.
		if (((GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false) && (g_pGameRules->SwitchToNextBestWeapon(pOwner, this)))
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if (UsesClipsForAmmo1() && !AutoFiresFullClip() &&
			!Clip1() &&
			GetActivity() != ACT_VM_ATTACH_SILENCER &&
			GetActivity() != ACT_VM_DETACH_SILENCER &&
			(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false &&
			m_flNextPrimaryAttack < gpGlobals->curtime &&
			m_flNextSecondaryAttack < gpGlobals->curtime)
		{
			if (GetWpnData().animData[m_bFireMode].PumpAnimation != 0)
			{
				if (GetWpnData().allowLoopSound && m_bIsFiring)
					StopLoopSound();

				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				/*if (GetWpnData().animData[m_bFireMode].UseHalfFullMagAnimations == 1)
				{
					if (Clip1() == 1)
					{
						if (DefaultReload(GetMaxClip1(), GetMaxClip2(), GetWpnData().animData[m_bFireMode].ReloadMidEmptyAnimation))
							return true;
					}
					else if (Clip1() < 1)
					{
						if (GetWpnData().animData[m_bFireMode].PumpAnimation == 0)
						{
							if (DefaultReload(GetMaxClip1(), GetMaxClip2(), GetWpnData().animData[m_bFireMode].ReloadFullEmptyAnimation))
								return true;
						}
						else
						{
							if (DefaultReloadEmpty(GetMaxClip1(), GetMaxClip2(), GetWpnData().animData[m_bFireMode].ReloadFullEmptyAnimation))
								return true;
						}
					}
				}
				else*/
				//{
				if (GetWpnData().animData[m_bFireMode].PumpAnimation == 0)
				{
					if (DefaultReload(GetMaxClip1(), GetMaxClip2(), GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation))
						return true;
				}
				else
				{
					if (DefaultReloadEmpty(GetMaxClip1(), GetMaxClip2(), GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation))
						return true;
				}
				//}
			}
			else
			{
				if (GetWpnData().allowLoopSound && m_bIsFiring)
					StopLoopSound();

				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				if (m_flNextSwitchTime < gpGlobals->curtime)
				{
					if (Reload())
						return true;
				}
			}
		}
		else if ((IsNearWall() || GetOwnerIsRunning())
			&& UsesClipsForAmmo1() &&// && !AutoFiresFullClip() &&
			!Clip1() &&
			GetActivity() != ACT_VM_ATTACH_SILENCER &&
			GetActivity() != ACT_VM_DETACH_SILENCER &&
			(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false &&
			(GetWpnData().animData[m_bFireMode].ReloadWaitWhileAnimation ? IsViewModelSequenceFinished() : true))
		{
			// if we're successfully reloading, we're done
			if (GetWpnData().allowLoopSound && m_bIsFiring)
				StopLoopSound();

			if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
				m_bIsInPrimaryAttack = false;

			if (m_flNextSwitchTime < gpGlobals->curtime)
			{
				if (Reload())
					return true;
			}
		}
	}

	return false;
}

bool CBaseCombatWeapon::GetOwnerIsRunning()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	bool isInSecAttack = (GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary1 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary2 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary3 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary4 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryLast ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryCharging ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted1 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted2 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted3 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted4 ||
		GetActivity() == ACT_VM_SECONDARYATTACK);

	if (IsInReload() ||
		isInSecAttack)
		return false;

	return (pOwner && pOwner->IsRunning());
}

Activity CBaseCombatWeapon::GetHolsterActivity()
{
	if (!Clip1())
	{
		if (GetWpnData().animData[m_bFireMode].HolsterFullEmpty)
			return GetWpnData().animData[m_bFireMode].HolsterFullEmpty;
		else
		{
			if (GetWpnData().animData[m_bFireMode].HolsterAnimation != ACT_RESET)
				return GetWpnData().animData[m_bFireMode].HolsterAnimation;
			else
				return ACT_VM_HOLSTER;
		}
	}
	else
	{
		if (GetWpnData().animData[m_bFireMode].HolsterAnimation != ACT_RESET)
			return GetWpnData().animData[m_bFireMode].HolsterAnimation;
		else
			return ACT_VM_HOLSTER;
	}

	return ACT_VM_HOLSTER;
}

bool CBaseCombatWeapon::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt)
{
	if (GetWpnData().allowLoopSound)
	{
		PrecacheScriptSound(GetWpnData().ShootLoopingSound);
		PrecacheScriptSound(GetWpnData().EndLoopingSound);

		if (GetWpnData().allowLoopSilencerSound)
		{
			PrecacheScriptSound(GetWpnData().ShootLoopingSilencerSound);
			PrecacheScriptSound(GetWpnData().EndLoopingSilencerSound);
		}
	}

	if (!HasAnyAmmo() && AllowsAutoSwitchFrom())
		return false;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		// Dead men deploy no weapons
		if (pOwner->IsAlive() == false)
			return false;

		g_ShowWeapon.UpdateWeaponActivites(pOwner->GetActiveWeapon(), true);

		pOwner->SetAnimationExtension(szAnimExt);

		SetViewModel();
		SendWeaponAnim(iActivity);

		pOwner->SetNextAttack(gpGlobals->curtime + GetViewModelSequenceDuration());

		if (GetWpnData().enableLaser)
			EnableLaserInterrupt = true;
		m_bZoomLevel = GetWpnData().maxScopeFov;

#ifndef CLIENT_DLL
		CHL2_Player *pHLPlayer = static_cast<CHL2_Player*>(ToBasePlayer(GetOwner()));
		if (pHLPlayer)
		{
			if (!IsGrenade())
				pHLPlayer->SetHandsVisible(false);
			else
				pHLPlayer->SetHandsVisible(true);
		}
#endif
		float flSequenceDuration = 0;
		CBaseCombatWeapon *pActive = GetOwner()->GetActiveWeapon();
		if (pActive && pActive->GetActivity() == GetHolsterActivity())
		{
			//cvar->FindVar("oc_weapon_holster_code")->SetValue(0);
			flSequenceDuration = 0.5f;//pActive->SequenceDuration();	// BriJee OVR: Must be always half of second, no matter what anim using.
		}

		if (pActive && pActive->GetActivity() == GetHolsterActivity())	// BriJee OVR: NEW ACTIVITY in action
		{
			//cvar->FindVar("oc_weapon_holster_code")->SetValue(0);
			flSequenceDuration = 0.5f;
		}

		g_ShowWeapon.SetShowWeapon(this, iActivity, flSequenceDuration);

		// Can't shoot again until we've finished deploying
		m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

		SetupWeapon(pOwner);

#ifndef CLIENT_DLL
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer)
		{
			pPlayer->SetupWeaponBloodOverlay(m_bShouldDrawWeaponBloodOverlay);
			pPlayer->SetupWeaponBloodOverlayFrame(m_iWeaponBloodOverlayDetailFrame);
		}
		// Cancel any pending hide events
		g_EventQueue.CancelEventOn(this, "HideWeapon");
#endif
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flHudHintMinDisplayTime = 0;

	m_bAltFireHudHintDisplayed = false;
	m_bReloadHudHintDisplayed = false;
	m_flHudHintPollTime = gpGlobals->curtime + 5.0f;

	SetWeaponVisible(true);

	SetContextThink(NULL, 0, HIDEWEAPON_THINK_CONTEXT);

	return true;
}

void CBaseCombatWeapon::SetVMBodyGroup(int iGroup, int iValue)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();
	if (pViewModel)
	{
		pViewModel->SetBodygroup(iGroup, iValue);
	}
}

void CBaseCombatWeapon::SetSkin(int skinNum, bool noSuit)
{
	//#ifndef CLIENT_DLL

	if (!noSuit)
	{
		iVMSkin = skinNum;

		m_nSkin = iVMSkin;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (pOwner && pOwner->GetActiveWeapon())
	{
		if (pOwner->GetActiveWeapon() == this)
		{
			CBaseViewModel *pViewModel = pOwner->GetViewModel();

			if (pViewModel)
			{
				pViewModel->m_nSkin = noSuit ? 1 : iVMSkin;

				CBaseAnimating *pParent = pViewModel->GetBaseAnimating();
				if (pParent)
					pParent->m_nSkin = noSuit ? 1 : iVMSkin;
			}
		}
	}

	//#endif

}

void CBaseCombatWeapon::RandomizeSkin(int iRange)
{
	//#ifndef CLIENT_DLL
	SetSkin(random->RandomInt(0, iRange));
	//#endif
}

void CBaseCombatWeapon::WeaponEmitSound(const char *m_sName, const Vector &m_vOrigin)
{
	CPASAttenuationFilter filter(this);

	EmitSound_t ep;
	ep.m_nChannel = CHAN_WEAPON;
	ep.m_pSoundName = m_sName;
	ep.m_flVolume = 1;
	ep.m_pOrigin = &m_vOrigin;
	ep.m_SoundLevel = SNDLVL_NORM;

	EmitSound(filter, entindex(), ep);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Deploy()
{
	MDLCACHE_CRITICAL_SECTION();

#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner && pOwner->GetActiveWeapon())
	{
		if (cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? pOwner->grenadeState <= 0 && thisType == TYPE_GRENADE : false)
			return false;
	}
#endif

	return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix());
}
bool CBaseCombatWeapon::FirstDeploy(int iActivity)
{
	MDLCACHE_CRITICAL_SECTION();

#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner && pOwner->GetActiveWeapon())
	{
		if (cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? pOwner->grenadeState <= 0 && thisType == TYPE_GRENADE : false)
			return false;
	}
#endif

	return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), iActivity, (char*)GetAnimPrefix());
}

Activity CBaseCombatWeapon::GetDrawActivity(void)
{
	if (!Clip1())
	{
		if (GetWpnData().animData[m_bFireMode].DrawEmptyAnimation)
			return GetWpnData().animData[m_bFireMode].DrawEmptyAnimation;
		else
		{
			if (GetWpnData().animData[m_bFireMode].DrawAnimation != ACT_RESET)
				return GetWpnData().animData[m_bFireMode].DrawAnimation;
			else
				return ACT_VM_DRAW;
		}
	}
	else
	{
		if (GetWpnData().animData[m_bFireMode].DrawAnimation != ACT_RESET)
			return GetWpnData().animData[m_bFireMode].DrawAnimation;
		else
			return ACT_VM_DRAW;
	}

	return ACT_VM_DRAW;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (!GetOwner())		// BriJee OVR : new functions crash fix
		return false;

	MDLCACHE_CRITICAL_SECTION();

	if (GetOwner() && GetOwner()->IsPlayer())
	{

		//MikeD Set all custom skins to 0
		/*iVMSkin = 0;
		SetSkin(iVMSkin);*/

		if (GetWpnData().allowTwoDScope && IsScopeSighted())
		{
			OffScopeSight();//ToggleZoom(ToBasePlayer(GetOwner()), IsScopeSighted());
		}
	}

	if (m_bWeaponBlockWall)
		m_bWeaponBlockWall = false;


	//if (GetOwner()->IsNPC())	// BriJee OVR: Npc sections split, no need to read player finctions as Npc
	if (!GetOwner()->IsPlayer())
	{
		m_bInReload = false;
		m_bFiringWholeClip = false;

		// kill any think functions
		SetThink(NULL);

		SendWeaponAnim(GetHolsterActivity());

		// Some weapon's don't have holster anims yet, so detect that
		float flSequenceDuration = 0;
		if (GetActivity() == GetHolsterActivity())
		{
			flSequenceDuration = GetViewModelSequenceDuration();
		}

		CBaseCombatCharacter *pOwner = GetOwner();
		if (pOwner)
		{
			pOwner->SetNextAttack(gpGlobals->curtime + flSequenceDuration);
		}

		// If we don't have a holster anim, hide immediately to avoid timing issues
		if (!flSequenceDuration)
		{
			SetWeaponVisible(false);
		}
		else
		{
			// Hide the weapon when the holster animation's finished
			SetContextThink(&CBaseCombatWeapon::HideThink, gpGlobals->curtime + flSequenceDuration, HIDEWEAPON_THINK_CONTEXT);
		}

		// if we were displaying a hud hint, squelch it.
		if (m_flHudHintMinDisplayTime && gpGlobals->curtime < m_flHudHintMinDisplayTime)
		{
			if (m_bAltFireHudHintDisplayed)
				RescindAltFireHudHint();

			if (m_bReloadHudHintDisplayed)
				RescindReloadHudHint();
		}

		return true;
	}
	else
	{
		// cancel any reload in progress.
		m_bInReload = false;

		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner || !pOwner->GetActiveWeapon())
			return false;

		if (GetOwner()->IsPlayer())
			RemoveMuzzleFlash();

		if (GetWpnData().allowLoopSound)
		{
			StopLoopSound();
			DestroyLoopSound();
		}

		if (thisType == TYPE_SHOTGUN)
			m_iShotgunReloadState = 0;

		if (GetWpnData().allowLoopSound && m_bIsFiring)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;

		if (GetWpnData().enableLaser)
			EnableLaserInterrupt = true;
#ifndef CLIENT_DLL	
		if (m_hLaserMuzzleSprite != NULL)
		{
			StopLaserEffects(pOwner);
		}
#endif

		StopParticleEffects(pOwner->GetViewModel());

		m_bFiringWholeClip = false;

		// kill any think functions
		SetThink(NULL);

		if (GetWpnData().WeaponModes.Count() > 0)
			allObjectWeapons.AddToTail(pOwner->GetActiveWeapon());
		if (allObjectWeapons.HasElement(pOwner->GetActiveWeapon()))
		{
			allObjectWeapons.Element(allObjectWeapons.Find(pOwner->GetActiveWeapon()))->m_bFireMode = m_bFireMode;
			allObjectWeapons.Element(allObjectWeapons.Find(pOwner->GetActiveWeapon()))->m_bFireModeCounter = m_bFireModeCounter;
		}

		//if (cvar->FindVar("oc_weapon_holster")->GetInt() == 1)
		//SendWeaponAnim(GetHolsterActivity());

		// Some weapon's don't have holster anims yet, so detect that
		float flSequenceDuration = 0;
		if (GetActivity() == GetHolsterActivity())
		{
			flSequenceDuration = 0.5f; //SequenceDuration();	// BriJee OVR: Must be always half of second, no matter what anim using.
			//flSequenceDuration = SequenceDuration();
		}

		if (pOwner)
		{
			pOwner->SetNextAttack(gpGlobals->curtime + flSequenceDuration);
		}


		// If we don't have a holster anim, hide immediately to avoid timing issues
		if (!flSequenceDuration)
		{
			SetWeaponVisible(false);
		}
		else
		{
			// Hide the weapon when the holster animation's finished
			SetContextThink(&CBaseCombatWeapon::HideThink, gpGlobals->curtime + flSequenceDuration, HIDEWEAPON_THINK_CONTEXT);
		}

		// if we were displaying a hud hint, squelch it.
		if (m_flHudHintMinDisplayTime && gpGlobals->curtime < m_flHudHintMinDisplayTime)
		{
			if (m_bAltFireHudHintDisplayed)
				RescindAltFireHudHint();

			if (m_bReloadHudHintDisplayed)
				RescindReloadHudHint();
		}

		return true;
	}

}

#ifdef CLIENT_DLL

void CBaseCombatWeapon::BoneMergeFastCullBloat(Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs) const
{
	// The default behavior pushes it out by BONEMERGE_FASTCULL_BBOX_EXPAND in all directions, but we can do better
	// since we know the weapon will never point behind him.

	localMaxs.x += 20;	// Leaves some space in front for long weapons.

	localMins.y -= 20;	// Fatten it to his left and right since he can rotate that way.
	localMaxs.y += 20;

	localMaxs.z += 15;	// Leave some space at the top.
}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::InputHideWeapon(inputdata_t &inputdata)
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if (GetOwner() && GetOwner()->GetActiveWeapon() == this)
	{
		SetWeaponVisible(false);
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::HideThink(void)
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if (GetOwner() && GetOwner()->GetActiveWeapon() == this)
	{
		SetWeaponVisible(false);
	}
}

bool CBaseCombatWeapon::CanReload(void)
{
	if (m_flNextSecondaryAttack >= gpGlobals->curtime)
		return false;

	if (AutoFiresFullClip() && m_bFiringWholeClip)
	{
		return false;
	}

	return true;
}

#if defined ( TF_CLIENT_DLL ) || defined ( TF_DLL )
//-----------------------------------------------------------------------------
// Purpose: Anti-hack
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AddToCritBucket(float flAmount)
{
	float flCap = tf_weapon_criticals_bucket_cap.GetFloat();

	// Regulate crit frequency to reduce client-side seed hacking
	if (m_flCritTokenBucket < flCap)
	{
		// Treat raw damage as the resource by which we add or subtract from the bucket
		m_flCritTokenBucket += flAmount;
		m_flCritTokenBucket = Min(m_flCritTokenBucket, flCap);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Anti-hack
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsAllowedToWithdrawFromCritBucket(float flDamage)
{
	// Note: If we're in this block of code, the assumption is that the
	// seed said we should grant a random crit.  If allowed, the cost
	// will be deducted here.

	// Track each seed request - in cases where a player is hacking, we'll 
	// see a silly ratio.
	m_nCritSeedRequests++;

	// Adjust token cost based on the ratio of requests vs granted, except
	// melee, which crits much more than ranged (as high as 60% chance)
	float flMult = (IsMeleeWeapon()) ? 0.5f : RemapValClamped(((float)m_nCritSeedRequests / (float)m_nCritChecks), 0.1f, 1.f, 1.f, 3.f);

	// Would this take us below our limit?
	float flCost = (flDamage * TF_DAMAGE_CRIT_MULTIPLIER) * flMult;
	if (flCost > m_flCritTokenBucket)
		return false;

	// Withdraw
	RemoveFromCritBucket(flCost);

	float flBottom = tf_weapon_criticals_bucket_bottom.GetFloat();
	if (m_flCritTokenBucket < flBottom)
		m_flCritTokenBucket = flBottom;

	return true;
}
#endif // TF_DLL

bool CBaseCombatWeapon::IsNearWall()
{
	if (!cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt())
		return false;

	/*if (IsScopeSighted())
	return false;*/

	float weaponDist = GetWpnData().DistanceToWall;

	return (distanceToWall < weaponDist) && clipNearWall;
}


void CBaseCombatWeapon::WallClip(CBasePlayer *pPlayer)
{
	if (m_flNextWallClipTime > gpGlobals->curtime)
		return;

	trace_t tr, m_trHit;
	Vector vecDir, vecStop, vecSrc, vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	QAngle Angle = pPlayer->EyeAngles();

	bool isInSecAttack = (GetActivity() != ACT_RESET) && (GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary1 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary2 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary3 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondary4 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryLast ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryCharging ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted1 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted2 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted3 ||
		GetActivity() == GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted4 ||
		GetActivity() == ACT_VM_SECONDARYATTACK);

	if (Angle.x > 35.f ||
		IsInReload() ||
		isInSecAttack ||
		m_flNextInspectAnimation > gpGlobals->curtime)
	{
		clipNearWall = false;
		return;
	}

	Vector res = pPlayer->EyePosition();
	res += vForward;
	res += vRight;
	res += vUp;
	vecSrc = res;
	AngleVectors(Angle, &vecDir);
	vecStop = vecSrc + vecDir * MAX_TRACE_LENGTH;
	//UTIL_TraceLine(vecSrc, vecStop, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	UTIL_TraceHull(vecSrc, vecStop, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	distanceToWall = tr.endpos.DistTo(vecSrc);

	if (tr.m_pEnt)
	{
		clipNearWall = (!tr.m_pEnt->IsNPC() && !(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))) &&
			!FClassnameIs(tr.m_pEnt, "physics_prop_ragdoll") && !FClassnameIs(tr.m_pEnt, "prop_ragdoll") && !FClassnameIs(tr.m_pEnt, "prop_ragdoll_attached") &&
			distanceToWall <= GetWpnData().DistanceToWall;
	}
	else
	{
		clipNearWall = (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX)) && distanceToWall <= GetWpnData().DistanceToWall);
	}

	m_flNextWallClipTime = gpGlobals->curtime + 0.1f;
}

void CBaseCombatWeapon::UpdatePenaltyTime(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	// Check our penalty time decay
	if (((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flSoonestPrimaryAttack < gpGlobals->curtime))
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp(m_flAccuracyPenalty, 0.0f, GetWpnData().mode_accuracy_maximum_penalty_time);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemPreFrame(void)
{
	if ((m_bFireMode == 1) || (m_bFireMode == 0))
	{
		UpdatePenaltyTime();
	}

	MaintainIdealActivity();

#ifndef CLIENT_DLL
#ifndef HL2_EPISODIC
	if (IsX360())
#endif
	{
		// If we haven't displayed the hint enough times yet, it's time to try to 
		// display the hint, and the player is not standing still, try to show a hud hint.
		// If the player IS standing still, assume they could change away from this weapon at
		// any second.
		if ((!m_bAltFireHudHintDisplayed || !m_bReloadHudHintDisplayed) && gpGlobals->curtime > m_flHudHintMinDisplayTime && gpGlobals->curtime > m_flHudHintPollTime && GetOwner() && GetOwner()->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer*)(GetOwner());

			if (pPlayer && pPlayer->GetStickDist() > 0.0f)
			{
				// If the player is moving, they're unlikely to switch away from the current weapon
				// the moment this weapon displays its HUD hint.
				if (ShouldDisplayReloadHUDHint())
				{
					DisplayReloadHudHint();
				}
				else if (ShouldDisplayAltFireHUDHint())
				{
					DisplayAltFireHudHint();
				}
			}
			else
			{
				m_flHudHintPollTime = gpGlobals->curtime + 2.0f;
			}
		}
	}
#endif
}


float nextFireThreshold = 0.5f;

void CBaseCombatWeapon::SwitchFiringModes(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (!GetOwnerIsRunning() &&
		!IsNearWall())
		//!cvar->FindVar("oc_state_near_wall_standing")->GetInt())
	{
		if (m_bFireModesSize > 1 && !(pOwner->m_nButtons & IN_ATTACK) && !(pOwner->m_nButtons & IN_ATTACK2) && (GetActivity() != GetWpnData().animData[m_bFireMode].WeaponSwitchAnim))
		{

			if ((pOwner->m_nButtons & IN_SWITCHMODE) && !switched && m_flNextSwitchTime <= gpGlobals->curtime)
			{

				m_bFireModeCounter++;

				if (m_bFireModeCounter > GetWpnData().WeaponModes.Count() - 1)
					m_bFireModeCounter = 0;

				m_bFireMode = GetWpnData().WeaponModes.Element(m_bFireModeCounter);

				if (allObjectWeapons.HasElement(pOwner->GetActiveWeapon()))
				{
					allObjectWeapons.Element(allObjectWeapons.Find(pOwner->GetActiveWeapon()))->m_bFireMode = m_bFireMode;
					allObjectWeapons.Element(allObjectWeapons.Find(pOwner->GetActiveWeapon()))->m_bFireModeCounter = m_bFireModeCounter;
				}


				switch (m_bFireMode)
				{
				case(1) :
#ifndef CLIENT_DLL
					Q_strncpy(nameOfFireMode.GetForModify(), "FireModeType1", sizeof(nameOfFireMode));
					//UTIL_HudHintText(pOwner, "Fire mode: semi-automatic");
#endif
					break;
				case(2) :
#ifndef CLIENT_DLL
					Q_strncpy(nameOfFireMode.GetForModify(), "FireModeType2", sizeof(nameOfFireMode));
					//UTIL_HudHintText(pOwner, "Fire mode: automatic");
#endif
					break;
				case(3) :
#ifndef CLIENT_DLL
					Q_strncpy(nameOfFireMode.GetForModify(), "FireModeType3", sizeof(nameOfFireMode));
					//UTIL_HudHintText(pOwner, "Fire mode: burst");
#endif
					break;
				default:
					break;
				}

				switch (m_bFireMode)
				{
				case(1) :
				{
					if (Q_strcmp(GetWpnData().WeaponSwitchingMode1Sound, "None") != 0)
						EmitSound(GetWpnData().WeaponSwitchingMode1Sound);

					m_flNextPrimaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode1Sound, STRING(GetModelName()));
					m_flNextSecondaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode1Sound, STRING(GetModelName()));
					m_flNextSwitchTime = m_flNextPrimaryAttack;
				}
						break;
				case(2) :
				{
					if (Q_strcmp(GetWpnData().WeaponSwitchingMode2Sound, "None") != 0)
						EmitSound(GetWpnData().WeaponSwitchingMode2Sound);

					m_flNextPrimaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode2Sound, STRING(GetModelName()));
					m_flNextSecondaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode2Sound, STRING(GetModelName()));
					m_flNextSwitchTime = m_flNextPrimaryAttack;
				}
						break;
				case(3) :
				{
					if (Q_strcmp(GetWpnData().WeaponSwitchingMode3Sound, "None") != 0)
						EmitSound(GetWpnData().WeaponSwitchingMode3Sound);

					m_flNextPrimaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode3Sound, STRING(GetModelName()));
					m_flNextSecondaryAttack = gpGlobals->curtime + GetSoundDuration(GetWpnData().WeaponSwitchingMode3Sound, STRING(GetModelName()));
					m_flNextSwitchTime = m_flNextPrimaryAttack;
				}
						break;
				default:
					break;
				}

				g_ShowWeapon.UpdateWeaponActivites(pOwner->GetActiveWeapon(), true);

#ifndef CLIENT_DLL
				if (GetWpnData().animData[m_bFireMode].WeaponSwitchAnim != 0)
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].WeaponSwitchAnim);
					m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
					m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
					m_flNextSwitchTime = m_flNextPrimaryAttack;
					m_flNextInspectAnimation = gpGlobals->curtime + GetViewModelSequenceDuration();
				}
#endif
				switched = true;
			}
			else if (!(pOwner->m_nButtons & IN_SWITCHMODE) && switched)
				switched = false;
		}
	}
}

void CBaseCombatWeapon::SwitchLaserGuidance(CBasePlayer *pOwner)
{
	if (!GetWpnData().enableLaser || GetActivity() == GetDrawActivity())
		return;

#ifndef CLIENT_DLL
	if (m_bInReload ||
		m_bReloadComplete ||
		IsNearWall() ||
		WeaponShouldBeLowered() ||
		GetActivity() == GetWpnData().animData[m_bFireMode].InspectAnimation ||
		GetActivity() == GetWpnData().animData[m_bFireMode].SwingAnim ||
		GetOwnerIsRunning())
		EnableLaserInterrupt = true;
	else
		EnableLaserInterrupt = !EnableLaser;
#endif

	if ((GetWpnData().useAsSecondaryAttack ? (pOwner->m_nButtons & IN_ATTACK2) : (pOwner->m_nButtons & IN_LASERSWITCH)) && !_enabledLaser && m_flNextLaserSwitchTime <= gpGlobals->curtime)
	{
		if (!EnableLaser)
			EnableLaser = true;
		else if (EnableLaser)
			EnableLaser = false;

		_enabledLaser = true;
	}
	if (GetWpnData().useAsSecondaryAttack ? (pOwner->m_afButtonReleased & IN_ATTACK2) : (pOwner->m_afButtonReleased & IN_LASERSWITCH))//((pOwner->m_afButtonReleased & IN_LASERSWITCH))
	{
		_enabledLaser = false;
	}

	if (EnableLaser && EnableLaserInterrupt == false)
	{
#ifndef CLIENT_DLL
		if (m_bInReload || m_bReloadComplete)
			StopLaserEffects(pOwner);
		else
			StartLaserEffects(pOwner);
#endif
	}
	else if (EnableLaser && EnableLaserInterrupt == true)
	{
#ifndef CLIENT_DLL		
		StopLaserEffects(pOwner);
#endif
	}
	else if (!EnableLaser)
	{
#ifndef CLIENT_DLL		
		StopLaserEffects(pOwner);
#endif
	}

}
void CBaseCombatWeapon::StartLaserEffects(CBasePlayer *pOwner)
{
	if (!GetWpnData().enableLaser || EnableLaserInterrupt)
		return;

#ifndef CLIENT_DLL

	if (m_hLaserMuzzleSprite == NULL)
	{
		if (Q_strcmp(GetWpnData().beamOnSound, "None") != 0)
		{
			CSingleUserRecipientFilter filter(ToBasePlayer(GetOwner()));
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			if (m_bIsFiring)
				EmitSound(filter, pOwner->entindex(), GetWpnData().beamOnSound, NULL, 0.0f);
			else
				EmitSound(filter, entindex(), GetWpnData().beamOnSound, NULL, 0.0f);

			m_flNextLaserSwitchTime = gpGlobals->curtime + GetSoundDuration(GetWpnData().beamOnSound, STRING(GetModelName()));
		}

		m_hLaserMuzzleSprite = CSprite::SpriteCreate(GetWpnData().beamHaloMaterial, GetAbsOrigin(), false);
		if (m_hLaserMuzzleSprite == NULL)
		{
			// We were unable to create the sprite
			DevMsg("basecombatweapon_shared->StartLaserEffects(): unable to create the m_hLaserMuzzleSprite \n");
			return;
		}
		m_hLaserMuzzleSprite->SetAttachment(pOwner->GetViewModel(), LookupAttachment(GetWpnData().beamAttachment));
		m_hLaserMuzzleSprite->SetTransparency(kRenderTransAdd, 255, 255, 180, 255, kRenderFxNoDissipation);
		m_hLaserMuzzleSprite->SetBrightness(255, 0.5f);
		m_hLaserMuzzleSprite->SetScale(GetWpnData().beamHaloViewModelWidth, 0.2f);
		m_hLaserMuzzleSprite->SetColor(GetWpnData().lightBeamColor.r, GetWpnData().lightBeamColor.g, GetWpnData().lightBeamColor.b);
		m_hLaserMuzzleSprite->TurnOn();
	}
	/*else if (m_hLaserMuzzleSprite != NULL)
	{
	m_hLaserMuzzleSprite->TurnOn();
	}*/
#endif
}
void CBaseCombatWeapon::StopLaserEffects(CBasePlayer *pOwner)
{
#ifndef CLIENT_DLL
	if (m_hLaserMuzzleSprite != NULL)
	{
		//DevMsg("Stop laser effect \n");
		if (Q_strcmp(GetWpnData().beamOffSound, "None") != 0)
		{
			CSingleUserRecipientFilter filter(ToBasePlayer(GetOwner()));
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			if (m_bIsFiring)
				EmitSound(filter, pOwner->entindex(), GetWpnData().beamOffSound, NULL, 0.0f);
			else
				EmitSound(filter, entindex(), GetWpnData().beamOffSound, NULL, 0.0f);

			m_flNextLaserSwitchTime = gpGlobals->curtime + GetSoundDuration(GetWpnData().beamOnSound, STRING(GetModelName()));
		}

		m_hLaserMuzzleSprite->SetScale(0.0f);
		m_hLaserMuzzleSprite->SetScale(0.001f);
		m_hLaserMuzzleSprite->SetBrightness(0, 0.5f);
		m_hLaserMuzzleSprite->TurnOff();

		delete m_hLaserMuzzleSprite;
		m_hLaserMuzzleSprite = NULL;
	}
#endif
}

void CBaseCombatWeapon::CheckSilencer(CBasePlayer *pOwner)
{
#ifndef CLIENT_DLL
	if (pOwner && pOwner->GetActiveWeapon() && GetWpnData().WeaponUsingSilencer &&
		!GetOwnerIsRunning() &&
		!IsScopeSighted() &&
		!IsIronSighted() &&
		!IsNearWall())
		//!cvar->FindVar("oc_state_near_wall_standing")->GetInt())
	{
		Activity attach = ACT_VM_ATTACH_SILENCER;
		Activity detach = ACT_VM_DETACH_SILENCER;

		if (pOwner->m_afButtonPressed & IN_SWITCHSILENCER)
		{
			if (GetWpnData().allowLoopSound && m_bIsFiring)
				StopLoopSound();

			if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
				m_bIsInPrimaryAttack = false;

			if (m_bSilenced)
			{
				m_bSilenced = false;
				//isSilenced = m_bSilenced;
				//wPD->IsSilencer = isSilenced;

				SendWeaponAnim(detach);
				WeaponSound(SPECIAL2);

				ReinitializeLoopSound(GetWpnData().ShootLoopingSound);
			}
			else
			{
				m_bSilenced = true;
				//isSilenced = m_bSilenced;
				//wPD->IsSilencer = isSilenced;

				SendWeaponAnim(attach);
				WeaponSound(SPECIAL3);

				ReinitializeLoopSound(GetWpnData().ShootLoopingSilencerSound);
			}
			m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
			m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
			m_flNextSilencer = gpGlobals->curtime + GetViewModelSequenceDuration()*0.9f;
		}


		bg = FindBodygroupByName(GetWpnData().silencerBodyGroup);


		CBaseViewModel *pViewModel = pOwner->GetViewModel();

		if (m_bSilenced && !pViewModel->GetBodygroup(bg))
		{
			bgV = GetWpnData().silencerBGEnableValue;
			pViewModel->SetBodygroup(bg, bgV);
		}
		else if (!m_bSilenced && gpGlobals->curtime > m_flNextSilencer && pViewModel->GetBodygroup(bg))
		{
			bgV = GetWpnData().silencerBGDisableValue;
			pViewModel->SetBodygroup(bg, bgV);
		}
	}
#endif
}

bool CBaseCombatWeapon::CanInspect(CBasePlayer *pOwner)
{
	return pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0 ? (Clip1() == GetMaxClip1()) : Clip1() > 0 ? Clip1() > 0 : pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0;
}

void CBaseCombatWeapon::CheckAdmireAnimations(CBasePlayer *pOwner)
{
#ifndef CLIENT_DLL
	if (pOwner->m_flSwingEventAnimTime > gpGlobals->curtime)
		return;
#endif

	if (pOwner && pOwner->IsAlive() && m_flNextSilencer < gpGlobals->curtime + 0.1f)
	{
		if (!m_bInReload && m_iShotgunReloadState == 0 && GetWpnData().animData[m_bFireMode].InspectAnimation != ACT_RESET 	&& m_flNextInspectAnimation < gpGlobals->curtime)
		{
			bool m_bCanInspect = CanInspect(pOwner);

			if (m_bCanInspect
				&& !GetOwnerIsRunning()
				&& !IsScopeSighted()
				&& !IsIronSighted()
				&& !IsNearWall()
				&& !WeaponShouldBeLowered()
				&& !(pOwner->m_nButtons & IN_ATTACK)
				&& !(pOwner->m_nButtons & IN_ATTACK2)
				&& !(pOwner->m_nButtons & IN_ATTACK3)
				&& !m_bIsInPrimaryAttack
				&& (pOwner->m_afButtonPressed & IN_RELOAD))
			{
				if (pOwner && pOwner->GetActiveWeapon() &&
					(GetWpnData().animData[m_bFireMode].InspectAnimation || GetWpnData().animData[m_bFireMode].InspectSilencedAnimation) &&
					(GetActivity() != GetWpnData().animData[m_bFireMode].WallAnimation &&
					GetActivity() != GetWpnData().animData[m_bFireMode].WalkAnimation &&
					GetActivity() != GetWpnData().animData[m_bFireMode].RunAnimation))
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].InspectAnimation);

					ChangeOnce = true;
				}
				else if (pOwner && pOwner->GetActiveWeapon() && !GetWpnData().animData[m_bFireMode].InspectAnimation && !GetWpnData().animData[m_bFireMode].InspectSilencedAnimation)
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].InspectAnimation);

					ChangeOnce = true;
				}
				m_flNextInspectAnimation = gpGlobals->curtime + GetViewModelSequenceDuration();
			}


			if (ChangeOnce && (GetOwnerIsRunning() ||
				IsScopeSighted() ||
				IsIronSighted() ||
				IsNearWall() ||
				m_flNextPrimaryAttack > gpGlobals->curtime ||
				m_flNextSecondaryAttack > gpGlobals->curtime ||
				m_flNextShotgunReload > gpGlobals->curtime ||
				m_flNextSwitchTime > gpGlobals->curtime ||
				m_flNextSilencer > gpGlobals->curtime))
			{
				ChangeOnce = false;
			}
		}
	}
}

void CBaseCombatWeapon::CheckScopeZoomState(CBasePlayer *pOwner)
{
	if (IsScopeSighted())
	{
		if (!scoped)
		{
			pOwner->SetFOV(this, m_bZoomLevel, 0.1f);
			scoped = true;
		}

		if ((pOwner->m_afButtonPressed & IN_ZOOM) && !m_bZoomUpped)
		{
			if (m_bZoomLevel > GetWpnData().minScopeFov)
				m_bZoomLevel -= 5;
			else
				m_bZoomLevel = GetWpnData().maxScopeFov;

			pOwner->SetFOV(this, m_bZoomLevel, 0.1f);

#ifndef CLIENT_DLL
			if (GetWpnData().iWeaponZoomUpSound != AllocPooledString("None"))
				EmitSound(STRING(GetWpnData().iWeaponZoomUpSound));
#endif

			m_bZoomUpped = true;
		}
		if (pOwner->m_afButtonReleased & IN_ZOOM)
		{
			//if (m_bZoomUpped)
			m_bZoomUpped = false;
		}
	}
	if (!IsScopeSighted())
	{
		scoped = false;
	}
}

void CBaseCombatWeapon::CheckZoomToggle(CBasePlayer *pPlayer)
{
	if (pPlayer)
	{
		if (pPlayer->m_afButtonPressed & IN_AIMMODE)
		{
			if (IsScopeSighted())
				OffScopeSight();
			else
			{
				if (!IsIronSighted())
					OnScopeSight();
			}
		}
	}
}
void CBaseCombatWeapon::ToggleZoom(CBasePlayer *pPlayer, bool state)
{
	if (pPlayer == NULL)
		return;

	if (m_iClip1 <= 0 && state)
		return;

	if (IsIronSighted())
		return;


	if (!state)
	{
#ifndef CLIENT_DLL
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			// Send a message to hide the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		}
#endif
	}
	else
	{
		CheckScopeZoomState(pPlayer);
#ifndef CLIENT_DLL
		//if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			// Send a message to Show the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
		}
#endif
	}
}
//====================================================================================
// WEAPON BEHAVIOUR
//====================================================================================
void CBaseCombatWeapon::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_flNextSilencer > gpGlobals->curtime + 0.1f)
		return;

#ifndef CLIENT_DLL
	if (pOwner->m_flSwingEventAnimTime > gpGlobals->curtime)
		return;
#endif

	//Check amdire anims
	//CheckAdmireAnimations(pOwner);

	if (engine->IsPaused() && GetWpnData().allowLoopSound && m_bIsFiring)
		StopLoopSound();

	if (GetWpnData().allowTwoDScope)
	{
		if (GetOwnerIsRunning())
		{
			if (IsScopeSighted())
				OffScopeSight();
		}
		if (IsNearWall())
		{
			if (IsScopeSighted())
				OffScopeSight();
		}
	}

	if (!m_bReloadComplete &&
		!m_bInReload &&
		(IsNearWall() ||
		GetOwnerIsRunning()))
	{
		if (GetWpnData().allowLoopSound)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;
#ifndef CLIENT_DLL
		if (pOwner->grenadeState > 0)
			return;
#endif

	}

	if (GetWpnData().allowTwoDScope)
	{
		if (!IsNearWall() && m_iClip1 && !GetOwnerIsRunning() && !m_bReloadComplete && !m_bInReload)
			CheckZoomToggle(pOwner);
	}

	if (GetActivity() != GetWpnData().animData[m_bFireMode].InspectAnimation)
	{
		m_flNextInspectAnimation = 0.f;
	}

	if (((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2)) && (m_flNextPrimaryAttack < gpGlobals->curtime || m_flNextSecondaryAttack < gpGlobals->curtime))
		letViewModelBob = false;
	else
		letViewModelBob = thisType == TYPE_BEAM ? !m_bIsFiring : true;

	if (thisType == TYPE_SHOTGUN)
	{
		if (!m_iShotgunReloadState && !m_bInReload && !m_bReloadComplete && cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? thisType != TYPE_GRENADE : true)
		{
			WeaponIdle();
		}
	}
	else
	{
		// no fire buttons down or reloading
		if ((m_bInReload == false) && m_bReloadComplete == 0 && cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? thisType != TYPE_GRENADE : true)
		{
			WeaponIdle();
		}
	}

	//OverCharged MikeN's features
#ifndef CLIENT_DLL
	if (GetWpnData().enableLaser)
	{
		if (IsWeaponVisible())
			SwitchLaserGuidance(pOwner);
	}
#endif

	SwitchFiringModes();

	UpdateAutoFire();

	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	CheckSilencer(pOwner);

	CheckReloadState(pOwner);

	UpdateHandleFireOnEmpty();

	bool bFired = false;

	if (GetWpnData().animData[m_bFireMode].UsePistolRefireChecker)
	{
		if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
		{
			m_nShotsFired = 0;
		}
	}
	else
	{
		if ((pOwner->m_nButtons & IN_ATTACK) == false)
		{
			m_nShotsFired = 0;
		}
	}

	//Check scope zoom state
	CheckScopeZoomState(pOwner);

	if (thisType == TYPE_SHOTGUN)
	{
		if ((GetWpnData().AllowShotgunSecondaryAttack ? true : (m_bFireMode == 1 || m_bFireMode == 0)) && m_flNextShotgunReload < gpGlobals->curtime)
		{
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;
			}
			if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;
			}

			if ((pOwner->m_nButtons & IN_ATTACK2) &&
				m_flNextShotgunReload <= gpGlobals->curtime &&
				m_iClip1 > 0 &&
				UsesClipsForAmmo1() &&
				!m_bInReload &&
				!m_bReloadComplete &&
				!m_iShotgunReloadState)
			{
				m_bIsInPrimaryAttack = true;

				SecondaryAttackShotgun();
			}
		}
	}
	else
	{
		// Secondary attack has priority
		if ((pOwner->m_nButtons & IN_ATTACK2) &&
			(m_flNextSecondaryAttack <= gpGlobals->curtime &&
			!(pOwner->m_nButtons & IN_ATTACK)))
		{
			bool hasAmmo = UsesSecondaryAmmoType() ? (UsesClipsForAmmo2() ? Clip2() > 0 : HasSecondaryAmmo()) : (UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo());

			if (GetWpnData().allowLoopSound)
			{
				StopLoopSound();
				m_bIsFiring = false;
			}

			if ((!hasAmmo && !IsMeleeWeapon()) || (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false))
			{
				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				HandleFireOnEmpty(true);

				if (m_flNextEmptySoundTime < gpGlobals->curtime)
				{
					m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
				}
				return;
			}
			else
			{
#if !defined(CLIENT_DLL)
				if (!IsX360() || !ClassMatches("weapon_crossbow"))
#endif
				{
					bFired = ShouldBlockPrimaryFire();
				}

				if (m_bReloadComplete == 0)
				{
					m_bIsInPrimaryAttack = true;

					SecondaryAttack();

					//WeaponSound(WPN_DOUBLE);
				}

				if (GetWpnData().m_bUsePrimaryAmmoAsSecondary)
				{

				}
				else
				{

				}
			}
		}
	}

	if (thisType == TYPE_SHOTGUN)
	{
		if (!HasAnyAmmo() && m_iClip1 <= 0 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && (m_flNextPrimaryAttack < gpGlobals->curtime || m_flNextShotgunReload < gpGlobals->curtime))
		{
			if (((GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false) && (g_pGameRules->SwitchToNextBestWeapon(pOwner, this)))
			{
				m_flNextPrimaryAttack = m_flNextShotgunReload = gpGlobals->curtime + 0.3;
				return;
			}

		}

		if ((pOwner->m_nButtons & IN_ATTACK) && m_bFireMode == 2 && gpGlobals->curtime > m_flNextPrimaryAttack && m_iShotgunReloadState == 0)//Auto shotgun
		{
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				//DryFire();

				return;
			}
			else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				//DryFire();

				return;
			}
			else
			{
				m_bIsInPrimaryAttack = true;

				PrimaryAttackShotgun();

				if (AutoFiresFullClip())
				{
					m_bFiringWholeClip = true;
				}
#ifdef CLIENT_DLL
				pOwner->SetFiredWeapon(true);
#endif
			}
		}
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_bFireMode == 1 || m_bFireMode == 0) && gpGlobals->curtime > m_flNextShotgunReload && m_iShotgunReloadState == 0)//manual shotgun
		{
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				return;
			}
			else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				return;
			}
			else
			{
				m_bIsInPrimaryAttack = true;

				PrimaryAttackShotgun();

				if (AutoFiresFullClip())
				{
					m_bFiringWholeClip = true;
				}
#ifdef CLIENT_DLL
				pOwner->SetFiredWeapon(true);
#endif
			}
		}
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_bFireMode == 1 || m_bFireMode == 0) && gpGlobals->curtime > m_flNextShotgunReload && m_iShotgunReloadState == 0)
		{
			m_bIsInPrimaryAttack = true;

			PrimaryAttackShotgun();

			if (GetWpnData().allowAmmoRegen)
				RemoveAmmo(GetPrimaryAmmoType(), 1);
		}
	}
	else if (thisType == TYPE_BEAM)
	{
		if (!bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			bool hasAmmo = UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo();

			if ((!hasAmmo && !IsMeleeWeapon()) || (pOwner->GetWaterLevel() == 3 && !m_bFiresUnderwater))
			{
				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				m_BeamFireState = FIRE_END;

				/*if (m_bIsFiring && GetWpnData().allowLoopSound)
				{
					StopLoopSound();
					m_bIsFiring = false;
				}*/

				HandleFireOnEmpty();

				return;
			}
			else
			{
				m_bIsInPrimaryAttack = true;

				PrimaryAttack();
				PrimaryAttackPostUpdate();

				if (AutoFiresFullClip())
				{
					m_bFiringWholeClip = true;
				}

#ifdef CLIENT_DLL
				pOwner->SetFiredWeapon(true);
#endif
			}
		}
	}
	else//SMG/Rifle/Pistol
	{
		if (!bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			bool hasAmmo = UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo();

			if ((!hasAmmo && !IsMeleeWeapon()) || (pOwner->GetWaterLevel() == 3 && !m_bFiresUnderwater))
			{
				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				HandleFireOnEmpty();

				return;
			}
			else
			{
				if (m_bFireMode != 3)
				{
					m_bIsInPrimaryAttack = true;

					PrimaryAttack();
					PrimaryAttackPostUpdate();

					if (m_bFireMode == 2 && GetWpnData().allowLoopSound)
						StartLoopSound();
				}
				else
				{
					HandleBurstFire(true);
				}

				if (AutoFiresFullClip())
				{
					m_bFiringWholeClip = true;
				}

#ifdef CLIENT_DLL
				pOwner->SetFiredWeapon(true);
#endif
			}
		}

		if (m_bFireMode == 1 || m_bFireMode == 0)
			HandleSingleFire();
	}


	if (m_bFireMode == 3 && (thisType == TYPE_PISTOL ||
		thisType == TYPE_DUAL_PISTOLS ||
		thisType == TYPE_DUAL_MACHINEGUNS ||
		thisType == TYPE_SMG ||
		thisType == TYPE_AIRBOATGUN))
		HandleBurstFire();

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if (thisType != TYPE_SHOTGUN)
	{
		if (CanReload() && (pOwner->m_nButtons & IN_RELOAD) &&
			!(pOwner->m_nButtons & IN_ATTACK) &&
			GetActivity() != ACT_VM_ATTACH_SILENCER &&
			GetActivity() != ACT_VM_DETACH_SILENCER &&
			UsesClipsForAmmo1() &&
			!m_bInReload &&
			!m_bReloadComplete)
		{
			if (GetWpnData().allowLoopSound && m_bIsFiring)
				StopLoopSound();

			if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
				m_bIsInPrimaryAttack = false;

			if (m_flNextSwitchTime < gpGlobals->curtime)
				Reload();

			m_fFireDuration = 0.0f;
		}
	}


	if (pOwner->m_nButtons & IN_FIREMODE) { FireMode(); }

	if (CanReload() && !((pOwner->m_nButtons & IN_RELOAD)) &&
		!(pOwner->m_nButtons & IN_ATTACK))
	{
		if (GetWpnData().allowLoopSound && m_bIsFiring && m_flNextPrimaryAttack < gpGlobals->curtime)
			StopLoopSound();

		if (!(pOwner->m_nButtons & IN_FIREMODE))
		{
			if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
				m_bIsInPrimaryAttack = false;
		}
	}


	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (!Clip1()))
	{
		if (GetWpnData().allowLoopSound && m_bIsFiring)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;
	}
	else if (!(pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (!Clip1()))
	{
		if (GetWpnData().allowLoopSound && m_bIsFiring)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;
	}

	if (Clip1() > 0 && m_bReloadComplete != 0)
		m_bReloadComplete = 0;
}

void CBaseCombatWeapon::HandleBurstFire(bool bPressedOnce)
{
	if (bPressedOnce)
	{
		if (m_bFireMode == 3 && !wasburstFire && m_flNextPrimaryAttack <= gpGlobals->curtime + GetFireRate() + GetWpnData().mode_fire_rate_burst_refire_time)
			wasburstFire = true;

		return;
	}

	if (wasburstFire)
	{
		if ((m_bInReload || !Clip1()))
		{
			burstFireCount = 0;
			wasburstFire = false;
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate() + GetWpnData().mode_fire_rate_burst_refire_time;

			if (GetWpnData().allowLoopSound && m_bIsFiring)
				StopLoopSound();

			if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
				m_bIsInPrimaryAttack = false;
		}
		else
		{
			if ((m_flNextPrimaryAttack <= gpGlobals->curtime) && burstFireCount < GetWpnData().mode_fire_burst_count)
			{
				if (!m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = true;

				PrimaryAttack();
				PrimaryAttackPostUpdate();

				if (GetWpnData().allowLoopSound && !m_bIsFiring)
					StartLoopSound();
			}
			if (burstFireCount >= GetWpnData().mode_fire_burst_count)//if (burstFireCount >= 3)
			{
				if (GetWpnData().allowLoopSound && m_bIsFiring)
					StopLoopSound();

				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;
			}
		}
	}
	else
	{
		if (!burstFireCount && m_bIsFiring)
			StopLoopSound();
	}
}

void CBaseCombatWeapon::HandleSingleFire()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (!(pOwner->m_nButtons & IN_ATTACK) && m_flSoonestPrimaryAttack < gpGlobals->curtime)
	{
		if (GetActivity() != GetWpnData().animData[m_bFireMode].WeaponSwitchAnim)
		{
			bool hasAmmo = UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo();

			if (hasAmmo && !IsInReload())
				m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
		}
	}
}

extern ConVar sk_auto_reload_time;
void CBaseCombatWeapon::ItemHolsterFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		if (GetWpnData().allowLoopSound && m_bIsFiring)
			StopLoopSound();

		if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_bIsInPrimaryAttack = false;
		return;
	}

	if (thisType == TYPE_SHOTGUN)
	{
		// Must be player held
		if (GetOwner() && GetOwner()->IsPlayer() == false)
			return;

		// We can't be active
		if (GetOwner()->GetActiveWeapon() == this)
			return;

		// If it's been longer than three seconds, reload
		if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
		{
			// Reset the timer
			m_flHolsterTime = gpGlobals->curtime;

			if (GetOwner() == NULL)
				return;

			if (m_iClip1 == GetMaxClip1())
				return;

			// Just load the clip with no animations
			int ammoFill = MIN((GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount(GetPrimaryAmmoType()));

			//GetOwner()->RemoveAmmo(ammoFill, GetPrimaryAmmoType());
			RemoveAmmo(GetPrimaryAmmoType(), ammoFill);
			m_iClip1 += ammoFill;
		}
	}

	if (GetWpnData().allowAmmoRegen)
		RechargeAmmo();

	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;

	// If it's been longer than three seconds, reload
	if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
	{
		// Just load the clip with no animations
		FinishReload();
		m_flHolsterTime = gpGlobals->curtime;
	}
}

void CBaseCombatWeapon::UpdateHandleFireOnEmpty()
{
	// If we're already firing on empty, reload if we can
	if (m_bFireOnEmpty && m_flNextEmptySoundTime < gpGlobals->curtime)
	{
		if (ReloadOrSwitchWeapons())
			m_bFireOnEmpty = false;

		m_fFireDuration = 0.0f;
		m_flNextEmptySoundTime = 0;
	}
	else
	{
		bool hasAmmo = UsesClipsForAmmo1() ? Clip1() > 0 : HasPrimaryAmmo();

		if (!hasAmmo && !m_bFireOnEmpty && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			m_bFireOnEmpty = true;
		}
	}
}

void CBaseCombatWeapon::HandleFireOnEmpty(bool bSecondary)
{
	if (m_bIsFiring && GetWpnData().allowLoopSound)
	{
		StopLoopSound();
		m_bIsFiring = false;
	}

	if (GetWpnData().allowAmmoRegen)
		return;

	DryFire(bSecondary);
	m_bFireOnEmpty = true;
}

void CBaseCombatWeapon::DryFire(bool bSecondary)
{
	const char *shootsound = GetShootSound(EMPTY);
	if (!shootsound || !shootsound[0])
		return;

	if (m_flNextEmptySoundTime > gpGlobals->curtime)
		return;

	if (GetWpnData().animData[m_bFireMode].DryFireAnimation != ACT_RESET)
	{
		if (m_bFireMode == 1 || m_bFireMode == 0)
			m_flSoonestPrimaryAttack = gpGlobals->curtime + GetWpnData().mode_fastest_dry_refire_time;

		WeaponSound(EMPTY);
		SendWeaponAnim(ACT_VM_DRYFIRE);

		m_flNextEmptySoundTime = gpGlobals->curtime + GetViewModelSequenceDuration();
	}
	else
	{
		WeaponSound(EMPTY);
		m_flNextEmptySoundTime = gpGlobals->curtime + GetSoundDuration(shootsound, STRING(GetModelName()));

		bool hasAmmo = bSecondary && UsesSecondaryAmmoType() ? HasSecondaryAmmo() : /*UsesClipsForAmmo1() ? Clip1() > 0 : */HasPrimaryAmmo();

		if (!hasAmmo && m_flNextEmptySoundTime - gpGlobals->curtime < 0.3f)
			m_flNextEmptySoundTime = gpGlobals->curtime + 0.3f;
	}

	if (m_bFireMode == 1 || m_bFireMode == 0)
		m_flSoonestPrimaryAttack = gpGlobals->curtime + GetWpnData().mode_fastest_dry_refire_time;

	if (GetWpnData().allowLoopSound && m_bIsFiring)
		StopLoopSound();

	if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
		m_bIsInPrimaryAttack = false;

}
//-----------------------------------------------------------------------------
// Purpose: Called each frame by the player PostThink, if the player's not ready to attack yet
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemBusyFrame(void)
{
	if (m_bFireMode == 1 || m_bFireMode == 0)
	{
		UpdatePenaltyTime();
	}

	UpdateAutoFire();
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting bullet type
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetBulletType(void)
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting spread
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector& CBaseCombatWeapon::GetBulletSpread(void)
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		static Vector spread;
		if (GetOwner()->GetActiveWeapon())
		{
			if ((GetOwner()->GetFlags() & FL_DUCKING) != 0)
			{
				if (IsIronSighted())
				{
					spread.x = GetWpnData().irSightSpread;
					spread.y = GetWpnData().irSightSpread;
					spread.z = GetWpnData().irSightSpread;
				}
				else if (IsScopeSighted())
				{
					spread.x = GetWpnData().ScopeSightSpread;
					spread.y = GetWpnData().ScopeSightSpread;
					spread.z = GetWpnData().ScopeSightSpread;
				}
				else
				{
					spread.x = GetWpnData().crouchSpread;
					spread.y = GetWpnData().crouchSpread;
					spread.z = GetWpnData().crouchSpread;
				}
			}
			else
			{
				if (GetOwnerIsRunning())//(cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() > cvar->FindVar("hl2_normspeed")->GetFloat() + 50.f)
				{
					spread.x = GetWpnData().runningSpread;
					spread.y = GetWpnData().runningSpread;
					spread.z = GetWpnData().runningSpread;
				}
				else if (!GetOwnerIsRunning() && cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 50.f)//(cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() <= cvar->FindVar("hl2_normspeed")->GetFloat())
				{
					if (IsIronSighted())
					{
						spread.x = GetWpnData().irSightSpread;
						spread.y = GetWpnData().irSightSpread;
						spread.z = GetWpnData().irSightSpread;
					}
					else if (IsScopeSighted())
					{
						spread.x = GetWpnData().ScopeSightSpread;
						spread.y = GetWpnData().ScopeSightSpread;
						spread.z = GetWpnData().ScopeSightSpread;
					}
					else
					{
						spread.x = GetWpnData().walkingSpread;
						spread.y = GetWpnData().walkingSpread;
						spread.z = GetWpnData().walkingSpread;
					}
				}
				else if (!GetOwnerIsRunning() && cvar->FindVar("oc_state_player_velocity")->GetFloat() < 50.f)
				{
					if (IsIronSighted())
					{
						spread.x = GetWpnData().irSightSpread;
						spread.y = GetWpnData().irSightSpread;
						spread.z = GetWpnData().irSightSpread;
					}
					else if (IsScopeSighted())
					{
						spread.x = GetWpnData().ScopeSightSpread;
						spread.y = GetWpnData().ScopeSightSpread;
						spread.z = GetWpnData().ScopeSightSpread;
					}
					else
					{
						spread.x = GetWpnData().standingSpread;
						spread.y = GetWpnData().standingSpread;
						spread.z = GetWpnData().standingSpread;
					}
				}
			}
		}
		return spread;
	}
	else if (GetOwner() && GetOwner()->IsNPC())
	{
		static Vector spread;
		if (GetOwner()->GetActiveWeapon())
		{
			if ((GetOwner()->GetFlags() & FL_DUCKING) != 0)
			{
				spread.x = GetWpnData().crouchSpreadNpc;
				spread.y = GetWpnData().crouchSpreadNpc;
				spread.z = GetWpnData().crouchSpreadNpc;
			}
			else
			{
				if (GetOwner()->m_flSpeed > 0)
				{
					spread.x = GetWpnData().runningSpreadNpc;
					spread.y = GetWpnData().runningSpreadNpc;
					spread.z = GetWpnData().runningSpreadNpc;
				}
				/*else if (cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() <= cvar->FindVar("hl2_normspeed")->GetFloat())
				{
				spread.x = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
				spread.y = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
				spread.z = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
				}*/
				else if (GetOwner()->m_flSpeed <= 0)
				{
					spread.x = GetWpnData().standingSpreadNpc;
					spread.y = GetWpnData().standingSpreadNpc;
					spread.z = GetWpnData().standingSpreadNpc;
				}
			}
		}
		return spread;
	}

	static Vector cone = VECTOR_CONE_15DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseCombatWeapon::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t defaultWeaponProficiencyTable[] =
	{
		{ 1.0, 1.0 },
		{ 1.0, 1.0 },
		{ 1.0, 1.0 },
		{ 1.0, 1.0 },
		{ 1.0, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(defaultWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);
	return defaultWeaponProficiencyTable;
}


//-----------------------------------------------------------------------------
// Purpose: Base class default for getting firerate
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetFireRate(void)
{
	if (cvar->FindVar("oc_test_weapon_firerate")->GetFloat() > 0)
		return	cvar->FindVar("oc_test_weapon_firerate")->GetFloat();

	float fire_rate = 1.f;
	switch (m_bFireMode)
	{
		case(0) :
		{
			fire_rate = this->GetWpnData().fireRate;
		}
		break;
		case(1) :
		{
			fire_rate = this->GetWpnData().mode_fire_rate_single;
		}
		break;
		case(2) :
		{
			fire_rate = this->GetWpnData().mode_fire_rate_automatic;
		}
		break;
		case(3) :
		{
			fire_rate = this->GetWpnData().mode_fire_rate_burst;
		}
		break;
		default:
			break;
	}

	return fire_rate;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for playing shoot sound
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::WeaponSound(WeaponSound_t sound_type, float soundtime /* = 0.0f */)
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound(sound_type);
	if (!shootsound || !shootsound[0])
		return;

	CSoundParameters params;

	if (!GetParametersForSound(shootsound, params, NULL))
		return;

	if (params.play_to_owner_only)
	{
		// Am I only to play to my owner?
		if (GetOwner() && GetOwner()->IsPlayer())
		{
			CSingleUserRecipientFilter filter(ToBasePlayer(GetOwner()));
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, GetOwner()->entindex(), shootsound, NULL, soundtime);

		}
	}
	else
	{
		// Play weapon sound from the owner
		if (GetOwner())
		{
			CPASAttenuationFilter filter(GetOwner(), params.soundlevel);
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, GetOwner()->entindex(), shootsound, NULL, soundtime);

#if !defined( CLIENT_DLL )
			if (sound_type == EMPTY)
			{
				CSoundEnt::InsertSound(SOUND_COMBAT, GetOwner()->GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, GetOwner());
			}
#endif
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			CPASAttenuationFilter filter(this, params.soundlevel);
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, entindex(), shootsound, NULL, soundtime);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop a sound played by this weapon.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::StopWeaponSound(WeaponSound_t sound_type)
{
	//if ( IsPredicted() )
	//	return;

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound(sound_type);
	if (!shootsound || !shootsound[0])
		return;

	CSoundParameters params;
	if (!GetParametersForSound(shootsound, params, NULL))
		return;

	// Am I only to play to my owner?
	if (params.play_to_owner_only)
	{
		if (GetOwner())
		{
			StopSound(GetOwner()->entindex(), shootsound);
		}
	}
	else
	{
		// Play weapon sound from the owner
		if (GetOwner())
		{
			StopSound(GetOwner()->entindex(), shootsound);
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			StopSound(entindex(), shootsound);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DefaultReload(int iClipSize1, int iClipSize2, int iActivity)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	if (m_iClip1 == GetMaxClip1())
		return false;
	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	bool bReload = false;

	// If you don't have clips, then don't try to reload them.
	if (UsesClipsForAmmo1())
	{
		// need to reload primary clip?
		int primary = MIN(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if (primary != 0)
		{
			bReload = true;
		}
	}

	if (UsesClipsForAmmo2())
	{
		// need to reload secondary clip?
		int secondary = MIN(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if (secondary != 0)
		{
			bReload = true;
		}
	}

	if (GetWpnData().allowTwoDScope && IsScopeSighted())
	{
		OffScopeSight();
	}

	if (!bReload)
		return false;

	if (GetWpnData().enableLaser)
		EnableLaserInterrupt = true;
#ifndef CLIENT_DLL	
	if (m_hLaserMuzzleSprite != NULL)
	{
		StopLaserEffects((CBasePlayer*)pOwner);
	}
#endif
#ifdef CLIENT_DLL
	// Play reload
	WeaponSound(RELOAD);
#endif


	SendWeaponAnim(iActivity);
	//SetActivity((Activity)iActivity, 0.1f);
	//SetAnimTime(0.1f);

	if (pOwner->IsPlayer())
	{
		((CBasePlayer *)pOwner)->SetAnimation(PLAYER_RELOAD);
	}

	MDLCACHE_CRITICAL_SECTION();
	float flSequenceEndTime = gpGlobals->curtime + GetViewModelSequenceDuration();
	pOwner->SetNextAttack(flSequenceEndTime);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	m_bInReload = true;

	if (pOwner && pOwner->IsPlayer())
	{
		pOwner->RemoveEffects(EF_M);

		pOwner->RemoveEffects(EF_ML);
	}

	return true;
}

bool CBaseCombatWeapon::DefaultReloadEmpty(int iClipSize1, int iClipSize2, int iActivity)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	bool bReload = false;

	// If you don't have clips, then don't try to reload them.
	if (UsesClipsForAmmo1())
	{
		// need to reload primary clip?
		int primary = MIN(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if (primary != 0)
		{
			bReload = true;
		}
	}

	if (UsesClipsForAmmo2())
	{
		// need to reload secondary clip?
		int secondary = MIN(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if (secondary != 0)
		{
			bReload = true;
		}
	}

	if (!bReload)
		return false;

	if (GetWpnData().enableLaser)
		EnableLaserInterrupt = true;
#ifndef CLIENT_DLL	
	if (m_hLaserMuzzleSprite != NULL)
	{
		StopLaserEffects((CBasePlayer*)pOwner);
	}
#endif
#ifdef CLIENT_DLL
	// Play reload
	WeaponSound(RELOAD);
#endif
	if (GetWpnData().animData[m_bFireMode].PumpAnimation != 0
		&& Clip1() < 1)
	{
		if (GetActivity() != iActivity && m_bReloadComplete == 0)
		{
			m_bReloadComplete = 1;
			SendWeaponAnim(iActivity);

			m_wSequence = gpGlobals->curtime + GetViewModelSequenceDuration();
		}

		if (m_wSequence <= gpGlobals->curtime && m_bReloadComplete == 1)
		{
			if (GetActivity() != GetWpnData().animData[m_bFireMode].PumpAnimation && m_bReloadComplete == 1)
			{
				m_bReloadComplete = 2;
				SendWeaponAnim(GetWpnData().animData[m_bFireMode].PumpAnimation);
				m_wSequence2 = gpGlobals->curtime + GetViewModelSequenceDuration();
			}
		}

		if (m_wSequence2 <= gpGlobals->curtime && m_bReloadComplete == 2)
		{
			m_bReloadComplete = 0;

			m_bInReload = true;

			if (pOwner->IsPlayer())
			{
				((CBasePlayer *)pOwner)->SetAnimation(PLAYER_RELOAD);
			}

			MDLCACHE_CRITICAL_SECTION();
			float flSequenceEndTime = gpGlobals->curtime;// +SequenceDuration();
			pOwner->SetNextAttack(flSequenceEndTime);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

			if (pOwner && pOwner->IsPlayer())
			{

				pOwner->RemoveEffects(EF_M);

				pOwner->RemoveEffects(EF_ML);

			}

			m_wSequence = 0;
			m_wSequence2 = 0;

			return true;
		}
	}
	return true;
}

bool CBaseCombatWeapon::ReloadsSingly(void) const
{
#if defined ( TF_DLL ) || defined ( TF_CLIENT_DLL )
	float fHasReload = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(fHasReload, mod_no_reload_display_only);
	if (fHasReload != 1.0f)
	{
		return false;
	}

	int iWeaponMod = 0;
	CALL_ATTRIB_HOOK_INT(iWeaponMod, set_scattergun_no_reload_single);
	if (iWeaponMod == 1)
	{
		return false;
	}
#endif // TF_DLL || TF_CLIENT_DLL

	return m_bReloadsSingly;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Reload(void)
{
	Activity reloadAct = ACT_VM_RELOAD;

	if (GetOwner() && GetOwner()->IsPlayer() && GetOwner()->GetActiveWeapon())
	{
		if (!Clip1())
		{
			if (GetWpnData().animData[m_bFireMode].ReloadFullEmptyAnimation != ACT_RESET)
			{
				reloadAct = GetWpnData().animData[m_bFireMode].ReloadFullEmptyAnimation;
			}
			else
			{
				if (GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation != ACT_RESET)
					reloadAct = GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation;
			}
		}
		else
		{
			if (GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation != ACT_RESET)
				reloadAct = GetWpnData().animData[m_bFireMode].ReloadDefaultAnimation;
		}

		if (GetWpnData().allowTwoDScope && IsScopeSighted())
		{
			OffScopeSight();//ToggleZoom(ToBasePlayer(GetOwner()), IsScopeSighted());
		}
		//ToggleZoom(ToBasePlayer(GetOwner()), false);
	}

	return DefaultReload(GetMaxClip1(), GetMaxClip2(), reloadAct);
}


bool CBaseCombatWeapon::WeaponShouldBeLowered(void)
{
#if !defined( CLIENT_DLL )
	if (cvar->FindVar("oc_ally_allow_friendlyfire")->GetInt())
		return false;
#endif
	CheckLowering();

	return m_bShouldBeLowered;
}

void CBaseCombatWeapon::CheckLowering()
{	
	ConVarRef allow_friendlyfire("oc_ally_allow_friendlyfire");

	bool shouldNotLowering = (GetActivity() == GetWpnData().animData[m_bFireMode].InspectAnimation) || IsIronSighted() || IsScopeSighted() || /*IsInReload() || */allow_friendlyfire.GetBool();

	if (shouldNotLowering)
	{
		if (m_bShouldBeLowered)
			m_bShouldBeLowered = false;

		return;
	}

	/*if (cvar->FindVar("oc_ally_allow_friendlyfire")->GetInt())
	{
		m_bShouldBeLowered = false;
		return;
	}

	if (IsIronSighted() || IsScopeSighted() || IsInReload())
	{
		m_bShouldBeLowered = false;
		return;
	}

	if (GetActivity() == GetWpnData().animData[m_bFireMode].InspectAnimation)
	{
		m_bShouldBeLowered = false;
		return;
	}*/

#if !defined( CLIENT_DLL )

	if (m_flNextLowerTime > gpGlobals->curtime)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		trace_t tr;
		Vector forward;
		Vector Eye = pPlayer->EyePosition();
		pPlayer->EyeVectors(&forward);

		ConVarRef oc_ally_distance_to_lowing("oc_ally_distance_to_lowing");

		UTIL_TraceHull(Eye, Eye + forward * oc_ally_distance_to_lowing.GetFloat(), Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
		//debugoverlay->AddLineOverlay(Eye, Eye + forward * MAX_TRACE_LENGTH, 255, 0, 0, false, 0.1f);

		if (tr.m_pEnt != NULL && tr.m_pEnt->IsNPC())
		{
			if (/*pPlayer->FInViewCone(tr.m_pEnt) && */tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY || tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL || tr.m_pEnt->Classify() == CLASS_VORTIGAUNT)
			{
				GlobalEntity_SetState(MAKE_STRING("friendly_encounter"), GLOBAL_ON);

				m_bShouldBeLowered = true;
			}
			else
			{
				GlobalEntity_SetState(MAKE_STRING("friendly_encounter"), GLOBAL_OFF);

				m_bShouldBeLowered = false;
			}
		}
		else
			m_bShouldBeLowered = false;
	}
	else
		m_bShouldBeLowered = false;

	m_flNextLowerTime = gpGlobals->curtime + 0.3f;

#endif
}

Activity CBaseCombatWeapon::SetIdleActivity(bool lowering)
{
	if (lowering)
	{
#if !defined( CLIENT_DLL )

		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(GetOwner());

		if (pPlayer)
			pPlayer->Weapon_Lower();
#endif
		if (GetOwner() && GetOwner()->GetActiveWeapon())
		{
			if (!Clip1())
			{
				if (GetWpnData().animData[m_bFireMode].LoweredFullEmpty)
				{
					return GetWpnData().animData[m_bFireMode].LoweredFullEmpty;
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].LoweredAnimation != ACT_RESET)
						return GetWpnData().animData[m_bFireMode].LoweredAnimation;
					else
						return ACT_VM_IDLE_LOWERED;
				}
			}
			else
			{
				if (GetWpnData().animData[m_bFireMode].LoweredAnimation != ACT_RESET)
					return GetWpnData().animData[m_bFireMode].LoweredAnimation;
				else
					return ACT_VM_IDLE_LOWERED;
			}
		}
	}
	else
	{
		if ((IsIronSighted() || IsScopeSighted()) && GetWpnData().animData[m_bFireMode].ZeroIdleAnim)
			SendWeaponAnim(ACT_VM_IDLE);

		if (IsScopeSighted())
		{
			if (!Clip1())
			{
				if (GetWpnData().animData[m_bFireMode].IronFullEmpty != ACT_RESET)
				{
					return GetWpnData().animData[m_bFireMode].IronFullEmpty;
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].IronsightIdle != ACT_RESET)
						return GetWpnData().animData[m_bFireMode].IronsightIdle;
					else
						return ACT_VM_IDLE;
				}
			}
			else
			{
				if (GetWpnData().animData[m_bFireMode].IronsightIdle != ACT_RESET)
					return GetWpnData().animData[m_bFireMode].IronsightIdle;
				else
					return ACT_VM_IDLE;
			}
		}
		else
		{

			if (!Clip1())
			{
				if (GetWpnData().animData[m_bFireMode].IdleFullEmpty != ACT_RESET)
				{
					return GetWpnData().animData[m_bFireMode].IdleFullEmpty;
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].IdleAnimation != ACT_RESET)
						return GetWpnData().animData[m_bFireMode].IdleAnimation;
					else
						return ACT_VM_IDLE;
				}
			}
			else
			{
				if (GetWpnData().animData[m_bFireMode].IdleAnimation != ACT_RESET)
					return GetWpnData().animData[m_bFireMode].IdleAnimation;
				else
					return ACT_VM_IDLE;
			}
		}
	}

	return ACT_VM_HOLSTER;
}

bool CBaseCombatWeapon::CanWeaponIdle(CBasePlayer *pOwner)
{
	if (!pOwner)
		return false;

	if (!pOwner->GetActiveWeapon())
		return false;

	if (m_bInReload || m_bReloadComplete)
		return false;

	if (cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? thisType == TYPE_GRENADE : false)
		return false;

	if ((IsIronSighted() || IsScopeSighted()) && GetWpnData().animData[m_bFireMode].ZeroIdleAnim)
	{
		if (m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime)
			SendWeaponAnim(ACT_VM_IDLE);
		return false;
	}

	Activity curAct = GetActivity();

	bool WallRunResetActivity = (IsNearWall() || GetOwnerIsRunning());

	bool AttackActivityPlaying = WallRunResetActivity ?
		false :
		(curAct == ACT_RESET ? false : attackActivites.HasElement(curAct)) && m_flTimeWeaponIdle >= gpGlobals->curtime;

	bool WeaponBusyByAttack = thisType == TYPE_BEAM ? m_bIsFiring || (m_BeamFireState > 0 && m_BeamFireState < 3) :
		(AttackActivityPlaying || m_bIsFiring ||
		(WallRunResetActivity ? (m_flNextPrimaryAttack >= gpGlobals->curtime || m_flNextSecondaryAttack >= gpGlobals->curtime) :
		false));


	bool WeaponBusyByShotgunState = GetActivity() == ACT_SHOTGUN_PUMP ||
		GetActivity() == ACT_SHOTGUN_PUMP2 ||
		GetActivity() == ACT_SHOTGUN_PUMP_DOUBLE ||
		GetActivity() == ACT_SHOTGUN_PUMP2_DOUBLE ||
		GetActivity() == ACT_SHOTGUN_RELOAD_START ||
		GetActivity() == ACT_SHOTGUN_RELOAD_FINISH ||
		GetActivity() == ACT_VM_PULLBACK_HIGH ||
		GetActivity() == ACT_VM_PULLBACK;

	if (WeaponBusyByShotgunState && !m_iShotgunReloadState && m_flTimeWeaponIdle < gpGlobals->curtime)
		WeaponBusyByShotgunState = false;

	bool tFPNextAkt = false;
	if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
		tFPNextAkt = GetActivity() == GetPrimaryAttackActivity();

	bool WeaponBusyByMoving = false;

	if (WallRunResetActivity)
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextShotgunReload = m_flSoonestPrimaryAttack = gpGlobals->curtime + TIME_AFTER_ANIMSTATE;
		m_flTimeWeaponIdle = gpGlobals->curtime;
	}

	bool isInspect = GetOwnerIsRunning() ? false : m_flNextInspectAnimation >= gpGlobals->curtime;

	if (IsIronSighted() && m_flNextInspectAnimation > gpGlobals->curtime)
		isInspect = false;

	bool WeaponBusyBeSpecial = isInspect || m_flNextSilencer >= gpGlobals->curtime || m_flNextSwitchTime >= gpGlobals->curtime;

	bool WeaponBusyCheckEntire = WallRunResetActivity ? WeaponBusyBeSpecial ? WeaponBusyBeSpecial : false : (WeaponBusyByAttack || WeaponBusyByShotgunState || WeaponBusyBeSpecial) ? true : WeaponBusyByMoving;

	bool debugIdle = false;
	if (debugIdle)
	{
		DevMsg("WeaponBusyByAttack: %i\n", WeaponBusyByAttack);
		DevMsg("WeaponBusyByShotgunState: %i\n", WeaponBusyByShotgunState);
		DevMsg("WeaponBusyBeSpecial: %i\n", WeaponBusyBeSpecial);
		DevMsg("WeaponBusyByMoving: %i\n", WeaponBusyByMoving);
		DevMsg("wallBump_Running: %i\n", WallRunResetActivity);
	}

	if (WeaponBusyCheckEntire)
		return false;

	return true;
}

void CBaseCombatWeapon::WeaponIdle(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!CanWeaponIdle(pOwner))
		return;

	bool loweredNoScopeNoSight = WeaponShouldBeLowered() && !IsScopeSighted() && !IsIronSighted();

	bool ActNotEqualIdle = (GetActivity() != SetIdleActivity()) || m_flTimeWeaponIdle < gpGlobals->curtime;

	if (loweredNoScopeNoSight)
	{
		if (!IsInReload() && GetActivity() != SetIdleActivity(loweredNoScopeNoSight) && GetActivity() != ACT_TRANSITION)
		{
			SendWeaponAnim(SetIdleActivity(loweredNoScopeNoSight));
			//m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();
		}

		/*if (IsInReload())
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;*/

		return;
	}
	//else// if (noLoweringNoScopeNoSight)
	{
		if (pOwner->GetFlags() & FL_ONGROUND
			&& ((pOwner->m_nButtons & IN_FORWARD)
			|| (pOwner->m_nButtons & IN_BACK)
			|| (pOwner->m_nButtons & IN_MOVELEFT)
			|| (pOwner->m_nButtons & IN_MOVERIGHT))
			&& cvar->FindVar("oc_state_player_velocity")->GetFloat() > 5
			&& !IsScopeSighted()
			&& !IsIronSighted()
			/*&& (GetWpnData().animData[m_bFireMode].UseHalfFullMagAnimations != ACT_RESET//Убрать
			|| GetWpnData().animData[m_bFireMode].WallAnimation != ACT_RESET//Убрать
			|| GetWpnData().animData[m_bFireMode].RunAnimation != ACT_RESET//Убрать
			|| GetWpnData().animData[m_bFireMode].WalkAnimation != ACT_RESET)*/)//Убрать
		{
			if (IsNearWall() && !GetOwnerIsRunning()
				&& (GetWpnData().animData[m_bFireMode].WallAnimation != ACT_RESET || GetWpnData().animData[m_bFireMode].WallAlmostEmpty != ACT_RESET))
			{
				if (GetWpnData().animData[m_bFireMode].WallFullEmpty != ACT_RESET && !HasPrimaryAmmo())
				{
					if (GetActivity() != GetWpnData().animData[m_bFireMode].WallFullEmpty)
						SendWeaponAnim(GetWpnData().animData[m_bFireMode].WallFullEmpty);
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].WallAnimation != ACT_RESET)
					{
						if (GetActivity() != GetWpnData().animData[m_bFireMode].WallAnimation)
							SendWeaponAnim(GetWpnData().animData[m_bFireMode].WallAnimation);
					}
					else
					{
						if (ActNotEqualIdle)
							SendWeaponAnim(SetIdleActivity());
					}
				}
			}
			if (IsNearWall() && GetOwnerIsRunning()
				&& (GetWpnData().animData[m_bFireMode].RunAnimation != ACT_RESET || GetWpnData().animData[m_bFireMode].RunAlmostEmpty != ACT_RESET))
			{
				if (GetWpnData().animData[m_bFireMode].RunFullEmpty != ACT_RESET && !HasPrimaryAmmo())
				{
					if (GetActivity() != GetWpnData().animData[m_bFireMode].RunFullEmpty)
						SendWeaponAnim(GetWpnData().animData[m_bFireMode].RunFullEmpty);
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].RunAnimation != ACT_RESET)
					{
						if (GetActivity() != GetWpnData().animData[m_bFireMode].RunAnimation)
							SendWeaponAnim(GetWpnData().animData[m_bFireMode].RunAnimation);
					}
					else
					{
						if (ActNotEqualIdle)
							SendWeaponAnim(SetIdleActivity());
					}
				}
			}
			else if (GetOwnerIsRunning() && !IsNearWall()/*cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0*/)
			{
				if (GetWpnData().animData[m_bFireMode].RunFullEmpty != ACT_RESET && !HasPrimaryAmmo())
				{
					if (GetActivity() != GetWpnData().animData[m_bFireMode].RunFullEmpty)
						SendWeaponAnim(GetWpnData().animData[m_bFireMode].RunFullEmpty);
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].RunAnimation != ACT_RESET)
					{
						if (GetActivity() != GetWpnData().animData[m_bFireMode].RunAnimation)
							SendWeaponAnim(GetWpnData().animData[m_bFireMode].RunAnimation);
					}
					else
					{
						if (ActNotEqualIdle)
							SendWeaponAnim(SetIdleActivity());
					}
				}
			}
			else if (!GetOwnerIsRunning() && !IsNearWall()/*cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0*/)
			{
				if (GetWpnData().animData[m_bFireMode].WalkFullEmpty != ACT_RESET && !HasPrimaryAmmo())
				{
					if (GetActivity() != GetWpnData().animData[m_bFireMode].WalkFullEmpty)
						SendWeaponAnim(GetWpnData().animData[m_bFireMode].WalkFullEmpty);
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].WalkAnimation != ACT_RESET)
					{
						if (GetActivity() != GetWpnData().animData[m_bFireMode].WalkAnimation)
							SendWeaponAnim(GetWpnData().animData[m_bFireMode].WalkAnimation);
					}
					else
					{
						if (ActNotEqualIdle)
							SendWeaponAnim(SetIdleActivity());
					}
				}
			}
		}
		else
		{
			if (IsNearWall())//(cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1)
			{
				if (GetWpnData().animData[m_bFireMode].WallFullEmpty != ACT_RESET && !HasPrimaryAmmo())
				{
					if (GetActivity() != GetWpnData().animData[m_bFireMode].WallFullEmpty)
						SendWeaponAnim(GetWpnData().animData[m_bFireMode].WallFullEmpty);
				}
				else
				{
					if (GetWpnData().animData[m_bFireMode].WallAnimation != ACT_RESET)
					{
						if (GetActivity() != GetWpnData().animData[m_bFireMode].WallAnimation)
							SendWeaponAnim(GetWpnData().animData[m_bFireMode].WallAnimation);
					}
					else
					{
						if (ActNotEqualIdle)
							SendWeaponAnim(SetIdleActivity());
					}
				}
			}
			else
			{
				if (ActNotEqualIdle)
					SendWeaponAnim(SetIdleActivity());
			}
		}
	}
}
//=========================================================
Activity CBaseCombatWeapon::GetPrimaryAttackActivity(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return ACT_VM_PRIMARYATTACK;

	AnimationData AnimData = GetWpnData().animData[m_bFireMode];

	if (pPlayer)
	{
		if (m_bFireMode <= 1 && (AnimData.UsePistolRefireChecker || (thisType == TYPE_PISTOL)))
		{
			if ((gpGlobals->curtime - m_flLastAttackTime) <= 0.5f)
				m_nShotsFired++;
		}
		else
			m_nShotsFired++;

		m_flLastAttackTime = gpGlobals->curtime;

		if (IsIronSighted() && !IsScopeSighted())
		{
			if (Clip1() == 2 && AnimData.FireIronsightedTwoRounds != 0)
				return (Activity)AnimData.FireIronsightedTwoRounds;

			if (Clip1() == 1 && AnimData.FireIronsightedCompleteEmpty != 0)
				return (Activity)AnimData.FireIronsightedCompleteEmpty;

			if (m_nShotsFired < 2 && AnimData.FireIronsighted1 != 0)
				return (Activity)AnimData.FireIronsighted1;

			if (m_nShotsFired < 3 && AnimData.FireIronsighted2 != 0)
				return (Activity)AnimData.FireIronsighted2;

			if (m_nShotsFired < 4 && AnimData.FireIronsighted3 != 0)
				return (Activity)AnimData.FireIronsighted3;

			if (m_nShotsFired >= 4 && AnimData.FireIronsighted4 != 0)
				return (Activity)AnimData.FireIronsighted4;

		}
		else if (IsScopeSighted() && !IsIronSighted())
		{
			if (Clip1() == 2 && AnimData.FireScopedTwoRounds != 0)
				return (Activity)AnimData.FireScopedTwoRounds;

			if (Clip1() == 1 && AnimData.FireScopedCompleteEmpty != 0)
				return (Activity)AnimData.FireScopedCompleteEmpty;

			if (m_nShotsFired < 2 && AnimData.FireScoped1 != 0)
				return (Activity)AnimData.FireScoped1;

			if (m_nShotsFired < 3 && AnimData.FireScoped2 != 0)
				return (Activity)AnimData.FireScoped2;

			if (m_nShotsFired < 4 && AnimData.FireScoped3 != 0)
				return (Activity)AnimData.FireScoped3;

			if (m_nShotsFired >= 4 && AnimData.FireScoped4 != 0)
				return (Activity)AnimData.FireScoped4;

		}
		else if (!IsIronSighted() && !IsScopeSighted())
		{
			if (Clip1() == 2 && AnimData.FirePrimaryTwoRounds != 0)
				return (Activity)AnimData.FirePrimaryTwoRounds;

			if (Clip1() == 1 && AnimData.FirePrimaryCompleteEmpty != 0)
				return (Activity)AnimData.FirePrimaryCompleteEmpty;

			if (m_nShotsFired < 2 && AnimData.FirePrimary1 != 0)
				return (Activity)AnimData.FirePrimary1;

			if (m_nShotsFired < 3 && AnimData.FirePrimary2 != 0)
				return (Activity)AnimData.FirePrimary2;

			if (m_nShotsFired < 4 && AnimData.FirePrimary3 != 0)
				return (Activity)AnimData.FirePrimary3;

			if (m_nShotsFired >= 4 && AnimData.FirePrimary4 != 0)
				return (Activity)AnimData.FirePrimary4;

		}
		else
			return ACT_VM_PRIMARYATTACK;
	}

	return ACT_VM_PRIMARYATTACK;
}

bool CBaseCombatWeapon::IsNonAttachmentActivity(Activity Act)
{
	const int arrCount = 98;
	int attActivites[arrCount] =
	{
		ACT_VM_PRIMARYATTACK,
		GetWpnData().animData[m_bFireMode].SwingAnim,

		GetWpnData().animData[m_bFireMode].FirePrimaryTwoRounds,
		GetWpnData().animData[m_bFireMode].FirePrimaryCompleteEmpty,
		GetWpnData().animData[m_bFireMode].FirePrimary1,
		GetWpnData().animData[m_bFireMode].FirePrimary2,
		GetWpnData().animData[m_bFireMode].FirePrimary3,
		GetWpnData().animData[m_bFireMode].FirePrimary4,
		GetWpnData().animData[m_bFireMode].FireScopedTwoRounds,
		GetWpnData().animData[m_bFireMode].FireScopedCompleteEmpty,
		GetWpnData().animData[m_bFireMode].FireScoped1,
		GetWpnData().animData[m_bFireMode].FireScoped2,
		GetWpnData().animData[m_bFireMode].FireScoped3,
		GetWpnData().animData[m_bFireMode].FireScoped4,
		GetWpnData().animData[m_bFireMode].FireIronsightedTwoRounds,
		GetWpnData().animData[m_bFireMode].FireIronsightedCompleteEmpty,
		GetWpnData().animData[m_bFireMode].FireIronsighted1,
		GetWpnData().animData[m_bFireMode].FireIronsighted2,
		GetWpnData().animData[m_bFireMode].FireIronsighted3,
		GetWpnData().animData[m_bFireMode].FireIronsighted4,
		GetWpnData().animData[m_bFireMode].MeleeAttack1,
		GetWpnData().animData[m_bFireMode].MeleeAttack2,
		GetWpnData().animData[m_bFireMode].MeleeSwing1,
		GetWpnData().animData[m_bFireMode].MeleeSwing2,
		GetWpnData().animData[m_bFireMode].MeleeMiss1,
		GetWpnData().animData[m_bFireMode].MeleeMiss2,
		GetWpnData().animData[m_bFireMode].ShotgunFireAnimation,
		GetWpnData().animData[m_bFireMode].FireScopedSilenced1,
		GetWpnData().animData[m_bFireMode].FireScopedSilenced2,
		GetWpnData().animData[m_bFireMode].FireScopedSilenced3,
		GetWpnData().animData[m_bFireMode].FireScopedSilenced4,
		GetWpnData().animData[m_bFireMode].FireIronsightedSilenced1,
		GetWpnData().animData[m_bFireMode].FireIronsightedSilenced2,
		GetWpnData().animData[m_bFireMode].FireIronsightedSilenced3,
		GetWpnData().animData[m_bFireMode].FireIronsightedSilenced4,
		GetWpnData().animData[m_bFireMode].FirePrimarySilenced1,
		GetWpnData().animData[m_bFireMode].FirePrimarySilenced2,
		GetWpnData().animData[m_bFireMode].FirePrimarySilenced3,
		GetWpnData().animData[m_bFireMode].FirePrimarySilenced4,
		ACT_GAUSS_SPINCYCLE,
		ACT_GAUSS_SPINUP,
		ACT_VM_PRIMARYATTACK,
		ACT_VM_SECONDARYATTACK,
		ACT_VM_SECONDARYATTACK_SPECIAL,
		ACT_VM_PRIMARYATTACK_SPECIAL,
		ACT_VM_PRIMARYATTACK_MIDEMPTY,
		ACT_VM_SHOOTLAST,
		ACT_VM_SHOOTLAST_IRONSIGHTED,
		ACT_VM_PRIMARYATTACK_1,
		ACT_VM_PRIMARYATTACK_2,
		ACT_VM_PRIMARYATTACK_3,
		ACT_VM_PRIMARYATTACK_4,
		ACT_VM_PRIMARYATTACK_5,
		ACT_VM_PRIMARYATTACK_6,
		ACT_VM_PRIMARYATTACK_7,
		ACT_VM_PRIMARYATTACK_8,
		ACT_VM_PRIMARYATTACK_DEPLOYED,
		ACT_VM_PRIMARYATTACK_DEPLOYED_1,
		ACT_VM_PRIMARYATTACK_DEPLOYED_2,
		ACT_VM_PRIMARYATTACK_DEPLOYED_3,
		ACT_VM_PRIMARYATTACK_DEPLOYED_4,
		ACT_VM_PRIMARYATTACK_DEPLOYED_5,
		ACT_VM_PRIMARYATTACK_DEPLOYED_6,
		ACT_VM_PRIMARYATTACK_DEPLOYED_7,
		ACT_VM_PRIMARYATTACK_DEPLOYED_8,
		ACT_VM_PRIMARYATTACK_DEPLOYED_EMPTY,
		ACT_VM_PRIMARYATTACK_EMPTY,
		ACT_VM_PRIMARYATTACK_IRONSIGHTED1,
		ACT_VM_PRIMARYATTACK_IRONSIGHTED2,
		ACT_VM_PRIMARYATTACK_IRONSIGHTED3,
		ACT_VM_PRIMARYATTACK_MIDEMPTY,
		ACT_VM_PRIMARYATTACK_MIDEMPTY_IRONSIGHTED,
		ACT_VM_PRIMARYATTACK_QRL,
		ACT_VM_PRIMARYATTACK_SILENCED,
		ACT_VM_RECOIL1,
		ACT_VM_RECOIL2,
		ACT_VM_RECOIL3,
		GetWpnData().animData[m_bFireMode].FireSecondary1,
		GetWpnData().animData[m_bFireMode].FireSecondary2,
		GetWpnData().animData[m_bFireMode].FireSecondary3,
		GetWpnData().animData[m_bFireMode].FireSecondary4,
		GetWpnData().animData[m_bFireMode].FireSecondaryLast,
		GetWpnData().animData[m_bFireMode].FireSecondaryCharging,
		GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted1,
		GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted2,
		GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted3,
		GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted4,


		ACT_VM_DRAW,
		GetWpnData().animData[m_bFireMode].DrawEmptyAnimation,
		GetWpnData().animData[m_bFireMode].DrawAnimation,
		GetWpnData().animData[m_bFireMode].DrawEmptySilencedAnimation,
		GetWpnData().animData[m_bFireMode].DrawSilencedAnimation,

		ACT_VM_HOLSTER,
		GetWpnData().animData[m_bFireMode].HolsterFullEmpty,
		GetWpnData().animData[m_bFireMode].HolsterAnimation,
		GetWpnData().animData[m_bFireMode].HolsterMode1Animation,
		GetWpnData().animData[m_bFireMode].HolsterMode2Animation,
		GetWpnData().animData[m_bFireMode].HolsterMode3Animation
	};

	for (int i = 0; i < arrCount; i++)
	{
		if (Act == attActivites[i]) return true;
	}

	return false;
}

//=========================================================
Activity CBaseCombatWeapon::GetSecondaryAttackActivity(void)
{
	MDLCACHE_CRITICAL_SECTION();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return ACT_VM_SECONDARYATTACK;
	}
	else
	{
		if (GetSecondaryAmmoType() >= 0 && pPlayer->GetAmmoCount(GetSecondaryAmmoType()) <= 0)
			return ACT_VM_DRYFIRE;

		if (!m_bAltFiresUnderwater && pPlayer->GetWaterLevel() == 3)
			return ACT_VM_DRYFIRE;

		if (pPlayer->GetAmmoCount(GetSecondaryAmmoType()) == 1)
		{
			if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryLast != 0)
				return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryLast;
		}

		if (IsIronSighted() || IsScopeSighted())
		{
			int sequence = random->RandomInt(0, 3);

			switch (sequence)
			{
			case 0:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted1 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted1;
			}
			break;
			case 1:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted2 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted2;
			}
			break;
			case 2:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted3 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted3;
			}
			break;
			case 3:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted4 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondaryIronsighted4;
			}
			break;
			}
		}
		else
		{
			int sequence = random->RandomInt(0, 3);

			switch (sequence)
			{
			case 0:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary1 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary1;
			}
			break;
			case 1:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary2 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary2;
			}
			break;
			case 2:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary3 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary3;
			}
			break;
			case 3:
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary4 != 0)
					return (Activity)pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].FireSecondary4;
			}
			break;
			}
		}
	}

	return ACT_VM_SECONDARYATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: Adds in view kick and weapon accuracy degradation effect
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AddViewKick(void)
{
	//NOTENOTE: By default, weapon will not kick up (defined per weapon)
#ifndef CLIENT_DLL

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (!pPlayer->GetActiveWeapon()->GetWpnData().useHl2KickSystem)
	{
		if (pPlayer)
		{
			pPlayer->ViewPunchReset();
		}

		QAngle angles = pPlayer->GetLocalAngles();

		float koef = Clamp(pPlayer->GetActiveWeapon()->GetWpnData().ironOrScopeKoefficient, 0.f, 1.f);

		if (!pPlayer->GetActiveWeapon()->GetWpnData().AutoReduce)
		{
			float X = pPlayer->GetActiveWeapon()->GetWpnData().ViewPunch_X;
			float Y = pPlayer->GetActiveWeapon()->GetWpnData().ViewPunch_Y;
			float Z = pPlayer->GetActiveWeapon()->GetWpnData().ViewPunch_Z;

			if (pPlayer->m_Local.m_bDucked || pPlayer->m_Local.m_bDucking || IsIronSighted() || IsScopeSighted())
			{
				X *= koef;
				Y *= koef;
				Z *= koef;
			}

			angles.x += (pPlayer->GetActiveWeapon()->GetWpnData().DoRandomPunch != 0) ? random->RandomFloat(-X, X) /*- pPlayer->GetPunchAngle().x*/ : -X /*- pPlayer->GetPunchAngle().x*/; //random->RandomInt(-1.1, 1.1);
			angles.y += random->RandomFloat(-Y, Y);// - pPlayer->GetPunchAngle().y;//random->RandomInt(-1.1, 1.1);//0.45
			angles.z += random->RandomFloat(-Z, Z);// -pPlayer->GetPunchAngle().z;

			//angles.x = Clamp(angles.x, -89.f, 89.f);

			pPlayer->SnapEyeAngles(angles);

			if (pPlayer->GetActiveWeapon()->GetWpnData().AllowFreeAim == 1)
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().DoRandomPunch == 0)
				{
					cvar->FindVar("oc_recoil_x")->SetValue(random->RandomFloat(-Y / 50, Y / 50));//(-X / 70);
					cvar->FindVar("oc_recoil_y")->SetValue(-X / 50);//(random->RandomFloat(-Y / 70, Y / 70));
				}
				else
				{
					cvar->FindVar("oc_recoil_x")->SetValue(random->RandomFloat(-X / 50, X / 50));
					cvar->FindVar("oc_recoil_y")->SetValue(random->RandomFloat(-Y / 50, Y / 50));
				}
			}
		}

		float X2 = abs(pPlayer->GetActiveWeapon()->GetWpnData().ViewKick_X);
		float Y2 = pPlayer->GetActiveWeapon()->GetWpnData().ViewKick_Y;
		float Z2 = pPlayer->GetActiveWeapon()->GetWpnData().ViewKick_Z;

		/*if (abs(pPlayer->LocalEyeAngles().x) + X2 >= 89.0f)
		{
		X2 = 89.0f - abs(pPlayer->LocalEyeAngles().x);
		}*/

		if (pPlayer->m_Local.m_bDucked || pPlayer->m_Local.m_bDucking || IsIronSighted() || IsScopeSighted())
		{
			X2 *= koef;
			Y2 *= koef;
			Z2 *= koef;
		}


		if (!pPlayer->GetActiveWeapon()->GetWpnData().DoRandomKick)
			pPlayer->ViewPunch(QAngle(-X2, random->RandomFloat(-Y2, Y2), random->RandomFloat(-Z2, Z2)));
		else
			pPlayer->ViewPunch(QAngle(random->RandomFloat(-X2, X2), random->RandomFloat(-Y2, Y2), random->RandomFloat(-Z2, Z2)));

	}
	else
	{
		//if (thisType != TYPE_PISTOL && thisType != TYPE_SHOTGUN && thisType != TYPE_ANNABELLE && thisType != TYPE_DUAL_PISTOLS && thisType != TYPE_RPG && thisType != TYPE_GRENADE && thisType != TYPE_MELEE)
		{
			//Machineguns only

			float EASY_DAMPEN = pPlayer->GetActiveWeapon()->GetWpnData().easyDampen;
			float MAX_VERTICAL_KICK = pPlayer->GetActiveWeapon()->GetWpnData().maxVerticalKick;	//Degrees
			float SLIDE_LIMIT = pPlayer->GetActiveWeapon()->GetWpnData().slideLimit;	//Seconds

			float KICK_MIN_X = 0.2f;	//Degrees
			float KICK_MIN_Y = 0.2f;	//Degrees
			float KICK_MIN_Z = 0.1f;	//Degrees
			float flDuration = m_fFireDuration;

			if (g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE)
			{
				flDuration = MIN(flDuration, 0.75f);
			}

			QAngle vecScratch;

			float duration = (flDuration > SLIDE_LIMIT) ? SLIDE_LIMIT : flDuration;
			float kickPerc = duration / SLIDE_LIMIT;

			pPlayer->ViewPunchReset(10);

			vecScratch.x = -(KICK_MIN_X + (MAX_VERTICAL_KICK * kickPerc));
			vecScratch.y = -(KICK_MIN_Y + (MAX_VERTICAL_KICK * kickPerc)) / 3;
			vecScratch.z = KICK_MIN_Z + (MAX_VERTICAL_KICK * kickPerc) / 8;

			if (random->RandomInt(-1, 1) >= 0)
				vecScratch.y *= -1;

			if (random->RandomInt(-1, 1) >= 0)
				vecScratch.z *= -1;

			if (g_pGameRules->IsSkillLevel(SKILL_EASY))
			{
				for (int i = 0; i < 3; i++)
				{
					vecScratch[i] *= EASY_DAMPEN;
				}
			}

			UTIL_ClipPunchAngleOffset(vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle(24.0f, 3.0f, 1.0f));

			QAngle punchAng = vecScratch * 0.5;

			//punchAng.x = Clamp(punchAng.x, -89.f, 89.f);

			pPlayer->ViewPunch(punchAng);


		}
		/*else if (thisType == TYPE_MELEE && thisType != TYPE_PISTOL && thisType != TYPE_SHOTGUN && thisType != TYPE_ANNABELLE && thisType != TYPE_DUAL_PISTOLS && thisType != TYPE_RPG && thisType != TYPE_GRENADE && thisType != TYPE_MELEE)
		{
		//MELLE only

		}
		else
		{
		//Pistols, shotguns only


		}*/


		if (pPlayer->GetActiveWeapon()->GetWpnData().AllowFreeAim == 1)
		{
			float X = pPlayer->GetActiveWeapon()->GetWpnData().ViewPunch_X;
			float Y = pPlayer->GetActiveWeapon()->GetWpnData().ViewPunch_Y;

			if (pPlayer->m_Local.m_bDucked || pPlayer->m_Local.m_bDucking || IsIronSighted() || IsScopeSighted())
			{
				X *= pPlayer->GetActiveWeapon()->GetWpnData().ironOrScopeKoefficient;
				Y *= pPlayer->GetActiveWeapon()->GetWpnData().ironOrScopeKoefficient;
			}

			if (pPlayer->GetActiveWeapon()->GetWpnData().DoRandomPunch == 0)
			{
				cvar->FindVar("oc_recoil_x")->SetValue(random->RandomFloat(-Y / 50, Y / 50));
				cvar->FindVar("oc_recoil_y")->SetValue(-X / 50);
			}
			else
			{
				cvar->FindVar("oc_recoil_x")->SetValue(random->RandomFloat(-X / 50, X / 50));
				cvar->FindVar("oc_recoil_y")->SetValue(random->RandomFloat(-Y / 50, Y / 50));
			}
		}
	}

	float vel = pPlayer->GetActiveWeapon()->GetWpnData().recoilVelocity;
	if (vel > 0)
	{
		Vector recoilForce = pPlayer->GetAbsVelocity() - pPlayer->GetAutoaimVector(0) * vel;
		pPlayer->SetAbsVelocity(recoilForce); // отдача на тело игрока
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose: Get the string to print death notices with
//-----------------------------------------------------------------------------
char *CBaseCombatWeapon::GetDeathNoticeName(void)
{
#if !defined( CLIENT_DLL )
	return (char*)STRING(m_iszName);
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

void CBaseCombatWeapon::CheckReloadState(CBasePlayer *pOwner)
{
	MDLCACHE_CRITICAL_SECTION();

	if (thisType == TYPE_SHOTGUN)
	{
		if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonPressed & IN_ATTACK2))
		{
			if (m_iClip1 > 0 && m_iShotgunReloadState == 2)
			{
				if (m_bFireMode == 2)
				{
					m_iShotgunReloadState = 3;
				}
				else
				{
					reloadFromEmpty = false;
					m_iShotgunReloadState = 5;
				}
			}
		}

		if (m_iClip1 <= 0 && m_iShotgunReloadState == 0)
		{
			reloadFromEmpty = true;

			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
				m_iShotgunReloadState = 1;
		}

		if (m_bFireMode == 2)
		{
			if ((pOwner->m_nButtons & IN_RELOAD) && m_iShotgunReloadState == 0 && m_iClip1 < GetMaxClip1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0 && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (m_iClip1 > 0)
					reloadFromEmpty = false;

				m_iShotgunReloadState = 1;
			}
		}
		else
		{
			if ((pOwner->m_nButtons & IN_RELOAD) && m_iShotgunReloadState == 0 && m_iClip1 < GetMaxClip1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0 && m_flNextShotgunReload < gpGlobals->curtime)
			{
				if (m_iClip1 > 0)
					reloadFromEmpty = false;

				m_iShotgunReloadState = 1;
			}
		}




		if (m_bFireMode == 2)
		{
			if (m_bNeedPump)//NoNeed pump while auto
				m_bNeedPump = false;

			if (gpGlobals->curtime > /*m_flNextPrimaryAttack*/ m_flNextShotgunReload && m_iClip1 <= GetMaxClip1())
			{

				if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
				{
					if (m_iShotgunReloadState > 0 && m_iShotgunReloadState <= 2)
						m_iShotgunReloadState = reloadFromEmpty ? 3 : 5;
				}

				if (m_iShotgunReloadState == 1)
					StartReloadShotgun();
				else if (m_iShotgunReloadState == 2)
					ReloadShotgun();
				else if (m_iShotgunReloadState == 3)
					FinishReloadShotgun(reloadFromEmpty);//true
				else if (m_iShotgunReloadState == 4)
				{
					if (reloadFromEmpty)
						PumpShotgun();
					else
						m_iShotgunReloadState = 5;
				}
				else if (m_iShotgunReloadState == 5)
					FinishReloadShotgun(reloadFromEmpty);//false

				/*if (m_iShotgunReloadState == 1)
				StartReloadShotgun();
				else if (m_iShotgunReloadState == 2)
				ReloadShotgun();
				else if (m_iShotgunReloadState == 3)
				{
				FinishReloadShotgun(false);
				m_iShotgunReloadState = 0;
				m_bNeedPump = false;
				}*/
			}
		}
		else
		{
			if (m_bNeedPump && gpGlobals->curtime > m_flNextShotgunReload &&
				m_iShotgunReloadState == 0 &&
				m_iClip1 > 0)
			{
				reloadFromEmpty = false;
				m_iShotgunReloadState = 4;
			}
			else if (gpGlobals->curtime > m_flNextShotgunReload && m_iClip1 <= GetMaxClip1())
			{
				if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
				{
					if (m_iShotgunReloadState > 0 && m_iShotgunReloadState <= 2)
						m_iShotgunReloadState = reloadFromEmpty ? 3 : 5;
				}

				if (m_iShotgunReloadState == 1)
					StartReloadShotgun();
				else if (m_iShotgunReloadState == 2)
					ReloadShotgun();
				else if (m_iShotgunReloadState == 3)
					FinishReloadShotgun(reloadFromEmpty);//true
				else if (m_iShotgunReloadState == 4)
					PumpShotgun();
				else if (m_iShotgunReloadState == 5)
					FinishReloadShotgun(reloadFromEmpty);//false
			}
		}
	}
	else
	{
		if (GetWpnData().allowAmmoRegen)
			RechargeAmmo();

		if (IsInReload() && UsesClipsForAmmo1() && m_flNextPrimaryAttack < gpGlobals->curtime /*&& !WeaponShouldBeLowered()*/)
		{
			CheckReload();
		}
		/*if (gpGlobals->curtime > m_flNextShotgunReload && UsesClipsForAmmo1())
		{
			CheckReload();
		}*/
	}
}

//====================================================================================
// WEAPON RELOAD TYPES
//====================================================================================
void CBaseCombatWeapon::CheckReload(void)
{
	if (m_bReloadsSingly)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;

		if (m_bInReload &&
			GetActivity() != ACT_VM_ATTACH_SILENCER &&
			GetActivity() != ACT_VM_DETACH_SILENCER &&
			m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			if (pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0)
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				if (GetWpnData().allowLoopSound && m_bIsFiring)
					StopLoopSound();

				if (m_bIsInPrimaryAttack && m_flNextPrimaryAttack < gpGlobals->curtime)
					m_bIsInPrimaryAttack = false;

				// Add them to the clip
				m_iClip1 += 1;
				//pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
				RemoveAmmo(GetPrimaryAmmoType(), 1);

				Reload();

				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();

				m_flNextPrimaryAttack = gpGlobals->curtime;
				m_flNextSecondaryAttack = gpGlobals->curtime;
				return;
			}
		}
	}
	else
	{
		//if (GetActivity() != SetIdleActivity(true))//if ((m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			FinishReload();

			m_flNextPrimaryAttack = 0;
			m_flNextSecondaryAttack = 0;
			m_bInReload = false;
		}
	}
}

void CBaseCombatWeapon::CheckReloadShotgun(void)
{
	if (m_bReloadsSingly)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;

		if ((m_iShotgunReloadState != 0) && (gpGlobals->curtime > m_flNextShotgunReload))
		{
			if (pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0)
			{
				m_bInReload = false;
				m_iShotgunReloadState = 0;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				FinishReloadShotgun(1);
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				if (m_iShotgunReloadState == 1)
					ReloadShotgun();

				return;
			}
			// Clip full, stop reloading
			else
			{
				if (m_iShotgunReloadState == 2)
					FinishReloadShotgun(1);
				else if (m_iShotgunReloadState == 3)
					PumpShotgun();

				m_flNextPrimaryAttack = gpGlobals->curtime;
				m_flNextSecondaryAttack = gpGlobals->curtime;
				return;
			}
		}
	}
	else
	{
		if ((m_bInReload && m_iShotgunReloadState != 0) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			FinishReloadShotgun(1);

			m_flNextPrimaryAttack = gpGlobals->curtime;
			m_flNextSecondaryAttack = gpGlobals->curtime;
			m_flNextShotgunReload = gpGlobals->curtime;
			m_iShotgunReloadState = 0;
			m_bInReload = false;
		}
	}
}

void CBaseCombatWeapon::PumpShotgun(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bNeedPump = false;

	//DevMsg("PumpAfterTwoBarrels: %i \n", (int)PumpAfterTwoBarrels);

	if (PumpAfterTwoBarrels)
	{
		PumpAfterTwoBarrels = false;

		if (GetWpnData().animData[m_bFireMode].ShotgunDoublePumpAnimation == ACT_RESET)
		{
			m_iShotgunReloadState = 0;
			return;
		}

		SendWeaponAnim(GetWpnData().animData[m_bFireMode].ShotgunDoublePumpAnimation);
	}
	else
	{
		if (GetWpnData().animData[m_bFireMode].ShotgunPumpAnimation == ACT_RESET)
		{
			m_iShotgunReloadState = 0;
			return;
		}

		SendWeaponAnim(GetWpnData().animData[m_bFireMode].ShotgunPumpAnimation);
	}

	WeaponSound(SPECIAL1);

	pOwner->m_flNextAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextShotgunReload = gpGlobals->curtime + GetViewModelSequenceDuration();

	m_iShotgunReloadState = 0;

}
bool CBaseCombatWeapon::StartReloadShotgun(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	if (m_iClip1 <= 0)
	{
		m_bNeedPump = true;
	}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	m_bInReload = true;
	//SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	if (GetWpnData().animData[m_bFireMode].ShotgunStartReloadAnimation == ACT_RESET)
	{
		SetBodygroup(1, 0);

		m_iShotgunReloadState = 2;

		return true;
	}

	SendWeaponAnim(GetWpnData().animData[m_bFireMode].ShotgunStartReloadAnimation);

	SetBodygroup(1, 0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextShotgunReload = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_iShotgunReloadState = 2;

	return true;
}
bool CBaseCombatWeapon::ReloadShotgun(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
	{
		m_iShotgunReloadState = 3;
		return false;
	}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	//FillClip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
		}
	}

	if (GetWpnData().animData[m_bFireMode].ShotgunFillClipAnimation == ACT_RESET)
	{
		m_flNextShotgunReload = gpGlobals->curtime + 0.2f;

		return true;
	}
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);

	SendWeaponAnim(GetWpnData().animData[m_bFireMode].ShotgunFillClipAnimation);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextShotgunReload = gpGlobals->curtime + GetViewModelSequenceDuration();

	return true;
}
void CBaseCombatWeapon::FinishReloadShotgun(bool needPump)
{
	SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	//SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	if (GetWpnData().animData[m_bFireMode].ShotgunStopReloadAnimation == ACT_RESET)
	{
		if (needPump)
			m_iShotgunReloadState = 4;
		else
			m_iShotgunReloadState = 0;

		return;
	}

	SendWeaponAnim(GetWpnData().animData[m_bFireMode].ShotgunStopReloadAnimation);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextShotgunReload = gpGlobals->curtime + GetViewModelSequenceDuration();

	if (needPump)
		m_iShotgunReloadState = 4;
	else
		m_iShotgunReloadState = 0;
}
//-----------------------------------------------------------------------------
// Purpose: Reload has finished.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FinishReload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner)
	{
		// If I use primary clips, reload primary
		if (UsesClipsForAmmo1())
		{
			int primary = MIN(GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
			m_iClip1 += primary;
			//pOwner->RemoveAmmo( primary, m_iPrimaryAmmoType);
			RemoveAmmo(GetPrimaryAmmoType(), primary);
		}

		// If I use secondary clips, reload secondary
		if (UsesClipsForAmmo2())
		{
			int secondary = MIN(GetMaxClip2() - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
			m_iClip2 += secondary;
			//pOwner->RemoveAmmo( secondary, m_iSecondaryAmmoType );
			RemoveAmmo(GetSecondaryAmmoType(), secondary);
		}

		if (m_bReloadsSingly)
		{
			m_bInReload = false;
		}
	}
}

void CBaseCombatWeapon::RemoveAmmo(int ammoType, int count)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	if (GetWpnData().allowAmmoRegen)
	{
		if (GetAmmoCountForRecharge() > 0)
		{
			if (GetWpnData().ammoRemoveDelay < 0)
			{
				m_flNextAmmoRegen = gpGlobals->curtime + GetWpnData().ammoRegenDelay;
				RemoveRechargedAmmo();
				return;
			}

			if (m_flNextAmmoRemove < gpGlobals->curtime && GetAmmoCountForRecharge() > 0)
			{
				m_flNextAmmoRemove = gpGlobals->curtime + GetWpnData().ammoRemoveDelay;
				RemoveRechargedAmmo();
			}
			if (GetAmmoCountForRecharge() == 0)
			{
				m_flNextAmmoRemove = gpGlobals->curtime;
				m_flNextAmmoRegen = gpGlobals->curtime + GetWpnData().ammoRemoveDelay * 0.5f;
				//m_flNextAmmoRegen = m_flNextShotgunReload = m_flNextSecondaryAttack = m_flNextPrimaryAttack;
			}
		}
	}
	else
	{
		if (thisType == TYPE_SHOTGUN || thisType == TYPE_ANNABELLE)
		{
			if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt())
			{
				if (GetDefaultClip1() > 0)
					m_iClip1 -= count;
			}
			else
				m_iClip1 -= count;
		}
		else
		{
			/*if (delay < 0)
			return;

			if (delay > 0)
			{
			if (m_flNextRemoveAmmo < gpGlobals->curtime)
			{
			m_flNextRemoveAmmo = gpGlobals->curtime + delay;
			pPlayer->RemoveAmmo(count, ammoType);
			}
			}
			else*/
			pPlayer->RemoveAmmo(count, ammoType);
		}
	}
}

int CBaseCombatWeapon::GetMaxAmmoCountForRecharge()
{
	if (IsClippedAmmo())
		return GetMaxClip1();
	else
		return GetDefaultClip1();
}

int CBaseCombatWeapon::GetAmmoCountForRecharge()
{
	/*if (IsClippedAmmo())
	return Clip1();// m_iClip1
	else
	return GetPrimaryAmmoCount();*///m_iPrimaryAmmoCount


	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
		return pOwner->GetAmmoCount(GetPrimaryAmmoType());

	return -1;
}
void CBaseCombatWeapon::RemoveRechargedAmmo()
{
#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
		pOwner->RemoveAmmo(1, GetPrimaryAmmoType());
#endif
	/*if (IsClippedAmmo())
	m_iClip1--;
	else
	m_iPrimaryAmmoCount--;*/
}
void CBaseCombatWeapon::AddRechargedAmmo()
{
#ifndef CLIENT_DLL
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
		pOwner->GiveAmmo(1, GetPrimaryAmmoType(), true);
#endif
	/*if (IsClippedAmmo())
	m_iClip1++;
	else
	m_iPrimaryAmmoCount++;*/
}
//-----------------------------------------------------------------------------
// Recharges the ammo...
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::RechargeAmmo(void)
{
	if (m_flNextAmmoRegen < gpGlobals->curtime && GetAmmoCountForRecharge() < GetMaxAmmoCountForRecharge())
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner && ((pOwner->m_nButtons & IN_ATTACK) || (GetWpnData().m_bUsePrimaryAmmoAsSecondary ? (pOwner->m_nButtons & IN_ATTACK2) : false)))
		{
			m_flNextAmmoRegen = gpGlobals->curtime + 0.2f;
			return;
		}

		if (GetWpnData().ammoEmptyDelay < 0)
		{
			m_flNextAmmoRegen = gpGlobals->curtime + GetWpnData().ammoRegenDelay;
			AddRechargedAmmo();// m_iClip1++;

			if (!IsHolstered())
				WeaponSound(SPECIAL3);

			return;
		}

		if ((GetAmmoCountForRecharge() <= 0 || !GetMaxAmmoCountForRecharge()) && !blockToAmmoRegen)
		{
			m_flNextAmmoRegen = gpGlobals->curtime + GetWpnData().ammoEmptyDelay;
			blockToAmmoRegen = true;
		}
		else
		{
			if (blockToAmmoRegen)
				blockToAmmoRegen = false;

			m_flNextAmmoRegen = gpGlobals->curtime + GetWpnData().ammoRegenDelay;
			AddRechargedAmmo();//m_iClip1++;

			if (!IsHolstered())
				WeaponSound(SPECIAL3);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Abort any reload we have in progress
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AbortReload(void)
{
#ifdef CLIENT_DLL
	StopWeaponSound(RELOAD);
#endif
	m_bInReload = false;
}

void CBaseCombatWeapon::UpdateAutoFire(void)
{
	if (!AutoFiresFullClip())
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (m_iClip1 == 0)
	{
		// Ready to reload again
		m_bFiringWholeClip = false;
	}

	if (m_bFiringWholeClip)
	{
		// If it's firing the clip don't let them repress attack to reload
		pOwner->m_nButtons &= ~IN_ATTACK;
	}

	// Don't use the regular reload key
	if (pOwner->m_nButtons & IN_RELOAD)
	{
		pOwner->m_nButtons &= ~IN_RELOAD;
	}

	// Try to fire if there's ammo in the clip and we're not holding the button
	bool bReleaseClip = m_iClip1 > 0 && !(pOwner->m_nButtons & IN_ATTACK);

	if (!bReleaseClip)
	{
		if (CanReload() && (pOwner->m_nButtons & IN_ATTACK))
		{
			// Convert the attack key into the reload key
			pOwner->m_nButtons |= IN_RELOAD;
		}

		// Don't allow attack button if we're not attacking
		pOwner->m_nButtons &= ~IN_ATTACK;
	}
	else
	{
		// Fake the attack key
		pOwner->m_nButtons |= IN_ATTACK;
	}
}

void CBaseCombatWeapon::FireClientPrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

#ifndef CLIENT_DLL
	CEffectData	fireData;
	fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
	fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
	fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
	fireData.m_nAttachmentIndex = 21;//AE_MUZZLEFLASH
	DispatchEffect("FireEvent", fireData);
#endif
}

void CBaseCombatWeapon::FireClientSecondaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

#ifndef CLIENT_DLL
	CEffectData	fireData;
	fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
	fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
	fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
	fireData.m_nAttachmentIndex = 22;//AE_MUZZLEFLASH_SECONDARY
	DispatchEffect("FireEvent", fireData);
#endif
}

void CBaseCombatWeapon::FireClientDelayedAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

#ifndef CLIENT_DLL
	CEffectData	fireData;
	fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
	fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
	fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
	fireData.m_nAttachmentIndex = 23;//AE_MUZZLEFLASH_CHARGE
	DispatchEffect("FireEvent", fireData);
#endif
}

const Vector &CBaseCombatWeapon::GetBulletSpreadHolded()
{
	static Vector spread;
	spread.x = GetWpnData().standingSpread;
	spread.y = GetWpnData().standingSpread;
	spread.z = GetWpnData().standingSpread;

	return spread;
}

void CBaseCombatWeapon::PrimaryAttackHolding(CBaseCombatWeapon *pWeapon)
{
	if (!EnabledHoldingFire() || IsMarkedForDeletion())
	{
		EnableHoldingFire(false);
		if (pWeapon)
			pWeapon = NULL;

		return;
	}

	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		EnableHoldingFire(false);
		if (pWeapon)
			pWeapon = NULL;
		return;
	}

	Vector origin, orig2;

	QAngle ang;

	origin = EyePosition();

	Vector	forward;

	int attachment = LookupAttachment("muzzle");

	if (attachment == -1)
		GetAttachment("muzzle", orig2, ang);
	else
		GetAttachment(attachment, orig2, ang);

	//AngleVectors(EyeAngles(), &forward);
	AngleVectors(ang, &forward);

	forward *= 5.f;

	FireBulletsInfo_t info;
	info.m_bHoldedAttack = true;
	info.m_vecSrc = origin;
	info.m_vecSpread = GetBulletSpreadHolded();
	info.m_vecDirShooting = forward;
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	if (m_bSilenced)
	{
		WeaponSound(SPECIAL1);
	}
	else
	{
		WeaponSound(SINGLE_NPC);
	}
#ifndef CLIENT_DLL
	CEffectData	fireData;
	fireData.m_nEntIndex = entindex();
	DispatchEffect("MuzzleFlashHolding", fireData);
#endif
	m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
	info.m_iShots = 1;

	m_iClip1 -= info.m_iShots;

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;

	if (thisType == TYPE_SHOTGUN ||
		thisType == TYPE_AIRBOATGUN)
		info.m_iShots = NumShots();

	if (thisType != TYPE_BEAM)
	{
		if (thisType == TYPE_SHOTGUN)
			CBaseEntity::FireBulletsShotgun(info);
		else
			CBaseEntity::FireBullets(info);
	}
}
//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::PrimaryAttack(void)
{
	// If my clip is empty (and I use clips) start reload
	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}

	if (!m_iClip1 && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		Reload();
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (!m_bSilenced)
		pPlayer->DoMuzzleFlash();

	/*if (pPlayer && ((m_iClip1 <= (GetMaxClip1() / 4)) && m_iClip1 != 0 && m_iClip1 <= 8))
	WeaponSound(LOW_AMMO);*/

	Activity primaryAttack = iMuzzle == 2 ? GetSecondaryAttackActivity() : GetPrimaryAttackActivity();

	if (iMuzzle == 2)
		SendWeaponAnim(primaryAttack);
	else
		SendWeaponAnim(primaryAttack);

	attackActivites[0] = primaryAttack;


	pPlayer->SetAnimation(PLAYER_ATTACK1);

	FireBulletsInfo_t info;
	info.m_bHoldedAttack = false;
	info.m_vecSrc = pPlayer->EyePosition();//->Weapon_ShootPosition( );	
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	/*Vector vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
	info.m_vecSrc += vForward * 0.0f;
	info.m_vecSrc += vRight * 5.5f;
	info.m_vecSrc += vUp * -3.4f;
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
	info.m_vecSrc += vForward * 0.0f;
	info.m_vecSrc += vRight * 0.0f;
	info.m_vecSrc += vUp * -3.4f;
	}*/
	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.

	/*if (pPlayer->GetActiveWeapon()->m_iClip1 <= pPlayer->GetActiveWeapon()->GetMaxClip1() / 4 && pPlayer->GetActiveWeapon()->m_iClip1 <= 8)
	WeaponSound(LOW_AMMO);*/


	if (m_bFireMode == 3)
	{
		if (m_bSilenced)
		{
			WeaponSound(SPECIAL1);
		}
		else
		{
			if (!GetWpnData().allowLoopSound)
				WeaponSound(SINGLE);
		}
		/*while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
		if (!fireRate)
		break;*/
		/*if (m_nShotsFired % 3 != 0)
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		else
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate + GetWpnData().mode_fire_rate_burst_refire_time;*/
		burstFireCount++;
		if (burstFireCount >= GetWpnData().mode_fire_burst_count)
		{
			burstFireCount = 0;
			wasburstFire = false;
			m_flNextPrimaryAttack = gpGlobals->curtime + fireRate + GetWpnData().mode_fire_rate_burst_refire_time;
		}
		else
			m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		info.m_iShots = 1;
		//info.m_iShots++;
		//}
	}
	else if (m_bFireMode == 2)
	{
		if (m_bSilenced)
		{
			WeaponSound(SPECIAL1);
		}
		else
		{
			if (!GetWpnData().allowLoopSound)
				WeaponSound(SINGLE);
		}
		/*while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
		if (!fireRate)
		break;
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		info.m_iShots++;
		}*/
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		info.m_iShots = 1;
	}
	else if (m_bFireMode == 1)
	{
		m_flSoonestPrimaryAttack = gpGlobals->curtime + GetWpnData().mode_fastest_refire_time;
		m_flAccuracyPenalty += GetWpnData().mode_accuracy_shot_penalty_time;
		m_flLastAttackTime = gpGlobals->curtime;

		if (m_bSilenced)
		{
			WeaponSound(SPECIAL1);
		}
		else
		{
			WeaponSound(SINGLE);
		}
		/*while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
		if (!fireRate)
		break;
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;//m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		}*/
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		info.m_iShots = 1;
	}
	else if (m_bFireMode == 0)
	{
		m_flSoonestPrimaryAttack = gpGlobals->curtime + GetWpnData().mode_fastest_refire_time;
		m_flAccuracyPenalty += GetWpnData().mode_accuracy_shot_penalty_time;
		m_flLastAttackTime = gpGlobals->curtime;

		if (m_bSilenced)
		{
			WeaponSound(SPECIAL1);
		}
		else
		{
			WeaponSound(SINGLE);
		}
		/*while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
		if (!fireRate)
		break;
		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;//m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		}*/

		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
		info.m_iShots = 1;

		//m_flSoonestPrimaryAttack = gpGlobals->curtime + GetWpnData().mode_fastest_refire_time;
		//m_flAccuracyPenalty += GetWpnData().mode_accuracy_shot_penalty_time;
	}
	// Make sure we don't fire more than the amount in the clip
	if (UsesClipsForAmmo1())
	{
		info.m_iShots = MIN(info.m_iShots, m_iClip1);

		if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt() == 1)
		{
			if (GetDefaultClip1() > 0)
				m_iClip1 -= info.m_iShots;
		}
		else
			m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));

		//if (GetWpnData().allowAmmoRegen)
		RemoveAmmo(m_iPrimaryAmmoType, info.m_iShots);
		/*else
		pPlayer->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );*/
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL



#if !defined( CLIENT_DLL )

#endif // CLIENT_DLL

	//if (Q_strcmp(pPlayer->GetActiveWeapon()->GetWpnData().WeaponType, "shotgun") == 0 || 
	//Q_strcmp(pPlayer->GetActiveWeapon()->GetWpnData().WeaponType, "AirboatGun") == 0)
	if (thisType == TYPE_SHOTGUN ||
		thisType == TYPE_AIRBOATGUN)
		info.m_iShots = NumShots();

	if (thisType != TYPE_BEAM)
		pPlayer->FireBullets(info);

	/*if (thisType == TYPE_BEAM)
	{
	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
	// HEV suit - indicate out of ammo condition
	pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
	}
	else
	{
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && thisType != TYPE_AIRBOATGUN)
	{
	// HEV suit - indicate out of ammo condition
	pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
	}*/

	AddViewKick();

	if (GetWpnData().iMuzzleSmokeNumShots > 0)
	{
		if (NumShots() == GetWpnData().iMuzzleSmokeNumShots)
		{
			DispatchParticleEffect(STRING(GetWpnData().iMuzzleSmoke), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), GetWpnData().iAttachment, false);
		}
	}
}

void CBaseCombatWeapon::PrimaryAttackPostUpdate(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (GetWpnData().allowAmmoRegen)
		return;

	if (thisType == TYPE_BEAM)
	{
		if (m_iClip1 <= 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}
	}
	else
	{
		if (m_iClip1 <= 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && thisType != TYPE_AIRBOATGUN)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}
	}
}

void CBaseCombatWeapon::SecondaryAttackEffects(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	/*if (cvar->FindVar("oc_particle_muzzleflashes")->GetInt())
	{
	int index = random->RandomInt(0, GetWpnData().externalIndexS);

	string_t sMFName;

	if (pPlayer->GetWaterLevel() != 3)
	sMFName = GetWpnData().iMuzzleFlashTypeS[index];
	else
	sMFName = GetWpnData().iMuzzleFlashUnderWaterType;

	DispatchParticleEffect(STRING(sMFName), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), STRING(GetWpnData().iSecondaryAttachment), false);
	}
	else
	{
	FireEventOnClient();
	}*/

	FireClientSecondaryAttack();

	DoMuzzleFlash();
	//AddViewKick();
}

void CBaseCombatWeapon::DelayedAttackEffects(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	FireClientDelayedAttack();

	DoMuzzleFlash();
}

void CBaseCombatWeapon::PrimaryAttackShotgun(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (IsNearWall() /*(cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1)*/ || GetOwnerIsRunning())
	{
		return;
	}

	Activity primaryAttack = GetPrimaryAttackActivity();

	SendWeaponAnim(primaryAttack);

	attackActivites[0] = primaryAttack;

	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (GetOwner() && GetOwner()->IsNPC())
		m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();	// overcharged: 3rd person anims bug fix
	else if (GetOwner() && GetOwner()->IsPlayer())
	{
		if (m_bFireMode == 2 || m_bFireMode == 3)
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		else
			m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	}

	m_flNextShotgunReload = m_flNextPrimaryAttack;

	/*if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt())
	{
	if (GetDefaultClip1() > 0)
	m_iClip1 -= 1;
	}
	else
	m_iClip1 -= 1;*/
	RemoveAmmo(GetPrimaryAmmoType(), 1);

	PumpAfterTwoBarrels = false;

	//pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);

	CBasePlayer *pOwner = pPlayer;

	if (pOwner == NULL)
		return;

	FireBulletsInfo_t info;

	info.m_bHoldedAttack = false;

	info.m_iShots = NumShots();//sk_plr_num_shotgun_pellets.GetInt();

	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	info.m_vecSrc = pPlayer->EyePosition();

	info.m_flDistance = MAX_TRACE_LENGTH;

	info.m_iAmmoType = m_iPrimaryAmmoType;

	info.m_iTracerFreq = 1;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	if (thisType != TYPE_BEAM)
		pPlayer->FireBulletsShotgun(info);

	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner() );

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && thisType != TYPE_AIRBOATGUN)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_bNeedPump = true;

	if (!m_bSilenced)
		pPlayer->DoMuzzleFlash();

	AddViewKick();

	if (GetWpnData().iMuzzleSmokeNumShots > 0)
	{
		if (NumShots() >= GetWpnData().iMuzzleSmokeNumShots)
		{
			DispatchParticleEffect(STRING(GetWpnData().iMuzzleSmoke), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), GetWpnData().iAttachment, false);
		}
	}
}
void CBaseCombatWeapon::SecondaryAttackShotgun(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (IsNearWall()/*(cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1)*/ || GetOwnerIsRunning())
	{
		return;
	}

	pPlayer->m_nButtons &= ~IN_ATTACK2;

	if (m_iClip1 <= 1)
	{
		PrimaryAttackShotgun();
		return;
	}

	PumpAfterTwoBarrels = true;
	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(WPN_DOUBLE);

	pPlayer->DoMuzzleFlash();


	Activity primaryAttack = (Activity)0;

	if (pPlayer && pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].ShotgunFireAnimation == 0)
	{
		primaryAttack = GetSecondaryAttackActivity();
		SendWeaponAnim(primaryAttack);
	}
	if (pPlayer && pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].ShotgunFireAnimation == 1)
	{
		primaryAttack = GetPrimaryAttackActivity();
		SendWeaponAnim(primaryAttack);
	}
	if (pPlayer && pPlayer->GetActiveWeapon()->GetWpnData().animData[m_bFireMode].ShotgunFireAnimation == 2)
	{
		int i = RandomInt(1, 2);
		if (i == 1)
		{
			primaryAttack = GetSecondaryAttackActivity();
			SendWeaponAnim(primaryAttack);
		}
		if (i == 2)
		{
			primaryAttack = GetPrimaryAttackActivity();
			SendWeaponAnim(primaryAttack);
		}
	}

	attackActivites[0] = primaryAttack;

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (GetOwner() && GetOwner()->IsNPC())
		m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();	// overcharged: 3rd person anims bug fix
	else if (GetOwner() && GetOwner()->IsPlayer())
	{
		if (m_bFireMode == 2 || m_bFireMode == 3)
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		else
			m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	}

	m_flNextShotgunReload = m_flNextPrimaryAttack;

	/*if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt() == 1)
	{
	if (GetDefaultClip1() > 0)
	m_iClip1 -= 2;
	}
	else
	m_iClip1 -= 2;*/
	RemoveAmmo(GetPrimaryAmmoType(), 2);	// Shotgun uses same clip for primary and secondary attacks

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	FireBulletsInfo_t info;

	info.m_iShots = NumShots() * 2;//sk_plr_num_shotgun_pellets.GetInt();

	info.m_vecSrc = pPlayer->Weapon_ShootPosition();

	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	info.m_vecSrc = pPlayer->EyePosition();

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	if (thisType != TYPE_BEAM)
		pPlayer->FireBulletsShotgun(info);

	AddViewKick();

	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2 );

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && pPlayer->GetActiveWeapon()->thisType != TYPE_AIRBOATGUN)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_bNeedPump = true;

	if (!m_bSilenced)
		pPlayer->DoMuzzleFlash();

	if (GetWpnData().iMuzzleSmokeNumShots > 0)
	{
		if (NumShots() >= GetWpnData().iMuzzleSmokeNumShots)
		{
			DispatchParticleEffect(STRING(GetWpnData().iMuzzleSmoke), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), GetWpnData().iAttachment, false);
		}
	}
}


bool CBaseCombatWeapon::CanHolster()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return false;
#ifndef CLIENT_DLL
	if (pPlayer->grenadeState > 0)
		return false;
#endif
	if (m_flNextSilencer > gpGlobals->curtime ||
		m_flNextSwitchTime > gpGlobals->curtime)
		return false;

	return true;
}

bool CBaseCombatWeapon::CanDrop()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return false;

	if (GetWpnData().m_bReservedSlot)
		return false;

	if ((pPlayer->m_nButtons & IN_ATTACK) ||
		(pPlayer->m_nButtons & IN_ATTACK2) ||
		m_bInReload ||
		m_bReloadComplete ||
#ifndef CLIENT_DLL
		pPlayer->grenadeState > 0 ||
#endif
		IronSightState > 0 || IsScopeSighted() ||
		IsNearWall() ||
		//cvar->FindVar("oc_state_near_wall_standing")->GetInt() || 
		GetOwnerIsRunning() ||
		m_flNextPrimaryAttack > gpGlobals->curtime ||
		m_flNextInspectAnimation > gpGlobals->curtime ||
		m_flNextSecondaryAttack > gpGlobals->curtime ||
		m_flNextShotgunReload > gpGlobals->curtime ||
		m_flNextSilencer > gpGlobals->curtime ||
		m_flNextSwitchTime > gpGlobals->curtime ||
		cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt() ? thisType == TYPE_GRENADE : false)
		return false;

	return CanHolster();
}

bool CBaseCombatWeapon::DeployGrenade()
{
	if (!HasPrimaryAmmo())
		return false;

	if (!HasAnyAmmo() && AllowsAutoSwitchFrom())
		return false;

	float flSequenceDuration = 0.0f;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		// Dead men deploy no weapons
		if (pOwner->IsAlive() == false)
			return false;

		pOwner->SetAnimationExtension((char*)GetAnimPrefix());

		g_ShowWeapon.SetShowWeapon(this, GetDrawActivity(), flSequenceDuration);

		SetupWeapon(pOwner);
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

#ifndef CLIENT_DLL
	// Cancel any pending hide events
	g_EventQueue.CancelEventOn(this, "HideWeapon");
#endif
	return true;
}

void CBaseCombatWeapon::PullUpGrenade(bool down)
{
	if (down)
	{

		if (!HasPrimaryAmmo())
			return;


		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (pPlayer == NULL)
			return;

		if (IsNearWall() /*(cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1)*/ || GetOwnerIsRunning())
		{
			return;
		}

		SendWeaponAnim(GetWpnData().animData[0].ThrowPullDown);

	}
	else
	{

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (!pPlayer)
			return;

		if (IsNearWall() /*(cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1)*/ || GetOwnerIsRunning())
		{
			return;
		}

		SendWeaponAnim(GetWpnData().animData[0].ThrowPullUp);
		m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration() * 2;

	}
}
void CBaseCombatWeapon::ReleaseGrenade(bool secondary)
{
	if (secondary)
		SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);
	else
		SendWeaponAnim(GetWpnData().animData[0].ThrowPrimary);

	AddViewKick();
}

//-----------------------------------------------------------------------------
// Create/destroy looping sounds 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ReinitializeLoopSound(const char *sample)
{
#ifndef CLIENT_DLL
	m_bIsFiring = false;
	//if (!m_pLoopSoundController || !m_pLoopSound)
	/*{
	if (m_pLoopSoundController && m_pLoopSound)
	{
	m_pLoopSoundController->Shutdown(m_pLoopSound);
	m_pLoopSoundController->CommandClear(m_pLoopSound);
	m_pLoopSoundController->SoundDestroy(m_pLoopSound);
	}
	else if (m_pLoopSoundController && !m_pLoopSound)
	{
	m_pLoopSoundController->SystemReset();
	m_pLoopSoundController->SystemUpdate();
	}

	m_pLoopSound = NULL;
	m_pLoopSoundController = NULL;

	soundInitialVolume = 0.0f;
	soundInitialPitch = 0.0f;
	}*/
	DestroyLoopSound();
	if (!m_pLoopSoundController || !m_pLoopSound)
	{
		CreateLoopSound(sample);
	}
#endif
}
void CBaseCombatWeapon::CreateLoopSound(const char *sample)
{
#ifndef CLIENT_DLL
	if (!m_pLoopSoundController)
		m_pLoopSoundController = &CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);

	if (!m_pLoopSound)
	{
		int index = m_bSilenced ? GetOwner()->entindex() : this->entindex();
		int channel = 0;//m_bSilenced ? 1 : 0;
		m_pLoopSound = m_pLoopSoundController->SoundCreateOVR(filter, index, channel, sample);

		soundInitialVolume = m_pLoopSoundController->SoundGetVolume(m_pLoopSound);

		soundInitialPitch = m_pLoopSoundController->SoundGetPitch(m_pLoopSound);
		if (m_bFireMode == 3)
			soundInitialPitch = soundInitialPitch * GetWpnData().LoopSoundBurstPitchMultiplier;//(GetWpnData().mode_fire_rate_automatic / GetWpnData().mode_fire_rate_burst);
		//(1 / GetFireRate());// (GetWpnData().mode_fire_rate_automatic / this->GetWpnData().mode_fire_rate_burst);//(1 / GetFireRate());

	}
#endif
}

void CBaseCombatWeapon::DestroyLoopSound()
{
#ifndef CLIENT_DLL

	if (m_bIsFiring)
		m_bIsFiring = false;

	if (m_pLoopSoundController)
	{
		m_pLoopSoundController->Shutdown(m_pLoopSound);
		//m_pLoopSoundController->CommandClear(m_pLoopSound);
		m_pLoopSoundController->SoundDestroy(m_pLoopSound);

		//m_pLoopSoundController->SystemReset();
		//m_pLoopSoundController->SystemUpdate();
	}

	soundInitialVolume = 0.0f;
	soundInitialPitch = 0.0f;

	m_pLoopSound = NULL;
	m_pLoopSoundController = NULL;

	/*if (m_pLoopSoundController && m_pLoopSound)
	{
	m_pLoopSoundController->SoundDestroy(m_pLoopSound);
	m_pLoopSound = NULL;
	}*/
#endif
}

void CBaseCombatWeapon::StartLoopSound()
{
	if (IsNearWall() || GetOwnerIsRunning())
		return;

#ifndef CLIENT_DLL

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	/*if (pPlayer->m_nButtons & IN_ATTACK2)
	{
	if (m_flNextSecondaryAttack > gpGlobals->curtime)
	{
	StopLoopSound();
	return;
	}
	}*/

	bool usingClips = m_iClip1 <= 0;

	if (pPlayer && thisType == TYPE_BEAM)
		usingClips = pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0;

	if (usingClips && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		DestroyLoopSound();
		return;
	}


	if (!m_bIsFiring)
	{
		DestroyLoopSound();

		if (!m_pLoopSoundController || !m_pLoopSound)
		{
			/*PrecacheScriptSound(GetWpnData().ShootLoopingSound);
			PrecacheScriptSound(GetWpnData().EndLoopingSound);

			if (GetWpnData().allowLoopSilencerSound)
			{
			PrecacheScriptSound(GetWpnData().ShootLoopingSilencerSound);
			PrecacheScriptSound(GetWpnData().EndLoopingSilencerSound);
			}*/

			/*if (m_bSilenced)
			ReinitializeLoopSound(GetWpnData().ShootLoopingSilencerSound);
			else
			ReinitializeLoopSound(GetWpnData().ShootLoopingSound);*/

			//if (!m_pLoopSoundController || !m_pLoopSound)
			//{

			PrecacheScriptSound(GetWpnData().ShootLoopingSound);
			PrecacheScriptSound(GetWpnData().EndLoopingSound);

			if (GetWpnData().allowLoopSilencerSound)
			{
				PrecacheScriptSound(GetWpnData().ShootLoopingSilencerSound);
				PrecacheScriptSound(GetWpnData().EndLoopingSilencerSound);
			}

			CreateLoopSound(m_bSilenced ? GetWpnData().ShootLoopingSilencerSound : GetWpnData().ShootLoopingSound);
			//}
		}

		//m_pLoopSoundController->Play(m_pLoopSound, m_pLoopSoundController->SoundGetVolume(m_pLoopSound), m_pLoopSoundController->SoundGetPitch(m_pLoopSound));
		m_pLoopSoundController->Play(m_pLoopSound, soundInitialVolume, soundInitialPitch);

		m_bIsFiring = true;
	}

	if (m_pLoopSoundController)
	{
		if (pPlayer->GetSlowMoIsEnabled())
		{
			if (m_bSilenced && GetWpnData().allowLoopSilencerSound)
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, (soundInitialPitch / GetWpnData().LoopSilencerSoundSlowMoPitchDivider) * HostPitchLoopingFire.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
			else
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, (soundInitialPitch / GetWpnData().LoopSoundSlowMoPitchDivider) * HostPitchLoopingFire.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
		}
		else
		{
			if (m_bSilenced && GetWpnData().allowLoopSilencerSound)
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, soundInitialPitch * HostPitchLoopingFire.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
			else
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, soundInitialPitch * HostPitchLoopingFire.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
		}
	}
#endif
}

void CBaseCombatWeapon::StopLoopSound()
{
#ifndef CLIENT_DLL
	if (m_bIsFiring)
	{
		if (m_bSilenced && GetWpnData().allowLoopSilencerSound)
		{
			//m_pLoopSoundController->Shutdown(m_pLoopSound);
			StopSound(GetWpnData().EndLoopingSilencerSound);
			EmitSound(GetWpnData().EndLoopingSilencerSound);
			DestroyLoopSound();
		}
		else
		{
			//m_pLoopSoundController->Shutdown(m_pLoopSound);
			StopSound(GetWpnData().EndLoopingSound);
			EmitSound(GetWpnData().EndLoopingSound);
			DestroyLoopSound();
		}

		m_bIsFiring = false;
	}
#endif
}


#if !defined( CLIENT_DLL )

void CBaseCombatWeapon::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
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

	WeaponSound(SINGLE_NPC);

	if (hl2_episodic.GetBool())
	{
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
	else
	{
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	}

	pOperator->DoMuzzleFlash();

	if (hl2_episodic.GetBool())
	{
		// Never fire Alyx's last bullet just in case there's an emergency
		// and she needs to be able to shoot without reloading.
		if (m_iClip1 > 1)
		{
			m_iClip1 = m_iClip1 - 1;
		}
	}
	else
	{
		m_iClip1 = m_iClip1 - 1;
	}
}

ConVar	sk_npc_num_shotgun_pellets("sk_npc_num_shotgun_pellets", "9");
void CBaseCombatWeapon::FireNPCPrimaryAttackShotgun(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
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

	WeaponSound(SINGLE_NPC);

	if (hl2_episodic.GetBool())
	{
		pOperator->FireBullets(sk_npc_num_shotgun_pellets.GetInt(), vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
	else
	{
		pOperator->FireBullets(sk_npc_num_shotgun_pellets.GetInt(), vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	}

	pOperator->DoMuzzleFlash();

	if (hl2_episodic.GetBool())
	{
		// Never fire Alyx's last bullet just in case there's an emergency
		// and she needs to be able to shoot without reloading.
		if (m_iClip1 > 1)
		{
			m_iClip1 = m_iClip1 - 1;
		}
	}
	else
	{
		m_iClip1 = m_iClip1 - 1;
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Operator_NPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	m_iClip1++;

	// HACK: We need the gun to fire its muzzle flash
	if (bSecondary == false)
	{
		SetActivity(ACT_RANGE_ATTACK_PISTOL, 0.0f);
	}

	FireNPCPrimaryAttack(pOperator, true);
}

void CBaseCombatWeapon::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	if ((pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER))
	{
		if (pEvent->event == AE_NPC_WEAPON_FIRE)
		{
			bool bSecondary = (atoi(pEvent->options) != 0);
			Operator_ForceNPCFire(pOperator, bSecondary);
			return;
		}
		else if (pEvent->event == AE_WPN_PLAYWPNSOUND)
		{
			int iSnd = GetWeaponSoundFromString(pEvent->options);
			if (iSnd != -1)
			{
				WeaponSound((WeaponSound_t)iSnd);
			}
		}
	}

	switch (pEvent->event)
	{
		case EVENT_WEAPON_RELOAD:
		{
			ShellOut(pEvent);
		}
		break;
		case EVENT_WEAPON_MAG_OUT:
		{
			if (cvar->FindVar("oc_weapons_enable_drop_mags")->GetBool())
				MagOut();
		}
		break;
		case EVENT_WEAPON_AR2:
		{
			FireNPCPrimaryAttack(pOperator, false);
		}
		break;
		case EVENT_WEAPON_SMG1:
		{
			FireNPCPrimaryAttack(pOperator, false);
		}
		break;
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			FireNPCPrimaryAttackShotgun(pOperator, false);
		}
		break;
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT(npc != NULL);

			vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

			CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

			WeaponSound(SINGLE_NPC);
			pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			break;
	}
}

void CBaseCombatWeapon::SetClientVectors(Vector TrVec, Vector muzVec)
{
	m_vTracerMuzzleVector = TrVec;
	m_vMuzzleVector = muzVec;
}

void CBaseCombatWeapon::SetClientAngles(QAngle muzAng)
{
	m_vMuzzleAngle = muzAng;
}

const Vector &CBaseCombatWeapon::GetClientTracerVector()
{
	m_vTracerMuzzleVector = Vector(cvar->FindVar("oc_muzzle_tracer_x")->GetFloat(), cvar->FindVar("oc_muzzle_tracer_y")->GetFloat(), cvar->FindVar("oc_muzzle_tracer_z")->GetFloat());

	return m_vTracerMuzzleVector;
}

const Vector &CBaseCombatWeapon::GetClientMuzzleVector()
{
	m_vMuzzleVector = Vector(cvar->FindVar("oc_muzzle_vector_x")->GetFloat(), cvar->FindVar("oc_muzzle_vector_y")->GetFloat(), cvar->FindVar("oc_muzzle_vector_z")->GetFloat());

	return m_vMuzzleVector;
}

const QAngle &CBaseCombatWeapon::GetClientMuzzleAngles()
{
	m_vMuzzleAngle = QAngle(cvar->FindVar("oc_muzzle_angle_x")->GetFloat(), cvar->FindVar("oc_muzzle_angle_y")->GetFloat(), cvar->FindVar("oc_muzzle_angle_z")->GetFloat());

	return m_vMuzzleAngle;
}

void CBaseCombatWeapon::ShellOut(animevent_t *pEvent)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	int opt = atoi(pEvent->options);

	int shells = opt ? opt : GetMaxClip1() - Clip1();

	CBaseViewModel *vm = pPlayer->GetViewModel();
	if (vm)
	{
		CEffectData data;
		data.m_nAttachmentIndex = vm->LookupAttachment(GetWpnData().iWeaponShellAttachment);
		data.m_nEntIndex = vm->entindex();

		// Emit empty(shells) spended shells
		for (int i = 0; i < shells; i++)
		{
			data.m_vAngles = QAngle(90, random->RandomInt(0, 360), 0);
			DispatchEffect("CommonShellEject", data);
		}
	}
}

#include "physobj.h"
void CBaseCombatWeapon::MagOut(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer && GetWpnData().m_bHasMag)
	{
		if (!UTIL_GetIsPrecachedModel(this, GetWpnData().iMagName))
			return;

		Vector attachOrigin;
		QAngle attachAngles;

		//if (this->GetAttachment("muzzle", attachOrigin, attachAngles))
		if (this->GetAttachment(GetWpnData().Mag_Attachment, attachOrigin, attachAngles))
		{
			Vector Origin;
			QAngle Ang;

			Origin = GetClientMuzzleVector();
			/*Origin.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
			Origin.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
			Origin.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();*/

			Ang = GetClientMuzzleAngles();

			if (cvar->FindVar("oc_mag_test_pos_enabled")->GetInt() == 1)
			{
				Vector forward, right, up;

				pPlayer->EyeVectors(&forward, &right, &up);
				AngleVectors(Ang, &forward, &right, &up);

				if (DoOnceMPS)
				{
					cvar->FindVar("oc_mag_test_pos_x")->SetValue(GetWpnData().Mag_Offset.x);
					cvar->FindVar("oc_mag_test_pos_y")->SetValue(GetWpnData().Mag_Offset.y);
					cvar->FindVar("oc_mag_test_pos_z")->SetValue(GetWpnData().Mag_Offset.z);

					cvar->FindVar("oc_mag_test_ang_x")->SetValue(GetWpnData().Mag_Angle.x);
					cvar->FindVar("oc_mag_test_ang_y")->SetValue(GetWpnData().Mag_Angle.y);
					cvar->FindVar("oc_mag_test_ang_z")->SetValue(GetWpnData().Mag_Angle.z);
					DoOnceMPS = false;
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
				DoOnceMPS = true;
				Vector forward, right, up;
				pPlayer->EyeVectors(&forward, &right, &up);
				AngleVectors(Ang, &forward, &right, &up);

				Origin += forward * GetWpnData().Mag_Offset.x;
				Origin += right * GetWpnData().Mag_Offset.y;
				Origin += up * GetWpnData().Mag_Offset.z;

				Ang.x += GetWpnData().Mag_Angle.x;
				Ang.y += GetWpnData().Mag_Angle.y;
				Ang.z += GetWpnData().Mag_Angle.z;

			}

			CBaseEntity *pVial = NULL;
			pVial = CBaseEntity::Create("item_empty_mag", Origin, Ang, this);

			//pVial->SoundIndex = 75;

			//magHitSound
#ifndef CLIENT_DLL
			CE_MAG *pMag = (CE_MAG*)pVial;
			if (pMag)
			{
				//const char* hitSound = STRING(GetWpnData().magHitSound);//STRING(GetWpnData().magHitSound);
				pMag->hitSound = MAKE_STRING(GetWpnData().magHitSound);
				//pMag->PrecacheScriptSound(hitSound);
			}
#endif
			//pVial->PrecacheModel(GetWpnData().iMagName);
			pVial->SetModel(GetWpnData().iMagName);
			pVial->SetSolid(SOLID_VPHYSICS);
			pVial->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
			pVial->SetMoveType(MOVETYPE_VPHYSICS);
			pVial->AddSpawnFlags(SF_PHYSBOX_ASLEEP);
			pVial->AddEffects(EF_NOSHADOW);
			pVial->SetLocalOrigin(Origin);
			pVial->SetLocalAngles(Ang);
			pVial->Spawn();


			if (cvar->FindVar("oc_mag_test_vel_enabled")->GetInt() == 1)
			{
				ConColorMsg(Color(0, 255, 255, 255), "Mag_Offset.x: %.2f \n", GetWpnData().Mag_Offset.x);
				ConColorMsg(Color(0, 255, 255, 255), "Mag_Offset.y: %.2f \n", GetWpnData().Mag_Offset.y);
				ConColorMsg(Color(0, 255, 255, 255), "Mag_Offset.z: %.2f \n", GetWpnData().Mag_Offset.z);
				Vector forward, right, up;
				pPlayer->EyeVectors(&forward, &right, &up);

				Vector vel;
				Vector Start = pPlayer->EyeDirection3D();
				Vector End;
				End += (forward * cvar->FindVar("oc_mag_test_vel_x")->GetFloat());
				End += (right * cvar->FindVar("oc_mag_test_vel_y")->GetFloat());
				End += (up * cvar->FindVar("oc_mag_test_vel_z")->GetFloat());
				vel = Start + End;

				IPhysicsObject *pPhysics = pVial->VPhysicsGetObject();
				if (pPhysics != NULL)
				{
					AngularImpulse	angImp(cvar->FindVar("oc_mag_test_angvel_x")->GetFloat(), cvar->FindVar("oc_mag_test_angvel_y")->GetFloat(), cvar->FindVar("oc_mag_test_angvel_y")->GetFloat());
					pPhysics->AddVelocity(&vel, &angImp);
				}
				//pVial->ApplyLocalVelocityImpulse(vel);
				//debugoverlay->AddLineOverlay(Start, End, 255, 255, 255, false, 10.1f);
			}
			else if (GetWpnData().Mag_Velocity != Vector(0, 0, 0) && cvar->FindVar("oc_mag_test_vel_enabled")->GetInt() == 0)
			{
				Vector forward, right, up;
				pPlayer->EyeVectors(&forward, &right, &up);
				Vector vel;
				Vector Start = pPlayer->EyeDirection3D();
				Vector End;
				End += (forward * GetWpnData().Mag_Velocity.x);
				End += (right * GetWpnData().Mag_Velocity.y);
				End += (up * GetWpnData().Mag_Velocity.z);
				vel = Start + End;

				IPhysicsObject *pPhysics = pVial->VPhysicsGetObject();
				if (pPhysics != NULL)
				{
					if (GetWpnData().Mag_AngularVelocity != Vector(0, 0, 0))
					{
						AngularImpulse	angImp(GetWpnData().Mag_AngularVelocity.x, GetWpnData().Mag_AngularVelocity.y, GetWpnData().Mag_AngularVelocity.z);
						pPhysics->AddVelocity(&vel, &angImp);
					}
					else
						pPhysics->AddVelocity(&vel, NULL);
				}
			}
			pVial->SUB_StartFadeOut(15, false);
		}
	}
}
#endif

//================== BriJee: Overcharged IMPORTANT PART, All new weapon functions define UNDER
bool CBaseCombatWeapon::IsBehindTarget(CBaseEntity *pTarget)
{
	Assert(pTarget);

	// Get the forward view vector of the target, ignore Z
	Vector vecVictimForward;
	AngleVectors(pTarget->EyeAngles(), &vecVictimForward, NULL, NULL);
	vecVictimForward.z = 0.0f;
	vecVictimForward.NormalizeInPlace();

	// Get a vector from my origin to my targets origin
	Vector vecToTarget;
	vecToTarget = pTarget->WorldSpaceCenter() - GetOwner()->WorldSpaceCenter();
	vecToTarget.z = 0.0f;
	vecToTarget.NormalizeInPlace();

	float flDot = DotProduct(vecVictimForward, vecToTarget);

	return (flDot > -0.1);
}
//-----------------------------------------------------------------------------
// Purpose: Called every frame to check if the weapon is going through transition animations
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MaintainIdealActivity(void)
{
	// Must be transitioning
	if (GetActivity() != ACT_TRANSITION)
		return;

	// Must not be at our ideal already 
	if ((GetActivity() == m_IdealActivity) && (GetSequence() == m_nIdealSequence))
		return;

	// Must be finished with the current animation
	if (IsViewModelSequenceFinished() == false)
		return;

	// Move to the next animation towards our ideal
	SendWeaponAnim(m_IdealActivity);
}

//-----------------------------------------------------------------------------
// Purpose: Sets the ideal activity for the weapon to be in, allowing for transitional animations inbetween
// Input  : ideal - activity to end up at, ideally
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SetIdealActivity(Activity ideal)
{
	MDLCACHE_CRITICAL_SECTION();
	int idealSequence = SelectWeightedSequence(ideal);

	if (idealSequence == -1 && GetOwner() && cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
	{
#ifndef CLIENT_DLL
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		idealSequence = SelectWeightedSequence(pPlayer->Weapon_TranslateActivity(ideal));
#endif
	}

	if (idealSequence == -1)
		return false;

	//Take the new activity
	m_IdealActivity = ideal;
	m_nIdealSequence = idealSequence;

	//Find the next sequence in the potential chain of sequences leading to our ideal one
	int nextSequence = FindTransitionSequence(GetSequence(), m_nIdealSequence, NULL);

	// Don't use transitions when we're deploying
	if (ideal != ACT_VM_DRAW && IsWeaponVisible() && nextSequence != m_nIdealSequence)
	{
		//Set our activity to the next transitional animation
		SetActivity(ACT_TRANSITION);
		SetSequence(nextSequence);
		SendViewModelAnim(nextSequence);
	}
	else
	{
		//Set our activity to the ideal
		SetActivity(m_IdealActivity);
		SetSequence(m_nIdealSequence);
		SendViewModelAnim(m_nIdealSequence);
	}

	//Set the next time the weapon will idle
	SetWeaponIdleTime(gpGlobals->curtime + GetViewModelSequenceDuration());
	return true;

}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelInfo(int nPanelIndex, const char *&pPanelName)
{
	pPanelName = NULL;
}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelClassName(int nPanelIndex, const char *&pPanelName)
{
	pPanelName = "vgui_screen";
}


//-----------------------------------------------------------------------------
// Locking a weapon is an exclusive action. If you lock a weapon, that means 
// you are preventing others from doing so for themselves.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Lock(float lockTime, CBaseEntity *pLocker)
{
	m_flUnlockTime = gpGlobals->curtime + lockTime;
	m_hLocker.Set(pLocker);
}

//-----------------------------------------------------------------------------
// If I'm still locked for a period of time, tell everyone except the person
// that locked me that I'm not available. 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsLocked(CBaseEntity *pAsker)
{
	return (m_flUnlockTime > gpGlobals->curtime && m_hLocker != pAsker);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Activity CBaseCombatWeapon::ActivityOverride(Activity baseAct, bool *pRequired)
{
	acttable_t *pTable = ActivityList();
	int actCount = ActivityListCount();

	for (int i = 0; i < actCount; i++, pTable++)
	{
		if (baseAct == pTable->baseAct)
		{
			if (pRequired)
			{
				*pRequired = pTable->required;
			}
			return (Activity)pTable->weaponAct;
		}
	}
	return baseAct;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CDmgAccumulator::CDmgAccumulator(void)
{
#ifdef GAME_DLL
	SetDefLessFunc(m_TargetsDmgInfo);
#endif // GAME_DLL

	m_bActive = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CDmgAccumulator::~CDmgAccumulator()
{
	// Did a weapon get deleted while aggregating CTakeDamageInfo events?
	Assert(!m_bActive);
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Collect trace attacks for weapons that fire multiple bullets per attack that also penetrate
//-----------------------------------------------------------------------------
void CDmgAccumulator::AccumulateMultiDamage(const CTakeDamageInfo &info, CBaseEntity *pEntity)
{
	if (!pEntity)
		return;

	Assert(m_bActive);

#if defined( GAME_DLL )
	int iIndex = m_TargetsDmgInfo.Find(pEntity->entindex());
	if (iIndex == m_TargetsDmgInfo.InvalidIndex())
	{
		m_TargetsDmgInfo.Insert(pEntity->entindex(), info);
	}
	else
	{
		CTakeDamageInfo *pInfo = &m_TargetsDmgInfo[iIndex];
		if (pInfo)
		{
			// Update
			m_TargetsDmgInfo[iIndex].AddDamageType(info.GetDamageType());
			m_TargetsDmgInfo[iIndex].SetDamage(pInfo->GetDamage() + info.GetDamage());
			m_TargetsDmgInfo[iIndex].SetDamageForce(pInfo->GetDamageForce() + info.GetDamageForce());
			m_TargetsDmgInfo[iIndex].SetDamagePosition(info.GetDamagePosition());
			m_TargetsDmgInfo[iIndex].SetReportedPosition(info.GetReportedPosition());
			m_TargetsDmgInfo[iIndex].SetMaxDamage(MAX(pInfo->GetMaxDamage(), info.GetDamage()));
			m_TargetsDmgInfo[iIndex].SetAmmoType(info.GetAmmoType());
		}

	}
#endif	// GAME_DLL
}

//-----------------------------------------------------------------------------
// Purpose: Send aggregate info
//-----------------------------------------------------------------------------
void CDmgAccumulator::Process(void)
{
	FOR_EACH_MAP(m_TargetsDmgInfo, i)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex(m_TargetsDmgInfo.Key(i));
		if (pEntity)
		{
			AddMultiDamage(m_TargetsDmgInfo[i], pEntity);
		}
	}

	m_bActive = false;
	m_TargetsDmgInfo.Purge();
}
#endif // GAME_DLL

#if defined( CLIENT_DLL )

BEGIN_PREDICTION_DATA(CBaseCombatWeapon)

DEFINE_PRED_FIELD(m_nNextThinkTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
// Networked
DEFINE_PRED_FIELD(m_hOwner, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE),
// DEFINE_FIELD( m_hWeaponFileInfo, FIELD_SHORT ),
DEFINE_PRED_FIELD(m_iState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
DEFINE_PRED_FIELD(m_iWorldModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
DEFINE_PRED_FIELD_TOL(m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
DEFINE_PRED_FIELD_TOL(m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
DEFINE_PRED_FIELD_TOL(m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),

// BriJee OVR: new functions
DEFINE_PRED_FIELD(m_bSilenced, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iPrimaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iSecondaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iClip1, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iClip2, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_nViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),

// Not networked

DEFINE_PRED_FIELD(m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_UTLVECTOR(attackActivites, FIELD_INTEGER),
DEFINE_FIELD(IronSightState, FIELD_INTEGER),

DEFINE_FIELD(m_bShouldDrawWeaponBloodOverlay, FIELD_BOOLEAN),
DEFINE_FIELD(m_iWeaponBloodOverlayDetailFrame, FIELD_INTEGER),
DEFINE_FIELD(ScopeSightState, FIELD_BOOLEAN),
DEFINE_FIELD(wasInIronSighted, FIELD_BOOLEAN),
DEFINE_FIELD(m_bInReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFireOnEmpty, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFiringWholeClip, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextEmptySoundTime, FIELD_FLOAT),
DEFINE_FIELD(m_flNextInspectAnimation, FIELD_TIME),
DEFINE_FIELD(m_Activity, FIELD_INTEGER),
DEFINE_FIELD(m_fFireDuration, FIELD_FLOAT),
DEFINE_FIELD(m_iszName, FIELD_INTEGER),
DEFINE_FIELD(m_bFiresUnderwater, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAltFiresUnderwater, FIELD_BOOLEAN),
DEFINE_FIELD(m_fMinRange1, FIELD_FLOAT),
DEFINE_FIELD(m_fMinRange2, FIELD_FLOAT),
DEFINE_FIELD(m_fMaxRange1, FIELD_FLOAT),
DEFINE_FIELD(m_fMaxRange2, FIELD_FLOAT),
DEFINE_FIELD(m_bReloadsSingly, FIELD_BOOLEAN),
DEFINE_FIELD(wasburstFire, FIELD_BOOLEAN),
DEFINE_FIELD(m_bRemoveable, FIELD_BOOLEAN),
DEFINE_FIELD(m_iPrimaryAmmoCount, FIELD_INTEGER),
DEFINE_FIELD(m_bFireMode, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondaryAmmoCount, FIELD_INTEGER),
DEFINE_FIELD(m_iMuzzleFlashState, FIELD_INTEGER),
// BriJee OVR: new functions
DEFINE_FIELD(m_bSilenced, FIELD_BOOLEAN),
DEFINE_FIELD(bg, FIELD_INTEGER),
DEFINE_FIELD(bgV, FIELD_INTEGER),
DEFINE_FIELD(m_bIsFiring, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsInPrimaryAttack, FIELD_BOOLEAN),
DEFINE_FIELD(letViewModelBob, FIELD_BOOLEAN),
DEFINE_FIELD(switched, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
DEFINE_FIELD(m_flHolsterTime, FIELD_TIME),
DEFINE_FIELD(m_iShotgunReloadState, FIELD_INTEGER),
DEFINE_FIELD(burstFireCount, FIELD_INTEGER),
DEFINE_FIELD(PumpAfterTwoBarrels, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFirstEquip, FIELD_BOOLEAN),
DEFINE_FIELD(reloadFromEmpty, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextShotgunReload, FIELD_TIME),
DEFINE_FIELD(m_flNextAmmoRegen, FIELD_TIME),
DEFINE_FIELD(m_flNextAmmoRemove, FIELD_TIME),
DEFINE_FIELD(m_flNextSilencer, FIELD_TIME),
DEFINE_FIELD(m_bWeaponBlockWall, FIELD_BOOLEAN),
DEFINE_FIELD(m_bEnableMainLaser, FIELD_BOOLEAN),

DEFINE_FIELD(clipNearWall, FIELD_BOOLEAN),
DEFINE_FIELD(distanceToWall, FIELD_FLOAT),

//DEFINE_PHYSPTR( m_pConstraint ),

// DEFINE_FIELD( m_iOldState, FIELD_INTEGER ),
// DEFINE_FIELD( m_bJustRestored, FIELD_BOOLEAN ),

// DEFINE_FIELD( m_OnPlayerPickup, COutputEvent ),
// DEFINE_FIELD( m_pConstraint, FIELD_INTEGER ),

END_PREDICTION_DATA()

#endif	// ! CLIENT_DLL

// Special hack since we're aliasing the name C_BaseCombatWeapon with a macro on the client
IMPLEMENT_NETWORKCLASS_ALIASED(BaseCombatWeapon, DT_BaseCombatWeapon)

#if !defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: Save Data for Base Weapon object
//-----------------------------------------------------------------------------// 
BEGIN_DATADESC(CBaseCombatWeapon)

DEFINE_AUTO_ARRAY(nameOfFireMode, FIELD_CHARACTER),
DEFINE_FIELD(m_bIsFiring, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsInPrimaryAttack, FIELD_BOOLEAN),
DEFINE_FIELD(letViewModelBob, FIELD_BOOLEAN),
//DEFINE_SOUNDPATCH(m_pLoopSound),
//DEFINE_FIELD(m_pLoopSoundController, FIELD_CLASSPTR),
//DEFINE_FIELD(m_pLoopSound, FIELD_CLASSPTR),
DEFINE_FIELD(m_bShouldDrawWeaponBloodOverlay, FIELD_BOOLEAN),
DEFINE_FIELD(m_iWeaponBloodOverlayDetailFrame, FIELD_INTEGER),
DEFINE_UTLVECTOR(attackActivites, FIELD_INTEGER),
DEFINE_UTLVECTOR(allWeapons, FIELD_STRING),
DEFINE_UTLVECTOR(allObjectWeapons, FIELD_CLASSPTR),
DEFINE_FIELD(iVMSkin, FIELD_INTEGER),
DEFINE_FIELD(m_bSilenced, FIELD_BOOLEAN),
DEFINE_FIELD(m_bWeaponBlockWall, FIELD_BOOLEAN),
DEFINE_FIELD(m_bShouldBeLowered, FIELD_BOOLEAN),
DEFINE_FIELD(Pickable, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextPick, FIELD_TIME),
DEFINE_FIELD(m_flNextLowerTime, FIELD_TIME),
DEFINE_FIELD(m_flNextWallClipTime, FIELD_TIME),
//DEFINE_FIELD(m_flNextRemoveAmmo, FIELD_TIME),
//DEFINE_FIELD(m_flAmmoRemoveDelay, FIELD_FLOAT),
DEFINE_FIELD(soundInitialVolume, FIELD_FLOAT),
DEFINE_FIELD(soundInitialPitch, FIELD_FLOAT),
DEFINE_FIELD(thisType, FIELD_INTEGER),
DEFINE_FIELD(bg, FIELD_INTEGER),
DEFINE_FIELD(bgV, FIELD_INTEGER),
DEFINE_FIELD(PumpAfterTwoBarrels, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
DEFINE_FIELD(reloadFromEmpty, FIELD_BOOLEAN),
DEFINE_FIELD(IronSightState, FIELD_INTEGER),
DEFINE_FIELD(ScopeSightState, FIELD_BOOLEAN),
DEFINE_FIELD(clipNearWall, FIELD_BOOLEAN),
DEFINE_FIELD(distanceToWall, FIELD_FLOAT),
DEFINE_FIELD(wasInIronSighted, FIELD_BOOLEAN),
DEFINE_FIELD(m_iShotgunReloadState, FIELD_INTEGER),
DEFINE_FIELD(m_iMuzzleFlashState, FIELD_INTEGER),
DEFINE_FIELD(wasburstFire, FIELD_BOOLEAN),
DEFINE_FIELD(EnableLaser, FIELD_BOOLEAN),
DEFINE_FIELD(blockToAmmoRegen, FIELD_BOOLEAN),
DEFINE_FIELD(EnableLaserInterrupt, FIELD_BOOLEAN),
DEFINE_FIELD(m_hLaserMuzzleSprite, FIELD_EHANDLE),
DEFINE_FIELD(tr_endPos, FIELD_VECTOR),
DEFINE_FIELD(iMuzzle, FIELD_INTEGER),
DEFINE_FIELD(m_flNextSwitchTime, FIELD_TIME),
DEFINE_FIELD(m_flNextSilencer, FIELD_TIME),
DEFINE_FIELD(m_flHolsterTime, FIELD_TIME),
DEFINE_FIELD(m_flNextInspectAnimation, FIELD_TIME),
DEFINE_FIELD(m_flNextLaserSwitchTime, FIELD_TIME),
DEFINE_FIELD(m_flAccuracyPenalty, FIELD_FLOAT), //NOTENOTE: This is NOT tracking game time
DEFINE_FIELD(m_flSoonestPrimaryAttack, FIELD_TIME),
DEFINE_FIELD(m_flNextShotgunReload, FIELD_TIME),
DEFINE_FIELD(m_flNextAmmoRegen, FIELD_TIME),
DEFINE_FIELD(m_flNextAmmoRemove, FIELD_TIME),
DEFINE_FIELD(m_bFireMode, FIELD_INTEGER),
DEFINE_FIELD(burstFireCount, FIELD_INTEGER),
DEFINE_FIELD(m_bFireModeCounter, FIELD_INTEGER),
DEFINE_FIELD(m_bFireModesSize, FIELD_INTEGER),
DEFINE_AUTO_ARRAY(m_bFireModes, FIELD_INTEGER),
DEFINE_FIELD(switched, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFirstEquip, FIELD_BOOLEAN),
DEFINE_FIELD(m_bLowered2, FIELD_BOOLEAN),
DEFINE_FIELD(m_flRaiseTime2, FIELD_TIME),
DEFINE_FIELD(m_nShotsFired, FIELD_INTEGER),
DEFINE_FIELD(m_flLastAttackTime, FIELD_TIME),
DEFINE_FIELD(m_bZoomUpped, FIELD_BOOLEAN),
DEFINE_FIELD(m_bZoomLevel, FIELD_INTEGER),
DEFINE_FIELD(m_flNextPrimaryAttack, FIELD_TIME),
DEFINE_FIELD(m_flNextSecondaryAttack, FIELD_TIME),
DEFINE_FIELD(m_flTimeWeaponIdle, FIELD_TIME),
DEFINE_FIELD(m_bInReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFireOnEmpty, FIELD_BOOLEAN),
DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
DEFINE_FIELD(m_iState, FIELD_INTEGER),
DEFINE_FIELD(m_iszName, FIELD_STRING),
DEFINE_FIELD(m_iPrimaryAmmoType, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondaryAmmoType, FIELD_INTEGER),
DEFINE_FIELD(m_iClip1, FIELD_INTEGER),
DEFINE_FIELD(m_iClip2, FIELD_INTEGER),
DEFINE_FIELD(m_bFiresUnderwater, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAltFiresUnderwater, FIELD_BOOLEAN),
DEFINE_FIELD(m_fMinRange1, FIELD_FLOAT),
DEFINE_FIELD(m_fMinRange2, FIELD_FLOAT),
DEFINE_FIELD(m_fMaxRange1, FIELD_FLOAT),
DEFINE_FIELD(m_fMaxRange2, FIELD_FLOAT),
DEFINE_FIELD(m_iPrimaryAmmoCount, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondaryAmmoCount, FIELD_INTEGER),
DEFINE_FIELD(m_nViewModelIndex, FIELD_INTEGER),
DEFINE_FIELD(m_nIdealSequence, FIELD_INTEGER),
DEFINE_FIELD(m_IdealActivity, FIELD_INTEGER),
DEFINE_FIELD(m_fFireDuration, FIELD_FLOAT),
DEFINE_FIELD(m_bReloadsSingly, FIELD_BOOLEAN),
DEFINE_FIELD(m_iSubType, FIELD_INTEGER),
DEFINE_FIELD(m_bRemoveable, FIELD_BOOLEAN),
DEFINE_FIELD(m_flUnlockTime, FIELD_TIME),
DEFINE_FIELD(m_hLocker, FIELD_EHANDLE),
DEFINE_PHYSPTR(m_pConstraint),
DEFINE_FIELD(m_iReloadHudHintCount, FIELD_INTEGER),
DEFINE_FIELD(m_iAltFireHudHintCount, FIELD_INTEGER),
DEFINE_FIELD(m_bReloadHudHintDisplayed, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAltFireHudHintDisplayed, FIELD_BOOLEAN),
DEFINE_FIELD(m_flHudHintPollTime, FIELD_TIME),
DEFINE_FIELD(m_flHudHintMinDisplayTime, FIELD_TIME),
DEFINE_FIELD(m_bEnableMainLaser, FIELD_BOOLEAN),
// Function pointers
DEFINE_ENTITYFUNC(DefaultTouch),
DEFINE_THINKFUNC(FallThink),
DEFINE_THINKFUNC(Materialize),
DEFINE_THINKFUNC(AttemptToMaterialize),
DEFINE_THINKFUNC(DestroyItem),
DEFINE_THINKFUNC(SetPickupTouch),
DEFINE_THINKFUNC(HideThink),
DEFINE_INPUTFUNC(FIELD_VOID, "HideWeapon", InputHideWeapon),
// Outputs
DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_OUTPUT(m_OnPlayerPickup, "OnPlayerPickup"),
DEFINE_OUTPUT(m_OnNPCPickup, "OnNPCPickup"),
DEFINE_OUTPUT(m_OnCacheInteraction, "OnCacheInteraction"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Only send to local player if this weapon is the active weapon
// Input  : *pStruct - 
//			*pVarData - 
//			*pRecipients - 
//			objectID - 
// Output : void*
//-----------------------------------------------------------------------------
void* SendProxy_SendActiveLocalWeaponDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if (pWeapon)
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer(pWeapon->GetOwner());
		if (pPlayer /*&& pPlayer->GetActiveWeapon() == pWeapon*/)
		{
			pRecipients->SetOnly(pPlayer->GetClientIndex());
			return (void*)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendActiveLocalWeaponDataTable);

//-----------------------------------------------------------------------------
// Purpose: Only send the LocalWeaponData to the player carrying the weapon
//-----------------------------------------------------------------------------
void* SendProxy_SendLocalWeaponDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if (pWeapon)
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer(pWeapon->GetOwner());
		if (pPlayer)
		{
			pRecipients->SetOnly(pPlayer->GetClientIndex());
			return (void*)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendLocalWeaponDataTable);

//-----------------------------------------------------------------------------
// Purpose: Only send to non-local players
//-----------------------------------------------------------------------------
void* SendProxy_SendNonLocalWeaponDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	pRecipients->SetAllRecipients();

	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if (pWeapon)
	{
		CBasePlayer *pPlayer = ToBasePlayer(pWeapon->GetOwner());
		if (pPlayer)
		{
			pRecipients->ClearRecipient(pPlayer->GetClientIndex());
			return (void *)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendNonLocalWeaponDataTable);

#endif

#if PREDICTION_ERROR_CHECK_LEVEL > 1
#define SendPropTime SendPropFloat
#define RecvPropTime RecvPropFloat
#endif

//-----------------------------------------------------------------------------
// Purpose: Propagation data for weapons. Only sent when a player's holding it.
//-----------------------------------------------------------------------------
BEGIN_NETWORK_TABLE_NOBASE(CBaseCombatWeapon, DT_LocalActiveWeaponData)
#if !defined( CLIENT_DLL )
SendPropTime(SENDINFO(m_flNextPrimaryAttack)),
SendPropTime(SENDINFO(m_flNextSecondaryAttack)),
SendPropTime(SENDINFO(m_flNextSilencer)),
SendPropTime(SENDINFO(m_flNextInspectAnimation)),
SendPropInt(SENDINFO(m_nNextThinkTick)),
SendPropTime(SENDINFO(m_flTimeWeaponIdle)),
SendPropTime(SENDINFO(m_flHolsterTime)),
SendPropTime(SENDINFO(m_flNextShotgunReload)),
SendPropTime(SENDINFO(m_flNextAmmoRegen)),
SendPropTime(SENDINFO(m_flNextAmmoRemove)),
SendPropInt(SENDINFO(m_bZoomLevel)),
SendPropInt(SENDINFO(m_bFireMode)),
SendPropInt(SENDINFO(iMuzzle)),
SendPropInt(SENDINFO(m_iMuzzleFlashState)),
SendPropInt(SENDINFO(burstFireCount)),
SendPropVector(SENDINFO(tr_endPos), -1, SPROP_NORMAL),
SendPropBool(SENDINFO(EnableLaser)),
SendPropBool(SENDINFO(m_bFirstEquip)),
SendPropBool(SENDINFO(m_bSilenced)),
SendPropBool(SENDINFO(m_bIsFiring)),
SendPropBool(SENDINFO(m_bIsInPrimaryAttack)),
SendPropBool(SENDINFO(m_bNeedPump)),
SendPropBool(SENDINFO(wasburstFire)),
SendPropBool(SENDINFO(reloadFromEmpty)),
SendPropBool(SENDINFO(switched)),
SendPropBool(SENDINFO(letViewModelBob)),
SendPropBool(SENDINFO(PumpAfterTwoBarrels)),
SendPropBool(SENDINFO(EnableLaserInterrupt)),
SendPropBool(SENDINFO(wasInIronSighted)),
SendPropBool(SENDINFO(m_bEnableMainLaser)),
SendPropInt(SENDINFO(IronSightState)),
SendPropBool(SENDINFO(ScopeSightState)),
SendPropInt(SENDINFO(bg)),
SendPropInt(SENDINFO(bgV)),
SendPropInt(SENDINFO(m_iShotgunReloadState)),
SendPropString(SENDINFO(nameOfFireMode)),
SendPropBool(SENDINFO(m_bShouldBeLowered)),
SendPropBool(SENDINFO(m_bWeaponBlockWall)),
SendPropBool(SENDINFO(clipNearWall)),
SendPropBool(SENDINFO(m_bInReload)),
SendPropBool(SENDINFO(m_bShouldDrawWeaponBloodOverlay)),
SendPropInt(SENDINFO(m_iWeaponBloodOverlayDetailFrame)),
SendPropFloat(SENDINFO(distanceToWall)),
#if defined( TF_DLL )
SendPropExclude("DT_AnimTimeMustBeFirst", "m_flAnimTime"),
#endif

#else
RecvPropTime(RECVINFO(m_flNextPrimaryAttack)),
RecvPropTime(RECVINFO(m_flNextSecondaryAttack)),
RecvPropTime(RECVINFO(m_flNextSilencer)),
RecvPropTime(RECVINFO(m_flHolsterTime)),
RecvPropTime(RECVINFO(m_flNextShotgunReload)),
RecvPropTime(RECVINFO(m_flNextAmmoRegen)),
RecvPropTime(RECVINFO(m_flNextAmmoRemove)),
RecvPropTime(RECVINFO(m_flNextInspectAnimation)),
RecvPropInt(RECVINFO(m_nNextThinkTick)),
RecvPropInt(RECVINFO(burstFireCount)),
RecvPropBool(RECVINFO(wasburstFire)),
RecvPropBool(RECVINFO(m_bFirstEquip)),
RecvPropTime(RECVINFO(m_flTimeWeaponIdle)),
RecvPropInt(RECVINFO(m_bZoomLevel)),
RecvPropInt(RECVINFO(m_bFireMode)),
RecvPropInt(RECVINFO(iMuzzle)),
RecvPropInt(RECVINFO(m_iMuzzleFlashState)),
RecvPropVector(RECVINFO(tr_endPos)),
RecvPropBool(RECVINFO(EnableLaser)),
RecvPropBool(RECVINFO(switched)),
RecvPropBool(RECVINFO(letViewModelBob)),
RecvPropBool(RECVINFO(m_bSilenced)),
RecvPropBool(RECVINFO(m_bIsFiring)),
RecvPropBool(RECVINFO(m_bIsInPrimaryAttack)),
RecvPropBool(RECVINFO(m_bNeedPump)),
RecvPropBool(RECVINFO(PumpAfterTwoBarrels)),
RecvPropBool(RECVINFO(reloadFromEmpty)),
RecvPropBool(RECVINFO(EnableLaserInterrupt)),
RecvPropBool(RECVINFO(wasInIronSighted)),
RecvPropBool(RECVINFO(m_bEnableMainLaser)),
RecvPropString(RECVINFO(nameOfFireMode)),
RecvPropInt(RECVINFO(IronSightState)),
RecvPropBool(RECVINFO(ScopeSightState)),
RecvPropInt(RECVINFO(bg)),
RecvPropInt(RECVINFO(bgV)),
RecvPropInt(RECVINFO(m_iShotgunReloadState)),
RecvPropBool(RECVINFO(m_bShouldBeLowered)),
RecvPropBool(RECVINFO(m_bWeaponBlockWall)),
RecvPropBool(RECVINFO(clipNearWall)),
RecvPropBool(RECVINFO(m_bInReload)),
RecvPropBool(RECVINFO(m_bShouldDrawWeaponBloodOverlay)),
RecvPropInt(RECVINFO(m_iWeaponBloodOverlayDetailFrame)),
RecvPropFloat(RECVINFO(distanceToWall)),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Propagation data for weapons. Only sent when a player's holding it.
//-----------------------------------------------------------------------------
BEGIN_NETWORK_TABLE_NOBASE(CBaseCombatWeapon, DT_LocalWeaponData)
#if !defined( CLIENT_DLL )
SendPropIntWithMinusOneFlag(SENDINFO(m_iClip1), 8),
SendPropIntWithMinusOneFlag(SENDINFO(m_iClip2), 8),
SendPropInt(SENDINFO(m_iPrimaryAmmoType), 8),
SendPropInt(SENDINFO(m_iSecondaryAmmoType), 8),

SendPropInt(SENDINFO(m_nViewModelIndex), VIEWMODEL_INDEX_BITS, SPROP_UNSIGNED),

SendPropInt(SENDINFO(m_bFlipViewModel)),

#if defined( TF_DLL )
SendPropExclude("DT_AnimTimeMustBeFirst", "m_flAnimTime"),
#endif

#else
RecvPropIntWithMinusOneFlag(RECVINFO(m_iClip1)),
RecvPropIntWithMinusOneFlag(RECVINFO(m_iClip2)),
RecvPropInt(RECVINFO(m_iPrimaryAmmoType)),
RecvPropInt(RECVINFO(m_iSecondaryAmmoType)),

RecvPropInt(RECVINFO(m_nViewModelIndex)),

RecvPropBool(RECVINFO(m_bFlipViewModel)),

#endif
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE(CBaseCombatWeapon, DT_BaseCombatWeapon)
#if !defined( CLIENT_DLL )
SendPropDataTable("LocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalWeaponData), SendProxy_SendLocalWeaponDataTable),
SendPropDataTable("LocalActiveWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponData), SendProxy_SendActiveLocalWeaponDataTable),
SendPropModelIndex(SENDINFO(m_iViewModelIndex)),
SendPropModelIndex(SENDINFO(m_iWorldModelIndex)),
SendPropInt(SENDINFO(m_iState), 8, SPROP_UNSIGNED),
SendPropEHandle(SENDINFO(m_hOwner)),

#else
RecvPropDataTable("LocalWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalWeaponData)),
RecvPropDataTable("LocalActiveWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponData)),
RecvPropInt(RECVINFO(m_iViewModelIndex)),
RecvPropInt(RECVINFO(m_iWorldModelIndex)),
RecvPropInt(RECVINFO(m_iState)),
RecvPropEHandle(RECVINFO(m_hOwner)),

#endif
END_NETWORK_TABLE()
