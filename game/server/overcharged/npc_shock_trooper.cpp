//========= Overcharged 2021. ============//
//
// Purpose: Shock trooper NPC
//
//
// 9 september 2021 BJ grenade throw fix 
// 26 oct melee sounds fix, cleanup hl1 unused
//=============================================================================//

#include	"cbase.h"
#include	"beam_shared.h"
#include	"AI_Default.h"
#include	"AI_Task.h"
#include	"AI_Schedule.h"
#include	"AI_Node.h"
#include	"AI_Hull.h"
#include	"AI_Hint.h"
#include	"AI_Route.h"
#include	"AI_Squad.h"
#include	"AI_SquadSlot.h"
#include	"AI_Motor.h"
#include	"npc_shock_trooper.h"	//fx
#include	"soundent.h"
#include	"game.h"
#include	"NPCEvent.h"
#include	"EntityList.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"basecombatweapon.h"
#include	"ai_interactions.h"
#include	"scripted.h"
#include	"grenade_strooper_bounce.h"
#include	"grenade_strooper_instant.h"
#include	"movevars_shared.h"

ConVar	sk_shock_trooper_health("sk_shock_trooper_health", "100");
ConVar  sk_shock_trooper_kick("sk_shock_trooper_kick", "40");
ConVar  sk_shock_trooper_pellets("sk_shock_trooper_pellets", "1");
ConVar  sk_shock_trooper_gspeed("sk_shock_trooper_gspeed", "60");
ConVar  sk_shock_trooper_num_balls("sk_shock_trooper_num_balls", "6");

extern ConVar sk_plr_dmg_grenade;

#define SF_GRUNT_LEADER	( 1 << 5  )

int g_fGruntQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.
int g_iSquadIndex = 0;

#define shock_trooper_GUN_SPREAD 0.08716f

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	GRUNT_CLIP_SIZE					36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GRUNT_VOL						0.35		// volume of grunt sounds
#define GRUNT_SNDLVL					SNDLVL_NORM	// soundlevel of grunt sentences
#define shock_trooper_LIMP_HEALTH				20
#define shock_trooper_DMG_HEADSHOT				( DMG_BULLET | DMG_CLUB )	// damage types that can kill a grunt with a single headshot.
#define shock_trooper_NUM_HEADS				2 // how many grunt heads are there? 
#define shock_trooper_MINIMUM_HEADSHOT_DAMAGE	15 // must do at least this much damage in one shot to head to score a headshot kill
#define	shock_trooper_SENTENCE_VOLUME			(float)0.35 // volume of grunt sentences

#define shock_trooper_9MMAR				( 1 << 0)
#define shock_trooper_HANDGRENADE			( 1 << 1)
#define shock_trooper_GRENADELAUNCHER		( 1 << 2)
#define shock_trooper_SHOTGUN				( 1 << 3)

#define HEAD_GROUP					1
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		shock_trooper_AE_RELOAD		( 2 )
#define		shock_trooper_AE_KICK			( 3 )
#define		shock_trooper_AE_BURST1		( 4 )
#define		shock_trooper_AE_BURST2		( 5 ) 
#define		shock_trooper_AE_BURST3		( 6 ) 
#define		shock_trooper_AE_GREN_TOSS		( 7 )
#define		shock_trooper_AE_GREN_LAUNCH	( 8 )
#define		shock_trooper_AE_GREN_DROP		( 9 )
#define		shock_trooper_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		shock_trooper_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define BOLT_MODEL_NPC	cvar->FindVar("oc_weapon_ShockRifle_model")->GetString() //"models/ShockRifle.mdl"
#define BOLT_SKIN_GLOW_NPC		1
#define BOLT_AIR_VELOCITY_NPC	1500
#ifndef CLIENT_DLL

void TE_StickyBolt(IRecipientFilter& filter, float delay, Vector vecDirection, const Vector *origin);

//-----------------------------------------------------------------------------
// ShockRifle
//-----------------------------------------------------------------------------
class CShockRifleProjectileNPC : public CBaseCombatCharacter
{
	DECLARE_CLASS(CShockRifleProjectileNPC, CBaseCombatCharacter);

public:
	CShockRifleProjectileNPC() { };
	~CShockRifleProjectileNPC();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);	// changed
	void BoltTouch(CBaseEntity *pOther);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CShockRifleProjectileNPC *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseActor *pentOwner = NULL);

protected:

	bool	CreateSprites(void);

	CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS(ShockRifle_projectile_npc, CShockRifleProjectileNPC);

BEGIN_DATADESC(CShockRifleProjectileNPC)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_pGlowSprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CShockRifleProjectileNPC, DT_ShockRifleProjectileNPC)
END_SEND_TABLE()

