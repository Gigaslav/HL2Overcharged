//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Voltigore & Voltigore baby op4 Remake
//
//	9 sept 2021
//=============================================================================//

#include	"cbase.h"
#include	"beam_shared.h"
#include	"Sprite.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_memory.h"
#include	"ai_route.h"
#include	"ai_motor.h"
#include	"overcharged/npc_voltigore.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"shake.h"
#include	"decals.h"
#include	"particle_smokegrenade.h"
#include	"gib.h"
#include	"func_break.h"
#include	"hl2_shareddefs.h"	//added
#include	"particle_parse.h"		
#include	"prop_combine_ball.h"
#include	"physics_prop_ragdoll.h"	//post delayed anim
#include	"npc_antlion.h"	//flip antlions in volt baby rare attack

//=========================================================
// voltigore
//=========================================================
#define volt_MELEE_ATTACKDIST 100.0f	// melee attack distance	//120 def		// MELEE
#define volt_MELEE_ATTACKDIST_BABY 50.0f

// volt animation events
#define volt_AE_SLASH_LEFT			1
//#define volt_AE_BEAM_ATTACK_RIGHT	2		// unused
#define volt_AE_LEFT_FOOT			3
#define volt_AE_RIGHT_FOOT			4
#define volt_AE_voenergy			5
#define volt_AE_BREATHE				6
#define volt_AE_EXPLODE				7

// ALLOWED damage types
#define volt_DAMAGE	( DMG_ENERGYBEAM | DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BURN | DMG_VEHICLE | DMG_CLUB | DMG_SHOCK | DMG_ENERGYBEAM | DMG_AIRBOAT | DMG_BUCKSHOT | DMG_GAUSS | DMG_BLAST | DMG_NEVERGIB ) 

//nevergib - crossbow

// dont use this
// DMG_DISSOLVE | DMG_DISSOLVE_EGON | DMG_DISSOLVE_EGON_ELECTRICAL

#define volt_ELSPHERE_DIST			330	// BJ: SHOOT ELECTRIC SPHERE DIST

ConVar sk_voltigore_health ( "sk_voltigore_health", "0" );
ConVar sk_voltigore_dmg_slash( "sk_voltigore_dmg_slash", "0" );
ConVar sk_voltigore_dmg_electric_prepare("sk_voltigore_dmg_electric_prepare", "0");	//pre shot dmg inside volt class
ConVar sk_voltigore_dmg_electric_sphere( "sk_voltigore_dmg_electric_sphere", "0" );
ConVar sk_voltigore_dmg_electric_sphere_explosion("sk_voltigore_dmg_electric_sphere_explosion", "0");
ConVar sk_voltigore_dmg_electric_sphere_speed("sk_voltigore_dmg_electric_sphere_speed", "0");
ConVar sk_voltigore_dmg_bigexplode("sk_voltigore_dmg_bigexplode", "0");
ConVar sk_voltigore_allow_extra_electric_throw("sk_voltigore_allow_extra_electric_throw", "0", FCVAR_ARCHIVE);

ConVar sk_voltigore_baby_health("sk_voltigore_baby_health", "0");
ConVar sk_voltigore_baby_dmg_slash("sk_voltigore_baby_dmg_slash", "0");
ConVar sk_voltigore_baby_dmg_electric_prepare("sk_voltigore_baby_dmg_electric_prepare", "0");
ConVar sk_voltigore_baby_dmg_electric_beam("sk_voltigore_baby_dmg_electric_beam", "0");

enum
{
	TASK_SOUNDS_ATTACK = LAST_SHARED_TASK,
	TASK_ELECTRO_SHOT,
};

enum
{
	SCHED_volt_ELECTRO = LAST_SHARED_SCHEDULE,
	SCHED_volt_SWIPE,
	SCHED_volt_CHASE_ENEMY,
	SCHED_volt_CHASE_ENEMY_FAILED,
};

LINK_ENTITY_TO_CLASS( npc_voltigore, CNPC_voltigore );
LINK_ENTITY_TO_CLASS( npc_voltigore_baby, CNPC_voltigore);	//BJ: baby class added


IMPLEMENT_SERVERCLASS_ST(CNPC_voltigore, DT_NPC_voltigore)	// BJ: Bleed added
SendPropInt(SENDINFO(m_iBleedingLevel), 2, SPROP_UNSIGNED),

END_SEND_TABLE()	//end

BEGIN_DATADESC( CNPC_voltigore )
	DEFINE_FIELD( m_seeTime_vo, FIELD_TIME ),
	DEFINE_FIELD( m_flameTime_vo, FIELD_TIME ),
	DEFINE_FIELD( m_streakTime_vo, FIELD_TIME ),
	DEFINE_FIELD( m_flDmgTime_vo, FIELD_TIME ),
	DEFINE_FIELD( m_painSoundTime_vo, FIELD_TIME ),
	DEFINE_FIELD( m_fIsBabyVolt, FIELD_BOOLEAN),
	DEFINE_FIELD( m_PresetDamage, FIELD_FLOAT ),
	DEFINE_FIELD( m_MeleeDistRecalc, FIELD_FLOAT ),
	DEFINE_FIELD(m_iBleedingLevel, FIELD_CHARACTER),	// BJ: Bleed added
