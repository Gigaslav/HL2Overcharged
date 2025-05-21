//========= Copyright © LOLOLOL, All rights reserved. ============//
//
// Purpose: A big heavily-armored monstrosity who works for the combine and carries the combine 
// equivalent to the BFG.
//
// FIXED DAMAGE! EXPLOSION ON DEATH NO LONGER NECESSARY!!!!
//=============================================================================//

#include "cbase.h"
#include "AI_Task.h"
#include "AI_Default.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Motor.h"
#include "AI_Memory.h"
#include "npc_combine.h"
#include "physics.h"
#include "bitstring.h"
#include "activitylist.h"
#include "game.h"
#include "NPCEvent.h"
#include "Player.h"
#include "EntityList.h"
#include "AI_Interactions.h"
#include "soundent.h"
#include "Gib.h"
#include "shake.h"
#include "Sprite.h"
#include "explode.h"
#include "grenade_homer.h"
#include "AI_BaseNPC.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "props.h"
#include "gib.h"
#define CGUARD_GIB_COUNT			5   //Assuming we get a new modeller who can make these for us.

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_combineguard_health("sk_combineguard_health", "0");
ConVar sk_combineguard_allow_jump("sk_combineguard_allow_jump", "0", FCVAR_ARCHIVE);
ConVar sk_combineguard_allow_climb("sk_combineguard_allow_climb", "0", FCVAR_ARCHIVE);

class CSprite;

extern void CreateConcussiveBlast(const Vector &origin, const Vector &surfaceNormal, CBaseEntity *pOwner, float magnitude);












class CNPC_CombineGuard_Sphere : public CBaseCombatCharacter
{
	DECLARE_CLASS(CNPC_CombineGuard_Sphere, CBaseCombatCharacter);

public:
	CNPC_CombineGuard_Sphere(void);

	static CNPC_CombineGuard_Sphere *GuardSphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *Owner = NULL);
	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	//	void 				SphereTouch(CBaseEntity *pOther);
	//	void				SetSphereSize(int nSize);
	void				Detonate(void);
	void				Think(void);
	void vectors(const Vector &Origin);
private:
	DECLARE_DATADESC();

};



ConVar oc_combineguard_sphere_model("oc_combineguard_sphere_model", "models/ballsphere.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Combine guard shooting sphere model.");

#define GUARDSPHERE_MODEL	oc_combineguard_sphere_model.GetString() //"models/ShockRifle.mdl"


LINK_ENTITY_TO_CLASS(npc_combineguard_sphere, CNPC_CombineGuard_Sphere);

BEGIN_DATADESC(CNPC_CombineGuard_Sphere)


END_DATADESC()


CNPC_CombineGuard_Sphere *CNPC_CombineGuard_Sphere::GuardSphereCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *Owner)
{
	// Create a new entity with CShockRifleProjectile private data
	CNPC_CombineGuard_Sphere *GuardSphere = (CNPC_CombineGuard_Sphere *)CreateEntityByName("npc_combineguard_sphere");

	/*CBaseViewModel *vm = pentOwner->GetViewModel();

	if (!vm)
	Sphere->FollowEntity(pentOwner);
	else
	Sphere->FollowEntity(vm);*/

	//UTIL_SetOrigin(GuardSphere, vecOrigin);
	//GuardSphere->FollowEntity(Owner);
	GuardSphere->SetAbsAngles(angAngles);
	GuardSphere->Spawn();
	//GuardSphere->SetOwnerEntity(Owner);

	//Sphere->m_iDamage = iDamage;

	return GuardSphere;
}



CNPC_CombineGuard_Sphere::CNPC_CombineGuard_Sphere(void)
{
}


void CNPC_CombineGuard_Sphere::Precache(void)
{

	PrecacheModel(GUARDSPHERE_MODEL);
	PrecacheModel("models/ballsphere.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");

	PrecacheScriptSound("GrenadeSpit.Hit");

	PrecacheParticleSystem("grenade_spit_player");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
}

float ScaleG = 0.0f;
bool RevScaleG = false;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CombineGuard_Sphere::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_NONE);
	//SetSolidFlags(FSOLID_NOT_SOLID);
	//Vector SizeMin(0.2, 0.2, 0.2), SizeMax(1, 1, 1);
	ScaleG = 0.0f;
	RevScaleG = false;
	//SetModel("models/spitball_large.mdl");
	SetModel(GUARDSPHERE_MODEL);
	//UTIL_SetSize(this, SizeMin, SizeMax);

	SetModelScale(4.1f, 0.0f);

	//SetUse(&CBaseGrenade::DetonateUse);
	//SetTouch(&CCguard_Sphere::SphereTouch);

	SetThink(&CNPC_CombineGuard_Sphere::Think);
	SetNextThink(gpGlobals->curtime + 0.001f);

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);
}


void CNPC_CombineGuard_Sphere::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

void CNPC_CombineGuard_Sphere::Detonate(void)
{

	UTIL_Remove(this);
}


static ConVar oc_combineguard_maxscale("oc_combineguard_maxscale", "5", FCVAR_ARCHIVE);
static ConVar oc_combineguard_increment_scale("oc_combineguard_increment_scale", "0.1", FCVAR_ARCHIVE);
static ConVar oc_combineguard_decrement_scale("oc_combineguard_decrement_scale", "0.9", FCVAR_ARCHIVE);


float vector_x = 0.0f;
float vector_y = 0.0f;
float vector_z = 0.0f;
float angle_x = 0.0f;
float angle_y = 0.0f;
float angle_z = 0.0f;
void CNPC_CombineGuard_Sphere::Think(void)
{
	//	CBaseEntity *NPC = GetBaseEntity();
	// Add a doppler effect to the balls as they travel
	//CBaseEntity *NPC = AI_GetSinglePlayer();
	//if (NPC != NULL)
	{
		if (ScaleG <= cvar->FindVar("oc_combineguard_maxscale")->GetFloat() && !RevScaleG)
		{
			ScaleG = ScaleG + (cvar->FindVar("oc_combineguard_increment_scale")->GetFloat() * gpGlobals->frametime);

			if (ScaleG >= cvar->FindVar("oc_combineguard_maxscale")->GetFloat())
				RevScaleG = true;
		}
		else if (RevScaleG)
		{

			if (ScaleG > 0.0f && RevScaleG)
				ScaleG = ScaleG - (cvar->FindVar("oc_combineguard_decrement_scale")->GetFloat() * gpGlobals->frametime);
			else
			{
				ScaleG = 0.0f;
				Detonate();
			}
		}

		SetModelScale(ScaleG, 0.0f);




		Vector start2;
		QAngle StartAng;
		start2.x = vector_x; // cvar->FindVar("vector_x")->GetFloat();
		start2.y = vector_y; //cvar->FindVar("vector_y")->GetFloat();
		start2.z = vector_z; //cvar->FindVar("vector_z")->GetFloat();

		StartAng.x = angle_x; // cvar->FindVar("vector_x")->GetFloat();
		StartAng.y = angle_y; //cvar->FindVar("vector_y")->GetFloat();
		StartAng.z = angle_z;



		QAngle	angShootDir;
		//GetAttachment(1, start2, angShootDir);
		//vectors(start2);
		//Vector vecShootDir = NPC->GetShootEnemyDir(vecShootOrigin);
		UTIL_SetOrigin(this, start2);
		this->SetAbsAngles(StartAng);
	}

	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.001f);
}

void CNPC_CombineGuard_Sphere::vectors(const Vector &Origin)
{

}


















#define	COMBINEGUARD_MODEL	"models/combine_guard.mdl"

