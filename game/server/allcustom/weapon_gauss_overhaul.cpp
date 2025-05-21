//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Gauss overhaul experimental version
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"

//#include "Sprite.h"
#include "beam_shared.h"
#include "ammodef.h"

#include "te_effect_dispatch.h"
#include "te_particlesystem.h"
#include "effect_dispatch_data.h"

#define GAUSS_BEAM_SPRITE_LITE		"effects/gauss/beam.vmt"
#define GAUSS_BEAM_SPRITE_HIGH		"sprites/laserbeam.vmt"

//#include "grenade_ar2.h"
//#include "prop_combine_ball.h"

//#define	BEAMWEAPON_BEAM_SPRITE "sprites/crystal_beam1.vmt"
//#define	BEAMWEAPON_SMALLBEAM_SPRITE "sprites/hydraspinalcord.vmt"
//#define	BEAMWEAPON_KILLERBEAM_SPRITE "sprites/physbeam.vmt"
//#define	BEAMWEAPON_KILLERSMALLBEAM_SPRITE "sprites/laser.vmt"

//#define	BEAMWEAPON_BEAM_ATTACHMENT "muzzle"
//#define	BEAMWEAPON_TARMBEAM_ATTACHMENT "charge" //"tarmtip"
//#define	BEAMWEAPON_RARMBEAM_ATTACHMENT "charge" //"rarmtip"
//#define	BEAMWEAPON_LARMBEAM_ATTACHMENT "charge" //"larmtip"

//====================================================================== paarticles used aliencontroller, magnusson_burner, struderbuster

#define BEAMWEAPON_SAMMO_COSUM .15;

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_gauss_ovh_primary("sk_plr_dmg_gauss_ovh_primary", "0");			//6
ConVar sk_plr_dmg_gauss_ovh_secondary1("sk_plr_dmg_gauss_ovh_secondary1", "0");		//20
ConVar sk_plr_dmg_gauss_ovh_secondary2("sk_plr_dmg_gauss_ovh_secondary2", "0");		//80
ConVar sk_plr_dmg_gauss_ovh_secondary3("sk_plr_dmg_gauss_ovh_secondary3", "0");		//220
ConVar sk_plr_dmg_gauss_ovh_overcharge("sk_plr_dmg_gauss_ovh_overcharge", "0");		//100

class CWeaponTaussOver : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponTaussOver, CBaseHLCombatWeapon );

	CWeaponTaussOver(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );

	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	AttackCharge( int ChargeType );

	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	bool	CanHolster( void );

	float	GetFireRate( void ) { return 0.075f; }	// 13.3hz

	//void	DrawBeam(const Vector &startPos, const Vector &endPos, char* t_sprite, char* t_Attachment, float width, float attackduration);
	//void	DoImpactEffect( trace_t &tr, int nDamageType);

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = Vector(0,0,0);
		return cone;
	}

	virtual bool Reload( void );

	DECLARE_ACTTABLE();

protected:
	virtual void	ShootBeam_lite(void);
	virtual void	ShootBeam_high(void);

	virtual void	DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle = false);		//h1
	void			DrawBeam_high(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle = false);	//h2
	virtual bool	ShouldDrawWaterImpacts(const trace_t &shot_trace);

private:
	float	m_flSpinAnimTime;
	//bool	m_bSecAttackAmmodelay1;
	//bool	m_bSecAttackAmmodelay2;
	int		m_nSecAttackAmmoDelayCount;
	int		m_nChargeType;
	bool	m_bHasCharge;
	bool	m_bStopCharge;
	int		m_nBulletType;
	int		m_nNumShotsFired;
	float	m_fRandomPushTime;
	float	m_flNextChargeTime;
	//float	m_nSecondaryAmmoTime; // Ammo is removed 1 by every beamweapon_secondary_consumption (.3?) sec

	int		m_bWeaponCurState;
	float	m_nNextAnimationTime;

	// beamgrow
	// to gradually widen the beam (add to dataesc)
	//float m_fBeamWidth;
	//float m_fBeamCounter;

	EHANDLE			m_hViewModel;


	// sprites used
	//int		m_nGlowSpriteIndex;

};


IMPLEMENT_SERVERCLASS_ST(CWeaponTaussOver, DT_WeaponTaussOver)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_gauss_overhaul, CWeaponTaussOver);
PRECACHE_WEAPON_REGISTER( weapon_gauss_overhaul );

BEGIN_DATADESC(CWeaponTaussOver)
	
	DEFINE_FIELD( m_flSpinAnimTime,			FIELD_TIME),
	DEFINE_FIELD( m_nChargeType,			FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextChargeTime,		FIELD_TIME ),
	//DEFINE_FIELD( m_nSecondaryAmmoTime,		FIELD_TIME ),
	DEFINE_FIELD( m_bHasCharge,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),
	DEFINE_FIELD( m_nBulletType,			FIELD_INTEGER ),
	//DEFINE_FIELD( m_nGlowSpriteIndex,		FIELD_INTEGER ),
	DEFINE_FIELD( m_bWeaponCurState,		FIELD_INTEGER ),
	DEFINE_FIELD( m_nNextAnimationTime,		FIELD_TIME ),
	DEFINE_FIELD( m_fRandomPushTime,		FIELD_TIME ),
	//DEFINE_FIELD( m_fBeamWidth,			    FIELD_FLOAT ),
	//DEFINE_FIELD( m_fBeamCounter,		    FIELD_FLOAT ),
	DEFINE_FIELD( m_hViewModel,				FIELD_EHANDLE),


END_DATADESC()

acttable_t	CWeaponTaussOver::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },

	{ ACT_IDLE, ACT_IDLE_SMG1, true },

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

