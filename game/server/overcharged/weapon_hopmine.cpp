//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "hl2_player.h"
#include "grenade_tripmine.h"
#include "grenade_satchel.h"
#include "entitylist.h"
#include "eventqueue.h"
#include "combine_mine.h"

#include "weapon_hopmine.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CWeapon_HopMine)
	DEFINE_FIELD(m_iAttackState, FIELD_INTEGER),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeapon_HopMine, DT_Weapon_HopMine)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_hopmine, CWeapon_HopMine);
PRECACHE_WEAPON_REGISTER(weapon_hopmine);

acttable_t	CWeapon_HopMine::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SLAM, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SLAM, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SLAM, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SLAM, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SLAM, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SLAM, false },
};

IMPLEMENT_ACTTABLE(CWeapon_HopMine);

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CWeapon_HopMine::CWeapon_HopMine(void)
{
	m_bIsFiring = false;
	m_iAttackState = 0;
	m_flTimeWeaponIdle = 0.f;
}

void CWeapon_HopMine::Spawn()
{
	BaseClass::Spawn();

	Precache();

	FallInit();// get ready to fall down

	// Give 1 piece of default ammo when first picked up
	//m_iClip2 = 1;
}

void CWeapon_HopMine::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("combine_bouncemine");

	PrecacheScriptSound("Weapon_SLAM.TripMineMode");
	PrecacheScriptSound("Weapon_SLAM.SatchelDetonate");
	PrecacheScriptSound("Weapon_SLAM.SatchelThrow");
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_HopMine::PrimaryAttack(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (pPlayer == NULL)
		return;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		return;
	}

	m_bIsFiring = true;

	m_iAttackState = 1;

	SendWeaponAnim(GetWpnData().animData[0].ThrowPullUp);

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextSecondaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if (!HasPrimaryAmmo())
	{
		pPlayer->SwitchToNextBestWeapon(this);
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack switches between satchel charge and tripmine mode
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_HopMine::SecondaryAttack(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (pPlayer == NULL)
		return;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		return;
	}

	m_bIsFiring = true;

	m_iAttackState = 2;

	SendWeaponAnim(GetWpnData().animData[0].ThrowPullDown);

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextSecondaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if (!HasPrimaryAmmo())
	{
		pPlayer->SwitchToNextBestWeapon(this);
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_HopMine::SatchelThrow(bool m_bPrimary)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	Vector vecSrc = pPlayer->WorldSpaceCenter();
	Vector vecFacing = pPlayer->BodyDirection3D();
	vecSrc = vecSrc + vecFacing * 18.0;
	// BUGBUG: is this because vecSrc is not from Weapon_ShootPosition()???
	vecSrc.z += 34.0f;

	Vector vecThrow;
	GetOwner()->GetVelocity(&vecThrow, NULL);
	vecThrow += vecFacing * (m_bPrimary ? 2500 : 950);

	// Player may have turned to face a wall during the throw anim in which case
	// we don't want to throw the SLAM into the wall

	vecThrow = vecFacing;
	vecSrc = pPlayer->WorldSpaceCenter() + vecFacing * 5.0;

	CBounceBomb *pMine = (CBounceBomb*)Create("combine_bouncemine", vecSrc, vec3_angle, GetOwner());

	pMine->m_nSkin = this->m_nSkin;

	if (pMine)
	{
		pMine->KeyValue("StartDisarmed", "0");
		DispatchSpawn(pMine);
		pMine->Activate();
		pMine->OnPhysGunDrop(pPlayer, DROPPED_BY_CANNON);

		IPhysicsObject *pPhysicsObject = pMine->VPhysicsGetObject();

		if (pPhysicsObject != NULL)
		{
			Vector fwd;

			GetVectors(&fwd, NULL, NULL);

			pPhysicsObject->Wake();
			pPhysicsObject->ApplyForceCenter(fwd * (m_bPrimary ? 3500 : 950));
			pPhysicsObject->ApplyTorqueCenter(AngularImpulse(random->RandomFloat(15, 40), random->RandomFloat(15, 40), random->RandomFloat(30, 60)));
		}

		/*Vector eyes = pPlayer->EyePosition();

		trace_t tr;
		UTIL_TraceHull(eyes, pMine->GetAbsOrigin(), Vector(-3,-3,-3), Vector(3,3,3), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt && tr.m_pEnt == pMine)
		UTIL_BloodDecalTrace(&tr, 1);*/
	}

#define RETHROW_DELAY	0.5

	m_flNextPrimaryAttack = gpGlobals->curtime + RETHROW_DELAY;
	m_flNextSecondaryAttack = gpGlobals->curtime + RETHROW_DELAY;

	// Play throw sound
	EmitSound("Weapon_SLAM.SatchelThrow");

	DecrementAmmo(pPlayer);

	PrimaryAttackPostUpdate();
}

//-----------------------------------------------------------------------------
// Purpose: Override so SLAM to so secondary attack when no secondary ammo
//			but satchel is in the world
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_HopMine::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_bIsFiring)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner)
		{
			switch (m_iAttackState)
			{
			case 1:
				if (!(pOwner->m_nButtons & IN_ATTACK))
				{
					//SendWeaponAnim(ACT_VM_THROW);
					SendWeaponAnim(GetWpnData().animData[0].ThrowPrimary);
					m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();
					m_bIsFiring = false;
				}
				break;

			case 2:
				if (!(pOwner->m_nButtons & IN_ATTACK2))
				{
					//See if we're ducking
					if (pOwner->m_nButtons & IN_DUCK)
					{
						SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);
						m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();
						m_bIsFiring = false;
					}
					else
					{
						SendWeaponAnim(GetWpnData().animData[0].ThrowSecondary);	
						m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();
						m_bIsFiring = false;
					}
				}
				break;

			default:
				break;
			}
		}
	}

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		m_iAttackState = 0;

	if ((m_iAttackState == 1 || m_iAttackState == 2) && m_flTimeWeaponIdle < gpGlobals->curtime)
		m_iAttackState = 3;

	if (m_iAttackState == 3)
	{
		SendWeaponAnim(GetDrawActivity());

		RandomizeSkin(2);

		m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();
		m_iAttackState = 4;
	}

	if (m_flTimeWeaponIdle < gpGlobals->curtime)
		m_iAttackState = 0;

	if (m_iAttackState == 0)
		BaseClass::ItemPostFrame();

	if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime)
	{
		m_bFireOnEmpty = false;
		// weapon isn't useable, switch.
		if (((GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false) && (g_pGameRules->SwitchToNextBestWeapon(pOwner, this)))
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeapon_HopMine::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SEQUENCE_FINISHED:
		break;

	case EVENT_WEAPON_THROW:
		SatchelThrow(true);
		break;

	case EVENT_WEAPON_THROW2:
		SatchelThrow(false);
		break;

	case EVENT_WEAPON_THROW3:
		SatchelThrow(false);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeapon_HopMine::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
	RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

//-----------------------------------------------------------------------------
// Purpose: Switch to next best weapon
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_HopMine::Weapon_Switch(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return;

	pOwner->SwitchToNextBestWeapon(pOwner->GetActiveWeapon());

	// If not armed and have no ammo
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		pOwner->ClearActiveWeapon();
	}

}

bool CWeapon_HopMine::Deploy(void)
{
	RandomizeSkin(2);
	
	return BaseClass::Deploy();//DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix());
}