#define CGUARD_MSG_SHOT	1
#define CGUARD_MSG_SHOT_START 2

enum
{
	CGUARD_REF_INVALID = 0,
	CGUARD_REF_MUZZLE,
	CGUARD_REF_LEFT_SHOULDER,
	CGUARD_REF_HUMAN_HEAD,
	CGUARD_REF_RIGHT_ARM_HIGH,
	CGUARD_REF_RIGHT_ARM_LOW,
	CGUARD_REF_LEFT_ARM_HIGH,
	CGUARD_REF_LEFT_ARM_LOW,
	CGUARD_REF_TORSO,
	CGUARD_REF_LOWER_TORSO,
	CGUARD_REF_RIGHT_THIGH_HIGH,
	CGUARD_REF_RIGHT_THIGH_LOW,
	CGUARD_REF_LEFT_THIGH_HIGH,
	CGUARD_REF_LEFT_THIGH_LOW,
	//	CGUARD_SHOVE,
	CGUARD_REF_RIGHT_SHIN_HIGH,
	CGUARD_REF_RIGHT_SHIN_LOW,
	CGUARD_REF_LEFT_SHIN_HIGH,
	CGUARD_REF_LEFT_SHIN_LOW,
	CGUARD_REF_RIGHT_SHOULDER,

	NUM_CGUARD_ATTACHMENTS,
};

enum
{
	CGUARD_BGROUP_INVALID = -1,
	CGUARD_BGROUP_MAIN,
	CGUARD_BGROUP_GUN,
	CGUARD_BGROUP_RIGHT_SHOULDER,
	CGUARD_BGROUP_LEFT_SHOULDER,
	CGUARD_BGROUP_HEAD,
	CGUARD_BGROUP_RIGHT_ARM,
	CGUARD_BGROUP_LEFT_ARM,
	CGUARD_BGROUP_TORSO,
	CGUARD_BGROUP_LOWER_TORSO,
	CGUARD_BGROUP_RIGHT_THIGH,
	CGUARD_BGROUP_LEFT_THIGH,
	CGUARD_BGROUP_RIGHT_SHIN,
	CGUARD_BGROUP_LEFT_SHIN,

	NUM_CGUARD_BODYGROUPS,
};

struct armorPiece_t
{
	DECLARE_DATADESC();

	bool destroyed;
	int health;
};

class CNPC_CombineGuard : public CNPC_Combine
{
	DECLARE_CLASS(CNPC_CombineGuard, CNPC_Combine);
public:

	DECLARE_SERVERCLASS();

	CNPC_CombineGuard(void);

	//	int	OnTakeDamage( const CTakeDamageInfo &info );
	int	OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void Event_Killed(const CTakeDamageInfo &info);
	int	TranslateSchedule(int type);
	int	MeleeAttack1Conditions(float flDot, float flDist);
	int	RangeAttack1Conditions(float flDot, float flDist);
	void Gib(void);

	void Precache(void);
	void Spawn(void);
	void PrescheduleThink(void);
	void TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType);
	void HandleAnimEvent(animevent_t *pEvent);
	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);
	void			DropWeapon(void);
	bool AllArmorDestroyed(void);
	bool IsArmorPiece(int iArmorPiece);

	void			DeathSound(void);
	void			PainSound(const CTakeDamageInfo &info);
	void			IdleSound(void);

	void			LostEnemySound(void);
	void			FoundEnemySound(void);
	void			AnnounceAssault(void);

	float MaxYawSpeed(void);

	Class_T Classify(void) { return CLASS_COMBINEGUARD; }

	Activity NPC_TranslateActivity(Activity baseAct);

	virtual int	SelectSchedule(void);

	void BuildScheduleTestBits(void);

	int m_nMagnitude;

	DECLARE_DATADESC();

private:
	float	m_flChargeTime;
	int		m_beamIndex;
	int		m_haloIndex;
	bool update;
	bool m_fOffBalance;
	float m_flNextClobberTime;
	float			m_flNextPainSound;
	float			m_flNextLostSound;
	int	GetReferencePointForBodyGroup(int bodyGroup);

	void InitArmorPieces(void);
	void DamageArmorPiece(int pieceID, float damage, const Vector &vecOrigin, const Vector &vecDir);
	void DestroyArmorPiece(int pieceID);
	void Shove(void);
	void FireRangeWeapon(void);
	void UpdateLasers(void);
	float GetLegDamage(void);

	bool AimGunAt(CBaseEntity *pEntity, float flInterval);

	CSprite *m_pGlowSprite[NUM_CGUARD_ATTACHMENTS];

	armorPiece_t m_armorPieces[NUM_CGUARD_BODYGROUPS];

	int	m_nGibModel;

	float m_flGlowTime;
	float m_flLastRangeTime;

	float m_aimYaw;
	float m_aimPitch;

	int	m_YawControl;
	int	m_PitchControl;
	int	m_MuzzleAttachment;

	enum
	{
		SCHED_COMBINE_PATROL = 110,
		NEXT_SCHEDULE,
	};

	//DEFINE_CUSTOM_AI;
};

#define	CGUARD_DEFAULT_ARMOR_HEALTH	50

#define	COMBINEGUARD_MELEE1_RANGE	92
#define	COMBINEGUARD_MELEE1_CONE	0.5f

#define	COMBINEGUARD_RANGE1_RANGE	1024
#define	COMBINEGUARD_RANGE1_CONE	0.0f

#define	CGUARD_GLOW_TIME			0.5f

IMPLEMENT_SERVERCLASS_ST(CNPC_CombineGuard, DT_NPC_CombineGuard)
END_SEND_TABLE()

BEGIN_DATADESC_NO_BASE(armorPiece_t)

DEFINE_FIELD(destroyed, FIELD_BOOLEAN),
DEFINE_FIELD(health, FIELD_INTEGER),

END_DATADESC()

BEGIN_DATADESC(CNPC_CombineGuard)
DEFINE_FIELD(m_flNextPainSound, FIELD_TIME),
DEFINE_FIELD(m_flNextLostSound, FIELD_TIME),
DEFINE_FIELD(update, FIELD_BOOLEAN),
DEFINE_FIELD(m_flChargeTime, FIELD_TIME),
DEFINE_FIELD(m_fOffBalance, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextClobberTime, FIELD_TIME),
DEFINE_ARRAY(m_pGlowSprite, FIELD_CLASSPTR, NUM_CGUARD_ATTACHMENTS),
DEFINE_EMBEDDED_AUTO_ARRAY(m_armorPieces),
DEFINE_FIELD(m_nGibModel, FIELD_INTEGER),
DEFINE_FIELD(m_flGlowTime, FIELD_TIME),
DEFINE_FIELD(m_flLastRangeTime, FIELD_TIME),
DEFINE_FIELD(m_aimYaw, FIELD_FLOAT),
DEFINE_FIELD(m_aimPitch, FIELD_FLOAT),
DEFINE_FIELD(m_YawControl, FIELD_INTEGER),
DEFINE_FIELD(m_PitchControl, FIELD_INTEGER),
DEFINE_FIELD(m_MuzzleAttachment, FIELD_INTEGER),

END_DATADESC()

enum CombineGuardSchedules
{
	SCHED_CGUARD_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE,
	SCHED_COMBINEGUARD_CLOBBERED,
	SCHED_COMBINEGUARD_TOPPLE,
	SCHED_COMBINEGUARD_HELPLESS,
	SCHED_COMBINE_PATROL,
};

enum CombineGuardTasks
{
	TASK_CGUARD_RANGE_ATTACK1 = LAST_SHARED_TASK,
	TASK_COMBINEGUARD_SET_BALANCE,
};