IMPLEMENT_ACTTABLE( CWeaponTaussOver );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponTaussOver::CWeaponTaussOver( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;
	m_nBulletType = -1;
	m_nChargeType = 0;
	m_nSecAttackAmmoDelayCount = 0;
	m_bHasCharge = false;
	m_fRandomPushTime = gpGlobals->curtime;

	m_flSpinAnimTime = 0;

	m_bWeaponCurState	 = 0;
	m_nNextAnimationTime = gpGlobals->curtime;
	m_flNextChargeTime = gpGlobals->curtime;

	//beamgrow
	//m_fBeamWidth = .5f;
	//m_fBeamCounter = 0;

	m_hViewModel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTaussOver::Precache( void )
{
	//m_nGlowSpriteIndex = PrecacheModel("sprites/glow08.vmt");

	//PrecacheParticleSystem("gauss_charge");
	//PrecacheParticleSystem("gauss_balls01");

	PrecacheParticleSystem("explosion_turret_break_pre_flash");	// charge start
	PrecacheParticleSystem("he_crystal_beams");		// charge
	PrecacheParticleSystem("he_crystal_explode");	// over charge explosion
	PrecacheParticleSystem("striderbuster_break");	// max shoot trail
	PrecacheParticleSystem("striderbuster_shotdown_trail");	// mid state 2 charge
	
	//PrecacheParticleSystem("weapon_muzzle_flash_pistol_FP_cheap");	// primary shot vm
	PrecacheParticleSystem("striderbuster_break_c");	// over charge explosion 2 lite

	BaseClass::Precache();
	//UTIL_PrecacheOther( "env_entity_dissolver" );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTaussOver::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponTaussOver::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim(GetSecondaryAttackActivity());
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration(GetSecondaryAttackActivity());

	if(m_bWeaponCurState != 0)	 // do I need this?
		m_bWeaponCurState = 0;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponTaussOver::PrimaryAttack( void )
{
	if(m_bHasCharge)
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	/*if ( UsesClipsForAmmo1() && !m_iClip1 )		// BJ moved to itempostframe	
	{
		Reload();
		return;
	}*/

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)	// BJ: no ammo NO shooting
	{
		DevMsg("Tau no ammo no shoot \n");
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		return;
	}
		

	// Cannot fire underwater
	if ( GetOwner() && GetOwner()->GetWaterLevel() == 3 )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		WeaponSound( EMPTY );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration( ACT_VM_DRYFIRE );

		if(m_bWeaponCurState != 0)	 // do I need this?
			m_bWeaponCurState = 0;

		/*
		// beamgrow
		if(m_fBeamWidth != 0)
		{
			m_fBeamCounter = 0;
			m_fBeamWidth = .5f;
		}
		return;
		*/
	}

	/*
	// Check to see if pre-fire spinup animation has ran through, or start it, and keep checking to see if it finished, then move on.
	if(m_bWeaponCurState == 0)
	{
		m_bWeaponCurState = 1;
		SendWeaponAnim( ACT_VM_PULLPIN );
		m_nNextAnimationTime = gpGlobals->curtime + SequenceDuration( ACT_VM_PULLPIN ); //+ .4f; hackin?

		//beamgrow
		if(m_fBeamWidth != 0)
		{
			m_fBeamCounter = 0;
			m_fBeamWidth = .5f;
		}
		return;
	} 
	else 
	{
		if (m_nNextAnimationTime <= gpGlobals->curtime)
		{
			if (m_bWeaponCurState == 1)
			{
				m_nNextAnimationTime = gpGlobals->curtime + SequenceDuration( ACT_VM_PRIMARYATTACK );
				SendWeaponAnim( ACT_VM_PRIMARYATTACK );
				// player "shoot" animation
				pPlayer->SetAnimation( PLAYER_ATTACK1 );
			}
		} 
		else 
		{
			if (m_nNextAnimationTime > gpGlobals->curtime)
			{
				if(m_bWeaponCurState == 1)
				{
					return;
				}
			} 
		}
		// beamgrow
		if( m_fBeamWidth < 3)
		{
			if( m_fBeamCounter < 3)
			{
				m_fBeamCounter += 1;
			}
			else
			{
				m_fBeamCounter = 0;
				m_fBeamWidth += .5f;
			}
		}
	}
	*/

//	DispatchParticleEffect("weapon_muzzle_flash_pistol_FP_cheap", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	ShootBeam_lite();	// lite beam
	m_iClip1 -= 1;	// remove 1 ammo pre shot
/*
	FireBulletsInfo_t info;
	info.m_vecSrc	 = pPlayer->Weapon_ShootPosition( );
	
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		
		//m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration(ACT_VM_PULLBACK);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;										// BJ: Main prim attack delay

		info.m_iShots++;
		if ( !fireRate )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = min( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = min( info.m_iShots, pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) );
		pPlayer->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}
	
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	info.m_flDamage = sk_plr_dmg_gauss_ovh_primary.GetFloat();	//6
	info.m_iPlayerDamage = 6; // if hit player
	info.m_flDamageForceScale = 100.0f;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets( info );

*/

	WeaponSound(SINGLE);
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;										// BJ: Main prim attack delay

	/*if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}*/

	// push the player back randomly
	if(m_fRandomPushTime < gpGlobals->curtime)
	{
		m_fRandomPushTime = gpGlobals->curtime + RandomFloat( .02f, .2f);

		Vector forward;
		AngleVectors( pPlayer->GetAbsAngles(), &forward );
		forward = -(forward * 45);
		pPlayer->VelocityPunch( forward );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTaussOver::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	
	if (IsNearWall() || GetOwnerIsRunning())
	{
		//SendWeaponAnim(ACT_VM_IDLE);
		return;
	}

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)	// BJ: no ammo NO shooting
	{
		DevMsg("Tau no ammo no shoot \n");
		WeaponSound(EMPTY);
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		return;
	}

	if (pPlayer->GetWaterLevel() == 3)
	{
		//EmitSound("Weapon_Gauss.Zap1");
		//SendWeaponAnim(ACT_VM_IDLE);
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		return;
	}

	if (m_bStopCharge)
		return;

	
	// The way it works now, it uses up ammo every .5 sec (need faster), then it should stop for a sec, when
	// it reaches the next charge level, then continue.. but in the end it never stops consuming ammo.. has to change.

	// make sure we have enough ammo every BEAMWEAPON_SAMMO_COSUM sec or you can't charge anymore.
	/*if(m_iClip1 >= 1)
	{
		//if(m_nChargeType < 3)
		//{
			//if(m_nSecondaryAmmoTime < gpGlobals->curtime)
			//{
				//m_iClip1--;
				//m_nSecondaryAmmoTime = gpGlobals->curtime + BEAMWEAPON_SAMMO_COSUM;
			//}
		//}
	}
	else
	{
		if(!m_bHasCharge)
		{
			Reload();
		}
		return;
	}*/

	if (m_nSecAttackAmmoDelayCount < 8)	// BJ: Main decrease ammo slowly
	{
		m_nSecAttackAmmoDelayCount++;
		//DevMsg("Tau sec ammo remove delay %i \n", m_nSecAttackAmmoDelayCount);
	}
	else if (m_nSecAttackAmmoDelayCount == 8)
	{
		m_iClip1--;	
		m_nSecAttackAmmoDelayCount = 0;
		//DevMsg("Tau sec ammo minus \n");
	}
	//else
	//{
		//m_nSecAttackAmmoDelayCount = m_nSecAttackAmmoDelayCount + 1;
	//}
		

	if(m_flNextChargeTime < gpGlobals->curtime)
	{
		if (!m_bHasCharge)
		{
			m_bHasCharge = true;
			//m_flAmmoRemoveDelay = 0.2f;
		}

		if(m_nChargeType < 4)	//3
		{
			if(m_nChargeType == 0)
			{
				WeaponSound(SPECIAL1);
			} 
			else if(m_nChargeType == 1)
			{
				WeaponSound( SPECIAL2 );
				//UTIL_ScreenShake( GetAbsOrigin(), (m_nChargeType * 5), 100.0f, 1.5f, 128, SHAKE_START, false );
			} 
			else if(m_nChargeType == 2)
			{
				WeaponSound( SPECIAL3 );
				//UTIL_ScreenShake( GetAbsOrigin(), (m_nChargeType * 5), 100.0f, 1.5f, 128, SHAKE_START, false );
			}
			m_nChargeType++;
			//m_nSecondaryAmmoTime = gpGlobals->curtime + .8f;		// pause the ammo consumption
		}

		if (m_nChargeType == 4)
		{
			//m_flNextChargeTime = gpGlobals->curtime + SequenceDuration(ACT_VM_PULLBACK_HIGH);	// ACT_VM_PRIMARYATTACK
			
			DevMsg("Tau State 4 ??? BOOM \n");

			m_bStopCharge = true;		// BJ: Stop read secondaryattack, stop use ammo
			WeaponSound(TAUNT);

			// BJ: Self damage to player. works method.
			CTakeDamageInfo info(this, this, sk_plr_dmg_gauss_ovh_overcharge.GetFloat(), DMG_SHOCK);	
			pPlayer->TakeDamage(info);

			// BJ: Punch player bigger on overcharge
			Vector forward;
			AngleVectors(pPlayer->GetAbsAngles(), &forward);
			forward = -(forward * 500);
			pPlayer->VelocityPunch(forward);

			// BJ: View fade effect
			color32 gaussDamage = { 255, 128, 0, 128 };
			UTIL_ScreenFade(pPlayer, gaussDamage, 0.2f, 0.2f, FFADE_IN);

			//DispatchParticleEffect("he_crystal_explode", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);		// BJ: Overcharge charging particle explosion
			//DispatchParticleEffect("he_crystal_beams", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);

			pPlayer->m_nButtons &= ~IN_ATTACK2;		// BJ: Button RELEASE! Stop hold it.

			//AttackCharge(m_nChargeType);
			m_flNextSecondaryAttack = gpGlobals->curtime + 1.5;

			//if (m_bHasCharge)
				//m_bHasCharge = false;
		}
		else if(m_nChargeType == 3)
		{
			// play charge animation
			//SendWeaponAnim( ACT_VM_PRIMARYATTACK );
//			SendWeaponAnim(ACT_VM_PULLBACK_HIGH);
			//m_flNextChargeTime = gpGlobals->curtime + SequenceDuration(ACT_VM_PULLBACK_HIGH);	// ACT_VM_PRIMARYATTACK
			m_flNextChargeTime = gpGlobals->curtime + 4.8f;

			//DispatchParticleEffect("he_crystal_beams", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);		// BJ: Overcharge charging particle mid
			DispatchParticleEffect("explosion_turret_break_pre_flash", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);
			DevMsg("Tau State 3 \n");
		}
		else if(m_nChargeType == 2)
		{
			// play charge animation
//			SendWeaponAnim( ACT_VM_PULLBACK_LOW );
			m_flNextChargeTime = gpGlobals->curtime + 2.7f;

			DispatchParticleEffect("explosion_turret_break_pre_flash", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);		// BJ: Overcharge charging particle start
			DevMsg("Tau State 2 \n");
		}
		else if (m_nChargeType == 1)
		{
			m_flNextChargeTime = gpGlobals->curtime + 1.3f;	// precheck

			DispatchParticleEffect("explosion_turret_break_pre_flash", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);		// start spin effects, replace?
			DevMsg("Tau State 1 \n");
		}
		else	// BJ: Seems this case didn't reading.
		{
			// play charge animation
//			SendWeaponAnim( ACT_VM_PULLBACK );
			// set the next charge time
			m_flNextChargeTime = gpGlobals->curtime + 0.5f;

			DevMsg("Tau State 0 \n");
		}
	}
	
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}
}