CShockRifleProjectileNPC *CShockRifleProjectileNPC::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseActor *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CShockRifleProjectileNPC *pShockRifleProjectileNPC = (CShockRifleProjectileNPC *)CreateEntityByName("ShockRifle_projectile_npc");
	UTIL_SetOrigin(pShockRifleProjectileNPC, vecOrigin);
	pShockRifleProjectileNPC->SetAbsAngles(angAngles);
	pShockRifleProjectileNPC->Spawn();
	pShockRifleProjectileNPC->SetOwnerEntity(pentOwner);

	pShockRifleProjectileNPC->m_iDamage = iDamage;

	return pShockRifleProjectileNPC;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CShockRifleProjectileNPC::~CShockRifleProjectileNPC(void)
{
	if (m_pGlowSprite)
	{
		UTIL_Remove(m_pGlowSprite);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CShockRifleProjectileNPC::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CShockRifleProjectileNPC::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CShockRifleProjectileNPC::CreateSprites(void)
{
	DispatchParticleEffect("hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	DispatchParticleEffect("Shockrifle_projectile", PATTACH_ABSORIGIN_FOLLOW, this);
	/*// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate( "sprites/light_glow02_noz.vmt", GetLocalOrigin(), false );

	if ( m_pGlowSprite != NULL )
	{
	m_pGlowSprite->FollowEntity( this );
	m_pGlowSprite->SetTransparency( kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation );
	m_pGlowSprite->SetScale( 0.2f );
	m_pGlowSprite->TurnOff();
	}*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShockRifleProjectileNPC::Spawn(void)
{
	Precache();

	SetModel(BOLT_MODEL_NPC);
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_BBOX);
	SetModelScale(0.1f, 0.0f);
	//AddEffects( EF_NODRAW );	// BriJee OVR : Little trick, nodraw model
	//AddEffects( EF_NOSHADOW );
	// Use our hitboxes to determine our render bounds
	CollisionProp()->SetSurroundingBoundsType(USE_HITBOXES);
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch(&CShockRifleProjectileNPC::BoltTouch);

	SetThink(&CShockRifleProjectileNPC::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	CreateSprites();

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW_NPC;
}


void CShockRifleProjectileNPC::Precache(void)
{
	PrecacheModel(BOLT_MODEL_NPC);
	PrecacheParticleSystem("Shockrifle_sparks");
	//PrecacheModel( "sprites/light_glow02_noz.vmt" );

	//PrecacheParticleSystem( "hunter_flechette_trail_striderbuster" );
	PrecacheParticleSystem("hunter_flechette_trail");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CShockRifleProjectileNPC::BoltTouch(CBaseEntity *pOther)
{
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

		if (FClassnameIs(pOther, "npc_shocktrooper") || FClassnameIs(pOther, "npc_shockroach") || FClassnameIs(pOther, "npc_pitdrone") || 
			FClassnameIs(pOther, "npc_voltigore") || FClassnameIs(pOther, "npc_voltigore_baby"))
		{
			DevMsg("Shocktrooper Friendly Fire. Skip damage. \n");
		}
		else if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), cvar->FindVar("sk_npc_dmg_ShockRifle")->GetFloat(), DMG_SHOCK);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		}
		else
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), cvar->FindVar("sk_npc_dmg_ShockRifle")->GetFloat(), DMG_NEVERGIB | DMG_SHOCK);
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		//EmitSound( "Weapon_Crossbow.BoltHitBody" );
		CPASAttenuationFilter filter(this);
		EmitSound_t params;
		params.m_pSoundName = "Weapon_Shock.Impact";
		params.m_flSoundTime = 0.f;
		params.m_pOrigin = &GetAbsOrigin();
		float duration = 0.f;
		params.m_pflSoundDuration = &duration;
		params.m_bWarnOnDirectWaveReference = true;

		EmitSound(filter, this->entindex(), params);
		//EmitSound("Weapon_Shock.Impact");

		CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);


		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}
	else
	{
		trace_t	tr;

		Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);


		CPASAttenuationFilter filter(this);
		EmitSound_t params;
		params.m_pSoundName = "Weapon_Shock.Impact";
		params.m_flSoundTime = 0.f;
		params.m_pOrigin = &GetAbsOrigin();
		float duration = 0.f;
		params.m_pflSoundDuration = &duration;
		params.m_bWarnOnDirectWaveReference = true;

		EmitSound(filter, this->entindex(), params);
		//EmitSound("Weapon_Shock.Impact");

		CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);

		UTIL_DecalTrace(&tr, "SmallScorch"); //"Scorch" );


		UTIL_Remove(this);
	}


}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShockRifleProjectileNPC::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.02f);

	if (GetWaterLevel() || GetAbsVelocity().Length() <= 0)
	{
		DispatchParticleEffect("Shockrifle_sparks", PATTACH_ABSORIGIN, NULL);

		UTIL_Remove(this);	// BriJee OVR : Little trick to get entity killed in water
	}
	//UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

#endif

const char *CNPC_shock_trooper::pGruntSentences[] =
{
	"ST_GREN", // grenade scared grunt
	"ST_ALERT", // sees player
	"ST_MONSTER", // sees monster
	"ST_COVER", // running to cover
	"ST_THROW", // about to throw grenade
	"ST_CHARGE",  // running out to get the enemy
	"ST_TAUNT", // say rude things
};

enum
{
	shock_trooper_SENT_NONE = -1,
	shock_trooper_SENT_GREN = 0,
	shock_trooper_SENT_ALERT,
	shock_trooper_SENT_MONSTER,
	shock_trooper_SENT_COVER,
	shock_trooper_SENT_THROW,
	shock_trooper_SENT_CHARGE,
	shock_trooper_SENT_TAUNT,
} shock_trooper_SENTENCE_TYPES;

LINK_ENTITY_TO_CLASS(npc_shocktrooper, CNPC_shock_trooper);		// shocktrooper слитно как в оригинале, остальное можно раздельно

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_FAIL = LAST_SHARED_SCHEDULE,
	SCHED_GRUNT_COMBAT_FAIL,
	SCHED_GRUNT_VICTORY_DANCE,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY,
	SCHED_GRUNT_FOUND_ENEMY,
	SCHED_GRUNT_COMBAT_FACE,
	SCHED_GRUNT_SIGNAL_SUPPRESS,
	SCHED_GRUNT_SUPPRESS,
	SCHED_GRUNT_WAIT_IN_COVER,
	SCHED_GRUNT_TAKE_COVER,
	SCHED_GRUNT_GRENADE_COVER,
	SCHED_GRUNT_TOSS_GRENADE_COVER,
	SCHED_GRUNT_HIDE_RELOAD,
	SCHED_GRUNT_SWEEP,
	SCHED_GRUNT_RANGE_ATTACK1A,
	SCHED_GRUNT_RANGE_ATTACK1B,
	SCHED_GRUNT_RANGE_ATTACK2,
	SCHED_GRUNT_REPEL,
	SCHED_GRUNT_REPEL_ATTACK,
	SCHED_GRUNT_REPEL_LAND,
	SCHED_GRUNT_TAKE_COVER_FAILED,
	SCHED_GRUNT_RELOAD,
	SCHED_GRUNT_TAKE_COVER_FROM_ENEMY,
	SCHED_GRUNT_BARNACLE_HIT,
	SCHED_GRUNT_BARNACLE_PULL,
	SCHED_GRUNT_BARNACLE_CHOMP,
	SCHED_GRUNT_BARNACLE_CHEW,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_SHARED_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
};


//=========================================================
// monster-specific conditions
//=========================================================
enum
{
	COND_GRUNT_NOFIRE = LAST_SHARED_CONDITION + 1,
};

// -----------------------------------------------
//	> Squad slots
// -----------------------------------------------
enum SquadSlot_T
{
	SQUAD_SLOT_GRENADE1 = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOT_GRENADE2,
	SQUAD_SLOT_ENGAGE1,
	SQUAD_SLOT_ENGAGE2,
};


int ACT_GRUNT_LAUNCH_GRENADE;
int ACT_GRUNT_TOSS_GRENADE;
int ACT_GRUNT_MP5_STANDING;
int ACT_GRUNT_MP5_CROUCHING;
int ACT_GRUNT_SHOTGUN_STANDING;
int ACT_GRUNT_SHOTGUN_CROUCHING;

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_shock_trooper)
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),
DEFINE_FIELD(m_flNextPainTime, FIELD_TIME),
DEFINE_FIELD(m_flCheckAttackTime, FIELD_FLOAT),
DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_iLastGrenadeCondition, FIELD_INTEGER),
DEFINE_FIELD(m_fStanding, FIELD_BOOLEAN),
DEFINE_FIELD(m_fFirstEncounter, FIELD_BOOLEAN),
DEFINE_FIELD(m_iClipSize, FIELD_INTEGER),
DEFINE_FIELD(m_voicePitch, FIELD_INTEGER),
DEFINE_FIELD(m_iSentence, FIELD_INTEGER),
DEFINE_KEYFIELD(m_iWeapons, FIELD_INTEGER, "weapons"),
DEFINE_KEYFIELD(m_SquadName, FIELD_STRING, "netname"),

DEFINE_FIELD(m_bInBarnacleMouth, FIELD_BOOLEAN),

DEFINE_FIELD(m_flLastEnemySightTime, FIELD_TIME),
DEFINE_FIELD(m_flTalkWaitTime, FIELD_TIME),
//DEFINE_FIELD( m_iAmmoType, FIELD_INTEGER ),

END_DATADESC()

CNPC_shock_trooper::CNPC_shock_trooper()
{
	m_bIsBodygrouped = true;
	m_iBodyGroup = FindBodygroupByName("shockroach");
	m_iBodyGroupValue = 1;
}
//=========================================================
// Spawn
//=========================================================
void CNPC_shock_trooper::Spawn()
{
	Precache();

	SetModel("models/RaceX/shock_trooper.mdl");

	SetHullType(HULL_MEDIUM_TALL);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_YELLOW;
	ClearEffects();
	m_iHealth = sk_shock_trooper_health.GetFloat();
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->curtime + 1;
	m_flNextPainTime = gpGlobals->curtime;
	m_iSentence = shock_trooper_SENT_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_MOVE_GROUND);

	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);

	// Innate range attack for grenade
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK2);
	// Innate range attack for kicking
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	m_fFirstEncounter = true;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	if (m_iWeapons == 0)
	{
		// initialize to original values
		m_iWeapons = shock_trooper_9MMAR | shock_trooper_HANDGRENADE;
		// pev->weapons = shock_trooper_SHOTGUN;
		// pev->weapons = shock_trooper_9MMAR | shock_trooper_GRENADELAUNCHER;
	}

	if (FBitSet(m_iWeapons, shock_trooper_SHOTGUN))
	{
		SetBodygroup(GUN_GROUP, GUN_SHOTGUN);
		m_iClipSize = 8;
	}
	else
	{
		m_iClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_iClipSize;

	if (random->RandomInt(0, 99) < 80)
		m_nSkin = 0;	// light skin
	else
		m_nSkin = 1;	// dark skin

	if (FBitSet(m_iWeapons, shock_trooper_SHOTGUN))
	{
		SetBodygroup(HEAD_GROUP, HEAD_SHOTGUN);
	}
	else if (FBitSet(m_iWeapons, shock_trooper_GRENADELAUNCHER))
	{
		SetBodygroup(HEAD_GROUP, HEAD_M203);
		m_nSkin = 1; // alway dark skin
	}

	m_flTalkWaitTime = 0;

	//HACK
	g_iSquadIndex = 0;

	//DispatchParticleEffect("shock_trooper_eye_glow", PATTACH_POINT_FOLLOW, this, "2", false);

	BaseClass::Spawn();

	NPCInit();
}

int CNPC_shock_trooper::IRelationPriority(CBaseEntity *pTarget)
{
	//I hate alien grunts more than anything.
	if (pTarget->Classify() == CLASS_ALIENGRUNT)
	{
		if (FClassnameIs(pTarget, "npc_aliengrunt"))
		{
			return (BaseClass::IRelationPriority(pTarget) + 1);
		}
	}

	return BaseClass::IRelationPriority(pTarget);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_shock_trooper::Precache()
{
	m_iAmmoType = GetAmmoDef()->Index("9mmRound");
	UTIL_PrecacheOther("ShockRifle_projectile_npc");
	PrecacheModel("models/RaceX/shock_trooper.mdl");
	PrecacheParticleSystem("weapon_muzzle_flash_shock_npc");
	//PrecacheParticleSystem("shock_trooper_eye_glow");

	// get voice pitch
	if (random->RandomInt(0, 1))
		m_voicePitch = 109 + random->RandomInt(0, 7);
	else
		m_voicePitch = 100;

	PrecacheScriptSound("Zombie.AttackHit");	// melee
	PrecacheScriptSound("Zombie.AttackMiss");

	PrecacheScriptSound("NPC_shock_trooper.Reload");
	PrecacheScriptSound("NPC_shock_trooper.GrenadeLaunch");
	PrecacheScriptSound("NPC_shock_trooper.ShockFire");	// 9MM
	PrecacheScriptSound("NPC_shock_trooper.Pain");
	PrecacheScriptSound("NPC_shock_trooper.Die");

	PrecacheScriptSound("NPC_shock_trooper.ST_TAUNT");
	PrecacheScriptSound("NPC_shock_trooper.ST_CHECK");
	PrecacheScriptSound("NPC_shock_trooper.ST_QUEST");
	PrecacheScriptSound("NPC_shock_trooper.ST_IDLE");
	PrecacheScriptSound("NPC_shock_trooper.ST_GREN");
	PrecacheScriptSound("NPC_shock_trooper.ST_CLEAR");
	PrecacheScriptSound("NPC_shock_trooper.ST_MONSTER");
	PrecacheScriptSound("NPC_shock_trooper.ST_ANSWER");
	PrecacheScriptSound("NPC_shock_trooper.ST_ALERT");
	PrecacheScriptSound("NPC_shock_trooper.ST_THROW");

	PrecacheScriptSound("NPC_shock_trooper.ST_CHARGE");	// 2 non state
	PrecacheScriptSound("NPC_shock_trooper.ST_COVER");
	BaseClass::Precache();

	UTIL_PrecacheOther("grenade_bounce");
	UTIL_PrecacheOther("spore_launcher_bounced_projectile");
	//UTIL_PrecacheOther("grenade_instant");

}

//=========================================================
// someone else is talking - don't speak
//=========================================================
bool CNPC_shock_trooper::FOkToSpeak(void)
{
	// if someone else is talking, don't speak
	if (gpGlobals->curtime <= m_flTalkWaitTime)
		return FALSE;

	if (m_spawnflags & SF_NPC_GAG)
	{
		if (m_NPCState != NPC_STATE_COMBAT)
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	return TRUE;
}


//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after 
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has 
// started moving.
//=========================================================
void CNPC_shock_trooper::SpeakSentence(void)
{
	if (m_iSentence == shock_trooper_SENT_NONE)
	{
		// no sentence cued up.
		return;
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz(edict(), pGruntSentences[m_iSentence], shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
		JustSpoke();
	}
}

//=========================================================
//=========================================================
void CNPC_shock_trooper::JustSpoke(void)
{
	m_flTalkWaitTime = gpGlobals->curtime + random->RandomFloat(1.5f, 2.0f);
	m_iSentence = shock_trooper_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CNPC_shock_trooper::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (m_pSquad && GetEnemy() != NULL)
	{
		if (m_pSquad->GetLeader() == NULL)
			return;

		CNPC_shock_trooper *pSquadLeader = (CNPC_shock_trooper*)m_pSquad->GetLeader()->MyNPCPointer();

		if (pSquadLeader == NULL)
			return; //Paranoid, so making sure it's ok.		

		if (HasCondition(COND_SEE_ENEMY))
		{
			// update the squad's last enemy sighting time.
			pSquadLeader->m_flLastEnemySightTime = gpGlobals->curtime;
		}
		else
		{
			if (gpGlobals->curtime - pSquadLeader->m_flLastEnemySightTime > 5)
			{
				// been a while since we've seen the enemy
				pSquadLeader->GetEnemies()->MarkAsEluded(GetEnemy());
			}
		}
	}
}

Class_T	CNPC_shock_trooper::Classify(void)
{
	return CLASS_RACEX; //CLASS_HUMAN_MILITARY;
}

//=========================================================
//
// SquadRecruit(), get some monsters of my classification and
// link them as a group.  returns the group size
//
//=========================================================
int CNPC_shock_trooper::SquadRecruit(int searchRadius, int maxMembers)
{
	int squadCount;
	int iMyClass = Classify();// cache this monster's class

	if (maxMembers < 2)
		return 0;

	// I am my own leader
	squadCount = 1;

	CBaseEntity *pEntity = NULL;

	if (m_SquadName != NULL_STRING)
	{
		// I have a netname, so unconditionally recruit everyone else with that name.
		pEntity = gEntList.FindEntityByClassname(pEntity, "npc_shock_trooper");

		while (pEntity)
		{
			CNPC_shock_trooper *pRecruit = (CNPC_shock_trooper*)pEntity->MyNPCPointer();

			if (pRecruit)
			{
				if (!pRecruit->m_pSquad && pRecruit->Classify() == iMyClass && pRecruit != this)
				{
					// minimum protection here against user error.in worldcraft. 
					if (pRecruit->m_SquadName != NULL_STRING && FStrEq(STRING(m_SquadName), STRING(pRecruit->m_SquadName)))
					{
						pRecruit->InitSquad();
						squadCount++;
					}
				}
			}

			pEntity = gEntList.FindEntityByClassname(pEntity, "npc_shock_trooper");
		}

		return squadCount;
	}
	else
	{
		char szSquadName[64];
		Q_snprintf(szSquadName, sizeof(szSquadName), "squad%d\n", g_iSquadIndex);

		m_SquadName = MAKE_STRING(szSquadName);

		while ((pEntity = gEntList.FindEntityInSphere(pEntity, GetAbsOrigin(), searchRadius)) != NULL)
		{
			if (!FClassnameIs(pEntity, "npc_shock_trooper"))
				continue;

			CNPC_shock_trooper *pRecruit = (CNPC_shock_trooper*)pEntity->MyNPCPointer();

			if (pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_hCine)
			{
				// Can we recruit this guy?
				if (!pRecruit->m_pSquad && pRecruit->Classify() == iMyClass &&
					(((iMyClass != CLASS_ALIENGRUNT) ||
					(iMyClass != CLASS_ALIENCONTROLLER) ||
					(iMyClass != CLASS_HEADCRAB) ||
					(iMyClass != CLASS_HOUNDEYE) ||
					(iMyClass != CLASS_ANTLION) ||
					(iMyClass != CLASS_VORTIGAUNT) ||
					(iMyClass != CLASS_ZOMBIE)) || FClassnameIs(this, pRecruit->GetClassname())) &&
					!pRecruit->m_SquadName)
				{
					trace_t tr;
					UTIL_TraceLine(GetAbsOrigin() + GetViewOffset(), pRecruit->GetAbsOrigin() + GetViewOffset(), MASK_NPCSOLID_BRUSHONLY, pRecruit, COLLISION_GROUP_NONE, &tr);// try to hit recruit with a traceline.

					if (tr.fraction == 1.0)
					{
						//We're ready to recruit people, so start a squad if I don't have one.
						if (!m_pSquad)
						{
							InitSquad();
						}

						pRecruit->m_SquadName = m_SquadName;

						pRecruit->CapabilitiesAdd(bits_CAP_SQUAD);
						pRecruit->InitSquad();

						squadCount++;
					}
				}
			}
		}

		if (squadCount > 1)
		{
			g_iSquadIndex++;
		}
	}

	return squadCount;
}

void CNPC_shock_trooper::StartNPC(void)
{
	if (!m_pSquad)
	{
		if (m_SquadName != NULL_STRING)
		{
			// if I have a groupname, I can only recruit if I'm flagged as leader
			if (GetSpawnFlags() & SF_GRUNT_LEADER)
			{
				InitSquad();

				// try to form squads now.
				//int iSquadSize = SquadRecruit(1024, 4);

				/*if (iSquadSize)
				{
					Msg("Squad of %d %s formed\n", iSquadSize, GetClassname());
				}*/
			}
			else
			{

				//Hacky.
				//Revisit me later.
				const char *pSquadName = STRING(m_SquadName);

				m_SquadName = NULL_STRING;

				BaseClass::StartNPC();

				m_SquadName = MAKE_STRING(pSquadName);

				return;
			}
		}
		else
		{
			//int iSquadSize = SquadRecruit(1024, 4);

			/*if (iSquadSize)
			{
				Msg("Squad of %d %s formed\n", iSquadSize, GetClassname());
			}*/
		}
	}

	BaseClass::StartNPC();

	if (m_pSquad && m_pSquad->IsLeader(this))
	{
		SetBodygroup(1, 1); // UNDONE: truly ugly hack
		m_nSkin = 0;
	}
	/*SetBodygroup(GUN_GROUP, 0);//GUN_NONE
	SetBodygroup(0, 0);//GUN_NONE
	SetBodygroup(3, 0);//GUN_NONE*/
}

