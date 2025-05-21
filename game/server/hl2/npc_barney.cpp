//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"

#include "BloodDrips.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BARNEY_MODEL "models/barney.mdl"

ConVar	sk_barney_health( "sk_barney_health","0");

//=========================================================
// Barney activities
//=========================================================

class CNPC_Barney : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Barney, CNPC_PlayerCompanion );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache()
	{
		// Prevents a warning
		//SelectModel( );						// BJ: over rework, custom model

		PrecacheModel(STRING(GetModelName()));	// BJ: over rework, custom model

		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
		PrecacheScriptSound( "NPC_Barney.FootstepRight" );
		PrecacheScriptSound( "NPC_Barney.Die" );

		PrecacheInstancedScene( "scenes/Expressions/BarneyIdle.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/BarneyAlert.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/BarneyCombat.vcd" );
	}

	void	Spawn( void );
	//void	SelectModel();		// BJ: over rework, custom model
	Class_T Classify( void );
	void	Weapon_Equip( CBaseCombatWeapon *pWeapon );

	bool CreateBehaviors( void );
	int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void HandleAnimEvent( animevent_t *pEvent );

	bool ShouldLookForBetterWeapon() { return false; }

	void OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

	void DeathSound( const CTakeDamageInfo &info );
	void GatherConditions();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_barney, CNPC_Barney );

//---------------------------------------------------------
// 
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Barney, DT_NPC_Barney)
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Barney )
//						m_FuncTankBehavior
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

// BJ: over rework, custom model
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Barney::SelectModel()
{
	SetModelName( AllocPooledString( BARNEY_MODEL ) );
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Barney::Spawn( void )
{
	
	// Allow custom model usage
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = BARNEY_MODEL;
		SetModelName(AllocPooledString(szModel));
	}
	
	Precache();

	m_iHealth = 80;

	m_iszIdleExpression = MAKE_STRING("scenes/Expressions/BarneyIdle.vcd");
	m_iszAlertExpression = MAKE_STRING("scenes/Expressions/BarneyAlert.vcd");
	m_iszCombatExpression = MAKE_STRING("scenes/Expressions/BarneyCombat.vcd");

	BaseClass::Spawn();

	SetModel(szModel);	// Allow custom model usage

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	if (!FStrEq(gpGlobals->mapname.ToCStr(), "d1_trainstation_06"))
		CapabilitiesAdd(bits_CAP_MOVE_JUMP);

	NPCInit();

	SetUse( &CNPC_Barney::UseFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Barney::Classify( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );

	if( hl2_episodic.GetBool() && FClassnameIs( pWeapon, "weapon_ar2" ) )
	{
		// Allow Barney to defend himself at point-blank range in c17_05.
		pWeapon->m_fMinRange1 = 0.0f;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Barney::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_Barney.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_Barney.FootstepRight", pEvent->eventtime );
		}
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Barney::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound( "npc_barney.die" );

}

int CNPC_Barney::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;
	/*if (cvar->FindVar("oc_ragdoll_enable_blooddrips")->GetInt())
	{
		int MaxDrips = RandomInt(4, 12);
		Vector	vecToss, vecTraceDir;
		if (info.GetDamageType() & (DMG_BULLET | DMG_SHOCK))
		{
			for (int i = 0; i < MaxDrips; i++)
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



				CBloodDrips *pGrenade = (CBloodDrips*)CreateEntityByName("BloodDrips");
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

bool CNPC_Barney::CreateBehaviors( void )
{
	BaseClass::CreateBehaviors();
	AddBehavior( &m_FuncTankBehavior );

	return true;
}

void CNPC_Barney::OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior )
{
	if ( pNewBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = false;
	}
	else if ( pOldBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior( pOldBehavior, pNewBehavior );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		( ( m_NPCState == NPC_STATE_SCRIPT ) && CanSpeakWhileScripting() ) )
	{
		DoCustomSpeechAI();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed( TLK_USE );
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput( pActivator, pCaller );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_barney, CNPC_Barney )

AI_END_CUSTOM_NPC()