END_DATADESC()

static void MoveToGround( Vector *position, CBaseEntity *ignore, const Vector &mins, const Vector &maxs )
{
	trace_t tr;
	// Find point on floor where enemy would stand at chasePosition
	Vector floor = *position;
	floor.z -= 1024;
	UTIL_TraceHull( *position, floor, mins, maxs, MASK_NPCSOLID, ignore, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction < 1 )
	{
		position->z = tr.endpos.z;
	}
}

//============================================================================================================= electric sphere ent
class CVoltElectricSphere : public CBaseCombatCharacter
{
	DECLARE_CLASS(CVoltElectricSphere, CBaseCombatCharacter);

public:
	CVoltElectricSphere() { };
	~CVoltElectricSphere();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);
	void BoltTouch(CBaseEntity *pOther);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CVoltElectricSphere *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *pentOwner = NULL);

protected:
	bool	ShouldDrawWaterImpacts(const trace_t &shot_trace);
	int     m_nLightningSprite1;
	bool	CreateSprites(void);

	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

LINK_ENTITY_TO_CLASS(VoltigoreElectricSphere, CVoltElectricSphere);

BEGIN_DATADESC(CVoltElectricSphere)

DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CVoltElectricSphere, DT_VoltElectricSphere)
END_SEND_TABLE()

#define VSPHERE_MODEL "models/blackout.mdl"		// sphere model preset

CVoltElectricSphere *CVoltElectricSphere::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CAI_BaseNPC *pentOwner)
{
	CVoltElectricSphere *pVoltElectricSphere = (CVoltElectricSphere *)CreateEntityByName("VoltigoreElectricSphere");
	UTIL_SetOrigin(pVoltElectricSphere, vecOrigin);
	pVoltElectricSphere->SetAbsAngles(angAngles);
	pVoltElectricSphere->Spawn();
	pVoltElectricSphere->SetOwnerEntity(pentOwner);

	pVoltElectricSphere->m_iDamage = iDamage;

	return pVoltElectricSphere;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CVoltElectricSphere::~CVoltElectricSphere(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoltElectricSphere::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CVoltElectricSphere::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoltElectricSphere::CreateSprites(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVoltElectricSphere::Spawn(void)
{
	Precache();

	SetModel(VSPHERE_MODEL);
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_BBOX);

	//AddEffects( EF_NODRAW );
	AddEffects(EF_NOSHADOW);

	UpdateWaterState();

	SetTouch(&CVoltElectricSphere::BoltTouch);

	SetThink(&CVoltElectricSphere::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	CreateSprites();

	// 2 particles. one line one electric
	DispatchParticleEffect("volt_sphere_longfire", PATTACH_ABSORIGIN_FOLLOW, this);
	DispatchParticleEffect("volt_sphere_particles", PATTACH_ABSORIGIN_FOLLOW, this);
}

void CVoltElectricSphere::Precache(void)
{
	PrecacheModel(VSPHERE_MODEL);

	// 2 particles precache
	PrecacheParticleSystem("volt_sphere_longfire");
	PrecacheParticleSystem("volt_sphere_particles");

	PrecacheScriptSound("NPC_Voltigore.ElectricSphereExplode");
	m_nLightningSprite1 = PrecacheModel("sprites/bluelaser1.vmt");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CVoltElectricSphere::BoltTouch(CBaseEntity *pOther)
{
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();
		ShouldDrawWaterImpacts(tr);

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_voltigore_dmg_electric_sphere.GetFloat(), DMG_SHOCK);
		CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
		dmgInfo.SetDamagePosition(tr.endpos);
		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

		RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_dmg_electric_sphere_explosion.GetFloat(), DMG_SHOCK), GetAbsOrigin(), 256, CLASS_RACEX, NULL);

		ApplyMultiDamage();

		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		SetAbsVelocity(Vector(0, 0, 0));

		EmitSound("NPC_Voltigore.ElectricSphereExplode");

		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}
	else
	{
		trace_t	tr;
		//tr = BaseClass::GetTouchTrace();
		//UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, 0 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		ShouldDrawWaterImpacts(tr);
		Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);

		RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_dmg_electric_sphere_explosion.GetFloat(), DMG_SHOCK), GetAbsOrigin(), 256, CLASS_RACEX, NULL);

		EmitSound("NPC_Voltigore.ElectricSphereExplode");

		UTIL_DecalTrace( &tr, "FadingScorch" );
		UTIL_Remove(this);
	}

}

//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )
bool CVoltElectricSphere::ShouldDrawWaterImpacts(const trace_t &shot_trace)
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
		data.m_flScale = random->RandomFloat(2.0, 4.0f);

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
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVoltElectricSphere::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity() / 2.0f, angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.1f);

	if (GetWaterLevel() == 0)
		return;

	RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_dmg_electric_sphere_explosion.GetFloat(), DMG_SHOCK), GetAbsOrigin(), 256, CLASS_RACEX, NULL);
	EmitSound("NPC_Voltigore.ElectricSphereExplode");

	UTIL_Remove(this);	// remove ent on water hit
}

