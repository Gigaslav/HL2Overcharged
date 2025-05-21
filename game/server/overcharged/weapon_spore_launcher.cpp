//========= Copyright © 1996-2022 Overcharged ====================================//
//
// Purpose: Spore Launcher op4 recreation
//
//
//=============================================================================//
#include "cbase.h"
#include "NPCEvent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "effect_dispatch_data.h"
#include "BloodDripsGreen.h"

#include "weapon_spore_launcher.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BOLT_AIR_VELOCITY	1100

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

ConVar oc_weapon_spore_launcher_projectile_x("oc_weapon_spore_launcher_projectile_x", "0");
ConVar oc_weapon_spore_launcher_projectile_y("oc_weapon_spore_launcher_projectile_y", "0");
ConVar oc_weapon_spore_launcher_projectile_z("oc_weapon_spore_launcher_projectile_z", "0");

BEGIN_DATADESC(CWeaponSporeLauncher)

DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFire, FIELD_BOOLEAN),
DEFINE_FIELD(m_nShotsFired, FIELD_INTEGER),
DEFINE_FIELD(ChangeOnce, FIELD_BOOLEAN),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponSporeLauncher, DT_WeaponSporeLauncher)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_spore_launcher, CWeaponSporeLauncher);
PRECACHE_WEAPON_REGISTER(weapon_spore_launcher);

acttable_t CWeaponSporeLauncher::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },	// BJ : MP animstate for singleplayer
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

IMPLEMENT_ACTTABLE(CWeaponSporeLauncher);

CWeaponSporeLauncher::CWeaponSporeLauncher(void)
{
	m_bReloadsSingly = true;
	m_bNeedPump = false;
	m_bDelayedFire = false;
	ChangeOnce = true;
	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 200;

	m_nShotsFired = 0;
}

