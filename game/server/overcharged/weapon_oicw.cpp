//========= Copyright © 1996-2010, Valve Corporation and Cave, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "player.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "AI_Memory.h"
#include "particle_parse.h"//OverCharged
#include "rumble_shared.h"//OverCharged
#include "gamestats.h"//OverCharged
#include "baseviewmodel_shared.h"//OverCharged
#ifdef CLIENT_DLL
#define CWeaponOICW C_WeaponOICW
#endif

#include "weapon_oicw.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SMG1_GRENADE_DAMAGE 100.0f
#define SMG1_GRENADE_RADIUS 250.0f

extern ConVar    sk_plr_dmg_smg1_grenade;


LINK_ENTITY_TO_CLASS(weapon_oicw, CWeaponOICW);
PRECACHE_WEAPON_REGISTER(weapon_oicw);

IMPLEMENT_SERVERCLASS_ST(CWeaponOICW, DT_WeaponOICW)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponOICW)

//DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bMustReload, FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponOICW::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_AR2,                false },		//END

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
};

IMPLEMENT_ACTTABLE(CWeaponOICW);

//=========================================================
CWeaponOICW::CWeaponOICW()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 4400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("grenade_ar2");
#endif
	PrecacheScriptSound("Weapon_oicw.Draw");
	PrecacheScriptSound("Weapon_oicw.EndFire");
	PrecacheScriptSound("Weapon_oicw.EndFire2");
	PrecacheScriptSound("Weapon_oicw.Double");
	PrecacheScriptSound("Weapon_oicw.Reload");
	BaseClass::Precache();
}


/*void CWeaponOICW::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;



	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		if ((m_nShotsFired == 15) || (m_nShotsFired == 30) || (m_nShotsFired == 45) || (m_nShotsFired == 60))
		{
			DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
		}
	}

	BaseClass::PrimaryAttack();
}*/

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponOICW::Equip(CBaseCombatCharacter *pOwner)
{
	m_fMaxRange1 = 1400;

	BaseClass::Equip(pOwner);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*bool CWeaponOICW::Reload(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer && pPlayer->SetFOV(this, 40, 0.1f) != NULL)
	{
		pPlayer->SetFOV(this, 0, 0.1f);
		// Send a message to hide the scope
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_oicw")->SetValue(0);
	}
	return BaseClass::Reload();
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponOICW::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer && (pPlayer->m_afButtonPressed & IN_AIMMODE))
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::ItemPostFrame(void)
{

	CheckZoomToggle();

	if (m_bMustReload && HasWeaponIdleTimeElapsed())
	{
		Reload();
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_oicw")->SetValue(0);
	}

	BaseClass::ItemPostFrame();
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponOICW::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		return;

	if (m_bInReload)
		return;

#ifndef CLIENT_DLL
	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;

			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_oicw")->SetValue(0);
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 40, 0.1f))
		{

			m_bInZoom = true;

			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
			cvar->FindVar("oc_state_InSecondFire_oicw")->SetValue(1);
		}
	}
#endif
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CWeaponOICW::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool bRet;
	pSwitchingTo = NULL;
	bRet = BaseClass::Holster(pSwitchingTo);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer != NULL)
	{
		StopParticleEffects(pPlayer->GetViewModel());
		pPlayer->SetFOV(this, 0, 0.1f);
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
	}
	cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
	cvar->FindVar("oc_state_InSecondFire_oicw")->SetValue(0);

	return bRet;

	return BaseClass::Holster(pSwitchingTo);
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::SecondaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)	
		return;	

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.15f;

	/*if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0) || (pPlayer->GetWaterLevel() == 3))
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.15f;
		return;
	}*/
	if (m_bInReload)
		m_bInReload = false;

	//EmitSound("Weapon_oicw.Double");
	pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);

	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();

	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		vThrowPos += vForward * 22.0f;
		vThrowPos += vRight * 0.05f;
		vThrowPos += vUp * -3.4f;
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		if ((cvar->FindVar("oc_state_InSecondFire")->GetInt() == 1) || (cvar->FindVar("oc_state_InSecondFire_oicw")->GetInt() == 1))
		{
			vThrowPos += vForward * 22.0f;
			vThrowPos += vRight * 0.05f;
			vThrowPos += vUp * -3.4f;
		}
		else if ((cvar->FindVar("oc_state_InSecondFire")->GetInt() == 0) || (cvar->FindVar("oc_state_InSecondFire_oicw")->GetInt() == 0))
		{
			vThrowPos += vForward * 22.0f;
			vThrowPos += vRight * 5.4f;
			vThrowPos += vUp * -3.4f;
		}

	}

	Vector	vForward1, vRight1, vUp1;
	pPlayer->EyeVectors(&vForward1, &vRight1, &vUp1);
	QAngle vecAngles;
	VectorAngles(vForward1, vecAngles);

	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeOICW *pGrenade = (CGrenadeOICW*)Create("grenade_oicw", vThrowPos, vecAngles, pPlayer);
	pGrenade->SetAbsVelocity(vecThrow);

	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

	SendWeaponAnim(GetSecondaryAttackActivity());

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Decrease ammo
	RemoveAmmo(GetSecondaryAmmoType(), 1); //pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.4f;

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

	//Disorient the player
	QAngle angles2 = pPlayer->GetLocalAngles();

	angles2.x += random->RandomInt(-4, 4);
	angles2.y += random->RandomInt(-4, 4);
	angles2.z = 0;

	pPlayer->SnapEyeAngles(angles2);

	pPlayer->ViewPunch(QAngle(random->RandomFloat(-2.f, -4.f), random->RandomFloat(0.2f, 0.5f), 0));

	SecondaryAttackEffects();
}
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponOICW::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

void CWeaponOICW::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
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

	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponOICW::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

/*void CWeaponOICW::Drop(const Vector &vecVelocity)
{
	BaseClass::Drop(vecVelocity);
}*/