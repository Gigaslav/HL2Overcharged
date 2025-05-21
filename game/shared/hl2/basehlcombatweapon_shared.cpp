//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon_shared.h"
#include "hl2_player_shared.h"
#include "in_buttons.h"
#include <algorithm>
#include <iterator>
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( basehlcombatweapon, CBaseHLCombatWeapon );

IMPLEMENT_NETWORKCLASS_ALIASED(BaseHLCombatWeapon, DT_BaseHLCombatWeapon)

BEGIN_NETWORK_TABLE(CBaseHLCombatWeapon, DT_BaseHLCombatWeapon)
#if !defined( CLIENT_DLL )
//	SendPropInt( SENDINFO( m_bReflectViewModelAnimations ), 1, SPROP_UNSIGNED ),
#else
//	RecvPropInt( RECVINFO( m_bReflectViewModelAnimations ) ),
#endif
END_NETWORK_TABLE()


#if !defined( CLIENT_DLL )

ConVar oc_ally_distance_to_lowing("oc_ally_distance_to_lowing", "0", FCVAR_REPLICATED);

#include "globalstate.h"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CBaseHLCombatWeapon )

	DEFINE_FIELD( m_bLowered,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flRaiseTime,		FIELD_TIME ),
	DEFINE_FIELD( m_iPrimaryAttacks,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAttacks,	FIELD_INTEGER ),

END_DATADESC()

#endif

BEGIN_PREDICTION_DATA( CBaseHLCombatWeapon )
END_PREDICTION_DATA()

ConVar sk_auto_reload_time( "sk_auto_reload_time", "3", FCVAR_REPLICATED );

