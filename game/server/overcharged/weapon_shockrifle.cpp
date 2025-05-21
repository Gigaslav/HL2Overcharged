//========= Copyright © 2011-2018. Overcharged ================================//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "sprite.h"
#include "spritetrail.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "particle_parse.h"
#include "grenade_spit.h"
#include "particle_system.h"
#include "effect_dispatch_data.h"

#include "weapon_shockrifle.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar oc_weapon_ShockRifle_model("oc_weapon_ShockRifle_model", "models/spitball_small.mdl", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Shock rifle projectile model.");
// models/weapons/hunter_flechette.mdl
//ConVar aa_wpn_ShockRifle_drop( "aa_wpn_ShockRifle_drop", "0", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Allow drop shockrifle or not." );

#define BOLT_MODEL	oc_weapon_ShockRifle_model.GetString() //"models/ShockRifle.mdl"
#define GLOW_SPRITE	"particles/particle_glow/particle_glow_03.vmt"
#define BOLT_AIR_VELOCITY	1500
//#define BOLT_WATER_VELOCITY	1200
#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1
#define	SHOCK_BEAM_SPRITE		"sprites/laserbeam.vmt"
//extern ConVar sk_plr_dmg_ShockRifle;
//extern ConVar sk_npc_dmg_ShockRifle;
ConVar	sk_plr_dmg_ShockRifle("sk_plr_dmg_ShockRifle", "30");
ConVar	sk_npc_dmg_ShockRifle("sk_npc_dmg_ShockRifle", "30");

/*ConVar    sk_shock_rifle_sprite_r("sk_shock_rifle_sprite_r", "0", FCVAR_CHEAT);
ConVar    sk_shock_rifle_sprite_g("sk_shock_rifle_sprite_g", "0", FCVAR_CHEAT);
ConVar    sk_shock_rifle_sprite_b("sk_shock_rifle_sprite_b", "0", FCVAR_CHEAT);
ConVar    sk_shock_rifle_sprite_a("sk_shock_rifle_sprite_a", "0", FCVAR_CHEAT);
ConVar    sk_shock_rifle_sprite_scale("sk_shock_rifle_sprite_scale", "0", FCVAR_CHEAT);*/

ConVar oc_weapon_shock_rifle_projectile_x("oc_weapon_shock_rifle_projectile_x", "0");
ConVar oc_weapon_shock_rifle_projectile_y("oc_weapon_shock_rifle_projectile_y", "0");
ConVar oc_weapon_shock_rifle_projectile_z("oc_weapon_shock_rifle_projectile_z", "0");
#ifndef CLIENT_DLL

void TE_StickyBolt(IRecipientFilter& filter, float delay, Vector vecDirection, const Vector *origin);

//-----------------------------------------------------------------------------
// ShockRifle
//-----------------------------------------------------------------------------
class CShockRifleProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS(CShockRifleProjectile, CBaseCombatCharacter);

public:
	CShockRifleProjectile() { };
	~CShockRifleProjectile();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);	// changed
	void BoltTouch(CBaseEntity *pOther);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CShockRifleProjectile *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL);

protected:

	bool	CreateSprites(void);
	//CHandle< CParticleSystem >	m_Spite;
	//CHandle<CSprite>		m_Sprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS(ShockRifle_projectile, CShockRifleProjectile);

BEGIN_DATADESC(CShockRifleProjectile)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
//DEFINE_FIELD(m_Spite, FIELD_EHANDLE),
//DEFINE_FIELD(m_Sprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CShockRifleProjectile, DT_ShockRifleProjectile)
END_SEND_TABLE()