void CNPC_shock_trooper::RunAI(void)
{
	//GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
	//GetNavigator()->SetGoal(corpseCoord);

	SetNextThink(gpGlobals->curtime + 0.01);

	BaseClass::RunAI();
}
//=========================================================
// CheckMeleeAttack1
//=========================================================
int CNPC_shock_trooper::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > 64)
		return COND_TOO_FAR_TO_ATTACK;
	else if (flDot < 0.7)
		return COND_NOT_FACING_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}

bool CNPC_shock_trooper::NoFriendlyFire(CBaseEntity *pTarget)
{
	/*trace_t tr;
	Vector vecSrc = GetAbsOrigin();
	Vector vecDir;
	Vector vecShootDir = GetShootEnemyDir(vecDir);
	Vector vecStop = vecSrc + vecShootDir * MAX_TRACE_LENGTH;
	UTIL_TraceLine(vecSrc, vecStop, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);*/

	/*DevMsg("DistTo: %i \n", GetAbsOrigin().DistTo(pTarget->WorldSpaceCenter()));
	if (GetAbsOrigin().DistTo(pTarget->WorldSpaceCenter()) > 500)
	return false;*/


	if (pTarget && pTarget->Classify() == CLASS_RACEX)
		return false;
	else
		return true;

	return true;
}
//=========================================================
// CheckRangeAttack1 - overridden for shock_trooper, cause 
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the shock_trooper can attack when the enemy is
// occluded (throw grenade over wall, etc). We must 
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
int CNPC_shock_trooper::RangeAttack1Conditions(float flDot, float flDist)
{
	if (!HasCondition(COND_ENEMY_OCCLUDED) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire(GetEnemy()))
	{
		trace_t	tr;

		if (!GetEnemy()->IsPlayer() && flDist <= 64)
		{
			// kick nonclients, but don't shoot at them.
			return COND_NONE;
		}

		Vector vecSrc;
		QAngle angAngles;

		GetAttachment("0", vecSrc, angAngles);

		//NDebugOverlay::Line( GetAbsOrigin() + GetViewOffset(), GetEnemy()->BodyTarget(GetAbsOrigin() + GetViewOffset()), 255, 0, 0, false, 0.1 );
		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine(GetAbsOrigin() + GetViewOffset(), GetEnemy()->BodyTarget(GetAbsOrigin() + GetViewOffset()), MASK_SHOT, this/*pentIgnore*/, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction == 1.0 || tr.m_pEnt == GetEnemy())
		{
			//NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 255, 0, false, 1.0 );

			return COND_CAN_RANGE_ATTACK1;
		}

		//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 1.0 );
	}


	if (!NoFriendlyFire(GetEnemy()))
		return COND_WEAPON_BLOCKED_BY_FRIEND; //err =|

	return COND_NONE;
}

int CNPC_shock_trooper::RangeAttack2Conditions(float flDot, float flDist)
{
	m_iLastGrenadeCondition = GetGrenadeConditions(flDot, flDist);
	return m_iLastGrenadeCondition;
}

int CNPC_shock_trooper::GetGrenadeConditions(float flDot, float flDist)
{
	if (!FBitSet(m_iWeapons, (shock_trooper_HANDGRENADE | shock_trooper_GRENADELAUNCHER)))
		return COND_NONE;

	// assume things haven't changed too much since last time
	if (gpGlobals->curtime < m_flNextGrenadeCheck)
		return m_iLastGrenadeCondition;

	if (m_flGroundSpeed != 0)
		return COND_NONE;

	CBaseEntity *pEnemy = GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector flEnemyLKP = GetEnemyLKP();
	if (!(pEnemy->GetFlags() & FL_ONGROUND) && pEnemy->GetWaterLevel() == 0 && flEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z))
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}

	Vector vecTarget;

	if (FBitSet(m_iWeapons, shock_trooper_HANDGRENADE))
	{
		// find feet
		if (random->RandomInt(0, 1))
		{
			// magically know where they are
			pEnemy->CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &vecTarget);
		}
		else
		{
			// toss it to where you last saw them
			vecTarget = flEnemyLKP;
		}
	}
	else
	{
		// find target
		// vecTarget = GetEnemy()->BodyTarget( GetAbsOrigin() );
		vecTarget = GetEnemy()->GetAbsOrigin() + (GetEnemy()->BodyTarget(GetAbsOrigin()) - GetEnemy()->GetAbsOrigin());
		// estimate position
		if (HasCondition(COND_SEE_ENEMY))
		{
			vecTarget = vecTarget + ((vecTarget - GetAbsOrigin()).Length() / sk_shock_trooper_gspeed.GetFloat()) * GetEnemy()->GetAbsVelocity();
		}
	}

	// are any of my squad members near the intended grenade impact area?
	if (m_pSquad)
	{
		if (m_pSquad->SquadMemberInRange(vecTarget, 256))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return COND_NONE;
		}
	}

	if ((vecTarget - GetAbsOrigin()).Length2D() <= 256)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_NONE;
	}


	if (FBitSet(m_iWeapons, shock_trooper_HANDGRENADE))
	{
		Vector vGunPos;
		QAngle angGunAngles;
		GetAttachment("0", vGunPos, angGunAngles);


		Vector vecToss = VecCheckToss(this, vGunPos, vecTarget, -1, 0.5, false);

		if (vecToss != vec3_origin)
		{
			m_vecTossVelocity = vecToss;

			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.

			return COND_CAN_RANGE_ATTACK2;
		}
		else
		{
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.

			return COND_NONE;
		}
	}
	else
	{
		Vector vGunPos;
		QAngle angGunAngles;
		GetAttachment("0", vGunPos, angGunAngles);

		Vector vecToss = VecCheckThrow(this, vGunPos, vecTarget, sk_shock_trooper_gspeed.GetFloat(), 0.5);

		if (vecToss != vec3_origin)
		{
			m_vecTossVelocity = vecToss;

			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.

			return COND_CAN_RANGE_ATTACK2;
		}
		else
		{
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.

			return COND_NONE;
		}
	}
}


//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================
bool CNPC_shock_trooper::FCanCheckAttacks(void)
{
	// This condition set when too close to a grenade to blow it up
	if (!HasCondition(COND_TOO_CLOSE_TO_ATTACK))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CNPC_shock_trooper::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_PLAYER |
		SOUND_BULLET_IMPACT |
		SOUND_DANGER;
}


