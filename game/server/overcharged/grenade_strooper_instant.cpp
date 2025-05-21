//========= Overcharged 2019. ============//
//
// Purpose: Shock trooper instant grenade
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "grenade_strooper_instant.h"
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


extern short	g_sModelIndexFireball;
extern short	g_sModelIndexWExplosion;
#define	SPRITE	"particles/particle_glow/particle_glow_03.vmt"
//extern ConVar sk_plr_dmg_instant_grenade;
//ConVar    sk_plr_dmg_instant_grenade("sk_plr_dmg_instant_grenade", "40", FCVAR_NONE);
//ConVar    sk_max_instant_grenade("sk_max_instant_grenade", "10", FCVAR_NONE);

/*ConVar    sk_spore_launcher_sprite_r("sk_spore_launcher_sprite_r", "0");
ConVar    sk_spore_launcher_sprite_g("sk_spore_launcher_sprite_g", "0");
ConVar    sk_spore_launcher_sprite_b("sk_spore_launcher_sprite_b", "0");
ConVar    sk_spore_launcher_sprite_a("sk_spore_launcher_sprite_a", "0");
ConVar    sk_spore_launcher_sprite_scale("sk_spore_launcher_sprite_scale", "0");*/
//extern ConVar sk_max_instant_grenade;
//extern ConVar sk_instant_grenade_radius;

BEGIN_DATADESC(CSporeLauncherBouncedProjectile)
	// SR-BUGBUG: These are borked!!!!
//	float				 m_fSpawnTime;

	// Function pointers
	DEFINE_ENTITYFUNC( grenade_strooper_instantTouch ),
	DEFINE_THINKFUNC(Think),
	DEFINE_FIELD( m_fSpawnTime, FIELD_TIME ),
	DEFINE_FIELD(lifetime, FIELD_FLOAT),
END_DATADESC()

LINK_ENTITY_TO_CLASS(spore_launcher_bounced_projectile, CSporeLauncherBouncedProjectile);

CSporeLauncherBouncedProjectile *CSporeLauncherBouncedProjectile::SporeCreate(const Vector &vecOrigin, const Vector &vecFwd, const QAngle &angAngles, CBaseEntity *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CSporeLauncherBouncedProjectile *Sphere = (CSporeLauncherBouncedProjectile *)CreateEntityByName("spore_launcher_bounced_projectile");

	UTIL_SetOrigin(Sphere, vecOrigin);
	Sphere->SetAbsAngles(angAngles);

	Vector vel = (vecOrigin + vecFwd) * cvar->FindVar("oc_sporegenerator_projectile_impulse")->GetFloat();
	Sphere->SetAbsVelocity(vel);
	Sphere->Spawn();
	Sphere->SetOwnerEntity(pentOwner);

	return Sphere;
}

bool CSporeLauncherBouncedProjectile::CreateSprites(void)
{
	//DispatchParticleEffect("hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	//DispatchParticleEffect("Shockrifle_projectile", PATTACH_ABSORIGIN_FOLLOW, this);



	// Create the dust effect in place
	m_hSpitEffectA = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffectA != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffectA->KeyValue("start_active", "1");
		m_hSpitEffectA->KeyValue("effect_name", "antlion_spit_trail");
		m_hSpitEffectA->SetParent(this);
		m_hSpitEffectA->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffectA);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffectA->Activate();
	}

	/*m_hSpitEffectC = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffectC != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffectC->KeyValue("start_active", "1");
		m_hSpitEffectC->KeyValue("effect_name", "Spore_launcher_acid_glow");
		m_hSpitEffectC->SetParent(this);
		m_hSpitEffectC->SetAbsOrigin(GetAbsOrigin());
		m_hSpitEffectC->SetLocalOrigin(GetAbsOrigin());
		//m_hSpitEffectC->SetLocalOrigin(GetAbsOrigin());
		//DispatchSpawn(m_hSpitEffectC);
		//if (gpGlobals->curtime > 0.5f)
		m_hSpitEffectC->FollowEntity(this, true);
		m_hSpitEffectC->Activate();
	}*/
	if (m_hSpitEffectC == NULL)
	{
		m_hSpitEffectC = CSprite::SpriteCreate(SPRITE, GetAbsOrigin(), false);

		if (m_hSpitEffectC == NULL)
		{
			// We were unable to create the sprite
			Assert(0);
			return false;
		}
		m_hSpitEffectC->SetParent(this);
		m_hSpitEffectC->FollowEntity(this);
		//m_hSpitEffectC->SetAttachment(pOwner->GetViewModel(), LookupAttachment("laser"));
		m_hSpitEffectC->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_hSpitEffectC->SetBrightness(/*sk_spore_launcher_sprite_a.GetInt()*/40, 0.5f);
		m_hSpitEffectC->SetColor(/*sk_spore_launcher_sprite_r.GetInt()*/45, /*sk_spore_launcher_sprite_g.GetInt()*/180, /*sk_spore_launcher_sprite_b.GetInt()*/60);
		m_hSpitEffectC->SetScale(/*sk_spore_launcher_sprite_scale.GetFloat()*/0.5f, 0.5f);
		m_hSpitEffectC->TurnOn();
	}
	else
	{
		m_hSpitEffectC->TurnOn();
		/*m_hSpitEffectC->SetScale(0.25f, 0.25f);
		m_hSpitEffectC->SetBrightness(255);*/
	}

	return true;
}