//============================================================================================================= volt npc

//=========================================================================
// voltigore
//=========================================================================

//=========================================================
// Spawn
//=========================================================
void CNPC_voltigore::Spawn()
{
	Precache();

	if (FClassnameIs(this, "npc_voltigore"))
	{
		m_fIsBabyVolt = false;
		SetModel("models/RaceX/voltigore.mdl");
	}
	else if (FClassnameIs(this, "npc_voltigore_baby"))
	{
		m_fIsBabyVolt = true;
		SetModel("models/RaceX/voltigore_baby.mdl");
	}
	else
	{
		// custom models here reserved
		SetModel("models/RaceX/voltigore.mdl");	//temp enhance later
	}

	SetNavType(NAV_GROUND);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	if (!IsBabyVolt())	// only bigger cannot be dissolved or grabbed by physcannon
	{
		AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL);	// BJ: NO DISSOLVE NO MEGAPHYS instakill
	}

	Vector vecSurroundingMins(-80, -80, 0);
	Vector vecSurroundingMaxs(80, 80, 214);
	CollisionProp()->SetSurroundingBoundsType(USE_SPECIFIED_BOUNDS, &vecSurroundingMins, &vecSurroundingMaxs);

	m_bloodColor = BLOOD_COLOR_YELLOW;

	if (IsBabyVolt())
	{
		m_iHealth = sk_voltigore_baby_health.GetFloat();
	}
	else
	{
		m_iHealth = sk_voltigore_health.GetFloat();
	}

	SetViewOffset( Vector ( 0, 0, 96 ) );
	//m_flFieldOfView		= -0.2;

	m_flFieldOfView = -0.707;	// 270 degrees
	SetDistLook(1024);	// BJ: Test view

	m_NPCState			= NPC_STATE_NONE;

	CapabilitiesAdd( bits_CAP_MOVE_GROUND );
	CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );

	if (IsBabyVolt())
	{
		SetHullType(HULL_TINY);
	}
	else
	{
		SetHullType(HULL_LARGE);
	}
	SetHullSizeNormal();

	m_seeTime_vo = gpGlobals->curtime + 5;
	m_flameTime_vo = gpGlobals->curtime + 2;
		
	NPCInit();

	BaseClass::Spawn();

	GetEnemies()->SetFreeKnowledgeDuration( 59.0f );

	iNumDeathAnim = 0;	// BJ: Reset death anim index
	iCountTillExplode = 0;	// BJ: Count events to DO regular volt explode, must be 3
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_voltigore::Precache()
{
	PrecacheModel("models/RaceX/voltigore.mdl");
	PrecacheModel("models/RaceX/voltigore_baby.mdl");

	if (FClassnameIs(this, "npc_voltigore_baby"))	// BJ: Baby first, all other regular sounds. Precache goes first so didnt use IsBabyVolt here.
	{
		PrecacheScriptSound("NPC_Voltigore_Baby.AttackHit");
		PrecacheScriptSound("NPC_Voltigore_Baby.AttackMiss");
		PrecacheScriptSound("NPC_Voltigore_Baby.Footstep");
		PrecacheScriptSound("NPC_Voltigore_Baby.Communicate");
		PrecacheScriptSound("NPC_Voltigore_Baby.Attack");
		PrecacheScriptSound("NPC_Voltigore_Baby.Pain");
		PrecacheScriptSound("NPC_Voltigore_Baby.AttackElectricBeam");
	}
	else //if (FClassnameIs(this, "npc_voltigore"))
	{
		PrecacheScriptSound("NPC_Voltigore.AttackHit");
		PrecacheScriptSound("NPC_Voltigore.AttackMiss");
		PrecacheScriptSound("NPC_Voltigore.Footstep");
		PrecacheScriptSound("NPC_Voltigore.Communicate");
		PrecacheScriptSound("NPC_Voltigore.Attack");
		PrecacheScriptSound("NPC_Voltigore.Pain");
		PrecacheScriptSound("NPC_Voltigore.AttackElectric");
		PrecacheScriptSound("NPC_Voltigore.SelfExplode");
	}

	// paw charging effects
	PrecacheParticleSystem("voltigore_baby_paw_charge_preshot");
	PrecacheParticleSystem("voltigore_baby_middle_charge_preshot");
	PrecacheParticleSystem("voltigore_baby_beam_closeattack");

	//PrecacheParticleSystem("Weapon_Combine_Ion_Cannon");	//TEST Replace later with closeattack

	PrecacheParticleSystem("voltigore_paw_charge_preshot");
	PrecacheParticleSystem("voltigore_middle_charge_preshot");

	PrecacheParticleSystem("voltigore_big_explode_pre");
	PrecacheParticleSystem("voltigore_big_explode_post");

	PrecacheParticleSystem("blood_antlionguard_injured_light");	// BJ: Bleed added
	PrecacheParticleSystem("blood_antlionguard_injured_heavy");

}

Class_T  CNPC_voltigore::Classify ( void )
{
	return CLASS_RACEX;	// BJ: RaceX Custom Class
}