//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CNPC_shock_trooper::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	CTakeDamageInfo info = inputInfo;

	// check for helmet shot
	if (ptr->hitgroup == 11)
	{
		// make sure we're wearing one
		if (GetBodygroup(1) == HEAD_GRUNT && (info.GetDamageType() & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		{
			// absorb damage
			info.SetDamage(info.GetDamage() - 20);
			if (info.GetDamage() <= 0)
				info.SetDamage(0.01);
		}
		// it's head shot anyways
		ptr->hitgroup = HITGROUP_HEAD;
	}
	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}


//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CNPC_shock_trooper::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	Forget(bits_MEMORY_INCOVER);

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CNPC_shock_trooper::MaxYawSpeed(void)
{
	float flYS;

	switch (GetActivity())
	{
	case ACT_IDLE:
		flYS = 150;
		break;
	case ACT_RUN:
		flYS = 150;
		break;
	case ACT_WALK:
		flYS = 180;
		break;
	case ACT_RANGE_ATTACK1:
		flYS = 120;
		break;
	case ACT_RANGE_ATTACK2:
		flYS = 120;
		break;
	case ACT_MELEE_ATTACK1:
		flYS = 120;
		break;
	case ACT_MELEE_ATTACK2:
		flYS = 120;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		flYS = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		flYS = 30;
		break;
	default:
		flYS = 90;
		break;
	}

	// Yaw speed is handled differently now!
	return flYS * 0.5f;
}

void CNPC_shock_trooper::IdleSound(void)
{
	CPASAttenuationFilter filter(this);
	if (FOkToSpeak() && (g_fGruntQuestion || random->RandomInt(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (random->RandomInt(0, 2))
			{
				case 0:
				{// check in
					//SENTENCEG_PlayRndSz( edict(), "ST_CHECK", shock_trooper_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);

					EmitNpcSound("NPC_shock_trooper.ST_CHECK", entindex());

					//EmitSound(filter, entindex(), "NPC_shock_trooper.ST_CHECK");
					g_fGruntQuestion = 1;
					break;
				}
				case 1: // question
				{
					//SENTENCEG_PlayRndSz( edict(), "ST_QUEST", shock_trooper_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);

					EmitNpcSound("NPC_shock_trooper.ST_QUEST", entindex());

					//EmitSound(filter, entindex(), "NPC_shock_trooper.ST_QUEST");
					g_fGruntQuestion = 2;
					break;
				}
				case 2: // statement
				{
					//SENTENCEG_PlayRndSz( edict(), "ST_IDLE", shock_trooper_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);

					EmitNpcSound("NPC_shock_trooper.ST_IDLE", entindex());

					//EmitSound(filter, entindex(), "NPC_shock_trooper.ST_IDLE");
					break;
				}
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
				case 1: // check in
				{
					EmitNpcSound("NPC_shock_trooper.ST_CLEAR", entindex());
					//EmitSound(filter, entindex(), "NPC_shock_trooper.ST_CLEAR");
					//SENTENCEG_PlayRndSz( edict(), "ST_CLEAR", shock_trooper_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
					break;
				}
				case 2: // question 
				{
					EmitNpcSound("NPC_shock_trooper.ST_ANSWER", entindex());
					//EmitSound(filter, entindex(), "NPC_shock_trooper.ST_ANSWER");
					//SENTENCEG_PlayRndSz( edict(), "ST_ANSWER", shock_trooper_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
					break;
				}
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

bool CNPC_shock_trooper::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
	{
		// Force choosing of a new schedule
		ClearSchedule("Soldier being eaten by a barnacle");
		m_bInBarnacleMouth = true;
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimReleased)
	{
		SetState(NPC_STATE_IDLE);
		m_bInBarnacleMouth = false;
		SetAbsVelocity(vec3_origin);
		SetMoveType(MOVETYPE_STEP);
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimGrab)
	{
		if (GetFlags() & FL_ONGROUND)
		{
			SetGroundEntity(NULL);
		}

		//Maybe this will break something else.
		if (GetState() == NPC_STATE_SCRIPT)
		{
			m_hCine->CancelScript();
			ClearSchedule("Soldier grabbed by a barnacle");
		}

		SetState(NPC_STATE_PRONE);

		CTakeDamageInfo info;
		PainSound(info);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Combine needs to check ammo
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_shock_trooper::CheckAmmo(void)
{
	if (m_cAmmoLoaded <= 0)
		SetCondition(COND_NO_PRIMARY_AMMO);

}

//=========================================================
//=========================================================
CBaseEntity *CNPC_shock_trooper::Kick(void)
{
	trace_t tr;

	Vector forward;
	AngleVectors(GetAbsAngles(), &forward);
	Vector vecStart = GetAbsOrigin();
	vecStart.z += WorldAlignSize().z * 0.5;
	Vector vecEnd = vecStart + (forward * 70);

	UTIL_TraceHull(vecStart, vecEnd, Vector(-16, -16, -18), Vector(16, 16, 18), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	if (tr.m_pEnt)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		return pEntity;
	}

	return NULL;
}

Vector CNPC_shock_trooper::Weapon_ShootPosition(void)
{
	if (m_fStanding)
		return GetAbsOrigin() + Vector(0, 0, 60);
	else
		return GetAbsOrigin() + Vector(0, 0, 48);
}

void CNPC_shock_trooper::Event_Killed(const CTakeDamageInfo &info)
{
	Vector	vecGunPos;
	QAngle	vecGunAngles;

	GetAttachment("0", vecGunPos, vecGunAngles);

	//int bg = FindBodygroupByName("shockroach");
	//SetBodygroup(bg, 1);//GUN_NONE

	CBaseEntity *pShockNPC = NULL;
	pShockNPC = CBaseEntity::Create("npc_shockroach", vecGunPos, GetAbsAngles(), this); //0 60 0

	// If the gun would drop into a wall, spawn it at our origin
	/*if( UTIL_PointContents( vecGunPos ) & CONTENTS_SOLID )
	{
	vecGunPos = GetAbsOrigin();
	}*/

	// now spawn a gun.
	/*if (FBitSet( m_iWeapons, shock_trooper_SHOTGUN ))
	{
	DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
	}
	else
	{
	DropItem( "weapon_mp5", vecGunPos, vecGunAngles );
	}

	if (FBitSet( m_iWeapons, shock_trooper_GRENADELAUNCHER ))
	{
	DropItem( "ammo_ARgrenades", BodyTarget( GetAbsOrigin() ), vecGunAngles );
	}*/

	BaseClass::Event_Killed(info);
}

//====================================== BJ added
Vector VecCheckThrowToleranceNade(CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance)
{
	flSpeed = MAX(1.0f, flSpeed);

	float flGravity = GetCurrentGravity() / 2;	// BJ Soooo high, reduced by 2

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);


	trace_t tr;
	UTIL_TraceLine(vecSpot1, vecApex, MASK_SOLID, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		return vec3_origin;
	}

	UTIL_TraceLine(vecApex, vecSpot2, MASK_SOLID_BRUSHONLY, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		bool bFail = true;

		// Didn't make it all the way there, but check if we're within our tolerance range
		if (flTolerance > 0.0f)
		{
			float flNearness = (tr.endpos - vecSpot2).LengthSqr();
			if (flNearness < Square(flTolerance))
			{
				bFail = false;
			}
		}

		/*
		if (bFail)
		{
			DevMsg("Shocktrooper FAIL green gren throw \n");		// BJ: This prevent to throw even in wall vec3_origin is just forward throw

			return vec3_origin;
		}
		else
			DevMsg("Shocktrooper OK green gren throw \n");
		*/
	}

	return vecGrenadeVel;
}

// BJ added
bool CNPC_shock_trooper::GetNadeVector(const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut)
{

//#if HL2_EPISODIC
	// Try the most direct route
	Vector vecToss = VecCheckThrowToleranceNade(this, vecStartPos, vecTarget, /*sk_st_grenspeed_speed.GetFloat()*/ 600, (10.0f*12.0f));

	// If this failed then try a little faster (flattens the arc)
	if (vecToss == vec3_origin)
	{
		vecToss = VecCheckThrowToleranceNade(this, vecStartPos, vecTarget, /*sk_st_grenspeed_speed.GetFloat()*/ 600 * 1.5f, (10.0f*12.0f));
		if (vecToss == vec3_origin)
			return false;
	}

	// Save out the result
	if (vecOut)
	{
		*vecOut = vecToss;
	}

	return true;
//#else
	//return false;
//#endif
}
// end

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_shock_trooper::HandleAnimEvent(animevent_t *pEvent)
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	//DevMsg("pEvent->event: %i \n", pEvent->event);
	switch (pEvent->event)
	{
	case shock_trooper_AE_RELOAD:
	{
		/*CPASAttenuationFilter filter(this);		
		EmitSound(filter, entindex(), "NPC_shock_trooper.Reload");*/

		EmitNpcSound("NPC_shock_trooper.Reload", entindex());

		m_cAmmoLoaded = m_iClipSize;
		ClearCondition(COND_NO_PRIMARY_AMMO);
	}
	break;

	case shock_trooper_AE_GREN_TOSS:
	{
		/*
		Cgrenade_strooper_bounce *pGrenade = (Cgrenade_strooper_bounce*)Create("grenade_bounce", GetAbsOrigin() + Vector(0, 0, 60), vec3_angle);
		if (pGrenade)
		{
			//pGrenade->ShootTimed( this, m_vecTossVelocity, 3.5 );
		}
		*/

		if (GetEnemy())
		{

			Vector vSpitPos;
			GetAttachment("0", vSpitPos);	// hand

			Vector	vTarget;

			vSpitPos.y += 0.0;
			vSpitPos.x += 0.0;
			vSpitPos.z += 8.0;

			// If our enemy is looking at us and far enough away, lead him
			if (HasCondition(COND_ENEMY_FACING_ME) && UTIL_DistApprox(GetAbsOrigin(), GetEnemy()->GetAbsOrigin()) > (40 * 12))
			{
				UTIL_PredictedPosition(GetEnemy(), 0.5f, &vTarget);
				vTarget.z = GetEnemy()->GetAbsOrigin().z;
			}
			else
			{
				// Otherwise he can't see us and he won't be able to dodge
				vTarget = GetEnemy()->BodyTarget(vSpitPos, true);
			}

			vTarget[2] += random->RandomFloat(0.0f, 32.0f);

			// Try and spit at our target
			Vector	vecToss;
			if (GetNadeVector(vSpitPos, vTarget, &vecToss) == false)
			{
				//DevMsg("GetSpitVector( vSpitPos, vTarget, &vecToss ) == false\n");
				// Now try where they were
				if (GetNadeVector(vSpitPos, m_vSavePosition, &vecToss) == false)
				{
					//DevMsg("GetSpitVector( vSpitPos, m_vSavePosition, &vecToss ) == false\n");
					// Failing that, just shoot with the old velocity we calculated initially!
					vecToss = m_vecSaveNadeVelocity;
				}
			}

			// Find what our vertical theta is to estimate the time we'll impact the ground
			Vector vecToTarget = (vTarget - vSpitPos);
			VectorNormalize(vecToTarget);
			float flVelocity = VectorNormalize(vecToss);
			float flCosTheta = DotProduct(vecToTarget, vecToss);
			float flTime = (vSpitPos - vTarget).Length2D() / (flVelocity * flCosTheta);

			// Emit a sound where this is going to hit so that targets get a chance to act correctly
			CSoundEnt::InsertSound(SOUND_DANGER, vTarget, (15 * 12), flTime, this);

			CSporeLauncherBouncedProjectile *pGrenade = (CSporeLauncherBouncedProjectile*)CreateEntityByName("grenade_bounce");
			pGrenade->SetAbsOrigin(vSpitPos);
			pGrenade->SetAbsAngles(vec3_angle);
			DispatchSpawn(pGrenade);
			pGrenade->SetThrower(this);
			pGrenade->SetOwnerEntity(this);

			pGrenade->SetAbsVelocity(vecToss * flVelocity);


			// Tumble through the air
			pGrenade->SetLocalAngularVelocity(QAngle(random->RandomFloat(-300, -400), random->RandomFloat(-300, -400), random->RandomFloat(-300, -400)));

			DevMsg("Shocktrooper grenade_bounce throw. \n");
			//}
			//end


		}

		m_iLastGrenadeCondition = COND_NONE;
		m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
	}
	break;

	case shock_trooper_AE_GREN_LAUNCH:
	{
		/*CPASAttenuationFilter filter2(this);
		EmitSound(filter2, entindex(), "NPC_shock_trooper.GrenadeLaunch");*/

		EmitNpcSound("NPC_shock_trooper.GrenadeLaunch", entindex());


		// BJ ================== REWORK THROW LOGIC
		//Vector vecSrc;
		//QAngle angAngles;

		//GetAttachment("0", vecSrc, angAngles);

		/*Cgrenade_strooper_instant * m_pMyGrenade = (Cgrenade_strooper_instant*)Create("grenade_instant", vecSrc, angAngles, this);
		m_pMyGrenade->SetAbsVelocity( m_vecTossVelocity );
		m_pMyGrenade->SetLocalAngularVelocity( QAngle( random->RandomFloat( -100, -500 ), 0, 0 ) );
		m_pMyGrenade->SetMoveType(MOVETYPE_VPHYSICS);
		m_pMyGrenade->SetGravity(200);
		m_pMyGrenade->SetSolid(SOLID_VPHYSICS);
		m_pMyGrenade->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
		m_pMyGrenade->SetThrower( this );

		m_pMyGrenade->SetDamage(cvar->FindVar("sk_plr_dmg_instant_grenade")->GetFloat());*/

		if (GetEnemy())
		{
			/*
			CBaseEntity *pVial = NULL;
			pVial = CBaseEntity::Create("spore_launcher_bounced_projectile", vecSrc, angAngles, this); // Creates
			pVial->PrecacheModel("models/spitball_medium.mdl");
			pVial->SetModel("models/spitball_medium.mdl");
			//pVial->SetSolid(SOLID_VPHYSICS);
			//pVial->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
			pVial->SetMoveType(MOVETYPE_VPHYSICS);
			pVial->SetAbsVelocity(m_vecTossVelocity*Vector(1, 1, 0.2f) * 2);
			//pVial->SetLocalAngularVelocity(QAngle(random->RandomFloat(-200, -300), 0, 0));
			//pVial->SetLocalOrigin(vecSrc);
			//pVial->SetLocalAngles(-angAngles);
			//pVial->SetAbsVelocity((GetAbsOrigin() + GetEnemy()->GetAbsOrigin()) * 2);

			pVial->SetDamage(cvar->FindVar("sk_npc_dmg_spore_acid_bounce")->GetFloat());
			pVial->Spawn();
			//pVial->SUB_StartFadeOut(15, false);
			*/

			// Light Kill : New spit form.

			Vector vSpitPos;
			GetAttachment("0", vSpitPos);	// hand

			Vector	vTarget;

			vSpitPos.y += 0.0;
			vSpitPos.x += 0.0;
			vSpitPos.z += 8.0;

			// If our enemy is looking at us and far enough away, lead him
			if (HasCondition(COND_ENEMY_FACING_ME) && UTIL_DistApprox(GetAbsOrigin(), GetEnemy()->GetAbsOrigin()) > (40 * 12))
			{
				UTIL_PredictedPosition(GetEnemy(), 0.5f, &vTarget);
				vTarget.z = GetEnemy()->GetAbsOrigin().z;
			}
			else
			{
				// Otherwise he can't see us and he won't be able to dodge
				vTarget = GetEnemy()->BodyTarget(vSpitPos, true);
			}

			vTarget[2] += random->RandomFloat(0.0f, 32.0f);

			// Try and spit at our target
			Vector	vecToss;
			if (GetNadeVector(vSpitPos, vTarget, &vecToss) == false)
			{
				//DevMsg("GetSpitVector( vSpitPos, vTarget, &vecToss ) == false\n");
				// Now try where they were
				if (GetNadeVector(vSpitPos, m_vSavePosition, &vecToss) == false)
				{
					//DevMsg("GetSpitVector( vSpitPos, m_vSavePosition, &vecToss ) == false\n");
					// Failing that, just shoot with the old velocity we calculated initially!
					vecToss = m_vecSaveNadeVelocity;
				}
			}

			// Find what our vertical theta is to estimate the time we'll impact the ground
			Vector vecToTarget = (vTarget - vSpitPos);
			VectorNormalize(vecToTarget);
			float flVelocity = VectorNormalize(vecToss);
			float flCosTheta = DotProduct(vecToTarget, vecToss);
			float flTime = (vSpitPos - vTarget).Length2D() / (flVelocity * flCosTheta);

			// Emit a sound where this is going to hit so that targets get a chance to act correctly
			CSoundEnt::InsertSound(SOUND_DANGER, vTarget, (15 * 12), flTime, this);

			// Don't fire again until this volley would have hit the ground (with some lag behind it)
			//SetNextAttack(gpGlobals->curtime + flTime + random->RandomFloat(0.5f, 2.0f));					// BJ not on trooper, under func already checking

			//for (int i = 0; i < 6; i++)
			//{
				//CGrenadeSpit *pGrenade = (CGrenadeSpit*)CreateEntityByName("grenade_spit");
				//CGrenadeSpit *pGrenade = CBaseEntity::Create("spore_launcher_bounced_projectile", vecSrc, angAngles, this);
				CSporeLauncherBouncedProjectile *pGrenade = (CSporeLauncherBouncedProjectile*)CreateEntityByName("spore_launcher_bounced_projectile");
				pGrenade->SetAbsOrigin(vSpitPos);
				pGrenade->SetAbsAngles(vec3_angle);
				DispatchSpawn(pGrenade);
				pGrenade->SetThrower(this);
				pGrenade->SetOwnerEntity(this);

				
				//if (i == 0)
				//{
					//pGrenade->SetSpitSize(SPIT_LARGE);
					pGrenade->SetAbsVelocity(vecToss * flVelocity);
				//}
				//else
				//{
					//pGrenade->SetAbsVelocity((vecToss + RandomVector(-0.035f, 0.035f)) * flVelocity);
					//pGrenade->SetSpitSize(random->RandomInt(SPIT_SMALL, SPIT_MEDIUM));
				//}
				

				// Tumble through the air
				pGrenade->SetLocalAngularVelocity(QAngle(random->RandomFloat(-300, -400), random->RandomFloat(-300, -400), random->RandomFloat(-300, -400)));

				DevMsg("Shocktrooper spore_launcher_bounced_projectile throw. \n");
			//}
			//end


		}


		if (g_iSkillLevel == SKILL_HARD)
			m_flNextGrenadeCheck = gpGlobals->curtime + random->RandomFloat(2, 5);// wait a random amount of time before shooting again
		else
			m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

		m_iLastGrenadeCondition = COND_NONE;

		//Msg("Using grenade launcer to flush you out!\n");
	}
	break;

	case shock_trooper_AE_GREN_DROP:
	{
		
		Cgrenade_strooper_bounce *pGrenade = (Cgrenade_strooper_bounce*)Create("grenade_bounce", Weapon_ShootPosition(), vec3_angle);
		if (pGrenade)
		{
			DevMsg("Shocktrooper dropping grenade near. \n");
			//pGrenade->ShootTimed( this, m_vecTossVelocity, 3.5 );
		}
		

		m_iLastGrenadeCondition = COND_NONE;
	}
	break;

	case shock_trooper_AE_BURST1:
	{
		//if ( FBitSet( m_iWeapons, shock_trooper_9MMAR ) )
		{
			Shoot();

			/*CPASAttenuationFilter filter3(this);
			EmitSound(filter3, entindex(), "NPC_shock_trooper.ShockFire");*/

			EmitNpcSound("NPC_shock_trooper.ShockFire", entindex());
		}
		/*else
		{
		Shotgun( );

		CPASAttenuationFilter filter4( this );
		EmitSound( filter4, entindex(), "shock_trooper.Shotgun" );
		}*/

		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 384, 0.3);
	}
	break;

	case shock_trooper_AE_BURST2:
	case shock_trooper_AE_BURST3:
	{
		{
			//for (int i = 0; i < sk_shock_trooper_num_balls.GetInt(); i++)

			Shotgun();

			/*CPASAttenuationFilter filter2(this);
			EmitSound(filter2, entindex(), "NPC_shock_trooper.ShockFire");*/

			EmitNpcSound("NPC_shock_trooper.ShockFire", entindex());
		}
	}
	break;

	case shock_trooper_AE_KICK:
	{
		CBaseEntity *pHurt = Kick();

		if (pHurt)
		{
			EmitSound("Zombie.AttackHit");
			
			Vector forward, up;
			AngleVectors(GetAbsAngles(), &forward, NULL, &up);

			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
				pHurt->ViewPunch(QAngle(15, 0, 0));

			// Don't give velocity or damage to the world
			if (pHurt->entindex() > 0)
			{
				pHurt->ApplyAbsVelocityImpulse(forward * 100 + up * 50);

				CTakeDamageInfo info(this, this, sk_shock_trooper_kick.GetFloat(), DMG_CLUB);
				CalculateMeleeDamageForce(&info, forward, pHurt->GetAbsOrigin());
				pHurt->TakeDamage(info);
			}
		}
		else
		{
			EmitSound("Zombie.AttackMiss");
		}
	}
	break;

	case shock_trooper_AE_CAUGHT_ENEMY:
	{
		if (FOkToSpeak())
		{
			//SENTENCEG_PlayRndSz( edict(), "HG_ALERT", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
			EmitNpcSound("NPC_shock_trooper.ST_ALERT", entindex());

			/*CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "NPC_shock_trooper.ST_ALERT");*/
			JustSpoke();
		}

	}

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CNPC_shock_trooper::SetAim(const Vector &aimDir)
{
	QAngle angDir;
	VectorAngles(aimDir, angDir);

	float curPitch = GetPoseParameter("XR");
	float newPitch = curPitch + UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 60), curPitch);

	SetPoseParameter("XR", -newPitch);
}

//=========================================================
// Shoot
//=========================================================
void CNPC_shock_trooper::Shoot(void)
{
	if (GetEnemy() == NULL)
		return;

	Vector vecShootOrigin;// = Weapon_ShootPosition();
	GetAttachment("rifle", vecShootOrigin, NULL);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	Vector	vecShellVelocity = right * random->RandomFloat(40, 90) + up * random->RandomFloat(75, 200) + forward * random->RandomFloat(-40, 40);
	//EjectShell( vecShootOrigin - vecShootDir * 24, vecShellVelocity, GetAbsAngles().y, 0 );
	//FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, m_iAmmoType ); // shoot +-5 degrees

	Vector src;
	GetAttachment("rifle", src, NULL);

	QAngle angAiming;
	VectorAngles(vecShootDir, angAiming);

	CShockRifleProjectileNPC *pShockRifleProjectileNPC = CShockRifleProjectileNPC::BoltCreate(src, angAiming, 0, this);
	if (this->GetWaterLevel() == 3)
	{
		//pShockRifleProjectile->Remove();	//->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
		CTakeDamageInfo hitself(this, this, 210, DMG_SHOCK);
		TakeDamage(hitself);
	}
	else
	{
		pShockRifleProjectileNPC->SetAbsVelocity(vecShootDir * BOLT_AIR_VELOCITY_NPC);
	}
	DispatchParticleEffect("weapon_muzzle_flash_shock_npc", PATTACH_POINT_FOLLOW, this, "rifle", false);

	DoMuzzleFlash();

	m_cAmmoLoaded--;// take away a bullet!

	SetAim(vecShootDir);
}

//=========================================================
// Shoot
//=========================================================
void CNPC_shock_trooper::Shotgun(void)
{
	if (GetEnemy() == NULL)
		return;

	Vector vecShootOrigin;// = Weapon_ShootPosition();
	GetAttachment("rifle", vecShootOrigin, NULL);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	Vector	vecShellVelocity = right * random->RandomFloat(40, 90) + up * random->RandomFloat(75, 200) + forward * random->RandomFloat(-40, 40);
	//EjectShell( vecShootOrigin - vecShootDir * 24, vecShellVelocity, GetAbsAngles().y, 0 );
	//FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, m_iAmmoType ); // shoot +-5 degrees

	Vector src;
	GetAttachment("rifle", src, NULL);

	QAngle angAiming;
	VectorAngles(vecShootDir, angAiming);

	CShockRifleProjectileNPC *pShockRifleProjectileNPC = CShockRifleProjectileNPC::BoltCreate(src, angAiming, 0, this);
	if (this->GetWaterLevel() == 3)
	{
		//pShockRifleProjectile->Remove();	//->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
		CTakeDamageInfo hitself(this, this, 210, DMG_SHOCK);
		TakeDamage(hitself);
	}
	else
	{
		pShockRifleProjectileNPC->SetAbsVelocity(vecShootDir * BOLT_AIR_VELOCITY_NPC);
	}
	DispatchParticleEffect("weapon_muzzle_flash_shock_npc", PATTACH_POINT_FOLLOW, this, "rifle", false);

	DoMuzzleFlash();

	m_cAmmoLoaded--;// take away a bullet!

	SetAim(vecShootDir);


	/*	if ( GetEnemy() == NULL )
	return;

	Vector vecShootOrigin = Weapon_ShootPosition();
	Vector vecShootDir = GetShootEnemyDir( vecShootOrigin );

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	Vector	vecShellVelocity = right * random->RandomFloat(40,90) + up * random->RandomFloat( 75,200 ) + forward * random->RandomFloat( -40, 40 );
	//EjectShell( vecShootOrigin - vecShootDir * 24, vecShellVelocity, GetAbsAngles().y, 1 );
	FireBullets( sk_shock_trooper_pellets.GetFloat(), vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, m_iAmmoType, 0 ); // shoot +-7.5 degrees

	DoMuzzleFlash();

	m_cAmmoLoaded--;// take away a bullet!

	SetAim( vecShootDir );*/
}

//=========================================================
// start task
//=========================================================
void CNPC_shock_trooper::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_CHECK_FIRE:
		if (!NoFriendlyFire(GetEnemy()))
		{
			SetCondition(COND_WEAPON_BLOCKED_BY_FRIEND);
		}
		TaskComplete();
		break;

	case TASK_GRUNT_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget(bits_MEMORY_INCOVER);
		BaseClass::StartTask(pTask);
		break;

	case TASK_RELOAD:
		SetIdealActivity(ACT_RELOAD);
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		BaseClass::StartTask(pTask);
		if (GetMoveType() == MOVETYPE_FLYGRAVITY)
		{
			SetIdealActivity(ACT_GLIDE);
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CNPC_shock_trooper::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
	{
		// project a point along the toss vector and turn to face that point.
		GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + m_vecTossVelocity * 64, AI_KEEP_YAW_SPEED);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

//=========================================================
// PainSound
//=========================================================
void CNPC_shock_trooper::PainSound(const CTakeDamageInfo &info)
{
	if (gpGlobals->curtime > m_flNextPainTime)
	{
		/*CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "NPC_shock_trooper.Pain");*/

		EmitNpcSound("NPC_shock_trooper.Pain", entindex());

		m_flNextPainTime = gpGlobals->curtime + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CNPC_shock_trooper::DeathSound(const CTakeDamageInfo &info)
{
	/*CPASAttenuationFilter filter(this, ATTN_IDLE);
	EmitSound(filter, entindex(), "NPC_shock_trooper.Die");*/
	EmitNpcSound("NPC_shock_trooper.Die", entindex());
}

//=========================================================
// SetActivity 
//=========================================================
Activity CNPC_shock_trooper::NPC_TranslateActivity(Activity eNewActivity)
{
	switch (eNewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet(m_iWeapons, shock_trooper_9MMAR))
		{
			if (m_fStanding)
			{
				// get aimable sequence
				return (Activity)ACT_GRUNT_MP5_STANDING;
			}
			else
			{
				// get crouching shoot
				return (Activity)ACT_GRUNT_MP5_CROUCHING;
			}
		}
		else
		{
			if (m_fStanding)
			{
				// get aimable sequence
				return (Activity)ACT_GRUNT_SHOTGUN_STANDING;
			}
			else
			{
				// get crouching shoot
				return (Activity)ACT_GRUNT_SHOTGUN_CROUCHING;
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if (m_iWeapons & shock_trooper_HANDGRENADE)
		{
			// get toss anim
			return (Activity)ACT_GRUNT_TOSS_GRENADE;
		}
		else
		{
			// get launch anim
			return (Activity)ACT_GRUNT_LAUNCH_GRENADE;
		}
		break;
	case ACT_RUN:
		if (m_iHealth <= shock_trooper_LIMP_HEALTH)
		{
			// limp!
			return ACT_RUN_HURT;
		}
		else
		{
			return eNewActivity;
		}
		break;
	case ACT_WALK:
		if (m_iHealth <= shock_trooper_LIMP_HEALTH)
		{
			// limp!
			return ACT_WALK_HURT;
		}
		else
		{
			return eNewActivity;
		}
		break;
	case ACT_IDLE:
		if (m_NPCState == NPC_STATE_COMBAT)
		{
			eNewActivity = ACT_IDLE_ANGRY;
		}

		break;
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}

void CNPC_shock_trooper::ClearAttackConditions(void)
{
	bool fCanRangeAttack2 = HasCondition(COND_CAN_RANGE_ATTACK2);

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if (fCanRangeAttack2)
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition(COND_CAN_RANGE_ATTACK2);
	}
}

int CNPC_shock_trooper::SelectSchedule(void)
{

	// clear old sentence
	m_iSentence = shock_trooper_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if (GetMoveType() == MOVETYPE_FLYGRAVITY && m_NPCState != NPC_STATE_PRONE)
	{
		if (GetFlags() & FL_ONGROUND)
		{
			// just landed
			SetMoveType(MOVETYPE_STEP);
			SetGravity(1.0);
			return SCHED_GRUNT_REPEL_LAND;
		}
		else
		{
			// repel down a rope, 
			if (m_NPCState == NPC_STATE_COMBAT)
				return SCHED_GRUNT_REPEL_ATTACK;
			else
				return SCHED_GRUNT_REPEL;
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if (HasCondition(COND_HEAR_DANGER))
	{
		// dangerous sound nearby!

		//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
		// and the grunt should find cover from the blast
		// good place for "SHIT!" or some other colorful verbal indicator of dismay.
		// It's not safe to play a verbal order here "Scatter", etc cause 
		// this may only affect a single individual in a squad. 

		if (FOkToSpeak())
		{
			//SENTENCEG_PlayRndSz( edict(), "HG_GREN", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
			/*CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "NPC_shock_trooper.ST_GREN");*/

			EmitNpcSound("NPC_shock_trooper.ST_GREN", entindex());
			JustSpoke();
		}

		return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	switch (m_NPCState)
	{

	case NPC_STATE_PRONE:
	{
		if (m_bInBarnacleMouth)
		{
			return SCHED_GRUNT_BARNACLE_CHOMP;
		}
		else
		{
			return SCHED_GRUNT_BARNACLE_HIT;
		}
	}

	case NPC_STATE_COMBAT:
	{
		// dead enemy
		if (HasCondition(COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return BaseClass::SelectSchedule();
		}

		// new enemy
		if (HasCondition(COND_NEW_ENEMY))
		{
			if (m_pSquad)
			{
				if (!m_pSquad->IsLeader(this))
				{
					return SCHED_TAKE_COVER_FROM_ENEMY;
				}
				else
				{
					//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
					// monster and has made it the squad's enemy. You
					// can check pev->flags for FL_CLIENT to determine whether this is the player
					// or a monster. He's going to immediately start
					// firing, though. If you'd like, we can make an alternate "first sight" 
					// schedule where the leader plays a handsign anim
					// that gives us enough time to hear a short sentence or spoken command
					// before he starts pluggin away.
					if (FOkToSpeak())// && RANDOM_LONG(0,1))
					{
						if ((GetEnemy() != NULL) && GetEnemy()->IsPlayer())
						{
							// player
							//SENTENCEG_PlayRndSz( edict(), "HG_ALERT", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
							/*CPASAttenuationFilter filter(this);
							EmitSound(filter, entindex(), "NPC_shock_trooper.ST_ALERT");*/
							EmitNpcSound("NPC_shock_trooper.ST_ALERT", entindex());

						}
						else if ((GetEnemy() != NULL) &&
							(GetEnemy()->Classify() != CLASS_PLAYER_ALLY) &&
							(GetEnemy()->Classify() != CLASS_PLAYER) &&
							(GetEnemy()->Classify() != CLASS_COMBINE))
						{
							// monster
							//SENTENCEG_PlayRndSz(edict(), "HG_MONST", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
							/*CPASAttenuationFilter filter(this);
							EmitSound(filter, entindex(), "NPC_shock_trooper.ST_MONSTER");*/	// BriJee: Name fixup MONST MONSTER
							EmitNpcSound("NPC_shock_trooper.ST_MONSTER", entindex());

						}
						JustSpoke();
					}

					if (HasCondition(COND_CAN_RANGE_ATTACK1))
					{
						return SCHED_GRUNT_SUPPRESS;
					}
					else
					{
						return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
					}
				}
			}
		}
		// no ammo
		else if (HasCondition(COND_NO_PRIMARY_AMMO))
		{
			//!!!KELLY - this individual just realized he's out of bullet ammo. 
			// He's going to try to find cover to run to and reload, but rarely, if 
			// none is available, he'll drop and reload in the open here. 
			return SCHED_GRUNT_HIDE_RELOAD;
		}

		// damaged just a little
		else if (HasCondition(COND_LIGHT_DAMAGE))
		{
			// if hurt:
			// 90% chance of taking cover
			// 10% chance of flinch.
			int iPercent = random->RandomInt(0, 99);

			if (iPercent <= 90 && GetEnemy() != NULL)
			{
				// only try to take cover if we actually have an enemy!

				//!!!KELLY - this grunt was hit and is going to run to cover.
				if (FOkToSpeak()) // && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "ST_COVER", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					/*CPASAttenuationFilter filter(this);
					EmitSound(filter, entindex(), "NPC_shock_trooper.ST_COVER");*/

					EmitNpcSound("NPC_shock_trooper.ST_COVER", entindex());

					//m_iSentence = shock_trooper_SENT_COVER;
					JustSpoke();
				}
				return SCHED_TAKE_COVER_FROM_ENEMY;
			}
			else
			{
				return SCHED_SMALL_FLINCH;
			}
		}
		// can kick
		else if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			return SCHED_MELEE_ATTACK1;
		}
		// can grenade launch

		else if (FBitSet(m_iWeapons, shock_trooper_GRENADELAUNCHER) && HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
		{
			// shoot a grenade if you can
			return SCHED_RANGE_ATTACK2;
		}
		// can shoot
		else if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			if (m_pSquad)
			{
				if (m_pSquad->GetLeader() != NULL)
				{

					CAI_BaseNPC *pSquadLeader = m_pSquad->GetLeader()->MyNPCPointer();

					// if the enemy has eluded the squad and a squad member has just located the enemy
					// and the enemy does not see the squad member, issue a call to the squad to waste a 
					// little time and give the player a chance to turn.
					if (pSquadLeader && pSquadLeader->EnemyHasEludedMe() && !HasCondition(COND_ENEMY_FACING_ME))
					{
						return SCHED_GRUNT_FOUND_ENEMY;
					}
				}
			}

			if (OccupyStrategySlotRange(SQUAD_SLOT_ENGAGE1, SQUAD_SLOT_ENGAGE2))
			{
				// try to take an available ENGAGE slot
				return SCHED_RANGE_ATTACK1;
			}
			else if (HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				// throw a grenade if can and no engage slots are available
				return SCHED_RANGE_ATTACK2;
			}
			else
			{
				// hide!
				return SCHED_TAKE_COVER_FROM_ENEMY;
			}
		}
		// can't see enemy
		else if (HasCondition(COND_ENEMY_OCCLUDED))
		{
			if (HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
				if (FOkToSpeak())
				{
					//SENTENCEG_PlayRndSz( edict(), "HG_THROW", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					/*CPASAttenuationFilter filter(this);
					EmitSound(filter, entindex(), "NPC_shock_trooper.ST_THROW");*/

					EmitNpcSound("NPC_shock_trooper.ST_THROW", entindex());

					JustSpoke();
				}
				return SCHED_RANGE_ATTACK2;
			}
			else if (OccupyStrategySlotRange(SQUAD_SLOT_ENGAGE1, SQUAD_SLOT_ENGAGE2))
			{
				//!!!KELLY - grunt cannot see the enemy and has just decided to 
				// charge the enemy's position. 
				if (FOkToSpeak())// && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "ST_CHARGE", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					/*CPASAttenuationFilter filter(this);
					EmitSound(filter, entindex(), "NPC_shock_trooper.ST_CHARGE");*/

					EmitNpcSound("NPC_shock_trooper.ST_CHARGE", entindex());

					//m_iSentence = shock_trooper_SENT_CHARGE;
					JustSpoke();	//test
				}

				return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
			}
			else
			{
				//!!!KELLY - grunt is going to stay put for a couple seconds to see if
				// the enemy wanders back out into the open, or approaches the
				// grunt's covered position. Good place for a taunt, I guess?
				if (FOkToSpeak() && random->RandomInt(0, 1))
				{
					//SENTENCEG_PlayRndSz( edict(), "HG_TAUNT", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					/*CPASAttenuationFilter filter(this);
					EmitSound(filter, entindex(), "NPC_shock_trooper.ST_TAUNT");*/

					EmitNpcSound("NPC_shock_trooper.ST_TAUNT", entindex());

					JustSpoke();
				}
				return SCHED_STANDOFF;
			}
		}

		if (HasCondition(COND_SEE_ENEMY) && !HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
		}
	}
	case NPC_STATE_ALERT:
		if (HasCondition(COND_ENEMY_DEAD) && SelectWeightedSequence(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		{
			// Scan around for new enemies
			return SCHED_VICTORY_DANCE;
		}
		break;
	}

	return BaseClass::SelectSchedule();
}

int CNPC_shock_trooper::TranslateSchedule(int scheduleType)
{

	if (scheduleType == SCHED_CHASE_ENEMY_FAILED)
	{
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}
	switch (scheduleType)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		if (m_pSquad)
		{
			if (g_iSkillLevel == SKILL_HARD && HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				if (FOkToSpeak())
				{
					//SENTENCEG_PlayRndSz( edict(), "HG_THROW", shock_trooper_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					/*CPASAttenuationFilter filter(this);
					EmitSound(filter, entindex(), "NPC_shock_trooper.ST_THROW");*/

					EmitNpcSound("NPC_shock_trooper.ST_THROW", entindex());

					JustSpoke();
				}
				return SCHED_GRUNT_TOSS_GRENADE_COVER;
			}
			else
			{
				return SCHED_GRUNT_TAKE_COVER;
			}
		}
		else
		{
			if (random->RandomInt(0, 1))
			{
				return SCHED_GRUNT_TAKE_COVER;
			}
			else
			{
				return SCHED_GRUNT_GRENADE_COVER;
			}
		}
	}
	case SCHED_GRUNT_TAKE_COVER_FAILED:
	{
		if (HasCondition(COND_CAN_RANGE_ATTACK1) && OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
		{
			return SCHED_RANGE_ATTACK1;
		}

		return SCHED_FAIL;
	}
	break;

	case SCHED_RANGE_ATTACK1:
	{
		// randomly stand or crouch
		if (random->RandomInt(0, 9) == 0)
		{
			m_fStanding = random->RandomInt(0, 1) != 0;
		}

		if (m_fStanding)
			return SCHED_GRUNT_RANGE_ATTACK1B;
		else
			return SCHED_GRUNT_RANGE_ATTACK1A;
	}

	case SCHED_RANGE_ATTACK2:
	{
		return SCHED_GRUNT_RANGE_ATTACK2;
	}
	case SCHED_VICTORY_DANCE:
	{
		if (m_pSquad)
		{
			if (!m_pSquad->IsLeader(this))
			{
				return SCHED_GRUNT_FAIL;
			}
		}

		return SCHED_GRUNT_VICTORY_DANCE;
	}
	case SCHED_GRUNT_SUPPRESS:
	{
		if (GetEnemy()->IsPlayer() && m_fFirstEncounter)
		{
			m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
			return SCHED_GRUNT_SIGNAL_SUPPRESS;
		}
		else
		{
			return SCHED_GRUNT_SUPPRESS;
		}
	}
	case SCHED_FAIL:
	{
		if (GetEnemy() != NULL)
		{
			// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
			return SCHED_GRUNT_COMBAT_FAIL;
		}

		return SCHED_GRUNT_FAIL;
	}
	case SCHED_GRUNT_REPEL:
	{
		Vector vecVel = GetAbsVelocity();
		if (vecVel.z > -128)
		{
			vecVel.z -= 32;
			SetAbsVelocity(vecVel);
		}

		return SCHED_GRUNT_REPEL;
	}
	case SCHED_GRUNT_REPEL_ATTACK:
	{
		Vector vecVel = GetAbsVelocity();
		if (vecVel.z > -128)
		{
			vecVel.z -= 32;
			SetAbsVelocity(vecVel);
		}

		return SCHED_GRUNT_REPEL_ATTACK;
	}
	default:
	{
		return BaseClass::TranslateSchedule(scheduleType);
	}
	}
}

//=========================================================
// Cshock_trooperRepel - when triggered, spawns a monster_human_grunt
// repelling down a line.
//=========================================================

class CNPC_shock_trooperRepel :public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_shock_trooperRepel, CAI_BaseNPC);
public:
	void Spawn(void);
	void Precache(void);
	void RepelUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture;	// Don't save, precache

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(npc_shock_trooper_repel, CNPC_shock_trooperRepel);



//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_shock_trooperRepel)
DEFINE_USEFUNC(RepelUse),
//DEFINE_FIELD( m_iSpriteTexture, FIELD_INTEGER ),
END_DATADESC()

void CNPC_shock_trooperRepel::Spawn(void)
{
	Precache();
	SetSolid(SOLID_NONE);

	SetUse(&CNPC_shock_trooperRepel::RepelUse);
}

void CNPC_shock_trooperRepel::Precache(void)
{
	UTIL_PrecacheOther("npc_shock_trooper");
	m_iSpriteTexture = PrecacheModel("sprites/rope.vmt");
}

void CNPC_shock_trooperRepel::RepelUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -4096.0), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	CBaseEntity *pEntity = Create("npc_shock_trooper", GetAbsOrigin(), GetAbsAngles());
	CAI_BaseNPC *pGrunt = pEntity->MyNPCPointer();
	pGrunt->SetMoveType(MOVETYPE_FLYGRAVITY);
	pGrunt->SetGravity(0.001);
	pGrunt->SetAbsVelocity(Vector(0, 0, random->RandomFloat(-196, -128)));
	pGrunt->SetActivity(ACT_GLIDE);
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.endpos;

	CBeam *pBeam = CBeam::BeamCreate("sprites/rope.vmt", 10);
	pBeam->PointEntInit(GetAbsOrigin() + Vector(0, 0, 112), pGrunt);
	pBeam->SetBeamFlags(FBEAM_SOLID);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + -4096.0 * tr.fraction / pGrunt->GetAbsVelocity().z + 0.5);

	UTIL_Remove(this);
}


//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC(monster_human_grunt, CNPC_shock_trooper)

DECLARE_ACTIVITY(ACT_GRUNT_LAUNCH_GRENADE)
DECLARE_ACTIVITY(ACT_GRUNT_TOSS_GRENADE)
DECLARE_ACTIVITY(ACT_GRUNT_MP5_STANDING);
DECLARE_ACTIVITY(ACT_GRUNT_MP5_CROUCHING);
DECLARE_ACTIVITY(ACT_GRUNT_SHOTGUN_STANDING);
DECLARE_ACTIVITY(ACT_GRUNT_SHOTGUN_CROUCHING);

DECLARE_CONDITION(COND_GRUNT_NOFIRE)

DECLARE_TASK(TASK_GRUNT_FACE_TOSS_DIR)
DECLARE_TASK(TASK_GRUNT_SPEAK_SENTENCE)
DECLARE_TASK(TASK_GRUNT_CHECK_FIRE)

DECLARE_SQUADSLOT(SQUAD_SLOT_GRENADE1)
DECLARE_SQUADSLOT(SQUAD_SLOT_GRENADE2)
DECLARE_SQUADSLOT(SQUAD_SLOT_ENGAGE1)
DECLARE_SQUADSLOT(SQUAD_SLOT_ENGAGE2)

//=========================================================
// > SCHED_GRUNT_FAIL
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_FAIL,

"	Tasks"
"		TASK_STOP_MOVING		0"
"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
"		TASK_WAIT				0"
"		TASK_WAIT_PVS			0"
"	"
"	Interrupts"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_MELEE_ATTACK1"
)

//=========================================================
// > SCHED_GRUNT_COMBAT_FAIL
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_COMBAT_FAIL,

"	Tasks"
"		TASK_STOP_MOVING		0"
"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
"		TASK_WAIT_FACE_ENEMY	2"
"		TASK_WAIT_PVS			0"
"	"
"	Interrupts"
"		COND_CAN_RANGE_ATTACK1"
)

//=========================================================
// > SCHED_GRUNT_VICTORY_DANCE
// Victory dance!
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_VICTORY_DANCE,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_ENEMY					0"
"		TASK_WAIT						1.5"
"		TASK_GET_PATH_TO_ENEMY_CORPSE	0"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_FACE_ENEMY					0"
"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_VICTORY_DANCE"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

//=========================================================
// > SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY"
"		TASK_GET_PATH_TO_ENEMY		0"
"		TASK_GRUNT_SPEAK_SENTENCE	0"
"		TASK_RUN_PATH				0"
"		TASK_WAIT_FOR_MOVEMENT		0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
"		COND_CAN_MELEE_ATTACK2"
"		COND_HEAR_DANGER"
)

//=========================================================
// This is a schedule I added that borrows some HL2 technology
// to be smarter in cases where HL1 was pretty dumb. I've wedged
// this between ESTABLISH_LINE_OF_FIRE and TAKE_COVER_FROM_ENEMY (sjb)
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_GRUNT_TAKE_COVER_FROM_ENEMY"
"		TASK_GET_PATH_TO_ENEMY_LKP_LOS	0"
"		TASK_GRUNT_SPEAK_SENTENCE		0"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
"		COND_CAN_MELEE_ATTACK2"
"		COND_HEAR_DANGER"
)

//=========================================================
// > SCHED_GRUNT_FOUND_ENEMY
// Grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_FOUND_ENEMY,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_ENEMY					0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SIGNAL1"
"	"
"	Interrupts"
"		COND_HEAR_DANGER"
)


//=========================================================
// > SCHED_GRUNT_COMBAT_FACE
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_COMBAT_FACE,

"	Tasks"
"		TASK_STOP_MOVING		0"
"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
"		TASK_FACE_ENEMY			0"
"		TASK_WAIT				1.5"
"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_GRUNT_SWEEP"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
)