void CSporeLauncherBouncedProjectile::Spawn(void)
{
	Precache( );
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);//
	SetCollisionGroup(HL2COLLISION_GROUP_SPIT);
	//SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS | COLLISION_GROUP_INTERACTIVE | COLLISION_GROUP_PLAYER | COLLISION_GROUP_NPC);//
	//SetMoveType( MOVETYPE_FLY );
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);//
	AddFlag( FL_GRENADE );
	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
	SetModel("models/spitball_medium.mdl");

	UTIL_SetSize(this, Vector(2, 2, 2), Vector(2, 2, 2));
	//DispatchParticleEffect("grenade_spit_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	CreateSprites();
	//DispatchParticleEffect("Spore_launcher_acid_glow", PATTACH_ABSORIGIN_FOLLOW, this);
	//SetUse( &CBaseGrenade::DetonateUse );
	SetTouch(&CSporeLauncherBouncedProjectile::grenade_strooper_instantTouch);
	SetThink(&CSporeLauncherBouncedProjectile::Think);
	SetNextThink(gpGlobals->curtime);
	
	m_flDamage		= cvar->FindVar("sk_plr_dmg_spore_acid_bounce")->GetFloat();
	m_DmgRadius = cvar->FindVar("sk_spore_acid_radius")->GetFloat();
	m_takedamage	= DAMAGE_YES;
	m_bIsLive		= true;
	m_iHealth		= 1;
	
	SetGravity( UTIL_ScaleForGravity( 350 ) );	// use a lower gravity for grenades to make them easier to see //400
	SetFriction( 0.8 );

	//SetSequence( 0 );


	//m_fSpawnTime	= gpGlobals->curtime;
	startVelocity = GetAbsVelocity();
	velocity = 1.0f;
}


void CSporeLauncherBouncedProjectile::Event_Killed(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType)
{
	Detonate( );
}


void CSporeLauncherBouncedProjectile::grenade_strooper_instantTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
		return;

	if (!pOther->IsSolid())
		return;

	//DevMsg("Touch\n");
	// If I'm live go ahead and blow up
	if (GetOwnerEntity() && pOther == GetOwnerEntity()->GetOwnerEntity())
	{
		StopParticleEffects(this);
		UTIL_Remove(m_hSpitEffectA);
		UTIL_Remove(m_hSpitEffectC);
		UTIL_Remove(this);
		return;
	}

	if (pOther && (pOther->IsNPC() || pOther->IsPlayer()))//(m_bIsLive)
	{
		Detonate();		
	}
	else
	{
		velocity -= 0.1f;

		trace_t tr;
		tr = CBaseEntity::GetTouchTrace();
		Vector abs = GetAbsVelocity() * 4 * velocity;//GetAbsVelocity() * velocity;
		if (VectorLength(startVelocity)*4 > VectorLength(abs))
			SetAbsVelocity(abs);
		else
			SetAbsVelocity(GetAbsVelocity());
		
		//DevMsg("VectorLength absvel: %.2f \n", VectorLength(GetAbsVelocity()));
		if (VectorLength(GetAbsVelocity()) <= 250.f)
		{
			Detonate();
		}

		//DevMsg("y: %.2f \n", GetAbsVelocity().y);
		//DevMsg("z: %.2f \n", GetAbsVelocity().z);
		if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
		{
			// non-world needs smaller decals
			//UTIL_DecalTrace(&tr, "SpitSplat");
			UTIL_DecalTrace(&tr, "YellowBlood");
		}
		else
		{
			//UTIL_DecalTrace(&tr, "SpitSplat");
			UTIL_DecalTrace(&tr, "YellowBlood");
		}

		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Weapon_spore_acid.bounce");
		EmitSound(filter, entindex(), "Weapon_spore_acid.hit");
	}
}

