//========= Copyright © 2019 Overcharged ============//
//
// Purpose: Xen fauna angry tree.
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "ai_task.h"
#include "npcevent.h"
#include "bitstring.h"
#include "eventqueue.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "npcevent.h"
#include "game.h"
//#include "Sprite.h"
#include "soundenvelope.h"
#include "hl2_gamerules.h"
//#include "actanimating.h"
#include "baseanimating.h"
//#include "animation.h"
#include "shareddefs.h"
#include "shake.h"
#include "EntityList.h"
#include "activitylist.h"
//#include "soundscape.h"

#include "tier0/memdbgon.h"

ConVar sk_xentree_dmg_melee("sk_xentree_dmg_melee", "0");
// Tree Trigger
class CXenTreeTrigger : public CBaseEntity
{
	DECLARE_CLASS( CXenTreeTrigger, CBaseEntity );
public:
	void		Spawn(void);
	void		Touch( CBaseEntity *pOther );
	void		Think(void);
	static CXenTreeTrigger *TriggerCreate( CBaseEntity *pOwner, const Vector &position );

	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS( xen_treetrigger, CXenTreeTrigger );

BEGIN_DATADESC(CXenTreeTrigger)
DEFINE_ENTITYFUNC(Touch),
DEFINE_THINKFUNC(Think),
END_DATADESC()

CXenTreeTrigger *CXenTreeTrigger::TriggerCreate( CBaseEntity *pOwner, const Vector &position )
{
	CXenTreeTrigger *pTrigger = CREATE_ENTITY( CXenTreeTrigger, "xen_treetrigger" ); 

	//pTrigger->SetCollisionGroup(COLLISION_GROUP_PLAYER);
	pTrigger->SetSolid(SOLID_BBOX);
	pTrigger->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	pTrigger->SetMoveType(MOVETYPE_NONE);
	UTIL_SetSize(pTrigger, Vector(-10, -20, 0), Vector(110, 20, 128));

	return pTrigger;
}

void CXenTreeTrigger::Spawn(void)
{
	/*BaseClass::Spawn();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_NONE);
	UTIL_SetSize(this, Vector(-10, -15, 0), Vector(90, 15, 188));	//DispatchParticleEffect("grenade_spit_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	//SetCollisionGroup(COLLISION_GROUP_NPC);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);*/
	SetTouch(&CXenTreeTrigger::Touch);
	SetThink(&CXenTreeTrigger::Think);

	SetNextThink(gpGlobals->curtime);
}

void CXenTreeTrigger::Think(void)
{
	SetNextThink(gpGlobals->curtime + 5.1);
	if (GetOwnerEntity())
	{
		Vector _min, _max, outMin, outMax;
		_min = Vector(-10, -20, 0);
		_max = Vector(110, 20, 128);

		QAngle in2 = GetOwnerEntity()->GetAbsAngles();

		VectorRotate(_min, in2, outMin);
		VectorRotate(_max, in2, outMax);

		//this->SetParent(GetOwnerEntity());
		this->SetAbsOrigin(GetOwnerEntity()->GetAbsOrigin());
		this->SetAbsAngles(GetOwnerEntity()->GetAbsAngles());
		this->SetCollisionGroup(COLLISION_GROUP_NONE);
		this->SetSolid(SOLID_BBOX);
		this->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
		UTIL_SetSize(this, _min, _max);
		this->SetCollisionBounds(_min, _max);

		NDebugOverlay::Box(GetAbsOrigin(), _min, _max, 0, 255, 0, 255, 5.0);
	}


}

void CXenTreeTrigger::Touch( CBaseEntity *pOther )
{
	//SetNextThink(gpGlobals->curtime + 0.1);
	if (pOther && (pOther->IsNPC() || pOther->IsPlayer()) && GetOwnerEntity())
	{
		GetOwnerEntity()->Touch( pOther );
	}
}

// Tree itself
#define TREE_AE_ATTACK		1

class CXenTree : public CAI_BaseNPC
{
	DECLARE_CLASS(CXenTree, CAI_BaseNPC);
public:
	void		Spawn( void );
	void		Precache( void );
	bool		CreateVPhysics(void);
	void		Touch( CBaseEntity *pOther );
	void		Think( void );
	int			OnTakeDamage( const CTakeDamageInfo &info ) { Attack(); return 0; }
	void		HandleAnimEvent( animevent_t *pEvent );
	void		Attack( void );	
	Class_T		Classify(void) { return CLASS_BEE; }	// BriJee: Bee Temp Class

