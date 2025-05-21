//========= Copyright © 2011-2018. Overcharged ================================//
//
// Purpose: 
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
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "sprite.h"
#include "spritetrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "particle_parse.h"
#include "grenade_spit.h"
#include "particle_system.h"
#include "effect_dispatch_data.h"

#include "weapon_hornetgun.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BOLT_MODEL	oc_weapon_hornetgun_model.GetString() //"models/hornetgun.mdl"
#define GLOW_SPRITE	"particles/particle_glow/particle_glow_03.vmt"
#define BOLT_AIR_VELOCITY	1500
#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1
#define	SHOCK_BEAM_SPRITE		"sprites/laserbeam.vmt"

ConVar oc_weapon_hornetgun_model("oc_weapon_hornetgun_model", "models/spitball_small.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");

ConVar sk_plr_dmg_hornetgun("sk_plr_dmg_hornetgun", "30");
ConVar sk_npc_dmg_hornetgun("sk_npc_dmg_hornetgun", "30");

ConVar oc_weapon_hornetgun_projectile_x("oc_weapon_hornetgun_projectile_x", "0");
ConVar oc_weapon_hornetgun_projectile_y("oc_weapon_hornetgun_projectile_y", "0");
ConVar oc_weapon_hornetgun_projectile_z("oc_weapon_hornetgun_projectile_z", "0");



LINK_ENTITY_TO_CLASS(weapon_hornetgun, CWeaponHornetGun);

PRECACHE_WEAPON_REGISTER(weapon_hornetgun);

IMPLEMENT_SERVERCLASS_ST(CWeaponHornetGun, DT_WeaponHornetGun)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponHornetGun)

DEFINE_FIELD(m_hLaserBeam1, FIELD_EHANDLE),
DEFINE_FIELD(m_hLaserBeam2, FIELD_EHANDLE),
DEFINE_FIELD(m_hLaserBeam3, FIELD_EHANDLE),
DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
DEFINE_FIELD(m_bMustReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_iFirePhase, FIELD_INTEGER),
DEFINE_FIELD(bFirstPickUp, FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponHornetGun::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, false },

	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },

	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SHOTGUN, true },
	//{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,		true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_WALK, ACT_WALK_RIFLE, false },
};

IMPLEMENT_ACTTABLE(CWeaponHornetGun);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponHornetGun::CWeaponHornetGun(void)
{
	m_bReloadsSingly = true;
	m_bInZoom = false;
	m_bMustReload = false;
	m_iFirePhase = 0;
	bFirstPickUp = true;
}