enum CombineGuardConditions
{
	COND_COMBINEGUARD_CLOBBERED = LAST_SHARED_CONDITION,
};

int	ACT_CGUARD_IDLE_TO_ANGRY;
int ACT_COMBINEGUARD_CLOBBERED;
int ACT_COMBINEGUARD_TOPPLE;
int ACT_COMBINEGUARD_GETUP;
int ACT_COMBINEGUARD_HELPLESS;

#define	CGUARD_AE_SHOVE	11
#define	CGUARD_AE_FIRE 12
#define	CGUARD_AE_FIRE_START 13
#define	CGUARD_AE_GLOW 14
#define CGUARD_AE_LEFTFOOT 20
#define CGUARD_AE_RIGHTFOOT	21
#define CGUARD_AE_SHAKEIMPACT 22

CNPC_CombineGuard::CNPC_CombineGuard(void)
{
	m_nMagnitude = 1;
	m_flNextPainSound = 0;
	m_flNextLostSound = 0;
	m_bIsBodygrouped = true;
	m_iBodyGroup = 1;
	m_iBodyGroupValue = 1;
}

LINK_ENTITY_TO_CLASS(npc_combineguard, CNPC_CombineGuard);

void CNPC_CombineGuard::Precache(void)
{
	PrecacheModel(COMBINEGUARD_MODEL);
	PrecacheModel("models/gibs/Antlion_gib_Large_2.mdl"); //Antlion gibs for now

	PrecacheModel("sprites/blueflare1.vmt"); //For some reason this appears between his feet.

	PrecacheScriptSound("NPC_CombineGuard.FootstepLeft");
	PrecacheScriptSound("NPC_CombineGuard.FootstepRight");
	PrecacheScriptSound("NPC_CombineGuard.Pain1");
	PrecacheScriptSound("NPC_CombineGuard.SurrenderNow");
	PrecacheScriptSound("NPC_CombineGuard.RequestBackup");
	PrecacheScriptSound("NPC_CombineGuard.Fire");
	PrecacheScriptSound("NPC_CombineGuard.MeleeAttack");
	PrecacheScriptSound("NPC_CombineGuard.Charging");
	PrecacheParticleSystem("CombineGuard_Attack_Laser");
	PrecacheParticleSystem("CombineGuard_Muzzle_Flash");

	PrecacheParticleSystem("weapon_tracer_cguard");
	PrecacheParticleSystem("weapon_muzzle_smoke_cguard");
	PrecacheParticleSystem("weapon_muzzle_smoke_cguard1");
	PrecacheParticleSystem("charge");
	PrecacheParticleSystem("charge_npc");
	PrecacheParticleSystem("Weapon_cguard_Cannon");
	PrecacheParticleSystem("CombineGuard_Outer");
	PrecacheParticleSystem("CombineGuard_Outer_impact");

	m_beamIndex = engine->PrecacheModel("sprites/bluelaser1.vmt");
	m_haloIndex = engine->PrecacheModel("sprites/blueshaft1.vmt");
	BaseClass::Precache();
}

void CNPC_CombineGuard::InitArmorPieces(void)
{
	for (int i = 1; i < NUM_CGUARD_BODYGROUPS; i++)
	{
		SetBodygroup(i, true);

		m_armorPieces[i].health = CGUARD_DEFAULT_ARMOR_HEALTH;


		if (IsArmorPiece(i))
		{
			m_armorPieces[i].destroyed = false;
		}
		else
		{
			m_armorPieces[i].destroyed = true;
		}
	}
}

void CNPC_CombineGuard::Spawn(void)
{
	Precache();

	SetModel(COMBINEGUARD_MODEL);

	SetHullType(HULL_MEDIUM_TALL);	// HULL_LARGE	// BJ: Changed to Hunter hullsize, it's perfect
	SetHullSizeNormal();	// BJ: Reactivated
	SetNavType(NAV_GROUND);
	m_NPCState = NPC_STATE_NONE;


	m_iHealth = m_iMaxHealth = sk_combineguard_health.GetFloat();
	m_flFieldOfView = 0.1f;


	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	m_flGlowTime = gpGlobals->curtime;
	m_flLastRangeTime = gpGlobals->curtime;
	m_aimYaw = 0;
	m_aimPitch = 0;
	update = false;
	m_flChargeTime = gpGlobals->curtime;
	m_flNextClobberTime = gpGlobals->curtime;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_OPEN_DOORS | bits_CAP_SQUAD);	// BJ: Additions to fit more beta like. open doors, form squads

	/*for (int i = 1; i < NUM_CGUARD_ATTACHMENTS; i++)
	{
	m_pGlowSprite[i] = CSprite::SpriteCreate("sprites/blueflare1.vmt", GetAbsOrigin(), false);

	Assert(m_pGlowSprite[i]);

	if (m_pGlowSprite[i] == NULL)
	return;

	m_pGlowSprite[i]->TurnOff();
	m_pGlowSprite[i]->SetTransparency(kRenderGlow, 16, 16, 16, 255, kRenderFxNoDissipation);
	m_pGlowSprite[i]->SetScale(1.0f);
	m_pGlowSprite[i]->SetAttachment(this, i);
	}*/

	NPCInit();

	//InitArmorPieces();	// BJ: Not needed, not implemented anyway in model.

	SetBodygroup(1, 0);		// BJ: always draw gun? ok, but it already ON in qc

	m_YawControl = LookupPoseParameter("aim_yaw");
	m_PitchControl = LookupPoseParameter("aim_pitch");
	m_MuzzleAttachment = LookupAttachment("muzzle");

	m_fOffBalance = false;

	BaseClass::Spawn();

	if ((cvar->FindVar("sk_combineguard_allow_jump")->GetFloat() == 0) || (!g_pGameRules->IsSkillLevel(SKILL_HARD)))	// BJ: But on hard still allow him jumping
	{
		CapabilitiesRemove(bits_CAP_MOVE_JUMP | bits_CAP_DUCK);	// BJ: LOL baseclass override jumping from combine!!! so lock them AFTER
	}

	if (!cvar->FindVar("sk_combineguard_allow_climb")->GetFloat() == 0)
	{
		CapabilitiesAdd( bits_CAP_MOVE_CLIMB );	// BJ: LOL climbing cguard!
	}
	SetBloodColor(BLOOD_COLOR_YELLOW);
}