//=========================================================
// > SCHED_GRUNT_SIGNAL_SUPPRESS
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_SIGNAL_SUPPRESS,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_IDEAL					0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SIGNAL2"
"		TASK_FACE_ENEMY					0"
"		TASK_GRUNT_CHECK_FIRE			0"
"		TASK_RANGE_ATTACK1				0"
"		TASK_FACE_ENEMY					0"
"		TASK_GRUNT_CHECK_FIRE			0"
"		TASK_RANGE_ATTACK1				0"
"		TASK_FACE_ENEMY					0"
"		TASK_GRUNT_CHECK_FIRE			0"
"		TASK_RANGE_ATTACK1				0"
"		TASK_FACE_ENEMY					0"
"		TASK_GRUNT_CHECK_FIRE			0"
"		TASK_RANGE_ATTACK1				0"
"		TASK_FACE_ENEMY					0"
"		TASK_GRUNT_CHECK_FIRE			0"
"		TASK_RANGE_ATTACK1				0"
"	"
"	Interrupts"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_GRUNT_NOFIRE"
"		COND_NO_PRIMARY_AMMO"
"		COND_HEAR_DANGER"
)

//=========================================================
// > SCHED_GRUNT_SUPPRESS
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_SUPPRESS,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_ENEMY				0"
"		TASK_GRUNT_CHECK_FIRE		0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_FACE_ENEMY				0"
"		TASK_GRUNT_CHECK_FIRE		0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_FACE_ENEMY				0"
"		TASK_GRUNT_CHECK_FIRE		0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_FACE_ENEMY				0"
"		TASK_GRUNT_CHECK_FIRE		0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_FACE_ENEMY				0"
"		TASK_GRUNT_CHECK_FIRE		0"
"		TASK_RANGE_ATTACK1			0"
"	"
"	Interrupts"
"		COND_ENEMY_DEAD"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_GRUNT_NOFIRE"
"		COND_NO_PRIMARY_AMMO"
"		COND_HEAR_DANGER"
)