CWeaponHornetGun::~CWeaponHornetGun()
{

	if (m_hLaserBeam1)
	{
		UTIL_Remove(m_hLaserBeam1);
	}
	if (m_hLaserBeam2)
	{
		UTIL_Remove(m_hLaserBeam2);
	}
	if (m_hLaserBeam3)
	{
		UTIL_Remove(m_hLaserBeam3);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHornetGun::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("ShockRifle_projectile");
#endif
	PrecacheModel(SHOCK_BEAM_SPRITE);
	PrecacheScriptSound("Weapon_Shock.Draw");
	PrecacheScriptSound("Weapon_Shock.Impact");
	PrecacheScriptSound("Weapon_Crossbow.BoltHitBody");
	PrecacheScriptSound("Weapon_Crossbow.BoltHitWorld");
	PrecacheScriptSound("Weapon_Crossbow.BoltSkewer");
	PrecacheParticleSystem("Shock_rifle_main_ball");
	PrecacheParticleSystem("Shock_rifle_main_glow");
	UTIL_PrecacheOther("grenade_spit");
	PrecacheModel(GLOW_SPRITE);

	BaseClass::Precache();
}

void CWeaponHornetGun::Spawn(void)
{
	BaseClass::Spawn();

	m_takedamage = DAMAGE_YES;
	m_iHealth = 15;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHornetGun::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		return;
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	FireBolt();

	AddViewKick();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHornetGun::SecondaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		return;
	}

	//if (GetWpnData().m_bUsePrimaryAmmoAsSecondary ? GetPrimaryAmmoCount() <= 0 : GetSecondaryAmmoCount() <= 0)
	/*if (GetWpnData().m_bUsePrimaryAmmoAsSecondary ? pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 : pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = 0.15;

		return;
	}*/

	WeaponSound(SINGLE);

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	//CBaseEntity *pHornet;
	Vector vecSrc;

	Vector	vForward, vRight, vUp;
	QAngle	vecAngles;

	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	VectorAngles(vForward, vecAngles);

	Vector vThrowPos;
	vThrowPos = GetClientTracerVector();
	/*vThrowPos.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	vThrowPos.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	vThrowPos.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/

	trace_t	tr;
	UTIL_TraceLine(pPlayer->EyePosition(), vThrowPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	vecSrc = tr.endpos + vForward * -15 + vRight * 8 + vUp * -12;

	m_iFirePhase++;
	switch (m_iFirePhase)
	{
	case 1:
		vecSrc = vecSrc + vUp * 8;
		break;
	case 2:
		vecSrc = vecSrc + vUp * 8;
		vecSrc = vecSrc + vRight * 8;
		break;
	case 3:
		vecSrc = vecSrc + vRight * 8;
		break;
	case 4:
		vecSrc = vecSrc + vUp * -8;
		vecSrc = vecSrc + vRight * 8;
		break;
	case 5:
		vecSrc = vecSrc + vUp * -8;
		break;
	case 6:
		vecSrc = vecSrc + vUp * -8;
		vecSrc = vecSrc + vRight * -8;
		break;
	case 7:
		vecSrc = vecSrc + vRight * -8;
		break;
	case 8:
		vecSrc = vecSrc + vUp * 8;
		vecSrc = vecSrc + vRight * -8;
		m_iFirePhase = 0;
		break;
	}

	CBaseEntity *pHornet = CBaseEntity::Create("hornet", vecSrc, vecAngles, pPlayer);
	pHornet->SetAbsVelocity(vForward * 1200);
	pHornet->SetThink(&CNPC_Hornet::StartDart);

	/*CHornet *pBee = CHornet::Create(vecSrc, vecAngles, pPlayer);

	if (pBee)
	{
	pBee->m_hOwner = this;
	pBee->SetGracePeriod(2);
	pBee->m_bDisaleSearch = true;
	pBee->FindEnemy();
	pBee->m_vInitialDirection = vForward;
	pBee->SetAbsVelocity(vForward * HORNET_SPEED);
	//pBee->SetEnemy(GetEnemy());
	}*/

	RemoveAmmo(GetPrimaryAmmoType(), 1); //pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.7f;						// BJ: 0.7 delay from second to primary, no extra calculations
	m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate() * 0.4f;	// BJ: Fixin timings, 0.4 is perfect like hl1
	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

	AddViewKick();

	SecondaryAttackEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHornetGun::Reload(void)
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHornetGun::FireBolt(void)
{
	/*if (GetPrimaryAmmoCount() <= 0)
	{
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = 0.15;

		return;
	}*/

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	//pOwner->AbortReload();	//test

#ifndef CLIENT_DLL
	Vector vecAiming = pOwner->GetAutoaimVector(0);

	Vector vecMuzzlePos2; Vector vecEndPos2;
	QAngle angDir;
	/*Vector Muzzle = pOwner->Weapon_ShootPosition(1);

	Muzzle.x += oc_weapon_hornetgun_projectile_x.GetFloat();
	Muzzle.y += oc_weapon_hornetgun_projectile_y.GetFloat();
	Muzzle.z += oc_weapon_hornetgun_projectile_z.GetFloat();*/

	Vector vThrowPos;
	vThrowPos = GetClientTracerVector();
	/*vThrowPos.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	vThrowPos.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	vThrowPos.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/

	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), vThrowPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	//CShockRifleProjectile *pShockRifleProjectile = CShockRifleProjectile::BoltCreate(vThrowPos, angAiming, 0, pOwner); // BriJee OVR : Better start angles

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	CBaseEntity *pHornet = CBaseEntity::Create("hornet", tr.endpos, angAiming, pOwner);
	//DevMsg("WeaponVel: %.2f %.2f %.2f \n", (vForward * 300).x, (vForward * 300).y, (vForward * 300).z);
	pHornet->SetAbsVelocity(vForward * 300);

	/*CHornet *pBee = CHornet::Create(vThrowPos, angAiming, pOwner);

	if (pBee)
	{
	pBee->m_hOwner = this;
	pBee->SetGracePeriod(2);
	pBee->m_bDisaleSearch = false;
	pBee->FindEnemy();
	pBee->m_vInitialDirection = vForward;
	pBee->SetAbsVelocity(vForward * HORNET_SPEED);
	//pBee->SetEnemy(GetEnemy());
	}*/


#endif

	RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);

	WeaponSound(SINGLE);
	WeaponSound(SPECIAL2);

	SendWeaponAnim(GetPrimaryAttackActivity());
}