void CNPC_voltigore::PrescheduleThink( void )
{
	if ( !HasCondition( COND_SEE_ENEMY ) )
	{
		m_seeTime_vo = gpGlobals->curtime + 5;
	}
	else
	{
		//BJ: Nothing! Just nothing.
	}

	// BJ: Bleed added
	m_iBleedingLevel = GetBleedingLevel();
}

float CNPC_voltigore::MaxYawSpeed ( void )	
{
	float ys = 60;

	switch ( GetActivity() )
	{
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_WALK:
	case ACT_RUN:
		ys = 60;
		break;

	default:
		ys = 60;
		break;
	}

	return ys;
}

//=========================================================
// Baby Voltigore rare beam attack
//=========================================================
void CNPC_voltigore::ElectricBeam(void)
{
	DevMsg("Volt Rare Electric Beam Attack \n");

	/*

	Vector forward;
	GetVectors(&forward, NULL, NULL);

	Vector vecSrc = GetAbsOrigin() + GetViewOffset();
	Vector vecAim = GetShootEnemyDir(vecSrc, false);

	if (GetEnemy())
	{
		Vector vecTarget = GetEnemy()->BodyTarget(vecSrc, false);
	}

	trace_t tr;
	AI_TraceLine(vecSrc, vecSrc + (vecAim * InnateRange1MaxRange()), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_left", false);
	DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_right", false);

	DispatchParticleEffect("voltigore_baby_beam_closeattack", GetAbsOrigin(), tr.endpos, vec3_angle, NULL );
	//DispatchParticleEffect("Weapon_Combine_Ion_Cannon", GetAbsOrigin(), tr.endpos, vec3_angle, NULL );	// BJ: Replace later

	*/

	Vector vecSrc, vecAiming, vecShootOrigin, vecTarget;
	vecShootOrigin = this->GetAbsOrigin(); //pOperator->Weapon_ShootPosition();

	//GetAttachment(LookupAttachment("muzzle"), vecShootOrigin); //use attachment as start position

	GetVectors(&vecAiming, NULL, NULL);
	vecSrc = WorldSpaceCenter() + vecAiming * 64;

	Vector	impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	//CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	//vecTarget = pNPC->GetEnemy()->BodyTarget(vecSrc);
	if (GetEnemy())
	{
		Vector vecTarget = GetEnemy()->BodyTarget(vecSrc, false);
	}


	//CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	//WeaponSound(SINGLE_NPC);
	//pOperator->DoMuzzleFlash();
	//m_iClip1 = m_iClip1 - 1;

	trace_t	tr;
	Vector vecShootPos;

	AI_TraceLine(vecShootOrigin, vecTarget, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	DispatchParticleEffect("voltigore_baby_beam_closeattack", vecShootOrigin, tr.endpos, vec3_angle, NULL);


	// end

	DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_left", false);
	DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_right", false);


	CBaseEntity *pEntity = tr.m_pEnt;
	if (pEntity != NULL && m_takedamage)
	{
		CTakeDamageInfo dmgInfo(this, this, sk_voltigore_baby_dmg_electric_beam.GetFloat(), DMG_PARALYZE);
		dmgInfo.SetDamagePosition(tr.endpos);
		VectorNormalize(vecAiming);

		dmgInfo.SetDamageForce(1 * 10 * 12 * vecAiming);	//5 100 12 def

		if (FClassnameIs(pEntity, "npc_antlion"))		// BJ: Antlion flip specials
		{
			CNPC_Antlion *pAntlion = static_cast<CNPC_Antlion *>(pEntity);
			pAntlion->Flip();
		}

		pEntity->DispatchTraceAttack(dmgInfo, vecAiming, &tr);
	}

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "NPC_Voltigore_Baby.AttackElectricBeam");

}