//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponTaussOver::CanHolster( void )
{
	if(m_bHasCharge)
		return false;

	if(m_bWeaponCurState != 0)
		m_bWeaponCurState = 0;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponTaussOver::Reload( void )
{
	if(m_bHasCharge)
		return false;

	//WeaponSound(RELOAD);

	return DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
}

//-----------------------------------------------------------------------------
// Purpose: Just need to check if you released the fire button, 
//          if so you'll have to wait the spin up time again.
// Input  : 
//-----------------------------------------------------------------------------

void CWeaponTaussOver::ItemPostFrame( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	// test
	BaseClass::ItemPostFrame();

	// check if we reloading
	if ( m_bInReload )
	{
		if(m_bWeaponCurState != 0)
			m_bWeaponCurState = 0;
		if (m_bHasCharge)
		{
			m_bHasCharge = false;
			//m_flAmmoRemoveDelay = -1;
		}
		return;
	}

	// BJ: Charge spin animations repeate
	if (m_bHasCharge && m_flSpinAnimTime < gpGlobals->curtime)
	{

		if ((m_nChargeType == 0) || (m_nChargeType == 1))
		{
			SendWeaponAnim(ACT_VM_PULLBACK);
			DevMsg("Tau spin anim 1 \n");
		}

		/*if (m_nChargeType == 1)
		{
		}*/

		if (m_nChargeType == 2)
		{
			SendWeaponAnim(ACT_VM_PULLBACK_LOW);
			DevMsg("Tau spin anim 2 \n");
		}

		if ((m_nChargeType == 3) || (m_nChargeType == 4))
		{
			SendWeaponAnim(ACT_VM_PULLBACK_HIGH);
			DevMsg("Tau spin anim 3 \n");
		}

		/*if (m_nChargeType == 4)
		{
		}*/


		//SendWeaponAnim(ACT_VM_PULLBACK);
		//SendWeaponAnim(ACT_GAUSS_SPINCYCLE);

		m_flSpinAnimTime = gpGlobals->curtime + GetViewModelSequenceDuration();
		//m_flNextPrimaryAttack = m_flSpinAnimTime + 0.1f;
	}

	// BJ: Player speed
	/*
	// check if we are max charge and restric running
	if( m_nChargeType > 2 ){
		if( pPlayer->IsSuitEquipped() ){
			pPlayer->SetMaxSpeed( 100 );
		} else {
			pPlayer->SetMaxSpeed( 90 );
		}
	} else {
		if( pPlayer->IsSuitEquipped() ){
			pPlayer->SetMaxSpeed( 190 );
		} else {
			pPlayer->SetMaxSpeed( 150 );
		}
	}
	*/

	if (m_iClip1 >= 1)		// BJ: Tricky auto-reload method
	{

	}
	else if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)	// BJ: if no ammo at all - switch
	{
		pPlayer->SwitchToNextBestWeapon(this);
	}
	else
	{
		if (m_bHasCharge)
		{
			pPlayer->m_nButtons &= ~IN_ATTACK2;		// BJ: Button RELEASE! Stop hold it.
			m_flNextSecondaryAttack = gpGlobals->curtime + 1.5;
			AttackCharge(m_nChargeType);
		}

		//if (!m_bHasCharge)
		//{
		Reload();
		DevMsg("Tau auto reload \n");
		//}
		//else

		//return;
	}

	//if (!(!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0))	// BJ: no ammo NO shooting no check
	//{

		// -----------------------
		//  No buttons down
		// -----------------------
		if (!((pPlayer->m_nButtons & IN_ATTACK) || (pPlayer->m_nButtons & IN_ATTACK2) || (pPlayer->m_nButtons & IN_RELOAD)) /*|| (m_bStopCharge)*/)
		{
			//DevMsg("Tau buttons unpressed \n");
			// no fire buttons down or reloading
			if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
			{
				if (m_nChargeType)
				{
					// send sound
					WeaponSound(SINGLE);

					// reset the charge time to now
					m_flNextChargeTime = gpGlobals->curtime;

					// do the attack
					AttackCharge(m_nChargeType);

					m_bHasCharge = false;
					//m_flAmmoRemoveDelay = -1;
					m_nChargeType = 0;
				}
				else if (m_bWeaponCurState != 0)
				{
					// send the spin down animation
					//SendWeaponAnim( ACT_VM_SWINGMISS );	// BJ: Disabled

					// reset m_bWeaponCurState to 0
					m_bWeaponCurState = 0;
				}
			}
		}

	//}


}