void CSporeLauncherBouncedProjectile::Detonate(void)
{
	if (!m_bIsLive)
	{
		return;
	}
	m_bIsLive		= false;
	m_takedamage	= DAMAGE_NO;	

	/*CPASFilter filter( GetAbsOrigin() );

	te->Explosion( filter, 0.0,
		&GetAbsOrigin(), 
		GetWaterLevel() == 0 ? g_sModelIndexFireball : g_sModelIndexWExplosion,
		(m_flDamage - 50) * .60, 
		15,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		m_flDamage );*/

	trace_t tr;	
	tr = CBaseEntity::GetTouchTrace();

	/*if ( (tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0) )
	{
		// non-world needs smaller decals
		UTIL_DecalTrace( &tr, "SmallScorch");
	}
	else
	{
		UTIL_DecalTrace( &tr, "Scorch" );
	}*/
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
		vecVelocity += GetAbsVelocity();

		CBloodDripsGreen *pGrenade = (CBloodDripsGreen*)CreateEntityByName("BloodDripsGreen");
		//pGrenade->SetAbsOrigin(vecTraceDir);
		pGrenade->SetLocalOrigin(vecSpawn);
		pGrenade->SetAbsAngles(RandomAngle(-360, 360));
		DispatchSpawn(pGrenade);
		pGrenade->SetOwnerEntity(this);
		pGrenade->SetAbsVelocity(vecVelocity);

	}
	//CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
	lifetime = 0.f;

	RadiusDamage(CTakeDamageInfo(this, GetThrower(), m_flDamage, DMG_BULLET), GetAbsOrigin(), cvar->FindVar("sk_spore_acid_radius")->GetFloat(), CLASS_NONE, NULL);

	CPASAttenuationFilter filter2( this );
	EmitSound( filter2, entindex(), "Weapon_spore_acid.impact" );

	/*if ( GetWaterLevel() == 0 )
	{
		int sparkCount = random->RandomInt( 0,3 );
		QAngle angles;
		VectorAngles( tr.plane.normal, angles );

		for ( int i = 0; i < sparkCount; i++ )
			Create( "spark_shower", GetAbsOrigin(), angles, NULL );
	}*/
	float Radius = 110;
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), random->RandomInt(10, 25), random->RandomInt(190, 255), random->RandomInt(25, 55), 3, Radius, random->RandomInt(0.018, 0.23), Radius / 0.5);//OverCharged

	DispatchParticleEffect("grenade_spit", GetAbsOrigin(), GetAbsAngles());
	/*if (GetEnemy()->IsPlayer())
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect("grenade_spit_player", GetAbsOrigin(), GetAbsAngles());
	}
	else
	{
		DispatchParticleEffect("grenade_spit", GetAbsOrigin(), GetAbsAngles());
	}*/
	StopParticleEffects(this);
	//m_hSpitEffectA->StopParticleSystem();
	UTIL_Remove(m_hSpitEffectA);
	UTIL_Remove(m_hSpitEffectC);
	UTIL_Remove( this );
}

void CSporeLauncherBouncedProjectile::Think(void)
{
	//DevMsg("VectorLength absvel: %.2f \n", VectorLength(GetAbsVelocity()));
	//DevMsg("lifetime: %.2f \n", lifetime);
	if (lifetime < cvar->FindVar("sk_spore_acid_lifetime")->GetFloat())
	{
		lifetime++;
	}
	else
	{
		Detonate();
		lifetime = 0;
	}

	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.01f);
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSporeLauncherBouncedProjectile::Precache(void)
{
	BaseClass::Precache();

	PrecacheModel( "models/grenade.mdl" ); 
	PrecacheModel("models/spitball_large.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");
	PrecacheModel(SPRITE);
	PrecacheScriptSound("GrenadeSpit.Hit");
	PrecacheParticleSystem("Spore_launcher_acid_glow");
	PrecacheParticleSystem("grenade_spit_player");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
	PrecacheScriptSound( "grenade_strooper_instant.Detonate" );
	PrecacheScriptSound("Weapon_spore_acid.hit");
	PrecacheScriptSound("Weapon_spore_acid.bounce");
	PrecacheScriptSound("Weapon_spore_acid.impact");
}