void CNPC_voltigore::ElectricAttack(void)
{
	DevMsg("Volt Electric Prepare near damage \n");

	if (IsBabyVolt())
	{
		RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_baby_dmg_electric_prepare.GetFloat(), DMG_SHOCK), GetAbsOrigin(), 128, CLASS_RACEX, NULL);
		ElectricBeam();
	}
	else
	{
		RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_dmg_electric_prepare.GetFloat(), DMG_SHOCK), GetAbsOrigin(), 256, CLASS_RACEX, NULL);
	}

	if (GetEnemy() == NULL)
		return;

	Vector vecShootOrigin;
	GetAttachment("electric", vecShootOrigin, NULL);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	Vector	vecShellVelocity = right * random->RandomFloat(40, 90) + up * random->RandomFloat(75, 200) + forward * random->RandomFloat(-40, 40);

	Vector src;
	GetAttachment("electric", src, NULL);

	QAngle angAiming;
	VectorAngles(vecShootDir, angAiming);

	if (!IsBabyVolt())	// only regular can throw electric sphere
	{
		CVoltElectricSphere *pVoltElectricSphere = CVoltElectricSphere::BoltCreate(src, angAiming, 0, this);

		if (this->GetWaterLevel() == 3)
		{
			CTakeDamageInfo hitself(this, this, 200, DMG_SHOCK);
			TakeDamage(hitself);
		}
		else
		{
			pVoltElectricSphere->SetAbsVelocity(vecShootDir * sk_voltigore_dmg_electric_sphere_speed.GetFloat());	// last adjust speed // * 600 ok
		}

	}
	else	// baby volt can only damage self if in water
	{
		if (this->GetWaterLevel() == 3)
		{
			CTakeDamageInfo hitself(this, this, 100, DMG_SHOCK);
			TakeDamage(hitself);
		}
	}


	// BJ: Both paw plus electric in the middle
	if (IsBabyVolt())
	{
		DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_left", false);
		DispatchParticleEffect("voltigore_baby_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_right", false);
		DispatchParticleEffect("voltigore_baby_middle_charge_preshot", PATTACH_POINT_FOLLOW, this, "electric", false);
	}
	else
	{
		DispatchParticleEffect("voltigore_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_left", false);
		DispatchParticleEffect("voltigore_paw_charge_preshot", PATTACH_POINT_FOLLOW, this, "paw_right", false);
		DispatchParticleEffect("voltigore_middle_charge_preshot", PATTACH_POINT_FOLLOW, this, "electric", false);
	}

	if (!IsBabyVolt())	// BJ Only Bigger play voice sound
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "NPC_Voltigore.AttackElectric");
	}

	DoMuzzleFlash();
	m_cAmmoLoaded--;

	SetAim(vecShootDir);
}

void CNPC_voltigore::PreBigExplode(void)
{
	DevMsg("Volt pre big explode event \n");

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "NPC_Voltigore.AttackElectric");
	
	DispatchParticleEffect("voltigore_big_explode_pre", PATTACH_POINT_FOLLOW, this, "electric", false);	// BJ: Pre explosion fx on attachment
}

void CNPC_voltigore::BigExplode(void)
{
	DevMsg("Volt big explode activated \n");

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "NPC_Voltigore.SelfExplode");

	// BJ: ADD GIBS LATER IF NEEDS

	DispatchParticleEffect("voltigore_big_explode_post", GetAbsOrigin(), QAngle(0, 0, 0)); //BJ: Gib explosion in middle of NPC	// def -90 0 0
	RadiusDamage(CTakeDamageInfo(this, this, sk_voltigore_dmg_bigexplode.GetFloat(), DMG_NERVEGAS), GetAbsOrigin(), 300, CLASS_RACEX, NULL);
	UTIL_Remove(this);
}

int CNPC_voltigore::MeleeAttack1Conditions( float flDot, float flDist )	// paw punch attack
{
	if (flDot >= 0.7)
	{

		if (IsBabyVolt())
		{
			if (flDist <= volt_MELEE_ATTACKDIST_BABY)
			{
				return COND_CAN_MELEE_ATTACK1;
			}
		}
		else
		{
			if (flDist <= volt_MELEE_ATTACKDIST)
			{
				return COND_CAN_MELEE_ATTACK1;
			}
		}

	}

	return COND_NONE;
}

int CNPC_voltigore::MeleeAttack2Conditions( float flDot, float flDist )	// electro close combat attacks?
{
	if ( gpGlobals->curtime > m_flameTime_vo )
	{
		if ( flDot >= 0.8 )
		{

			if (IsBabyVolt())
			{
				if (flDist > volt_MELEE_ATTACKDIST_BABY)
				{
	
					if ((flDist <= 100) && (random->RandomInt(0, 27) == 3))	// BJ: Allow to use unique baby shockline very rare
					{
						DevMsg("RAND Volt Baby electro special attack \n");
						return COND_CAN_MELEE_ATTACK2;
					}
				}
			}
			else
			{
				if (flDist > volt_MELEE_ATTACKDIST)
				{

					if ((!cvar->FindVar("sk_voltigore_allow_extra_electric_throw")->GetFloat() == 0) || (g_pGameRules->IsSkillLevel(SKILL_HARD)))	// BJ: Second electro sphere usage in movement.
					{
						if (random->RandomInt(0, 7) == 3)	// BJ: IMPORTANT! Regulate chance to throw electric sphere yes = throw no = ignore, no allow
						{
							DevMsg("Volt Pre sphere \n");
							if (flDist <= volt_ELSPHERE_DIST)
								return COND_CAN_MELEE_ATTACK2;
						}
					}
				}
			}

		}
	}
	
	return COND_NONE;
}

//=========================================================
// CheckRangeAttack1
// flDot is the cos of the angle of the cone within which
// the attack can occur.
//=========================================================
//
// voenergy attack
//
//=========================================================
int CNPC_voltigore::RangeAttack1Conditions( float flDot, float flDist )	// long distance electric sphere throw, rare usage
{
	if ( gpGlobals->curtime > m_seeTime_vo )
	{
		if ( flDot >= 0.7 )
		{

			if (IsBabyVolt())
			{
				if (flDist > volt_MELEE_ATTACKDIST_BABY)
				{
					//return COND_CAN_RANGE_ATTACK1;	// BJ: Useless, not needed for volt baby
				}
			}
			else
			{
				if (flDist > volt_MELEE_ATTACKDIST)
				{
					return COND_CAN_RANGE_ATTACK1;
				}
			}

		}
	}

	return COND_NONE;
}