CShockRifleProjectile *CShockRifleProjectile::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CShockRifleProjectile *pShockRifleProjectile = (CShockRifleProjectile *)CreateEntityByName("ShockRifle_projectile");
	UTIL_SetOrigin(pShockRifleProjectile, vecOrigin);
	pShockRifleProjectile->SetAbsAngles(angAngles);
	pShockRifleProjectile->Spawn();
	pShockRifleProjectile->SetOwnerEntity(pentOwner);

	pShockRifleProjectile->m_iDamage = iDamage;

	return pShockRifleProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CShockRifleProjectile::~CShockRifleProjectile(void)
{
	/*if (m_Spite)
	{
	UTIL_Remove(m_Spite);
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CShockRifleProjectile::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CShockRifleProjectile::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CShockRifleProjectile::CreateSprites(void)
{
	//DispatchParticleEffect( "hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this );
	DispatchParticleEffect("Shock_rifle_main_ball", PATTACH_ABSORIGIN_FOLLOW, this);
	/*DispatchParticleEffect("Shock_rifle_main_glow", PATTACH_ABSORIGIN_FOLLOW, this);*/

	DispatchParticleEffect("hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	DispatchParticleEffect("Shockrifle_projectile", PATTACH_ABSORIGIN_FOLLOW, this);

	/*m_Spite = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_Spite != NULL)
	{
	// Setup our basic parameters
	m_Spite->KeyValue("start_active", "1");
	m_Spite->KeyValue("effect_name", "Shock_rifle_main_glow");
	m_Spite->SetParent(this);
	m_Spite->SetLocalOrigin(vec3_origin);
	DispatchSpawn(m_Spite);
	if (gpGlobals->curtime > 0.5f)
	m_Spite->Activate();
	}

	if (m_Sprite == NULL)
	{
	m_Sprite = CSprite::SpriteCreate(GLOW_SPRITE, GetAbsOrigin(), false);

	if (m_Sprite == NULL)
	{
	// We were unable to create the sprite
	Assert(0);
	return false;
	}
	m_Sprite->SetParent(this);
	m_Sprite->FollowEntity(this);
	//m_Sprite->SetAttachment(pOwner->GetViewModel(), LookupAttachment("laser"));
	m_Sprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
	m_Sprite->SetBrightness(sk_shock_rifle_sprite_a.GetInt(), 0.5f);
	m_Sprite->SetColor(sk_shock_rifle_sprite_r.GetInt(), sk_shock_rifle_sprite_g.GetInt(), sk_shock_rifle_sprite_b.GetInt());
	m_Sprite->SetScale(sk_shock_rifle_sprite_scale.GetFloat(), 0.5f);
	m_Sprite->TurnOn();
	}
	else
	{
	m_Sprite->TurnOn();
	/*m_Sprite->SetScale(0.25f, 0.25f);
	m_Sprite->SetBrightness(255);*/
	//}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShockRifleProjectile::Spawn(void)
{
	Precache();

	SetModel(BOLT_MODEL);
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_BBOX);
	SetModelScale(0.1f, 0.0f);
	//AddEffects( EF_NODRAW );	// BriJee OVR : Little trick, nodraw model
	//AddEffects( EF_NOSHADOW );

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch(&CShockRifleProjectile::BoltTouch);

	SetThink(&CShockRifleProjectile::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);

	CreateSprites();

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW;
}


void CShockRifleProjectile::Precache(void)
{
	PrecacheModel(BOLT_MODEL);
	PrecacheParticleSystem("Shockrifle_sparks");
	//PrecacheModel( "sprites/light_glow02_noz.vmt" );
	PrecacheParticleSystem("Shockrifle_projectile");
	//PrecacheParticleSystem( "hunter_flechette_trail_striderbuster" );
	PrecacheParticleSystem("hunter_flechette_trail");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CShockRifleProjectile::BoltTouch(CBaseEntity *pOther)
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

		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_ShockRifle.GetFloat(), DMG_SHOCK);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		}
		else
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_ShockRifle.GetFloat(), DMG_NEVERGIB | DMG_SHOCK);
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
		EmitSound("Weapon_Shock.Impact");

		CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);
		/*Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if ( tr2.fraction != 1.0f )
		{
		//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
		//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

		if ( tr2.m_pEnt == NULL || ( tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
		{
		CEffectData	data;

		data.m_vOrigin = tr2.endpos;
		data.m_vNormal = vForward;
		data.m_nEntIndex = tr2.fraction != 1.0f;

		DispatchEffect( "BoltImpact", data );
		}
		}*/

		SetTouch(NULL);
		SetThink(NULL);
		//UTIL_Remove(m_Sprite);
		UTIL_Remove(this);
	}
	else
	{
		trace_t	tr;
		//tr = BaseClass::GetTouchTrace();
		//UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, 0 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

		Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);
		/*
		// Reflect off of anything that isnt the sky
		if (tr.surface.flags & SURF_SKY)
		{
		UTIL_Remove( this );
		}
		else
		{*/
		EmitSound("Weapon_Shock.Impact");

		/*// if what we hit is static architecture, can stay around for a while.
		Vector vecDir = GetAbsVelocity();
		float speed = VectorNormalize( vecDir );

		float hitDot = DotProduct( tr.plane.normal, -vecDir );

		Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;

		QAngle reflectAngles;

		VectorAngles( vReflection, reflectAngles );

		SetLocalAngles( reflectAngles );

		SetAbsVelocity( vReflection * speed * 0.75f );*/

		// Shoot some sparks
		/*if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
		{
		g_pEffects->Sparks( GetAbsOrigin() );
		}*/
		//}
		CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);
		//UTIL_DecalTrace( &tr, "extinguish" );	// BriJee OVR : Draw the decal
		UTIL_DecalTrace(&tr, "SmallScorch"); //"Scorch" );
		//UTIL_DecalTrace(&tr, "RedGlowFade");
		//UTIL_ImpactTrace( &tr, DMG_SLASH );
		//UTIL_Remove(m_Sprite);
		UTIL_Remove(this);
	}


	/*if ( g_pGameRules->IsMultiplayer() )
	{
	//		SetThink( &CShockRifleProjectile::ExplodeThink );
	//		SetNextThink( gpGlobals->curtime + 0.1f );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShockRifleProjectile::BubbleThink(void)
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
}

#endif



LINK_ENTITY_TO_CLASS(weapon_ShockRifle, CWeaponShockRifle);

PRECACHE_WEAPON_REGISTER(weapon_ShockRifle);

IMPLEMENT_SERVERCLASS_ST(CWeaponShockRifle, DT_WeaponShockRifle)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponShockRifle)

DEFINE_FIELD(m_hLaserBeam1, FIELD_EHANDLE),
DEFINE_FIELD(m_hLaserBeam2, FIELD_EHANDLE),
DEFINE_FIELD(m_hLaserBeam3, FIELD_EHANDLE),
DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
DEFINE_FIELD(m_bMustReload, FIELD_BOOLEAN),
DEFINE_FIELD(bFirstPickUp, FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponShockRifle::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, false },

	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },

	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SHOTGUN, true },
	//{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,		true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_WALK, ACT_WALK_RIFLE, false },
};

IMPLEMENT_ACTTABLE(CWeaponShockRifle);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponShockRifle::CWeaponShockRifle(void)
{
	m_bReloadsSingly = true;
	m_bInZoom = false;
	m_bMustReload = false;
	bFirstPickUp = true;
}

CWeaponShockRifle::~CWeaponShockRifle()
{
	if (m_hLaserBeam1)
	{
		UTIL_Remove(m_hLaserBeam1);
	}
	if (m_hLaserBeam2)
	{
		UTIL_Remove(m_hLaserBeam2);
	}
	if (m_hLaserBeam3)
	{
		UTIL_Remove(m_hLaserBeam3);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShockRifle::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("ShockRifle_projectile");
#endif
	PrecacheModel(SHOCK_BEAM_SPRITE);
	PrecacheScriptSound("Weapon_Shock.Draw");
	PrecacheScriptSound("Weapon_Shock.Impact");
	PrecacheScriptSound("Weapon_Crossbow.BoltHitBody");
	PrecacheScriptSound("Weapon_Crossbow.BoltHitWorld");
	PrecacheScriptSound("Weapon_Crossbow.BoltSkewer");
	PrecacheParticleSystem("Shock_rifle_main_ball");
	PrecacheParticleSystem("Shock_rifle_main_glow");
	UTIL_PrecacheOther("grenade_spit");
	PrecacheModel(GLOW_SPRITE);

	BaseClass::Precache();
}

void CWeaponShockRifle::Spawn(void)
{
	BaseClass::Spawn();

	m_takedamage = DAMAGE_YES;
	m_iHealth = 15;
}

void CWeaponShockRifle::Event_Killed(const CTakeDamageInfo &info)
{
	m_takedamage = DAMAGE_NO;

	CBaseEntity::Create("grenade_spit", this->GetAbsOrigin(), QAngle(0, 0, 0), this); // Create spit

	SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponShockRifle::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	RemoveAmmo(GetPrimaryAmmoType(), 1); //pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

	WeaponSound(SINGLE);
	WeaponSound(SPECIAL2);

	SendWeaponAnim(GetPrimaryAttackActivity());

	StartLaserEffects();

	FireBolt();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

	AddViewKick();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponShockRifle::Reload(void)
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShockRifle::FireBolt(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;


#ifndef CLIENT_DLL
	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecMuzzlePos2; Vector vecEndPos2;
	QAngle angDir;
	Vector Muzzle = pOwner->Weapon_ShootPosition(1);

	Muzzle.x += oc_weapon_shock_rifle_projectile_x.GetFloat();
	Muzzle.y += oc_weapon_shock_rifle_projectile_y.GetFloat();
	Muzzle.z += oc_weapon_shock_rifle_projectile_z.GetFloat();

	Vector vThrowPos;
	vThrowPos = GetClientTracerVector();
	/*vThrowPos.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	vThrowPos.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	vThrowPos.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/

	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), vThrowPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	CShockRifleProjectile *pShockRifleProjectile = CShockRifleProjectile::BoltCreate(tr.endpos, angAiming, 0, pOwner); // BriJee OVR : Better start angles

	if (pOwner->GetWaterLevel() == 3)
	{
		CTakeDamageInfo hitself(this, this, 210, DMG_SHOCK);
		pOwner->TakeDamage(hitself);
	}
	else
	{
		pShockRifleProjectile->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}

#endif

}

void CWeaponShockRifle::Drop(const Vector &vecVelocity)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		UTIL_Remove(this);

		Vector vecForward;
		pPlayer->EyeVectors(&vecForward);

		QAngle spawnAng;
		VectorAngles(pPlayer->BodyDirection2D(), spawnAng);

		CBaseEntity *pShockNPC = NULL;
		//pShockNPC = CBaseEntity::Create("npc_shockroach", pPlayer->Weapon_ShootPosition(), pPlayer->GetAbsAngles(), pPlayer); //0 60 0
		pShockNPC = CBaseEntity::Create("npc_shockroach", pPlayer->Weapon_ShootPosition(), spawnAng, pPlayer); //0 60 0


		if (pShockNPC)
		{
			pShockNPC->SetAbsVelocity(vecForward * 320 + pPlayer->GetAbsVelocity());
		}
	}

	StopLaserEffects();
	BaseClass::Drop(vecVelocity);
}


void CWeaponShockRifle::StartLaserEffects(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	CBaseViewModel *pBeamEnt = static_cast<CBaseViewModel *>(pOwner->GetViewModel());

	if (m_hLaserBeam1 == NULL)
	{
		m_hLaserBeam1 = CBeam::BeamCreate(SHOCK_BEAM_SPRITE, 1.5f);

		if (m_hLaserBeam1 == NULL)
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam1->EntsInit(pBeamEnt, pBeamEnt);

		int	startAttachment1 = LookupAttachment("1");
		int endAttachment1 = LookupAttachment("0");

		m_hLaserBeam1->FollowEntity(pBeamEnt);
		m_hLaserBeam1->SetStartAttachment(startAttachment1);
		m_hLaserBeam1->SetEndAttachment(endAttachment1);
		m_hLaserBeam1->SetNoise(30);
		m_hLaserBeam1->SetColor(0, 180, 255);
		m_hLaserBeam1->SetScrollRate(0);
		m_hLaserBeam1->SetWidth(1.5f);
		m_hLaserBeam1->SetEndWidth(0.5f);
		m_hLaserBeam1->SetBrightness(128);
	}
	else
	{
		m_hLaserBeam1->SetBrightness(128);
	}

	if (m_hLaserBeam2 == NULL)
	{
		m_hLaserBeam2 = CBeam::BeamCreate(SHOCK_BEAM_SPRITE, 1.5f);

		if (m_hLaserBeam2 == NULL)
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam2->EntsInit(pBeamEnt, pBeamEnt);

		int	startAttachment2 = LookupAttachment("2");
		int endAttachment2 = LookupAttachment("0");

		m_hLaserBeam2->FollowEntity(pBeamEnt);
		m_hLaserBeam2->SetStartAttachment(startAttachment2);
		m_hLaserBeam2->SetEndAttachment(endAttachment2);
		m_hLaserBeam2->SetNoise(30);
		m_hLaserBeam2->SetColor(0, 180, 255);
		m_hLaserBeam2->SetScrollRate(0);
		m_hLaserBeam2->SetWidth(1.5f);
		m_hLaserBeam2->SetEndWidth(0.5f);
		m_hLaserBeam2->SetBrightness(128);
	}
	else
	{
		m_hLaserBeam2->SetBrightness(128);
	}

	if (m_hLaserBeam3 == NULL)
	{
		m_hLaserBeam3 = CBeam::BeamCreate(SHOCK_BEAM_SPRITE, 1.5f);

		if (m_hLaserBeam3 == NULL)
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam3->EntsInit(pBeamEnt, pBeamEnt);

		int	startAttachment3 = LookupAttachment("3");
		int endAttachment3 = LookupAttachment("0");

		m_hLaserBeam3->FollowEntity(pBeamEnt);
		m_hLaserBeam3->SetStartAttachment(startAttachment3);
		m_hLaserBeam3->SetEndAttachment(endAttachment3);
		m_hLaserBeam3->SetNoise(30);
		m_hLaserBeam3->SetColor(0, 180, 255);
		m_hLaserBeam3->SetScrollRate(0);
		m_hLaserBeam3->SetWidth(1.5f);
		m_hLaserBeam3->SetEndWidth(0.5f);
		m_hLaserBeam3->SetBrightness(128);
	}
	else
	{
		m_hLaserBeam3->SetBrightness(128);
	}

}
void CWeaponShockRifle::StopLaserEffects(void)
{
	if (m_hLaserBeam1 != NULL)
	{
		m_hLaserBeam1->SetBrightness(0);
	}
	if (m_hLaserBeam2 != NULL)
	{
		m_hLaserBeam2->SetBrightness(0);
	}
	if (m_hLaserBeam3 != NULL)
	{
		m_hLaserBeam3->SetBrightness(0);
	}
}

void CWeaponShockRifle::ItemPostFrame(void)
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime)
		StopLaserEffects();

	BaseClass::ItemPostFrame();
}

bool CWeaponShockRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopLaserEffects();

	return BaseClass::Holster(pSwitchingTo);
}