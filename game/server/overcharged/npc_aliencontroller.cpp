//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	Alien Controllers from HL1 now in updated form
//
//
//=============================================================================//


#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_basenpc.h"
#include "ai_basenpc_physicsflyer.h"
#include "npcevent.h"
#include "ai_basenpc_physicsflyer.h"
#include "soundenvelope.h"
#include "ai_hint.h"
#include "ai_route.h"
#include "ai_moveprobe.h"
#include "ai_squad.h"
#include "ai_network.h"

#include "scripted.h"

#include "effect_dispatch_data.h" //muzzle flash

#include "prop_combine_ball.h"

#include "decals.h"

#include "particle_parse.h"
#include "particle_system.h"

#include "npc_manhack.h"

#include "weapon_fireball.h"
#include "npc_aliencontroller.h"
#include "hl2_shareddefs.h"

#include "BloodDripsGreen.h"
//#include "Human_Error/hlss_combine_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar g_debug_aliencontroller("oc_aliencontroller_debug","0");

ConVar sk_aliencontroller_health( "sk_aliencontroller_health", "0");
ConVar sk_aliencontroller_dmg_claw( "sk_aliencontroller_dmg_claw", "0");

ConVar aliencontroller_max_enemy_distance( "oc_aliencontroller_max_enemy_distance", "1024");
ConVar aliencontroller_min_enemy_distance( "oc_aliencontroller_min_enemy_distance", "256");
ConVar aliencontroller_inf_enemy_distance( "oc_aliencontroller_inf_enemy_distance", "96");
ConVar aliencontroller_preferred_height("oc_aliencontroller_preferred_height","256");


ConVar controller_avoid_dist("oc_controller_avoid_dist","100");
//ConVar aliencontroller_fly_speed("oc_aliencontroller_fly_speed","50");

//#define CONTROLLER_FLYSPEED 50 //aliencontroller_fly_speed.GetFloat()
#define CONTROLLER_FLYSPEED_MAX 320//320
#define CONTROLLER_FLYSPEED_MIN 100//100

#define CONTROLLER_THROW_SPEED 2000

#define CONTROLLER_TOO_CLOSE_TO_ATTACK 50

#define CONTROLLER_OFFSET_SWITCH_DELAY random->RandomInt(4,7)

static const char *ACONTROLLER_ATTACK = "ACONTROLLER_ATTACK";
static const char *ACONTROLLER_ALERT  = "ACONTROLLER_ALERT";
static const char *ACONTROLLER_IDLE   = "ACONTROLLER_IDLE";

#define SOUND_ALIENCONTROLLER_PAIN "NPC_AlienController.Vortigese"
#define SOUND_ALIENCONTROLLER_DIE "NPC_AlienController.Die"
#define HLSS_CONTROLLER_TELEKINESIS 1 // Light Kill : Enable telekinesis for Alien Controller

int AE_CONTROLLER_FIREGLOW;
int AE_CONTROLLER_FIREGLOW_STOP;
int AE_CONTROLLER_FIREBALL;
int AE_CONTROLLER_LAND;
int AE_CONTROLLER_START_LANDING;
int AE_CONTROLLER_FLY;
int AE_CONTROLLER_START_FLYING;
int AE_CONTROLLER_HAND;
int AE_CONTROLLER_SWING_SOUND;


#define HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT	HL2COLLISION_GROUP_GUNSHIP
#define HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_ATTACK		COLLISION_GROUP_NPC

#ifdef HLSS_CONTROLLER_TELEKINESIS
Activity ACT_CONTROLLER_TELEKINESIS_STAND_START;
Activity ACT_CONTROLLER_TELEKINESIS_STAND_STOP;
Activity ACT_CONTROLLER_TELEKINESIS_STAND_LOOP;
Activity ACT_CONTROLLER_TELEKINESIS_FLY_START;
Activity ACT_CONTROLLER_TELEKINESIS_FLY_STOP;
Activity ACT_CONTROLLER_TELEKINESIS_FLY_LOOP;
#endif

Activity ACT_CONTROLLER_LAND;
Activity ACT_CONTROLLER_LIFTOFF;


BEGIN_DATADESC( CNPC_AlienController )

	DEFINE_FIELD( m_iFireBallAttachment,		FIELD_INTEGER),
	DEFINE_FIELD( m_iBrainsAttachment,			FIELD_INTEGER),

	DEFINE_FIELD( m_hFireBallTarget,			FIELD_EHANDLE),

	DEFINE_FIELD( m_iLastOffsetAngle,			FIELD_INTEGER),
	DEFINE_FIELD( m_fLastOffsetSwithcTime,		FIELD_TIME),
	DEFINE_KEYFIELD( m_flPreferredHeight,		FIELD_FLOAT, "preferred_height"),
	DEFINE_FIELD( m_vecPreferedAttackPoint,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD(m_flNextPreferredAttackTime,	FIELD_TIME ),

	DEFINE_FIELD( m_flNextAttackTime,			FIELD_TIME ),
	DEFINE_FIELD( m_iNumberOfAttacks,			FIELD_INTEGER ),

	DEFINE_FIELD (m_flNextPainSoundTime,		FIELD_TIME ),

#ifdef HLSS_CONTROLLER_TELEKINESIS
	DEFINE_ARRAY( m_hPhysicsEnt,				FIELD_EHANDLE,			ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS),
	DEFINE_ARRAY( m_vecPhysicsEntOrigin,		FIELD_POSITION_VECTOR,	ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS),
	DEFINE_FIELD( m_iNumberOfPhysiscsEnts,		FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextTelekinesisThrow,		FIELD_TIME ),
	DEFINE_FIELD( m_flNextTelekinesisScan,		FIELD_TIME ),
	DEFINE_FIELD( m_flNextTelekinesisCancel,	FIELD_TIME ),
#endif

	DEFINE_KEYFIELD( m_bIsFlying,				FIELD_BOOLEAN,		"landed" ),
	DEFINE_KEYFIELD( m_bCanLand,				FIELD_BOOLEAN,		"canland" ),
	DEFINE_FIELD( m_bIsLanding,					FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextLandingTime,			FIELD_TIME ),
	DEFINE_FIELD( m_vecLandPosition,			FIELD_POSITION_VECTOR ),
//	DEFINE_FIELD( m_vLastFlyPosition,			FIELD_POSITION_VECTOR ),

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	DEFINE_FIELD( m_flNextCombineBallScan,		FIELD_TIME ),
	DEFINE_FIELD( m_hCombineBall,				FIELD_EHANDLE ),
#endif

	DEFINE_FIELD( m_vOldGoal,					FIELD_POSITION_VECTOR ),

	DEFINE_FIELD( m_bFireballEffects,			FIELD_BOOLEAN ),

	DEFINE_OUTPUT( m_OnLand,		"OnLand" ),
	DEFINE_OUTPUT( m_OnLiftOff,		"OnLiftOff" ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableLanding",			InputEnableLanding),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableLanding",			InputDisableLanding ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StopScriptingAndGesture",	InputStopScriptingAndGesture ),

END_DATADESC()


LINK_ENTITY_TO_CLASS(npc_aliencontroller, CNPC_AlienController);
LINK_ENTITY_TO_CLASS(npc_advisor_fighter, CNPC_AlienController);

IMPLEMENT_SERVERCLASS_ST( CNPC_AlienController, DT_NPC_AlienController )
	SendPropBool( SENDINFO( m_bFireballEffects )),
#ifdef HLSS_CONTROLLER_TELEKINESIS
	SendPropArray3
	(
		SENDINFO_ARRAY3(m_hPhysicsEnt), 
		SendPropEHandle( SENDINFO_ARRAY(m_hPhysicsEnt), SPROP_CHANGES_OFTEN)
	),
#endif
END_SEND_TABLE()


void CNPC_AlienController::PainSound ( const CTakeDamageInfo &info )
{
	if (m_flNextPainSoundTime < gpGlobals->curtime)
	{
		m_flNextPainSoundTime = gpGlobals->curtime + 5.0f;
		EmitSound( SOUND_ALIENCONTROLLER_PAIN );
	}
}

void CNPC_AlienController::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( SOUND_ALIENCONTROLLER_DIE  );
}

void CNPC_AlienController::IdleSound( void )
{
	Speak( ACONTROLLER_IDLE );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_AlienController::AttackSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		Speak( ACONTROLLER_ATTACK );
	}
}

void CNPC_AlienController::AlertSound( void )
{
	if ( GetEnemy() != NULL && IsOkToCombatSpeak() )
	{
		Speak( ACONTROLLER_ALERT );
	}
}

CNPC_AlienController::CNPC_AlienController()
{
	m_iFireBallAttachment  = -1;
	m_iBrainsAttachment	   = -1;
	//m_iFireBallFadeIn	  = 0;
	//m_fFireBallFadeInTime = 0.f;

	m_bIsFlying = false;
	m_bCanLand = true;

	m_flPreferredHeight = -1;

	//m_iNodeToLand = -1;

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	m_flNextCombineBallScan = 0;
#endif
}

void CNPC_AlienController::Spawn(void)
{
	Precache();

	if (FClassnameIs(this, "npc_advisor_fighter"))
	{
		SetModel("models/Advisor.mdl");
	}
	else
	SetModel( "models/Controller.mdl" );

	BaseClass::Spawn();

	//SetHullType( HULL_HUMAN_CENTERED ); //HULL_WIDE_HUMAN SetHullType( HULL_HUMAN_CENTERED );
	SetHullType( HULL_LARGE_CENTERED ); // Light Kill : Trying to fix mistake with npc integration.
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetCollisionBounds( ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS );

	//TERO: so that the Controllers wont collide with each other and block each others fly path
	SetCollisionGroup( HL2COLLISION_GROUP_GUNSHIP );

	AddSolidFlags( FSOLID_NOT_STANDABLE );
	
	CapabilitiesClear();

	CapabilitiesAdd ( bits_CAP_SQUAD | bits_CAP_TURN_HEAD ); // bits_CAP_ANIMATEDFACE |
	CapabilitiesAdd	( bits_CAP_INNATE_RANGE_ATTACK1 );
	CapabilitiesAdd	( bits_CAP_INNATE_MELEE_ATTACK1 );
	CapabilitiesAdd	( bits_CAP_DOORS_GROUP );
	CapabilitiesAdd	( bits_CAP_MOVE_SHOOT );

	CapabilitiesAdd( bits_CAP_NO_HIT_SQUADMATES | bits_CAP_FRIENDLY_DMG_IMMUNE );

	//TERO: the rest of the capabilities are set here
	Land(!m_bIsFlying);


	AddEFlags( EFL_NO_DISSOLVE );

	m_bloodColor		= BLOOD_COLOR_GREEN;
	m_iHealth			= sk_aliencontroller_health.GetFloat();

	m_flFieldOfView		= VIEW_FIELD_FULL;

	AddSpawnFlags( SF_NPC_LONG_RANGE );

	//m_NPCState			= NPC_STATE_NONE;

	m_iFireBallAttachment   = LookupAttachment("FireBallGlow");
	m_iBrainsAttachment		= LookupAttachment("Brains");

	//CreateFireGlow();

	m_hFireBallTarget = NULL;

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	m_hCombineBall	  = NULL;
#endif

	m_iLastOffsetAngle = random->RandomInt( 30, 75 );
	m_iLastOffsetAngle = m_iLastOffsetAngle * ((-1)^(random->RandomInt(1,2)));
	m_fLastOffsetSwithcTime = 0;

	m_vecPreferedAttackPoint = GetAbsOrigin();
	m_flNextPreferredAttackTime = 0;
	if (m_flPreferredHeight < 0)
		m_flPreferredHeight = aliencontroller_preferred_height.GetFloat();

	m_bIsLanding			= false;
	m_flNextLandingTime		= 0;

	m_flNextAttackTime = 0;
	m_iNumberOfAttacks = random->RandomInt(3,6);

	m_vOldGoal = vec3_origin;

//	m_vLastFlyPosition = GetAbsOrigin();


	//TERO: sounds

	m_flNextPainSoundTime = 0;

#ifdef HLSS_CONTROLLER_TELEKINESIS
	//TELEKINESIS
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		m_hPhysicsEnt.Set( i, NULL );// = NULL;
	}
	m_iNumberOfPhysiscsEnts = 0;

	m_flNextTelekinesisThrow = 0;
	m_flNextTelekinesisScan = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_INITIAL_SCAN_DELAY;
	m_flNextTelekinesisCancel = 0;
#endif

	NPCInit();

	/*if (CheckLanding() )
	{
		SetCondition(COND_ALIENCONTROLLER_SHOULD_LAND);
		m_flNextLandingTime = 0; //gpGlobals->curtime + 3;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*bool CNPC_AlienController::FValidateHintType( CAI_Hint *pHint )
{
	//BaseClass::FValidateHintType( pHint );
	return true; //( pHint->HintType() == HINT_CROW_FLYTO_POINT );
}*/