//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
// Used for many contact-range melee attacks. Bites, claws, etc.

// Overridden for voltigore because his swing starts lower as
// a percentage of his height (otherwise he swings over the
// players head)
//=========================================================
CBaseEntity* CNPC_voltigore::voltigoreCheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	trace_t tr;

	Vector vForward, vUp;
	AngleVectors( GetAbsAngles(), &vForward, NULL, &vUp );

	Vector vecStart = GetAbsOrigin();
	vecStart.z += 64;
	Vector vecEnd = vecStart + ( vForward * flDist) - ( vUp * flDist * 0.3);

	UTIL_TraceEntity( this, GetAbsOrigin(), vecEnd, MASK_SOLID, &tr );
	
	if ( tr.m_pEnt )
	{
		CBaseEntity *pEntity = tr.m_pEnt;

		if ( iDamage > 0 )
		{
			CTakeDamageInfo info( this, this, iDamage, iDmgType );
			CalculateMeleeDamageForce( &info, vForward, tr.endpos );
			pEntity->TakeDamage( info );
		}

		return pEntity;
	}

	return NULL;
}

void CNPC_voltigore::HandleAnimEvent( animevent_t *pEvent )
{
	CPASAttenuationFilter filter( this );

	switch( pEvent->event )
	{
	case volt_AE_SLASH_LEFT:	// BJ: Now left AND both paws attack, merged in QC
		{

			if (IsBabyVolt())	// BJ: Tricky damage info setting
			{
				m_PresetDamage = sk_voltigore_baby_dmg_slash.GetFloat();
			}
			else
			{
				m_PresetDamage = sk_voltigore_dmg_slash.GetFloat();
			}

			if (IsBabyVolt())	// BJ: Tricky re-calc distance for static functions
			{
				m_MeleeDistRecalc = volt_MELEE_ATTACKDIST_BABY;
			}
			else
			{
				m_MeleeDistRecalc = volt_MELEE_ATTACKDIST;
			}

			CBaseEntity *pHurt = voltigoreCheckTraceHullAttack(m_MeleeDistRecalc + 10.0, m_PresetDamage, DMG_SLASH);

			if (pHurt)
			{
				if ( pHurt->GetFlags() & ( FL_NPC | FL_CLIENT ) )
				{

					if (IsBabyVolt())
					{
						if (random->RandomInt(0, 3) == 3)	// more interesting to see
							pHurt->ViewPunch(QAngle(10, 10, -10));
						else
							pHurt->ViewPunch(QAngle(-10, -10, 10));	// left punch
					}
					else
					{
						if (random->RandomInt(0, 3) == 3)
							pHurt->ViewPunch(QAngle(30, 30, -30));
						else
							pHurt->ViewPunch(QAngle(-30, -30, 30));
					}

					Vector vRight;
					AngleVectors( GetAbsAngles(), NULL, &vRight, NULL );
					pHurt->SetAbsVelocity( pHurt->GetAbsVelocity() - vRight * 100 );
				}

				if (IsBabyVolt())
				{
					EmitSound(filter, entindex(), "NPC_Voltigore_Baby.AttackHit");
				}
				else
				{
					EmitSound(filter, entindex(), "NPC_Voltigore.AttackHit");
				}
			}
			else
			{
				if (IsBabyVolt())
				{
					EmitSound(filter, entindex(), "NPC_Voltigore_Baby.AttackMiss");
				}
				else
				{
					EmitSound(filter, entindex(), "NPC_Voltigore.AttackMiss");
				}
			}
		}
		break;

	case volt_AE_RIGHT_FOOT:
	case volt_AE_LEFT_FOOT:

		if (IsBabyVolt())
		{
			EmitSound(filter, entindex(), "NPC_Voltigore_Baby.Footstep");
		}
		else
		{
			UTIL_ScreenShake(GetAbsOrigin(), 4.0, 3.0, 1.0, 1500, SHAKE_START);	// BJ: Moved, only for regular
			EmitSound(filter, entindex(), "NPC_Voltigore.Footstep");
		}
		break;

	case volt_AE_voenergy:	// electric attack event
		if (!IsBabyVolt())	// BJ: Only big one can throw spheres on distance // double check
		{
			DevMsg("Volt volt_AE_voenergy throw electric \n");
			ElectricAttack();
		}
		m_seeTime_vo = gpGlobals->curtime + 12;	//def 12
		break;

	case volt_AE_BREATHE:
		if (IsBabyVolt())
		{
			EmitSound(filter, entindex(), "NPC_Voltigore_Baby.Communicate");
		}
		else
		{
			EmitSound(filter, entindex(), "NPC_Voltigore.Communicate");
		}
		break;

	case volt_AE_EXPLODE:	// BJ: Explode event, defined in QC
		BigExplode();
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

int CNPC_voltigore::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
		case SCHED_MELEE_ATTACK2:
			return SCHED_volt_ELECTRO;
		case SCHED_MELEE_ATTACK1:
			return SCHED_volt_SWIPE;

		case SCHED_CHASE_ENEMY:
			return SCHED_volt_CHASE_ENEMY;
			
		case SCHED_CHASE_ENEMY_FAILED:
			return SCHED_volt_CHASE_ENEMY_FAILED;

		case SCHED_ALERT_STAND:
			return SCHED_CHASE_ENEMY;
			 
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

void CNPC_voltigore::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_ELECTRO_SHOT:

			if (IsBabyVolt())
			{
				DevMsg("Volt TASK_ELECTRO_SHOT throw electric \n");
				ElectricAttack();	// BJ: 1 function 2 different attacks for regular and baby HERE. Baby attack instant, Bigger with delay.

				m_flWaitFinished = gpGlobals->curtime + 1;	//pTask->flTaskData;
				m_flameTime_vo = gpGlobals->curtime + 1;	//def 6	//test
			}
			else
			{
				if ((!cvar->FindVar("sk_voltigore_allow_extra_electric_throw")->GetFloat() == 0) || (g_pGameRules->IsSkillLevel(SKILL_HARD)))
				{
					m_flWaitFinished = gpGlobals->curtime + 0.8;	// BJ Bigger volt delay on hard
					m_flameTime_vo = gpGlobals->curtime + 0.8;
					// attack defined under!
				}
				else
				{
					m_flWaitFinished = gpGlobals->curtime + 0;	// BJ Bigger volt 0 delay
					m_flameTime_vo = gpGlobals->curtime + 0;
				}

			}

		break;

	case TASK_SOUNDS_ATTACK:

		if ( random->RandomInt(0,100) < 30 )
		{
			CPASAttenuationFilter filter( this );
			if (IsBabyVolt())
			{
				EmitSound(filter, entindex(), "NPC_Voltigore_Baby.Attack");
			}
			else
			{
				EmitSound(filter, entindex(), "NPC_Voltigore.Attack");
			}
		}
			
		TaskComplete();
		break;
	
	case TASK_DIE:
		m_flWaitFinished = gpGlobals->curtime + 1.6;

	default: 
		BaseClass::StartTask( pTask );
		break;
	}
}

