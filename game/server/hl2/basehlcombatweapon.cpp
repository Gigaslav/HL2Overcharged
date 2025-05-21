//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "soundent.h"
#include "ai_basenpc.h"
#include "game.h"
#include "in_buttons.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( CHLMachineGun, DT_HLMachineGun )
END_SEND_TABLE()

//=========================================================
//	>> CHLSelectFireMachineGun
//=========================================================
BEGIN_DATADESC( CHLMachineGun )

	DEFINE_FIELD( m_nShotsFired,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHLMachineGun::CHLMachineGun( void )
{
	ChangeOnce = true;
}

const Vector &CHLMachineGun::GetBulletSpread( void )
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		static Vector spread;
		if (GetOwner()->GetActiveWeapon())
		{
			if ((GetOwner()->GetFlags() & FL_DUCKING) != 0)
			{
				if (IsIronSighted())
				{
					spread.x = GetWpnData().irSightSpread;
					spread.y = GetWpnData().irSightSpread;
					spread.z = GetWpnData().irSightSpread;
				}
				else if (IsScopeSighted())
				{
					spread.x = GetWpnData().ScopeSightSpread;
					spread.y = GetWpnData().ScopeSightSpread;
					spread.z = GetWpnData().ScopeSightSpread;
				}
				else
				{
					spread.x = GetWpnData().crouchSpread;
					spread.y = GetWpnData().crouchSpread;
					spread.z = GetWpnData().crouchSpread;
				}
			}
			else
			{
				if (GetOwnerIsRunning())//(cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() > cvar->FindVar("hl2_normspeed")->GetFloat() + 50.f)
				{
					spread.x = GetWpnData().runningSpread;
					spread.y = GetWpnData().runningSpread;
					spread.z = GetWpnData().runningSpread;
				}
				else if (!GetOwnerIsRunning() && cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 50.f)//(cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() <= cvar->FindVar("hl2_normspeed")->GetFloat())
				{
					if (IsIronSighted())
					{
						spread.x = GetWpnData().irSightSpread;
						spread.y = GetWpnData().irSightSpread;
						spread.z = GetWpnData().irSightSpread;
					}
					else if (IsScopeSighted())
					{
						spread.x = GetWpnData().ScopeSightSpread;
						spread.y = GetWpnData().ScopeSightSpread;
						spread.z = GetWpnData().ScopeSightSpread;
					}
					else
					{
						spread.x = GetWpnData().walkingSpread;
						spread.y = GetWpnData().walkingSpread;
						spread.z = GetWpnData().walkingSpread;
					}
				}
				else if (!GetOwnerIsRunning() && cvar->FindVar("oc_state_player_velocity")->GetFloat() < 50.f)
				{
					if (IsIronSighted())
					{
						spread.x = GetWpnData().irSightSpread;
						spread.y = GetWpnData().irSightSpread;
						spread.z = GetWpnData().irSightSpread;
					}
					else if (IsScopeSighted())
					{
						spread.x = GetWpnData().ScopeSightSpread;
						spread.y = GetWpnData().ScopeSightSpread;
						spread.z = GetWpnData().ScopeSightSpread;
					}
					else
					{
						spread.x = GetWpnData().standingSpread;
						spread.y = GetWpnData().standingSpread;
						spread.z = GetWpnData().standingSpread;
					}
				}
			}
		}
		return spread;
	}
	else if (GetOwner() && GetOwner()->IsNPC())
	{
		static Vector spread;
		if (GetOwner()->GetActiveWeapon())
		{
			if ((GetOwner()->GetFlags() & FL_DUCKING) != 0)
			{
				spread.x = GetWpnData().crouchSpreadNpc;
				spread.y = GetWpnData().crouchSpreadNpc;
				spread.z = GetWpnData().crouchSpreadNpc;
			}
			else
			{
				if (GetOwner()->m_flSpeed > 0)
				{
					spread.x = GetWpnData().runningSpreadNpc;
					spread.y = GetWpnData().runningSpreadNpc;
					spread.z = GetWpnData().runningSpreadNpc;
				}
				/*else if (cvar->FindVar("oc_state_player_velocity")->GetFloat() >= 10 && cvar->FindVar("oc_state_player_velocity")->GetFloat() <= cvar->FindVar("hl2_normspeed")->GetFloat())
				{
					spread.x = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
					spread.y = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
					spread.z = GetOwner()->GetActiveWeapon()->GetWpnData().walkingSpread;
				}*/
				else if (GetOwner()->m_flSpeed <= 0)
				{
					spread.x = GetWpnData().standingSpreadNpc;
					spread.y = GetWpnData().standingSpreadNpc;
					spread.z = GetWpnData().standingSpreadNpc;
				}
			}
		}
		return spread;
	}

	static Vector cone = VECTOR_CONE_3DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHLMachineGun::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CHLMachineGun::FireBullets( const FireBulletsInfo_t &info )
{
	if(CBasePlayer *pPlayer = ToBasePlayer ( GetOwner() ) )
	{
		pPlayer->FireBullets(info);
	}
}
void CHLMachineGun::FireBulletsShotgun(const FireBulletsInfo_t &info)
{
	if (CBasePlayer *pPlayer = ToBasePlayer(GetOwner()))
	{
		pPlayer->FireBulletsShotgun(info);
	}
}
void CHLMachineGun::FireBulletsVehicle(const FireBulletsInfo_t &info)
{
	if (CBasePlayer *pPlayer = ToBasePlayer(GetOwner()))
	{
		pPlayer->FireBulletsVehicle(info);
	}
}
//-----------------------------------------------------------------------------
// Purpose: Weapon firing conditions
//-----------------------------------------------------------------------------
int CHLMachineGun::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	if ( m_iClip1 <=0 )
	{
		return COND_NO_PRIMARY_AMMO;
	}
	else if ( flDist < m_fMinRange1 ) 
	{
		return COND_TOO_CLOSE_TO_ATTACK;
	}
	else if ( flDist > m_fMaxRange1 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if ( flDot < 0.5f )	// UNDONE: Why check this here? Isn't the AI checking this already?
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLMachineGun::DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime )
{
	#define	KICK_MIN_X			0.2f	//Degrees
	#define	KICK_MIN_Y			0.2f	//Degrees
	#define	KICK_MIN_Z			0.1f	//Degrees

	QAngle vecScratch;
	
	//Find how far into our accuracy degradation we are
	float duration	= ( fireDurationTime > slideLimitTime ) ? slideLimitTime : fireDurationTime;
	float kickPerc = duration / slideLimitTime;

	// do this to get a hard discontinuity, clear out anything under 10 degrees punch
	pPlayer->ViewPunchReset( 10 );

	//Apply this to the view angles as well
	vecScratch.x = -( KICK_MIN_X + ( maxVerticleKickAngle * kickPerc ) );
	vecScratch.y = -( KICK_MIN_Y + ( maxVerticleKickAngle * kickPerc ) ) / 3;
	vecScratch.z = KICK_MIN_Z + ( maxVerticleKickAngle * kickPerc ) / 8;

	//Wibble left and right
	if ( random->RandomInt( -1, 1 ) >= 0 )
		vecScratch.y *= -1;

	//Wobble up and down
	if ( random->RandomInt( -1, 1 ) >= 0 )
		vecScratch.z *= -1;

	//If we're in easy, dampen the effect a bit
	if ( g_pGameRules->IsSkillLevel( SKILL_EASY ) )
	{
		for ( int i = 0; i < 3; i++ )
		{
			vecScratch[i] *= dampEasy;
		}
	}

	//Clip this to our desired min/max
	UTIL_ClipPunchAngleOffset( vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle( 24.0f, 3.0f, 1.0f ) );

	//Add it to the view punch
	// NOTE: 0.5 is just tuned to match the old effect before the punch became simulated
	pPlayer->ViewPunch( vecScratch * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset our shots fired
//-----------------------------------------------------------------------------
bool CHLMachineGun::Deploy( void )
{
	m_nShotsFired = 0;

	return BaseClass::Deploy();
}
bool CHLMachineGun::FirstDeploy(int iActivity)
{
	m_nShotsFired = 0;
	return BaseClass::FirstDeploy(iActivity);
}


//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CHLMachineGun::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if (m_flNextSoundTime < gpGlobals->curtime)
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}



bool DoOnceM = true;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLMachineGun::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		m_nShotsFired = 0;
	}

	BaseClass::ItemPostFrame();
}