bool DoOnceS = false;
bool DoOnceB = false;
void CNPC_CombineGuard::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (HasCondition(COND_COMBINEGUARD_CLOBBERED))
	{
		//Msg("CLOBBERED!\n");
	}

	for (int i = 1; i < NUM_CGUARD_ATTACHMENTS; i++)
	{
		if (m_pGlowSprite[i] == NULL)
			continue;

		if (m_flGlowTime > gpGlobals->curtime)
		{
			float brightness;
			float perc = (m_flGlowTime - gpGlobals->curtime) / CGUARD_GLOW_TIME;

			m_pGlowSprite[i]->TurnOn();

			brightness = perc * 92.0f;
			m_pGlowSprite[i]->SetTransparency(kRenderGlow, brightness, brightness, brightness, 255, kRenderFxNoDissipation);

			m_pGlowSprite[i]->SetScale(perc * 1.0f);
		}
		else
		{
			m_pGlowSprite[i]->TurnOff();
		}
	}

	if (GetEnemy() != NULL)
	{
		AimGunAt(GetEnemy(), 0.1f);

		/*if (!DoOnceB)
		{
		EmitSound("NPC_CombineGuard.SurrenderNow");
		DoOnceB = true;
		}*/
	}
	if (GetEnemy() == NULL)
	{
		DoOnceB = false;
	}

	Vector vecDamagePoint;
	QAngle vecDamageAngles;

	for (int i = 1; i < NUM_CGUARD_BODYGROUPS; i++)
	{
		if (m_armorPieces[i].destroyed)
		{
			int	referencePoint = GetReferencePointForBodyGroup(i);

			if (referencePoint == CGUARD_REF_INVALID)
				continue;

			GetAttachment(referencePoint, vecDamagePoint, vecDamageAngles);

			if (random->RandomInt(0, 4) == 0)
			{
				if (random->RandomInt(0, 800) == 0)
				{
					//		ExplosionCreate( vecDamagePoint, vecDamageAngles, this, 0.5f, 0, false );// Doesn't work for some reason.
				}
				else
				{
				}
			}
		}
	}

	if ((m_iHealth <= m_iMaxHealth * 0.3f) && (!DoOnceS))
	{
		EmitSound("NPC_CombineGuard.RequestBackup");
		//DevMsg("RequestBackup");
		DoOnceS = true;
	}

	if (update)
	{
		AutoMovement();

		Vector vecEnemyLKP = GetEnemyLKP();
		if (!FInAimCone(vecEnemyLKP))
		{
			GetMotor()->SetIdealYawToTargetAndUpdate(vecEnemyLKP, AI_KEEP_YAW_SPEED);
		}
		else
		{
			GetMotor()->SetIdealYawAndUpdate(GetMotor()->GetIdealYaw(), AI_KEEP_YAW_SPEED);
		}

		UpdateLasers();
	}
}

void CNPC_CombineGuard::Event_Killed(const CTakeDamageInfo &info)
{
	DropWeapon();
	BaseClass::Event_Killed(info);
	static int CG_gunbodygroup1 = FindBodygroupByName("cguard_gun");	// BJ: Hey remove that from view
	SetBodygroup(CG_gunbodygroup1, 1);

	RemoveAllDecals();		// BJ: Bullet decals on weapon still here, so CLEAN them all
	//	UTIL_Remove(this);

	//	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), NULL, random->RandomInt(30, 40), 0, true);

	//  Gib();
}

void CNPC_CombineGuard::DropWeapon(void)
{
	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName("weapon_cguard"));
	pGib->SetAbsOrigin(GetAbsOrigin());
	pGib->SetAbsAngles(GetAbsAngles());
	pGib->SetAbsVelocity(GetAbsVelocity());
	//pGib->SetModel(GetHeadpropModel());
	pGib->Spawn();
	pGib->SetMoveType(MOVETYPE_VPHYSICS);

	Vector vecVelocity;
	pGib->GetMassCenter(&vecVelocity);
	vecVelocity -= WorldSpaceCenter();
	vecVelocity.z = fabs(vecVelocity.z);
	VectorNormalize(vecVelocity);

	//	float flRandomVel = random->RandomFloat( 35, 75 );
	//	vecVelocity *= (iVelocity * flRandomVel) / 15;
	//	vecVelocity.z += 100.0f;
	AngularImpulse angImpulse = RandomAngularImpulse(-500, 500);

	IPhysicsObject *pObj = pGib->VPhysicsGetObject();
	if (pObj != NULL)
	{
		pObj->AddVelocity(&vecVelocity, &angImpulse);
	}
	pGib->SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);
}

void CNPC_CombineGuard::Gib(void)
{
	// Sparks
	for (int i = 0; i < 4; i++)
	{
		Vector sparkPos = GetAbsOrigin();
		sparkPos.x += random->RandomFloat(-12, 12);
		sparkPos.y += random->RandomFloat(-12, 12);
		sparkPos.z += random->RandomFloat(-12, 12);
		g_pEffects->Sparks(sparkPos);
	}
	// Smoke
	UTIL_Smoke(GetAbsOrigin(), random->RandomInt(10, 15), 10);

	// Light
	CBroadcastRecipientFilter filter;
	te->DynamicLight(filter, 0.0,
		&GetAbsOrigin(), 255, 180, 100, 0, 100, 0.1, 0);

	// Throw gibs
	CGib::SpawnSpecificGibs(this, CGUARD_GIB_COUNT, 800, 1000, "models/gibs/Antlion_gib_Large_2.mdl");

	//	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), NULL, random->RandomInt(30, 40), 0, true);

	UTIL_Remove(this);
}


void CNPC_CombineGuard::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case CGUARD_AE_SHAKEIMPACT:
		Shove();
		UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START);
		break;

	case CGUARD_AE_LEFTFOOT:
	{
		EmitSound("NPC_CombineGuard.FootstepLeft");
		//DevMsg("FootstepLeft");
	}
	break;

	case CGUARD_AE_RIGHTFOOT:
	{
		EmitSound("NPC_CombineGuard.FootstepRight");
		//DevMsg("FootstepRight");
	}
	break;

	case CGUARD_AE_SHOVE:
	{
		Shove();

		//int Rndm = RandomInt(1, 3);
		//if (Rndm == 2)
			EmitSound("NPC_CombineGuard.MeleeAttack");
	}
	break;

	case CGUARD_AE_FIRE:
	{
		m_flLastRangeTime = gpGlobals->curtime + 6.0f;
		FireRangeWeapon();
		update = false;
		EmitSound("NPC_CombineGuard.Charging");

		EntityMessageBegin(this, true);
		WRITE_BYTE(CGUARD_MSG_SHOT);
		MessageEnd();
	}
	break;

	case CGUARD_AE_FIRE_START:
	{
		EmitSound("NPC_CombineGuard.Fire");
		update = true;
		EntityMessageBegin(this, true);
		WRITE_BYTE(CGUARD_MSG_SHOT_START);
		MessageEnd();
	}
	break;

	case CGUARD_AE_GLOW:
		m_flGlowTime = gpGlobals->curtime + CGUARD_GLOW_TIME;
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		return;
	}
}

void CNPC_CombineGuard::Shove(void) // Doesn't work for some reason // It sure works now! -Dan
{
	if (GetEnemy() == NULL)
		return;

	CBaseEntity *pHurt = NULL;

	Vector forward;
	AngleVectors(GetLocalAngles(), &forward);

	float flDist = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length();
	Vector2D v2LOS = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).AsVector2D();
	Vector2DNormalize(v2LOS);
	float flDot = DotProduct2D(v2LOS, forward.AsVector2D());

	flDist -= GetEnemy()->WorldAlignSize().x * 0.5f;

	if (flDist < COMBINEGUARD_MELEE1_RANGE && flDot >= COMBINEGUARD_MELEE1_CONE)
	{
		Vector vStart = GetAbsOrigin();
		vStart.z += WorldAlignSize().z * 0.5;

		Vector vEnd = GetEnemy()->GetAbsOrigin();
		vEnd.z += GetEnemy()->WorldAlignSize().z * 0.5;

		pHurt = CheckTraceHullAttack(vStart, vEnd, Vector(-16, -16, 0), Vector(16, 16, 24), 25, DMG_CLUB);
	}

	if (pHurt)
	{
		pHurt->ViewPunch(QAngle(-20, 0, 20));

		UTIL_ScreenShake(pHurt->GetAbsOrigin(), 100.0, 1.5, 1.0, 2, SHAKE_START);

		color32 white = { 255, 155, 155, 64 };
		UTIL_ScreenFade(pHurt, white, 0.5f, 0.1f, FFADE_IN);

		if (pHurt->IsPlayer())
		{
			Vector forward, up;
			AngleVectors(GetLocalAngles(), &forward, NULL, &up);
			pHurt->ApplyAbsVelocityImpulse(forward * 300 + up * 250);
		}
	}


}