#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1
#define BOLT_MODEL	oc_grenade_spore_launcher_model.GetString() //"models/ShockRifle.mdl"

ConVar oc_grenade_spore_launcher_model("oc_grenade_spore_launcher_model", "models/spitball_small.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");
ConVar	sk_plr_dmg_spore_launcher("sk_plr_dmg_spore_launcher", "30");
ConVar	sk_npc_dmg_spore_launcher("sk_npc_dmg_spore_launcher", "30");

LINK_ENTITY_TO_CLASS(spore_launcher_projectile, CSporeLauncherProjectile);

BEGIN_DATADESC(CSporeLauncherProjectile)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_hSpitEffectB, FIELD_EHANDLE),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CSporeLauncherProjectile, DT_SporeLauncherProjectile)
END_SEND_TABLE()

CSporeLauncherProjectile *CSporeLauncherProjectile::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CSporeLauncherProjectile *pSporeLauncherProjectile = (CSporeLauncherProjectile *)CreateEntityByName("spore_launcher_projectile");
	UTIL_SetOrigin(pSporeLauncherProjectile, vecOrigin);
	pSporeLauncherProjectile->SetAbsAngles(angAngles);
	pSporeLauncherProjectile->Spawn();
	pSporeLauncherProjectile->SetOwnerEntity(pentOwner);

	pSporeLauncherProjectile->m_iDamage = iDamage;

	return pSporeLauncherProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSporeLauncherProjectile::~CSporeLauncherProjectile(void)
{
	/*if (m_pGlowSprite)
	{
		UTIL_Remove(m_pGlowSprite);
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSporeLauncherProjectile::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CSporeLauncherProjectile::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSporeLauncherProjectile::CreateSprites(void)
{
	//DispatchParticleEffect("hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	//DispatchParticleEffect("Shockrifle_projectile", PATTACH_ABSORIGIN_FOLLOW, this);



	// Create the dust effect in place
	m_hSpitEffectD = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffectD != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffectD->KeyValue("start_active", "1");
		m_hSpitEffectD->KeyValue("effect_name", "antlion_spit_trail");
		m_hSpitEffectD->SetParent(this);
		m_hSpitEffectD->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffectD);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffectD->Activate();
	}

	/*m_hSpitEffectB = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffectB != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffectB->KeyValue("start_active", "1");
		m_hSpitEffectB->KeyValue("effect_name", "Spore_launcher_acid_glow");
		m_hSpitEffectB->SetParent(this);
		m_hSpitEffectB->SetAbsOrigin(GetAbsOrigin());
		m_hSpitEffectB->SetLocalOrigin(GetAbsOrigin());
		//m_hSpitEffectB->SetLocalOrigin(vec3_origin);
		//DispatchSpawn(m_hSpitEffectB);
		//if (gpGlobals->curtime > 0.5f)
		m_hSpitEffectB->FollowEntity(this, true);
		m_hSpitEffectB->Activate();
	}*/
	if (m_hSpitEffectB == NULL)
	{
		m_hSpitEffectB = CSprite::SpriteCreate(SPRITE, GetAbsOrigin(), false);

		if (m_hSpitEffectB == NULL)
		{
			// We were unable to create the sprite
			Assert(0);
			return false;
		}
		m_hSpitEffectB->SetParent(this);
		m_hSpitEffectB->FollowEntity(this);
		//m_hSpitEffectB->SetAttachment(pOwner->GetViewModel(), LookupAttachment("laser"));
		m_hSpitEffectB->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_hSpitEffectB->SetBrightness(/*sk_spore_launcher_sprite_a.GetInt()*/40, 0.5f);
		m_hSpitEffectB->SetColor(/*sk_spore_launcher_sprite_r.GetInt()*/45, /*sk_spore_launcher_sprite_g.GetInt()*/180, /*sk_spore_launcher_sprite_b.GetInt()*/60);
		m_hSpitEffectB->SetScale(/*sk_spore_launcher_sprite_scale.GetFloat()*/0.5f, 0.5f);
		m_hSpitEffectB->TurnOn();
	}
	else
	{
		m_hSpitEffectB->TurnOn();
		/*m_hSpitEffectB->SetScale(0.25f, 0.25f);
		m_hSpitEffectB->SetBrightness(255);*/
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSporeLauncherProjectile::Spawn(void)
{
	Precache();

	SetModel(BOLT_MODEL);
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_BBOX);
	SetModelScale(1.5f, 0.0f);
	m_flDamage = cvar->FindVar("sk_plr_dmg_spore_acid")->GetFloat();
	UpdateWaterState();
	//DispatchParticleEffect("grenade_spit_trail", PATTACH_ABSORIGIN_FOLLOW, this);

	SetTouch(&CSporeLauncherProjectile::BoltTouch);

	SetThink(&CSporeLauncherProjectile::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	CreateSprites();

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW;
}


void CSporeLauncherProjectile::Precache(void)
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
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CSporeLauncherProjectile::BoltTouch(CBaseEntity *pOther)
{
	if (pOther == GetOwnerEntity())
		return;

	if (Q_strstr(STRING(MAKE_STRING(pOther->GetClassname())), "trigger"))
		return;

	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
		{
			Detonate();
			return;
		}
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
	{
		Detonate();
		return;
	}

	if (pOther == GetOwnerEntity()->GetOwnerEntity())
	{
		m_hSpitEffectD->StopParticleSystem();
		UTIL_Remove(m_hSpitEffectB);
		UTIL_Remove(m_hSpitEffectD);
		StopParticleEffects(this);
		UTIL_Remove(this);
		return;
	}
	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ((pTrace->contents & CONTENTS_WATER) != 0);
	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if (bHitWater)
	{
		// Splash!
		/*CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector(0, 0, 1);
		data.m_flScale = 8.0f;

		DispatchEffect("watersplash", data);*/
		StopParticleEffects(this);
	}
	else
	{
		// Make a splat decal
		UTIL_DecalTrace(pTrace, "SpitSplat");
	}

	// Part normal damage, part poison damage
	float poisonratio = cvar->FindVar("sk_bullsquid_grenade_poison_ratio")->GetFloat();

	// Take direct damage if hit
	// NOTE: assume that pTrace is invalidated from this line forward!
	if (pTraceEnt)
	{
		pTraceEnt->TakeDamage(CTakeDamageInfo(this, this, m_flDamage * (1.0f - poisonratio), DMG_ACID));
		pTraceEnt->TakeDamage(CTakeDamageInfo(this, this, m_flDamage * poisonratio, DMG_POISON));
	}
	float m_DmgRadius = cvar->FindVar("sk_spore_acid_radius")->GetFloat();
	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), m_DmgRadius * 2.0f, 0.5f, this);

	QAngle vecAngles;
	VectorAngles(tracePlaneNormal, vecAngles);

	if (pOther->IsPlayer() || bHitWater)
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect("grenade_spit_player", GetAbsOrigin(), vecAngles);
	}
	else
	{
		DispatchParticleEffect("grenade_spit", GetAbsOrigin(), vecAngles);
	}

	Detonate();
}