//=========================================================
// > SCHED_GRUNT_WAIT_IN_COVER
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_WAIT_IN_COVER,

"	Tasks"
"		TASK_STOP_MOVING		0"
"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
"		TASK_WAIT_FACE_ENEMY	1"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_HEAR_DANGER"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_MELEE_ATTACK2"
)


//=========================================================
// > SCHED_GRUNT_TAKE_COVER
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_TAKE_COVER,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_GRUNT_TAKE_COVER_FAILED"
"		TASK_WAIT					0.2"
"		TASK_FIND_COVER_FROM_ENEMY	0"
"		TASK_GRUNT_SPEAK_SENTENCE	0"
"		TASK_RUN_PATH				0"
"		TASK_WAIT_FOR_MOVEMENT		0"
"		TASK_REMEMBER				MEMORY:INCOVER"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER"
"	"
"	Interrupts"
)


//=========================================================
// > SCHED_GRUNT_GRENADE_COVER
// drop grenade then run to cover.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_GRENADE_COVER,

"	Tasks"
"		TASK_STOP_MOVING						0"
"		TASK_FIND_COVER_FROM_ENEMY				99"
"		TASK_FIND_FAR_NODE_COVER_FROM_ENEMY		384"
"		TASK_PLAY_SEQUENCE						ACTIVITY:ACT_SPECIAL_ATTACK1"
"		TASK_CLEAR_MOVE_WAIT					0"
"		TASK_RUN_PATH							0"
"		TASK_WAIT_FOR_MOVEMENT					0"
"		TASK_SET_SCHEDULE						SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER"
"	"
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_TOSS_GRENADE_COVER
// drop grenade then run to cover.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_TOSS_GRENADE_COVER,