void CWeaponTaussOver::AttackCharge( int ChargeType )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	m_bStopCharge = false;	// BJ: Always reset charging state after shot

	//DispatchParticleEffect("he_crystal_explode", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", true);		// BJ: CLEANUP ALL before shot. Main VM shoot particle.
	DispatchParticleEffect("he_crystal_beams", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "charge", false);

	/*
	FireBulletsInfo_t info;
	info.m_vecSrc	 = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	*/

	if(ChargeType == 1)
	{
		ShootBeam_lite();	// different beam
		//info.m_flDamage = sk_plr_dmg_gauss_ovh_secondary1.GetFloat();		// 20
		//info.m_iPlayerDamage = 20;
		//info.m_flDamageForceScale = 100.0f;
	} 
	else if(ChargeType == 2)
	{
		ShootBeam_high();	// auto check for state 2 and 3
		//info.m_flDamage = sk_plr_dmg_gauss_ovh_secondary2.GetFloat();		// 80
		//info.m_iPlayerDamage = 80;
		//info.m_flDamageForceScale = 300.0f;
	} 
	else if(ChargeType == 3)
	{
		ShootBeam_high();	// auto check for state 2 and 3
		//info.m_flDamage = sk_plr_dmg_gauss_ovh_secondary3.GetFloat();		//220
		//info.m_iPlayerDamage = 220;
		//info.m_flDamageForceScale = 1000.0f;
	}
	
	/*
#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets( info );
	*/

	// push the player back
	Vector forward;
	AngleVectors( pPlayer->GetAbsAngles(), &forward );
	if(ChargeType == 1)
	{
		forward = -(forward * 10);
	} 
	else if(ChargeType == 2)
	{
		forward = -(forward * 100);
	} 
	else if(ChargeType == 3)
	{
		forward = -(forward * 380);
	}
	//else if (ChargeType == 4)				// BJ: Reused upper on secattack
	//{
		//forward = -(forward * 600);
	//}
	pPlayer->VelocityPunch( forward );

	SendWeaponAnim(GetSecondaryAttackActivity());
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 1;
}


