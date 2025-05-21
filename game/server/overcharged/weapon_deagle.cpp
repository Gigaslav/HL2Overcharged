//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Deagle - Head explode gun
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "gamestats.h"
#include "NPCEvent.h"

#include "weapon_deagle.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_pLaserDotThink2 = "LaserThinkContext";
extern ConVar   weapon_laser_pointer;




// a list of laser dots to search quickly
CEntityClassList<CLaserDot2> g_LaserDotList;
CLaserDot2 *CEntityClassList<CLaserDot2>::m_pClassList = NULL;
CLaserDot2 *GetLaserDotList()
{
	return g_LaserDotList.m_pClassList;
}

#define	DEAGLE_BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	DEAGLE_LASER_SPRITE	"sprites/redglow1.vmt"


#define	PISTOL_FASTEST_REFIRE_TIME		0.2f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out


IMPLEMENT_SERVERCLASS_ST( CWeaponDeagle, DT_WeaponDeagle )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_deagle, CWeaponDeagle );
PRECACHE_WEAPON_REGISTER( weapon_deagle );

BEGIN_DATADESC( CWeaponDeagle )

	DEFINE_FIELD( m_bInitialStateUpdate,FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bGuiding,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecNPCLaserDot2,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_hLaserDot,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hLaserMuzzleSprite, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hLaserBeam,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHideGuiding,		FIELD_BOOLEAN ),
	DEFINE_FIELD(m_flAccuracyPenalty, FIELD_FLOAT), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD(m_nNumShotsFired, FIELD_INTEGER),

END_DATADESC()