void CNPC_AlienController::Precache(void)
{
	PrecacheModel( "models/Controller.mdl" );
	PrecacheModel("models/Advisor.mdl");

	
	//PrecacheModel( CONTROLLER_FIRE_SPRITE );
	PrecacheParticleSystem( "controller_fireball" );

	//TERO: testing this shit, remove later
	PrecacheParticleSystem( "controller_telekinesis" );

	PrecacheScriptSound( "NPC_Vortigaunt.FootstepLeft" );
	PrecacheScriptSound( "NPC_Vortigaunt.FootstepRight" );
	PrecacheScriptSound( "NPC_Vortigaunt.Claw" );
	PrecacheScriptSound( "NPC_Vortigaunt.Swing" );
	PrecacheScriptSound( "NPC_Vortigaunt.StartHealLoop" );

	PrecacheScriptSound( SOUND_ALIENCONTROLLER_PAIN );
	PrecacheScriptSound( SOUND_ALIENCONTROLLER_DIE );
	
	UTIL_PrecacheOther( "fireball_missile" ); 

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CNPC_AlienController::Activate()
{
	BaseClass::Activate();

}


#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
void CNPC_AlienController::CombineBallCheck( void )
{
	CSound *pSound = GetLoudestSoundOfType( SOUND_DANGER );

	if (pSound && pSound->m_hOwner && FClassnameIs( pSound->m_hOwner, "prop_combine_ball" ) )
	{
		m_flNextCombineBallScan = gpGlobals->curtime + 4.0f;

//		DevMsg("ALIENCONTROLLER: We have found a combine energy ball\n");
		CPropCombineBall *pBally = dynamic_cast<CPropCombineBall*>((CBaseEntity*)pSound->m_hOwner);

		if (pBally && pBally->VPhysicsGetObject() )
		{
			Vector forward;
			GetVectors(&forward, NULL, NULL);

			Vector vecPosition, vecDirection;
			pBally->VPhysicsGetObject()->GetPosition( &vecPosition, NULL );
			vecDirection = vecPosition - GetAbsOrigin();
			//pBally->VPhysicsGetObject()->GetVelocity( &vecVelocity, NULL );

			if (DotProduct( forward, vecDirection ) > 0.45)
			{
				//TERO: this is all the stuff we need to change in order for the Bally to behave nicely
				pBally->SetOwnerEntity( this );
				pBally->StartLifetime( 2.0f );
				pBally->SetWeaponLaunched( true );
				pBally->SetCollisionGroup( HL2COLLISION_GROUP_COMBINE_BALL_NPC );

				PhysSetGameFlags( pBally->VPhysicsGetObject(), FVPHYSICS_NO_NPC_IMPACT_DMG );
				PhysClearGameFlags( pBally->VPhysicsGetObject(), FVPHYSICS_DMG_DISSOLVE | FVPHYSICS_HEAVY_OBJECT );
				
				m_hCombineBall = pBally;

				m_flNextCombineBallScan = gpGlobals->curtime + 0.5f;

				if (GetEnemy() && HasCondition( COND_SEE_ENEMY ))
				{
					//TERO: setting speed for the Combine Ball
					vecDirection = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
					VectorNormalize(vecDirection);
					vecDirection *= 0.5;
					pBally->VPhysicsGetObject()->SetVelocity( &vecDirection, NULL );
				} else
				{
					pBally->VPhysicsGetObject()->GetVelocity( &vecDirection, NULL );
					VectorNormalize(vecDirection);
					vecDirection *= -0.5;
					pBally->VPhysicsGetObject()->SetVelocity( &vecDirection, NULL );
				}
			}
		}
	}
}
#endif

void CNPC_AlienController::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

#ifdef HLSS_USE_COMBINE_BALL_DEFENSE
	if (m_hCombineBall)
	{
		if ( m_flNextCombineBallScan < gpGlobals->curtime )
		{
			m_flNextCombineBallScan = gpGlobals->curtime + 4.0f;

			CPropCombineBall *pBally = dynamic_cast<CPropCombineBall*>((CBaseEntity*)m_hCombineBall);

			if (pBally && pBally->VPhysicsGetObject())
			{
				if (GetEnemy() && HasCondition(COND_SEE_ENEMY) )
				{
					Vector vecEnemy = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
					VectorNormalize( vecEnemy );
					vecEnemy *= 1000.0f;
					pBally->VPhysicsGetObject()->SetVelocity( &vecEnemy, NULL );
				} else
				{
					Vector vecDirection;
					pBally->VPhysicsGetObject()->GetVelocity( &vecDirection, NULL );
					VectorNormalize( vecDirection );
					vecDirection *= 1000.0f;
					pBally->VPhysicsGetObject()->SetVelocity( &vecDirection, NULL );
				}
			}

			m_hCombineBall = NULL;
		}
	}
	else if ( m_flNextCombineBallScan < gpGlobals->curtime ) //HasCondition(COND_HEAR_DANGER) &&
	{
		CombineBallCheck();
	}
#endif

#ifdef CONTROLLER_PATH_FINDING_DEBUG_MESSAGES
	if (IsCurSchedule(SCHED_FALL_TO_GROUND) && m_bIsFlying)
		DevMsg("npc_aliencontroller: WARNING! WARNING! WARNING! SCHED_FALL_TO_GROUND while flying!\n");

	//DevMsg("npc_aliencontroller: running activity with id: %d\n", GetActivity());
#endif


	//TERO: this is to make sure we stop the fireball particles when there's no attack anymore
	if (!IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) &&
		!IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK2 ) &&
		m_flNextAttackTime < gpGlobals->curtime && m_flNextAttackTime != 0)
	{
		m_flNextAttackTime = 0;
		//ParticleMessages(MESSAGE_STOP_FIREBALL);
		m_bFireballEffects = false;
	}

	if ( IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 )  &&
		!IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK2 ) &&
		 m_flNextAttackTime < gpGlobals->curtime )
	{
		RemoveGesture( ACT_GESTURE_RANGE_ATTACK1 );
		AddGesture( ACT_GESTURE_RANGE_ATTACK2, true );

		//ParticleMessages(MESSAGE_STOP_FIREBALL);
		m_bFireballEffects = false;
		ShootFireBall();

		m_iNumberOfAttacks--;
		if (m_iNumberOfAttacks <= 0)
		{
			m_iNumberOfAttacks = random->RandomInt(3,6);
			m_flNextAttackTime = gpGlobals->curtime + random->RandomFloat( 3.5, 4.5 );
		}
		else
		{
			m_flNextAttackTime = gpGlobals->curtime + 2.0f;
		}
	}
	else if (!HasCondition( COND_CAN_MELEE_ATTACK1 ) &&
#ifdef HLSS_CONTROLLER_TELEKINESIS
		!IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS) &&
#endif
		!HasCondition( COND_ALIENCONTROLLER_SHOULD_LAND ) &&
		 HasCondition( COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL ) && 
	    !IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK2 ) &&
		!IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) )
	{
		int iLayer = AddGesture( ACT_GESTURE_RANGE_ATTACK1, false );
		m_flNextAttackTime = gpGlobals->curtime + GetLayerDuration( iLayer );

		ClearCondition( COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL );

		//ParticleMessages(MESSAGE_START_FIREBALL);
		m_bFireballEffects = true;
	}

	UpdateHead();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_AlienController::UpdateHead( void )
{
	float yaw = GetPoseParameter( "head_yaw" );
	float pitch = GetPoseParameter( "head_pitch" );

	Vector vecTarget = vec3_origin;

	/*CBaseEntity *pTarget = GetEnemy();

	if (!pTarget)
		pTarget = GetTarget();*/

	if (GetEnemy())
		vecTarget = GetEnemy()->WorldSpaceCenter();
	else if (GetNavigator() && GetNavigator()->IsGoalActive())
		vecTarget = GetNavigator()->GetCurWaypointPos() + CONTROLLER_BODY_CENTER;

	// If we should be watching our enemy, turn our head
	if ( vecTarget != vec3_origin ) // ( pTarget != NULL ) )
	{
		Vector	enemyDir =  vecTarget - WorldSpaceCenter(); //pTarget->WorldSpaceCenter()
		VectorNormalize( enemyDir );
		
		float angle = VecToYaw( BodyDirection3D() );
		float angleDiff = VecToYaw( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + yaw );

		SetPoseParameter( "head_yaw", UTIL_Approach( yaw + angleDiff, yaw, 50 ) );

		angle = UTIL_VecToPitch( BodyDirection3D() );
		angleDiff = UTIL_VecToPitch( enemyDir );
		angleDiff = UTIL_AngleDiff( angleDiff, angle + pitch );

		SetPoseParameter( "head_pitch", UTIL_Approach( pitch + angleDiff, pitch, 50 ) );
	}
	else
	{
		// Otherwise turn the head back to its normal position
		SetPoseParameter( "head_yaw",	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( "head_pitch", UTIL_Approach( 0, pitch, 10 ) );
	}

#ifdef HLSS_CONTROLLER_TELEKINESIS
	float poseHead = GetPoseParameter( "head_open" );

	//TERO: in case something bad happened, and the physics ent is gone, we need to close our headszzz

	if ( IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS))
	{
		SetPoseParameter( "head_open", UTIL_Approach( 20, poseHead, 10 ));
	}
	else
	{
		SetPoseParameter( "head_open", UTIL_Approach( 0, poseHead, 10 ));
	}
#endif

}

void CNPC_AlienController::OnRestore()
{
	BaseClass::OnRestore();
}

void CNPC_AlienController::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_CONTROLLER_FIREGLOW )
	{
		//ParticleMessages(MESSAGE_START_FIREBALL);
		m_bFireballEffects = true;
		return;
	}

	if ( pEvent->event == AE_CONTROLLER_FIREGLOW_STOP )
	{
		//ParticleMessages(MESSAGE_STOP_FIREBALL);
		m_bFireballEffects = false;
		return;
	}

	if ( pEvent->event == AE_CONTROLLER_FIREBALL )
	{

		//ParticleMessages(MESSAGE_STOP_FIREBALL);
		m_bFireballEffects = false;

		ShootFireBall();
		return;
	}

	if ( pEvent->event == AE_CONTROLLER_START_LANDING )
	{
		if (m_bIsFlying)
			Land(true);

		return;
	}

	if ( pEvent->event == AE_CONTROLLER_LAND )
	{
		m_bIsFlying = false;
		m_bIsLanding = false;

		ClearCondition(COND_ALIENCONTROLLER_SHOULD_LAND);

		m_OnLand.FireOutput( this, this );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("Succesfully landed\n");
#endif
		return;
	}

	if ( pEvent->event == AE_CONTROLLER_START_FLYING )
	{
		if (!m_bIsFlying)
			Land(false);

		return;
	}

	if ( pEvent->event == AE_CONTROLLER_FLY )
	{

		m_bIsFlying = true;
		m_bIsLanding = false;

		ClearCondition(COND_ALIENCONTROLLER_SHOULD_LAND);

		m_OnLiftOff.FireOutput( this, this );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("Seccesfully lifted off\n");
#endif
		return;
	}

	if ( pEvent->event == AE_CONTROLLER_HAND )
	{
		Claw();

		return;
	}

	if ( pEvent->event == AE_CONTROLLER_SWING_SOUND )
	{
		EmitSound( "NPC_Vortigaunt.Swing" );	
		return;
	}

	if ( pEvent->event == AE_NPC_LEFTFOOT )
	{
		EmitSound( "NPC_Vortigaunt.FootstepLeft", pEvent->eventtime );
		return;
	}

	if ( pEvent->event == AE_NPC_RIGHTFOOT )
	{
		EmitSound( "NPC_Vortigaunt.FootstepRight", pEvent->eventtime );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}


void CNPC_AlienController::Claw()
{
	CBaseEntity *pHurt = NULL;
	if (m_bIsFlying)
	{
		pHurt = CheckTraceHullAttack( 50, Vector(-10,-10,-60), Vector(10,10,-10),sk_aliencontroller_dmg_claw.GetFloat(), DMG_SLASH );
	}
	else
	{
		pHurt = CheckTraceHullAttack( 50, Vector(-10,-10,-20), Vector(10,10,20),sk_aliencontroller_dmg_claw.GetFloat(), DMG_SLASH );
	}

	if ( pHurt )
	{
		pHurt->ViewPunch( QAngle(5,0,-18) );
		// Play a random attack hit sound
		EmitSound( "NPC_Vortigaunt.Claw" );
	}
}

int CNPC_AlienController::GetSoundInterests( void )
{
	return	SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER;
}