"	Tasks"
"		TASK_FACE_ENEMY				0"
"		TASK_RANGE_ATTACK2			0"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_TAKE_COVER_FROM_ENEMY"
"	"
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_HIDE_RELOAD
// Grunt reload schedule
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_HIDE_RELOAD,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_GRUNT_RELOAD"
"		TASK_FIND_COVER_FROM_ENEMY		0"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_REMEMBER					MEMORY:INCOVER"
"		TASK_FACE_ENEMY					0"
"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_RELOAD"
"	"
"	Interrupts"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
)

//=========================================================
// > SCHED_GRUNT_SWEEP
// Do a turning sweep of the area
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_SWEEP,

"	Tasks"
"		TASK_TURN_LEFT			179"
"		TASK_WAIT				1"
"		TASK_TURN_LEFT			179"
"		TASK_WAIT				1"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_RANGE_ATTACK2"
"		COND_HEAR_WORLD"
"		COND_HEAR_DANGER"
"		COND_HEAR_PLAYER"
)

//=========================================================
// > SCHED_GRUNT_RANGE_ATTACK1A
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_RANGE_ATTACK1A,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCH"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_HEAVY_DAMAGE"
"		COND_ENEMY_OCCLUDED"
"		COND_HEAR_DANGER"
"		COND_GRUNT_NOFIRE"
"		COND_NO_PRIMARY_AMMO"
)