void CWeaponTaussOver::ShootBeam_lite( void )		// BJ: if (secondary) replaced with if (m_bHasCharge) for primary and 1st secondary charge
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner){ return; }

	//m_bCharging = false;
	//isCharging = m_bCharging;
	//m_bInReload = false;

	//m_flNextSoundTime = gpGlobals->curtime + random->RandomFloat(0.5f, 1.2f);
	//blockSoundThink = true;

	if (m_hViewModel == NULL)
	{
		CBaseViewModel *vm = pOwner->GetViewModel();

		if (vm)
		{
			m_hViewModel.Set(vm);
		}
	}

	float flDamage = 0;	// BJ: First set to 0, then rewrite
	//float flDamagePrimary = sk_plr_dmg_gauss_ovh_primary.GetFloat();
	//float flDamageSecondary = sk_plr_dmg_gauss_ovh_secondary1.GetFloat();

	if (m_bHasCharge)
	{	
		flDamage = sk_plr_dmg_gauss_ovh_secondary1.GetFloat();
		//DevMsg("Tau litebeam set secondary dmg \n");
	}
	else
	{
		flDamage = sk_plr_dmg_gauss_ovh_primary.GetFloat();
		//DevMsg("Tau litebeam set primary dmg \n");
	}

	Vector		vecSrc = pOwner->Weapon_ShootPosition();
	Vector		aimDir = pOwner->GetAutoaimVector(0);
	Vector		vecDest = vecSrc + aimDir *MAX_TRACE_LENGTH;
	bool		fFirstBeam = true;
	bool		fHasPunched = false;
	float		flMaxFrac = 1.0;
	int			nMaxHits = 10;
	trace_t		trEnd;

	//if (secondary)
	if (m_bHasCharge)
		trEnd.startpos = vecSrc;

	CTakeDamageInfo dmgInfo(this, pOwner, flDamage, DMG_GAUSS);
	CTakeDamageInfo dmgInfo2(this, pOwner, flDamage, DMG_BULLET);


	while (flDamage > 10 && nMaxHits > 0)
	{
		trace_t	tr;

		nMaxHits--;

		UTIL_TraceLine(vecSrc, vecDest, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

		DrawBeam(tr.startpos, tr.endpos, 1, fFirstBeam/*, secondary*/);

		UTIL_DecalTrace(&tr, "FadingScorch");

		//if (secondary)
		if (m_bHasCharge)
			trEnd.endpos = tr.endpos;
		else
			trEnd = tr;

		if (tr.allsolid)
		{
			break;
		}

		CBaseEntity *pHit = tr.m_pEnt;

		if (pHit == NULL)
		{
			break;
		}

		if (fFirstBeam)
		{
			fFirstBeam = false;
		}

		float n;

		n = -DotProduct(tr.plane.normal, aimDir);

		if (pHit)
		{
			if (pHit->m_takedamage != DAMAGE_NO)
			{
				ClearMultiDamage();

				Vector Dir = /*secondary ? aimDir*-1 : */aimDir;
				//DevMsg("Dir: %.2f, %.2f, %.2f\n", Dir.x, Dir.y, Dir.z);

				if (pHit != NULL && pHit->IsNPC())
				{
					CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, Dir, tr.endpos);
					pHit->DispatchTraceAttack(dmgInfo, Dir, &tr);
				}
				else if (pHit != NULL && !pHit->IsNPC())
				{
					CalculateBulletDamageForce(&dmgInfo2, m_iPrimaryAmmoType, Dir, tr.endpos);
					pHit->DispatchTraceAttack(dmgInfo2, Dir, &tr);
				}
			}

			ApplyMultiDamage();

			if (n >= 0.5)
			{
				//if (secondary)
				if (m_bHasCharge)
					UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGaussCharged");
				else
					UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

				CPVSFilter filter(tr.endpos);

				te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
			}

			//ShouldDrawWaterImpacts(tr);

			//CPVSFilter filter(tr.endpos);

			//te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);

			if (/*!secondary &&*/ (!m_bHasCharge) && !pHit->IsBSPModel())
			{
				fHasPunched = true;
				break;
			}

		}

		//if (secondary)
		//{
			//ApplyMultiDamage();

			//UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

			ShouldDrawWaterImpacts(tr);

		//}

		if (pHit && pHit->IsBSPModel() && (pHit->m_takedamage == DAMAGE_NO))
		{


			if (n < 0.5) // 60 degrees
			{
				Vector r;

				r = 2.0 * tr.plane.normal * n + aimDir;
				flMaxFrac = flMaxFrac - tr.fraction;
				aimDir = r;
				vecSrc = tr.endpos;
				vecDest = vecSrc + aimDir * MAX_TRACE_LENGTH;

				// explode a bit
				RadiusDamage(CTakeDamageInfo(this, pOwner, flDamage * n, DMG_BLAST), tr.endpos, flDamage * n * 2.5, CLASS_NONE, NULL);

				if (n == 0)
					n = 0.1;

				flDamage = flDamage * (1 - n);
			}
			else
			{
				//if (!secondary)
				if (!m_bHasCharge)
					fHasPunched = true;

				if (fHasPunched)
				{
					break;
				}

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				//if (secondary)
				if (m_bHasCharge)
				{
					trace_t beam_tr;

					UTIL_TraceLine(tr.endpos + aimDir * 8, vecDest, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &beam_tr);

					if (!beam_tr.allsolid)
					{
						// trace backwards to find exit point
						UTIL_TraceLine(beam_tr.endpos, tr.endpos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &beam_tr);

						float n = (beam_tr.endpos - tr.endpos).Length();

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;

							flDamage -= n;

							float flDamageRadius;

							/*if (g_pGameRules->IsMultiplayer())
							{
								flDamageRadius = flDamage * 1.75;  // Old code == 2.5
							}
							else*/
							//{
								flDamageRadius = flDamage * 2.5;
							//}

							RadiusDamage(CTakeDamageInfo(this, pOwner, flDamage, DMG_BLAST), beam_tr.endpos + aimDir * 8, flDamageRadius, CLASS_NONE, NULL);

							vecSrc = beam_tr.endpos + aimDir;
						}
						else
						{
							fHasPunched = true;

							break;
						}
					}
					else
					{
						flDamage = 0;
					}
				}
				else
				{
					flDamage = 0;
				}
			}
		}
		else
		{
			vecSrc = tr.endpos + aimDir;
		}
	}

	//if (secondary)
	if (m_bHasCharge)
	{
		CEffectData data;
		data.m_vOrigin = trEnd.endpos;
		data.m_nEntIndex = pOwner->entindex();
		DispatchEffect("GaussLightCharged", data);
	}	
	else
	{
		CEffectData data;
		data.m_vOrigin = trEnd.endpos;
		data.m_nEntIndex = pOwner->entindex();
		data.m_vStart.x = GetWpnData().iMuzzleFlashLightR;
		data.m_vStart.y = GetWpnData().iMuzzleFlashLightG;
		data.m_vStart.z = GetWpnData().iMuzzleFlashLightB;
		DispatchEffect("GaussLight", data);
	}


	//m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	AddViewKick();

	pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

}