int CNPC_AlienController::SelectSchedule( void )
{
	if ( IsInAScript() )
	{
		//m_bWasInAScript = true;
		return BaseClass::SelectSchedule();
	}

	if (HasCondition(COND_ALIENCONTROLLER_SHOULD_LAND))
	{
		return SCHED_ALIENCONTROLLER_LAND;
	}

#ifdef HLSS_CONTROLLER_TELEKINESIS
	if ( m_iNumberOfPhysiscsEnts > 0 && HasCondition( COND_SEE_ENEMY) )
	{
		m_iNumberOfPhysiscsEnts = 0;

		for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
		{
			if (m_hPhysicsEnt[i])
			{
				m_iNumberOfPhysiscsEnts++;
			}
		}

		if ( m_iNumberOfPhysiscsEnts > 0)
		{
			//DevMsg("npc_aliencontroller: we have %d physics objects, selecting telekinesis\n", m_iNumberOfPhysiscsEnts);
			m_flNextTelekinesisThrow  = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_INITIAL_DELAY;
			m_flNextTelekinesisCancel = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_INITIAL_CANCEL_DELAY;
			//ParticleMessages(MESSAGE_STOP_FIREBALL);
			m_bFireballEffects = false;
			StartObjectSounds();
			return SCHED_ALIENCONTROLLER_TELEKINESIS;
		}
	}
#endif

	if ( HasCondition( COND_NEW_ENEMY ) )
	{
		m_flNextPreferredAttackTime = 0;
	}

	if ( m_NPCState == NPC_STATE_COMBAT && GetEnemy())
	{
		if ( BehaviorSelectSchedule() )
			return BaseClass::SelectSchedule();

		// dead enemy
		if ( HasCondition( COND_ENEMY_DEAD ) )
		{
			// call base class, all code to handle dead enemies is centralized there.
			return BaseClass::SelectSchedule();
		}

		if (!m_bIsFlying &&
			 HasCondition( COND_SEE_ENEMY ) && 
			!HasCondition( COND_CAN_RANGE_ATTACK1 ) && 
			!HasCondition( COND_CAN_MELEE_ATTACK1 ) && 
			!HasCondition( COND_CAN_MELEE_ATTACK2 ) )
		{
			if ( HasCondition( COND_TOO_FAR_TO_ATTACK )  && OccupyStrategySlotRange( SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2 )) //|| IsUsingTacticalVariant(TACTICAL_VARIANT_PRESSURE_ENEMY) )
			{
				return SCHED_ALIENCONTROLLER_PRESS_ATTACK;
			}
		}

		if (m_bIsFlying && HasCondition( COND_SEE_ENEMY ))
		{
			return SCHED_ALIENCONTROLLER_LEVITATE; //SCHED_IDLE_STAND; //SCHED_ALIENCONTROLLER_LEVITATE;
		}
	}


	//if (HasCondition(COND_ALIENCONTROLLER_FLY_BLOCKED))
	//	return SCHED_CHASE_ENEMY;

	return BaseClass::SelectSchedule();
}

void CNPC_AlienController::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask)
	{
	case TASK_ALIENCONTROLLER_LAND:
		{
			RemoveAllGestures();	//TERO: I am not sure if this is really that smart thing to do

			if (m_bIsFlying)
			{
				SetActivity( (Activity) ACT_CONTROLLER_LAND );	//SetActivity is better than SetIdealActivity( because otherwise it might way when not funny
			}
			else
			{
				SetActivity( (Activity) ACT_CONTROLLER_LIFTOFF );
			}
		}
		break;
#ifdef HLSS_CONTROLLER_TELEKINESIS
	case TASK_ALIENCONTROLLER_TELEKINESIS:
		{
			m_flNextTelekinesisThrow  = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_INITIAL_DELAY;
			m_flNextTelekinesisCancel = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_INITIAL_CANCEL_DELAY;

			if (m_bIsFlying)
			{
				SetActivity(ACT_CONTROLLER_TELEKINESIS_FLY_START);
			}
			else
			{
				SetActivity(ACT_CONTROLLER_TELEKINESIS_STAND_START);
			}
		}
		break;
#endif
	case TASK_ALIENCONTROLLER_FIREBALL_EXTINGUISH:
		{
			m_bFireballEffects = false;

			TaskComplete();
		}
		break;
	default:
		{
			BaseClass::StartTask( pTask );
		}
		break;
	}

}

void CNPC_AlienController::RunTask( const Task_t *pTask )
{

	switch ( pTask->iTask )
	{
		case TASK_ALIENCONTROLLER_LAND:
			{
				if ( IsActivityFinished() )	//TERO: added recently, a bug?
				{
					if (!m_bIsLanding)
					{
						TaskComplete();
					}
				}
			}
			break;
#ifdef HLSS_CONTROLLER_TELEKINESIS
		case TASK_ALIENCONTROLLER_TELEKINESIS:
			{
				Activity iActivity = GetActivity();

				if (iActivity == ACT_CONTROLLER_TELEKINESIS_STAND_START ||
					iActivity == ACT_CONTROLLER_TELEKINESIS_FLY_START)
				{
					if( IsActivityFinished() )
					{
						if (m_bIsFlying)
						{
							SetActivity(ACT_CONTROLLER_TELEKINESIS_FLY_LOOP);
						}
						else
						{
							SetActivity(ACT_CONTROLLER_TELEKINESIS_STAND_LOOP);
						}
					}
				}
				else if (iActivity == ACT_CONTROLLER_TELEKINESIS_STAND_STOP ||
						 iActivity == ACT_CONTROLLER_TELEKINESIS_FLY_STOP )
				{
					if( IsActivityFinished() )
					{
						TaskComplete();
						
					}
				}
				else if (ACT_CONTROLLER_TELEKINESIS_STAND_LOOP ||
						 ACT_CONTROLLER_TELEKINESIS_FLY_LOOP)
				{
					if (m_iNumberOfPhysiscsEnts <= 0 || !GetEnemy())
					{
						if (m_bIsFlying)
						{
							SetActivity(ACT_CONTROLLER_TELEKINESIS_FLY_STOP);
						}
						else
						{
							SetActivity(ACT_CONTROLLER_TELEKINESIS_STAND_STOP);
						}
					}
				}
				else
				{
					if (m_bIsFlying)
					{
						SetActivity(ACT_CONTROLLER_TELEKINESIS_FLY_START);
					}
					else
					{
						SetActivity(ACT_CONTROLLER_TELEKINESIS_STAND_START);
					}
				}
			}
			break;
#endif
		case TASK_ALIENCONTROLLER_FIREBALL_EXTINGUISH:
			{
				TaskComplete();
			}
			break;
		default:
			{
				BaseClass::RunTask( pTask );
			}
			break;
	}
}

//////////////////OVR/////////////////////////////
//-----------------------------------------------------------------------------
// Don't become a ragdoll until we've finished our death anim
//-----------------------------------------------------------------------------
bool CNPC_AlienController::CanBecomeRagdoll(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;
	return (info.GetDamageType() & (DMG_BULLET) ||
		IsCurSchedule(SCHED_DIE, false) ||								// Finished playing death anim, time to ragdoll
		IsCurSchedule(SCHED_SCRIPTED_RUN, false) ||
		(GetActivity() == ACT_WALK) || (GetActivity() == ACT_RUN) ||
		GetCurSchedule() == NULL);
}


//-----------------------------------------------------------------------------
// Determines the best type of death anim to play based on how we died.
//-----------------------------------------------------------------------------
Activity CNPC_AlienController::GetDeathActivity()
{
	return ACT_DIESIMPLE;
}



int CNPC_AlienController::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;
	/*if (cvar->FindVar("oc_ragdoll_enable_blooddrips")->GetInt())
	{
		int MaxDripsGreen = RandomInt(4, 12);
		Vector	vecToss, vecTraceDir;
		if (info.GetDamageType() & (DMG_BULLET | DMG_SHOCK))
		{
			for (int i = 0; i < MaxDripsGreen; i++)
			{
				vecTraceDir = WorldSpaceCenter();
				if (LastHitGroup() == HITGROUP_CHEST)
				{
					vecTraceDir.x += random->RandomFloat(-0.1, 0.1);
					vecTraceDir.y += random->RandomFloat(-0.1, 0.1);
					vecTraceDir.z += random->RandomFloat(-0.1, 0.1);
				}
				if (LastHitGroup() == HITGROUP_HEAD)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.4);
					vecTraceDir.y += random->RandomFloat(0.1, 0.4);
					vecTraceDir.z += random->RandomFloat(12.1, 12.4);
				}
				if (LastHitGroup() == HITGROUP_LEFTARM)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
					vecTraceDir.z += random->RandomFloat(0.1, 0.2);
				}
				if (LastHitGroup() == HITGROUP_RIGHTARM)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(12.1, 12.4);
					vecTraceDir.z += random->RandomFloat(0.1, 0.2);
				}
				if (LastHitGroup() == HITGROUP_LEFTLEG)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
					vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
				}
				if (LastHitGroup() == HITGROUP_RIGHTLEG)
				{
					vecTraceDir.x += random->RandomFloat(0.1, 0.2);
					vecTraceDir.y += random->RandomFloat(12.1, 12.4);
					vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
				}

				// Set the velocity
				Vector vecVelocity;
				AngularImpulse angImpulse;

				QAngle angles;
				angles.x = random->RandomFloat(-70, 20);
				angles.y = random->RandomFloat(0, 360);
				angles.z = 0.0f;
				AngleVectors(angles, &vecVelocity);

				vecVelocity *= random->RandomFloat(150, 300);
				vecVelocity += GetAbsVelocity();

				angImpulse = RandomAngularImpulse(-180, 180);



				CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
				//pGrenade->SetAbsOrigin(vecTraceDir);
				pGrenade->SetLocalOrigin(vecTraceDir);
				pGrenade->SetAbsAngles(RandomAngle(0, 360));
				DispatchSpawn(pGrenade);
				pGrenade->SetThrower(this);
				pGrenade->SetOwnerEntity(this);
				pGrenade->SetAbsVelocity(vecVelocity);
			}
		}
	}*/
	return BaseClass::OnTakeDamage_Alive(info);
}