void CSporeLauncherProjectile::Detonate(void)
{

	m_takedamage = DAMAGE_NO;

	trace_t tr;
	tr = CBaseEntity::GetTouchTrace();

	/*if ( (tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0) )
	{
	// non-world needs smaller decals
	UTIL_DecalTrace( &tr, "SmallScorch");
	}
	else
	{
	UTIL_DecalTrace( &tr, "Scorch" );
	}*/
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
		vecVelocity -= GetAbsVelocity()/14;


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
	/*if ( GetWaterLevel() == 0 )
	{
	int sparkCount = random->RandomInt( 0,3 );
	QAngle angles;
	VectorAngles( tr.plane.normal, angles );

	for ( int i = 0; i < sparkCount; i++ )
	Create( "spark_shower", GetAbsOrigin(), angles, NULL );
	}*/


	DispatchParticleEffect("grenade_spit", GetAbsOrigin(), GetAbsAngles());
	m_hSpitEffectD->StopParticleSystem();
	UTIL_Remove(m_hSpitEffectB);
	UTIL_Remove(m_hSpitEffectD);
	StopParticleEffects(this);
	UTIL_Remove(this);
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSporeLauncherProjectile::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.02f);

	if (GetAbsVelocity().Length() <= 0)
		Detonate();

	/*if (GetWaterLevel() == 0)
		return;
	//DispatchParticleEffect("Shockrifle_sparks", PATTACH_ABSORIGIN, NULL);

	UTIL_Remove(this);*/	// BriJee OVR : Little trick to get entity killed in water
}

