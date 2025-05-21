//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		357 - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"

#include "weapon_357.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"
#include "IEffects.h"//дым от ствола
#include "beam_shared.h"//For Laser
#include "Sprite.h"//For laser
#include "OverCharged/ShotgunBullet.h"//For laser
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar oc_weapon_357_enable_scope;

//-----------------------------------------------------------------------------
// CWeapon357
//-----------------------------------------------------------------------------
#define	PISTOL_FASTEST_REFIRE_TIME		0.15f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

LINK_ENTITY_TO_CLASS( weapon_357, CWeapon357 );

PRECACHE_WEAPON_REGISTER( weapon_357 );

IMPLEMENT_SERVERCLASS_ST( CWeapon357, DT_Weapon357 )
END_SEND_TABLE()

BEGIN_DATADESC( CWeapon357 )

	DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
	DEFINE_FIELD(m_flAccuracyPenalty, FIELD_FLOAT), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD(m_nNumShotsFired, FIELD_INTEGER),
	DEFINE_FIELD(DoOnceMPD, FIELD_BOOLEAN),

END_DATADESC()

acttable_t CWeapon357::m_acttable[] =
{

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },		// L1ght 15 : MP animstate
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
	{ ACT_IDLE_RELAXED, ACT_IDLE, false },//never aims
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

IMPLEMENT_ACTTABLE(CWeapon357);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon357::CWeapon357( void )
{
	DoOnceMPD = true;
	m_flAccuracyPenalty = 0.0f;
	m_bReloadsSingly	= false;
	m_bInZoom = false;

	//=======BriJee: NPC allow shoot.
	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;
	//=======BriJee: END.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("Weapon_357.Draw");
}

bool CWeapon357::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool rRet;
	pSwitchingTo = NULL;
	rRet = BaseClass::Holster(pSwitchingTo);

	StopEffects();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		pPlayer->SetFOV(this, 0, 0.1f);
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
	}

	return BaseClass::Holster(pSwitchingTo);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_RELOAD:
	{
		ShellOut(pEvent);
	}
	break;
	case EVENT_WEAPON_THROW3:
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		pPlayer->RemoveEffects(EF_LASER);/////////////Убираем лазерку при перезарядке
		StopEffects();
		if (pPlayer->SetFOV(this, 20, 0.2f))
		{
			m_bInZoom = false;
			// Send a message to hide the scope
			pPlayer->SetFOV(this, 0, 0.1f);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
		}
	  }
	break;
	//=======BriJee: Player thirdperson animations & NPC anims, allow shoot.
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
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;		
	//=======BriJee: END. break added.
	}
}

void CWeapon357::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();

	//UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (oc_weapon_357_enable_scope.GetInt() != 1)
		return;

	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if (IsInReload())
	{
		if (pPlayer->SetFOV(this, 20, 0.2f))
		{
			m_bInZoom = false;
			// Send a message to hide the scope
			pPlayer->SetFOV(this, 0, 0.1f);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
		}
	}

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
			// Send a message to hide the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			m_bInZoom = true;
			// Send a message to Show the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(1);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop all zooming and special effects on the viewmodel
//-----------------------------------------------------------------------------
void CWeapon357::StopEffects(void)
{
	// Stop zooming
	if (m_bInZoom)
	{
		ToggleZoom();
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::Drop(const Vector &vecVelocity)
{
	StopEffects();
	BaseClass::Drop(vecVelocity);
}