Activity CNPC_AlienController::NPC_TranslateActivity( Activity eNewActivity )
{
	if (m_bIsFlying)
	{
		if (eNewActivity == ACT_IDLE || eNewActivity == ACT_GLIDE )
		{
			return ACT_FLY;
		}
		if (eNewActivity == ACT_MELEE_ATTACK1)
		{
			return ACT_MELEE_ATTACK2;
		}

		if (eNewActivity == ACT_RUN || eNewActivity == ACT_WALK)
		{
			return ACT_FLY;
		}
	} 
	else
	{
		if (eNewActivity == ACT_RANGE_ATTACK1)
		{
			return ACT_RANGE_ATTACK2;
		}
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_AlienController::TranslateSchedule( int scheduleType )
{
	//int baseType;

	switch( scheduleType )
	{
	case SCHED_ESTABLISH_LINE_OF_FIRE: //TERO: we don't need this in flying state because GetPreferredAttackPoint does it for us
	case SCHED_ALIENCONTROLLER_PRESS_ATTACK:
		{
			if (m_bIsFlying)
			{
				return SCHED_ALIENCONTROLLER_LEVITATE; //SCHED_IDLE_STAND;
			}
			
			return BaseClass::TranslateSchedule( scheduleType );
		}
		break;
	case SCHED_FAIL_TAKE_COVER:
		{
			// Fail schedule doesn't go through SelectSchedule()
			// So we have to clear beams and glow here
			/*m_fFireBallFadeInTime =0.f;
			if (m_pFireGlow)
				m_pFireGlow->SetRenderColorA(m_iFireBallFadeIn);	
			else
				CreateFireGlow();*/

			//ParticleMessages(MESSAGE_STOP_FIREBALL);
			m_bFireballEffects = false;

			return SCHED_CHASE_ENEMY;
			break;
		}
	case SCHED_RANGE_ATTACK1:
		{
			return SCHED_ALIENCONTROLLER_SHOOT_FIREBALL;
			break;
		}
	}
	
	return BaseClass::TranslateSchedule( scheduleType );
}


float CNPC_AlienController::InnateRange1MaxRange()
{
	return aliencontroller_max_enemy_distance.GetFloat();
}

float CNPC_AlienController::InnateRange1MinRange()
{
	return aliencontroller_min_enemy_distance.GetFloat();
}

Vector CNPC_AlienController::GetPreferredAttackPoint(CBaseEntity *moveTarget)
{
	if (moveTarget)
	{
		if (m_flNextPreferredAttackTime < gpGlobals->curtime)
		{
			m_flNextPreferredAttackTime = gpGlobals->curtime + random->RandomFloat(0.3,0.6);

			Vector vecTarget;

			Vector vecDist = moveTarget->GetAbsOrigin()  - GetAbsOrigin();
			vecDist.z = 0;
			float flDistance = ( vecDist ).Length();

			if (flDistance > aliencontroller_max_enemy_distance.GetFloat() )
				flDistance = aliencontroller_max_enemy_distance.GetFloat();
			else if (flDistance < aliencontroller_min_enemy_distance.GetFloat() )
				flDistance = aliencontroller_min_enemy_distance.GetFloat();

			//DevMsg("flDistance: %f\n", flDistance );

			//Note: if enemy's closer to prefered, we maybe don't want to get away straight away

			//We want to change the direction we are going once in a while or if that direction is blocked
			if (HasCondition(COND_ALIENCONTROLLER_FLY_BLOCKED) || gpGlobals->curtime > m_fLastOffsetSwithcTime )
			{
				m_fLastOffsetSwithcTime = gpGlobals->curtime + CONTROLLER_OFFSET_SWITCH_DELAY;
				m_iLastOffsetAngle      = -m_iLastOffsetAngle;
			}
	
			QAngle angTarget = QAngle(0, moveTarget->GetAbsAngles().y + m_iLastOffsetAngle,0);
			AngleVectors( angTarget, &vecTarget );

			vecTarget = moveTarget->GetAbsOrigin() + vecTarget * flDistance;

			vecTarget.z = moveTarget->GetAbsOrigin().z + m_flPreferredHeight; 

			if ( g_debug_aliencontroller.GetInt() == 2 )
			{
				NDebugOverlay::Box( vecTarget, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, 255, 0, 0, 0, 5 );
			}

			trace_t tr;
			AI_TraceHull( moveTarget->GetAbsOrigin(), vecTarget, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, moveTarget, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_ATTACK, &tr );

			//TERO: we are very close to another alien controller, let's fly pass him
			if ( (GetAbsOrigin() - tr.endpos).Length() < 96.0f && tr.m_pEnt && tr.m_pEnt->Classify() == CLASS_ALIENCONTROLLER )
			{
				//DevMsg("%s hit %s during GetPreferredAttackPoint(), ignoring\n", GetDebugName(), tr.m_pEnt->GetDebugName());
				AI_TraceHull( moveTarget->GetAbsOrigin(), vecTarget, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, moveTarget, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );
			}


			vecTarget = tr.endpos; 

			if ( g_debug_aliencontroller.GetInt() == 2 )
			{
				NDebugOverlay::Box( vecTarget, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, 0, 255, 0, 0, 5 );
			}

			//TERO: don't get closer if we would get too close to our enemy
			if ((moveTarget->GetAbsOrigin() - vecTarget).Length() < aliencontroller_inf_enemy_distance.GetFloat())
			{
				//DevMsg("end position is too close to our target %f\n", (moveTarget->GetAbsOrigin() - vecTarget).Length());
				//NDebugOverlay::Line( moveTarget->GetAbsOrigin(), vecTarget, 255, 0, 0, true, 1.0f );

				m_vecPreferedAttackPoint = GetAbsOrigin();

				//TERO: even if not blocked
				if (m_fLastOffsetSwithcTime < gpGlobals->curtime)
				{
					m_fLastOffsetSwithcTime = gpGlobals->curtime + CONTROLLER_OFFSET_SWITCH_DELAY;
					m_iLastOffsetAngle      = -m_iLastOffsetAngle;
				}
			}
			//TERO: then if it's too close to ourselves, fly to enemy
			else if ((GetAbsOrigin() - vecTarget).Length() < aliencontroller_inf_enemy_distance.GetFloat())
			{
				//DevMsg("end position is too close to ourselves %f\n", (GetAbsOrigin() - vecTarget).Length());
				//NDebugOverlay::Line( GetAbsOrigin(), vecTarget, 255, 0, 0, true, 1.0f );

				m_vecPreferedAttackPoint = moveTarget->GetAbsOrigin();
			}
			else
			{
				m_vecPreferedAttackPoint = vecTarget;
			}
		}
		return m_vecPreferedAttackPoint;
	}

	return GetAbsOrigin();
}

bool CNPC_AlienController::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{
	if (m_bIsFlying)
	{
		return true;
	}

	if ( GetEnemy() && (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).Length() < ALIENCONTROLLER_FACE_ENEMY_DISTANCE )
	{
		AddFacingTarget( GetEnemy(), GetEnemy()->WorldSpaceCenter(), 0.3f, 0.2f );
		return BaseClass::OverrideMoveFacing( move, flInterval );
	}


	return BaseClass::OverrideMoveFacing( move, flInterval );
}

void CNPC_AlienController::InputEnableLanding( inputdata_t &inputdata )
{
	m_bCanLand = true;
	m_flNextLandingTime = 0;
}

void CNPC_AlienController::InputDisableLanding( inputdata_t &inputdata )
{
	m_bCanLand = false;
}

void CNPC_AlienController::InputStopScriptingAndGesture( inputdata_t &inputdata )
{
	SetInAScript(false);
	RemoveAllGestures();
}

void CNPC_AlienController::UpdateEfficiency( bool bInPVS )	
{
	if ( m_bIsLanding || HasCondition(COND_ALIENCONTROLLER_SHOULD_LAND) )
	{
		SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); 
		SetMoveEfficiency( AIME_NORMAL ); 
		return;
	}

	BaseClass::UpdateEfficiency( bInPVS );
}

//-----------------------------------------------------------------------------
// Purpose: Checks if we are too close to ground to land or if there's enough space above us to fly
// Input  : 
// Output : bool if we should land or lift off
//-----------------------------------------------------------------------------
bool CNPC_AlienController::CheckLanding()
{
	m_flNextLandingTime = gpGlobals->curtime + 1.0f;

	float flPreferredHeight = m_flPreferredHeight;

	if (!m_bIsFlying && HasCondition( COND_ENEMY_UNREACHABLE ))
	{
		//DevMsg("npc_aliencontroller: enemy is unreachable by ground nodes, we better use decreased flPreferredHeight\n");
		flPreferredHeight = 72.0f;
	}

	//First check if we should keep flying or lift off
	float height = flPreferredHeight;
	if (height < 32) 
			height = 32;

	trace_t tr;
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector(0,0,flPreferredHeight), ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
			NDebugOverlay::Line(GetAbsOrigin() + Vector(0,0,72), tr.endpos, 255,255,255, true, 5);
			NDebugOverlay::Box( tr.endpos, Vector(10,10,10), Vector(-10,-10,-10), 255, 255, 255, 0, 5 );
#endif

	if ( tr.fraction == 1  )
	{
			if (m_bIsFlying)
			{
				return false;
			}
			else
			{
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
				DevMsg("npc_aliencontroller: Should Fly\n");
#endif
				return true;
			}
	}
	
	if (m_bIsFlying)
	{
		SetNavType( NAV_GROUND );

		CapabilitiesAdd ( bits_CAP_MOVE_GROUND );
		CapabilitiesRemove( bits_CAP_MOVE_FLY );

		int iNodeToLand = GetNavigator()->GetNetwork()->NearestNodeToPoint( this, GetAbsOrigin() - Vector(0,0,32), false );

		if (iNodeToLand >= 0)
		{
			Vector vecLand = GetNavigator()->GetNetwork()->GetNodePosition( this, iNodeToLand );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
			NDebugOverlay::Box( vecLand,		Vector(-10,-10,-10), Vector(10,10,10), 255, 0, 0, 0, 5 );
			NDebugOverlay::Box( GetAbsOrigin(), Vector(-10,-10,-10), Vector(10,10,10), 255, 0, 0, 0, 5 );
			NDebugOverlay::Line( GetLocalOrigin(), vecLand, 255, 255, 0, true, 5.0f );
#endif

			AI_TraceHull( GetAbsOrigin(), vecLand, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );

			if ( tr.fraction == 1 && (tr.endpos - GetAbsOrigin()).Length() < 70)
			{
				SetNavType( NAV_FLY );
				CapabilitiesAdd ( bits_CAP_MOVE_FLY );
				CapabilitiesRemove( bits_CAP_MOVE_GROUND );

				m_vecLandPosition = vecLand;
				return true;
			}
		}

		CapabilitiesAdd ( bits_CAP_MOVE_FLY );
		CapabilitiesRemove( bits_CAP_MOVE_GROUND );

		SetNavType( NAV_FLY );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Lands or Lifts Off depending if we are flying or walking
// Input  : 
// Output :
//-----------------------------------------------------------------------------
void CNPC_AlienController::Land(bool bIsFlying)
{
	m_bIsLanding = true;

	//TERO: we do this here second time in case some uses the animation as a scripted_sequence
	m_flNextLandingTime = gpGlobals->curtime + 6;


	if (bIsFlying)
	{
		SetHullType( HULL_HUMAN );

		//TERO: lets fix the angles
		QAngle angles = GetAbsAngles();
		angles.x = angles.z = 0.0f;
		SetAbsAngles(angles);

		//We are going to walk
		SetNavType( NAV_GROUND );

		RemoveFlag( FL_FLY );
		
		CapabilitiesAdd ( bits_CAP_MOVE_GROUND );
		CapabilitiesRemove( bits_CAP_MOVE_FLY ); //| bits_CAP_SKIP_NAV_GROUND_CHECK ); //| bits_CAP_SKIP_NAV_GROUND_CHECK 
		SetMoveType( MOVETYPE_STEP );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("Is now Landing\n");
#endif
	} 
	else
	{
		//SetHullType( HULL_HUMAN_CENTERED );
		SetHullType( HULL_LARGE_CENTERED ); // Light Kill : Trying to fix mistake with npc integration.

		UTIL_SetOrigin( this, GetAbsOrigin() + Vector( 0 , 0 , 1 ));

		//We are gonna fly
		SetGroundEntity( NULL );

		SetNavType( NAV_FLY );

		AddFlag( FL_FLY );
		
		CapabilitiesAdd ( bits_CAP_MOVE_FLY ); //| bits_CAP_SKIP_NAV_GROUND_CHECK ); //| bits_CAP_SKIP_NAV_GROUND_CHECK 
		CapabilitiesRemove( bits_CAP_MOVE_GROUND );

		SetMoveType( MOVETYPE_STEP ); //TERO: crow has MOVETYPE_STEP here
	}

	ClearHintNode(0.0f);

	if (GetNavigator()->IsGoalActive())
	{
		Vector vecGoal = GetNavigator()->GetGoalPos();
		CBaseEntity *pGoal = GetNavigator()->GetGoalTarget();
		GetNavigator()->ClearGoal();
		AI_NavGoal_t goal(vecGoal, ACT_FLY, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST, pGoal);
		GetNavigator()->SetGoal( goal );
	}
}


#ifdef ALIENCONTROLLER_OVERRIDE_SCHEDULE_CHANGE_FUNCTIONS_TO_FIX_GOAL_FROM_CLEARING

void CNPC_AlienController::SetSchedule( CAI_Schedule *pNewSchedule, bool DontClearGoal )
{
	//DevMsg("npc_aliencontroller: SET SCHEDULE! SET SCHEDULE! SET SCHEDULE! SET SCHEDULE! SET SCHEDULE!\n");
	
	BaseClass::SetSchedule(pNewSchedule,  (m_bIsFlying || HasCondition(COND_ALIENCONTROLLER_SHOULD_LAND) || m_bIsLanding) && !IsInAScript() );

}

//=========================================================
//=========================================================
void CNPC_AlienController::OnScheduleChange ( void )
{
	if (!m_bIsFlying || IsInAScript() ) //|| m_bIsLanding
	{
		BaseClass::OnScheduleChange();
		return;
	}

	if (IsRunningBehavior())
	{
		GetRunningBehavior()->BridgeOnScheduleChange(); 
	}

	EndTaskOverlay();

	GetNavigator()->OnScheduleChange();

	m_flMoveWaitFinished = 0;

	VacateStrategySlot();
}

#endif


bool CNPC_AlienController::ControllerProgressFlyPath()
{
	AI_ProgressFlyPathParams_t params( MASK_NPCSOLID ); //MASK_SOLID );

	params.waypointTolerance = 8.0f; // * flDot;
	params.strictPointTolerance = 4.0f; // * flDot;
	params.goalTolerance = 8.0f;

	float waypointDist = ( GetNavigator()->GetCurWaypointPos() - (GetAbsOrigin() + CONTROLLER_BODY_CENTER)).Length();

	//DevMsg("dist %f\n", waypointDist);

	if ( GetNavigator()->CurWaypointIsGoal() )
	{
		float tolerance = max( params.goalTolerance, GetNavigator()->GetPath()->GetGoalTolerance() );
		if ( waypointDist <= tolerance )
				return true; //AINPP_COMPLETE;
	}
	else
	{
		bool bIsStrictWaypoint = ( (GetNavigator()->GetPath()->CurWaypointFlags() & (bits_WP_TO_PATHCORNER|bits_WP_DONT_SIMPLIFY) ) != 0 );
		float tolerance = (bIsStrictWaypoint) ? params.strictPointTolerance : params.waypointTolerance;
		if ( waypointDist <= tolerance )
		{
			trace_t tr;
			AI_TraceLine( GetAbsOrigin() + CONTROLLER_BODY_CENTER, GetNavigator()->GetPath()->GetCurWaypoint()->GetNext()->GetPos(), MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );
			if ( tr.fraction == 1.0f )
			{
				GetNavigator()->AdvancePath();	
				return false; //AINPP_ADVANCED;
			}
		}

		if ( HasCondition( COND_ALIENCONTROLLER_FLY_BLOCKED ) && GetNavigator()->SimplifyFlyPath( params, 35.0f ) )
		{
			ClearCondition( COND_ALIENCONTROLLER_FLY_BLOCKED);
			return false; //AINPP_ADVANCED;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: back away from overly close zombies
//-----------------------------------------------------------------------------
Disposition_t CNPC_AlienController::IRelationType( CBaseEntity *pTarget )
{
	/*CBasePlayer *pPlayer = ToBasePlayer( pTarget );

	if (pPlayer && pPlayer->IsRevealedTraitor())
	{
		return D_LI;
	}*/

	return BaseClass::IRelationType( pTarget );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_AlienController::OverrideMove( float flInterval )
{	
	if (!IsInAScript())
	{
		if (m_bIsLanding ) 
		{
			ControllerLand( flInterval );
			return true;
		}
#ifdef HLSS_CONTROLLER_TELEKINESIS
		else if ( (m_iNumberOfPhysiscsEnts <= 0) && m_flNextLandingTime < gpGlobals->curtime && !HasCondition(COND_ALIENCONTROLLER_SHOULD_LAND) )
#else
		else if ( m_flNextLandingTime < gpGlobals->curtime && !HasCondition( COND_ALIENCONTROLLER_SHOULD_LAND ))
#endif
		{
			if (m_bCanLand)
			{
				bool bShouldLand = CheckLanding();

				if (bShouldLand)
				{
					SetCondition(COND_ALIENCONTROLLER_SHOULD_LAND);
					m_flNextLandingTime = gpGlobals->curtime + 6.0f;
					return true;
				}
			}
		}
	}

	if (!m_bIsFlying)
	{
		return false;
	}

#ifdef HLSS_CONTROLLER_TELEKINESIS
	if (IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS ) && GetEnemy())
	{
		ControllerFly(flInterval, GetAbsOrigin(), GetEnemy()->WorldSpaceCenter());
	}
#endif

	Vector vMoveTargetPos=GetAbsOrigin();
	Vector vecGoal = vMoveTargetPos;
	CBaseEntity *pMoveTarget = NULL;

	//TERO: if we don't have a path, then get our move target
	if ( !GetNavigator()->IsGoalActive()  || ( GetNavigator()->GetCurWaypointFlags() | bits_WP_TO_PATHCORNER ) )
	{
		// Select move target 
		if ( GetTarget() != NULL )
		{
			pMoveTarget = GetTarget();
			vMoveTargetPos = pMoveTarget->GetAbsOrigin() - CONTROLLER_BODY_CENTER;
		}
		else if ( GetEnemy() != NULL )
		{
			pMoveTarget = GetEnemy();
			if (GetEnemy()->Classify() == CLASS_MANHACK || GetEnemy()->Classify() == CLASS_SCANNER )
			{
				vMoveTargetPos = pMoveTarget->GetAbsOrigin(); 
			}
			else
			{
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
				DevMsg("npc_aliencontroller: getting preferred attack point\n");
#endif
				vMoveTargetPos = GetPreferredAttackPoint(pMoveTarget); 
			}
		}

		if (GetEnemy()) // && HasCondition( COND_SEE_ENEMY ))
		{
			vecGoal = GetEnemy()->WorldSpaceCenter();
		}
		else
		{
			vecGoal = vMoveTargetPos;
		}
	}

	if (HasCondition( COND_ALIENCONTROLLER_FLY_BLOCKED) ) 
	{
		Vector position = GetAbsOrigin();

		bool bGotPath = false;

		if (GetNavigator()->IsGoalActive()) // && m_vOldGoal == vec3_origin)
		{

		

			
		}
		else //if ( !GetNavigator()->IsGoalActive())
		{

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
			DevMsg("npc_aliencontroller: trying to simply get a path to move goal\n");
#endif
			AI_NavGoal_t goal( GOALTYPE_LOCATION, vMoveTargetPos ); //, ACT_FLY, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST, pMoveTarget );
			bGotPath = GetNavigator()->SetGoal( goal );
			ClearCondition( COND_ALIENCONTROLLER_FLY_BLOCKED );
		}

		if (!GetHintNode())
		{
			if (!bGotPath)
			{
				ClearHintNode(0.0f);
			
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
				DevMsg("npc_aliencontroller: trying to find a node\n");
#endif

				CHintCriteria criteria;
				criteria.SetGroup( GetHintGroup() );
				criteria.SetHintTypeRange( HINT_TACTICAL_COVER_MED, HINT_TACTICAL_ENEMY_DISADVANTAGED );
				criteria.SetFlag( bits_HINT_NODE_NEAREST | bits_HINT_NODE_USE_GROUP );
				criteria.AddIncludePosition(GetAbsOrigin() + CONTROLLER_BODY_CENTER, 512);
				SetHintNode( CAI_HintManager::FindHint( this, position, criteria  ));
			}

			if ( GetHintNode() )
			{
				Vector vNodePos = vMoveTargetPos;
				GetHintNode()->GetPosition(this, &vNodePos);

				bool bGroundNode = ( GetHintNode()->GetNode() && GetHintNode()->GetNode()->GetType() == NODE_GROUND);
	
				if (m_vOldGoal == vec3_origin)
				{
					if (GetNavigator()->IsGoalActive())
					{
						m_vOldGoal = GetNavigator()->GetGoalPos();
					} 
					else if ( pMoveTarget)
					{
						m_vOldGoal = vMoveTargetPos;
					}
				}

				if ( bGroundNode ) //TERO: this node is too low
				{
					vMoveTargetPos = vNodePos + CONTROLLER_BODY_CENTER;
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
					DevMsg("npc_aliencontroller: we hit a ground node graph\n");
#endif
				}
				else
				{
					vMoveTargetPos = vNodePos; 
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
					DevMsg("npc_aliencontroller: we hit an air node graph\n");
#endif
				}

				AI_NavGoal_t goal( GOALTYPE_LOCATION, vMoveTargetPos ); //, ACT_FLY, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST );
				GetNavigator()->SetGoal( goal );
				ClearCondition( COND_ALIENCONTROLLER_FLY_BLOCKED );

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
				NDebugOverlay::Line(position, vMoveTargetPos, 255,255,255, true, 0);
				NDebugOverlay::Box( vMoveTargetPos, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 128, 1 );
				DevMsg("npc_aliencontroller: Trying to survive with a node!!! *********************************** WORKING!\n");
#endif
			}

		}// END if (!GetHintNode())
	}

	//TERO: I start a new if here on purpose
	if (GetNavigator()->IsGoalActive()) //  && !( GetNavigator()->GetCurWaypointFlags() | bits_WP_TO_PATHCORNER ))
	{
		if (g_debug_aliencontroller.GetInt() == 5)
		{
			GetNavigator()->DrawDebugRouteOverlay();
		}

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("npc_aliencontroller: We have a goal\n");
		GetNavigator()->DrawDebugRouteOverlay();
#endif

		if (GetNavigator()->GetPath()->GetCurWaypoint()->NavType() == NAV_GROUND)
		{
			vMoveTargetPos = GetNavigator()->GetCurWaypointPos() + CONTROLLER_BODY_CENTER;
		}
		else
		{
			vMoveTargetPos = GetNavigator()->GetCurWaypointPos();
		}

		if (!GetEnemy())
		{
			vecGoal = vMoveTargetPos;
		}

		if (ControllerProgressFlyPath())
		{
			ClearHintNode(0.0f);

			ClearCondition( COND_ALIENCONTROLLER_FLY_BLOCKED );

			//GetNavigator()->ClearGoal();
			TaskMovementComplete();

			if (m_vOldGoal != vec3_origin)
			{
				AI_NavGoal_t goal( GOALTYPE_LOCATION, m_vOldGoal ); 
					
				GetNavigator()->SetGoal( goal);

				m_vOldGoal = vec3_origin;
			}
		}

		ControllerFly(flInterval, vMoveTargetPos, vecGoal );
	}
	else if ( pMoveTarget != NULL )
	{
#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("npc_aliencontroller: We have a move target!\n");
		NDebugOverlay::Box( vMoveTargetPos, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 128, 1 );
#endif
		//Since we are not going on a path, we are not blocked and should clear the hint
		//ClearHintNode(0.0f);

		if ( pMoveTarget == GetHintNode())
		{
			DevMsg("move target is hint node\n");

			if ( ControllerReachedPoint( vMoveTargetPos ) )
			{
				SetTarget( NULL );
				ClearHintNode( 0.0f);
			}
		}
		else
		{
			ClearHintNode(0.0f);
		}

		ControllerFly(flInterval, vMoveTargetPos, vecGoal); //vMoveTargetPos + CONTROLLER_BODY_CENTER );
	} 
	else 
	{
		//Since we are not going on a path, we are not blocked and should clear the hint
		ClearHintNode(0.0f);

		//We don't have a target, lets slow down
		Vector vecSpeed = GetAbsVelocity();
		float flSpeed   = VectorNormalize(vecSpeed);

		if (flSpeed < 5)
			flSpeed = 0;
		else
		{
			if (flInterval > 1.0f)
				flInterval = 1.0f;
			flSpeed = flSpeed * (1.0f-flInterval) * 0.7f;
		}

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
		DevMsg("npc_aliencontroller: slowing down, speed now: %f\n", flSpeed);
#endif

		SetAbsVelocity( vecSpeed * flSpeed );

	}


	if (!HasCondition( COND_ALIENCONTROLLER_FLY_BLOCKED )) // && !GetHintNode())
	{
		//TERO: blocking detection

		trace_t tr;
		AI_TraceHull( GetAbsOrigin(), vMoveTargetPos, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );

		float fTargetDist = (1.0f-tr.fraction) * ((GetAbsOrigin() - vMoveTargetPos).Length());
			
		if ( ( tr.m_pEnt == pMoveTarget ) || ( fTargetDist < 50 ) )
		{
			if ( g_debug_aliencontroller.GetInt() == 1 )
			{
				NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 0,255,0, true, 0);
				NDebugOverlay::Cross3D(tr.endpos, Vector(-5,-5,-5),Vector(5,5,5),0,255,0,true,0.1);
			}
			ClearCondition( COND_ALIENCONTROLLER_FLY_BLOCKED );
		}
		else		
		{
			//HANDY DEBUG TOOL	
			if ( g_debug_aliencontroller.GetInt() == 1 )
			{
				NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 255,0,0, true, 0);
				NDebugOverlay::Cross3D(tr.endpos,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,0.1);
			}

			/*if ( tr.m_pEnt && tr.m_pEnt->GetCollisionGroup()  == COLLISION_GROUP_BREAKABLE_GLASS )
			{
				//SetIdealActivity( (Activity) ACT_RANGE_ATTACK1 );
				m_hFireBallTarget = tr.m_pEnt;

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
				DevMsg("npc_aliencontroller: We hit glass, we should shoot it. ");
#endif

			}*/

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
			DevMsg("npc_aliencontroller: Blocked!\n");
#endif
			DevMsg("npc_aliencontroller: Blocked!\n");

			SetCondition( COND_ALIENCONTROLLER_FLY_BLOCKED );
		}
	}

	return true;
}


bool CNPC_AlienController::ControllerReachedPoint(Vector vecOrigin)
{
	Vector vecDir = vecOrigin - (GetAbsOrigin() + CONTROLLER_BODY_CENTER);
	Vector vecVelocity = GetAbsVelocity();

	float flZDist  = vecDir.z;
	vecDir.z	   = 0;
	float flXYDist = VectorNormalize(vecDir);  
	VectorNormalize(vecVelocity);
	float flDot	   = DotProduct( vecDir, vecVelocity );

	if (flDot < 0.8)
	{
		if (flZDist < ALIENCONTROLLER_Z_DISTANCE_NOT_FACING && flXYDist < ALIENCONTROLLER_XY_DISTANCE_NOT_FACING)
		{
			return true;
		}
	}
	else
	{
		if (flZDist < ALIENCONTROLLER_Z_DISTANCE_FACING && flXYDist < ALIENCONTROLLER_XY_DISTANCE_FACING)
		{
			return true;
		}
	}

	return false;
}


float CNPC_AlienController::CheckLandingSpace(Vector origin)
{
	trace_t tr;
	AI_TraceHull( origin, origin - CONTROLLER_BODY_CENTER, ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );

	return tr.fraction;
}

//-----------------------------------------------------------------------------
// Purpose: This is a bit useless currently
//-----------------------------------------------------------------------------
void CNPC_AlienController::ControllerLand(float flInterval)
{
	if (m_bIsFlying) // && m_iNodeToLand >= 0)
	{
		//CapabilitiesAdd ( bits_CAP_MOVE_GROUND );
		//CapabilitiesRemove( bits_CAP_MOVE_FLY );

		//Vector vecLand = GetNavigator()->GetNetwork()->GetNodePosition( this, m_iNodeToLand ); 
		Vector vecDir  = m_vecLandPosition - GetAbsOrigin();
		VectorNormalize( vecDir );

		float flSpeed = 100.0f;

		Vector vecDeflect;
		if ( Probe( vecDir, flSpeed * flInterval, vecDeflect ) ) //flInterval * 1.2
		{
			vecDir = vecDeflect;
			VectorNormalize( vecDir );
		}

		SetAbsVelocity( vecDir * flSpeed );

	}
	else
	{
		Vector vecMoveDir;

		trace_t tr;
		AI_TraceLine ( GetAbsOrigin() + CONTROLLER_BODY_CENTER, GetAbsOrigin(), MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr);

		float flSpeed = (1-(2*tr.fraction)) * CONTROLLER_BODY_CENTER_Z * flInterval;

		if (!m_bIsFlying) 
			flSpeed = -flSpeed;

		SetAbsVelocity( CONTROLLER_BODY_CENTER * flSpeed );
	}

//	m_vLastFlyPosition = GetAbsOrigin();

	//Now, lets turn to our target, if it's enemy, we turn to him, if not, we turn to the target pos
	//for enemy the vTargetPos is different than vMoveTargetPos, for others it's the same
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_AlienController::ControllerFly(float flInterval, Vector vMoveTargetPos, Vector vTargetPos )
{
	Vector vecMoveDir = ( vMoveTargetPos - (GetAbsOrigin() + CONTROLLER_BODY_CENTER) );

	float flDistance = VectorNormalize( vecMoveDir );

	/*Vector vecTravel = (GetAbsOrigin() - m_vLastFlyPosition);
	float flTravel = VectorNormalize(vecTravel);

	float flDot = DotProduct(vecTravel, vecMoveDir);
	float flScale = (1.0f + flDot) * 0.5f;
	float flMaxSpeed = 320; //20.0f + (flScale * 300.0f);

	DevMsg("DOT PRODUCT %f, SCALE %f, MAX SPEED %f\n", flDot, flScale, flMaxSpeed);

	NDebugOverlay::Line( m_vLastFlyPosition,					GetAbsOrigin(),	255, 0, 0, true, 0.1 );
	NDebugOverlay::Line( vMoveTargetPos,						GetAbsOrigin(),	0, 255, 0, true, 0.1 );*/

	//NDebugOverlay::Box( GetAbsOrigin(),	GetHullMins(), GetHullMaxs(), 0, 255, 0, 0, 0.1 );

	//NDebugOverlay::Line( vMoveTargetPos,						GetAbsOrigin(),	0, 255, 0, true, 0.1 );

	/*NDebugOverlay::Line( vMoveTargetPos,						GetAbsOrigin(),	0, 0, 255, true, 0.1 );
	NDebugOverlay::Line( vTargetPos,							GetAbsOrigin(),	255, 0, 0, true, 0.1 );

	NDebugOverlay::Box( vMoveTargetPos,	GetHullMins(), GetHullMaxs(), 0, 0, 255, 0, 0.1 );
	NDebugOverlay::Box( vTargetPos,		GetHullMins(), GetHullMaxs(), 255, 0, 0, 0, 0.1 );*/
	
	// BJ: Over Advisor Controller speed values

	/*if (FClassnameIs(this, "npc_advisor_combat"))
	{
		float flSpeed	 = sin( RemapValClamped( flDistance, 0.0f, 256.0f, 0.0f, M_PI * 0.5f)) * 130.0f;//320
	}
	else*/
		float flSpeed = (FClassnameIs(this, "npc_advisor_fighter")) ? sin(RemapValClamped(flDistance, 0.0f, 256.0f, 0.0f, M_PI * 0.5f)) * 130.0f : sin(RemapValClamped(flDistance, 0.0f, 256.0f, 0.0f, M_PI * 0.5f)) * 200.0f;//320

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
	DevMsg("npc_aliencontroller: running schedule with name: %s\n", GetCurSchedule()->GetName());
	DevMsg("npc_aliencontroller: moving with speed: %f, dist %f, remapped dist %f\n", flSpeed, flDistance, RemapValClamped( flDistance, 0.0f, 256.0f, 0.0f, M_PI * 0.5f));
#endif

	// Look to see if we are going to hit anything.
	Vector vecDeflect;
	if ( Probe( vecMoveDir, flSpeed * flInterval, vecDeflect ) )
	{
		vecMoveDir = vecDeflect;
		VectorNormalize( vecMoveDir );
	}

	SetAbsVelocity( vecMoveDir * flSpeed );

	//NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + (vecMoveDir * flSpeed),	0, 255, 0, true, 0.1 );

	//Now, lets turn to our target, if it's enemy, we turn to him, if not, we turn to the target pos
	//for enemy the vTargetPos is different than vMoveTargetPos, for others it's the same

	vecMoveDir = ( vTargetPos - GetAbsOrigin() );

	QAngle angTarget, angCurrent;
	VectorAngles( vecMoveDir, angTarget );

	angCurrent = GetAbsAngles();

//TERO: used to be 12.0f
#define CONTROLLER_ANGULAR_SPEED 60
#define CONTROLLER_ANGLE_Y_LIMIT 35.0f

	angTarget.z = 0; 
	angTarget.x = 0;

	//DevMsg("Target %f, current %f\n", angTarget.y, angCurrent.y );

	float angleDiff = UTIL_AngleDiff( angTarget.y, angCurrent.y  );

	//DevMsg("diff %f\n", angleDiff);

	angTarget.y = UTIL_ApproachAngle(angCurrent.y + angleDiff, angCurrent.y, CONTROLLER_ANGULAR_SPEED);

	//DevMsg("calculated %f\n", angTarget.y);

#ifdef CONTROLLER_MOVE_DEBUG_MESSAGES
	DevMsg("npc_aliencontroller: z: %f, x: %f, y: %f\n", angTarget.z, angTarget.x, angTarget.y);
#endif

	//m_fLastTurnSpeed = newAngleDiff;

	SetAbsAngles( angTarget );

	//m_vLastFlyPosition = GetAbsOrigin();
}

// Purpose: Looks ahead to see if we are going to hit something. If we are, a
//			recommended avoidance path is returned.
// Input  : vecMoveDir - 
//			flSpeed - 
//			vecDeflect - 
// Output : Returns true if we hit something and need to deflect our course,
//			false if all is well.
//-----------------------------------------------------------------------------
bool CNPC_AlienController::Probe( const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect )
{
	//
	// Look 1/2 second ahead.
	//
	trace_t tr;
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + (vecMoveDir * flSpeed), ALIENCONTROLLER_HULL_MINS, ALIENCONTROLLER_HULL_MAXS, MASK_NPCSOLID, this, HLSS_ALIENCONTROLLER_COLLISIONGROUP_FOR_MOVEMENT, &tr );
	if ( tr.fraction < 1.0f )
	{
		/*DevMsg("we hit something\n");

		if (tr.m_pEnt)
		{
			DevMsg("tr.m_pEnt %s\n", tr.m_pEnt->GetDebugName());
		}

		NDebugOverlay::Line( GetAbsOrigin() + Vector(0,0,35), tr.endpos, 255, 0, 0, 0, 5 );

		NDebugOverlay::Line( GetAbsOrigin() + Vector(0,0,35), GetAbsOrigin() + (vecMoveDir * flSpeed), 255, 0, 0, 0, 5 );*/

		//
		// If we hit something, deflect flight path parallel to surface hit.
		//
		Vector vecUp;
		CrossProduct( vecMoveDir, tr.plane.normal, vecUp );
		CrossProduct( tr.plane.normal, vecUp, vecDeflect );
		VectorNormalize( vecDeflect );
		return true;
	}

	vecDeflect = vec3_origin;
	return false;
}



int CNPC_AlienController::RangeAttack1Conditions( float flDot, float flDist )
{
	//return COND_NONE;	//TERO: remove this

	//TERO: only because we don't have animations yet for land shooting
	if (m_bIsLanding)
	{
		return ( COND_NONE );
	}

	if (GetEnemy() == NULL)
	{
		return( COND_NONE );
	}

	if ( m_flNextAttackTime > gpGlobals->curtime )
		return( COND_NONE );

	if ( flDot < 0.65 )
	{
		return( COND_NOT_FACING_ATTACK );
	}

	return COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL; 
	//return( COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL ); //COND_CAN_RANGE_ATTACK1 );
}

int CNPC_AlienController::MeleeAttack1Conditions( float flDot, float flDist )
{
	//TERO: only because we don't have animations yet for land shooting
	if (m_bIsLanding)
	{
		return ( COND_NONE );
	}

	if (!GetEnemy())
		return COND_NONE;

	/*if (GetEnemy()->Classify() == CLASS_COMBINE_MAIN_FRAME)
	{
		return COND_NONE;
	}*/

	if (m_bIsFlying)
	{
		if (GetEnemy()->Classify() != CLASS_PLAYER)
			return COND_NONE;
	}

	if ( fabs(GetEnemy()->GetAbsOrigin().z - GetAbsOrigin().z) > 30 )
		return COND_NONE;

	if (flDist > 70 )
	{
		//return COND_TOO_FAR_TO_ATTACK;

		//TERO: so that we wont fly closer just to melee
		return COND_NONE;
	}

	if ( flDot < 0.65 )
	{
		return( COND_NOT_FACING_ATTACK );
	}

	if ( m_bIsFlying )
	{
		//TERO: disabled for now since we don't have special "hands" attack animation
		/*if ( !(GetEnemy()->Classify() == CLASS_MANHACK || GetEnemy()->Classify() == CLASS_SCANNER) )
		{
			return( COND_NONE );
		}*/

		if ( (GetEnemy()->GetAbsOrigin().z - GetAbsOrigin().z) > 0 )
		{
			return (COND_NONE);
		}
	}

	return( COND_CAN_MELEE_ATTACK1 );
}



void CNPC_AlienController::ShootFireBall() 
{
	Vector vecOrigin, vecAttachmentDir;
	QAngle vecAngles, angWantedAngle;

	GetAttachment( m_iFireBallAttachment, vecOrigin, vecAngles );

	angWantedAngle = vecAngles;

	AngleVectors(vecAngles, &vecAttachmentDir);

	VectorNormalize( vecAttachmentDir );

	if (m_hFireBallTarget)
	{
		Vector vecWantedDir = m_hFireBallTarget->GetAbsOrigin() - vecOrigin;

		VectorAngles(vecWantedDir, angWantedAngle);

		//We should clamp our yaw (we are only interested on clamping the yaw since it's useless to clamp pitch or roll)
		float angleDiff   = UTIL_AngleDiff( angWantedAngle.y, vecAngles.y );
		angleDiff		  = clamp(angleDiff, -75, 75);
		angWantedAngle.y  = angWantedAngle.y + angleDiff;

		//We have already shot it once, that should be enough, maybe a counter later 

		CFireBall *pBall = CFireBall::Create( vecOrigin, angWantedAngle, this );

		if (pBall)
		{
			pBall->m_hTarget = m_hFireBallTarget;
			pBall->SetOwnerEntity(this);
		}

		m_hFireBallTarget = NULL;
	}
	/*else if (GetEnemy() && GetEnemy()->Classify() == CLASS_MANHACK )
	{
		CFireBall *pBall = CFireBall::Create( GetEnemy()->GetAbsOrigin(), GetEnemy()->GetAbsAngles(), this );
		if (pBall)
		{
			pBall->m_hTarget = GetEnemy();
			pBall->SetOwnerEntity(this);
		}
	}*/
	else if (GetEnemy())
	{
		bool bManhackEnemy = GetEnemy()->Classify() == CLASS_MANHACK;

		Vector vecPredictedPos;

		//TERO: lets not predict every frigging time
		if (random->RandomInt(0,2))
		{

			//Lets count predicted position for the enemy
			Vector vEnemyForward, vForward;

			GetEnemy()->GetVectors( &vEnemyForward, NULL, NULL );
			GetVectors( &vForward, NULL, NULL );

			float flDot = DotProduct( vForward, vEnemyForward );

			if ( flDot < 0.5f )
				 flDot = 0.5f;

			flDot = 1 - flDot;

			float flDist = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length() / 512;
	
			//Get our likely position in two seconds
			UTIL_PredictedPosition( GetEnemy(), flDot * flDist, &vecPredictedPos );
		
			//TERO: this used to be commented out
			if (GetEnemy()->IsPlayer())
				vecPredictedPos.z = vecPredictedPos.z + 32;	//otherwise it will target at the eyes and it misses
			else
				vecPredictedPos.z = vecPredictedPos.z + (GetEnemy()->BodyTarget(GetAbsOrigin(),true).z - GetEnemy()->GetAbsOrigin().z); 
		}
		else
		{
			vecPredictedPos = GetEnemy()->BodyTarget(GetAbsOrigin(),true);
		}

		if ( g_debug_aliencontroller.GetInt() == 3 )
		{
			NDebugOverlay::Box( vecPredictedPos, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 0, 5 );
		}

		Vector vecWantedDir = vecPredictedPos - vecOrigin;

		VectorAngles(vecWantedDir, angWantedAngle);

		//We should clamp our yaw (we are only interested on clamping the yaw since it's useless to clamp pitch or roll)
		float angleDiff   = UTIL_AngleDiff( angWantedAngle.y, vecAngles.y );
		angleDiff		  = clamp(angleDiff, -75, 75);
		angWantedAngle.y  = angWantedAngle.y + angleDiff;

		/*angleDiff		  = UTIL_AngleDiff( angWantedAngle.z, vecAngles.z );
		angleDiff		  = clamp(angleDiff, -80, 80);
		angWantedAngle.z  = angWantedAngle.z + angleDiff;

		angleDiff		  = UTIL_AngleDiff( angWantedAngle.x, vecAngles.x );
		angleDiff		  = clamp(angleDiff, -80, 80);
		angWantedAngle.x  = angWantedAngle.x + angleDiff;*/

		CFireBall *pBall = CFireBall::Create( vecOrigin, angWantedAngle, this );
		if (pBall)
		{
			pBall->m_hTarget = GetEnemy();
			pBall->SetOwnerEntity(this);

			if (!bManhackEnemy)
			{
				pBall->SetGracePeriod(0.1f);

				AngleVectors( angWantedAngle, &vecOrigin );

				pBall->SetAbsVelocity( vecOrigin * 1000 );
			}
		}
	} 

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );

	/*m_iNumberOfAttacks--;
	if (m_iNumberOfAttacks <= 0)
	{
		m_iNumberOfAttacks = random->RandomInt(3,6);
		m_flNextAttackTime = gpGlobals->curtime + random->RandomFloat( 0.5, 1.5 );
	}*/
}

void CNPC_AlienController::Event_Killed( const CTakeDamageInfo &info )
{
//	if (m_pFireGlow)
//		UTIL_Remove( m_pFireGlow );
	//ParticleMessages(MESSAGE_STOP_ALL);
	m_bFireballEffects = false;

	//DevMsg("aliencontroller: Event_killed()\n");
	
	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CNPC_AlienController::FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
	CBaseEntity	*pHitEntity = NULL;
	if ( BaseClass::FVisible( pEntity, traceMask, &pHitEntity ) )
		return true;

	if (pHitEntity && pHitEntity->Classify() == CLASS_COMBINE_MAIN_FRAME)
	{
		return true;
	}

	if (ppBlocker)
	{
		*ppBlocker = pHitEntity;
	}

	return false;
}*/


//TERO: PHYSICS THROWER STUFF COMES AFTER THIS

#ifdef HLSS_CONTROLLER_TELEKINESIS
//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_AlienController::GatherConditions( void )
{
//	ClearCondition( COND_ALIENCONTROLLER_LOCAL_MELEE_OBSTRUCTION );

	BaseClass::GatherConditions();

	if( m_NPCState == NPC_STATE_COMBAT && !m_bIsLanding ) //&& m_bIsFlying )
	{
		// This check for !m_pPhysicsEnt prevents a crashing bug, but also
		// eliminates the zombie picking a better physics object if one happens to fall
		// between him and the object he's heading for already. 
		if( gpGlobals->curtime >= m_flNextTelekinesisScan && 
			(m_iNumberOfPhysiscsEnts <= 0) && 
			 HasCondition( COND_SEE_ENEMY ) &&
			!IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS ))
		{
			DevMsg("AlienController: searching physobject...\n");


			//TERO: OI! OI! OI! PHYS GRAB SEARCH DISABLED! PUT IT BACK
			FindNearestPhysicsObjects( ALIENCONTROLLER_MAX_PHYSOBJ_MASS );
			m_flNextTelekinesisScan = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_SCAN_DELAY;
		}
	}

	if( (m_iNumberOfPhysiscsEnts > 0) && HasCondition( COND_SEE_ENEMY ) ) // &&gpGlobals->curtime >= m_flNextSwat
	{
		SetCondition( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK );
	}
	else
	{
		ClearCondition( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_AlienController::BuildScheduleTestBits( void )
{
	if ( !HasCondition( COND_ALIENCONTROLLER_SHOULD_LAND ) && HasCondition( COND_SEE_ENEMY) &&
		 !IsCurSchedule(SCHED_ALIENCONTROLLER_TELEKINESIS) &&
		 !IsCurSchedule(SCHED_ALIENCONTROLLER_LAND) &&
		 !IsCurSchedule(SCHED_ALIENCONTROLLER_FIREBALL_EXTINGUISH) && m_iNumberOfPhysiscsEnts > 0 )
	{

		// Everything should be interrupted if we get killed.
		SetCustomInterruptCondition( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK );
	}
	else
	{
		ClearCustomInterruptCondition( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK );
	}

	BaseClass::BuildScheduleTestBits();
}

bool CNPC_AlienController::FindNearestPhysicsObjects( int iMaxMass )
{
	CBaseEntity		*pList[ ALIENCONTROLLER_PHYSICS_SEARCH_DEPTH ];
	CBaseEntity		*pNearest[ ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS ];
	float			flNearestDist[ ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS ];
	int				iIndex = 0;
	float			flDist;
	IPhysicsObject	*pPhysObj;
	int				i;
	Vector			vecDirToEnemy;
	Vector			vecDirToObject;
	Vector			vecEnemyForward;

	m_iNumberOfPhysiscsEnts = 0;
	for (i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		m_hPhysicsEnt.Set( i, NULL ); // = NULL;
		pNearest[i] = NULL;
	}

	//TERO: commented out because we want swats even in assaults when the Grunt may have not seen the player yet
	if ( !GetEnemy() || GetEnemy()->Classify() == CLASS_PLAYER_ALLY_VITAL)
	{
		// Can't swat, or no enemy, so no swat.
		return false;
	}

	vecDirToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
	float dist = VectorNormalize(vecDirToEnemy);
	vecDirToEnemy.z = 0;

	if( dist > ALIENCONTROLLER_PLAYER_MAX_SWAT_DIST )
	{
		// Player is too far away. Don't bother 
		// trying to swat anything at them until
		// they are closer.
		return false;
	}

	for (i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		//TERO: every object has to be closer than player or 1024
		flNearestDist[i] = min( dist * 2, ALIENCONTROLLER_FARTHEST_PHYSICS_OBJECT );
	}

	//DevMsg("minimum distance is %f\n", min( dist * 2, ALIENCONTROLLER_FARTHEST_PHYSICS_OBJECT ));

	//float flNearestDist = min( dist, ALIENCONTROLLER_FARTHEST_PHYSICS_OBJECT * 0.5 );

	Vector vecDelta( flNearestDist[0], flNearestDist[0], GetHullHeight() * 2 );

	class CAlienControllerSwatEntitiesEnum : public CFlaggedEntitiesEnum
	{
	public:
		CAlienControllerSwatEntitiesEnum( CBaseEntity **pList, int listMax, int iMaxMass )
		 :	CFlaggedEntitiesEnum( pList, listMax, 0 ),
			m_iMaxMass( iMaxMass )
		{
		}

		virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
		{
			CBaseEntity *pEntity = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
			if ( pEntity && 
				 pEntity->VPhysicsGetObject() && 
				 pEntity->VPhysicsGetObject()->GetMass() <= m_iMaxMass && 
				 pEntity->VPhysicsGetObject()->GetMass() >= ALIENCONTROLLER_MIN_PHYSOBJ_MASS &&
				 pEntity->VPhysicsGetObject()->IsAsleep() && 
				 pEntity->GetCollisionGroup() != COLLISION_GROUP_DEBRIS && //TERO: added, don't bother with objects that go through player
				 pEntity->VPhysicsGetObject()->IsMoveable() )
			{
				return CFlaggedEntitiesEnum::EnumElement( pHandleEntity );
			}
			return ITERATION_CONTINUE;
		}

		int m_iMaxMass;
	};

	CAlienControllerSwatEntitiesEnum swatEnum( pList, ALIENCONTROLLER_PHYSICS_SEARCH_DEPTH, iMaxMass );

	Vector vecCheckPos = GetEnemy()->GetAbsOrigin();
	GetEnemy()->GetVectors(&vecEnemyForward, NULL, NULL);

	//NDebugOverlay::Box( vecCheckPos, vecDelta, -vecDelta, 255, 0, 0, 0, 5 );

	int count = UTIL_EntitiesInBox( vecCheckPos - vecDelta, vecCheckPos + vecDelta, &swatEnum );
	Vector vecAlienControllerKnees;
	CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.25f ), &vecAlienControllerKnees );

	float flTelekinesisHeight = ALIENCONTROLLER_TELEKINESIS_MIN_HEIGHT; //clamp(m_flPreferredHeight, ALIENCONTROLLER_TELEKINESIS_MIN_HEIGHT, ALIENCONTROLLER_TELEKINESIS_MAX_HEIGHT);

	for( i = 0 ; i < count ; i++ )
	{
		pPhysObj = pList[ i ]->VPhysicsGetObject();

		Assert( !( !pPhysObj || pPhysObj->GetMass() > iMaxMass || !pPhysObj->IsAsleep() ) );

		Vector center = pList[ i ]->WorldSpaceCenter();
		flDist = UTIL_DistApprox2D( GetEnemy()->GetAbsOrigin(), center );

		iIndex = -1;
		for (int j=0; j<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; j++)
		{
			if (flDist < flNearestDist[j] && (iIndex == -1 || flNearestDist[iIndex] < flNearestDist[j]))
			{
				iIndex = j;
			}
		}

		//DevMsg("got slot %d\n", iIndex);

		if( iIndex == -1 )
			continue;

		vecDirToObject = pList[ i ]->WorldSpaceCenter() - GetEnemy()->GetAbsOrigin();
		vecDirToObject.z = 0;
		VectorNormalize(vecDirToObject);

		if( DotProduct( vecEnemyForward, vecDirToObject ) < 0.2 )
		{
			continue;
		}

		//if( flDist >= UTIL_DistApprox2D( center, GetEnemy()->GetAbsOrigin() ) )
		//	continue;


		//TERO: poista kommentit jos tarvit nuita!
		/*vcollide_t *pCollide = modelinfo->GetVCollide( pList[i]->GetModelIndex() );
		
		//Vector *objMins, *objMaxs;
		//physcollision->CollideGetAABB( objMins, objMaxs, pCollide->solids[0], pList[i]->GetAbsOrigin(), pList[i]->GetAbsAngles() );*/

		//if ( objMaxs.z < vecAlienControllerKnees.z )
		//	continue;

		if ( !FVisible( pList[i] ) )
		{
			//DevMsg("not visible!\n");
			continue;
		}

		// Skip things that the enemy can't see. Do we want this as a general thing? 
		// The case for this feature is that zombies who are pursuing the player will
		// stop along the way to swat objects at the player who is around the corner or 
		// otherwise not in a place that the object has a hope of hitting. This diversion
		// makes the zombies very late (in a random fashion) getting where they are going. (sjb 1/2/06)
		if( !GetEnemy()->FVisible( pList[i] ) )
		{
			//DevMsg("not visible to enemy!\n");
			continue;
		}

		// Make this the last check, since it makes a string.
		// Don't swat server ragdolls! -TERO: why the hell not? -because it crashes the game, dumbass! -oh, sorry...
		/*if ( FClassnameIs( pList[ i ], "physics_prop_ragdoll" ) )
			continue;
			
		if ( FClassnameIs( pList[ i ], "prop_ragdoll" ) )
			continue;*/

		//TERO: instead of not using ragdolls, we are not using anything that is not these
		//:		Why? because I was getting some weird results in-game... like the player being the physobject
		if ( !FClassnameIs( pList[ i ], "prop_physics" ) && !FClassnameIs( pList[ i ], "func_physbox" ))
		{
			continue;
		}

		/*Vector vMaxs = pList[i]->WorldAlignMaxs();
		Vector vMins = pList[i]->WorldAlignMins();*/

		Vector vMaxs;
		Vector vMins;
		vcollide_t *pCollide = modelinfo->GetVCollide( pList[i]->GetModelIndex() );
		physcollision->CollideGetAABB( &vMins, &vMaxs, pCollide->solids[0], Vector(0,0,0), QAngle(0,0,0) );

		trace_t	tr;
		AI_TraceHull( pList[i]->GetAbsOrigin() + Vector(0,0,flTelekinesisHeight), GetEnemy()->WorldSpaceCenter(), vMins, vMaxs, MASK_SOLID, pList[i], COLLISION_GROUP_NPC, &tr );

		if (tr.fraction < 0.8 && (!tr.m_pEnt || (IRelationType( tr.m_pEnt ) != D_HT)) )
		{
			if (g_debug_aliencontroller.GetInt() == 6)
			{
				NDebugOverlay::Line( pList[i]->GetAbsOrigin(), tr.endpos, 255, 0, 0, 0, 5 );
				NDebugOverlay::Box( tr.endpos, vMins, vMaxs, 255, 0, 0, 0, 5 );
			}
			continue;
		}

		if (g_debug_aliencontroller.GetInt() == 6)
		{
			NDebugOverlay::Line( pList[i]->GetAbsOrigin(), tr.endpos, 0, 255, 0, 0, 5 );
			NDebugOverlay::Box( tr.endpos, vMins, vMaxs, 0, 255, 0, 0, 5 );
		}


		// The object must also be closer to the zombie than it is to the enemy
		pNearest[iIndex]		= pList[ i ];
		flNearestDist[iIndex]	= flDist; //UTIL_DistApprox2D( GetAbsOrigin(), center );
	}

	m_iNumberOfPhysiscsEnts = 0;
	for (i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		m_hPhysicsEnt.Set( i, pNearest[i] );

		if (pNearest[i])
		{
			m_iNumberOfPhysiscsEnts++;

			m_vecPhysicsEntOrigin[i] = pNearest[i]->GetAbsOrigin() + Vector(0,0,flTelekinesisHeight);
			pNearest[i]->SetOwnerEntity(this); 
		}
	}

	if ( m_iNumberOfPhysiscsEnts > 0)
	{
		return true;
	}

	return false;
}

void CNPC_AlienController::StartObjectSounds()
{
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		if (m_hPhysicsEnt[i])
		{
			m_hPhysicsEnt[i]->EmitSound("NPC_Vortigaunt.StartHealLoop");
		}
	}
}


void CNPC_AlienController::PullObjects()
{
	m_iNumberOfPhysiscsEnts = 0;

	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		if ( !m_hPhysicsEnt[i] )
			continue;

		IPhysicsObject *pPhysObj = m_hPhysicsEnt[i]->VPhysicsGetObject();

		if ( !pPhysObj )
		{
			Warning("npc_aliencontroller: physics object with no phys object?!");
			m_hPhysicsEnt.Set( i, NULL );
			continue;
		}

		//update number of objects
		m_iNumberOfPhysiscsEnts++;

		Vector vDir = ( m_vecPhysicsEntOrigin[i] -  m_hPhysicsEnt[i]->WorldSpaceCenter() );
		float flDistance = vDir.Length();

		Vector vCurrentVel;
		float flCurrentVel;
		AngularImpulse vCurrentAI;

		pPhysObj->GetVelocity( &vCurrentVel, &vCurrentAI );
		flCurrentVel = vCurrentVel.Length();

		VectorNormalize( vCurrentVel );
		VectorNormalize( vDir );

		float flVelMod = ALIENCONTROLLER_PULL_VELOCITY_MOD;

		vCurrentVel = vCurrentVel * flCurrentVel * flVelMod;

		vCurrentAI = vCurrentAI * ALIENCONTROLLER_PULL_ANGULARIMP_MOD;
		pPhysObj->SetVelocity( &vCurrentVel, &vCurrentAI );

		vDir = vDir * flDistance * (ALIENCONTROLLER_PULL_TO_GUN_VEL_MOD * 2);

		Vector vAngle( 0, 0, 0 );
		pPhysObj->AddVelocity( &vDir, &vAngle );

	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_AlienController::NPCThink( void )
{
	BaseClass::NPCThink();


	//IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS ) && 
	if (IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS ) && m_iNumberOfPhysiscsEnts > 0)
	{
		PullObjects();

		if ( m_iNumberOfPhysiscsEnts <= 0)
		{
			m_flNextTelekinesisScan = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_SCAN_DELAY;
		}

		bool bCancel = true;
		
		if (IsCurSchedule( SCHED_ALIENCONTROLLER_TELEKINESIS ) && m_flNextTelekinesisThrow < gpGlobals->curtime &&
			GetEnemy() && GetEnemy()->Classify() != CLASS_PLAYER_ALLY_VITAL )
		{
			if (ThrowObject())
			{
				m_flNextTelekinesisThrow = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_DELAY;
				m_flNextTelekinesisCancel = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_CANCEL_DELAY;

				bCancel = false;
			}
			else
			{
				m_flNextTelekinesisThrow = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_NO_BEST_OBJECT_DELAY;
			}
		}
		
		if (bCancel)
		{
			if ( (m_flNextTelekinesisCancel < gpGlobals->curtime) || (GetEnemy() && GetEnemy()->Classify() == CLASS_PLAYER_ALLY_VITAL))
			{
				CancelObject();
				m_flNextTelekinesisCancel = gpGlobals->curtime + ALIENCONTROLLER_TELEKINESIS_CANCEL_DELAY;
			}
		}
	}
}

void CNPC_AlienController::CancelObject()
{
	float flWorstDist = 0;
	float flDist;
	int iIndex = -1;

	//TERO: lets pick up the one least likely to throw 
	if ( GetEnemy() )
	{
		for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
		{
			if ( m_hPhysicsEnt[i] )
			{
				//TERO: if the physics prop is not visible, we are adding 128.0f to its distance, 
				//		to make it more likely to be canceled than the ones that are visible
				flDist = UTIL_DistApprox2D( GetEnemy()->WorldSpaceCenter(), m_hPhysicsEnt[i]->WorldSpaceCenter() ) + (GetEnemy()->FVisible( m_hPhysicsEnt[i] )) ? 0.0f : 128.0f;
				if ( flDist > flWorstDist)
				{
					flWorstDist = flDist;
					iIndex = i;
				}
			}
		}
	}
	else
	{
		for (int i=0; ((i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS) && iIndex == -1); i++)
		{
			if (m_hPhysicsEnt[i])
			{
				iIndex = i;
			}
		}
	}

	//TERO: should not be possible, but to make sure
	if (iIndex == -1)
	{
		for (int i=0; ((i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS) && iIndex == -1); i++)
		{
			if (m_hPhysicsEnt[i])
			{
				DevMsg("Slot %d had an physics entity, yet we failed to get one to cancel\n");
				m_hPhysicsEnt.Set( i, NULL );
			}
		}

		m_iNumberOfPhysiscsEnts = 0;
		return;
	}

	m_hPhysicsEnt[iIndex]->StopSound("NPC_Vortigaunt.StartHealLoop");
	m_hPhysicsEnt[iIndex]->SetOwnerEntity( NULL );
	m_hPhysicsEnt.Set( iIndex, NULL );
	m_iNumberOfPhysiscsEnts--;
}

bool CNPC_AlienController::ThrowObject()
{
	DevMsg("npc_aliencontroller: throwing object\n");

	float flNearest = ALIENCONTROLLER_FARTHEST_PHYSICS_OBJECT;
	float flDist;
	int	iIndex = -1;

	if (!GetEnemy())
	{
		return false;
	}

	//TERO: lets pick the best one to throw
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		if ( m_hPhysicsEnt[i] )
		{
			if (GetEnemy()->FVisible( m_hPhysicsEnt[i] ))
			{

				flDist = UTIL_DistApprox2D( GetEnemy()->WorldSpaceCenter(), m_hPhysicsEnt[i]->WorldSpaceCenter() );
				if ( flDist < flNearest)
				{
					flNearest = flDist;
					iIndex = i;
				}
			}
			else
			{
				DevMsg("npc_aliencontroller: object from slot %d is not visible to our enemy, cannot throw\n");
			}
		}
	}

	if (iIndex == -1)
	{
		DevMsg("npc_aliencontroller: no best physics prop for throwing.\n");
		return false;
	}

	IPhysicsObject *pPhysObj = m_hPhysicsEnt[iIndex]->VPhysicsGetObject();

	if ( pPhysObj )
	{
		AngularImpulse angVelocity = RandomAngularImpulse( -250 , -250 ) / pPhysObj->GetMass();
			

		//pPhysObj->Wake();

		//Lets count predicted position for the enemy
		Vector vEnemyForward, vForward;

		GetEnemy()->GetVectors( &vEnemyForward, NULL, NULL );
		GetVectors( &vForward, NULL, NULL );

		float flDot = DotProduct( vForward, vEnemyForward );

		if ( flDot < 0.5f )
			 flDot = 0.5f;
	
		Vector vecPredictedPos;

		//Get our likely position in two seconds
		UTIL_PredictedPosition( GetEnemy(), flDot * 0.6f, &vecPredictedPos );

		//vecPredictedPos.z = vecPredictedPos.z + (GetEnemy()->BodyTarget(GetAbsOrigin(),true).z - GetEnemy()->GetAbsOrigin().z); 

		if ( g_debug_aliencontroller.GetInt() == 3 )
		{
			NDebugOverlay::Box( vecPredictedPos, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 0, 5 );
		}

		Vector vecDir = vecPredictedPos - m_hPhysicsEnt[iIndex]->WorldSpaceCenter();

		VectorNormalize( vecDir );

		vecDir = vecDir * CONTROLLER_THROW_SPEED;

		pPhysObj->SetVelocity( &vecDir, &angVelocity );
	}

	m_hPhysicsEnt[iIndex]->StopSound("NPC_Vortigaunt.StartHealLoop");
	m_hPhysicsEnt[iIndex]->EmitSound( "NPC_Vortigaunt.Swing" );
	m_hPhysicsEnt[iIndex]->SetOwnerEntity( NULL );
	m_hPhysicsEnt.Set( iIndex, NULL );
	m_iNumberOfPhysiscsEnts--;

	return true;
}

#endif

AI_BEGIN_CUSTOM_NPC( npc_aliencontroller, CNPC_AlienController )

	DECLARE_ANIMEVENT( AE_CONTROLLER_FIREGLOW )
	DECLARE_ANIMEVENT( AE_CONTROLLER_FIREGLOW_STOP )
	DECLARE_ANIMEVENT( AE_CONTROLLER_FIREBALL )
	DECLARE_ANIMEVENT( AE_CONTROLLER_START_LANDING )
	DECLARE_ANIMEVENT( AE_CONTROLLER_LAND )
	DECLARE_ANIMEVENT( AE_CONTROLLER_START_FLYING )
	DECLARE_ANIMEVENT( AE_CONTROLLER_FLY )
	DECLARE_ANIMEVENT( AE_CONTROLLER_SWING_SOUND )
	DECLARE_ANIMEVENT( AE_CONTROLLER_HAND )

	DECLARE_CONDITION( COND_ALIENCONTROLLER_FLY_BLOCKED )
	DECLARE_CONDITION( COND_ALIENCONTROLLER_CAN_SHOOT_FIREBALL )
#ifdef HLSS_CONTROLLER_TELEKINESIS
	DECLARE_CONDITION( COND_ALIENCONTROLLER_CAN_PHYS_ATTACK )
#endif
	DECLARE_CONDITION( COND_ALIENCONTROLLER_SHOULD_LAND )

#ifdef HLSS_CONTROLLER_TELEKINESIS
	DECLARE_TASK(TASK_ALIENCONTROLLER_TELEKINESIS)
#endif
	DECLARE_TASK(TASK_ALIENCONTROLLER_FIREBALL_EXTINGUISH)
	DECLARE_TASK(TASK_ALIENCONTROLLER_LAND)

#ifdef HLSS_CONTROLLER_TELEKINESIS
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_STAND_START)
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_STAND_STOP)
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_STAND_LOOP)
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_FLY_START)
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_FLY_STOP)
	DECLARE_ACTIVITY(ACT_CONTROLLER_TELEKINESIS_FLY_LOOP)