acttable_t	CWeaponDeagle::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PISTOL,                    false },		// L1ght 15 : MP animstate
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PISTOL,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
    { ACT_HL2MP_GESTURE_RELOAD,          ACT_GESTURE_RELOAD_SMG1,					false },	//fix
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PISTOL,                    false },
    //{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_PISTOL,                false },		// END

	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			true },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	true },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE/*_PISTOL*/,				false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_STIMULATED,			false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_STEALTH,				ACT_IDLE_STEALTH_PISTOL,		false },

	{ ACT_WALK_RELAXED,				ACT_WALK,						false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_STIMULATED,			false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_STEALTH,				ACT_WALK_STEALTH_PISTOL,		false },

	{ ACT_RUN_RELAXED,				ACT_RUN,						false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_STIMULATED,				false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_STEALTH,				ACT_RUN_STEALTH_PISTOL,			false },

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_PISTOL,				false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_ANGRY_PISTOL,			false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_AIM_STEALTH,			ACT_IDLE_STEALTH_PISTOL,		false },

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK,						false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_PISTOL,			false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_AIM_STEALTH,			ACT_WALK_AIM_STEALTH_PISTOL,	false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN,						false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_PISTOL,				false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_AIM_STEALTH,			ACT_RUN_AIM_STEALTH_PISTOL,		false },//always aims
	//End readiness activities

	// Crouch activities
	{ ACT_CROUCHIDLE_STIMULATED,	ACT_CROUCHIDLE_STIMULATED,		false },
	{ ACT_CROUCHIDLE_AIM_STIMULATED,ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims
	{ ACT_CROUCHIDLE_AGITATED,		ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims

	// Readiness translations
	{ ACT_READINESS_RELAXED_TO_STIMULATED, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED, false },
	{ ACT_READINESS_RELAXED_TO_STIMULATED_WALK, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK, false },
	{ ACT_READINESS_AGITATED_TO_STIMULATED, ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED, false },
	{ ACT_READINESS_STIMULATED_TO_RELAXED, ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED, false },
};

IMPLEMENT_ACTTABLE(CWeaponDeagle);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponDeagle::CWeaponDeagle()
{
	m_flAccuracyPenalty = 0.0f;
	m_bReloadsSingly = false;
	m_bInitialStateUpdate= false;
	m_bHideGuiding = false;
	m_bGuiding = false;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponDeagle::~CWeaponDeagle()
{
	if ( m_hLaserDot != NULL )
	{
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}

	if ( m_hLaserMuzzleSprite )
	{
		UTIL_Remove( m_hLaserMuzzleSprite );
	}

	if ( m_hLaserBeam )
	{
		UTIL_Remove( m_hLaserBeam );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::Precache( void )
{

	// Laser dot...
	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( DEAGLE_LASER_SPRITE );
	PrecacheModel( DEAGLE_BEAM_SPRITE );
	PrecacheScriptSound("Weapon_DEagle.Draw");//OverCharged
	PrecacheScriptSound("Weapon_DEagle.LaserOn");//OverCharged
	PrecacheScriptSound("Weapon_DEagle.LaserOff");//OverCharged
	BaseClass::Precache();
}
/*Activity CWeaponDeagle::GetDrawActivity(void)
{
	//EmitSound("Weapon_DEagle.Draw");
	return ACT_VM_DRAW;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::Activate( void )
{
	BaseClass::Activate();
	
	// Restore the laser pointer after transition
	if ( m_bGuiding )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		
		if ( pOwner == NULL )
			return;

		if ( pOwner->GetActiveWeapon() == this )
		{
			StartGuiding();
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponDeagle::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_RELOAD:
		{
			ShellOut(pEvent);
		}
		break;
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}
bool CWeaponDeagle::Reload(void)
{
	SuppressGuiding();

	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ((m_iClip1 == 0) ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD));

	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (m_iClip1 > 1)
		SendWeaponAnim(GetPrimaryAttackActivity());
	if (m_iClip1 <= 1)
		SendWeaponAnim(GetSecondaryAttackActivity());

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	BaseClass::PrimaryAttack();

	//DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);

	//DispatchParticleEffect("weapon_dust_stream", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);

	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pOwner, true, GetClassname());
}

void CWeaponDeagle::SecondaryAttack( void )
{
	if (m_bInReload)
		return;

	
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());	// BJ: New laser method override, else use old method

	if (pPlayer == NULL)
		return;

	if (GetWpnData().enableLaser)
	{

		if (!pPlayer->GetActiveWeapon()->EnableLaser)
		{
			pPlayer->GetActiveWeapon()->EnableLaser = true;
		}
		else if (EnableLaser)
		{
			pPlayer->GetActiveWeapon()->EnableLaser = false;
		}

		DevMsg("Deagle Newlaser method \n");

	}
	else
	{
		DevMsg("Deagle Oldlaser method \n");

		if (IsGuiding())
		{
			StopGuiding();
		}
		else
		{
			StartGuiding();
		}
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void CWeaponDeagle::SuppressGuiding( bool state )
{
	m_bHideGuiding = state;

	if ( m_hLaserDot == NULL )
	{
		//StartGuiding();

		//STILL!?
		if ( m_hLaserDot == NULL )
			 return;
	}

	if ( state )
	{
		m_hLaserDot->TurnOff();
	}
	else
	{
		m_hLaserDot->TurnOn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
	
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	// Supress our guiding effects if we're lowered
	if ( GetIdealActivity() == ACT_VM_IDLE_LOWERED || GetIdealActivity() == ACT_VM_RELOAD )
	{
		SuppressGuiding();
	}
	else
	{
		SuppressGuiding( false );
	}

	if (IsNearWall() || GetOwnerIsRunning())
	{
		SuppressGuiding();
	}


	UpdateLaserPosition();
	UpdateLaserEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CWeaponDeagle::GetLaserPosition( void )
{
	CreateLaserPointer();

	if ( m_hLaserDot != NULL )
		return m_hLaserDot->GetAbsOrigin();

	//FIXME: The laser dot sprite is not active, this code should not be allowed!
	assert(0);
	return vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true if the rocket is being guided, false if it's dumb
//-----------------------------------------------------------------------------
bool CWeaponDeagle::IsGuiding( void )
{
	return m_bGuiding;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDeagle::Deploy( void )
{
	m_bInitialStateUpdate = true;

/*	if (cvar->FindVar("aa_PlrOnLadder")->GetInt() == 1)		// L1ght 15: Checking for ladder, bugfix
	{
		SuppressGuiding();
	}*/
	/*else
	{
		SuppressGuiding( false );
	}*/

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponDeagle::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if (m_bGuiding)
		StopGuiding();

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Turn on the guiding laser
//-----------------------------------------------------------------------------
void CWeaponDeagle::StartGuiding( void )
{
	// Don't start back up if we're overriding this
	if ( m_bHideGuiding )
		return;

	m_bGuiding = true;

	WeaponSound(SPECIAL1);

	//CreateLaserPointer();
	StartLaserEffects();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	pPlayer->AddEffects(EF_LASER);
}

//-----------------------------------------------------------------------------
// Purpose: Turn off the guiding laser
//-----------------------------------------------------------------------------
void CWeaponDeagle::StopGuiding( void )
{
	m_bGuiding = false;

	WeaponSound( SPECIAL2 );

	StopLaserEffects();

	// Kill the dot completely
	if ( m_hLaserDot != NULL )
	{
		m_hLaserDot->TurnOff();
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	pPlayer->RemoveEffects(EF_LASER);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::Drop( const Vector &vecVelocity )
{
	StopGuiding();

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDeagle::UpdateLaserPosition( Vector vecMuzzlePos, Vector vecEndPos )
{
	if ( vecMuzzlePos == vec3_origin || vecEndPos == vec3_origin )
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
			return;

		vecMuzzlePos = pPlayer->Weapon_ShootPosition();
		Vector	forward;

		if( g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE )
		{
			forward = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	
		}
		else
		{
			pPlayer->EyeVectors( &forward );
		}

		vecEndPos = vecMuzzlePos + ( forward * MAX_TRACE_LENGTH );
	}

	//Move the laser dot, if active
	trace_t	tr;
	
	// Trace out for the endpoint
	UTIL_TraceLine( vecMuzzlePos, vecEndPos, (MASK_SHOT & ~CONTENTS_WINDOW), this, COLLISION_GROUP_NONE, &tr );

	// Move the laser sprite
	if ( m_hLaserDot != NULL )
	{
		Vector	laserPos = tr.endpos;
		m_hLaserDot->SetLaserPosition( laserPos, tr.plane.normal );

		if ( tr.DidHitNonWorldEntity() )
		{
			CBaseEntity *pHit = tr.m_pEnt;

			if ( ( pHit != NULL ) && ( pHit->m_takedamage ) )
			{
				m_hLaserDot->SetTargetEntity( pHit );
			}
			else
			{
				m_hLaserDot->SetTargetEntity( NULL );
			}
		}
		else
		{
			m_hLaserDot->SetTargetEntity( NULL );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponDeagle::CreateLaserPointer( void )
{
	if ( m_hLaserDot != NULL )
		return;

	m_hLaserDot = CLaserDot2::Create( GetAbsOrigin(), GetOwnerEntity() );
	m_hLaserDot->TurnOff();

	UpdateLaserPosition();
}

//-----------------------------------------------------------------------------
// Purpose: Start the effects on the viewmodel of the Deagle
//-----------------------------------------------------------------------------
void CWeaponDeagle::StartLaserEffects( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

	CBaseViewModel *pBeamEnt = static_cast<CBaseViewModel *>(pOwner->GetViewModel());

	if ( m_hLaserBeam == NULL )
	{
		m_hLaserBeam = CBeam::BeamCreate( DEAGLE_BEAM_SPRITE, 1.0f );
		
		if ( m_hLaserBeam == NULL )
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam->EntsInit( pBeamEnt, pBeamEnt );

		int	startAttachment = LookupAttachment( "laser" );
		int endAttachment	= LookupAttachment( "laser_end" );

		//m_hLaserBeam->SetStartPos()
		m_hLaserBeam->FollowEntity( pBeamEnt );
		m_hLaserBeam->SetStartAttachment( startAttachment );
		m_hLaserBeam->SetEndAttachment( endAttachment );
		m_hLaserBeam->SetNoise( 0 );
		m_hLaserBeam->SetColor( 255, 0, 0 );
		m_hLaserBeam->SetScrollRate( 0 );
		m_hLaserBeam->SetWidth( 0.5f );
		m_hLaserBeam->SetEndWidth( 0.5f );
		m_hLaserBeam->SetBrightness( 128 );
		m_hLaserBeam->SetBeamFlags( SF_BEAM_SHADEIN );
	}
	else
	{
		m_hLaserBeam->SetBrightness( 128 );
	}

	if ( m_hLaserMuzzleSprite == NULL )
	{
		m_hLaserMuzzleSprite = CSprite::SpriteCreate( DEAGLE_LASER_SPRITE, GetAbsOrigin(), false );

		if ( m_hLaserMuzzleSprite == NULL )
		{
			// We were unable to create the sprite
			Assert(0);
			return;
		}

		m_hLaserMuzzleSprite->SetAttachment( pOwner->GetViewModel(), LookupAttachment( "laser" ) );
		m_hLaserMuzzleSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
		m_hLaserMuzzleSprite->SetBrightness( 255, 0.5f );
		m_hLaserMuzzleSprite->SetScale( 0.25f, 0.5f );
		m_hLaserMuzzleSprite->TurnOn();
	}
	else
	{
		m_hLaserMuzzleSprite->TurnOn();
		m_hLaserMuzzleSprite->SetScale( 0.25f, 0.25f );
		m_hLaserMuzzleSprite->SetBrightness( 255 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop the effects on the viewmodel of the Deagle
//-----------------------------------------------------------------------------
void CWeaponDeagle::StopLaserEffects( void )
{
	if ( m_hLaserBeam != NULL )
	{
		m_hLaserBeam->SetBrightness( 0 );
	}
	
	if ( m_hLaserMuzzleSprite != NULL )
	{
		m_hLaserMuzzleSprite->SetScale( 0.01f );
		m_hLaserMuzzleSprite->SetBrightness( 0, 0.5f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Pulse all the effects to make them more... well, laser-like
//-----------------------------------------------------------------------------
void CWeaponDeagle::UpdateLaserEffects( void )
{
	if ( !m_bGuiding )
		return;

	if ( m_hLaserBeam != NULL )
	{
/*		if (cvar->FindVar("aa_cammode")->GetInt() == 1)	// L1ght 15 : No VM laser in TPS
		{
			m_hLaserBeam->SetBrightness( 0 );
		}
		else*/
		m_hLaserBeam->SetBrightness( 128 + random->RandomInt( -8, 8 ) );
	}

	if ( m_hLaserMuzzleSprite != NULL )
	{
		m_hLaserMuzzleSprite->SetScale( 0.1f + random->RandomFloat( -0.025f, 0.025f ) );
	}
}

//-----------------------------------------------------------------------------
// Laser dot control
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserDot2( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot );
void SetLaserDotTarget2( CBaseEntity *pLaserDot, CBaseEntity *pTarget );
void EnableLaserDot2( CBaseEntity *pLaserDot, bool bEnable );

//=============================================================================
// Laser Dot
//=============================================================================

LINK_ENTITY_TO_CLASS( env_laserdot2, CLaserDot2 );

BEGIN_DATADESC( CLaserDot2 )
	DEFINE_FIELD( m_vecSurfaceNormal,	FIELD_VECTOR ),
	DEFINE_FIELD( m_hTargetEnt,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_bVisibleLaserDot,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsOn,				FIELD_BOOLEAN ),

	//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),	// don't save - regenerated by constructor
	DEFINE_THINKFUNC( LaserThink ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserDot2( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	return CLaserDot2::Create( origin, pOwner, bVisibleDot );
}

void SetLaserDotTarget2( CBaseEntity *pLaserDot, CBaseEntity *pTarget )
{
	CLaserDot2 *pDot = assert_cast< CLaserDot2* >(pLaserDot );
	pDot->SetTargetEntity( pTarget );
}

void EnableLaserDot2( CBaseEntity *pLaserDot, bool bEnable )
{
	CLaserDot2 *pDot = assert_cast< CLaserDot2* >(pLaserDot );
	if ( bEnable )
	{
		pDot->TurnOn();
	}
	else
	{
		pDot->TurnOff();
	}
}

CLaserDot2::CLaserDot2( void )
{
	m_hTargetEnt = NULL;
	m_bIsOn = true;
	g_LaserDotList.Insert( this );
}

CLaserDot2::~CLaserDot2( void )
{
	g_LaserDotList.Remove( this );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CLaserDot2
//-----------------------------------------------------------------------------
CLaserDot2 *CLaserDot2::Create( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	CLaserDot2 *pLaserDot = (CLaserDot2 *) CBaseEntity::Create( "env_laserdot2", origin, QAngle(0,0,0) );

	if ( pLaserDot == NULL )
		return NULL;

	pLaserDot->m_bVisibleLaserDot = bVisibleDot;
	pLaserDot->SetMoveType( MOVETYPE_NONE );
	pLaserDot->AddSolidFlags( FSOLID_NOT_SOLID );
	pLaserDot->AddEffects( EF_NOSHADOW );
	UTIL_SetSize( pLaserDot, vec3_origin, vec3_origin );

	//Create the graphic
	pLaserDot->SpriteInit( "sprites/redglow1.vmt", origin );

	pLaserDot->SetName( AllocPooledString("TEST") );

	pLaserDot->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	pLaserDot->SetScale( 0.5f );

	pLaserDot->SetOwnerEntity( pOwner );

	pLaserDot->SetContextThink( &CLaserDot2::LaserThink, gpGlobals->curtime + 0.1f, g_pLaserDotThink2 );
	pLaserDot->SetSimulatedEveryTick( true );

	if ( !bVisibleDot )
	{
		pLaserDot->MakeInvisible();
	}

	return pLaserDot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserDot2::LaserThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f, g_pLaserDotThink2 );

	if ( GetOwnerEntity() == NULL )
		return;

	Vector	viewDir = GetAbsOrigin() - GetOwnerEntity()->GetAbsOrigin();
	float	dist = VectorNormalize( viewDir );

	float	scale = RemapVal( dist, 32, 1024, 0.01f, 0.5f );
	float	scaleOffs = random->RandomFloat( -scale * 0.25f, scale * 0.25f );

	scale = clamp( scale + scaleOffs, 0.1f, 32.0f );

	SetScale( scale );
}

void CLaserDot2::SetLaserPosition( const Vector &origin, const Vector &normal )
{
	SetAbsOrigin( origin );
	m_vecSurfaceNormal = normal;
}

Vector CLaserDot2::GetChasePosition()
{
	return GetAbsOrigin() - m_vecSurfaceNormal * 10;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserDot2::TurnOn( void )
{
	m_bIsOn = true;
	if ( m_bVisibleLaserDot )
	{
		BaseClass::TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserDot2::TurnOff( void )
{
	m_bIsOn = false;
	if ( m_bVisibleLaserDot )
	{
		BaseClass::TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserDot2::MakeInvisible( void )
{
	BaseClass::TurnOff();
}
