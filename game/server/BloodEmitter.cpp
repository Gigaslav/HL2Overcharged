#include "cbase.h"
#include "BloodEmitter.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"
#include "BloodDripsGreen.h"
#include "hl2_shareddefs.h"
#include "te_effect_dispatch.h"

#include "IEffects.h"

#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1
#define BOLT_MODEL	oc_blood_drip_model.GetString() //"models/ShockRifle.mdl"

ConVar oc_blood_drip_model("oc_blood_drip_model", "models/spitball_small.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");
ConVar oc_blood_drip_maxcount("oc_blood_drip_maxcount", "0", FCVAR_ARCHIVE);

extern short	g_sModelIndexFireball;
extern short	g_sModelIndexWExplosion;
#define	SPRITE	"particles/particle_glow/particle_glow_03.vmt"

#if 0
LINK_ENTITY_TO_CLASS(blood_emitter, CBloodEmitter);

BEGIN_DATADESC(CBloodEmitter)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
//DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_hSpitEffectDD, FIELD_EHANDLE),

END_DATADESC()

//IMPLEMENT_SERVERCLASS_ST(CBloodEmitter, DT_BloodEmitter)
//END_SEND_TABLE()

CBloodEmitter *CBloodEmitter::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CBloodEmitter *pBloodEmitter = (CBloodEmitter *)CreateEntityByName("blood_emitter");
	UTIL_SetOrigin(pBloodEmitter, vecOrigin);
	pBloodEmitter->SetAbsAngles(angAngles);
	pBloodEmitter->Spawn();
	pBloodEmitter->SetOwnerEntity(pentOwner);

	pBloodEmitter->m_iDamage = iDamage;

	return pBloodEmitter;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBloodEmitter::~CBloodEmitter(void)
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CBloodEmitter::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CBloodEmitter::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBloodEmitter::CreateSprites(void)
{
	// Create the dust effect in place
	m_hSpitEffectDD = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffectDD != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffectDD->KeyValue("start_active", "1");
		m_hSpitEffectDD->KeyValue("effect_name", "antlion_spit_trail");
		m_hSpitEffectDD->SetParent(this);
		m_hSpitEffectDD->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffectDD);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffectDD->Activate();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBloodEmitter::Spawn(void)
{
	Precache();
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	//SetSolidFlags(FSOLID_NOT_STANDABLE);
	Vector SizeMin(0.2, 0.2, 0.2), SizeMax(1, 1, 1);

	SetModel(BOLT_MODEL);
	//SetModelScale(random->RandomFloat(0.01, 1), 0.0f);

	UTIL_SetSize(this, SizeMin, SizeMax);

	SetUse(&CBaseGrenade::DetonateUse);
	SetTouch(&CBloodDripsGreen::BloodDripsGreenTouch);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_takedamage = DAMAGE_NO;
	m_iHealth = 1;

	//SetGravity(UTIL_ScaleForGravity(DRIPS_GRAVITY));
	//SetFriction(0.8f);

	//SetCollisionGroup(COLLISION_GROUP_NONE);

	//AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	// We're self-illuminating, so we don't take or give shadows
	//AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	//SetTouch(&CBloodEmitter::BoltTouch);

	SetThink(&CBloodEmitter::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	CreateSprites();

	// Make us glow until we've hit the wall
	//m_nSkin = BOLT_SKIN_GLOW;

	touched = true;
	b_count = 0;
}


void CBloodEmitter::Precache(void)
{
	PrecacheModel(BOLT_MODEL);
	PrecacheParticleSystem("Shockrifle_sparks");
	PrecacheParticleSystem("hunter_flechette_trail");
	PrecacheModel("models/grenade.mdl");
	PrecacheModel("models/spitball_large.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");
	PrecacheModel(SPRITE);
	PrecacheScriptSound("GrenadeSpit.Hit");
	PrecacheParticleSystem("Spore_launcher_acid_glow");
	PrecacheParticleSystem("grenade_spit_player");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
	PrecacheScriptSound("grenade_strooper_instant.Detonate");
	PrecacheScriptSound("Weapon_spore_acid.hit");
	PrecacheScriptSound("Weapon_spore_acid.bounce");
	PrecacheScriptSound("Weapon_spore_acid.impact");
	//BaseClass::Precache();
}

#if 0
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBloodEmitter::BoltTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
		return;

	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	//bool bHitWater = ((pTrace->contents & CONTENTS_WATER) != 0);
	//CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	QAngle vecAngles;
	VectorAngles(tracePlaneNormal, vecAngles);

	touched = true;

	//Detonate();
}