#endif
	DECLARE_ACTIVITY(ACT_CONTROLLER_LAND)
	DECLARE_ACTIVITY(ACT_CONTROLLER_LIFTOFF)

	//===============================================
	//	> SCHED_ALIENCONTROLLER_LEVITATE
	//===============================================
#ifdef HLSS_CONTROLLER_TELEKINESIS
	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_LEVITATE,

		"	Tasks"
		"		TASK_WAIT				5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_OCCLUDED"
		"		COND_ALIENCONTROLLER_CAN_PHYS_ATTACK"
		"		COND_ALIENCONTROLLER_SHOULD_LAND"
	);

	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_TELEKINESIS,

		"	Tasks"
		"		TASK_ALIENCONTROLLER_TELEKINESIS		0"
		"	"
		"	Interrupts"
	);

#else
	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_LEVITATE,

		"	Tasks"
		"		TASK_WAIT				5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_OCCLUDED"
		"		COND_ALIENCONTROLLER_SHOULD_LAND"
	);
#endif

	//=========================================================
	// > SCHED_ALIENGRUNT_RANGE_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_SHOOT_FIREBALL,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ALIENCONTROLLER_FIREBALL_EXTINGUISH"
		"		TASK_FACE_IDEAL					0"
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_WAIT						0.1" // Wait a sec before firing again
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_HEAVY_DAMAGE"
		"		COND_ALIENCONTROLLER_SHOULD_LAND"
	);

	//=========================================================
	// > SCHED_ALIENCONTROLLER_FIREBALL_EXTINGUISH
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_FIREBALL_EXTINGUISH,

		"	Tasks"
		"		TASK_ALIENCONTROLLER_FIREBALL_EXTINGUISH	0"
		""
		"	Interrupts"
	);

	//=========================================================
	// > SCHED_ALIENCONTROLLER_LAND
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ALIENCONTROLLER_LAND,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_ALIENCONTROLLER_LAND	0"
		"		TASK_WAIT					0.1"
		""
		"	Interrupts"
	);

	//=========================================================
	// SCHED_COMBINE_PRESS_ATTACK
	//=========================================================
	DEFINE_SCHEDULE 
	(
		SCHED_ALIENCONTROLLER_PRESS_ATTACK,

		"	Tasks "
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_SET_TOLERANCE_DISTANCE		72"
		"		TASK_GET_PATH_TO_ENEMY_LKP		0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts "
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_MOVE_AWAY"
	);

AI_END_CUSTOM_NPC()