int CNPC_CombineGuard::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT)
	{
		return SCHED_COMBINE_PATROL;// SCHED_PATROL_WALK_LOOP;
	}

	if (HasCondition(COND_COMBINEGUARD_CLOBBERED))
	{
		ClearCondition(COND_COMBINEGUARD_CLOBBERED);

		if (m_fOffBalance)
		{
			int iArmorPiece;
			do
			{
				iArmorPiece = random->RandomInt(CGUARD_BGROUP_RIGHT_SHOULDER, CGUARD_BGROUP_LEFT_SHIN);
			} while (m_armorPieces[iArmorPiece].destroyed);

			DestroyArmorPiece(iArmorPiece);

			//DevMsg("DESTROYING PIECE:%d\n", iArmorPiece);

			if (AllArmorDestroyed())
			{
				//DevMsg(" NO!!!!!!!! I'M DEADZ0R!!\n");
				return SCHED_COMBINEGUARD_HELPLESS;
				//	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), NULL, random->RandomInt(30, 40), 0, true);
				UTIL_Remove(this);
				//When I put the guard in his "helpless" state ingame by throwing props at him, he sat there in his melee animation and would not attack me. I guess the Combine Guard
				//Had a "lying on the ground dead" animation at one point.
			}
			else
			{
				return SCHED_COMBINEGUARD_TOPPLE;
			}
		}
		else
		{
			return SCHED_COMBINE_PATROL;//SCHED_COMBINEGUARD_CLOBBERED;
		}
	}

	/*if ()
	return SCHED_CHASE_ENEMY;*/

	return BaseClass::SelectSchedule();
}

void CNPC_CombineGuard::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		//m_nShots = 1;
		//m_flShotDelay = 5.f;

		m_flNextAttack = gpGlobals->curtime + 1.f;
		ResetIdealActivity(ACT_RANGE_ATTACK1);
		m_flLastAttackTime = gpGlobals->curtime;
	}
	break;

	case TASK_COMBINEGUARD_SET_BALANCE:
	{
		if (pTask->flTaskData == 0.0)
		{
			m_fOffBalance = false;
		}
		else
		{
			m_fOffBalance = true;
		}

		TaskComplete();
	}
	break;

	/*
	case TASK_CGUARD_RANGE_ATTACK1:
	{
	Vector flEnemyLKP = GetEnemyLKP();
	GetMotor()->SetIdealYawToTarget(flEnemyLKP);
	}
	SetActivity(ACT_RANGE_ATTACK1);
	return;*/

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPC_CombineGuard::RunTask(const Task_t *pTask)
{
	/*switch (pTask->iTask)
	{
	case TASK_CGUARD_RANGE_ATTACK1:
	{
	Vector flEnemyLKP = GetEnemyLKP();
	GetMotor()->SetIdealYawToTargetAndUpdate(flEnemyLKP);

	if (IsActivityFinished())
	{
	TaskComplete();
	return;
	}
	}
	return;
	}*/

	BaseClass::RunTask(pTask);
}

float CNPC_CombineGuard::GetLegDamage(void)
{
	float damageTotal = 0.0f;

	if (m_armorPieces[CGUARD_BGROUP_RIGHT_THIGH].destroyed)
	{
		damageTotal += 0.25f;
	}

	if (m_armorPieces[CGUARD_BGROUP_LEFT_THIGH].destroyed)
	{
		damageTotal += 0.25f;
	}

	if (m_armorPieces[CGUARD_BGROUP_RIGHT_SHIN].destroyed)
	{
		damageTotal += 0.25f;
	}

	if (m_armorPieces[CGUARD_BGROUP_LEFT_SHIN].destroyed)
	{
		damageTotal += 0.25f;
	}

	return damageTotal;
}

#define TRANSLATE_SCHEDULE( type, in, out ) { if ( type == in ) return out; }

Activity CNPC_CombineGuard::NPC_TranslateActivity(Activity baseAct)
{
	if (baseAct == ACT_RUN)
	{
		float legDamage = GetLegDamage();

		if (legDamage > 0.75f)
		{
			return (Activity)ACT_WALK;
		}
		else if (legDamage > 0.5f)
		{
			return (Activity)ACT_WALK;
		}
	}

	if (baseAct == ACT_RUN)
	{
		return (Activity)ACT_WALK;
	}

	if (baseAct == ACT_IDLE && m_NPCState != NPC_STATE_IDLE)
	{
		return (Activity)ACT_IDLE_ANGRY;
	}

	return baseAct;
}

int CNPC_CombineGuard::TranslateSchedule(int type)
{
	switch (type)
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_CGUARD_RANGE_ATTACK1;
		break;
	}

	return BaseClass::TranslateSchedule(type);
}

int CNPC_CombineGuard::GetReferencePointForBodyGroup(int bodyGroup)
{
	switch (bodyGroup)
	{
	case CGUARD_BGROUP_MAIN:
	case CGUARD_BGROUP_GUN:
		return CGUARD_REF_INVALID;
		break;

	case CGUARD_BGROUP_RIGHT_SHOULDER:
		return CGUARD_REF_RIGHT_SHOULDER;
		break;

	case CGUARD_BGROUP_LEFT_SHOULDER:
		return CGUARD_REF_LEFT_SHOULDER;
		break;

	case CGUARD_BGROUP_HEAD:
		return CGUARD_REF_HUMAN_HEAD;
		break;

	case CGUARD_BGROUP_RIGHT_ARM:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_RIGHT_ARM_LOW;
		else
			return CGUARD_REF_RIGHT_ARM_HIGH;

		break;

	case CGUARD_BGROUP_LEFT_ARM:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_LEFT_ARM_LOW;
		else
			return CGUARD_REF_LEFT_ARM_HIGH;

		break;

	case CGUARD_BGROUP_TORSO:
		return CGUARD_REF_TORSO;
		break;

	case CGUARD_BGROUP_RIGHT_THIGH:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_RIGHT_THIGH_LOW;
		else
			return CGUARD_REF_RIGHT_THIGH_HIGH;

		break;

	case CGUARD_BGROUP_LEFT_THIGH:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_LEFT_THIGH_LOW;
		else
			return CGUARD_REF_LEFT_THIGH_HIGH;

		break;

	case CGUARD_BGROUP_RIGHT_SHIN:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_RIGHT_SHIN_LOW;
		else
			return CGUARD_REF_RIGHT_SHIN_HIGH;
		break;

	case CGUARD_BGROUP_LEFT_SHIN:
		if (random->RandomInt(0, 1))
			return CGUARD_REF_LEFT_SHIN_LOW;
		else
			return CGUARD_REF_LEFT_SHIN_HIGH;
		break;

	case CGUARD_BGROUP_LOWER_TORSO:
		return CGUARD_REF_LOWER_TORSO;
		break;
	}

	return CGUARD_REF_INVALID;
}

void CNPC_CombineGuard::DestroyArmorPiece(int pieceID)
{
	int	refPoint = GetReferencePointForBodyGroup(pieceID);

	if (refPoint == CGUARD_REF_INVALID)
		return;

	Vector vecDamagePoint;
	QAngle vecDamageAngles;

	GetAttachment(refPoint, vecDamagePoint, vecDamageAngles);

	Vector vecVelocity, vecSize;

	vecVelocity.Random(-1.0, 1.0);
	vecVelocity *= random->RandomInt(16, 64);

	vecSize = Vector(32, 32, 32);

	Vector gibVelocity = RandomVector(-100, 100) * 10;

	CPVSFilter filter(GetAbsOrigin());
	te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle, Vector(40, 40, 40), gibVelocity, m_nGibModel, 100, 0, 2.5, BREAK_METAL);

	m_armorPieces[pieceID].destroyed = true;
	SetBodygroup(pieceID, false);

	SetCondition(COND_LIGHT_DAMAGE);
}

