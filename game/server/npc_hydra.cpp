//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ==========
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"

#include "npc_hydra.h"
#include "npc_antliongrub.h"

#include "ai_hull.h"
#include "saverestore_utlvector.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "vcollide_parse.h"
#include "ragdoll_shared.h"
#include "physics_prop_ragdoll.h"

//-----------------------------------------------------------------------------
//
// CNPC_Hydra
//

#define HYDRA_MAX_LENGTH	500
#define HYDRA_MIN_STAB_HOLD_TIME	3.5f
#define HYDRA_MAX_STAB_HOLD_TIME	10.0f

LINK_ENTITY_TO_CLASS(npc_hydra, CNPC_Hydra);

//=========================================================
// Hydra activities
//=========================================================
int ACT_HYDRA_COWER;
int ACT_HYDRA_STAB;

//=========================================================
// Private conditions
//=========================================================

//==================================================
// AntlionConditions
//==================================================

enum
{
	COND_HYDRA_SNAGGED = LAST_SHARED_CONDITION,
	COND_HYDRA_STUCK,
	COND_HYDRA_OVERSHOOT,
	COND_HYDRA_OVERSTRETCH, // longer than max distance
	COND_HYDRA_STRIKE,		// head hit something
	COND_HYDRA_NOSTUCK		// no segments are stuck
};

//=========================================================
// Hydra schedules
//=========================================================
enum
{
	SCHED_HYDRA_DEPLOY = LAST_SHARED_SCHEDULE,
	SCHED_HYDRA_RETRACT,
	SCHED_HYDRA_IDLE,
	SCHED_HYDRA_STAB,		// shoot out head and try to hit object
	SCHED_HYDRA_PULLBACK,	// 
	SCHED_HYDRA_TIGHTEN_SLACK,	// snagged on something, tighten slack up to obstacle and try again from there
	SCHED_HYDRA_RETREAT,
	SCHED_HYDRA_THROW,
	SCHED_HYDRA_RANGE_ATTACK
};

//=========================================================
// Hydra tasks
//=========================================================
enum
{
	TASK_HYDRA_RETRACT = LAST_SHARED_TASK,
	TASK_HYDRA_DEPLOY,
	TASK_HYDRA_GET_OBJECT,
	TASK_HYDRA_THROW_OBJECT,
	TASK_HYDRA_PREP_STAB,
	TASK_HYDRA_STAB,
	TASK_HYDRA_PULLBACK,
	TASK_HYDRA_SET_MAX_TENSION,
	TASK_HYDRA_SET_BLEND_TENSION

};


//---------------------------------------------------------
// Custom Client entity
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Hydra, DT_NPC_Hydra)
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 0), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 1), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 2), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 3), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 4), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 5), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 6), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 7), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 8), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 9), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 10), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 11), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 12), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 13), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 14), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 15), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 16), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 17), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 18), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 19), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 20), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 21), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 22), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 23), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 24), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 25), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 26), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 27), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 28), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 29), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 30), -1, SPROP_COORD),
SendPropVector(SENDINFO_NETWORKARRAYELEM(m_vecChain, 31), -1, SPROP_COORD),
SendPropVector(SENDINFO(m_vecHeadDir), -1, SPROP_NORMAL),
SendPropFloat(SENDINFO(m_flRelaxedLength), 12, 0, 0.0, HYDRA_MAX_LENGTH * 1.5),
SendPropBool(SENDINFO(ChangeColor)),
SendPropBool(SENDINFO(PainColor)),
SendPropBool(SENDINFO(blinking)),
SendPropInt(SENDINFO(blinking_type)),
END_SEND_TABLE()

ConVar	sk_hydra_health("sk_hydra_health", "20");
ConVar	sk_hydra_stab_damage("sk_hydra_stab_damage", "12");

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Hydra)

DEFINE_AUTO_ARRAY(m_vecChain, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_activeChain, FIELD_INTEGER),
DEFINE_FIELD(m_bHasStuckSegments, FIELD_BOOLEAN),
DEFINE_FIELD(m_flCurrentLength, FIELD_FLOAT),
DEFINE_FIELD(m_vecHeadGoal, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_flHeadGoalInfluence, FIELD_FLOAT),
DEFINE_FIELD(m_vecHeadDir, FIELD_VECTOR),
DEFINE_FIELD(m_flRelaxedLength, FIELD_FLOAT),
DEFINE_FIELD(m_vecOutward, FIELD_VECTOR),
DEFINE_UTLVECTOR(m_body, FIELD_EMBEDDED),
DEFINE_FIELD(m_idealLength, FIELD_FLOAT),
DEFINE_FIELD(m_idealSegmentLength, FIELD_FLOAT),
DEFINE_FIELD(m_bExtendSoundActive, FIELD_BOOLEAN),
DEFINE_SOUNDPATCH(m_pExtendTentacleSound),
DEFINE_FIELD(m_seed, FIELD_FLOAT),
DEFINE_FIELD(m_vecTarget, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecTargetDir, FIELD_VECTOR),
DEFINE_FIELD(m_flLastAdjustmentTime, FIELD_TIME),
DEFINE_FIELD(m_flTaskStartTime, FIELD_TIME),
DEFINE_FIELD(m_flTaskEndTime, FIELD_TIME),
DEFINE_FIELD(m_flLengthTime, FIELD_TIME),
DEFINE_FIELD(m_bStabbedEntity, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDied, FIELD_BOOLEAN), // VXP
DEFINE_FIELD(m_flDieTime, FIELD_TIME), // VXP
DEFINE_FIELD(m_flNextStabTime, FIELD_TIME), // VXP
DEFINE_KEYFIELD(blinking, FIELD_BOOLEAN, "Hydra_blinking"),
DEFINE_KEYFIELD(blinking_type, FIELD_INTEGER, "Hydra_blinking_type"),
END_DATADESC()


//-------------------------------------

BEGIN_SIMPLE_DATADESC(HydraBone)
DEFINE_FIELD(vecPos, FIELD_POSITION_VECTOR),
DEFINE_FIELD(vecDelta, FIELD_VECTOR),
DEFINE_FIELD(flIdealLength, FIELD_FLOAT),
DEFINE_FIELD(flActualLength, FIELD_FLOAT),
DEFINE_FIELD(bStuck, FIELD_BOOLEAN),
DEFINE_FIELD(bOnFire, FIELD_BOOLEAN),
DEFINE_FIELD(vecGoalPos, FIELD_POSITION_VECTOR),
DEFINE_FIELD(flGoalInfluence, FIELD_FLOAT),
END_DATADESC()

//-------------------------------------

//static ConVar	sv_hydraMinTension( "hydra_min_tension", "1", FCVAR_ARCHIVE | FCVAR_SERVER, "Hydra Slack" );
//static ConVar	sv_hydraLengthTension( "hydra_length_tension", "1", FCVAR_ARCHIVE | FCVAR_SERVER, "Hydra Slack" );