//=========================================================
// > SCHED_GRUNT_RANGE_ATTACK1B
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_RANGE_ATTACK1B,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_IDLE_ANGRY"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"		TASK_FACE_ENEMY						0"
"		TASK_GRUNT_CHECK_FIRE				0"
"		TASK_RANGE_ATTACK1					0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_HEAVY_DAMAGE"
"		COND_ENEMY_OCCLUDED"
"		COND_HEAR_DANGER"
"		COND_GRUNT_NOFIRE"
"		COND_NO_PRIMARY_AMMO"
)

//=========================================================
// > SCHED_GRUNT_RANGE_ATTACK2
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_RANGE_ATTACK2,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_GRUNT_FACE_TOSS_DIR	0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_RANGE_ATTACK2"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER" // don't run immediately after throwing grenade.
"	"
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_REPEL
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_REPEL,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_GLIDE"
"	"
"	Interrupts"
"		COND_SEE_ENEMY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_PLAYER"
"		COND_HEAR_COMBAT"
)

//=========================================================
// > SCHED_GRUNT_REPEL_ATTACK
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_REPEL_ATTACK,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_ENEMY				0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_FLY"
"	"
"	Interrupts"
"		COND_ENEMY_OCCLUDED"
)

//=========================================================
// > SCHED_GRUNT_REPEL_LAND
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_REPEL_LAND,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_LAND"
"		TASK_GET_PATH_TO_LASTPOSITION		0"
"		TASK_RUN_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_CLEAR_LASTPOSITION				0"
"	"
"	Interrupts"
"		COND_SEE_ENEMY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
"		COND_HEAR_PLAYER"
)

//=========================================================
// > SCHED_GRUNT_RELOAD
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_RELOAD,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_RELOAD"
"	"
"	Interrupts"
"		COND_HEAVY_DAMAGE"
)

//=========================================================
// > SCHED_GRUNT_TAKE_COVER_FROM_ENEMY
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_TAKE_COVER_FROM_ENEMY,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_WAIT						0.2"
"		TASK_FIND_COVER_FROM_ENEMY		0"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_REMEMBER					MEMORY:INCOVER"
"		TASK_FACE_ENEMY					0"
"		TASK_WAIT						1"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
)

//=========================================================
// > SCHED_GRUNT_TAKE_COVER_FAILED
// special schedule type that forces analysis of conditions and picks
// the best possible schedule to recover from this type of failure.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_TAKE_COVER_FAILED,
"	Tasks"
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_BARNACLE_HIT
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_BARNACLE_HIT,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_HIT"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_BARNACLE_PULL"
""
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_BARNACLE_PULL
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_BARNACLE_PULL,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_PULL"
""
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_BARNACLE_CHOMP
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_BARNACLE_CHOMP,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHOMP"
"		 TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_BARNACLE_CHEW"
""
"	Interrupts"
)

//=========================================================
// > SCHED_GRUNT_BARNACLE_CHEW
//=========================================================
DEFINE_SCHEDULE
(
SCHED_GRUNT_BARNACLE_CHEW,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHEW"
)

AI_END_CUSTOM_NPC()


// BJ: NOT NEEDED, cleanup, maybe use later
/*
//=========================================================
// DEAD shock_trooper PROP
//=========================================================
class CNPC_Deadshock_trooper : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_Deadshock_trooper, CAI_BaseNPC);
public:
	void Spawn(void);
	Class_T	Classify(void) { return	CLASS_RACEX; }
	float MaxYawSpeed(void) { return 8.0f; }

	bool KeyValue(const char *szKeyName, const char *szValue);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[3];
};

char *CNPC_Deadshock_trooper::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

bool CNPC_Deadshock_trooper::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "pose"))
		m_iPose = atoi(szValue);
	else
		CAI_BaseNPC::KeyValue(szKeyName, szValue);

	return true;
}

LINK_ENTITY_TO_CLASS(npc_shock_trooper_dead, CNPC_Deadshock_trooper);

//=========================================================
// ********** Deadshock_trooper SPAWN **********
//=========================================================
void CNPC_Deadshock_trooper::Spawn(void)
{
	PrecacheModel("models/RaseX/shock_trooper.mdl");
	SetModel("models/RaseX/shock_trooper.mdl");

	ClearEffects();
	SetSequence(0);
	m_bloodColor = BLOOD_COLOR_YELLOW;

	SetSequence(LookupSequence(m_szPoses[m_iPose]));

	if (GetSequence() == -1)
	{
		Msg("Dead shock_trooper with bad pose\n");
	}

	// Corpses have less health
	m_iHealth = 8;

	// map old bodies onto new bodies
	switch (m_nBody)
	{
	case 0: // Grunt with Gun
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 1: // Commander with Gun
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 2: // Grunt no Gun
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	case 3: // Commander no Gun
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	}

	NPCInitDead();
}

*/