bool CNPC_voltigore::ShouldGib( const CTakeDamageInfo &info )
{
	return false;
}

//=========================================================
// RunTask
//=========================================================
void CNPC_voltigore::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_DIE:

		if ( gpGlobals->curtime > m_flWaitFinished )
		{
			m_nRenderFX = kRenderFxExplode;
			SetRenderColor( 255, 0, 0 , 255 );
			StopAnimation();
			SetNextThink( gpGlobals->curtime + 0.15 );
			SetThink( &CBaseEntity::SUB_Remove );
	
			return;
		}
		else
			BaseClass::RunTask( pTask );
		break;

	case TASK_ELECTRO_SHOT:
		if ( gpGlobals->curtime > m_flWaitFinished )
		{

			if ((!cvar->FindVar("sk_voltigore_allow_extra_electric_throw")->GetFloat() == 0) || (g_pGameRules->IsSkillLevel(SKILL_HARD)))		// BJ Non zero OR high difficulity
			{
				
				DevMsg("Volt TASK_ELECTRO_SHOT Extra electric \n");

				if (!IsBabyVolt())	// BJ: Moved Big volt sphere shot just like in original, 2 sec delay
				{
				ElectricAttack();
				}
				
			}

			TaskComplete();
			SetBoneController( 0, 0 );
			SetBoneController( 1, 0 );
		}
		else
		{
			bool cancel = false;

			QAngle angles = QAngle( 0, 0, 0 );

			CBaseEntity *pEnemy = GetEnemy();

			if ( pEnemy )
			{
				Vector org = GetAbsOrigin();
				org.z += 64;
				Vector dir = pEnemy->BodyTarget(org) - org;

				VectorAngles( dir, angles );
				angles.x = -angles.x;
				angles.y -= GetAbsAngles().y;

				if ( dir.Length() > 400 )
					cancel = true;
			}
			if ( fabs(angles.y) > 60 )
				cancel = true;
			
			if ( cancel )
			{
				m_flWaitFinished -= 0.5;
				m_flameTime_vo -= 0.5;
			}

		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Don't become a ragdoll until we've finished our death anim
//-----------------------------------------------------------------------------
bool CNPC_voltigore::CanBecomeRagdoll()
{
	//return false;	// wtf? This broke up Baby volt ragdolls

	if (IsBabyVolt())	// BJ: volt baby don't use special anims, original ragdoll is fine
		return true;

	if (iNumDeathAnim == 0)
	{
		if (GetActivity() == ACT_RUN)
			iNumDeathAnim = 1;

		if (GetActivity() == ACT_WALK)
			iNumDeathAnim = 2;
	}

	DevMsg("Volt become ragdoll count \n");
	iCountTillExplode = iCountTillExplode + 1;	// BJ: Should be 3. Tricky method

	if (iCountTillExplode == 3)			// BJ: Next second after death event
		PreBigExplode();

	return IsCurSchedule(SCHED_DIE, false); // BJ: Only die event, no need touch other
}


//-----------------------------------------------------------------------------
// Determines the best type of death anim to play based on how we died.
//-----------------------------------------------------------------------------
Activity CNPC_voltigore::GetDeathActivity()
{

	if (iNumDeathAnim == 1)	// BJ: Simple system that should looks like in HL1
	{
		DevMsg("Volt set death activity FWD \n");
		return ACT_DIEFORWARD;
	}

	if (iNumDeathAnim == 2)
	{
		DevMsg("Volt set death activity BCWRD \n");
		return ACT_DIEBACKWARD;
	}

	DevMsg("Volt set death activity ORIG\n");
	return ACT_DIESIMPLE;
}

void CNPC_voltigore::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );
	m_takedamage = DAMAGE_NO;

	// BJ: Bleed added
	m_iBleedingLevel = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Return desired level for the continuous bleeding effect (not the 
//				individual blood spurts you see per bullet hit)
//          Return 0 for don't bleed, 
//				   1 for mild bleeding
//                 2 for severe bleeding
//-----------------------------------------------------------------------------
unsigned char CNPC_voltigore::GetBleedingLevel(void) const	// BJ: Bleed added
{
	if (m_iHealth > (m_iMaxHealth >> 1))
	{	// more 50%
		return 0;
	}
	else if (m_iHealth > (m_iMaxHealth >> 2))
	{	// less 50% more 25%	26-49
		return 1;
	}
	else
	{
		return 2;
	}

	if ((m_fIsBabyVolt == true) && (m_iHealth > (m_iMaxHealth >> 2)))	// BJ: Volt baby options, don't bleed too much
	{
		return 1;
	}

}

void CNPC_voltigore::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo subInfo = info;

	if ( !IsAlive() )
	{
		BaseClass::TraceAttack( subInfo, vecDir, ptr, pAccumulator );
		return;
	}

	if ( subInfo.GetDamageType() & ( volt_DAMAGE | DMG_BLAST ) )
	{
		if ( m_painSoundTime_vo < gpGlobals->curtime )
		{
			CPASAttenuationFilter filter( this );
			if (IsBabyVolt())
			{
				EmitSound(filter, entindex(), "NPC_Voltigore_Baby.Pain");
			}
			else
			{
				EmitSound(filter, entindex(), "NPC_Voltigore.Pain");
			}

			m_painSoundTime_vo = gpGlobals->curtime + random->RandomFloat( 2.5, 4 );
		}
	}

	BaseClass::TraceAttack( subInfo, vecDir, ptr, pAccumulator );
}