void CNPC_CombineGuard::DamageArmorPiece(int pieceID, float damage, const Vector &vecOrigin, const Vector &vecDir)
{
	if (m_armorPieces[pieceID].destroyed)
	{
		if ((random->RandomInt(0, 8) == 0) && (pieceID != CGUARD_BGROUP_HEAD))
		{
			g_pEffects->Ricochet(vecOrigin, (vecDir * -1.0f));
		}
		return;
	}

	m_armorPieces[pieceID].health -= damage;

	if (m_armorPieces[pieceID].health <= 0.0f)
	{
		DestroyArmorPiece(pieceID);
		return;
	}

	g_pEffects->Sparks(vecOrigin);
}

int Upd = 0;
void CNPC_CombineGuard::UpdateLasers(void)
{
	if (!update)
		return;

	if ((GetEnemy() != NULL) && (GetEnemy()->Classify() == CLASS_BULLSEYE))
	{
		GetEnemy()->TakeDamage(CTakeDamageInfo(this, this, 1.0f, DMG_GENERIC));
	}

	Vector vecSrc, vecAiming;
	Vector vecShootOrigin;


	GetVectors(&vecAiming, NULL, NULL);
	vecSrc = WorldSpaceCenter() + vecAiming * 64;


	Vector	impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	vecShootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);
	QAngle angDir;

	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);

	trace_t	tr;
	AI_TraceLine(vecSrc, impactPoint, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);


	Vector	start, end, v_forward, v_right, v_up;
	GetVectors(&v_forward, &v_right, &v_up);
	start = GetAbsOrigin() + Vector(0, 0, 55);
	Vector start2 = GetShootEnemyDir(vecShootOrigin);
	start2 += (v_forward * 8.0f) + (v_right * 3.0f) + (v_up * -2.0f);
	end = start2 + (v_forward * MAX_TRACE_LENGTH);
	GetAttachment(LookupAttachment("muzzle"), start2, angShootDir);
	float	angleOffset = ((3.0f - (m_flChargeTime - gpGlobals->curtime)) / 3.0f);
	Vector	offset[4];
	offset[0] = Vector(0.0f, 0.5f, -0.5f);
	offset[1] = Vector(0.0f, 0.5f, 0.5f);
	offset[2] = Vector(0.0f, -0.5f, -0.5f);
	offset[3] = Vector(0.0f, -0.5f, 0.5f);
	QAngle  v_ang;
	Vector	v_dir;
	angleOffset *= 2.0f;
	if (angleOffset > 1.0f)
		angleOffset = 1.0f;
	for (int i = 0; i < 4; i++)
	{
		Vector	ofs = start2 + (v_forward * offset[i][0]) + (v_right * offset[i][1]) + (v_up * offset[i][2]);

		float hScale = (offset[i][1] <= 0.0f) ? 1.0f : -1.0f;
		float vScale = (offset[i][2] <= 0.0f) ? 1.0f : -1.0f;

		VectorAngles(v_forward, v_ang);
		v_ang[PITCH] = UTIL_AngleMod(v_ang[PITCH] + ((1.0f - angleOffset) * 15.0f * vScale));
		v_ang[YAW] = UTIL_AngleMod(v_ang[YAW] + ((1.0f - angleOffset) * 15.0f * hScale));

		AngleVectors(v_ang, &v_dir);

		//trace_t	tr;
		//UTIL_TraceLine(ofs, ofs + (v_dir * MAX_TRACE_LENGTH), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		//UTIL_Beam(ofs, tr.endpos, m_beamIndex, 0, 0, 0.4f, 0.1f, 1, 0, 1, 0, 255, 255, 255, 32, 100);//////////////

		//UTIL_Beam(ofs, tr.endpos, m_haloIndex, 0, 0, 0.4f, 0.1f, 1, 0, 1, 16, 255, 255, 255, 8, 100);//////////////




		/*	Vector	vFirePos, vForward1, vRight1, vUp1;
		pPlayer->GetVectors(&vForward1, &vRight1, &vUp1);
		vFirePos = pPlayer->Weapon_ShootPosition();
		vFirePos += (vForward1 * 8.0f) + (vRight1 * 6.0f) + (vUp1 * -2.8f);
		DispatchParticleEffect("Weapon_Combine_cguard_Cannon", vFirePos /*vecSrc*//*, tr.endpos, vec3_angle, NULL);*/


		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &ofs, 0, 70, 255, 6, 300, 0.07, 0);
	}

	// Just using the gunship tracers for a placeholder unless a better effect can be found. Maybe use the strider cannon's tracer or something.


	//DispatchParticleEffect("CombineGuard_Muzzle_Flash", PATTACH_POINT_FOLLOW, this, "muzzle", false);


	///////////////////////////////////DispatchParticleEffect("charge_npc", PATTACH_POINT_FOLLOW, this, "muzzle", false);


	/*cvar->FindVar("vector_x")->SetValue(vecShootOrigin.x);
	cvar->FindVar("vector_y")->SetValue(vecShootOrigin.y);
	cvar->FindVar("vector_z")->SetValue(vecShootOrigin.z);*/

	Vector Zero;
	QAngle ZeroAng;
	GetAttachment(LookupAttachment("muzzle"), Zero, ZeroAng);
	vector_x = Zero.x;
	vector_y = Zero.y;
	vector_z = Zero.z;
	angle_x = ZeroAng.x;
	angle_y = ZeroAng.y;
	angle_z = ZeroAng.z;

	angle_x = angle_x - 90.0f;

	Upd++;
	if (Upd == 1)
	{
		//Vector	ofs = start2 + (v_forward * offset[1][0]) + (v_right * offset[1][1]) + (v_up * offset[1][2]);
		CNPC_CombineGuard_Sphere *GuardSphere = CNPC_CombineGuard_Sphere::GuardSphereCreate(Zero, ZeroAng, 0, this);

		//GuardSphere->vectors(vecShootOrigin);
		//UTIL_SetOrigin(this, vecShootOrigin);
		GuardSphere->SetOwnerEntity(this);
		//GuardSphere->FollowEntity(this);

	}
	else
		Upd = 10;
	//UTIL_Tracer(tr.startpos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat(0, 2000), true, "GunshipTracer");
}