void CWeaponTaussOver::DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	float life = 0.1f; //oc_weapon_gauss_beam_life.GetFloat();
	//Check to store off our view model index
	if (m_hViewModel == NULL)// Приводит к вылету игры(жалуется на player.cpp(489-я строка) return m_hViewModel[index]
	{
		CBaseViewModel *vm = pOwner->GetViewModel();
		if (!vm)
			return;

		if (vm)
		{
			m_hViewModel.Set(vm);
		}

	}

	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)		// BJ: No beam with 0 ammo. Switch weapon beam effect fix.
		return;

	color32 beamColor;
	beamColor.a = 255;
	beamColor.r = 255;
	beamColor.g = /*secondary ? 255 :*/ 145 + random->RandomInt(-16, 16);
	beamColor.b = /*secondary ? 255 :*/ 0;

	CBeam *pBeam = CBeam::BeamCreate(GAUSS_BEAM_SPRITE_LITE, width);

	if (useMuzzle)
	{
		pBeam->PointEntInit(endPos, m_hViewModel);
		pBeam->SetEndAttachment(1);
	}
	else
	{
		pBeam->SetStartPos(startPos);
		pBeam->SetEndPos(endPos);
	}

	float widthKoef = /*secondary ? 4 :*/ 1;
	pBeam->SetBrightness(beamColor.a);
	pBeam->SetColor(beamColor.r, beamColor.g, beamColor.b);
	pBeam->SetWidth(width * widthKoef);
	pBeam->SetEndWidth(width * widthKoef);
	pBeam->RelinkBeam();
	pBeam->LiveForTime(life);


	Vector	recoilForce = pOwner->GetAbsVelocity() - pOwner->GetAutoaimVector(0) * (1.0f);
	recoilForce[2] += 10.0f;
	pOwner->SetAbsVelocity(recoilForce);

	QAngle punch;
	punch.Init(SharedRandomFloat("gaussX", -5, 5), SharedRandomFloat("gaussY", -5, 5), 0);

}

#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponTaussOver::ShouldDrawWaterImpacts(const trace_t &shot_trace)
{

	// We must start outside the water
	if (WaterContents(shot_trace.startpos))
		return false;

	// We must end inside of water
	if (!WaterContents(shot_trace.endpos))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(shot_trace.startpos, shot_trace.endpos, (CONTENTS_WATER | CONTENTS_SLIME), UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &waterTrace);


	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(2.0, 4.0f);	// Water effect scale

		// See if we hit slime
		if (FBitSet(waterTrace.contents, CONTENTS_SLIME))
		{
			FSetBit(data.m_fFlags, FX_WATER_IN_SLIME);
		}

		CPASFilter filter(data.m_vOrigin);
		te->DispatchEffect(filter, 0.0, data.m_vOrigin, "watersplash", data);
	}
	return true;
}

