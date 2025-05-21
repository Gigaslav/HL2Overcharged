//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		op4 sniper gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
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
#include "IEffects.h"
#include "OverCharged/ShotgunBullet.h"

#include "weapon_sniper_m40a1.h"
#include "tier0/memdbgon.h"

#define	CS_FASTEST_REFIRE_TIME		0.15f
#define	CS_FASTEST_DRY_REFIRE_TIME	0.2f

#define	CS_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	CS_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//-----------------------------------------------------------------------------
// Discrete zoom levels for the scope.	// old 3x scope preset
//-----------------------------------------------------------------------------
static int g_nZoomFOV[] =
{
	20,
	5
};


LINK_ENTITY_TO_CLASS(weapon_sniper_m40a1, CWeaponSniperM40A1);
PRECACHE_WEAPON_REGISTER(weapon_sniper_m40a1);

IMPLEMENT_SERVERCLASS_ST(CWeaponSniperM40A1, DT_WeaponSniperM40A1)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponSniperM40A1)
	DEFINE_FIELD(durationTime, FIELD_FLOAT),
	DEFINE_FIELD(durationTime2, FIELD_FLOAT),
	DEFINE_FIELD(wasInReload, FIELD_BOOLEAN),
END_DATADESC()

acttable_t CWeaponSniperM40A1::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

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

IMPLEMENT_ACTTABLE(CWeaponSniperM40A1);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSniperM40A1::CWeaponSniperM40A1(void)
{
	m_bReloadsSingly = false;

	//=======BriJee: NPC allow shoot.
	m_fMinRange1 = 24;
	m_fMinRange2 = 24;
	m_fMaxRange1 = 8000;
	m_fMaxRange2 = 8000;
	//Once = false;
	wasInReload = false;
	durationTime = 0;
	durationTime2 = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperM40A1::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_Sniper_m40a1.Pump");
	PrecacheScriptSound("Weapon_Sniper_m40a1.PumpFail");
}

/*void CWeaponSniperM40A1::Pump(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	if (m_iClip1 <= 0)
	{
		//EmitSound("Weapon_Sniper_m40a1.PumpFail");
		WeaponSound(SPECIAL2);
	}
	else
	{
		//EmitSound("Weapon_Sniper_m40a1.Pump");
		WeaponSound(SPECIAL1);
	}

	m_bNeedPump = false;
}

void CWeaponSniperM40A1::PrimaryAttack()
{
	m_bNeedPump = true;

	durationTime = gpGlobals->curtime + GetFireRate()*0.3f;

	BaseClass::PrimaryAttack();
}
void CWeaponSniperM40A1::ItemPostFrame(void)
{
	if (m_bNeedPump && gpGlobals->curtime > durationTime && m_iClip1 > 0)
	{
		Pump();
	}

	BaseClass::ItemPostFrame();
}*/

bool CWeaponSniperM40A1::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool rRet;
	pSwitchingTo = NULL;
	rRet = BaseClass::Holster(pSwitchingTo);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	StopParticleEffects(this);
	StopParticleEffects(pPlayer);

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponSniperM40A1::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	WeaponSound(SINGLE_NPC);
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperM40A1::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSniperM40A1::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{

	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
	{
		ShellOut(pEvent);
	}
	break;
	case EVENT_WEAPON_THROW3:
	{
		/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

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
		}*/

	}
	break;
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;
	//=======BriJee: Player thirdperson animations & NPC anims, allow shoot.
	case 4://EVENT_WEAPON_PISTOL_FIRE
	{
		DevMsg("Npc Shot \n");
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