void CNPC_CombineGuard::TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType)
{
	Vector vecDamagePoint = ptr->endpos;
	update = false;
	if (bitsDamageType & (DMG_BLAST))
	{
		Vector vecOrigin;
		QAngle vecAngles;
		float flNearestDist = 99999999.0f;
		float flDist;
		int	nReferencePoint;
		int	nNearestGroup = CGUARD_BGROUP_MAIN;
		float flAdjustedDamage = flDamage;

		for (int i = 1; i < NUM_CGUARD_BODYGROUPS; i++)
		{
			if (m_armorPieces[i].destroyed)
			{
				flAdjustedDamage *= 0.9f;
				continue;
			}

			nReferencePoint = GetReferencePointForBodyGroup(i);

			if (nReferencePoint == CGUARD_REF_INVALID)
				continue;

			GetAttachment(nReferencePoint, vecOrigin, vecAngles);

			flDist = (vecOrigin - ptr->endpos).Length();

			if (flDist < flNearestDist)
			{
				flNearestDist = flDist;
				nNearestGroup = i;
			}
		}

		if (nNearestGroup != CGUARD_BGROUP_MAIN)
		{
			DamageArmorPiece(nNearestGroup, flAdjustedDamage, vecDamagePoint, vecDir);
			return;
		}
	}

	if (ptr->hitgroup != CGUARD_BGROUP_MAIN)
	{
		DamageArmorPiece(ptr->hitgroup, flDamage, vecDamagePoint, vecDir);
	}
	else
	{
	}

	// BJ REWORK THIS BLOCK
	// BJ Hitgroup 1 - head
	if (ptr->hitgroup == 1 )
	{
		flDamage = 50;
		//if (flDamage <= 0)
			//flDamage = 0.1;

		SetDamage(flDamage);
	}
	trace_t tr;
	CreateConcussiveBlast(vecDir, tr.plane.normal, this, 1.0);
	//Do the radius damage
	RadiusDamage(CTakeDamageInfo(this, this, 125 * m_nMagnitude, DMG_SONIC), GetAbsOrigin(), 128 * m_nMagnitude, CLASS_NONE, this);

	Vector Dir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), Dir, angShootDir);
	DispatchParticleEffect("CombineGuard_Outer", vecDir, angShootDir);
	DispatchParticleEffect("CombineGuard_Outer_impact", vecDir, angShootDir);


	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &vecDir, 0, 70, 255, 6, 300, 0.07, 0);
	Upd = 0;
}


int CNPC_CombineGuard::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	if (info.GetAttacker() && info.GetAttacker() == this)
		return 0;

	CTakeDamageInfo newInfo = info;
	if (newInfo.GetDamageType() & (DMG_SONIC))//DMG_BURN |
	{
		newInfo.ScaleDamage(0);
		//DevMsg("Fire damage; no actual damage is taken\n");
	}
	else
	{
		/*int Rndm = RandomInt(1, 3);

		if (Rndm == 1)
		EmitSound("NPC_CombineGuard.Pain1");*/

	}
	int nDamageTaken = BaseClass::OnTakeDamage_Alive(newInfo);

	//	m_bHeadshot = false;
	//	m_bCanisterShot = false;
	//DevMsg("nDamageTaken: %.2f \n", nDamageTaken);	// BJ: Lol it's nothing, uncorrect

	return (newInfo.GetDamageType() & (DMG_SONIC)) ? 0 : nDamageTaken;
}


int CNPC_CombineGuard::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > COMBINEGUARD_MELEE1_RANGE)
		return COND_TOO_FAR_TO_ATTACK;

	if (flDot < COMBINEGUARD_MELEE1_CONE)
		return COND_NOT_FACING_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}

int CNPC_CombineGuard::RangeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > COMBINEGUARD_RANGE1_RANGE)
		return COND_TOO_FAR_TO_ATTACK;

	if (flDot < COMBINEGUARD_RANGE1_CONE)
		return COND_NOT_FACING_ATTACK;

	if (m_flLastRangeTime > gpGlobals->curtime)
		return COND_TOO_FAR_TO_ATTACK;

	return COND_CAN_RANGE_ATTACK1;
}

void CNPC_CombineGuard::FireRangeWeapon(void)
{
	if ((GetEnemy() != NULL) && (GetEnemy()->Classify() == CLASS_BULLSEYE))
	{
		GetEnemy()->TakeDamage(CTakeDamageInfo(this, this, 1.0f, DMG_GENERIC));
	}

	Vector vecSrc, vecAiming;
	Vector vecShootOrigin;

	GetVectors(&vecAiming, NULL, NULL);
	vecSrc = WorldSpaceCenter() + vecAiming * 64;

	Vector	impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	vecShootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);
	QAngle angDir;

	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin);//, angShootDir);


	Vector	vTarget;
	// If our enemy is looking at us and far enough away, lead him
	if (GetEnemy())
	{
		if (HasCondition(COND_ENEMY_FACING_ME) && UTIL_DistApprox(GetAbsOrigin(), GetEnemy()->GetAbsOrigin()) > (40 * 12))
		{
			UTIL_PredictedPosition(GetEnemy(), 0.5f, &vTarget);
			vTarget.z = GetEnemy()->GetAbsOrigin().z;
		}
		else
		{
			// Otherwise he can't see us and he won't be able to dodge
			vTarget = GetEnemy()->BodyTarget(vecSrc, true);
		}
	}
	VectorAngles(vecSrc + vTarget, angShootDir);

	trace_t	tr;
	AI_TraceLine(vecSrc, vTarget, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	// Just using the gunship tracers for a placeholder unless a better effect can be found. Maybe use the strider cannon's tracer or something.
	//UTIL_Tracer(vecShootOrigin, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat(0, 2000), true, "GunshipTracer");
	//UTIL_Tracer(vecShootOrigin, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat(0, 3000), true, "GunshipTracer");
	//UTIL_Tracer(vecShootOrigin, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat(0, 4000), true, "GunshipTracer");
	DispatchParticleEffect("CombineGuard_Attack_Laser", vecShootOrigin, tr.endpos, angShootDir, NULL /*pOperator->GetEnemy()*/);
	DispatchParticleEffect("weapon_tracer_cguard", vecShootOrigin /*vecSrc*/, tr.endpos, angShootDir, NULL);
	DispatchParticleEffect("weapon_muzzle_smoke_cguard", PATTACH_POINT_FOLLOW, this, "muzzle", true);
	DispatchParticleEffect("weapon_muzzle_smoke_cguard1", PATTACH_POINT_FOLLOW, this, "muzzle", false);

	CreateConcussiveBlast(tr.endpos, tr.plane.normal, this, 1.0);
	//Do the radius damage
	RadiusDamage(CTakeDamageInfo(this, this, 125 * m_nMagnitude, DMG_BLAST), GetAbsOrigin(), 128 * m_nMagnitude, CLASS_NONE, this);

	DispatchParticleEffect("CombineGuard_Outer", tr.endpos, angShootDir);
	DispatchParticleEffect("CombineGuard_Outer_impact", tr.endpos, angShootDir);

	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &tr.endpos, 0, 70, 255, 2, 300, 0.07, 0);
	update = false;
	Upd = 0;
}

#define	DEBUG_AIMING 0

bool CNPC_CombineGuard::AimGunAt(CBaseEntity *pEntity, float flInterval)
{
	if (!pEntity)
		return true;

	matrix3x4_t gunMatrix;
	GetAttachment(m_MuzzleAttachment, gunMatrix);

	Vector localEnemyPosition;
	VectorITransform(pEntity->GetAbsOrigin(), gunMatrix, localEnemyPosition);

	QAngle localEnemyAngles;
	VectorAngles(localEnemyPosition, localEnemyAngles);

	localEnemyAngles.x = UTIL_AngleDiff(localEnemyAngles.x, 0);
	localEnemyAngles.y = UTIL_AngleDiff(localEnemyAngles.y, 0);

	float targetYaw = m_aimYaw + localEnemyAngles.y;
	float targetPitch = m_aimPitch + localEnemyAngles.x;

	QAngle unitAngles = localEnemyAngles;
	float angleDiff = sqrt(localEnemyAngles.y * localEnemyAngles.y + localEnemyAngles.x * localEnemyAngles.x);
	const float aimSpeed = 1;

	float yawSpeed = fabsf(aimSpeed*flInterval*localEnemyAngles.y);
	float pitchSpeed = fabsf(aimSpeed*flInterval*localEnemyAngles.x);

	yawSpeed = max(yawSpeed, 15);
	pitchSpeed = max(pitchSpeed, 15);

	m_aimYaw = UTIL_Approach(targetYaw, m_aimYaw, yawSpeed);
	m_aimPitch = UTIL_Approach(targetPitch, m_aimPitch, pitchSpeed);

	SetPoseParameter(m_YawControl, m_aimYaw);
	SetPoseParameter(m_PitchControl, m_aimPitch);

	m_aimPitch = GetPoseParameter(m_PitchControl);
	m_aimYaw = GetPoseParameter(m_YawControl);



	if (angleDiff < 1)
		return true;

	return false;
}