void CBloodEmitter::Detonate(void)
{

	m_takedamage = DAMAGE_NO;

	trace_t tr;
	tr = CBaseEntity::GetTouchTrace();


	for (int i = 0; i < cvar->FindVar("sk_spore_acid_maxdrips")->GetFloat(); i++)
	{
		Vector vecSpawn;
		vecSpawn = this->WorldSpaceCenter();
		/*vecSpawn.x += random->RandomFloat(-0.2, 0.2);
		vecSpawn.y += random->RandomFloat(-12.1, 12.4);*/
		//vecSpawn.z += 40;// random->RandomFloat(-20.1, 20.2);
		Vector vecVelocity;
		QAngle angles;
		angles.x = random->RandomFloat(-360, 360);
		angles.y = random->RandomFloat(-360, 360);
		angles.z = random->RandomFloat(-360, 360);
		AngleVectors(angles, &vecVelocity);

		vecVelocity *= random->RandomFloat(150, 300);
		vecVelocity -= GetAbsVelocity() / 14;


		CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
		//pGrenade->SetAbsOrigin(vecTraceDir);
		pGrenade->SetLocalOrigin(vecSpawn);
		pGrenade->SetAbsAngles(RandomAngle(-360, 360));
		DispatchSpawn(pGrenade);
		pGrenade->SetOwnerEntity(this);
		pGrenade->SetAbsVelocity(vecVelocity);

	}
	//CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );


	RadiusDamage(CTakeDamageInfo(this, this, m_flDamage, DMG_BULLET), GetAbsOrigin(), cvar->FindVar("sk_spore_acid_radius")->GetFloat(), CLASS_NONE, NULL);

	CPASAttenuationFilter filter1(this);
	EmitSound(filter1, entindex(), "Weapon_spore_acid.hit");
	EmitSound(filter1, entindex(), "Weapon_spore_acid.impact");


	float Radius = 110;
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), random->RandomInt(10, 25), random->RandomInt(190, 255), random->RandomInt(25, 55), 3, Radius, random->RandomInt(0.018, 0.23), Radius / 0.5);//OverCharged



	DispatchParticleEffect("grenade_spit", GetAbsOrigin(), GetAbsAngles());
	m_hSpitEffectDD->StopParticleSystem();
	UTIL_Remove(m_hSpitEffectDD);
	StopParticleEffects(this);
	UTIL_Remove(this);
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBloodEmitter::BubbleThink(void)
{
	//QAngle angNewAngles;

	//VectorAngles(GetAbsVelocity(), angNewAngles);
	//SetAbsAngles(angNewAngles);

	if (false)//(touched)
	{
		CreateChildDrips();
		b_count++;

		if (b_count >= oc_blood_drip_maxcount.GetInt())
		{
			touched = false;
			b_count = 0;
			UTIL_Remove(this);
		}
	}

	SetNextThink(gpGlobals->curtime + 4.2f);

	/*if (GetWaterLevel() == 0)
	return;
	//DispatchParticleEffect("Shockrifle_sparks", PATTACH_ABSORIGIN, NULL);

	UTIL_Remove(this);*/	// BriJee OVR : Little trick to get entity killed in water
}

void CBloodEmitter::CreateChildDrips()
{
	Vector vecSpawn;
	vecSpawn = this->WorldSpaceCenter();
	Vector vecVelocity;
	QAngle angles;
	angles.x = random->RandomFloat(-360, 360);
	angles.y = random->RandomFloat(-360, 360);
	angles.z = random->RandomFloat(-360, 360);
	AngleVectors(angles, &vecVelocity);

	vecVelocity *= random->RandomFloat(150, 300);
	vecVelocity -= GetAbsVelocity() / 14;

	CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
	//pGrenade->SetAbsOrigin(vecTraceDir);
	pGrenade->SetModelScale(0.01f);
	pGrenade->SetLocalOrigin(vecSpawn);
	pGrenade->SetAbsAngles(RandomAngle(-360, 360));
	DispatchSpawn(pGrenade);
	pGrenade->SetOwnerEntity(this);
	pGrenade->SetAbsVelocity(vecVelocity);
}
#endif

LINK_ENTITY_TO_CLASS(blood_emitter, CBloodEmitter);

/*BEGIN_DATADESC(CEndPoint)
// Function Pointers
DEFINE_FUNCTION(Spawn),

END_DATADESC()*/

//IMPLEMENT_SERVERCLASS_ST(CEndPoint, DT_EndPoint)
//END_SEND_TABLE()


CBloodEmitter *CBloodEmitter::DispatchImpactSound(const Vector &end)
{
	CBloodEmitter *pPoint = (CBloodEmitter *)(CreateEntityByName("blood_emitter"));
	UTIL_SetOrigin(pPoint, end);
	//pPoint->SetAbsAngles(QAngle(0, 0, 0));
	pPoint->Spawn();
	//pPoint->SetOwnerEntity(pPoint);
	return pPoint;
}
void CBloodEmitter::Precache(void)
{
	//PrecacheModel("models/weapons/v_smg1_mmod.mdl");
	//PrecacheScriptSound("Weapon_Gluon.Hit");
}

/*bool CBloodEmitter::CreateVPhysics(void)
{
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}*/