void CWeaponTaussOver::ShootBeam_high(void)
{

	//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	//if ( !pOwner ){ return; }

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;


	pPlayer->DoMuzzleFlash();
	//	if (pPlayer)
	/*{

	pPlayer->AddEffects(EF_M);

	}*/
	//	m_flChargeTime2 = 0;
	bool penetrated = false;

	//WeaponSound(SINGLE);	//temp
	//WeaponSound(SPECIAL2);
	//WeaponSound( WPN_DOUBLE );
	//WeaponSound( SPECIAL2 );
	//EmitSound("Weapon_Gauss.After");

	/*
	if (oc_weapon_gauss_animation.GetInt() == 0)
	{
		SendWeaponAnim(GetSecondaryAttackActivity());
	}
	if (oc_weapon_gauss_animation.GetInt() == 1)
	{
		SendWeaponAnim(GetSecondaryAttackActivity());
	}
	if (oc_weapon_gauss_animation.GetInt() == 2)
	{
		int i = RandomInt(1, 2);
		if (i == 1)
		{
			SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		}
		if (i == 2)
		{
			SendWeaponAnim(GetSecondaryAttackActivity());
		}
	}
	*/


	//pPlayer->SetAnimation(PLAYER_ATTACK1);
	//StopChargeSound();

	//m_bCharging = false;
	//m_bChargeIndicated = false;

	//m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
	//m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	// Prepare tracers
	Vector	startPos = pPlayer->Weapon_ShootPosition();
	Vector	aimDir = pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector	endPos = startPos + (aimDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine(startPos, endPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
	ShouldDrawWaterImpacts(tr); // Check for water
	ClearMultiDamage();

	//DispatchParticleEffect("gauss_muzzle_flash", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged
	//DispatchParticleEffect("gauss_normal2", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged
	//DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);
	//DispatchParticleEffect("gauss_smoke", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);


	// Calc final damage
	//float flChargeAmount = (gpGlobals->curtime - m_flChargeStartTime) / MAX_GAUSS_CHARGE_TIME;

	//if (flChargeAmount > 1.0f){ flChargeAmount = 1.0f; }

	float flDamage2 = sk_plr_dmg_gauss_ovh_secondary2.GetFloat();
	float flDamage3 = sk_plr_dmg_gauss_ovh_secondary3.GetFloat();

	//float flDamage = sk_plr_dmg_gauss.GetFloat() + ((sk_plr_max_dmg_gauss.GetFloat() - sk_plr_dmg_gauss.GetFloat()) * flChargeAmount);

	CBaseEntity *pHit = tr.m_pEnt;
	if (tr.DidHitWorld())
	{
		UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");
		UTIL_DecalTrace(&tr, "RedGlowFade");
		if (m_nChargeType == 2)
		{
			DispatchParticleEffect("gauss_balls01", tr.endpos, vec3_angle);			//OverCharged
		}
		if (m_nChargeType == 3)
		{
			DispatchParticleEffect("striderbuster_break_c", tr.endpos, vec3_angle);	// "striderbuster_break_c" lite	 "striderbuster_break" full
		}
		CPVSFilter filter(tr.endpos);
		te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
		Vector  vStore = tr.endpos;
		Vector	testPos = tr.endpos + (aimDir * 48.0f);

		UTIL_TraceLine(testPos, tr.endpos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		if (!tr.allsolid)
		{
			UTIL_DecalTrace(&tr, "RedGlowFade");

			if (m_nChargeType == 3)		// BJ: Shoot through walls only on 3rd state
			{
				penetrated = true;
			}

			trace_t backward_tr;
			UTIL_TraceLine(tr.endpos, vStore, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &backward_tr);
			if (backward_tr.DidHit()){
				UTIL_ImpactTrace(&backward_tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");
			}

		}
	}
	else if (pHit != NULL)
	{

		if (m_nChargeType == 2)
		{
			CTakeDamageInfo dmgInfo(this, pPlayer, flDamage2, DMG_SHOCK);
			CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos);
			pHit->DispatchTraceAttack(dmgInfo, aimDir, &tr);
		}
		else if (m_nChargeType == 3)
		{
			CTakeDamageInfo dmgInfo(this, pPlayer, flDamage3, DMG_ENERGYBEAM);				// BJ: Gargantua allowed dmg + dissolve
			CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos);
			pHit->DispatchTraceAttack(dmgInfo, aimDir, &tr);
		}


	}

	ApplyMultiDamage();

	UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat(-4.0f, -8.0f);
	viewPunch.y = random->RandomFloat(-0.25f, 0.25f);
	viewPunch.z = 0;

	pPlayer->ViewPunch(viewPunch);

	DrawBeam_high(startPos, tr.endpos, 9.6, true);

	if (m_nChargeType == 2)
	{
		Vector	recoilForce = pPlayer->GetAbsVelocity() - pPlayer->GetAutoaimVector(0) * (flDamage2 * 1.0f);
		recoilForce[2] += 15.0f;
		pPlayer->SetAbsVelocity(recoilForce);
	}
	if (m_nChargeType == 3)
	{
		Vector	recoilForce = pPlayer->GetAbsVelocity() - pPlayer->GetAutoaimVector(0) * (flDamage3 * 1.0f);
		recoilForce[2] += 15.0f;
		pPlayer->SetAbsVelocity(recoilForce);
	}

	CPVSFilter filter(tr.endpos);
	te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
	if (penetrated)
	{
		if (m_nChargeType == 2)
		{
			RadiusDamage(CTakeDamageInfo(this, this, flDamage2, DMG_SHOCK), tr.endpos, 100.0f, CLASS_NONE, NULL);
		}
		if (m_nChargeType == 3)
		{
			RadiusDamage(CTakeDamageInfo(this, this, flDamage3, DMG_ENERGYBEAM), tr.endpos, 200.0f, CLASS_NONE, NULL);		// BJ: Gargantua allowed dmg + dissolve
		}

		UTIL_TraceLine(tr.endpos, endPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
		UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");
		UTIL_DecalTrace(&tr, "RedGlowFade");

		if (m_nChargeType == 2)
		{
			RadiusDamage(CTakeDamageInfo(this, this, flDamage2, DMG_SHOCK), tr.endpos, 100.0f, CLASS_NONE, NULL);
		}
		if (m_nChargeType == 3)
		{
			RadiusDamage(CTakeDamageInfo(this, this, flDamage3, DMG_ENERGYBEAM), tr.endpos, 200.0f, CLASS_NONE, NULL);		// BJ: Gargantua allowed dmg + dissolve
		}
	}

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	//	EmitSound("Weapon_gauss.Recharge");
	//PrepareHitmarker();
	//MuzzleFlashTime = 0.0f;
}

void CWeaponTaussOver::DrawBeam_high(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	float life = 0.2f; //oc_weapon_gauss_beam_life.GetFloat();

	if (m_hViewModel == NULL)
	{
		CBaseViewModel *vm = pOwner->GetViewModel();
		if (!vm)
			return;

		if (vm)
		{
			m_hViewModel.Set(vm);
		}

	}

	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)		// BJ: No beam with 0 ammo. Switch weapon beam effect fix.
		return;

	CBeam *pBeam = CBeam::BeamCreate(GAUSS_BEAM_SPRITE_HIGH, width);

	if (useMuzzle)
	{
		pBeam->PointEntInit(endPos, m_hViewModel);
		pBeam->SetEndAttachment(1);
		pBeam->SetWidth(width / 5.0f);
		pBeam->SetEndWidth(width);
	}
	else
	{
		pBeam->SetStartPos(startPos);
		pBeam->SetEndPos(endPos);
		pBeam->SetWidth(width);
		pBeam->SetEndWidth(width / 4.0f);
	}

	pBeam->SetBrightness(255);
	pBeam->SetColor(255, 145 + random->RandomInt(-16, 16), 0);
	pBeam->RelinkBeam();
	pBeam->LiveForTime(life);
	//	pBeam->LiveForTime( 0.025f );

	// Some sparks
	for (int i = 0; i < 3; i++)
	{
		pBeam = CBeam::BeamCreate(GAUSS_BEAM_SPRITE_HIGH, (width / 2.0f) + i);

		if (useMuzzle)
		{
			pBeam->PointEntInit(endPos, m_hViewModel);
			pBeam->SetEndAttachment(1);
		}
		else
		{
			pBeam->SetStartPos(startPos);
			pBeam->SetEndPos(endPos);
		}

		pBeam->SetBrightness(random->RandomInt(155, 255));
		pBeam->SetColor(255, 255, 150 + random->RandomInt(0, 64));
		pBeam->RelinkBeam();
		pBeam->LiveForTime(life);
		pBeam->SetNoise(1.6f * i);
		pBeam->SetEndWidth(0.04f);
	}
	//QAngle pAngles;
	//VectorAngles(endPos, pAngles);
	//DispatchParticleEffect("Gauss_impact_round_sparks", endPos, pAngles);

	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return;
	Vector	vForward, vRight, vUp, vThrowPos;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
	vThrowPos += vForward * 2.0f;
	vThrowPos += vRight * 0.1f;// *1.0f;
	vThrowPos += vUp * -3.4f;
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &vThrowPos, 140, 140, 0, 2, 120, 0.09, 0);
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
	vThrowPos += vForward * 2.0f;
	vThrowPos += vRight * 7.4f;
	vThrowPos += vUp * -3.4f;
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &vThrowPos, 140, 140, 0, 1, 120, 0.09, 0);
	}*/

	Vector	recoilForce = pOwner->GetAbsVelocity() - pOwner->GetAutoaimVector(0) * (1.0f);
	recoilForce[2] += 10.0f;
	pOwner->SetAbsVelocity(recoilForce);

	QAngle punch;
	punch.Init(SharedRandomFloat("gaussX", -5, 5), SharedRandomFloat("gaussY", -5, 5), 0);

}