	DECLARE_DATADESC();

private:
	trace_t trEnd;
	int zOffset;
	CXenTreeTrigger	*m_pTrigger = nullptr;
};

LINK_ENTITY_TO_CLASS( npc_xentree, CXenTree );

BEGIN_DATADESC( CXenTree )
	DEFINE_FIELD( m_pTrigger, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flFieldOfView, FIELD_FLOAT ),
	DEFINE_THINKFUNC(Think),
	DEFINE_ENTITYFUNC(Touch),
END_DATADESC()

bool CXenTree::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

void CXenTree::Spawn( void )
{
	Precache();

	SetModel( "models/xen_tree.mdl" );
	SetMoveType( MOVETYPE_NONE );
	SetSolid ( SOLID_BBOX );

	m_takedamage = DAMAGE_YES;

	//UTIL_SetSize( this, Vector(-10,-15,0), Vector(90,15,188));
	//UTIL_SetSize(this, Vector(-10, -20, 0), Vector(35, 20, 188));
	UTIL_SetSize(this, Vector(-20, -20, 0), Vector(20, 20, 188));
	SetActivity( ACT_IDLE );											//debug1
	SetNextThink( gpGlobals->curtime + 0.1 );
	SetCycle( random->RandomFloat( 0,1 ) );
	m_flPlaybackRate = random->RandomFloat( 0.7, 1.4 );

	m_flFieldOfView = 0.92f;

	SetThink(&CXenTree::Think);

	SetTouch(&CXenTree::Touch);

	/*Vector triggerPosition, vForward;
	SetBloodColor(BLOOD_COLOR_YELLOW);
	AngleVectors( GetAbsAngles(), &vForward );
	triggerPosition = GetAbsOrigin() + (vForward * 64);
	
	m_pTrigger = m_pTrigger->TriggerCreate((CBaseEntity*)this, triggerPosition);
	m_pTrigger->Precache();
	gEntList.NotifyCreateEntity(m_pTrigger);
	DispatchSpawn(m_pTrigger);
	m_pTrigger->Activate();
	m_pTrigger->SetAbsOrigin(GetAbsOrigin());
	m_pTrigger->SetAbsAngles(GetAbsAngles());
	m_pTrigger->SetOwnerEntity(this);*/

	//UTIL_SetSize( m_pTrigger, Vector( -128, -128, -128 ), Vector( 128, 128, 128 ) );
	zOffset = -40;
}

void CXenTree::Precache( void )
{
	PrecacheModel( "models/xen_tree.mdl" );
	//PrecacheModel( XEN_PLANT_GLOW_SPRITE );

	PrecacheScriptSound( "XenTree.MeleeMiss" );
	PrecacheScriptSound( "XenTree.MeleeHit" );
}


void CXenTree::Touch( CBaseEntity *pOther )
{
	/*if ( !pOther->IsPlayer() && FClassnameIs( pOther, "npc_bigmomma" ) )
		return;*/

	Attack();
}


void CXenTree::Attack( void )
{
	if (GetActivity() != ACT_MELEE_ATTACK1)
	{
		this->SetActivity( ACT_MELEE_ATTACK1 );										//debug1
		m_flPlaybackRate = random->RandomFloat( 1.0, 1.4 );

		CPASAttenuationFilter filter( this );
		EmitSound( filter, entindex(), "XenTree.MeleeMiss" );
	}
}

#include "grenade_strooper_instant.h"
void CXenTree::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
		case TREE_AE_ATTACK:
		{
			//CBaseEntity *pList[512];
			//BOOL sound = FALSE;
			//int count = UTIL_EntitiesInBox(pList, sizeof(pList), m_pTrigger->GetAbsOrigin() + m_pTrigger->WorldAlignMins(), m_pTrigger->GetAbsOrigin() + m_pTrigger->WorldAlignMaxs(), FL_NPC | FL_CLIENT);

			Vector forward, right, up;
			AngleVectors(GetAbsAngles(), &forward, &right, &up);

			trace_t	tr;
			/*AI_TraceHull(WorldSpaceCenter() + (forward * 105) + (up * 50), WorldSpaceCenter() + (forward * 105) - (up * 300), Vector(-34, -34, -34), Vector(34, 34, 34), MASK_NPCSOLID, this, COLLISION_GROUP_INTERACTIVE_DEBRIS, &tr);
			debugoverlay->AddLineOverlay(WorldSpaceCenter() + (forward * 105) + (up * 50), WorldSpaceCenter() + (forward * 105) - (up * 300), 0, 255, 0, false, 10);
			NDebugOverlay::Box(WorldSpaceCenter() + (forward * 105) + (up * 50), Vector(-34, -34, -34), Vector(34, 34, 34), 0, 255, 0, true, 10.0);
			if (tr.m_pEnt != NULL)// && tr.m_pEnt->m_takedamage == DAMAGE_YES)
			{
				tr.m_pEnt->TakeDamage(CTakeDamageInfo(this, this, 25, DMG_FALL));
				tr.m_pEnt->SetDamage(25);
			}*/


			CBaseEntity *pHurt = CheckTraceHullAttack(WorldSpaceCenter() + (forward * 105) + (up * 50), WorldSpaceCenter() + (forward * 105) - (up * 300), Vector(-22, -22, -20), Vector(22, 22, 20), sk_xentree_dmg_melee.GetInt(), DMG_SLASH);
			if (pHurt)
			{
				Vector forward, up;
				AngleVectors(GetAbsAngles(), &forward, NULL, &up);
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (forward * 100));
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (up * 100));
				pHurt->SetGroundEntity(NULL);
				//pHurt->SetDamage(25);
			}

			//if ( sound )
			{
				CPASAttenuationFilter filter( this );
				EmitSound( filter, entindex(), "XenTree.MeleeHit" );
			}
		}
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