float CNPC_CombineGuard::MaxYawSpeed(void)
{
	if (GetActivity() == ACT_RANGE_ATTACK1)
	{
		return 1.0f;
	}

	return 60.0f;
}

void CNPC_CombineGuard::BuildScheduleTestBits(void)
{
	SetCustomInterruptCondition(COND_COMBINEGUARD_CLOBBERED);
}

bool CNPC_CombineGuard::IsArmorPiece(int iArmorPiece)
{
	switch (iArmorPiece)
	{
	case CGUARD_BGROUP_MAIN:
	case CGUARD_BGROUP_GUN:
	case CGUARD_BGROUP_HEAD:
	case CGUARD_BGROUP_RIGHT_ARM:
	case CGUARD_BGROUP_LEFT_ARM:
	case CGUARD_BGROUP_TORSO:
	case CGUARD_BGROUP_LOWER_TORSO:
	case CGUARD_BGROUP_RIGHT_SHIN:
	case CGUARD_BGROUP_LEFT_SHIN:
		return false;
		break;

	default:
		return true;
		break;
	}
}

bool CNPC_CombineGuard::AllArmorDestroyed(void)
{
	for (int i = CGUARD_BGROUP_RIGHT_SHOULDER; i <= CGUARD_BGROUP_LEFT_SHIN; i++)
	{
		if (!m_armorPieces[i].destroyed)
		{
			return false;
		}
	}

	return true;
}


//=========================================================
// DeathSound 
//=========================================================
void CNPC_CombineGuard::DeathSound(void)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	//("NPC_CombineGuard.RequestBackup");
	EmitNPCSound("NPC_CombineGuard.RequestBackup", this, entindex());
}

//=========================================================
// PainSound
//=========================================================
//void CNPC_Combine::PainSound ( void )
void CNPC_CombineGuard::PainSound(const CTakeDamageInfo &info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	if (gpGlobals->curtime > m_flNextPainSound)
	{
		//EmitSound("NPC_CombineGuard.Pain1");

		EmitNPCSound("NPC_CombineGuard.Pain1", this, entindex());
		m_flNextPainSound = gpGlobals->curtime + 1;
	}
}

//=========================================================
// IdleSound 
//=========================================================
void CNPC_CombineGuard::IdleSound(void)
{

	if (random->RandomInt(0, 1))
	{
		// ask question or make statement
		switch (random->RandomInt(0, 2))
		{
		case 0: // check in
			//EmitNPCSound("", this, entindex());
			break;

		case 1: // question
			//EmitNPCSound("", this, entindex());
			break;

		case 2: // statement
			//EmitNPCSound("", this, entindex());
			break;
		}
	}
	else
	{
		//EmitNPCSound("", this, entindex());
	}

}

void CNPC_CombineGuard::LostEnemySound(void)
{
	if (gpGlobals->curtime <= m_flNextLostSound)
		return;

	if (!(CBaseEntity*)GetEnemy() || gpGlobals->curtime - GetEnemyLastTimeSeen() > 10)
	{
		//pSentence = "COMBINE_LOST_LONG";
		//EmitNPCSound("", this, entindex());
	}
	else
	{
		//pSentence = "COMBINE_LOST_SHORT";
		//EmitNPCSound("", this, entindex());
	}

	m_flNextLostSound = gpGlobals->curtime + random->RandomFloat(5.0, 15.0);

}

void CNPC_CombineGuard::FoundEnemySound(void)
{
	EmitNPCSound("", this, entindex());
}

void CNPC_CombineGuard::AnnounceAssault(void)
{
	if (random->RandomInt(0, 5) > 1)
		return;

	// If enemy can see me make assualt sound
	CBaseCombatCharacter* pBCC = GetEnemyCombatCharacterPointer();

	if (!pBCC)
		return;

	if (!FOkToMakeSound())
		return;

	// Make sure we are pretty close
	if (WorldSpaceCenter().DistToSqr(pBCC->WorldSpaceCenter()) > (2000 * 2000))
		return;

	// Make sure we are in view cone of player
	if (!pBCC->FInViewCone(this))
		return;

	// Make sure player can see me
	if (FVisible(pBCC))
	{
		//EmitSound("NPC_CombineGuard.SurrenderNow");
		EmitNPCSound("NPC_CombineGuard.SurrenderNow", this, entindex());
	}
}

/*AI_BEGIN_CUSTOM_NPC(npc_combineguard, CNPC_CombineGuard)

DECLARE_TASK(TASK_CGUARD_RANGE_ATTACK1)
DECLARE_TASK(TASK_COMBINEGUARD_SET_BALANCE)

DECLARE_CONDITION(COND_COMBINEGUARD_CLOBBERED)

DECLARE_ACTIVITY(ACT_CGUARD_IDLE_TO_ANGRY)
DECLARE_ACTIVITY(ACT_COMBINEGUARD_CLOBBERED)
DECLARE_ACTIVITY(ACT_COMBINEGUARD_TOPPLE)
DECLARE_ACTIVITY(ACT_COMBINEGUARD_GETUP)
DECLARE_ACTIVITY(ACT_COMBINEGUARD_HELPLESS)

DEFINE_SCHEDULE
(
SCHED_CGUARD_RANGE_ATTACK1,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_ENEMY				0"
"		TASK_ANNOUNCE_ATTACK		1"
"		TASK_CGUARD_RANGE_ATTACK1	0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_NO_PRIMARY_AMMO"
)

DEFINE_SCHEDULE
(
SCHED_COMBINEGUARD_CLOBBERED,

"	Tasks"
"		TASK_STOP_MOVING						0"
"		TASK_COMBINEGUARD_SET_BALANCE			1"
"		TASK_PLAY_SEQUENCE						ACTIVITY:ACT_COMBINEGUARD_CLOBBERED"
"		TASK_COMBINEGUARD_SET_BALANCE			0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_COMBINEGUARD_TOPPLE,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_COMBINEGUARD_TOPPLE"
"		TASK_WAIT						1"
"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_COMBINEGUARD_GETUP"
"		TASK_COMBINEGUARD_SET_BALANCE	0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_COMBINEGUARD_HELPLESS,

"	Tasks"
"	TASK_STOP_MOVING				0"
"	TASK_PLAY_SEQUENCE				ACTIVITY:ACT_COMBINEGUARD_TOPPLE"
"	TASK_WAIT						2"
"	TASK_PLAY_SEQUENCE				ACTIVITY:ACT_COMBINEGUARD_HELPLESS"
"	TASK_WAIT_INDEFINITE			0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_COMBINE_PATROL,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CGUARD_RANGE_ATTACK1"
"		TASK_WANDER						900540"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_REASONABLE			0"
"		TASK_WAIT						3"
"		TASK_WAIT_RANDOM				3"
"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
"		TASK_RUN_PATH					0"
"		TASK_FACE_ENEMY					0"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_COMBINE_PATROL" // keep doing it
""
"	Interrupts"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_MOVE_AWAY"
"		COND_NEW_ENEMY"
"		COND_SEE_ENEMY"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
)
AI_END_CUSTOM_NPC()*/