void CBloodEmitter::Spawn(void)
{
	Precache();
	PrecacheModel("models/weapons/MAGS/w_rif_ak47_mag.mdl");
	SetModel("models/weapons/MAGS/w_rif_ak47_mag.mdl");

	SetMoveType(MOVETYPE_VPHYSICS);

	SetSolid(SOLID_VPHYSICS);

	SetCollisionGroup(COLLISION_GROUP_DEBRIS);

	AddSpawnFlags(SF_PHYSBOX_ASLEEP);
	UTIL_SetSize(this, Vector(-3, -3, -3), Vector(3, 3, 3));

	SetThink(&CBloodEmitter::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	//EmitSound("Weapon_Gluon.Hit");
	CreateVPhysics();
	thisPhysics = VPhysicsGetObject();

	//UTIL_Remove(this);
	touched = true;
	b_count = 0;

}
void CBloodEmitter::BubbleThink(void)
{
	if (touched)
	{
		//CreateChildDrips();
		Vector origin = EyePosition();
		Vector	forward, rt, up;
		QAngle angles = GetLocalAngles();
		//AngleVectors(angles, &forward, &rt, &up);

		GetVectors(&forward, &rt, &up);

		origin += forward;
		origin += rt;
		origin += up;

		forward *= 5.f;

		CEffectData	data;
		data.m_vOrigin = origin;
		data.m_vNormal = origin + forward;
		data.m_vAngles = angles;
		data.m_nAttachmentIndex = composeColor;
		DispatchEffect("BloodDrips", data);

		b_count++;

		if (b_count >= oc_blood_drip_maxcount.GetInt())
		{
			touched = false;
			b_count = 0;
			UTIL_Remove(this);
		}
	}

	SetNextThink(gpGlobals->curtime + timeThink);
}

void CBloodEmitter::CreateChildDrips()
{
	Vector vecVelocity;
	QAngle angles;
	angles.x = random->RandomFloat(-360, 360);
	angles.y = random->RandomFloat(-360, 360);
	angles.z = random->RandomFloat(-360, 360);
	AngleVectors(angles, &vecVelocity);

	vecVelocity *= random->RandomFloat(10, 500);
	vecVelocity -= GetAbsVelocity() / 14;

	direction = direction * 0.9;
	if (composeColor == 0)
	{
		CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDrips");
		//pGrenade->SetAbsOrigin(vecTraceDir);
		pGrenade->SetModelScale(0.01f);
		VectorAngles(GetAbsOrigin(), angle);
		pGrenade->SetAbsOrigin(GetAbsOrigin());
		pGrenade->SetAbsAngles(angle);
		DispatchSpawn(pGrenade);
		//pGrenade->SetParent(this->GetParent());
		//pGrenade->SetOwnerEntity(this->GetParent());
		//pGrenade->ApplyLocalVelocityImpulse(this->GetLocalOrigin()*random->RandomInt(15, 35));
		pGrenade->ApplyLocalVelocityImpulse(direction*random->RandomInt(15, 35));
	}
	else if (composeColor == 1 || composeColor == 2)
	{
		CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
		pGrenade->SetModelScale(0.01f);
		VectorAngles(GetAbsOrigin(), angle);
		pGrenade->SetAbsOrigin(GetAbsOrigin());
		pGrenade->SetAbsAngles(angle);
		DispatchSpawn(pGrenade);
		//pGrenade->SetParent(this->GetParent());
		//pGrenade->SetOwnerEntity(this->GetParent());
		//pGrenade->ApplyLocalVelocityImpulse(this->GetLocalOrigin()*random->RandomInt(15, 35));
		pGrenade->ApplyLocalVelocityImpulse(direction*random->RandomInt(15, 35));
	}
	else if (composeColor >= 3)
	{
		int rndmInt = random->RandomInt(0, 1);
		if (rndmInt)
		{
			CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDrips");
			//pGrenade->SetAbsOrigin(vecTraceDir);
			pGrenade->SetModelScale(0.01f);
			VectorAngles(GetAbsOrigin(), angle);
			pGrenade->SetAbsOrigin(GetAbsOrigin());
			pGrenade->SetAbsAngles(angle);
			DispatchSpawn(pGrenade);
			//pGrenade->SetParent(this->GetParent());
			//pGrenade->SetOwnerEntity(this->GetParent());
			//pGrenade->ApplyLocalVelocityImpulse(this->GetLocalOrigin()*random->RandomInt(15, 35));
			pGrenade->ApplyLocalVelocityImpulse(direction*random->RandomInt(15, 35));
		}
		else
		{
			CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
			pGrenade->SetModelScale(0.01f);
			VectorAngles(GetAbsOrigin(), angle);
			pGrenade->SetAbsOrigin(GetAbsOrigin());
			pGrenade->SetAbsAngles(angle);
			DispatchSpawn(pGrenade);
			//pGrenade->SetParent(this->GetParent());
			//pGrenade->SetOwnerEntity(this->GetParent());
			//pGrenade->ApplyLocalVelocityImpulse(this->GetLocalOrigin()*random->RandomInt(15, 35));
			pGrenade->ApplyLocalVelocityImpulse(direction*random->RandomInt(15, 35));
		}
	}
}