int CNPC_voltigore::TakeDamageFromCombineBall(const CTakeDamageInfo &info)
{
	float damage = info.GetDamage();

	if (UTIL_IsAR2CombineBall(info.GetInflictor()))
	{
		damage = 60;	// dmg here!!!
	}

	if (info.GetAttacker() && info.GetAttacker()->IsPlayer())
	{
		damage = g_pGameRules->AdjustPlayerDamageInflicted(damage);
	}

	m_iHealth -= damage;

	return damage;
}

int CNPC_voltigore::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	/*
	if( GetState() == NPC_STATE_SCRIPT )	// No damage in scripted scenes. test.
	{
		
		return 0;
	}
	*/

	CTakeDamageInfo subInfo = info;

	// BJ : NO DISSOLVE, LESS DAMAGE from combine balls
	if (UTIL_IsCombineBall(info.GetInflictor()))
		return TakeDamageFromCombineBall(info);

	float flDamage = subInfo.GetDamage();

	// BJ: volt_DAMAGE is ALLOWED
	if ( IsAlive() )
	{
		if ( !(subInfo.GetDamageType() & volt_DAMAGE) )
		{
			 flDamage *= 0.01;
			 subInfo.SetDamage( flDamage );
		}
		if ( subInfo.GetDamageType() & DMG_BLAST )
		{
			SetCondition( COND_LIGHT_DAMAGE );
		}
	}

	return BaseClass::OnTakeDamage_Alive( subInfo );
}

AI_BEGIN_CUSTOM_NPC( npc_voltigore, CNPC_voltigore )

DECLARE_TASK ( TASK_SOUNDS_ATTACK )
DECLARE_TASK ( TASK_ELECTRO_SHOT )

	//=========================================================
	// > SCHED_volt_ELECTRO
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_volt_ELECTRO,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_SOUNDS_ATTACK			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_MELEE_ATTACK2"
		"		TASK_ELECTRO_SHOT			4.5"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	)

	//=========================================================
	// > SCHED_volt_SWIPE
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_volt_SWIPE,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_MELEE_ATTACK1			0"
		"	"
		"	Interrupts"
		"   COND_CAN_MELEE_ATTACK2"
	)

	DEFINE_SCHEDULE
	(
		SCHED_volt_CHASE_ENEMY,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_volt_CHASE_ENEMY_FAILED"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY					0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
	);

	DEFINE_SCHEDULE
	(
		SCHED_volt_CHASE_ENEMY_FAILED,

		"	Tasks"
		"		TASK_SET_ROUTE_SEARCH_TIME		2"	// 2 seconds to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	180"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
	);

AI_END_CUSTOM_NPC()
