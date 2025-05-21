//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_smg1.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "npc_combine.h"
#include "particle_parse.h"
#include "gib.h"
#include "props.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar    sk_plr_dmg_smg1_grenade;	


IMPLEMENT_SERVERCLASS_ST(CWeaponSMG1, DT_WeaponSMG1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_smg1, CWeaponSMG1 );
PRECACHE_WEAPON_REGISTER(weapon_smg1);

BEGIN_DATADESC( CWeaponSMG1 )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),

END_DATADESC()

acttable_t	CWeaponSMG1::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SMG1,                false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

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
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },

};

IMPLEMENT_ACTTABLE(CWeaponSMG1);

//=========================================================
CWeaponSMG1::CWeaponSMG1( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Precache( void )
{
	UTIL_PrecacheOther("grenade_ar2");
	PrecacheScriptSound("Weapon_SMG1.Draw");
	BaseClass::Precache();
}

bool CWeaponSMG1::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool rRet;
	pSwitchingTo = NULL;
	rRet = BaseClass::Holster(pSwitchingTo);

	return BaseClass::Holster(pSwitchingTo);
}
//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSMG1::Equip( CBaseCombatCharacter *pOwner )
{
	if( pOwner->Classify() == CLASS_PLAYER_ALLY )
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this),
				MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
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
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
		break;
	case EVENT_WEAPON_AR2_ALTFIRE:
	{
		FireNPCSecondaryAttack(pOperator, false);
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}


void CWeaponSMG1::FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
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

	Vector vecVelocity = vecAiming * 1000.0f;

	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, vec3_angle, pNPC);
	pGrenade->SetAbsVelocity(vecVelocity);
	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400)); //tumble in air
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	pGrenade->SetThrower(GetOwner());

	pGrenade->SetGravity(0.5); // lower gravity since grenade is aerodynamic and engine doesn't know it.

	pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	SendWeaponAnim(GetSecondaryAttackActivity());

	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();// +0.5f;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();//SequenceDuration();

	/*if ((pPlayer->GetAmmoCount(GetSecondaryAmmoType()) <= 0) || (pPlayer->GetWaterLevel() == 3))
	{
		BaseClass::WeaponSound(EMPTY);
		return;
	}
	else
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		
	}*/

	BaseClass::WeaponSound(WPN_DOUBLE);

	if (m_bInReload)
		m_bInReload = false;

	pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	m_iSecondaryAttacks++;

	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);

	//Create the grenade
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();

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

	Vector	vForward1, vRight1, vUp1;
	pPlayer->EyeVectors(&vForward1, &vRight1, &vUp1);
	QAngle vecAngles;
	VectorAngles(vForward1, vecAngles);

	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vThrowPos/*vecSrc*/, vecAngles, pPlayer);
	pGrenade->SetAbsVelocity(vecThrow);

	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

	QAngle angles2 = pPlayer->GetLocalAngles();

	angles2.x += random->RandomInt(-4, 4);
	angles2.y += random->RandomInt(-4, 4);
	angles2.z = 0;

	pPlayer->SnapEyeAngles(angles2);

	pPlayer->ViewPunch(QAngle(random->RandomInt(-8, -12), random->RandomInt(1, 2), 0));

	SecondaryAttackEffects();
}

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

const WeaponProficiencyInfo_t *CWeaponSMG1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