/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - where the beam should begin
//          &endPos - where the beam should end
//          width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeaponTaussOver::DrawBeam( const Vector &startPos, const Vector &endPos, char* t_sprite, char* t_Attachment, float width, float attackDuration )
{
	//Tracer down the middle
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );
 
	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( t_sprite, width ); //width here use to be 15.5

	// It starts at startPos
	pBeam->SetStartPos( startPos );
 
	// This sets up some things that the beam uses to figure out where
	// it should start and end
	pBeam->PointEntInit( endPos, this );
 
	// This makes it so that the laser appears to come from the assigned attachment
	pBeam->SetEndAttachment( LookupAttachment(t_Attachment) );
	//pBeam->SetEndAttachment( LookupAttachment("Muzzle") );

	if(m_nChargeType > 0)
	{
		float newWidth = width / (m_nChargeType + 1);
		pBeam->SetWidth( newWidth );
	}
	else
	{
		pBeam->SetWidth( width );
	}

	// Higher brightness means less transparent
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
 
	// The beam should only exist for a very short time
	pBeam->LiveForTime( attackDuration );

	// The Beams should scroll foward
	pBeam->SetScrollRate( 30 );

}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - used to figure out where to do the effect
//          nDamageType - ???
//-----------------------------------------------------------------------------
void CWeaponTaussOver::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw beams
	if(m_bHasCharge && m_nChargeType > 0)
	{
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_KILLERBEAM_SPRITE, BEAMWEAPON_BEAM_ATTACHMENT, (m_nChargeType), SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_nChargeType was 3
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_KILLERSMALLBEAM_SPRITE, BEAMWEAPON_TARMBEAM_ATTACHMENT, (m_nChargeType + 1), SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_nChargeType was 4
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_KILLERSMALLBEAM_SPRITE, BEAMWEAPON_RARMBEAM_ATTACHMENT, (m_nChargeType + 1), SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_nChargeType was 4
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_KILLERSMALLBEAM_SPRITE, BEAMWEAPON_LARMBEAM_ATTACHMENT, (m_nChargeType + 1), SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_nChargeType was 4
	}
	else
	{
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_BEAM_SPRITE, BEAMWEAPON_BEAM_ATTACHMENT, (m_fBeamWidth + 1), SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_fBeamWidth 4 before beamgrow
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_SMALLBEAM_SPRITE, BEAMWEAPON_TARMBEAM_ATTACHMENT, m_fBeamWidth, SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_fBeamWidth 3 before beamgrow
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_SMALLBEAM_SPRITE, BEAMWEAPON_RARMBEAM_ATTACHMENT, m_fBeamWidth, SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_fBeamWidth 3 before beamgrow
		DrawBeam( tr.startpos, tr.endpos, BEAMWEAPON_SMALLBEAM_SPRITE, BEAMWEAPON_LARMBEAM_ATTACHMENT, m_fBeamWidth, SequenceDuration( ACT_VM_PRIMARYATTACK ) ); // m_fBeamWidth 3 before beamgrow
	}
	
	// not shooting the sky && tracer did not end up underwater, if so, do not do the impact effects!
	if (!(tr.surface.flags & SURF_SKY))
	{
		CPVSFilter filter( tr.endpos );
		if(!( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) ))
		{
			te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );

			// if you hit the world, it should lave a glow sprite
			if ( tr.DidHitWorld() )
			{
				// new proper glow sprite
				CSprite *pSprite = CSprite::SpriteCreate( "sprites/glow08.vmt", tr.endpos, false );

				if ( pSprite )
				{
					pSprite->FadeAndDie( 1.5f );
					pSprite->SetScale( .4f );
					if(m_bHasCharge)
					{
						if(m_nChargeType > 0)
						{
							pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
						}
					}
					else
					{
						pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
					}
					pSprite->SetGlowProxySize( 4.0f );
				}
			}

			//UTIL_DecalTrace( &tr, "FadingScorch" );
			UTIL_DecalTrace(&tr, "RedGlowFade");
			//DispatchParticleEffect("gauss_balls01", tr.endpos, vec3_angle);	// BJ: Added
		}

		// BJ: No explode
		/*
		// if you are at charge level 3, you should always explode, even under water
		if(m_nChargeType == 3)
		{
			te->Explosion( filter, 0.0,
				&tr.endpos,   // &GetAbsOrigin()
				g_sModelIndexFireball,
				2.0, 
				15,
				TE_EXPLFLAG_NONE,
				200, //m_DmgRadius was 90
				120 ); //m_flDamage
		}
		*/
	//}			//


	/*
	// do shock damage on level 3
	if(m_bHasCharge)
	{
		if(m_nChargeType == 2)
		{
			RadiusDamage(CTakeDamageInfo(this, this, sk_plr_dmg_gauss_ovh_secondary2.GetFloat(), DMG_SHOCK), tr.endpos, 160.0f, CLASS_NONE, NULL);
		} 
		else if(m_nChargeType == 3)
		{
			RadiusDamage(CTakeDamageInfo(this, this, sk_plr_dmg_gauss_ovh_secondary3.GetFloat(), DMG_SHOCK), tr.endpos, 300.0f, CLASS_NONE, NULL);
		}
	}
	*/
//}		//