/*ConVar oc_viewbob_shake_speed("oc_viewbob_shake_speed", "70");//320
ConVar oc_viewbob_shake_origin("oc_viewbob_shake_origin", "0.1f");//0.1f
ConVar oc_viewbob_shake_origin2("oc_viewbob_shake_origin2", "1.8f");//0.8f
ConVar oc_viewbob_shake_angles_roll("oc_viewbob_shake_angles_roll", "0.5f");//0.5f
ConVar oc_viewbob_shake_angles_pitch("oc_viewbob_shake_angles_pitch", "0.4f");//0.4f
ConVar oc_viewbob_shake_angles_yaw("oc_viewbob_shake_angles_yaw", "1.3f");//0.3f*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CBaseHLCombatWeapon::ItemHolsterFrame( void )
{
	BaseClass::ItemHolsterFrame();

	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Just load the clip with no animations
		FinishReload();
		m_flHolsterTime = gpGlobals->curtime;
	}
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::CanLower()
{
	if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) == ACTIVITY_NOT_AVAILABLE )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Drops the weapon into a lowered pose
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Lower( void )
{

	//Don't bother if we don't have the animation
	if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) == ACTIVITY_NOT_AVAILABLE )
		return false;

	m_bLowered = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Brings the weapon up to the ready position
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Ready( void )
{
	//Don't bother if we don't have the animation
	if ( SelectWeightedSequence( ACT_VM_LOWERED_TO_IDLE ) == ACTIVITY_NOT_AVAILABLE )
		return false;

	m_bLowered = false;	
	m_flRaiseTime = gpGlobals->curtime + 0.5f;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Deploy( void )
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	/*if ( GetOwner() && GetOwner()->IsPlayer() )
	{
		CHL2_Player *pPlayer = assert_cast<CHL2_Player*>( GetOwner() );
		if ( pPlayer->IsWeaponLowered() )
		{
			if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) != ACTIVITY_NOT_AVAILABLE )
			{
				if ( DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix() ) )
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack( gpGlobals->curtime + 1.0 );
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack	= gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}


	m_bLowered = false;*/
	return BaseClass::Deploy();
}
bool CBaseHLCombatWeapon::FirstDeploy(int iActivity)
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	/*if ( GetOwner() && GetOwner()->IsPlayer() )
	{
	CHL2_Player *pPlayer = assert_cast<CHL2_Player*>( GetOwner() );
	if ( pPlayer->IsWeaponLowered() )
	{
	if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) != ACTIVITY_NOT_AVAILABLE )
	{
	if ( DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix() ) )
	{
	m_bLowered = true;

	// Stomp the next attack time to fix the fact that the lower idles are long
	pPlayer->SetNextAttack( gpGlobals->curtime + 1.0 );
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
	m_flNextSecondaryAttack	= gpGlobals->curtime + 1.0;
	return true;
	}
	}
	}

	}


	m_bLowered = false;*/
	return BaseClass::FirstDeploy(iActivity);
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHLCombatWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( BaseClass::Holster( pSwitchingTo ) )
	{
		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CBaseHLCombatWeapon::WeaponShouldBeLowered( void )
{
#if !defined( CLIENT_DLL )

	if (cvar->FindVar("oc_ally_allow_friendlyfire")->GetInt())
		return false;

	if (GetOwner() && GetOwner()->IsPlayer() && GetOwner()->GetActiveWeapon())
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		trace_t tr;
		Vector forward;
		Vector Eye = pPlayer->EyePosition();
		pPlayer->EyeVectors(&forward);

		UTIL_TraceLine(Eye, Eye + forward * oc_ally_distance_to_lowing.GetFloat(), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
		//debugoverlay->AddLineOverlay(Eye, Eye + forward * MAX_TRACE_LENGTH, 255, 0, 0, false, 0.1f);

		if (tr.m_pEnt != NULL && tr.m_pEnt->IsNPC())
		{
			if (tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY || tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL || tr.m_pEnt->Classify() == CLASS_VORTIGAUNT)
			{
				GlobalEntity_SetState(MAKE_STRING("friendly_encounter"), GLOBAL_ON);
				return true;
			}
			else
			{
				GlobalEntity_SetState(MAKE_STRING("friendly_encounter"), GLOBAL_OFF);
				return false;
			}
		}
	}

#endif
	return false;
}*/



float	g_lateralBob;
float	g_verticalBob;
#if defined( CLIENT_DLL ) && ( !defined( HL2MP ) && !defined( PORTAL ) )

#define	HL2_BOB_CYCLE_MIN	1.0f
#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB			0.002f
#define	HL2_BOB_UP		0.5f


static ConVar	cl_bobcycle( "cl_bobcycle","0.8" );
static ConVar	cl_bob( "cl_bob","0.002" );
static ConVar	cl_bobup( "cl_bobup","0.5" );

// Register these cvars if needed for easy tweaking
static ConVar	v_iyaw_cycle( "v_iyaw_cycle", "2"/*, FCVAR_UNREGISTERED*/ );
static ConVar	v_iroll_cycle( "v_iroll_cycle", "0.5"/*, FCVAR_UNREGISTERED*/ );
static ConVar	v_ipitch_cycle( "v_ipitch_cycle", "1"/*, FCVAR_UNREGISTERED*/ );
static ConVar	v_iyaw_level( "v_iyaw_level", "0.3"/*, FCVAR_UNREGISTERED*/ );
static ConVar	v_iroll_level( "v_iroll_level", "0.1"/*, FCVAR_UNREGISTERED*/ );
static ConVar	v_ipitch_level( "v_ipitch_level", "0.3"/*, FCVAR_UNREGISTERED*/ );

ConVar oc_viewmodel_bob_style("oc_viewmodel_bob_style", "0", FCVAR_ARCHIVE, "0 - HL2 style, 1 - HL1 style, 2 - HL2-OVR style");

#if 0
static ConVar oc_bob_frequency("oc_bob_frequency", "42.0", FCVAR_ARCHIVE);
static ConVar oc_bob_run_frequency("oc_bob_run_frequency", "38.0", FCVAR_ARCHIVE);
static ConVar oc_bob_run_frequency_speed_approach("oc_bob_run_frequency_speed_approach", "1.0", FCVAR_ARCHIVE);
static ConVar oc_bob_run_frequency_down_speed_approach("oc_bob_run_frequency_down_speed_approach", "3.0", FCVAR_ARCHIVE);

static ConVar oc_bob_up("oc_bob_ang_up", "5.2", FCVAR_ARCHIVE);
static ConVar oc_bob_right("oc_bob_ang_right", "4.1", FCVAR_ARCHIVE);
static ConVar oc_bob_pos_up("oc_bob_pos_up", "-1.5", FCVAR_ARCHIVE);
static ConVar oc_bob_pos_right("oc_bob_pos_right", "2.5", FCVAR_ARCHIVE);
static ConVar oc_bob_pos_forward("oc_bob_pos_forward", "-0.5", FCVAR_ARCHIVE);


static ConVar oc_bob_run_up("oc_bob_run_ang_up", "5.2", FCVAR_ARCHIVE);
static ConVar oc_bob_run_right("oc_bob_run_ang_right", "4.1", FCVAR_ARCHIVE);
static ConVar oc_bob_run_pos_up("oc_bob_run_pos_up", "-1.5", FCVAR_ARCHIVE);
static ConVar oc_bob_run_pos_right("oc_bob_run_pos_right", "2.5", FCVAR_ARCHIVE);
static ConVar oc_bob_run_pos_forward("oc_bob_run_pos_forward", "-10.5", FCVAR_ARCHIVE);

float Fwd;
float Uc = 0;
bool InSpeed = false;

float f_oc_bob_up = 0.f;
float f_oc_bob_run_up = 0.f;

float f_oc_bob_right = 0.f;
float f_oc_bob_run_right = 0.f;

float f_oc_bob_pos_up = 0.f;
float f_oc_bob_run_pos_up = 0.f;

float f_oc_bob_pos_right = 0.f;
float f_oc_bob_run_pos_right = 0.f;

float bob_multiply_global = 0.f;
//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::CalcViewmodelBob( void )
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;

	CBasePlayer *player = ToBasePlayer( GetOwner() );
	if (!player || !player->GetActiveWeapon())
		return 0.f;

	if ( ( !gpGlobals->frametime ) || ( player == NULL ) )
	{
		return 0.0f;
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();
	speed = clamp(speed, -50, 50);//320

	float bob_frequency = oc_bob_run_frequency.GetFloat();//33
	float bob_frequency0 = oc_bob_frequency.GetFloat();//42

	float bob_multiply_run = 4.f;//42
	float bob_multiply = 0.8f;//33

	if (player->GetFlags() & FL_DUCKING)
		speed *= 0.3f;

	float SpeedRun;

	Vector velocity;

	if (player->GetFlags() & FL_ONGROUND)
	{
		player->EstimateAbsVelocity(velocity);
		SpeedRun = velocity.NormalizeInPlace();///Normalized
	}
	else
	{
		SpeedRun = cvar->FindVar("hl2_normspeed")->GetInt();
	}

	if (SpeedRun < cvar->FindVar("hl2_sprintspeed")->GetInt() - 30)
	{
		if (!InSpeed)
		{
			Uc = oc_bob_frequency.GetFloat();

			bob_multiply_global = bob_multiply;
		}
		else
		{
			Uc = Approach(bob_frequency0, Uc, oc_bob_run_frequency_down_speed_approach.GetFloat() * gpGlobals->frametime);

			bob_multiply_global = Approach(bob_multiply, bob_multiply_global, oc_bob_run_frequency_down_speed_approach.GetFloat() * gpGlobals->frametime);

			if (Uc == bob_frequency0)
				InSpeed = false;
		}

	}
	else if ((SpeedRun >= cvar->FindVar("hl2_sprintspeed")->GetInt() - 30) && (player->GetFlags() & FL_ONGROUND))
	{
		Uc = Approach(bob_frequency, Uc, oc_bob_run_frequency_speed_approach.GetFloat() * gpGlobals->frametime);

		bob_multiply_global = Approach(bob_multiply_run, bob_multiply_global, oc_bob_run_frequency_down_speed_approach.GetFloat() * gpGlobals->frametime);

		InSpeed = true;
	}
	//DevMsg("Uc: %.2f \n", Uc);

	float bob_offset = RemapVal(speed, 0, Uc, 0.0f, 1.0f);//320

	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;//0.007

	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX)*HL2_BOB_CYCLE_MAX;
	cycle /= HL2_BOB_CYCLE_MAX;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}
	
	g_verticalBob = speed*0.005f;

	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -9.0f, 6.0f );//-7.0f,4.0f

	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX*2)*HL2_BOB_CYCLE_MAX*2;
	cycle /= HL2_BOB_CYCLE_MAX*2;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );

	return SpeedRun;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles, float bobScale )
{
	CBasePlayer *player = ToBasePlayer(GetOwner());
	if (!player || !player->GetActiveWeapon())
		return;

	if (gpGlobals->frametime != 0.0f)
	{
		f_oc_bob_up = oc_bob_up.GetFloat() * bob_multiply_global;
		f_oc_bob_right = oc_bob_right.GetFloat() * bob_multiply_global;
		f_oc_bob_pos_right = oc_bob_pos_right.GetFloat() * bob_multiply_global;
		f_oc_bob_pos_up = oc_bob_pos_up.GetFloat() * bob_multiply_global;

		Vector	forward, right, Up;
		AngleVectors(angles, &forward, &right, &Up);

		/*float speed = Clamp( (*/CalcViewmodelBob()/** 0.01f), 0.0f, 1.17f)*/;

		float bob = Clamp(bobScale, 0.f, 1.f);

		g_verticalBob = g_verticalBob * bob;// *speed;
		g_lateralBob = g_lateralBob * bob;// *speed;

		Fwd = oc_bob_pos_forward.GetFloat();

		VectorMA(origin, g_verticalBob * Fwd, forward, origin);//0.1f

		origin[2] -= g_verticalBob * 0.1f;//0.1f

		// bob the angles
		angles[ROLL] += g_verticalBob * 5.5f;//0.5f//Z


		//angles[PITCH] += g_verticalBob * ((InSpeed) ? oc_bob_run_up.GetFloat() : oc_bob_up.GetFloat());//0.4f//UP//5.2

		//angles[PITCH] += g_verticalBob * ((InSpeed) ? Approach(bob_frequency, Uc, oc_bob_run_frequency_speed_approach.GetFloat() * gpGlobals->frametime) : Approach(bob_frequency, Uc, oc_bob_run_frequency_speed_approach.GetFloat() * gpGlobals->frametime);

		angles[PITCH] += g_verticalBob * f_oc_bob_up;

		angles[YAW] += g_lateralBob  * /*((InSpeed) ? oc_bob_run_right.GetFloat() : */f_oc_bob_right;//0.3f//RIGHT//4.1

		VectorMA(origin, g_lateralBob * /*((InSpeed) ? oc_bob_run_pos_right.GetFloat() : */f_oc_bob_pos_right, right, origin);//2.5f
		VectorMA(origin, g_lateralBob * /*((InSpeed) ? oc_bob_run_pos_up.GetFloat() : */f_oc_bob_pos_up, Up, origin);//-1.5f

	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::CalcViewmodelBob(void)
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;

	CBasePlayer *player = ToBasePlayer(GetOwner());
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ((!gpGlobals->frametime) || (player == NULL))
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	float multiplicator = (player->GetFlags() & FL_DUCKING) ? 2.5f : 1.95f;
	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D()*multiplicator;
	//DevMsg("speed: %.2f\n", speed);
	//DevMsg("multiplicator: %.2f\n", multiplicator);
	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	speed = clamp(speed, -490, 490);

	float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);

	bobtime += (gpGlobals->curtime - lastbobtime) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime / HL2_BOB_CYCLE_MAX)*HL2_BOB_CYCLE_MAX;
	cycle /= HL2_BOB_CYCLE_MAX;

	if (cycle < HL2_BOB_UP)
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - HL2_BOB_UP) / (1.0 - HL2_BOB_UP);
	}

	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp(g_verticalBob, -7.0f, 4.0f);

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime / HL2_BOB_CYCLE_MAX * 2)*HL2_BOB_CYCLE_MAX * 2;
	cycle /= HL2_BOB_CYCLE_MAX * 2;

	if (cycle < HL2_BOB_UP)
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - HL2_BOB_UP) / (1.0 - HL2_BOB_UP);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp(g_lateralBob, -7.0f, 4.0f);

	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles, float bobScale)
{
	CBasePlayer *player = ToBasePlayer(GetOwner());
	if (!player || !player->GetActiveWeapon())
		return;

	if (gpGlobals->frametime != 0.0f)
	{
		Vector	forward, right;
		AngleVectors(angles, &forward, &right, NULL);

		float bob = Clamp(bobScale, 0.f, 1.f);

		CalcViewmodelBob();

		switch (oc_viewmodel_bob_style.GetInt())
		{
			case 0:
			{
				// Apply bob, but scaled down to 40%
				VectorMA(origin, g_verticalBob * 0.1f * bob, forward, origin);

				// Z bob a bit more
				origin[2] += g_verticalBob * 0.1f * bob;

				VectorMA(origin, g_lateralBob * 0.15f * bob, right, origin);
			}
			break;
			case 1:
			{
				VectorMA(origin, g_lateralBob * 0.9f * bob, -forward, origin);
			}
			break;
			case 2:
			{
				// Apply bob, but scaled down to 40%
				VectorMA(origin, g_verticalBob * 0.1f * bob, forward, origin);

				// Z bob a bit more
				origin[2] += g_verticalBob * 0.1f * bob;

				// bob the angles
				angles[ROLL] += g_verticalBob * 0.4f * bob;
				angles[PITCH] -= g_verticalBob * 0.3f * bob;

				angles[YAW] -= g_lateralBob  * 0.2f * bob;

				VectorMA(origin, g_lateralBob * 0.15f * bob, right, origin);
			}
			break;
			default:
				break;
		}
	}
}
//-----------------------------------------------------------------------------
Vector CBaseHLCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	return BaseClass::GetBulletSpread( proficiency );
}

//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	return BaseClass::GetSpreadBias( proficiency );
}
//-----------------------------------------------------------------------------

const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetProficiencyValues()
{
	return NULL;
}

#else

// Server stubs
float CBaseHLCombatWeapon::CalcViewmodelBob( void )
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHLCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles, float bobScale)
{
}

//-----------------------------------------------------------------------------
Vector CBaseHLCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	Vector baseSpread = BaseClass::GetBulletSpread( proficiency );

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[ proficiency ].spreadscale;
	return ( baseSpread * flModifier );
}

//-----------------------------------------------------------------------------
float CBaseHLCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	return (pProficiencyValues)[ proficiency ].bias;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetProficiencyValues()
{
	return GetDefaultProficiencyValues();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseHLCombatWeapon::GetDefaultProficiencyValues()
{
	// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
	static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
	{
		{ 2.50, 1.0	},
		{ 2.00, 1.0	},
		{ 1.50, 1.0	},
		{ 1.25, 1.0 },
		{ 1.00, 1.0	},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return g_BaseWeaponProficiencyTable;
}

#endif