static ConVar	sv_hydraLength("hydra_length", "100", FCVAR_ARCHIVE, "Hydra Length");
static ConVar	sv_hydraSlack("hydra_slack", "200", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	sv_hydraSegmentLength("hydra_segment_length", "30", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	sv_hydraTest("hydra_test", "1", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	sv_hydraBendTension("hydra_bend_tension", "0.4", FCVAR_ARCHIVE, "Hydra Slack");
static ConVar	sv_hydraBendDelta("hydra_bend_delta", "50", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	sv_hydraGoalTension("hydra_goal_tension", "0.5", FCVAR_ARCHIVE, "Hydra Slack");
static ConVar	sv_hydraGoalDelta("hydra_goal_delta", "400", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	sv_hydraMomentum("hydra_momentum", "0.5", FCVAR_ARCHIVE, "Hydra Slack");

static ConVar	oc_hydra_test_attachment("oc_hydra_test_attachment", "0", FCVAR_ARCHIVE, "Hydra Test impaling code");

//-------------------------------------
// Purpose: Initialize the custom schedules
//-------------------------------------
CNPC_Hydra::CNPC_Hydra()
{
	ChangeColor = false;
	PainColor = false;
	Inc = 0.0f;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
CNPC_Hydra::~CNPC_Hydra()
{
}



//-------------------------------------

void CNPC_Hydra::Precache()
{
	engine->PrecacheModel("models/Hydra.mdl");
	engine->PrecacheModel("models/hydrasmaacker.mdl");
	UTIL_PrecacheOther("hydra_impale");
	//PrecacheScriptSound("NPC_Hydra.HeartbeatIdle");
	PrecacheScriptSound("NPC_Hydra.HeartbeatFast");
	PrecacheScriptSound("NPC_Hydra.BreatheIdle");
	PrecacheScriptSound("NPC_Hydra.BreatheFast");
	//EmitSound("NPC_Hydra.Search");
	//EmitSound("NPC_Hydra.HeartbeatIdle");
	PrecacheScriptSound("NPC_Hydra.Alert");
	PrecacheScriptSound("NPC_Hydra.Pain");
	PrecacheScriptSound("NPC_Hydra.Search");
	PrecacheScriptSound("NPC_Hydra.ExtendTentacle");
	PrecacheScriptSound("NPC_Hydra.Attack");
	PrecacheScriptSound("NPC_Hydra.Bump");

	//CreateSounds();

	BaseClass::Precache();
}


void CNPC_Hydra::Activate(void)
{
	CPASAttenuationFilter filter(this);
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	m_pExtendTentacleSound = controller.SoundCreate(filter, entindex(), "NPC_Hydra.ExtendTentacle");

	controller.Play(m_pExtendTentacleSound, 1.0, 100);
	//StartSounds();
	BaseClass::Activate();
}


//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Hydra::Classify(void)
{
	//return CLASS_BARNACLE;//(blinking) ? CLASS_PLAYER_ALLY : 
	return CLASS_HYDRA;
}

//-------------------------------------

#define HYDRA_OUTWARD_BIAS	16
#define HYDRA_INWARD_BIAS	30

void CNPC_Hydra::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetModel("models/Hydra.mdl");

	SetHullType(HULL_HUMAN);
	
	//Vector vecSurroundingMins(-280, -280, -280);
	//Vector vecSurroundingMaxs(280, 280, 280);
	//CollisionProp()->SetSurroundingBoundsType(USE_HITBOXES, &vecSurroundingMins, &vecSurroundingMaxs);
	
	SetHullSizeNormal();
	//SetSolid(SOLID_BBOX);
	
	SetSolid(SOLID_BSP);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	//m_fEffects = 0;
	m_iHealth = sk_hydra_health.GetFloat();
	m_iMaxHealth = m_iHealth;
	m_flFieldOfView = -1.0;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;
	//CapabilitiesAdd( bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_MOVE_GROUND | bits_CAP_MOVE_CLIMB );
	// CapabilitiesAdd( bits_CAP_USE_WEAPONS );
	// CapabilitiesAdd( bits_CAP_ANIMATEDFACE );

	GetVectors(NULL, NULL, &m_vecOutward);

	SetAbsAngles(QAngle(0, 0, 0));

	m_vecChain.Set(0, GetAbsOrigin() - m_vecOutward * 32);
	m_vecChain.Set(1, GetAbsOrigin() + m_vecOutward * 16);

	m_vecHeadGoal = m_vecChain[1] + m_vecOutward * HYDRA_OUTWARD_BIAS;
	m_vecHeadDir = Vector(0, 0, 1);

	// init bones
	HydraBone bone;
	bone.vecPos = GetAbsOrigin() - m_vecOutward * HYDRA_INWARD_BIAS;
	m_body.AddToTail(bone);
	bone.vecPos = m_vecChain[1];
	m_body.AddToTail(bone);
	bone.vecPos = m_vecHeadGoal;
	m_body.AddToTail(bone);
	bone.vecPos = m_vecHeadGoal + m_vecHeadDir;
	m_body.AddToTail(bone);

	m_idealSegmentLength = sv_hydraSegmentLength.GetFloat();

	for (int i = 2; i < CHAIN_LINKS; i++)
	{
		m_vecChain.Set(i, m_vecChain[i - 1]);
	}

	m_seed = random->RandomFloat(0.0, 2000.0);

	NPCInit();

	//	m_takedamage = DAMAGE_NO;
	m_takedamage = DAMAGE_YES;

	m_bDied = false;
	m_flDieTime = 0;
	m_flNextStabTime = gpGlobals->curtime;
	ChangeColor = false;
	PainColor = false;
	//EmitSound("NPC_Hydra.HeartbeatIdle");
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//-----------------------------------------------------------------------------
void CNPC_Hydra::DeathSound(void)
{
	EmitSound("NPC_Hydra.Pain");

}

//-------------------------------------



void CNPC_Hydra::RunAI(void)
{
	//CheckLength();

	//AdjustLength();
	//DrawServerHitboxes(0.1f);
	//TestHitboxes
	//CStudioHdr *pStudioHdr = GetModelPtr();
	//mstudiohitboxset_t *set = pStudioHdr->pHitboxSet(m_nHitboxSet);
	//pStudioHdr->pHitboxSet(1)->pHitbox(1);
	
	BaseClass::RunAI();

	CalcGoalForces();
	MoveBody();

	/*int i;
	for (i = 1; i < CHAIN_LINKS && i < m_body.Count(); i++)
	{
		m_vecChain.Set(i, m_body[i].vecPos);

#if 0
		if (m_body[i].bStuck)
		{
			NDebugOverlay::Box(m_body[i].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 255, 0, 0, 20, .1);
		}
		else
		{
			NDebugOverlay::Box(m_body[i].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .1);
		}
		NDebugOverlay::Line(m_body[i].vecPos, m_body[i].vecPos + m_body[i].vecDelta, 0, 255, 0, true, .1);
		NDebugOverlay::Line(m_body[i - 1].vecPos, m_body[i].vecPos, 255, 255, 255, true, .1);
#endif

#if 0
		char text[128];
		Q_snprintf(text, sizeof(text), "%d", i);
		NDebugOverlay::Text(m_body[i].vecPos, text, false, 0.1);
#endif

#if 0
		char text[128];
		Q_snprintf(text, sizeof(text), "%4.0f", (m_body[i].vecPos - m_body[i - 1].vecPos).Length() * 100 / m_idealSegmentLength - 100);
		NDebugOverlay::Text(0.5*(m_body[i - 1].vecPos + m_body[i].vecPos), text, false, 0.1);
#endif
	}
	//NDebugOverlay::Box(m_body[i].vecPos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 255, 0, 20, .1);
	//NDebugOverlay::Box( m_vecHeadGoal, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 255, 255, 0, 20, .1);
	for (; i < CHAIN_LINKS; i++)
	{
		m_vecChain.Set(i, m_vecChain[i - 1]);
	}
	DevMsg("m_vecChain: %.2f \n", m_vecChain);
	SetNextThink(gpGlobals->curtime + oc_hydra_runtime.GetFloat());*/

	SendCoordinatesToClient();
}
ConVar	oc_hydra_runtime("oc_hydra_runtime", "0.05", FCVAR_ARCHIVE);
void CNPC_Hydra::SendCoordinatesToClient(void)
{
	CheckLength();

	AdjustLength();

	/*CalcGoalForces();
	MoveBody();*/

	//���� ����� ���������� �� m_vecChain//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//CStudioHdr *pStudioHdr = GetModelPtr();

	/*Vector local;
	mstudiobone_t *pBone64 = pStudioHdr->pBone(i);
	WorldToEntitySpace(m_body[i].vecPos, &pBone64->pos);
	mstudiobone_t *pBone63 = pStudioHdr->pBone(i - 1);
	WorldToEntitySpace(m_body[i - 2].vecPos, &pBone63->pos);
	mstudiobone_t *pBone63 = pStudioHdr->pBone(i - 1);
	WorldToEntitySpace(m_body[i - 2].vecPos, &pBone63->pos);*/

	//Quaternion qt;

	//mstudiobone_t *pBone00 = NULL;
	/*mstudiobone_t *pBone01 = NULL;
	mstudiobone_t *pBone02 = NULL;
	mstudiobone_t *pBone03 = NULL;
	mstudiobone_t *pBone04 = NULL;*/
	//int h;
	//SetupBones
	//DevMsg("m_body.Count: %2f \n", m_body.Count());
	int i;
	//int j = m_body.Count();
	for (i = 1; i < CHAIN_LINKS && i < m_body.Count(); i++)
	{
		m_vecChain.Set(i, m_body[i].vecPos);

		/*DrawServerHitboxes(0.05);

		pBone00 = pStudioHdr->pBone(i);
		WorldToEntitySpace((m_body[i - 1].vecPos + m_body[i].vecPos) / 2, &pBone00->pos);*/

		/*DevMsg("pStudioHdr->BoneFlexDriverCount(): %i \n", pStudioHdr->BoneFlexDriverCount());
		DevMsg("pStudioHdr->numhitboxsets(): %i \n", pStudioHdr->numhitboxsets());
		DevMsg("pStudioHdr->iHitboxCount(i): %i \n", pStudioHdr->iHitboxCount(i));*/

		//pStudioHdr->BoneFlexDriverCount();
		//for (int h = i; h < i + 5; h++)
		{
			//j += i;

			/*for (int j = 1; j <= 25; j++)
			{
				h = i - 1;

				pBone01 = pStudioHdr->pBone(h + j - 1);
				WorldToEntitySpace(m_body[h + j - 1].vecPos, &pBone01->pos);

				pBone00 = pStudioHdr->pBone(h + j);
				WorldToEntitySpace((m_body[h + j].vecPos + m_body[h + j - 1].vecPos) / 2, &pBone00->pos);

				pBone02 = pStudioHdr->pBone(h + j + 1);
				WorldToEntitySpace(m_body[h + j + 1].vecPos, &pBone02->pos);

				pBone04 = pStudioHdr->pBone(h + j + 2);
				WorldToEntitySpace((m_body[h + j].vecPos + m_body[h + j + 1].vecPos) / 2, &pBone04->pos);

				pBone03 = pStudioHdr->pBone(h + j + 3);
				WorldToEntitySpace(m_body[h + j + 2].vecPos, &pBone03->pos);

				//NDebugOverlay::Box(m_body[h + j - 1].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
				//NDebugOverlay::Box((m_body[h].vecPos + m_body[h + 1].vecPos) / 2, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			}*/

			/*mstudiobone_t *pBone = pStudioHdr->pBone(i);
			mstudiobone_t *pBone0 = pStudioHdr->pBone(i + 1);
			mstudiobone_t *pBone1 = pStudioHdr->pBone(i + 2);
			mstudiobone_t *pBone2 = pStudioHdr->pBone(i + 3);
			mstudiobone_t *pBone3 = pStudioHdr->pBone(i + 4);
			mstudiobone_t *pBone4 = pStudioHdr->pBone(i + 5);
			mstudiobone_t *pBone5 = pStudioHdr->pBone(i + 6);
			mstudiobone_t *pBone6 = pStudioHdr->pBone(i + 7);

			//Vector res = m_body[i].vecPos - m_body[i - 1].vecPos;
			WorldToEntitySpace(m_body[i].vecPos, &pBone->pos);
			WorldToEntitySpace((m_body[i].vecPos + m_body[i+1].vecPos)/2, &pBone0->pos);
			WorldToEntitySpace(m_body[i + 1].vecPos, &pBone1->pos);
			WorldToEntitySpace((m_body[i + 1].vecPos + m_body[i + 2].vecPos) / 2, &pBone2->pos);
			WorldToEntitySpace(m_body[i + 2].vecPos, &pBone3->pos);
			WorldToEntitySpace((m_body[i + 3].vecPos + m_body[i + 4].vecPos) / 2, &pBone4->pos);
			WorldToEntitySpace(m_body[i + 4].vecPos, &pBone5->pos);
			WorldToEntitySpace((m_body[i + 4].vecPos + m_body[i + 5].vecPos) / 2, &pBone6->pos);*/


			/*NDebugOverlay::Box(m_body[i].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box((m_body[i].vecPos + m_body[i + 1].vecPos) / 2, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box(m_body[i + 1].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box((m_body[i + 1].vecPos + m_body[i + 2].vecPos) / 2, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box(m_body[i + 2].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box((m_body[i + 2].vecPos + m_body[i + 3].vecPos) / 2, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box(m_body[i + 3].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box((m_body[i + 3].vecPos + m_body[i + 4].vecPos) / 2, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);
			NDebugOverlay::Box(m_body[i + 4].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .05f);*/
			//NDebugOverlay::Box(m_vecHeadGoal, Vector(-2, -2, -2), Vector(2, 2, 2), 255, 255, 0, 20, .05f);

			/*WorldToEntitySpace(m_body[i].vecPos, &pBone1->pos);
			WorldToEntitySpace(m_body[i].vecPos, &pBone2->pos);
			WorldToEntitySpace(m_body[i].vecPos, &pBone3->pos);
			WorldToEntitySpace(m_body[i].vecPos, &pBone4->pos);
			WorldToEntitySpace(m_body[i].vecPos, &pBone5->pos);*/
		}

		/*mstudiobone_t *pBone = pStudioHdr->pBone(i);
		mstudiobone_t *pBone0 = pStudioHdr->pBone(i - 1);

		Vector res = m_body[i].vecPos - m_body[i - 1].vecPos;
		//pBone->pos = res;
		WorldToEntitySpace(m_body[i].vecPos, &pBone->pos);
		WorldToEntitySpace(m_body[i].vecPos, &pBone0->pos);*/

		/*BasisToQuaternion(Vector(res.x, 0, 0), Vector(0, res.y, 0), Vector(0, 0, res.z), qt);
		pBone->qAlignment = qt;
		pBone->quat = qt;
		pBone0->qAlignment = qt;
		pBone0->quat = qt;*/
#if 0
		if (m_body[i].bStuck)
		{
			NDebugOverlay::Box(m_body[i].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 255, 0, 0, 20, .1);
		}
		else
		{
			NDebugOverlay::Box(m_body[i].vecPos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 255, 0, 20, .1);
		}
		NDebugOverlay::Line(m_body[i].vecPos, m_body[i].vecPos + m_body[i].vecDelta, 0, 255, 0, true, .1);
		NDebugOverlay::Line(m_body[i - 1].vecPos, m_body[i].vecPos, 255, 255, 255, true, .1);
#endif

#if 0
		char text[128];
		Q_snprintf(text, sizeof(text), "%d", i);
		NDebugOverlay::Text(m_body[i].vecPos, text, false, 0.1);
#endif

#if 0
		char text[128];
		Q_snprintf(text, sizeof(text), "%4.0f", (m_body[i].vecPos - m_body[i - 1].vecPos).Length() * 100 / m_idealSegmentLength - 100);
		NDebugOverlay::Text(0.5*(m_body[i - 1].vecPos + m_body[i].vecPos), text, false, 0.1);
#endif
	}

	for (; i < CHAIN_LINKS; i++)
	{
		m_vecChain.Set(i, m_vecChain[i - 1]);

	}
	
	//pStudioHdr->pBone(2)->pos = m_body[i].vecPos;
	//matrix3x4_t matrix;
	//VectorMatrix(m_body[i].vecPos, matrix);
	//pStudioHdr->pBone(2)->poseToBone = matrix;
	//DevMsg("m_vecChain: %.2f \n", m_vecChain);
	SetNextThink(gpGlobals->curtime + oc_hydra_runtime.GetFloat());
}



Vector CNPC_Hydra::TestPosition(float t)
{
	// return GetAbsOrigin( ) + Vector( sin( (m_seed + t) * 2.3 ) * 15, cos( (m_seed + t) * 2.4 ) * 150, sin( ( m_seed + t ) * 1.8 ) * 50 ) + m_vecOutward * sv_hydraLength.GetFloat();;
	t = (int)(t * 0.2);
#if 1
	Vector tmp = Vector(sin((m_seed + t) * 0.8) * 15, cos((m_seed + t) * 0.9) * 150, sin((m_seed + t) * 0.4) * 50);
	tmp += Vector(sin((m_seed + t) * 1.0) * 4, cos((m_seed + t) * 0.9) * 4, sin((m_seed + t) * 1.1) * 6);
	tmp += GetAbsOrigin() + m_vecOutward * sv_hydraLength.GetFloat();
	return tmp;
#else

	Vector tmp;
	tmp.Init;
	CBaseEntity *pPlayer = (CBaseEntity *)UTIL_PlayerByIndex(1);
	if (pPlayer)
	{
		tmp = pPlayer->EyePosition();
		Vector delta = (tmp - GetAbsOrigin());

		if (delta.Length() > 200)
		{
			tmp = GetAbsOrigin() + Vector(0, 0, 200);
		}
		m_vecHeadDir = (pPlayer->EyePosition() - m_body[m_body.Count() - 2].vecPos);
		VectorNormalize(m_vecHeadDir);
	}
	return tmp;
#endif
	// m_vecHeadGoal = GetAbsOrigin( ) + Vector( sin( gpGlobals->curtime * 0.3 ) * 15, cos( gpGlobals->curtime * 0.4 ) * 150, sin( gpGlobals->curtime * 0.2 ) * 50 + dt );
}




//-----------------------------------------------------------------------------
// Purpose: Calculate the bone forces based on goal positions, bending rules, stretching rules, etc.
// Input  :
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::CalcGoalForces()
{
	int i;

	int iFirst = 2;
	int iLast = m_body.Count() - 1;

	// keep head segment straight
	m_body[iLast].vecGoalPos = m_vecHeadGoal; // + m_vecHeadDir * m_body[iLast-1].flActualLength;
	m_body[iLast].flGoalInfluence = m_flHeadGoalInfluence;

	m_body[iLast - 1].vecGoalPos = m_vecHeadGoal - m_vecHeadDir * m_idealSegmentLength;
	m_body[iLast - 1].flGoalInfluence = 1.0; // m_flHeadGoalInfluence;


	// momentum?
	for (i = iFirst; i <= iLast; i++)
	{
		m_body[i].vecDelta = m_body[i].vecDelta * sv_hydraMomentum.GetFloat();
	}

	//Vector right, up;
	//VectorVectors( m_vecHeadDir, right, up );

	float flGoalSegmentLength = m_idealSegmentLength * (m_idealLength / m_flCurrentLength);

	// goal forces
#if 1
	for (i = iFirst; i <= iLast; i++)
	{
		// Msg("(%d) %.2f\n", i, t );

		float flInfluence = m_body[i].flGoalInfluence;
		if (flInfluence > 0)
		{
			m_body[i].flGoalInfluence = 0.0;

			Vector v0 = (m_body[i].vecGoalPos - m_body[i].vecPos);
			float length = v0.Length();
			if (length > sv_hydraGoalDelta.GetFloat())
			{
				v0 = v0 * sv_hydraGoalDelta.GetFloat() / length;
			}
			m_body[i].vecDelta += v0 * flInfluence * sv_hydraGoalTension.GetFloat();
			// NDebugOverlay::Box(m_body[i].vecGoalPos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 255, 255, 0, flInfluence * 255, .1);
		}
	}
#endif

	// bending forces
	for (i = iFirst - 1; i <= iLast - 1; i++)
	{
		// Msg("(%d) %.2f\n", i, t );
		Vector v3 = m_body[i + 1].vecPos - m_body[i - 1].vecPos;
		VectorNormalize(v3);

		Vector delta;
		float length;

		//NDebugOverlay::Line( m_body[i].vecPos + v3 * flGoalSegmentLength, m_body[i].vecPos - v3 * flGoalSegmentLength, 255, 0, 0, true, .1);

		if (i + 1 <= iLast)
		{
			// towards head
			delta = (m_body[i].vecPos + v3 * flGoalSegmentLength - m_body[i + 1].vecPos) * sv_hydraBendTension.GetFloat();
			length = delta.Length();
			if (length > sv_hydraBendDelta.GetFloat())
			{
				delta = delta * (sv_hydraBendDelta.GetFloat() / length);
			}
			m_body[i + 1].vecDelta += delta;
			//NDebugOverlay::Line( m_body[i+1].vecPos, m_body[i+1].vecPos + delta, 255, 0, 0, true, .1);
		}

		if (i - 1 >= iFirst)
		{
			// towards tail
			delta = (m_body[i].vecPos - v3 * flGoalSegmentLength - m_body[i - 1].vecPos) * sv_hydraBendTension.GetFloat();
			length = delta.Length();
			if (length > sv_hydraBendDelta.GetFloat())
			{
				delta = delta * (sv_hydraBendDelta.GetFloat() / length);
			}
			m_body[i - 1].vecDelta += delta * 0.8;
			//NDebugOverlay::Line( m_body[i-1].vecPos, m_body[i-1].vecPos + delta, 255, 0, 0, true, .1);
		}
	}

	m_body[0].vecDelta = Vector(0, 0, 0);
	m_body[1].vecDelta = Vector(0, 0, 0);

	// normal gravity forces
	for (i = iFirst; i <= iLast; i++)
	{
		if (!m_body[i].bStuck)
		{
			m_body[i].vecDelta.z -= 3.84 * 0.2;
		}
	}

#if 0
	// move delta's back toward the root
	for (i = iLast; i > iFirst; i--)
	{
		Vector tmp = m_body[i].vecDelta;

		m_body[i].vecDelta = tmp * 0.8;
		m_body[i - 1].vecDelta += tmp * 0.2;
	}
#endif

	// prevent stretching
	int maxChecks = m_body.Count() * 4;
	i = iLast;
	while (i > iFirst && maxChecks > 0)
	{
		bool didStretch = false;
		Vector stretch = (m_body[i].vecPos + m_body[i].vecDelta) - (m_body[i - 1].vecPos + m_body[i - 1].vecDelta);
		float t = VectorNormalize(stretch);
		if (t > flGoalSegmentLength)
		{
			float f0 = DotProduct(m_body[i].vecDelta, stretch);
			float f1 = DotProduct(m_body[i - 1].vecDelta, stretch);
			if (f0 > 0 && f0 > f1)
			{
				// Vector limit = stretch * (f0 - flGoalSegmentLength);
				Vector limit = stretch * (t - flGoalSegmentLength);
				// propagate pulling back down the chain
				m_body[i].vecDelta -= limit * 0.5;
				m_body[i - 1].vecDelta += limit * 0.5;
				didStretch = true;
			}
		}
		if (didStretch)
		{
			if (i < iLast)
			{
				i++;
			}
		}
		else
		{
			i--;
		}
		maxChecks--;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Move the body, check for collisions
// Input  :
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::MoveBody()
{
	int i;

	int iFirst = 2;
	int iLast = m_body.Count() - 1;

	// clear stuck flags
	for (i = 0; i <= iLast; i++)
	{
		m_body[i].bStuck = false;
	}

	// try to move all the nodes
	for (i = iFirst; i <= iLast; i++)
	{
		trace_t tr;

		// check direct movement
		AI_TraceHull(m_body[i].vecPos, m_body[i].vecPos + m_body[i].vecDelta,
			Vector(-2, -2, -2), Vector(2, 2, 2),
			MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
		
		//debugoverlay->AddLineOverlay(m_body[i].vecPos, m_body[i].vecPos - m_body[i-1].vecPos, 255, 0, 0, false, 0.1f);

		//UTIL_TraceModel(m_body[i].vecPos, m_body[i].vecPos + m_body[i].vecDelta, Vector(-22, -22, -22), Vector(22, 22, 22), this, COLLISION_GROUP_NONE, &tr);

		Vector direct = tr.endpos;
		Vector delta = Vector(0, 0, 0);

		Vector slide = m_body[i].vecDelta;
		if (tr.fraction != 1.0)
		{

			//DevMsg("Hit bullet \n");
			// slow down and remove all motion in the direction of the plane
			direct += tr.plane.normal;
			Vector impactSpeed = (slide * tr.plane.normal) * tr.plane.normal;

			slide = (slide - impactSpeed) * 0.8;

			if (tr.m_pEnt)
			{
				if (i == iLast)
				{
					Stab(tr.m_pEnt, impactSpeed, tr);
				}
				else
				{
					Nudge(tr.m_pEnt, direct, impactSpeed);
				}
			}

			// slow down and remove all motion in the direction of the plane
			slide = (slide - (slide * tr.plane.normal) * tr.plane.normal) * 0.8;

			// try to move the remaining distance anyways
			AI_TraceHull(direct, direct + slide * (1 - tr.fraction),
				Vector(-2, -2, -2), Vector(2, 2, 2),
				MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
			//debugoverlay->AddLineOverlay(direct, direct + slide * (1 - tr.fraction), 0, 255, 0, false, 10);
			// NDebugOverlay::Line( m_body[i].vecPos, tr.endpos, 255, 255, 0, true, 1);

			direct = tr.endpos;

			m_body[i].bStuck = true;

		}

		// make sure the new segment doesn't intersect the world
		AI_TraceHull(direct, m_body[i - 1].vecPos,
			Vector(-2, -2, -2), Vector(2, 2, 2),
			MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
		//debugoverlay->AddLineOverlay(direct, m_body[i - 1].vecPos, 0, 0, 255, false, 10);
		if (tr.fraction == 1.0)
		{
			if (i + 1 < iLast)
			{
				AI_TraceHull(direct, m_body[i + 1].vecPos,
					Vector(-2, -2, -2), Vector(2, 2, 2),
					MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
				//debugoverlay->AddLineOverlay(direct, m_body[i + 1].vecPos, 0, 255, 255, false, 10);
			}

			if (tr.fraction == 1.0)
			{
				m_body[i].vecPos = direct;
				delta = slide;
			}
			else
			{
				// FIXME: compute nudge force
				m_body[i].bStuck = true;
				//m_body[i+1].bStuck = true;
			}
		}
		else
		{
			// FIXME: compute nudge force
			m_body[i].bStuck = true;
			//m_body[i-1].bStuck = true;
		}

		// m_body[i-1].vecDelta += (m_body[i].vecDelta - delta) * 0.25;
		// m_body[i+1].vecDelta += (m_body[i].vecDelta - delta) * 0.25;
		m_body[i].vecDelta = delta;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Push physics objects around if they get hit
// Input  : vecContact = point in space where contact supposidly happened
//			vecSpeed = in/sec of contact
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::Nudge(CBaseEntity *pOther, const Vector &vecContact, const Vector &vecSpeed)
{
	if (pOther->GetMoveType() != MOVETYPE_VPHYSICS)
	{
		return;
	}

	IPhysicsObject *pOtherPhysics = pOther->VPhysicsGetObject();

	// Put the force on the line between the "contact point" and hit object origin
	//Vector posOther;
	//pOtherPhysics->GetPosition( &posOther, NULL );

	// force is a 30kg object going 100 in/s
	pOtherPhysics->ApplyForceOffset(vecSpeed * 30, vecContact);

}

//-----------------------------------------------------------------------------
// Purpose: Push physics objects around if they get hit
// Input  : vecContact = point in space where contact supposidly happened
//			vecSpeed = in/sec of contact
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::Stab(CBaseEntity *pOther, const Vector &vecSpeed, trace_t &tr)
{
	if (pOther->m_takedamage == DAMAGE_YES && !pOther->IsPlayer())
	{
		Vector dir = vecSpeed;
		VectorNormalize(dir);

		if (!oc_hydra_test_attachment.GetInt())
		{
			ClearMultiDamage();
			// FIXME: this is bogus
			CTakeDamageInfo info(this, this, pOther->m_iHealth + 25, DMG_SLASH);
			CalculateMeleeDamageForce(&info, dir, tr.endpos);
			pOther->DispatchTraceAttack(info, dir, &tr);
			ApplyMultiDamage();
		}
		else
		{
			if (GetEnemy() && !GetEnemy()->IsPlayer())
			{
				CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(pOther);
				if (pAnimating)
				{
					AttachStabbedEntity(pAnimating, vecSpeed * 30, tr);
				}
			}
			else if (GetEnemy() && GetEnemy()->IsPlayer())
			{
				ClearMultiDamage();
				// FIXME: this is bogus
				CTakeDamageInfo info(this, this, pOther->m_iHealth + 25, DMG_SLASH);
				CalculateMeleeDamageForce(&info, dir, tr.endpos);
				pOther->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			}
		}
	}
	else if (pOther->m_takedamage == DAMAGE_YES && pOther->IsPlayer())
	{
		if (gpGlobals->curtime >= m_flNextStabTime)
		{
			Vector dir = vecSpeed;
			VectorNormalize(dir);

			//	UTIL_ScreenShake( pOther->GetAbsOrigin(), 50, 1, 0.1, 50, SHAKE_START, true );
			UTIL_ScreenShake(pOther->EyePosition(), 50, 1, 0.1, 50, SHAKE_START, true);
			//	if( random->RandomInt( 0, 1 ) == 1 )
			/*	int damage = 0;
			if( random->RandomFloat( 0.0f, 1.0f ) < 0.1f )
			damage = pOther->m_iHealth+25;
			else
			damage = 25;*/
			//	float damage = sk_hydra_stab_damage.GetFloat() + random->RandomFloat( -2.0f, 3.0f );
			int damage = sk_hydra_stab_damage.GetInt() + random->RandomInt(-2, 3);
			//	ClearMultiDamage();
			// VXP: Maybe, this cause hydra stupid
			CTakeDamageInfo info(this, this, damage, DMG_SLASH);
			CalculateMeleeDamageForce(&info, dir, tr.endpos);
			pOther->DispatchTraceAttack(info, dir, &tr);
			//	pOther->TakeDamage( info );
			ApplyMultiDamage();

			CPASAttenuationFilter filter(this, "NPC_Hydra.Pain");
			Vector vecHead = EyePosition();
			EmitSound(filter, entindex(), "NPC_Hydra.Pain", &vecHead);

			m_flNextStabTime = gpGlobals->curtime + 1.3f;
		}
	}
	else
	{
		Nudge(pOther, tr.endpos, vecSpeed);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecContact = point in space where contact supposidly happened
//			vecSpeed = in/sec of contact
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::Kick(CBaseEntity *pHitEntity, const Vector &vecContact, const Vector &vecSpeed)
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecContact = point in space where contact supposidly happened
//			vecSpeed = in/sec of contact
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::Splash(const Vector &vecSplashPos)
{


}


//-----------------------------------------------------------------------------
// Purpose: Calculate the actual hydra length
// Input  : 
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::CheckLength()
{
	int i;

	ClearCondition(COND_HYDRA_SNAGGED);
	ClearCondition(COND_HYDRA_NOSTUCK);
	ClearCondition(COND_HYDRA_OVERSTRETCH);

	m_bHasStuckSegments = m_body[m_body.Count() - 1].bStuck;
	m_flCurrentLength = 0;

	for (i = 1; i < m_body.Count() - 1; i++)
	{
		float length = (m_body[i + 1].vecPos - m_body[i].vecPos).Length();

		Assert(m_body[i + 1].vecPos.IsValid());
		Assert(m_body[i].vecPos.IsValid());

		Assert(IsFinite(length));

		m_body[i].flActualLength = length;

		m_flCurrentLength += length;

		// check for over streatched segements
		if (length > m_idealSegmentLength * 3.0 && (m_body[i].bStuck || m_body[i + 1].bStuck))
		{
			//NDebugOverlay::Line( m_body[i].vecPos, m_body[i+1].vecPos, 255, 0, 0, true, 1.0);
			SetCondition(COND_HYDRA_SNAGGED);
		}
		if (m_body[i].bStuck)
		{
			m_bHasStuckSegments = true;
		}
	}

	if (m_flCurrentLength > HYDRA_MAX_LENGTH) // FIXME
	{
		SetCondition(COND_HYDRA_OVERSTRETCH);
	}

	if (!m_bHasStuckSegments)
	{
		SetCondition(COND_HYDRA_NOSTUCK);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Grow or shrink the hydra, as needed
// Input  : 
// Output :
//-----------------------------------------------------------------------------

void CNPC_Hydra::AdjustLength()
{
	m_body[0].vecPos = (m_body[1].vecPos - m_vecOutward * m_idealSegmentLength);

	// Msg( "actual %.0f ideal %.0f relaxed %.0f\n", actualLength, m_idealLength, m_idealSegmentLength * (m_body.Count() - 3) );

	CalcRelaxedLength();

	// "NPC_Hydra.ExtendTentacle"

	bool bAdjustFailed = false;
	bool bShouldAdjust = false;

	if (m_flCurrentLength < m_idealLength)
	{
		if (m_flRelaxedLength + m_idealSegmentLength * 0.5 < m_idealLength)
		{
			bShouldAdjust = true;
			//if (!GrowFromMostStretched( ))
			if (!GrowFromVirtualRoot())
			{
				bAdjustFailed = true;
			}
		}
	}
	else if (m_flCurrentLength > m_idealLength)
	{
		// if (relaxedLength > actualLength)
		if (m_flRelaxedLength - m_idealSegmentLength * 0.5 > m_idealLength || HasCondition(COND_HYDRA_SNAGGED))
		{
			bShouldAdjust = true;
			if (!ContractFromRoot())
			{
				if (!ContractBetweenStuckSegments())
				{
					if (!ContractFromHead())
					{
						bAdjustFailed = true;
					}
				}
			}
		}
		else if (gpGlobals->curtime - m_flLastAdjustmentTime > 1.0)
		{
			bShouldAdjust = true;
			// start to panic
			if (!GrowFromMostStretched())
			{
				bAdjustFailed = true;
			}

			// SplitLongestSegment( );
			/*
			if (!ContractBetweenStuckSegments())
			{
			if (!ContractFromHead())
			{
			}
			}
			*/
		}
		else
		{
			bAdjustFailed = true;
		}
	}

	if (!bAdjustFailed)
	{
		m_flLastAdjustmentTime = gpGlobals->curtime;
		if (bShouldAdjust && !m_bExtendSoundActive)
		{
			m_bExtendSoundActive = true;
			//CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			//controller.SoundChangeVolume( m_pExtendTentacleSound, 1.0, 0.1 );
		}
	}
	else if (bShouldAdjust)
	{
		m_bExtendSoundActive = false;
		//CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		//controller.SoundChangeVolume( m_pExtendTentacleSound, 0.0, 0.3 );
	}

	CalcRelaxedLength();
}


//-----------------------------------------------------------------------------
// Purpose: Remove nodes, starting at the end, regardless of length
// Input  : 
// Output :
//-----------------------------------------------------------------------------

bool CNPC_Hydra::ContractFromHead()
{
	if (m_body.Count() <= 2)
	{
		return false;
	}

	int iNode = m_body.Count() - 1;

	if (m_body[iNode].bStuck && m_body[iNode - 1].flActualLength > m_idealSegmentLength * 2.0)
	{
		AddNodeBefore(iNode);
		iNode = m_body.Count() - 1;
	}

	if (m_body.Count() <= 3)
	{
		return false;
	}

	// always legal since no new link is being formed

	m_body.Remove(iNode);

	CalcRelaxedLength();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Starting at the first stuck node back from the head, find a node to remove 
//			between it and the actual root who is part of a chain that isn't too long.
// Input  : 
// Output :
//-----------------------------------------------------------------------------

bool CNPC_Hydra::ContractBetweenStuckSegments()
{
	if (m_body.Count() <= 3)
		return false;

	// first first stuck segment closest to head;
	int iStuckHead = VirtualRoot();
	if (iStuckHead < 3)
		return false;

	// find a non stuck node with the shortest distance between its neighbors 
	int iShortest = -1;
	float dist = m_idealSegmentLength * 2;
	int i;
	for (i = iStuckHead - 1; i > 2; i--)
	{
		if (!m_body[i].bStuck)
		{
			float length = (m_body[i - 1].vecPos - m_body[i + 1].vecPos).Length();
			// check segment length
			if (length < dist)
			{
				if (IsValidConnection(i - 1, i + 1))
				{
					dist = length;
					iShortest = i;
				}
			}
		}
	}
	if (iShortest == -1)
		return false;

	// FIXME: check for tunneling
	m_body.Remove(iShortest);

	CalcRelaxedLength();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Try to remove segment closest to root
// Input  : 
// Output :
//-----------------------------------------------------------------------------

bool CNPC_Hydra::ContractFromRoot()
{
	if (m_body.Count() <= 3)
		return false;

	// don't contract overly long segments
	if (m_body[2].flActualLength > m_idealSegmentLength * 2.0)
		return false;

	if (!IsValidConnection(1, 3))
		return false;

	m_body.Remove(2);

	CalcRelaxedLength();

	return true;
}



//-----------------------------------------------------------------------------
// Purpose: Find the first stuck node that's closest to the head
// Input  : 
// Output :
//-----------------------------------------------------------------------------

int CNPC_Hydra::VirtualRoot()
{
	// first first stuck segment closest to head;
	int iStuckHead;
	for (iStuckHead = m_body.Count() - 2; iStuckHead > 1; iStuckHead--)
	{
		if (m_body[iStuckHead].bStuck)
		{
			return iStuckHead;
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Insert a node before the given node.
// Input  : 
// Output :
//-----------------------------------------------------------------------------

bool CNPC_Hydra::AddNodeBefore(int iNode)
{

	if (iNode < 1)
		return false;

	HydraBone bone;

	bone.vecPos = (m_body[iNode].vecPos + m_body[iNode - 1].vecPos) * 0.5;
	bone.vecDelta = (m_body[iNode].vecDelta + m_body[iNode - 1].vecDelta) * 0.5;

	/*
	// FIXME: can't do this, may be embedded in the world
	int i0 = (iNode>2)?iNode-2:0;
	int i1 = (iNode>1)?iNode-1:0;
	int i2 = iNode;
	int i3 = (iNode<m_body.Count()-1)?iNode+1:m_body.Count()-1;
	Catmull_Rom_Spline( m_body[i0].vecPos, m_body[i1].vecPos, m_body[i2].vecPos, m_body[i3].vecPos, 0.5, bone.vecPos );
	*/

	bone.flActualLength = (m_body[iNode].vecPos - bone.vecPos).Length();
	bone.flIdealLength = m_idealSegmentLength;

	m_body[iNode - 1].flActualLength = bone.flActualLength;

	//Vector	vecGoalPos;
	//float	flGoalInfluence;


	m_body.InsertBefore(iNode, bone);

	return true;
}


bool CNPC_Hydra::AddNodeAfter(int iNode)
{
	AddNodeBefore(iNode + 1);
	return false;
}


bool CNPC_Hydra::GrowFromVirtualRoot()
{
	if (m_body[1].flActualLength < m_idealSegmentLength * 0.5)
		return false;

	return AddNodeAfter(1);
}


bool CNPC_Hydra::GrowFromMostStretched()
{
	int iNode = VirtualRoot();

	int iLongest = iNode;
	float dist = m_idealSegmentLength * 0.5;

	for (iNode; iNode < m_body.Count() - 1; iNode++)
	{
		if (m_body[iNode].flActualLength > dist)
		{
			iLongest = iNode;
			dist = m_body[iNode].flActualLength;
		}
	}

	if (m_body[iLongest].flActualLength <= dist)
	{
		return AddNodeAfter(iLongest);
	}
	return false;
}


void CNPC_Hydra::CalcRelaxedLength()
{
	m_flRelaxedLength = m_idealSegmentLength * (m_body.Count() - 2) + HYDRA_OUTWARD_BIAS;
}


bool CNPC_Hydra::IsValidConnection(int iNode0, int iNode1)
{
	trace_t tr;
	// check to make sure new connection is valid
	AI_TraceHull(m_body[iNode0].vecPos, m_body[iNode1].vecPos,
		Vector(-2, -2, -2), Vector(2, 2, 2),
		MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);//  COLLISION_GROUP_NONE


	if (tr.fraction == 1.0)
	{
		return true;
	}
	return false;
}


//-------------------------------------

float CNPC_Hydra::MaxYawSpeed()
{
	return 0;

	if (IsMoving())
	{
		return 20;
	}

	switch (GetActivity())
	{
	case ACT_180_LEFT:
		return 30;
		break;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 30;
		break;
	default:
		return 15;
		break;
	}
}

//-------------------------------------

int CNPC_Hydra::TranslateSchedule(int scheduleType)
{
	return BaseClass::TranslateSchedule(scheduleType);
}

//-------------------------------------

void CNPC_Hydra::HandleAnimEvent(animevent_t *pEvent)
{
	BaseClass::HandleAnimEvent(pEvent);
}

//-------------------------------------
int H = 0;
void CNPC_Hydra::PrescheduleThink()
{
	BaseClass::PrescheduleThink();
	if (m_bStabbedEntity)
	{
		UpdateStabbedEntity();
	}

	if (m_bDied && gpGlobals->curtime >= m_flDieTime)
	{
#ifdef _DEBUG
		Msg("Hydra was removed.\n");
#endif
		SetThink(&CNPC_Hydra::SUB_Remove);
	}

	/*if (GetHealth() <= 1)
	{
		StopLoopingSounds();
		StopSound("NPC_Hydra.HeartbeatIdle");
	}*/
	if (PainColor)
	{
		H++;
		if (H >= 35)
		{
			PainColor = false;
		}
	}
	else if (!PainColor)
	{
		H = 0;
	}



}

//-------------------------------------

int CNPC_Hydra::SelectSchedule()
{
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
	{
		//if (GetHealth() > 1)
		//{


		//}
		//else
		/*{
			StopSound("NPC_Hydra.HeartbeatIdle");
			StopLoopingSounds();
		}*/


		

		SetState(NPC_STATE_ALERT);
		return SCHED_HYDRA_DEPLOY;
	}
	break;

	case NPC_STATE_ALERT:
	{
		if (GetEnemy())
		EmitSound("NPC_Hydra.Alert");

		return SCHED_HYDRA_STAB;
		/*	m_vecHeadGoal = GetAbsOrigin( ) + m_vecOutward * 100;
		CPASAttenuationFilter filter( this, "NPC_Hydra.Search" );
		Vector vecHead = EyePosition();
		EmitSound( filter, entindex(), "NPC_Hydra.Search", &vecHead );
		return SCHED_HYDRA_IDLE; // VXP: Maybe not..?*/
	}
	break;

	case NPC_STATE_COMBAT:
	{

		if (HasCondition(COND_HYDRA_SNAGGED))
		{
			return SCHED_HYDRA_PULLBACK;
		}
		else if (HasCondition(COND_HYDRA_OVERSTRETCH))
		{
			return SCHED_HYDRA_STAB;
		}
		return SCHED_HYDRA_STAB;
	}
	break;
	}

	return BaseClass::SelectSchedule();
}
/*void CNPC_Hydra::CreateSounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);
	if (!IdleSound)
	{
		IdleSound = controller.SoundCreate(filter, entindex(), "NPC_Hydra.HeartbeatIdle");
		controller.Play(IdleSound, 0, 100);
		controller.SoundChangePitch(IdleSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);
	}
}

void CNPC_Hydra::DestroySounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy(IdleSound);
	IdleSound = NULL;
}
void CNPC_Hydra::StartSounds()
{
	//if (!m_bIsFiring)
	{
		CSoundEnvelopeController *pController = &CSoundEnvelopeController::GetController();
		float flVolume = pController->SoundGetVolume(IdleSound);
		pController->SoundChangeVolume(IdleSound, 1.4f, 0.1f * (1.0f - flVolume));
		pController->SoundGetPitch(IdleSound);
		//float iPitch = 100.0f * cvar->FindVar("host_timescale")->GetFloat();
		pController->SoundChangePitch(IdleSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);
		//DevMsg("iPitch: %.2f \n", iPitch);
		//m_bIsFiring = true;
	}
}*/

void CNPC_Hydra::StopSounds()
{

	//if (m_bIsFiring)
	{
		/*CSoundEnvelopeController *pController = &CSoundEnvelopeController::GetController();
		float flVolume = pController->SoundGetVolume(IdleSound);
		pController->SoundChangeVolume(IdleSound, 0.0f, 0.1f * flVolume);
		//float iPitch = 100.0f;
		pController->SoundChangePitch(IdleSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);*/
		//DevMsg("StopSound");

		//StopSound("NPC_Hydra.HeartbeatIdle");
		StopLoopingSounds();
		//m_bIsFiring = false;

	}
}
//-------------------------------------

void CNPC_Hydra::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HYDRA_DEPLOY:
		m_vecHeadGoal = GetAbsOrigin() + m_vecOutward * 100;
		m_idealLength = 100;
		m_vecHeadDir = m_vecOutward;
		return;
	case TASK_HYDRA_PREP_STAB:
	{
		m_flTaskEndTime = gpGlobals->curtime + pTask->flTaskData;

		// Go outward
		m_vecHeadGoal = GetAbsOrigin() + m_vecOutward * 100;
		SetTarget((CBaseEntity *)UTIL_PlayerByIndex(1));
		if (GetEnemy())
		{
			SetTarget(GetEnemy());



				//EmitSound("NPC_Hydra.Search");
//				CBaseEntity *pTarget = GetTarget();
				trace_t tr;
				Vector Up(0,0,-1000);
				AI_TraceLine(this->GetAbsOrigin(), this->GetAbsOrigin() + Up + GetEnemy()->GetAbsOrigin(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
				//debugoverlay->AddLineOverlay(this->GetAbsOrigin(), this->GetAbsOrigin() + Up + GetEnemy()->GetAbsOrigin(), 255, 255, 255, false, 10);
				if (tr.endpos.DistTo(GetEnemy()->GetAbsOrigin()) > 700)
				{
					if (Inc < random->RandomInt(150, 200))
					{
						Inc++;
					}
					else if (Inc >= random->RandomInt(150, 200))
					{
						EmitSound("NPC_Hydra.Search");
						Inc = 0.0f;
					}
				}
				else
				{
					Inc = 0.0f;
				}
				//DevMsg("Inc: %.2f \n", Inc);


		}
		/*else if (!GetEnemy())
		{
			if (Inc < random->RandomInt(100, 150))
			{
				Inc++;
			}
			else if (Inc >= random->RandomInt(100, 150))
			{
				EmitSound("NPC_Hydra.Search");
				Inc = 0.0f;
			}
			DevMsg("Inc2: %.2f \n", Inc);
		}*/

		if (GetTarget() && GetTarget()->GetHealth() > 0)
		{
			/*CPASAttenuationFilter filter(this, "NPC_Hydra.Alert");
			Vector vecHead = EyePosition();
			EmitSound(filter, entindex(), "NPC_Hydra.Alert", &vecHead);*/
		}
		else if (GetTarget() && GetTarget()->GetHealth() <= 0)
		{
			//	m_flTaskEndTime += random->RandomFloat ( 2.0f, 5.0f );
			//	m_flTaskEndTime = gpGlobals->curtime + pTask->flTaskData + 10.0f;

			//	CPASAttenuationFilter filter( this, "NPC_Hydra.Search" );
			//	Vector vecHead = EyePosition();
			//	EmitSound( filter, entindex(), "NPC_Hydra.Search", &vecHead );
		}
		ChangeColor = false;
	}
	return;

	case TASK_HYDRA_STAB:
	{
		CPASAttenuationFilter filter(this, "NPC_Hydra.Attack");
		Vector vecHead = EyePosition();
		EmitSound(filter, entindex(), "NPC_Hydra.Attack", &vecHead);

		m_flTaskEndTime = gpGlobals->curtime + 0.5;
		ChangeColor = true;
	}
	return;

	case TASK_HYDRA_PULLBACK:
		m_vecHeadGoal = GetAbsOrigin() + m_vecOutward * pTask->flTaskData;
		m_idealLength = pTask->flTaskData * 1.1;
		//EmitSound("NPC_Hydra.HeartbeatFast");
		return;

	default:
		BaseClass::StartTask(pTask);
		break;
	}

}

//-------------------------------------

void CNPC_Hydra::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HYDRA_DEPLOY:
	{
		//	Msg( "TASK_HYDRA_DEPLOY\n" );
		m_flHeadGoalInfluence = 1.0;
		float dist = (EyePosition() - m_vecHeadGoal).Length();

		if (dist < m_idealSegmentLength)
		{
			TaskComplete();
		}

		AimHeadInTravelDirection(0.2);
	}
	break;


	case TASK_HYDRA_PREP_STAB:
	{
		//	Msg( "TASK_HYDRA_PREP_STAB\n" );
		int i;

		if (m_body.Count() < 2)
		{
			TaskFail("hydra is too short to begin stab");
			return;
		}

		if (m_bStabbedEntity)
		{
			TaskFail("hydra is stabbing someone");
			return;
		}

		CBaseEntity *pTarget = GetTarget();
		if (pTarget == NULL || (pTarget && pTarget->GetHealth() <= 0))
		{
			TaskFail(FAIL_NO_TARGET);
			return;
		}

		if (pTarget->GetFlags() & FL_NOTARGET)
		{
			TaskFail(FAIL_NO_TARGET);
			return;
		}

		if (pTarget->IsPlayer())
		{
			m_vecTarget = pTarget->EyePosition();
		}
		else
		{
			m_vecTarget = pTarget->BodyTarget(EyePosition());
		}

		float distToTarget = (m_vecTarget - m_vecHeadGoal).Length();
		float distToBase = (m_vecHeadGoal - GetAbsOrigin()).Length();
		m_idealLength = distToTarget + distToBase * 0.5;

		if (m_idealLength > HYDRA_MAX_LENGTH)
			m_idealLength = HYDRA_MAX_LENGTH;

		if (distToTarget < 100.0)
		{
			m_vecTargetDir = (m_vecTarget - m_vecHeadGoal);
			VectorNormalize(m_vecTargetDir);
			m_vecHeadGoal = m_vecHeadGoal - m_vecTargetDir * (100 - distToTarget) * 0.5;
		}
		else if (distToTarget > 200.0)
		{


			m_vecTargetDir = (m_vecTarget - m_vecHeadGoal);
			VectorNormalize(m_vecTargetDir);
			m_vecHeadGoal = m_vecHeadGoal - m_vecTargetDir * (200.0 - distToTarget) * 0.5;
		}

		

		// face enemy
		m_vecTargetDir = (m_vecTarget - m_body[m_body.Count() - 1].vecPos);
		VectorNormalize(m_vecTargetDir);
		m_vecHeadDir = m_vecHeadDir * 0.6 + m_vecTargetDir * 0.4;
		VectorNormalize(m_vecHeadDir.GetForModify());

		// build tension towards strike time
		float influence = 1.0 - (m_flTaskEndTime - gpGlobals->curtime) / pTask->flTaskData;
		if (influence > 1)
			influence = 1.0;

		influence = influence * influence * influence;

		m_flHeadGoalInfluence = influence;

		// keep head segment straight
		i = m_body.Count() - 2;
		m_body[i].vecGoalPos = m_vecHeadGoal - m_vecHeadDir * m_body[i].flActualLength;
		m_body[i].flGoalInfluence = influence;

		// curve neck into spiral
		float distBackFromHead = m_body[i].flActualLength;
		Vector right, up;
		VectorVectors(m_vecHeadDir, right, up);

		for (i = i - 1; i > 1 && distBackFromHead < distToTarget; i--)
		{
			distBackFromHead += m_body[i].flActualLength;

			float r = (distBackFromHead / 200) * 3.1415 * 2;

			// spiral
			Vector p0 = m_vecHeadGoal
				- m_vecHeadDir * distBackFromHead * 0.5
				+ cos(r) * m_body[i].flActualLength * right
				+ sin(r) * m_body[i].flActualLength * up;

			// base
			r = (distBackFromHead / m_idealLength) * 3.1415 * 0.2;
			r = sin(r);
			p0 = p0 * (1 - r) + r * GetAbsOrigin();

			m_body[i].vecGoalPos = p0;

			m_body[i].flGoalInfluence = influence * (1.0 - (distBackFromHead / distToTarget));

			/*
			if ( (pEnemy->EyePosition( ) - m_body[i].vecPos).Length() < distBackFromHead)
			{
			if ( gpGlobals->curtime - m_flLastAttackTime > 4.0)
			{
			TaskComplete();
			}
			return;
			}
			*/
		}

		// look to see if any of the goal positions are stuck
		for (i = i; i < m_body.Count() - 1; i++)
		{
			if (m_body[i].bStuck)
			{
				Vector delta = DotProduct(m_body[i].vecGoalPos - m_body[i].vecPos, m_vecHeadDir) * m_vecHeadDir;
				m_vecHeadGoal -= delta * m_body[i].flGoalInfluence;
				break;
			}
		}

		if (gpGlobals->curtime >= m_flTaskEndTime)
		{
			if (distToTarget < 500)
			{
				TaskComplete();
				return;
			}
			else
			{
				TaskFail("target is too far away");
				return;
			}
		}
	}
	return;



	case FAIL_NO_TARGET:
	{

	}
	break;


	case TASK_HYDRA_STAB:
	{
		//	Msg( "TASK_HYDRA_STAB\n" );
		int i;

		if (m_body.Count() < 2)
		{
			TaskFail("hydra is too short to begin stab");
			return;
		}

		if (m_flTaskEndTime <= gpGlobals->curtime)
		{
			TaskComplete();
			return;
		}

		m_flHeadGoalInfluence = 1.0;

		// face enemy
		//m_vecHeadDir = (pEnemy->EyePosition( ) - m_body[m_body.Count()-1].vecPos);
		//VectorNormalize( m_vecHeadDir.GetForModify() );

		// keep head segment straight
		i = m_body.Count() - 2;
		m_body[i].vecGoalPos = m_vecHeadGoal + m_vecHeadDir * m_body[i].flActualLength;
		m_body[i].flGoalInfluence = 1.0;

		Vector vecToTarget = (m_vecTarget - EyePosition());

		// check to see if we went past target
		if (DotProduct(vecToTarget, m_vecHeadDir) < 0.0)
		{
			TaskComplete();
			return;
		}

		float distToTarget = vecToTarget.Length();
		float distToBase = (EyePosition() - GetAbsOrigin()).Length();
		m_idealLength = distToTarget + distToBase;

		/*
		if (distToTarget < 20)
		{
		m_vecHeadGoal = m_vecTarget;
		SetLastAttackTime( gpGlobals->curtime );
		TaskComplete();
		return;
		}
		else
		*/
		{
			// hit enemy
			m_vecHeadGoal = m_vecTarget + m_vecHeadDir * 300;
		}

		if (m_idealLength > HYDRA_MAX_LENGTH)
			m_idealLength = HYDRA_MAX_LENGTH;

		// curve neck into spiral
		float distBackFromHead = m_body[i].flActualLength;
		Vector right, up;
		VectorVectors(m_vecHeadDir, right, up);

#if 1
		for (i = i - 1; i > 1 && distBackFromHead < distToTarget; i--)
		{
			Vector p0 = m_vecHeadGoal - m_vecHeadDir * distBackFromHead * 1.0;

			m_body[i].vecGoalPos = p0;

			if ((m_vecTarget - m_body[i].vecPos).Length() > distToTarget + distBackFromHead)
			{
				m_body[i].flGoalInfluence = 1.0 - (distBackFromHead / distToTarget);
			}
			else
			{
				m_body[i].vecGoalPos = EyePosition() - m_vecHeadDir * distBackFromHead;
				m_body[i].flGoalInfluence = 1.0 - (distBackFromHead / distToTarget);
			}

			distBackFromHead += m_body[i].flActualLength;
		}
#endif
	}
	return;

	case TASK_HYDRA_PULLBACK:
	{
		
		//	Msg( "TASK_HYDRA_PULLBACK\n" );
		if (m_body.Count() < 2)
		{
			TaskFail("hydra is too short to begin stab");
			return;
		}
		CBaseEntity *pEnemy = (CBaseEntity *)UTIL_PlayerByIndex(1);
		if (GetEnemy() != NULL)
		{
			pEnemy = GetEnemy();
		}

		AimHeadInTravelDirection(0.2);

		// float dist = (EyePosition() - m_vecHeadGoal).Length();

		if (m_flCurrentLength < m_idealLength + m_idealSegmentLength)
		{
			TaskComplete();
		}
	}
	break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}




}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Hydra::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	//	Msg("Hydra attacker: %s\n", inputInfo.GetAttacker()->GetClassname());
	if (!strncmp(inputInfo.GetAttacker()->GetClassname(), "prop_ragdoll", 12))
	{
		return 0; // VXP: Prevent from ragdoll damaging
	}
	EmitSound("NPC_Hydra.Pain");
	CTakeDamageInfo info = inputInfo;
	PainColor = true;
#if 0
	// Die instantly from a hit in idle/alert states
	if (m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT)
	{
		info.SetDamage(m_iHealth);
	}
#endif //0 

	return BaseClass::OnTakeDamage_Alive(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Hydra::Event_Killed(const CTakeDamageInfo &info)
{
	//StopSound("NPC_Hydra.HeartbeatIdle");
	//StopLoopingSounds();
	for (int i = 0; i <= 10; i++)
	StopSounds();
	//Msg("HYDRA KILLED\n");
	m_NPCState = NPC_STATE_NONE;
	EmitSound("NPC_Hydra.Pain");
	//	m_vecHeadGoal = GetAbsOrigin( ) - m_vecOutward * 100;
	//	m_idealLength = 0;
	//	m_vecHeadDir = -m_vecOutward;

	// VXP: Better, but still some part of hydra is above ground
	/*	GetVectors( NULL, NULL, &m_vecOutward );
	SetAbsAngles( QAngle( 0, 0, 0 ) );
	m_vecChain.Set( 0, GetAbsOrigin( ) - m_vecOutward * 32 );
	m_vecChain.Set( 1, GetAbsOrigin( ) + m_vecOutward * 16 );
	m_vecHeadGoal = m_vecChain[1] + m_vecOutward * HYDRA_OUTWARD_BIAS;
	m_vecHeadDir = Vector( 0, 0, 1 );
	m_idealLength = 0;*/

	GetVectors(NULL, NULL, &m_vecOutward);
	SetAbsAngles(QAngle(0, 0, 0));
	m_vecChain.Set(0, GetAbsOrigin() - m_vecOutward * 256);
	m_vecChain.Set(1, GetAbsOrigin() + m_vecOutward * 2);
	m_vecHeadGoal = m_vecChain[1] + m_vecOutward * HYDRA_OUTWARD_BIAS;
	m_vecHeadDir = Vector(0, 0, 1);
	m_idealLength = 0;

	m_flHeadGoalInfluence = 1.0;
	//	AimHeadInTravelDirection( 1.0 );
	CheckLength();
	AdjustLength();
	CalcGoalForces();
	MoveBody();

	m_bDied = true;
	m_flDieTime = gpGlobals->curtime + 2.0f;


	//UTIL_Remove(this);
	BaseClass::Event_Killed(info);
}

//-------------------------------------

Vector CNPC_Hydra::EyePosition()
{
	int i = m_body.Count() - 1;

	if (i >= 0)
	{
		return m_body[i].vecPos;
	}
	return GetAbsOrigin();
}

const QAngle &CNPC_Hydra::EyeAngles()
{
	return GetAbsAngles();
}


Vector CNPC_Hydra::BodyTarget(const Vector &posSrc, bool bNoisy)
{
	int i;

	if (m_body.Count() < 2)
	{
		return GetAbsOrigin();
	}

	int iShortest = 1;
	float flShortestDist = (posSrc - m_body[iShortest].vecPos).LengthSqr();
	for (i = 2; i < m_body.Count(); i++)
	{
		float flDist = (posSrc - m_body[i].vecPos).LengthSqr();
		if (flDist < flShortestDist)
		{
			iShortest = i;
			flShortestDist = flDist;
		}
	}

	// NDebugOverlay::Box(m_body[iShortest].vecPos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 0, 255, 20, .1);

	return m_body[iShortest].vecPos;
}


void CNPC_Hydra::AimHeadInTravelDirection(float flInfluence)
{

	// aim in the direction of movement enemy
	Vector delta = m_body[m_body.Count() - 1].vecDelta;
	VectorNormalize(delta);
	if (DotProduct(delta, m_vecHeadDir) < 0)
	{
		delta = -delta;
	}

	m_vecHeadDir = m_vecHeadDir * (1 - flInfluence) + delta * flInfluence;
	VectorNormalize(m_vecHeadDir.GetForModify());
}

//-------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Hydra impaling is done by creating an entity, forming a constraint
//			between that entity and the target ragdoll, and then updating then
//			entity to follow the hydra.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: This is the entity we create to follow the hydra
//-----------------------------------------------------------------------------
class CHydraImpale : public CBaseAnimating
{
	DECLARE_CLASS(CHydraImpale, CBaseAnimating);
public:
	DECLARE_DATADESC();

	void	Spawn(void);
	void	Precache(void);
	void	ImpaleThink(void);

	IPhysicsConstraint *CreateConstraint(CNPC_Hydra *pHydra, IPhysicsObject *pTargetPhys, IPhysicsConstraintGroup *pGroup);
	static CHydraImpale *Create(CNPC_Hydra *pHydra, CBaseEntity *pObject2);

	IPhysicsConstraint	*GetConstraint(void);

public:
	IPhysicsConstraint		*m_pConstraint;
	CHandle<CNPC_Hydra>		m_hHydra;
};

BEGIN_DATADESC(CHydraImpale)
DEFINE_PHYSPTR(m_pConstraint),
DEFINE_FIELD(m_hHydra, FIELD_EHANDLE),

DEFINE_THINKFUNC(ImpaleThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(hydra_impale, CHydraImpale);

//-----------------------------------------------------------------------------
// Purpose: To by usable by the constraint system, this needs to have a phys model.
//-----------------------------------------------------------------------------
void CHydraImpale::Spawn(void)
{
	Precache();
	SetModel("models/props_junk/cardboard_box001a.mdl");
	AddEffects(EF_NODRAW);

	// We don't want this to be solid, because we don't want it to collide with the hydra.
	SetSolid(SOLID_VPHYSICS);
	AddSolidFlags(FSOLID_NOT_SOLID);
	VPhysicsInitShadow(false, false);

	// Disable movement on this sucker, we're going to move him manually
	SetMoveType(MOVETYPE_FLY);

	BaseClass::Spawn();

	m_pConstraint = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHydraImpale::Precache(void)
{
	PrecacheModel("models/props_junk/cardboard_box001a.mdl");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Update the impale entity's position to the hydra's desired
//-----------------------------------------------------------------------------
void CHydraImpale::ImpaleThink(void)
{
	if (!m_hHydra)
	{
		// Remove ourselves.
		m_pConstraint->Deactivate();
		UTIL_Remove(this);
		return;
	}

	// Ask the Hydra where he'd like the ragdoll, and move ourselves there
	Vector vecOrigin;
	QAngle vecAngles;
	m_hHydra->GetDesiredImpaledPosition(&vecOrigin, &vecAngles);
	SetAbsOrigin(vecOrigin);
	SetAbsAngles(vecAngles);

	//NDebugOverlay::Cross3D( vecOrigin, Vector( -5, -5, -5 ), Vector( 5, 5, 5 ), 255, 0, 0, 20, .1);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Activate/create the constraint
//-----------------------------------------------------------------------------
IPhysicsConstraint *CHydraImpale::CreateConstraint(CNPC_Hydra *pHydra, IPhysicsObject *pTargetPhys, IPhysicsConstraintGroup *pGroup)
{
	if (GetEnemy() && GetEnemy()->IsPlayer())
		return false;

	m_hHydra = pHydra;

	IPhysicsObject *pImpalePhysObject = VPhysicsGetObject();
	Assert(pImpalePhysObject);

	constraint_fixedparams_t fixed;
	fixed.Defaults();
	fixed.InitWithCurrentObjectState(pImpalePhysObject, pTargetPhys);
	fixed.constraint.Defaults();

	m_pConstraint = physenv->CreateFixedConstraint(pImpalePhysObject, pTargetPhys, pGroup, fixed);
	if (m_pConstraint)
	{
		m_pConstraint->SetGameData((void *)this);
	}

	SetThink(&CHydraImpale::ImpaleThink);
	SetNextThink(gpGlobals->curtime);
	return m_pConstraint;
}

IPhysicsConstraint *CHydraImpale::GetConstraint()
{
	return m_pConstraint;
}

//-----------------------------------------------------------------------------
// Purpose: Create a Hydra Impale between the hydra and the entity passed in
//-----------------------------------------------------------------------------
CHydraImpale *CHydraImpale::Create(CNPC_Hydra *pHydra, CBaseEntity *pTarget)
{

	Vector vecOrigin;
	QAngle vecAngles;
	pHydra->GetDesiredImpaledPosition(&vecOrigin, &vecAngles);
	pTarget->Teleport(&vecOrigin, &vecAngles, &vec3_origin);

	CHydraImpale *pImpale = (CHydraImpale *)CBaseEntity::Create("hydra_impale", vecOrigin, vecAngles);
	if (!pImpale)
		return NULL;
	//if (pImpale->IsPlayer())
		//return NULL;

	IPhysicsObject *pTargetPhysObject = pTarget->VPhysicsGetObject();
	if (!pTargetPhysObject)
	{
		Msg(" Error: Tried to hydra_impale an entity without a physics model.\n");
		return NULL;
	}

	IPhysicsConstraintGroup *pGroup = NULL;
	// Ragdoll? If so, use it's constraint group
	CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>(pTarget);
	if (pRagdoll)
	{
		pGroup = pRagdoll->GetRagdoll()->pGroup;
	}

	if (!pImpale->CreateConstraint(pHydra, pTargetPhysObject, pGroup))
		return NULL;

	return pImpale;
}

void CNPC_Hydra::AttachStabbedEntity(CBaseAnimating *pAnimating, Vector vecForce, trace_t &tr)
{
	//if (GetEnemy() && GetEnemy()->IsPlayer())
		//return;

	CTakeDamageInfo info(this, this, pAnimating->m_iHealth + 25, DMG_SLASH);
	info.SetDamageForce(vecForce);
	info.SetDamagePosition(tr.endpos);

	//	if( pAnimating->Classify() == CLASS_ANTLION )
	/*if (FClassnameIs(pAnimating, "npc_antliongrub")) // VXP: Hack for antlion grub
	{
	CNPC_AntlionGrub *grub = dynamic_cast<CNPC_AntlionGrub*>(pAnimating);
	//	grub->SetHealth(-1);
	ClearMultiDamage();
	// FIXME: this is bogus
	CTakeDamageInfo info(this, this, 1000, DMG_SLASH);
	CalculateMeleeDamageForce(&info, vecForce, tr.endpos);
	grub->DispatchTraceAttack(info, vecForce, &tr);
	ApplyMultiDamage();
	return;
	}*/

	CBaseEntity *pRagdoll = CreateServerRagdoll(pAnimating, 0, info, COLLISION_GROUP_DEBRIS);
	//CBaseEntity *pRagdoll = CreateServerRagdollAttached(pAnimating, vec3_origin, 0, COLLISION_GROUP_DEBRIS, pLegPhys, this, bone->boneIndex, vecStabPos, -1, localHit);


	// Create our impale entity
	//	CHydraImpale::Create( this, pRagdoll );
	m_pHydraImpale = CHydraImpale::Create(this, pRagdoll);

	if (m_pHydraImpale)
	{
		m_bStabbedEntity = true;

		UTIL_Remove(pAnimating);

		m_flDetachEntityTime = gpGlobals->curtime + random->RandomFloat(HYDRA_MIN_STAB_HOLD_TIME, HYDRA_MAX_STAB_HOLD_TIME);
		//	m_flDetachEntityTime = gpGlobals->curtime + 1.0f;
	}
}

void CNPC_Hydra::UpdateStabbedEntity(void)
{
	/*
	CBaseEntity *pEntity = m_grabController.GetAttached();
	if ( !pEntity )
	{
	DetachStabbedEntity( false );
	return;
	}
	QAngle vecAngles(0,0,1);
	Vector vecOrigin = m_body[m_body.Count()-2].vecPos;
	//NDebugOverlay::Cross3D( vecOrigin, Vector( -5, -5, -5 ), Vector( 5, 5, 5 ), 255, 0, 0, 20, .1);
	m_grabController.SetTargetPosition( vecOrigin, vecAngles );
	*/
	/*	if( m_pHydraImpale )
	{
	Msg( "%f\n", m_pHydraImpale.GetAbsOrigin().x );
	}*/
	if (m_bStabbedEntity && gpGlobals->curtime >= m_flDetachEntityTime)
	{
		DetachStabbedEntity(true);
	}
}

void CNPC_Hydra::DetachStabbedEntity(bool playSound)
{
	/*
	CBaseEntity *pObject = m_grabController.GetAttached();
	if ( pObject != NULL )
	{
	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();
	// Enable collision with this object again
	if ( pPhysics != NULL )
	{
	physenv->EnableCollisions( pPhysics, VPhysicsGetObject() );
	pPhysics->RecheckCollisionFilter();
	}
	}
	m_grabController.DetachEntity();
	*/
	if (m_pHydraImpale == NULL)
		return;

	IPhysicsConstraint *pConstraint = m_pHydraImpale->GetConstraint();
	if (pConstraint != NULL)
	{
		pConstraint->Deactivate();
		pConstraint = NULL;

		UTIL_Remove(m_pHydraImpale);
		//	m_pHydraImpale = NULL;

		m_bStabbedEntity = false;
	}

	if (playSound)
	{
		//Play the detach sound
		CPASAttenuationFilter filter(this, "NPC_Hydra.Bump");
		Vector vecHead = EyePosition();
		EmitSound(filter, entindex(), "NPC_Hydra.Bump", &vecHead);
	}
}

void CNPC_Hydra::GetDesiredImpaledPosition(Vector *vecOrigin, QAngle *vecAngles)
{
	*vecOrigin = m_body[m_body.Count() - 2].vecPos;
	*vecAngles = QAngle(0, 0, 0);
}

//-----------------------------------------------------------------------------
//
// CNPC_Hydra Schedules
//
//-------------------------------------


AI_BEGIN_CUSTOM_NPC(npc_hydra, CNPC_Hydra)

//Register our interactions

//Conditions
DECLARE_CONDITION(COND_HYDRA_SNAGGED)
DECLARE_CONDITION(COND_HYDRA_STUCK)
DECLARE_CONDITION(COND_HYDRA_OVERSHOOT)
DECLARE_CONDITION(COND_HYDRA_OVERSTRETCH)
DECLARE_CONDITION(COND_HYDRA_STRIKE)
DECLARE_CONDITION(COND_HYDRA_NOSTUCK)

//Squad slots

//Tasks
DECLARE_TASK(TASK_HYDRA_RETRACT)
DECLARE_TASK(TASK_HYDRA_DEPLOY)
DECLARE_TASK(TASK_HYDRA_GET_OBJECT)
DECLARE_TASK(TASK_HYDRA_THROW_OBJECT)
DECLARE_TASK(TASK_HYDRA_PREP_STAB)
DECLARE_TASK(TASK_HYDRA_STAB)
DECLARE_TASK(TASK_HYDRA_PULLBACK)

//Activities
DECLARE_ACTIVITY(ACT_HYDRA_COWER)
DECLARE_ACTIVITY(ACT_HYDRA_STAB)

//=========================================================
// > SCHED_HYDRA_STAND_LOOK
//=========================================================
DEFINE_SCHEDULE
(
SCHED_HYDRA_DEPLOY,

"	Tasks"
"		TASK_HYDRA_DEPLOY			0"
"		TASK_WAIT					0.5"
""
"	Interrupts"
"		COND_NEW_ENEMY"
)

//=========================================================
// > SCHED_HYDRA_COWER
//=========================================================
DEFINE_SCHEDULE
(
SCHED_HYDRA_RETRACT,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_HYDRA_COWER"
"		TASK_WAIT					0.5"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HYDRA_IDLE,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_WAIT_INDEFINITE			0"
""
"	Interrupts "
"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
SCHED_HYDRA_STAB,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_HYDRA_DEPLOY"
"		TASK_HYDRA_PREP_STAB			4.0"
"		TASK_HYDRA_STAB					0"
"		TASK_WAIT						0.5"
// "		TASK_HYDRA_PULLBACK				100"
""
"	Interrupts "
"		COND_NEW_ENEMY"
"		COND_HYDRA_OVERSTRETCH"
)

DEFINE_SCHEDULE
(
SCHED_HYDRA_PULLBACK,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_WAIT						0.4"
"		TASK_HYDRA_PULLBACK				100"
""
"	Interrupts "
"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
SCHED_HYDRA_THROW,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_HYDRA_GET_OBJECT				0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_HYDRA_THROW_OBJECT				0"
"		TASK_WAIT							1"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HYDRA_RANGE_ATTACK,

"	Tasks"
"		TASK_STOP_MOVING		0"
"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
"		TASK_FACE_ENEMY			0"
"		TASK_RANGE_ATTACK1		0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_ENEMY_OCCLUDED"
"		COND_NO_PRIMARY_AMMO"
"		COND_HEAR_DANGER"
)

AI_END_CUSTOM_NPC()

