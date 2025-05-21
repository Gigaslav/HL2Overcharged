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
//#include "grenade_ar2.h"
#include "AI_Memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "weapon_machinegun.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_SERVERCLASS_ST(CWeaponMachineGun, DT_WeaponMachineGun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_MachineGun, CWeaponMachineGun );
PRECACHE_WEAPON_REGISTER(weapon_MachineGun);

BEGIN_DATADESC( CWeaponMachineGun )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD(BG, FIELD_FLOAT),

END_DATADESC()

acttable_t	CWeaponMachineGun::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponMachineGun);

//=========================================================
CWeaponMachineGun::CWeaponMachineGun( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMachineGun::Precache(void)
{
	PrecacheScriptSound("Weapon_M249.EndFire");//OverCharged
	PrecacheScriptSound("Weapon_M249.Draw");//OverCharged
	BaseClass::Precache();
}

/*Activity CWeaponMachineGun::GetDrawActivity(void)
{
	//EmitSound("Weapon_M249.Draw");

	return ACT_VM_DRAW;
}*/


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMachineGun::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	if (!m_bInReload)
	{
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel)
		{

			switch (m_iClip1)
			{
			case 1:
				pViewModel->SetBodygroup(1, 1);//0 - рендерится 1 -не рендерится
				break;
			case 2:
				pViewModel->SetBodygroup(2, 1);
				break;
			case 3:
				pViewModel->SetBodygroup(3, 1);
				break;
			case 4:
				pViewModel->SetBodygroup(4, 1);
				break;
			case 5:
				pViewModel->SetBodygroup(5, 1);
				break;
			case 6:
				pViewModel->SetBodygroup(6, 1);
				break;
			case 7:
				pViewModel->SetBodygroup(7, 1);
				break;
			case 8:
				pViewModel->SetBodygroup(8, 1);
				break;
			case 9:
				pViewModel->SetBodygroup(9, 1);
				break;
			case 10:
				pViewModel->SetBodygroup(10, 1);
				break;
			case 11:
				pViewModel->SetBodygroup(11, 1);
				break;
			case 12:
				pViewModel->SetBodygroup(12, 1);
				break;
			case 13:
				pViewModel->SetBodygroup(13, 1);
				break;
			case 14:
				pViewModel->SetBodygroup(14, 1);
				break;
			case 15:
				pViewModel->SetBodygroup(15, 1);
				break;
			case 16:
				pViewModel->SetBodygroup(16, 1);
				break;
			default:
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMachineGun::Equip( CBaseCombatCharacter *pOwner )
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
void CWeaponMachineGun::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this),
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMachineGun::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
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
void CWeaponMachineGun::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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
	case EVENT_WEAPON_RELOAD_FILL_CLIP:
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel == NULL)
			return;		//^test

		if (m_iClip1 <= 16)
		{
			for (BG = 1; BG <= 16; BG++)
			{
				pViewModel->SetBodygroup(BG, 0);

			}
		}
	}
	break;
	case EVENT_WEAPON_THROW:
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;
		CBaseViewModel *pViewModel = pPlayer->GetViewModel();
		if (pViewModel == NULL)
			return;		//^test

		if (m_iClip1 == GetMaxClip1())
		{

			for (BG = 1; BG <= 16; BG++)
			{
				pViewModel->SetBodygroup(BG, 0);

			}
		}
	}

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponMachineGun::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	return COND_NONE;

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if ( npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if ( !( pEnemy->GetFlags() & FL_ONGROUND ) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z) )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}
	
	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if (random->RandomInt(0,1))
	{
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter();
	}
	else
	{
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;
	}
	// vecTarget = m_vecEnemyLKP + (pEnemy->BodyTarget( GetLocalOrigin() ) - pEnemy->GetLocalOrigin());
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;


	if ( ( vecTarget - npcOwner->GetLocalOrigin() ).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return (COND_NONE);
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
	{
		//Check to see if the default relationship is hatred, and if so intensify that
		if ( npcOwner->IRelationType( pTarget ) == D_LI )
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return (COND_WEAPON_BLOCKED_BY_FRIEND);
		}
	}

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecToss = VecCheckThrow( this, npcOwner->GetLocalOrigin() + Vector(0,0,60), vecTarget, 600.0, 0.5 );
	if ( vecToss != vec3_origin )
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
		return COND_CAN_RANGE_ATTACK2;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponMachineGun::GetProficiencyValues()
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