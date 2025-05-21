//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Krieg - zoomed rifle
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "AI_Memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "shake.h"
#include "particle_parse.h"
#include "weapon_sg552.h"

#include "tier0/memdbgon.h"



IMPLEMENT_SERVERCLASS_ST( CWeaponSG552, DT_WeaponSG552 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_sg552, CWeaponSG552 );
PRECACHE_WEAPON_REGISTER( weapon_sg552 );

BEGIN_DATADESC( CWeaponSG552 )

	//DEFINE_FIELD( m_nShotsFired, FIELD_INTEGER ),
	//DEFINE_FIELD( m_bZoomed, FIELD_BOOLEAN ),

END_DATADESC()

acttable_t CWeaponSG552::m_acttable[] = 
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

IMPLEMENT_ACTTABLE( CWeaponSG552 );

CWeaponSG552::CWeaponSG552()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;
}
void CWeaponSG552::Precache(void)
{
	PrecacheScriptSound("Weapon_sg552.Draw");//OverCharged

	BaseClass::Precache();
}

/*void CWeaponSG552::ItemPostFrame( void )
{


	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( !pOwner )
		return;

	CBaseCombatWeapon *pWeapon = pOwner->GetActiveWeapon();

	if (pWeapon == NULL || (cvar->FindVar("oc_state_IRsight_on")->GetInt())) //pWeapon->IsIronsighted() )		// L1ght 15 : Return values on iron-sight
	{
		m_bZoomed = false;
	}



	if (m_bInReload)
	{
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_sg552")->SetValue(0);
	}



	BaseClass::ItemPostFrame();
}*/

void CWeaponSG552::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent -> event )
	{ 
		/*case EVENT_WEAPON_AR2:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator -> Weapon_ShootPosition( );

			CAI_BaseNPC *npc = pOperator -> MyNPCPointer();
			ASSERT( npc != NULL );
				
			vecShootDir = npc -> GetActualShootTrajectory( vecShootOrigin );

			WeaponSound( SINGLE_NPC );
			pOperator -> FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator -> DoMuzzleFlash();
		}*/
		case EVENT_WEAPON_AR2://EVENT_WEAPON_SMG1:
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
		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSG552::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
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
void CWeaponSG552::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}


/*float CWeaponSG552::GetFireRate( void )
{ 
	if ( m_bZoomed )
	{
		return 0.2f;
	}

	return 0.1f;
}*/

/*bool CWeaponSG552::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (m_bZoomed)
	{
		Zoom();
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer != NULL)
	{

		EmitSound("Weapon_RPG.LaserOff");
		pPlayer->SetFOV(this, 0, 0.1f);
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
	}
	cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
	cvar->FindVar("oc_state_InSecondFire_sg552")->SetValue(0);

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponSG552::Reload( void )
{
	//DisableIronsights();
	if ( m_bZoomed )
	{
		Zoom();
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());


	if (pPlayer->SetFOV(this, 20, 0.2f))
	{
		m_bZoomed = false;
		// Send a message to hide the scope
		pPlayer->SetFOV(this, 0, 0.1f);
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_sg552")->SetValue(0);
	}

	//return fRet;

	return BaseClass::Reload();
}

void CWeaponSG552::Drop( const Vector &vecVelocity )
{
	if ( m_bZoomed )
	{
		Zoom();
	}

	BaseClass::Drop( vecVelocity );
}*/