void CWeaponSporeLauncher::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

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

	info.m_vecSpread = pOperator->GetAttackSpread(this);

	if (((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 0)) || ((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 2)))
	{
		pOperator->FireBullets(8, vecShootOrigin, vecShootDir, info.m_vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
	else if ((cvar->FindVar("oc_weapons_enable_dynamic_bullets")->GetInt() == 1))
	{
		pOperator->FireBulletsShotgun(8, vecShootOrigin, vecShootDir, info.m_vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1);
	}
}

void CWeaponSporeLauncher::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

void CWeaponSporeLauncher::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CWeaponSporeLauncher::PrimaryAttackShotgun()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if (!pPlayer)
	{
		return;
	}

	WeaponSound(SINGLE);
	pPlayer->DoMuzzleFlash();
	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	//m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.55f; //+ GetViewModelSequenceDuration();	// L1ght 15: Fix-up animations

	if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt() == 1)
	{
		if (GetDefaultClip1() > 0)
			m_iClip1 -= 1;
	}
	else
		m_iClip1 -= 1;



	FireBolt();

	//pPlayer->ViewPunch( QAngle( random->RandomFloat( -2, -1 ), random->RandomFloat( -2, 2 ), 0 ) );
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner());

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	if (m_iClip1)
	{
		m_bNeedPump = true;
	}
	//PrepareHitmarker();
	AddViewKick();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSporeLauncher::FireBolt(void)
{
	if (m_iPrimaryAmmoType <= 0) //m_iClip1 <= 0 )
	{

		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = 0.2;


		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	pOwner->AbortReload();	//test

#ifndef CLIENT_DLL
	/*Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;		// BriJee OVR: Projectile start position
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();
	vThrowPos += vForward * 22.0f;
	vThrowPos += vRight * 7.8f;
	vThrowPos += vUp * -8.8f;


	QAngle angAiming;
	VectorAngles(vThrowPos, angAiming);


	CSporeLauncherProjectile *pSporeLauncherProjectile = CSporeLauncherProjectile::BoltCreate(vThrowPos, angAiming, 0, pOwner); // BriJee OVR : Better start angles

	if (pOwner->GetWaterLevel() == 3)
	{
		CTakeDamageInfo hitself(this, this, 210, DMG_SHOCK);
		pOwner->TakeDamage(hitself);
	}
	else
	{
		pSporeLauncherProjectile->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}*/
	info.m_vecSpread = pOwner->GetAttackSpread(this);
	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecAimingSpreaded = info.m_vecSpread;

	Vector vecMuzzlePos2; Vector vecEndPos2;
	QAngle angDir;
	Vector Muzzle = pOwner->Weapon_ShootPosition(1);
	Muzzle.x += oc_weapon_spore_launcher_projectile_x.GetFloat();
	Muzzle.y += oc_weapon_spore_launcher_projectile_y.GetFloat();
	Muzzle.z += oc_weapon_spore_launcher_projectile_z.GetFloat();

	Vector vThrowPos;
	vThrowPos = GetClientTracerVector();
	/*vThrowPos.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	vThrowPos.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	vThrowPos.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/

	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), vThrowPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	//Cgrenade_strooper_instant *pSporeLauncherProjectile = Cgrenade_strooper_instant::BoltCreate(Muzzle, angAiming, 0, pOwner); // BriJee OVR : Better start angles

	//CBaseEntity *pVial = NULL;
	//pVial = CBaseEntity::Create("grenade_instant", Muzzle, angAiming, this); // Creates
	CSporeLauncherProjectile *pSporeLauncherProjectile = CSporeLauncherProjectile::BoltCreate(tr.endpos, angAiming, 0, pOwner);

	pSporeLauncherProjectile->PrecacheModel("models/spitball_medium.mdl");
	pSporeLauncherProjectile->SetModel("models/spitball_medium.mdl");
	pSporeLauncherProjectile->SetMoveType(MOVETYPE_VPHYSICS);
	pSporeLauncherProjectile->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	pSporeLauncherProjectile->SetDamage(cvar->FindVar("sk_plr_dmg_spore_acid_bounce")->GetFloat());
	pSporeLauncherProjectile->Spawn();
#endif

	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);

	pOwner->ViewPunch(QAngle(-2, 0, 0));

	//WeaponSound(SINGLE);
	//WeaponSound(SPECIAL2);

	SendWeaponAnim(GetPrimaryAttackActivity());

	m_flNextShotgunReload = m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();//GetFireRate();

	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

}
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponSporeLauncher::SecondaryAttackShotgun(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	pPlayer->m_nButtons &= ~IN_ATTACK2;
	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(WPN_DOUBLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetSecondaryAttackActivity());

	m_flNextShotgunReload = m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate() * 2.f;//GetFireRate();
	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Don't fire again until fire animation has completed
	//m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();	// overcharged: 3rd person anims bug fix
	//m_flNextPrimaryAttack = gpGlobals->curtime + 1.2f;

	if (cvar->FindVar("oc_weapons_infinite_ammo")->GetInt() == 1)
	{
		if (GetDefaultClip1() > 0)
			m_iClip1 -= 1;		// BJ: Twi told to fix 1 ammo shot for alt attack
	}
	else
	m_iClip1 -= 1;				// BJ: Twi told to fix 1 ammo shot for alt attack

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	// Fire the bullets
	//pPlayer->FireBullets(18, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, -1, 1, 0, NULL, false, false);



	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	FireBoltBounced();

	AddViewKick();

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);

	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2 );

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	if (m_iClip1)
	{
		// pump so long as some rounds are left.
		m_bNeedPump = true;
	}

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

	//	CBasePlayer *pOwner = ToBasePlayer(GetOwner());


	Vector aimDir = pOwner->GetAutoaimVector(AUTOAIM_5DEGREES);
	trace_t tr;
	float flDamage = 20;
	CTakeDamageInfo dmgInfo(this, pOwner, flDamage, DMG_SHOCK);
	CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos);
	// Отдача как из Opposing force 
	Vector recoilForce = pOwner->GetAbsVelocity() - pOwner->GetAutoaimVector(0) * (flDamage * 5.0f);
	recoilForce[2] += 100.0f;
	pOwner->SetAbsVelocity(recoilForce); // отдача на тело игрока

	DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
	DispatchParticleEffect("weapon_dust_stream", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
}

void CWeaponSporeLauncher::FireBoltBounced(void)
{
	if (m_iPrimaryAmmoType <= 0) //m_iClip1 <= 0 )
	{

		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = 0.15;
		m_flNextSecondaryAttack = 0.15;


		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	pOwner->AbortReload();	//test

#ifndef CLIENT_DLL
	info.m_vecSpread = pOwner->GetAttackSpread(this);
	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecAimingSpreaded = info.m_vecSpread;
	/*Vector Muzzle;

	Muzzle.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	Muzzle.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	Muzzle.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/


	Vector vecMuzzlePos2; Vector vecEndPos2;
	QAngle angDir;
	/*Vector Muzzle, vecRes, vecX, vecY, vecZ;
	pOwner->GetViewModel()->GetAttachment(1, Muzzle, angDir);
	AngleVectors(angDir, &vecX, &vecY, &vecZ);
	Muzzle += vecX * oc_weapon_spore_launcher_projectile_x.GetFloat();
	Muzzle += vecY * oc_weapon_spore_launcher_projectile_y.GetFloat();
	Muzzle += vecZ * oc_weapon_spore_launcher_projectile_z.GetFloat();*/

	Vector Muzzle;
	Muzzle = GetClientTracerVector();
	/*Muzzle.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	Muzzle.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	Muzzle.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/

	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), Muzzle, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);


	//Cgrenade_strooper_instant *pSporeLauncherProjectile = Cgrenade_strooper_instant::BoltCreate(Muzzle, angAiming, 0, pOwner); // BriJee OVR : Better start angles

	CSporeLauncherBouncedProjectile *pSporeLauncherProjectile = CSporeLauncherBouncedProjectile::SporeCreate(tr.endpos, vecAiming, angAiming, pOwner);

	pSporeLauncherProjectile->PrecacheModel("models/spitball_medium.mdl");
	pSporeLauncherProjectile->SetModel("models/spitball_medium.mdl");
	pSporeLauncherProjectile->SetMoveType(MOVETYPE_VPHYSICS);
	pSporeLauncherProjectile->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	pSporeLauncherProjectile->SetDamage(cvar->FindVar("sk_plr_dmg_spore_acid_bounce")->GetFloat());
	pSporeLauncherProjectile->Spawn();

#endif

	pOwner->ViewPunch(QAngle(-2, 0, 0));

	SendWeaponAnim(GetPrimaryAttackActivity());

	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

}
