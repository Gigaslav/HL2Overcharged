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
#include "AI_Memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "fire.h"
//#include "soundenvelope.h"
#include "particle_parse.h"

#include "weapon_flamethrower.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar oc_weapon_flamethrower_dmg( "oc_weapon_flamethrower_dmg", "1", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Damage per second. Integer value. Def 1." );
ConVar oc_weapon_flamethrower_dmg_radius( "oc_weapon_flamethrower_dmg_radius", "140", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Damage radius. Def 140." );
ConVar oc_weapon_flamethrower_particle_name( "oc_weapon_flamethrower_particle_name", "flamethrower", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Fire particle name. Def flamethrower." );
ConVar oc_weapon_flamethrower_burn_lifetime("oc_weapon_flamethrower_burn_lifetime", "10", FCVAR_ARCHIVE);

IMPLEMENT_SERVERCLASS_ST(CWeaponflamethrower, DT_Weaponflamethrower)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flamethrower, CWeaponflamethrower );
PRECACHE_WEAPON_REGISTER(weapon_flamethrower);

BEGIN_DATADESC( CWeaponflamethrower )

	DEFINE_FIELD( bFlameSndLoop,		FIELD_BOOLEAN ),
	DEFINE_FIELD( bRemAmmoReducer, FIELD_BOOLEAN),
	DEFINE_FIELD( bRemAmmoReducer2, FIELD_BOOLEAN),

END_DATADESC()

acttable_t CWeaponflamethrower::m_acttable[] =					// BJ: Flamethrower act's over here
{
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PHYSGUN,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PHYSGUN,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,                    false },

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE(CWeaponflamethrower);

//=========================================================
CWeaponflamethrower::CWeaponflamethrower( )
{
	//m_fMinRange1		= 0;// No minimum range. 
	//m_fMaxRange1		= 1400;

	bFlameSndLoop		= false;
	DoOnce = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponflamethrower::Precache( void )
{
	PrecacheParticleSystem( oc_weapon_flamethrower_particle_name.GetString() ); 
	//PrecacheParticleSystem( "flamethrower");
	//PrecacheParticleSystem( "pyrotaunt_rainbow_bubbles_flame" );

	BaseClass::Precache();
}

void CWeaponflamethrower::StartLoopSound()
{
	if (IsNearWall() || GetOwnerIsRunning())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer) return;

	bool usingClips = pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0;

	if (usingClips && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		DestroyLoopSound();
		return;
	}

	if (!m_bIsFiring)
	{
		DestroyLoopSound();

		if (!m_pLoopSoundController || !m_pLoopSound)
		{
			PrecacheScriptSound(GetWpnData().ShootLoopingSound);
			PrecacheScriptSound(GetWpnData().EndLoopingSound);

			if (GetWpnData().allowLoopSilencerSound)
			{
				PrecacheScriptSound(GetWpnData().ShootLoopingSilencerSound);
				PrecacheScriptSound(GetWpnData().EndLoopingSilencerSound);
			}

			CreateLoopSound(m_bSilenced ? GetWpnData().ShootLoopingSilencerSound : GetWpnData().ShootLoopingSound);
		}

		m_pLoopSoundController->Play(m_pLoopSound, soundInitialVolume, soundInitialPitch);

		m_bIsFiring = true;
	}

	if (m_pLoopSoundController)
	{
		ConVarRef hostTimescale("host_timescale");

		if (pPlayer->GetSlowMoIsEnabled())
		{
			if (m_bSilenced && GetWpnData().allowLoopSilencerSound)
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, (soundInitialPitch / GetWpnData().LoopSilencerSoundSlowMoPitchDivider) * hostTimescale.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
			else
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, (soundInitialPitch / GetWpnData().LoopSoundSlowMoPitchDivider) * hostTimescale.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
		}
		else
		{
			if (m_bSilenced && GetWpnData().allowLoopSilencerSound)
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, soundInitialPitch * hostTimescale.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
			else
				m_pLoopSoundController->SoundChangePitch(m_pLoopSound, soundInitialPitch * hostTimescale.GetFloat()/*cvar->FindVar("host_timescale")->GetFloat()*/, -0.01f);
		}
	}
}

void CWeaponflamethrower::ItemPostFrame( void )
{	
	if (!m_bInReload && (IsNearWall() || GetOwnerIsRunning()))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
	}
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( (( pOwner->m_nButtons & IN_ATTACK ) == false) || (pOwner->GetWaterLevel() == 3 ) ) 
	{
		StopParticleEffects(pOwner->GetViewModel()); // stop that
		//m_flNextPrimaryAttack =  gpGlobals->curtime + 1;
		//m_flNextSecondaryAttack	= gpGlobals->curtime + 1;
		if (bFlameSndLoop)
		{
			WeaponSound( SINGLE, 0.1f );
			bFlameSndLoop = false;

			DoOnce = true;
			StopParticleEffects(pOwner->GetViewModel());
			StopParticleEffects(this);
			StopParticleEffects(pOwner);
		}
	}

	//if (( pOwner->m_nButtons & IN_SPEED ) == true)
	if (pOwner->m_nButtons & IN_SPEED )
	{
		StopParticleEffects(pOwner->GetViewModel()); // stop that
		if (bFlameSndLoop)
		{
			WeaponSound( SINGLE, 0.1f );
			bFlameSndLoop = false;

			DoOnce = true;
			StopParticleEffects(pOwner->GetViewModel());
			StopParticleEffects(this);
			StopParticleEffects(pOwner);
		}
	}

	if ( (UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType) ) )
	{
		StopParticleEffects(pOwner->GetViewModel()); // stop that
		if (bFlameSndLoop)
		{
			WeaponSound( SINGLE, 0.1f );
			bFlameSndLoop = false;

			DoOnce = true;
			StopParticleEffects(pOwner->GetViewModel());
			StopParticleEffects(this);
			StopParticleEffects(pOwner);
		}
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponflamethrower::Reload( void )
{

	//DisableIronsights();
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: Throw some fire...
//-----------------------------------------------------------------------------
void CWeaponflamethrower::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if (DoOnce)
	{
		DispatchParticleEffect("flamethrower", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", false);
		DoOnce = false;
	}
	// set up the vectors and traceline
	trace_t tr, m_trHit;	// Hit Target
	Vector	vecStart, vecStop, vecDir, vecEnd;

	// get the angles
	AngleVectors( pPlayer->EyeAngles( ), &vecDir );

	// get the vectors
	vecStart = pPlayer->Weapon_ShootPosition();
	vecStop = vecStart + vecDir * cvar->FindVar("oc_weapon_flamethrower_range")->GetFloat();

	// do the traceline
	UTIL_TraceLine( vecStart, vecStop, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	if (tr.DidHit())
	{
		FireSystem_StartFire(tr.endpos, random->RandomFloat(10, 70), 0.2f, random->RandomFloat(7, 30), (SF_FIRE_START_ON | SF_FIRE_SMOKELESS), (CBaseEntity*) this, FIRE_NATURAL);//firetrace.endpos
	}

	//DispatchParticleEffect( oc_weapon_flamethrower_particle_name.GetString(), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true);

	CBaseEntity *pObject = NULL;
	pObject = tr.m_pEnt;

	if ( tr.m_pEnt )
	{
		//if ( tr.m_pEnt->GetFlags() != FL_ONFIRE )
		//{
			//Msg("Tr %s, dist %.2f\n", /*tr.m_pEnt->*/ pObject->GetClassname(), tr.endpos );

			if ((tr.endpos.DistTo(vecStart) > 80) || tr.m_pEnt->IsNPC()) // Make sure we shoot if have enough space or aim on npc
			{
			//Msg("Free 2 Shoot \n");

				CBaseEntity *pHurt = pPlayer->CheckTraceHullAttack(oc_weapon_flamethrower_dmg_radius.GetFloat(), Vector(-16, -16, -16), Vector(16, 16, 16), oc_weapon_flamethrower_dmg.GetInt(), (DMG_BULLET | DMG_BURN));

			if (pHurt)
			{
				CBaseAnimating *pAnim;
				pAnim = dynamic_cast<CBaseAnimating*>(pObject);
				if ( pAnim )
				{

					//if (pAnim->IsNPC->IsPlayerAlly())
					//{

					//}
						//return;
					//else
						pAnim->IgniteLifetime(oc_weapon_flamethrower_burn_lifetime.GetFloat());		// BJ: lifetime func ignite more npc without limit

					//pAnim->Ignite( 30.0f );
				}

				pObject->AddFlag(FL_ONFIRE);
			}

			DispatchParticleEffect( oc_weapon_flamethrower_particle_name.GetString(), PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true);
			}
			else
			{
				StopParticleEffects(pPlayer->GetViewModel()); // stop that
				if (bFlameSndLoop)
				{
					WeaponSound( SINGLE, 0.1f );
					bFlameSndLoop = false;
				}
				return;
			}
				//Msg("Too Close \n");
	}
	//NDebugOverlay::Box( tr.endpos, Vector(-40, -40, -40), Vector(40, 40, 40), 0, 0, 255, false, 0.0f );

	//Msg("dist %.2f \n", tr.endpos.DistTo(vecStart));


	if (!bFlameSndLoop)
	{
		WeaponSound( SINGLE );
		bFlameSndLoop = true;
	}

	DevMsg("Flamethrower primaryattack event \n");

	//bool bRemAmmoReducer;		// BJ: Tricky trick to double reduce ammo usage
	//bool bRemAmmoReducer2;
	if (!bRemAmmoReducer)
	{

		if (!bRemAmmoReducer2)
		{
			RemoveAmmo(GetPrimaryAmmoType(), 1);			// BJ: Every 4th callback reduce 1 ammo
			DevMsg("Flamethrower ammocount event \n");
			bRemAmmoReducer2 = true;
		}
		else
		{
			bRemAmmoReducer2 = false;
		}

		bRemAmmoReducer = true;
	}
	else
	{
		bRemAmmoReducer = false;
	}

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation( PLAYER_ATTACK1 );	
}


void CWeaponflamethrower::Drop( const Vector &vecVelocity )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
		StopParticleEffects(pOwner->GetViewModel()); // stop that
		if (bFlameSndLoop)
		{
			WeaponSound( SINGLE, 0.1f );
			bFlameSndLoop = false;
		}
	}

	BaseClass::Drop( vecVelocity );
}

bool CWeaponflamethrower::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return false;

	if ( pOwner->m_nButtons & IN_ATTACK )
		return false;

	//return true;
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponflamethrower::GetProficiencyValues()
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