void CXenTree::Think( void )
{
	StudioFrameAdvance();
	SetNextThink( gpGlobals->curtime + 0.1 );
	DispatchAnimEvents( this );

	//CBaseEntity *pEnt = NULL;
	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);
	forward.x += 80;

	CBaseEntity *pEnt[256];
	int nEntCount = UTIL_EntitiesInSphere(pEnt, 256, this->GetAbsOrigin(), 115.f, 0);

	//pEnt = gEntList.FindEntityInSphere(this, GetAbsOrigin(), 115.f);
	//pEnt = gEntList.FindCharacterNearestFacing(this->GetAbsOrigin(), this->GetAbsOrigin() + forward * 50, 0.3f);
	//if (pEnt && pEnt != this && (pEnt->IsPlayer() || pEnt->IsNPC()))
	for (int i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (pEnt[i] != NULL && pEnt[i] != this && (pEnt[i]->IsPlayer() || pEnt[i]->IsNPC()))
		{
			/*trace_t tr;

			Vector forward, right, up;

			Vector dir = BodyDirection3D();

			AngleVectors(GetAbsAngles(), &forward, &right, &up);
			forward.x += 80;
			up.z += 0;
			right.y += 80;

			Vector middle = GetAbsOrigin();// +up;
			//CheckPVSCondition

			UTIL_TraceHull(middle, middle + dir * -60, Vector(-10, -10, 0), Vector(10, 10, 100), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
			debugoverlay->AddLineOverlay(middle, middle + forward, 0, 255, 0, false, 1);

			//debugoverlay->AddBoxOverlay2(WorldSpaceCenter() + (forward * 105) + (up * 50), WorldSpaceCenter() + (forward * 105) - (up * 300), Vector(-10, -10, 0), Vector(10, 10, 100), 0, 255, 0, false, 1);
			if (tr.m_pEnt != NULL && (tr.m_pEnt->IsPlayer() || tr.m_pEnt->IsNPC()) && GetActivity() != ACT_MELEE_ATTACK1)*/
			if (FInViewCone(pEnt[i]))
				Attack();
		}
	}


	switch( GetActivity() )
	{
	case ACT_MELEE_ATTACK1:
		if ( IsSequenceFinished() )
		{
			SetActivity( ACT_IDLE );											//debug1
			m_flPlaybackRate = random->RandomFloat( 0.6f, 1.4f );
		}
		break;

	default:
	case ACT_IDLE:
		break;

	}
}
