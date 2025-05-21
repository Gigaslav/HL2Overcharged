//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "weapon_rpg.h"
#include "triggers.h"
#include "AI_Memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "Filesystem.h"
#include "weapon_custom.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar    sk_plr_dmg_smg1_grenade;

//LINK_ENTITY_TO_CLASS( weapon_custom1, CWeaponCustom ); //I give up.
//PRECACHE_WEAPON_REGISTER(weapon_custom1);

//LINK_ENTITY_TO_CLASS( weapon_custom2, CWeaponCustom );
//PRECACHE_WEAPON_REGISTER(weapon_custom2);

//LINK_ENTITY_TO_CLASS( weapon_custom3, CWeaponCustom );
//PRECACHE_WEAPON_REGISTER(weapon_custom3);

//LINK_ENTITY_TO_CLASS( weapon_custom4, CWeaponCustom );
//PRECACHE_WEAPON_REGISTER(weapon_custom4);
//LINK_ENTITY_TO_CLASS_CUSTOM("weapon_ak47",CWeaponCustom)

IMPLEMENT_SERVERCLASS_ST(CWeaponCustom, DT_WeaponCustom)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponCustom)
DEFINE_FIELD(m_hMissile, FIELD_EHANDLE),
END_DATADESC()

//=========================================================
CWeaponCustom::CWeaponCustom()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;

	m_bAltFiresUnderwater = false;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Precache(void)
{

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponCustom::Equip(CBaseCombatCharacter *pOwner)
{
	if (pOwner->Classify() == CLASS_PLAYER_ALLY)
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip(pOwner);
}
void CWeaponCustom::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	if (UsesClipsForAmmo1())
	{
		CheckReload();
	}
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if (!IsMeleeWeapon() &&
			((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw

			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			SecondaryAttack();
		}
	}
	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if (!IsMeleeWeapon() &&
			((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw

			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	/*if (IsPrimaryBullet() == true)
	{
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
			MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);
		FireBulletsInfo_t info;
		info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
		info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
		info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
		info.m_vecSrc = vecShootOrigin;
		info.m_vecDirShooting = vecShootDir;
		info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 2;
		info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
		FireBullets(info);
		pOperator->DoMuzzleFlash();
	}*/
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;

	/*//FIXME: Re-enable
	case EVENT_WEAPON_AR2_GRENADE:
	{
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );
	Vector vecThrow = m_vecTossVelocity;
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
	pGrenade->SetAbsVelocity( vecThrow );
	pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY );
	pGrenade->m_hOwner			= npc;
	pGrenade->m_pMyWeaponAR2	= this;
	pGrenade->SetDamage(sk_npc_dmg_ar2_grenade.GetFloat());
	// FIXME: arrgg ,this is hard coded into the weapon???
	m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
	m_iClip2--;
	}
	break;
	*/

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
void CWeaponCustom::ShootBullets(bool isPrimary, bool usePrimaryAmmo)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	if (isPrimary || usePrimaryAmmo)
	{
		// Abort here to handle burst and auto fire modes
		if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
			return;
	}
	else
	{
		// Abort here to handle burst and auto fire modes
		if ((UsesClipsForAmmo2() && m_iClip2 == 0) || (!UsesClipsForAmmo2() && !pPlayer->GetAmmoCount(m_iSecondaryAmmoType)))
			return;
	}
	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;

	//float fireRate = (isPrimary) ? GetFireRate() : this->GetWpnData().m_sSecondaryFireRate;
	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		if (isPrimary)
			WeaponSound(SINGLE, m_flNextPrimaryAttack);
		else
			WeaponSound(WPN_DOUBLE, m_flNextPrimaryAttack);
		//m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}
	if (isPrimary || usePrimaryAmmo)
	{
		m_iPrimaryAttacks++;
		// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
		if (UsesClipsForAmmo1())
		{
			if (iBulletsToFire > m_iClip1)
				iBulletsToFire = m_iClip1;
			m_iClip1 -= iBulletsToFire;
		}
	}
	else
	{
		m_iSecondaryAttacks++;
		// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
		if (UsesClipsForAmmo2())
		{
			if (iBulletsToFire > m_iClip2)
				iBulletsToFire = m_iClip2;
			m_iClip2 -= iBulletsToFire;
		}
	}
	gamestats->Event_WeaponFired(pPlayer, isPrimary, GetClassname());

	// Fire the bullets
	FireBulletsInfo_t info;
	/*if (isPrimary)
	{
		info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
		info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
		info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
		info.m_vecSpread = GetBulletSpreadPrimary();
		info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
		info.m_flPenetrationForce = this->GetWpnData().m_flPrimaryPenetrateDepth;
	}
	else
	{
		info.m_iShots = this->GetWpnData().m_sSecondaryShotCount;
		info.m_flDamage = this->GetWpnData().m_sSecondaryDamage;
		info.m_iPlayerDamage = this->GetWpnData().m_sSecondaryDamage;
		info.m_vecSpread = GetBulletSpreadSecondary();
		info.m_iPenetrationCount = this->GetWpnData().m_iSecondaryPenetrateCount;
		info.m_flPenetrationForce = this->GetWpnData().m_flSecondaryPenetrateDepth;
	}*/
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets(info);

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
}

#ifdef HL2_DLL
extern int g_interactionPlayerLaunchedRPG;
#endif

void CWeaponCustom::ShootProjectile(bool isPrimary, bool usePrimaryAmmo)
{

	// Can't have an active missile out
	if (m_hMissile != NULL)
		return;

	// Can't be reloading
	if (GetActivity() == ACT_VM_RELOAD)
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;

	QAngle vecAngles;
	VectorAngles(vForward, vecAngles);
	m_hMissile = CMissile::Create(muzzlePoint, vecAngles, GetOwner()->edict());

	//	m_hMissile->m_hOwner = this;

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction == 1.0)
	{
		m_hMissile->SetGracePeriod(0.3);
	}


	// Register a muzzleflash for the AI
	pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	WeaponSound(SINGLE);

	pOwner->RumbleEffect(RUMBLE_SHOTGUN_SINGLE, 0, RUMBLE_FLAG_RESTART);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pOwner, true, GetClassname());

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// Check to see if we should trigger any RPG firing triggers
	//Yes, these now work.
	int iCount = g_hWeaponFireTriggers.Count();
	for (int i = 0; i < iCount; i++)
	{
		if (g_hWeaponFireTriggers[i]->IsTouching(pOwner))
		{
			if (FClassnameIs(g_hWeaponFireTriggers[i], "trigger_rpgfire"))
			{
				g_hWeaponFireTriggers[i]->ActivateMultiTrigger(pOwner);
			}
		}
	}

	if (hl2_episodic.GetBool())
	{
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();

		string_t iszStriderClassname = AllocPooledString("npc_strider");

		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i]->m_iClassname == iszStriderClassname)
			{
				ppAIs[i]->DispatchInteraction(g_interactionPlayerLaunchedRPG, NULL, m_hMissile);
			}
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponCustom::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
		return ACT_VM_PRIMARYATTACK;

	if (m_nShotsFired < 3)
		return ACT_VM_RECOIL1;

	if (m_nShotsFired < 4)
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

void CWeaponCustom::PrimaryAttack(void)
{
	/*if (this->GetWpnData().m_sPrimaryMissleEnabled)
		this->ShootProjectile(true, true);
	else if (IsPrimaryBullet() == true)
		ShootBullets(true, true);*/
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponCustom::Reload(void)
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound(RELOAD);
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	1.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::SecondaryAttack(void)
{
	/*if (this->GetWpnData().m_sSecondaryMissleEnabled)
		this->ShootProjectile(false, this->GetWpnData().m_sUsePrimaryAmmo);
	else if (IsSecondaryBullet())
		ShootBullets(false, this->GetWpnData().m_sUsePrimaryAmmo);*/
}

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponCustom::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	return COND_NONE;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponCustom::GetProficiencyValues()
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