IMPLEMENT_SERVERCLASS_ST( CHLSelectFireMachineGun, DT_HLSelectFireMachineGun )
END_SEND_TABLE()

//=========================================================
//	>> CHLSelectFireMachineGun
//=========================================================
BEGIN_DATADESC( CHLSelectFireMachineGun )

	DEFINE_FIELD( m_iBurstSize,		FIELD_INTEGER ),
	DEFINE_FIELD( m_iFireMode,		FIELD_INTEGER ),
	
	// Function pinters
	DEFINE_FUNCTION( BurstThink ),

END_DATADESC()



float CHLSelectFireMachineGun::GetBurstCycleRate( void )
{
	// this is the time it takes to fire an entire 
	// burst, plus whatever amount of delay we want
	// to have between bursts.
	return 0.5f;
}

float CHLSelectFireMachineGun::GetFireRate( void )
{
	return BaseClass::GetFireRate();
}

bool CHLSelectFireMachineGun::Deploy( void )
{
	// Forget about any bursts this weapon was firing when holstered
	m_iBurstSize = 0;
	return BaseClass::Deploy();
}
bool CHLSelectFireMachineGun::FirstDeploy(int iActivity)
{
	// Forget about any bursts this weapon was firing when holstered
	m_iBurstSize = 0;
	return BaseClass::FirstDeploy(iActivity);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHLSelectFireMachineGun::PrimaryAttack( void )
{
	/*if (m_bFireOnEmpty)
	{
		return;
	}
	switch( m_iFireMode )
	{
	case FIREMODE_FULLAUTO:
		BaseClass::PrimaryAttack();
		// Msg("%.3f\n", m_flNextPrimaryAttack.Get() );
		SetWeaponIdleTime( gpGlobals->curtime + 3.0f );
		break;

	case FIREMODE_ALYXGUN_AUTO://OverCharged
//		BaseClass::PrimaryAttack();//OverCharged
		// Msg("%.3f\n", m_flNextPrimaryAttack.Get() );
//		SetWeaponIdleTime(gpGlobals->curtime + 3.0f);
		break;

	case FIREMODE_3RNDBURST:
		m_iBurstSize = GetBurstSize();
		
		// Call the think function directly so that the first round gets fired immediately.
		BurstThink();
		SetThink( &CHLSelectFireMachineGun::BurstThink );
		m_flNextPrimaryAttack = gpGlobals->curtime + GetBurstCycleRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetBurstCycleRate();

		// Pick up the rest of the burst through the think function.
		SetNextThink( gpGlobals->curtime + GetFireRate() );
		break;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired( pOwner, true, GetClassname() );
	}

	AddViewKick();*/

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHLSelectFireMachineGun::SecondaryAttack( void )
{
	// change fire modes.

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	switch( m_iFireMode )
	{
	case FIREMODE_FULLAUTO:
		//Msg( "Burst\n" );
		m_iFireMode = FIREMODE_3RNDBURST;
		WeaponSound(SPECIAL2);
		if (pOwner)
			UTIL_ShowMessage("WPN_BURST", pOwner);			// BriJee OVR: New ingame notifications
		break;

	case FIREMODE_3RNDBURST:
		//Msg( "Auto\n" );
		m_iFireMode = FIREMODE_FULLAUTO;
		WeaponSound(SPECIAL1);
		if (pOwner)
			UTIL_ShowMessage("WPN_FULLAUTO", pOwner);
		break;
	}
	
	SendWeaponAnim( GetSecondaryAttackActivity() );

	m_flNextSecondaryAttack = gpGlobals->curtime + 1.3;		// BriJee OVR: Adjust time between modes!

	/*if (pOwner && ((m_iClip2 <= (GetMaxClip2() / 4)) && m_iClip2 != 0 && m_iClip2 <= 2))
		WeaponSound(LOW_AMMO);*/

	//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		m_iSecondaryAttacks++;
		gamestats->Event_WeaponFired( pOwner, false, GetClassname() );
	}

	AddViewKick();

	/*if (pOwner->GetActiveWeapon() && pOwner->GetActiveWeapon()->GetWpnData().iMuzzleFlashTypeS != NULL)
	{
		int ind = random->RandomInt(1, pOwner->GetActiveWeapon()->GetWpnData().externalIndexS);
		char prclName2[256];
		Q_snprintf(prclName2, sizeof(prclName2), "%s", pOwner->GetActiveWeapon()->GetWpnData().iMuzzleFlashTypeS[ind]);
		char prclAName2[256];
		Q_snprintf(prclAName2, sizeof(prclAName2), "%s", pOwner->GetActiveWeapon()->GetWpnData().iSecondaryAttachment);

		DispatchParticleEffect(prclName2, PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), prclAName2, false);
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHLSelectFireMachineGun::BurstThink( void )
{
	CHLMachineGun::PrimaryAttack();

	m_iBurstSize--;

	if( m_iBurstSize == 0 )
	{
		// The burst is over!
		SetThink(NULL);

		// idle immediately to stop the firing animation
		SetWeaponIdleTime( gpGlobals->curtime );
		return;
	}

	SetNextThink( gpGlobals->curtime + GetFireRate() );
}

void CHLSelectFireMachineGun::BurstThink2(void)
{
	CHLMachineGun::PrimaryAttack();

}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHLSelectFireMachineGun::WeaponSound( WeaponSound_t shoot_type, float soundtime /*= 0.0f*/ )
{
	if (shoot_type == SINGLE)
	{
		switch( m_iFireMode )
		{
		case FIREMODE_FULLAUTO:
			{
			BaseClass::WeaponSound(SINGLE, soundtime);
			}
			break;

		case FIREMODE_3RNDBURST:
			BaseClass::WeaponSound(WPN_DOUBLE, soundtime);//OverCharged
			if( m_iBurstSize == GetBurstSize() && m_iClip1 >= m_iBurstSize )
			{
				// First round of a burst, and enough bullets remaining in the clip to fire the whole burst
				BaseClass::WeaponSound( BURST, soundtime );
			}
			else if( m_iClip1 < m_iBurstSize )
			{
				//if (m_iClip1 <= GetMaxClip1() / 4 && m_iClip1 <= 8)
					//WeaponSound(LOW_AMMO, soundtime);
				// Not enough rounds remaining in the magazine to fire a burst, so play the gunshot
				// sounds individually as each round is fired.
				BaseClass::WeaponSound( SINGLE, soundtime );
			}
			break;
		}
		return;
	}

	BaseClass::WeaponSound( shoot_type, soundtime );
}

// BUGBUG: These need to be rethought
//-----------------------------------------------------------------------------
int CHLSelectFireMachineGun::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	if (m_iClip1 <=0)
	{
		return COND_NO_PRIMARY_AMMO;
	}
	else if ( flDist < m_fMinRange1) 
	{
		return COND_TOO_CLOSE_TO_ATTACK;
	}
	else if (flDist > m_fMaxRange1) 
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.5)	// UNDONE: Why check this here? Isn't the AI checking this already?
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
int CHLSelectFireMachineGun::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	return COND_NONE; // FIXME: disabled for now

	// m_iClip2 == -1 when no secondary clip is used
	if ( m_iClip2 == 0 && UsesSecondaryAmmo() )
	{
		return COND_NO_SECONDARY_AMMO;
	}
	else if ( flDist < m_fMinRange2 )
	{
		// Don't return	COND_TOO_CLOSE_TO_ATTACK only for primary attack
		return COND_NONE;
	}
	else if (flDist > m_fMaxRange2 )
	{
		// Don't return COND_TOO_FAR_TO_ATTACK only for primary attack
		return COND_NONE;
	}
	else if ( flDot < 0.5 ) // UNDONE: Why check this here? Isn't the AI checking this already?
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_RANGE_ATTACK2;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHLSelectFireMachineGun::CHLSelectFireMachineGun( void )
{
	m_fMinRange1	= 65;
	m_fMinRange2	= 65;
	m_fMaxRange1	= 1024;
	m_fMaxRange2	= 1024;
	m_iFireMode		= FIREMODE_FULLAUTO;
}
