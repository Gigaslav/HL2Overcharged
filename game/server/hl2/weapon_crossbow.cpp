//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "weapon_crossbow.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "baseviewmodel_shared.h"///
#include "baseviewmodel.h"///
#include "ragdoll_shared.h"
#include "vphysics/constraints.h"
#include "physics_prop_ragdoll.h"
#include "saverestore_utlvector.h"
#ifdef PORTAL
#include "portal_util_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define BOLT_MODEL			"models/crossbow_bolt.mdl"
#define BOLT_MODEL	"models/weapons/w_missile_closed.mdl"

#define BOLT_AIR_VELOCITY	2500
#define BOLT_WATER_VELOCITY	1500

extern ConVar sk_plr_dmg_crossbow;
extern ConVar sk_npc_dmg_crossbow;

/*ConVar oc_crossbow_x("oc_crossbow_x", "0", FCVAR_REPLICATED, "Set to values other than 0 to select a bonus map challenge type.");
ConVar oc_crossbow_y("oc_crossbow_y", "0", FCVAR_REPLICATED, "Set to values other than 0 to select a bonus map challenge type.");
ConVar oc_crossbow_z("oc_crossbow_z", "0", FCVAR_REPLICATED, "Set to values other than 0 to select a bonus map challenge type.");*/
ConVar oc_weapon_crossbow_explobolt("oc_weapon_crossbow_explobolt", "0", FCVAR_ARCHIVE, "Crossbow explode bolts");
ConVar oc_weapon_crossbow_bolt_lifetime("oc_weapon_crossbow_bolt_lifetime", "25", FCVAR_ARCHIVE, "bolt lifespan");

void TE_StickyBolt(IRecipientFilter& filter, float delay, Vector vecDirection, const Vector *origin);

#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CCrossbowBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS(CCrossbowBolt, CBaseCombatCharacter);

public:
	CCrossbowBolt() { };
	~CCrossbowBolt();

	Class_T Classify(void) { return CLASS_NONE; }

public:

	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);
	void ExploBoltTouch(CBaseEntity *pOther);
	void BoltTouch(CBaseEntity *pOther);
	void TryAttachBolt(CBaseEntity *pOther, const Vector &vForward, trace_t &tr, bool isNpc);
	bool CreateVPhysics(void);
	int VPhysicsGetObjectList(IPhysicsObject **pList, int listMax, ragdoll_t &pRagdoll);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CCrossbowBolt *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL, bool exploBolts = false);
	bool	exploBolts;

protected:
	bool	CreateSprites(void);
	int		m_iStopped;
	CHandle<CSprite>		m_pGlowSprite;
	CUtlVector<IPhysicsConstraint*> m_aConstraints;
	//CHandle<CSpriteTrail>	m_pGlowTrail;
	IPhysicsConstraint* m_oCurrentConstraint;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS(crossbow_bolt, CCrossbowBolt);

BEGIN_DATADESC(CCrossbowBolt)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(ExploBoltTouch),
DEFINE_FUNCTION(BoltTouch),
//DEFINE_UTLVECTOR(m_aConstraints, FIELD_EMBEDDED),
DEFINE_FIELD(m_oCurrentConstraint, FIELD_CLASSPTR),
DEFINE_AUTO_ARRAY(m_aConstraints, FIELD_EHANDLE),
DEFINE_FIELD(exploBolts, FIELD_BOOLEAN),
DEFINE_FIELD(m_iStopped, FIELD_INTEGER),
// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_pGlowSprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CCrossbowBolt, DT_CrossbowBolt)
END_SEND_TABLE()

CCrossbowBolt *CCrossbowBolt::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, bool exploBolts)
{
	// Create a new entity with CCrossbowBolt private data
	CCrossbowBolt *pBolt = (CCrossbowBolt *)CreateEntityByName("crossbow_bolt");
	UTIL_SetOrigin(pBolt, vecOrigin);
	pBolt->SetAbsAngles(angAngles);
	pBolt->Spawn();
	pBolt->SetOwnerEntity(pentOwner);
	pBolt->exploBolts = exploBolts;

	return pBolt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CCrossbowBolt::~CCrossbowBolt(void)
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
bool CCrossbowBolt::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CCrossbowBolt::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CCrossbowBolt::CreateSprites(void)
{
	// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate("sprites/light_glow02_noz.vmt", GetLocalOrigin(), false);

	if (m_pGlowSprite != NULL)
	{
		m_pGlowSprite->FollowEntity(this);
		m_pGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation);
		m_pGlowSprite->SetScale(0.2f);
		m_pGlowSprite->TurnOff();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrossbowBolt::Spawn(void)
{
	Precache();

	SetModel("models/crossbow_bolt.mdl");
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(0.3f, 0.3f, 0.3f), Vector(0.3f, 0.3f, 0.3f));
	SetSolid(SOLID_BBOX);
	SetGravity(0.05f);

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	/*if (exploBolts)
	SetTouch( &CCrossbowBolt::ExploBoltTouch );
	else
	SetTouch( &CCrossbowBolt::BoltTouch );*/

	SetTouch(&CCrossbowBolt::BoltTouch);

	SetThink(&CCrossbowBolt::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	//CreateSprites();

	DispatchParticleEffect("bolt_exhaust", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW;

	m_iStopped = 0;
}


void CCrossbowBolt::Precache(void)
{
	PrecacheModel(BOLT_MODEL);

	// This is used by C_TEStickyBolt, despte being different from above!!!
	PrecacheModel("models/crossbow_bolt_physic.mdl");//("models/Weapons/w_grenade.mdl");

	PrecacheModel("models/crossbow_bolt.mdl");

	PrecacheModel("sprites/light_glow02_noz.vmt");

	PrecacheScriptSound("Weapon_Crossbow.BoltSkewer");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CCrossbowBolt::ExploBoltTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

#if defined(HL2_EPISODIC)
		//!!!HACKHACK - specific hack for ep2_outland_10 to allow crossbow bolts to pass through her bounding box when she's crouched in front of the player
		// (the player thinks they have clear line of sight because Alyx is crouching, but her BBOx is still full-height and blocks crossbow bolts.
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->Classify() == CLASS_PLAYER_ALLY_VITAL && FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
		{
			// Change the owner to stop further collisions with Alyx. We do this by making her the owner.
			// The player won't get credit for this kill but at least the bolt won't magically disappear!
			SetOwnerEntity(pOther);
			return;
		}
#endif//HL2_EPISODIC

		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_NEVERGIB);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

			CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
			if (pPlayer)
			{
				gamestats->Event_WeaponHit(pPlayer, true, "weapon_crossbow", dmgInfo);
			}

			int Rndm = RandomInt(1, 2);
			if (oc_weapon_crossbow_explobolt.GetInt())
			{
				if (Rndm == 1)
					DispatchParticleEffect("Explosion_2", tr.endpos, GetAbsAngles());
				else if (Rndm == 2)
					DispatchParticleEffect("ExplosionCore", tr.endpos, GetAbsAngles());

				DispatchParticleEffect("Blood_explosion_shower", tr.endpos, GetAbsAngles());
				DispatchParticleEffect((cvar->FindVar("oc_fx_particle_crossbowbolt_explosion_effect_name")->GetString()), tr.endpos, GetAbsAngles());	//Новый партиклевый эффект
				//DispatchParticleEffect("explosion_turret_break", tr.endpos, GetAbsAngles());
				RadiusDamage(CTakeDamageInfo(this, NULL, 100, DMG_BLAST), GetAbsOrigin(), 130, CLASS_NONE, NULL);
				EmitSound("Weapon_Crossbow.BoltExplFlesh");
			}
		}
		else
		{
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_BULLET | DMG_NEVERGIB);
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

			int Rndm = RandomInt(1, 2);
			if (oc_weapon_crossbow_explobolt.GetInt())
			{
				if (Rndm == 1)
					DispatchParticleEffect("Explosion_2", tr.endpos, GetAbsAngles());
				else if (Rndm == 2)
					DispatchParticleEffect("ExplosionCore", tr.endpos, GetAbsAngles());

				DispatchParticleEffect((cvar->FindVar("oc_fx_particle_crossbowbolt_explosion_effect_name")->GetString()), tr.endpos, GetAbsAngles());	//Новый партиклевый эффект
				//DispatchParticleEffect("explosion_turret_break", tr.endpos, GetAbsAngles());
				RadiusDamage(CTakeDamageInfo(this, NULL, 100, DMG_BLAST), GetAbsOrigin(), 130, CLASS_NONE, NULL);
				EmitSound("Weapon_Crossbow.BoltExpl");
			}
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		if (!pOther->IsAlive())
		{
			// We killed it! 
			const surfacedata_t *pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);
			if (pdata->game.material == CHAR_TEX_GLASS)
			{
				return;
			}
		}

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		EmitSound("Weapon_Crossbow.BoltHitBody");

		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr2);

		if (tr2.fraction != 1.0f)
		{
			//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
			//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE))
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;

				DispatchEffect("BoltImpact", data);

			}
		}

		SetTouch(NULL);
		SetThink(NULL);

		if (!g_pGameRules->IsMultiplayer())
		{
			UTIL_Remove(this);
		}
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY))
		{
			EmitSound("Weapon_Crossbow.BoltHitWorld");

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			// See if we should reflect off this surface
			float hitDot = DotProduct(tr.plane.normal, -vecDir);

			if ((hitDot < 0.5f) && (speed > 100))
			{
				Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;

				QAngle reflectAngles;

				VectorAngles(vReflection, reflectAngles);

				SetLocalAngles(reflectAngles);

				SetAbsVelocity(vReflection * speed * 0.75f);

				// Start to sink faster
				SetGravity(1.0f);
			}
			else
			{
				SetThink(&CCrossbowBolt::SUB_Remove);
				SetNextThink(gpGlobals->curtime + 2.0f);

				//FIXME: We actually want to stick (with hierarchy) to what we've hit
				SetMoveType(MOVETYPE_NONE);

				Vector vForward;

				AngleVectors(GetAbsAngles(), &vForward);
				VectorNormalize(vForward);

				CEffectData	data;

				data.m_vOrigin = tr.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = 0;

				DispatchEffect("BoltImpact", data);

				UTIL_ImpactTrace(&tr, DMG_BULLET);

				AddEffects(EF_NODRAW);
				SetTouch(NULL);
				SetThink(&CCrossbowBolt::SUB_Remove);
				SetNextThink(gpGlobals->curtime + 2.0f);

				if (m_pGlowSprite != NULL)
				{
					m_pGlowSprite->TurnOn();
					m_pGlowSprite->FadeAndDie(3.0f);
				}

				int Rndm2 = RandomInt(1, 2);
				if (oc_weapon_crossbow_explobolt.GetInt())
				{
					if (Rndm2 == 1)
						DispatchParticleEffect("Explosion_2", tr.endpos, GetAbsAngles());
					else if (Rndm2 == 2)
						DispatchParticleEffect("ExplosionCore", tr.endpos, GetAbsAngles());

					DispatchParticleEffect((cvar->FindVar("oc_fx_particle_crossbowbolt_explosion_effect_name")->GetString()), tr.endpos, GetAbsAngles());	//Новый партиклевый эффект
					DispatchParticleEffect("explosion_turret_break", tr.endpos, GetAbsAngles());
					RadiusDamage(CTakeDamageInfo(this, NULL, 100, DMG_BLAST), GetAbsOrigin(), 130, CLASS_NONE, NULL);
					EmitSound("Weapon_Crossbow.BoltExpl");
				}
			}

			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER)
			{
				g_pEffects->Sparks(GetAbsOrigin());
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ((tr.surface.flags & SURF_SKY) == false)
			{
				UTIL_ImpactTrace(&tr, DMG_BULLET);
			}

			UTIL_Remove(this);
		}
	}


	if (g_pGameRules->IsMultiplayer())
	{
		//		SetThink( &CCrossbowBolt::ExplodeThink );
		//		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

void CCrossbowBolt::BoltTouch(CBaseEntity *pOther)
{
	if (exploBolts)
	{
		ExploBoltTouch(pOther);
		return;
	}
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		trace_t	tr, tr2;
		//tr = BaseClass::GetTouchTrace();

		UTIL_TraceHull(GetAbsOrigin() - vForward * 3, GetAbsOrigin() + vForward * 128, Vector(-5, -5, -5), Vector(5, 5, 5), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		Vector	vecNormalizedVel = GetAbsVelocity();

		bool isNpc = (pOther->IsNPC() && pOther->IsAlive()) || FStrEq(pOther->GetClassname(), "prop_ragdoll") || FStrEq(pOther->GetClassname(), "physics_prop_ragdoll") || FStrEq(pOther->GetClassname(), "prop_ragdoll_attached");

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

#if defined(HL2_EPISODIC)
		//!!!HACKHACK - specific hack for ep2_outland_10 to allow crossbow bolts to pass through her bounding box when she's crouched in front of the player
		// (the player thinks they have clear line of sight because Alyx is crouching, but her BBOx is still full-height and blocks crossbow bolts.
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->Classify() == CLASS_PLAYER_ALLY_VITAL && FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
		{
			// Change the owner to stop further collisions with Alyx. We do this by making her the owner.
			// The player won't get credit for this kill but at least the bolt won't magically disappear!
			SetOwnerEntity(pOther);
			return;
		}
#endif//HL2_EPISODIC

		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_NEVERGIB);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

			CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
			if (pPlayer)
			{
				gamestats->Event_WeaponHit(pPlayer, true, "weapon_crossbow", dmgInfo);
			}

		}
		else
		{
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_BULLET | DMG_NEVERGIB);
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		if (!pOther->IsAlive())
		{
			// We killed it! 
			const surfacedata_t *pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);
			if (pdata->game.material == CHAR_TEX_GLASS)
			{
				return;
			}
		}

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		EmitSound("Weapon_Crossbow.BoltHitBody");

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr2);

		m_iStopped = 0;

		TryAttachBolt(pOther, vForward, tr, isNpc);

		if (!m_iStopped)
		{
			if (tr2.fraction != 1.0f)
			{
				//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
				//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

				if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE))
				{
					CEffectData	data;

					data.m_vOrigin = tr2.endpos;
					data.m_vNormal = vForward;
					data.m_nEntIndex = tr2.fraction != 1.0f;

					DispatchEffect("BoltImpact", data);
				}
			}
		}

		SetTouch(NULL);

		if (!m_iStopped)
		SetThink(NULL);

		if (!g_pGameRules->IsMultiplayer())
		{
			if (!m_iStopped)
			UTIL_Remove(this);
		}
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY))
		{
			EmitSound("Weapon_Crossbow.BoltHitWorld");

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			// See if we should reflect off this surface
			float hitDot = DotProduct(tr.plane.normal, -vecDir);

			if ((hitDot < 0.5f) && (speed > 100))
			{
				Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;

				QAngle reflectAngles;

				VectorAngles(vReflection, reflectAngles);

				SetLocalAngles(reflectAngles);

				SetAbsVelocity(vReflection * speed * 0.75f);

				// Start to sink faster
				SetGravity(1.0f);
			}
			else
			{
				if (!m_iStopped)
				{
					SetThink(&CCrossbowBolt::SUB_Remove);
					SetNextThink(gpGlobals->curtime + 2.0f);
				}
				//FIXME: We actually want to stick (with hierarchy) to what we've hit
				SetMoveType(MOVETYPE_NONE);

				Vector vForward;

				AngleVectors(GetAbsAngles(), &vForward);
				VectorNormalize(vForward);

				CEffectData	data;

				data.m_vOrigin = tr.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = 0;

				DispatchEffect("BoltImpact", data);

				UTIL_ImpactTrace(&tr, DMG_BULLET);

				AddEffects(EF_NODRAW);
				SetTouch(NULL);

				if (!m_iStopped)
				{
					SetThink(&CCrossbowBolt::SUB_Remove);
					SetNextThink(gpGlobals->curtime + 2.0f);
				}

				if (m_pGlowSprite != NULL)
				{
					m_pGlowSprite->TurnOn();
					m_pGlowSprite->FadeAndDie(3.0f);
				}
			}

			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER)
			{
				g_pEffects->Sparks(GetAbsOrigin());
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ((tr.surface.flags & SURF_SKY) == false)
			{
				UTIL_ImpactTrace(&tr, DMG_BULLET);
			}

			if (!m_iStopped)
			UTIL_Remove(this);
		}
	}

	if (g_pGameRules->IsMultiplayer())
	{
		//		SetThink( &CCrossbowBolt::ExplodeThink );
		//		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

void CCrossbowBolt::TryAttachBolt(CBaseEntity *pOther, const Vector &vForward, trace_t &tr, bool isNpc)
{
	if (!cvar->FindVar("oc_ragdoll_serverside")->GetBool())
		return;

	if (!isNpc)
		return;

	bool attach_arrow = false;
	//m_iStopped = 1;

	if (pOther)
	{
		CTraceFilterSimpleList *m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
		m_pIgnoreList->AddEntityToIgnore(this);
		m_pIgnoreList->AddEntityToIgnore(pOther);

		trace_t Check;
		UTIL_TraceLine(GetAbsOrigin() - vForward * 3, GetAbsOrigin() + vForward * 128, MASK_SHOT, m_pIgnoreList, &Check);

		CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>(Check.m_pEnt);
		if (pRagdoll)
			m_pIgnoreList->AddEntityToIgnore(pRagdoll);

		UTIL_TraceLine(GetAbsOrigin() - vForward * 3, GetAbsOrigin() + vForward * 128, MASK_SHOT, m_pIgnoreList, &Check);

		delete m_pIgnoreList;

		if (!Check.DidHit())
			attach_arrow = true;
			//return;
	}

	//this->SUB_StartFadeOut(15, false);

	if (isNpc && !pOther->IsAlive())
	{
		trace_t tr3;

		UTIL_TraceLine(GetAbsOrigin() - vForward * 3, GetAbsOrigin() + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr3);

		bool hit = attach_arrow ? true : tr3.DidHit();

		if (hit)
		{
			UTIL_TraceHull(GetAbsOrigin() - vForward * 3, GetAbsOrigin() + vForward * 128, Vector(-5, -5, -5), Vector(5, 5, 5), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr3);
			//UTIL_TraceLine(GetAbsOrigin(), pOther->GetAbsOrigin(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr3);

			CBaseAnimating *pAnimating = pOther->GetBaseAnimating();
			if (pAnimating && tr3.m_pEnt)
			{
				CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>(tr3.m_pEnt);

				if (!pRagdoll)
					return;

				IPhysicsObject *pBonePhysics = pRagdoll->VPhysicsGetObject();
				if (pRagdoll && pBonePhysics)
				{			
					IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
					//VPhysicsGetObjectList(pList, ARRAYSIZE(pList), *pRagdoll->GetRagdoll());
					pRagdoll->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
					pBonePhysics = pList[tr3.physicsbone];
					if (pBonePhysics != NULL)
					{
						/*float mass = pBonePhysics->GetMass();
						if (mass >= 70)
						{
							DevMsg("Bolt: bone mass is too big to attach \n");
							return;
						}*/

						Vector bonePos;
						QAngle boneAng;
						pRagdoll->GetBonePosition(tr3.physicsbone, bonePos, boneAng);
						
						float minBound = /*pBonePhysics != nullptr ? pBonePhysics->GetSphereRadius() : */pOther->BoundingRadius();

						bool isSmall = minBound <= 25.f;
						
						float mult = 0.5f;

						switch (tr3.hitgroup)
						{
							//Do normal ragdoll stuff if no specific hitgroup was hit.
							case HITGROUP_GENERIC:

								break;

							default:
							{

							}

							case HITGROUP_HEAD:
							{
								mult = isSmall ? 2.f : 0.17f;
								break;
							}
							case HITGROUP_LEFTARM:
								mult = isSmall ? 2.f : 0.17f;
								break;

							case HITGROUP_RIGHTARM:
								mult = isSmall ? 2.f : 0.17f;
								break;

							case HITGROUP_CHEST:
								mult = isSmall ? 2.f : 0.17f;
								break;

							case HITGROUP_STOMACH:
								mult = isSmall ? 2.f : 0.17f;
								break;

							case HITGROUP_LEFTLEG:
								mult = isSmall ? 2.f : 0.17f;
								break;

							case HITGROUP_RIGHTLEG:
								mult = isSmall ? 2.f : 0.17f;
								break;
						}

						float minVal = minBound * mult;

						minVal = Clamp(minVal, 5.f, 200.f);

						//CHAR_FLESH STOMACH HEAD LEG

						//trace_t rt2;
						trace_t trEnd;

						Vector vForward;
						AngleVectors(GetAbsAngles(), &vForward);
						VectorNormalize(vForward);

						CTraceFilterSimpleList *m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
						m_pIgnoreList->AddEntityToIgnore(pRagdoll);
						m_pIgnoreList->AddEntityToIgnore(this);
						UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * MAX_TRACE_LENGTH, MASK_SHOT, m_pIgnoreList, &trEnd);
						delete m_pIgnoreList;
						m_pIgnoreList = NULL;

						/*UTIL_TraceLine(GetAbsOrigin() + vForward * 30, GetAbsOrigin() + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &rt2);

						int i = 0;
						while ((!rt2.m_pEnt || (rt2.m_pEnt && rt2.m_pEnt == pRagdoll)))
						{
							UTIL_TraceLine(GetAbsOrigin() + vForward * i, GetAbsOrigin() + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &rt2);

							if (i >= 100)
								break;
							i++;
						}*/

						if (trEnd.m_pEnt)
						{
							m_iStopped = 1;

							if (m_pGlowSprite != NULL)
							{
								m_pGlowSprite->FollowEntity(NULL);
								m_pGlowSprite->SetScale(0.0f);
								m_pGlowSprite->TurnOff();
								m_pGlowSprite->FadeAndDie(0.1f);
								m_pGlowSprite->Remove();
							}

							trace_t tr2;
							CTraceFilterSimpleList *m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
							m_pIgnoreList->AddEntityToIgnore(pRagdoll);
							m_pIgnoreList->AddEntityToIgnore(this);
							UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * MAX_TRACE_LENGTH, MASK_SHOT, m_pIgnoreList, &tr2);
							delete m_pIgnoreList;
							m_pIgnoreList = NULL;
							//float minVal = isSmall ? minBound : minBound * 0.3f;// +(rt2.DidHitWorld() ? 10.f : 15.f);

							//Vector endPos = tr.endpos - vForward * minVal;
							Vector endPos = tr2.endpos - vForward * minVal;
							//NDebugOverlay::Line(tr2.endpos, tr2.endpos - vForward * minVal, 255, 0, 0, true, 10.0f);
							//NDebugOverlay::Line(tr.endpos, rt2.endpos - vForward * minVal, 0, 255, 0, true, 10.0f);
							//NDebugOverlay::Line(rt2.endpos, rt2.endpos + vForward * -minVal, 0, 0, 255, true, 10.0f);
							//NDebugOverlay::Line(rt2.endpos, rt2.startpos, 255, 255, 255, true, 10.0f);

							if (!attach_arrow)
							{
								this->SetAbsOrigin(endPos);

								pBonePhysics->SetPosition(endPos, boneAng, true);

								CharacterEmitSound("Weapon_Crossbow.BoltSkewer", endPos, CHAN_WEAPON);
							}

							if (this)
							StopParticleEffects(this);

							SetModel("models/crossbow_bolt_physic.mdl");//("models/crossbow_bolt_physic.mdl");

							this->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
							//SetModelScale(0.f);

							attach_arrow = attach_arrow ? true : pRagdoll->VPhysicsGetObject()->GetMass() >= 100;

							if (trEnd.DidHitWorld() && !attach_arrow)
							{
								this->SetMoveType(MOVETYPE_NONE);
								this->SetSolid(SOLID_VPHYSICS);
								this->VPhysicsInitStatic();

								IPhysicsObject *pArrowPhysics = VPhysicsGetObject();
								if (pArrowPhysics)
								{
									IPhysicsConstraint *m_pConstraint;

									constraint_fixedparams_t constraintParams;
									constraintParams.Defaults();
									constraintParams.InitWithCurrentObjectState(pBonePhysics, pArrowPhysics);
									constraintParams.constraint.Defaults();
									constraintParams.constraint.forceLimit = ImpulseScale(pBonePhysics->GetMass(), 20);
									constraintParams.constraint.torqueLimit = ImpulseScale(pBonePhysics->GetMass(), 80);
									m_pConstraint = physenv->CreateFixedConstraint(pBonePhysics, pArrowPhysics, NULL, constraintParams);
									m_pConstraint->SetGameData((void *)this);

									m_aConstraints.AddToTail(m_pConstraint);

									m_oCurrentConstraint = m_pConstraint;
								}
							}
							else
							{
								//this->SetParent(rt2.m_pEnt);
								this->SetMoveType(MOVETYPE_NONE);
								this->SetSolid(SOLID_VPHYSICS);
								this->CreateVPhysics();
								AddSolidFlags(FSOLID_NOT_STANDABLE);
								//this->SetParent(pRagdoll);

								IPhysicsObject *pArrowPhysics = VPhysicsGetObject();
								if (pArrowPhysics)
								{
									IPhysicsConstraint *m_pConstraint;

									constraint_fixedparams_t constraintParams;
									constraintParams.Defaults();
									constraintParams.InitWithCurrentObjectState(pArrowPhysics, pBonePhysics);
									constraintParams.constraint.Defaults();
									constraintParams.constraint.forceLimit = ImpulseScale(pBonePhysics->GetMass(), 20);
									constraintParams.constraint.torqueLimit = ImpulseScale(pBonePhysics->GetMass(), 80);
									m_pConstraint = physenv->CreateFixedConstraint(pArrowPhysics, pBonePhysics, NULL, constraintParams);
									m_pConstraint->SetGameData((void *)this);

									m_aConstraints.AddToTail(m_pConstraint);

									m_oCurrentConstraint = m_pConstraint;
								}
							}
						}
					}
				}
			}
		}
	}
}

int CCrossbowBolt::VPhysicsGetObjectList(IPhysicsObject **pList, int listMax, ragdoll_t &pRagdoll)
{

	for (int i = 0; i < pRagdoll.listCount; i++)
	{
		if (i < listMax)
		{
			pList[i] = pRagdoll.list[i].pObject;
		}
	}

	return pRagdoll.listCount;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrossbowBolt::BubbleThink(void)
{
	if (m_iStopped == 2)
	{
		UTIL_Remove(this);
		return;
	}

	if (m_iStopped == 1)
	{
		SetNextThink(gpGlobals->curtime + oc_weapon_crossbow_bolt_lifetime.GetInt());
		m_iStopped = 2;
	}
	else
	{
		QAngle angNewAngles;

		VectorAngles(GetAbsVelocity(), angNewAngles);
		SetAbsAngles(angNewAngles);

		SetNextThink(gpGlobals->curtime + 0.1f);
	}

	// Make danger sounds out in front of me, to scare snipers back into their hole
	CSoundEnt::InsertSound(SOUND_DANGER_SNIPERONLY, GetAbsOrigin() + GetAbsVelocity() * 0.2, 120.0f, 0.5f, this, SOUNDENT_CHANNEL_REPEATED_DANGER);

	if (GetWaterLevel() == 0)
		return;

	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5);
}




LINK_ENTITY_TO_CLASS(weapon_crossbow, CWeaponCrossbow);

PRECACHE_WEAPON_REGISTER(weapon_crossbow);

IMPLEMENT_SERVERCLASS_ST(CWeaponCrossbow, DT_WeaponCrossbow)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponCrossbow)

//DEFINE_FIELD( m_bInZoom,		FIELD_BOOLEAN ),
DEFINE_FIELD(exploBolts, FIELD_BOOLEAN),
DEFINE_FIELD(m_bMustReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_nChargeState, FIELD_INTEGER),
DEFINE_FIELD(m_hChargerSprite, FIELD_EHANDLE),

END_DATADESC()

acttable_t	CWeaponCrossbow::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_CROSSBOW, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_CROSSBOW, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_CROSSBOW, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_CROSSBOW, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_CROSSBOW, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

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
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
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

IMPLEMENT_ACTTABLE(CWeaponCrossbow);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponCrossbow::CWeaponCrossbow(void)
{
	m_bReloadsSingly = true;
	//m_bInZoom			= false;
	m_bMustReload = false;
	Once = false;
	exploBolts = false;
}

#define	CROSSBOW_GLOW_SPRITE	"sprites/light_glow02_noz.vmt"
#define	CROSSBOW_GLOW_SPRITE2	"sprites/blueflare1.vmt"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::Precache(void)
{
	UTIL_PrecacheOther("crossbow_bolt");

	PrecacheScriptSound("Weapon_Crossbow.BoltHitBody");
	PrecacheScriptSound("Weapon_Crossbow.BoltHitWorld");
	PrecacheScriptSound("Weapon_Crossbow.BoltSkewer");
	PrecacheScriptSound("Weapon_Crossbow.BoltExpl");
	PrecacheScriptSound("Weapon_Crossbow.BoltExplFlesh");
	PrecacheScriptSound("Weapon_SniperRifle.Draw");
	PrecacheModel(CROSSBOW_GLOW_SPRITE);
	PrecacheModel(CROSSBOW_GLOW_SPRITE2);

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCrossbow::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if ( /*m_bInZoom*/IsScopeSighted() && g_pGameRules->IsMultiplayer())
	{
		//		FireSniperBolt();
		FireBolt();
	}
	else
	{
		FireBolt();
	}

	// Signal a reload
	m_bMustReload = true;

	SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration(ACT_VM_PRIMARYATTACK));


	if (pPlayer)
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCrossbow::SecondaryAttack(void)
{
	SendWeaponAnim(GetWpnData().animData[m_bFireMode].WeaponSwitchAnim);
	//SendWeaponAnim(ACT_VM_RELOAD);

	if (!exploBolts)
		exploBolts = true;
	else
		exploBolts = false;

	m_flNextSwitchTime = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();


	if (IsScopeSighted())//(m_bInZoom)
		ToggleZoom(ToBasePlayer(GetOwner()), false);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponCrossbow::Reload(void)
{
	if (BaseClass::Reload())
	{
		m_bMustReload = false;
		return true;

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return false;

		/*if (pPlayer->SetFOV(this, 0, 0.2f))
		{
		m_bInZoom = false;
		// Send a message to hide the scope
		//pPlayer->SetFOV(this, 0, 0.1f);
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "ShowScope");
		WRITE_BYTE(0);
		MessageEnd();
		cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
		cvar->FindVar("oc_state_InSecondFire_crossbow")->SetValue(0);
		}*/
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponCrossbow::ItemBusyFrame( void )
{
// Allow zoom toggling even when we're reloading
CheckZoomToggle();
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::ItemPostFrame(void)
{
	/*if (IsNearWall() || GetOwnerIsRunning())
	{


	if (cvar->FindVar("oc_state_InSecondFire")->GetInt() && !Once)
	{
	ToggleZoom();
	Once = true;
	}
	}
	else
	Once = false;
	// Allow zoom toggling
	CheckZoomToggle();*/

	if (m_bMustReload && HasWeaponIdleTimeElapsed())
	{
		Reload();
	}

	BaseClass::ItemPostFrame();
}

/*class CTraceFilterSkipPlayerAndViewModel : public CTraceFilter
{
public:
virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
// Test against the vehicle too?
// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
CBaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);
if (!pEntity)
return true;

if ((dynamic_cast<CBaseViewModel *>(pEntity) != NULL) ||
(dynamic_cast<CBasePlayer *>(pEntity) != NULL) ||
pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
{
return false;
}

return true;
}
};*/
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::FireBolt(void)
{
	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();

			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_crossbow")->SetValue(0);
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	pOwner->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAG_RESTART);

	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecSrc = pOwner->Weapon_ShootPosition();

	QAngle angAiming, WeapAngle;
	VectorAngles(vecAiming, angAiming);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	//VectorScale(vecThrow, 1000.0f, vecThrow);
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	vThrowPos = GetClientTracerVector();
	/*vThrowPos.x = cvar->FindVar("oc_muzzle_tracer_x")->GetFloat();
	vThrowPos.y = cvar->FindVar("oc_muzzle_tracer_y")->GetFloat();
	vThrowPos.z = cvar->FindVar("oc_muzzle_tracer_z")->GetFloat();*/
	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), vThrowPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	WeapAngle = GetClientMuzzleAngles();
	/*WeapAngle.x = cvar->FindVar("oc_muzzle_angle_x")->GetFloat();
	WeapAngle.y = cvar->FindVar("oc_muzzle_angle_x")->GetFloat();
	WeapAngle.z = cvar->FindVar("oc_muzzle_angle_x")->GetFloat();*/

	Vector Muzzle = pOwner->Weapon_ShootPosition(1);

#if defined(HL2_EPISODIC)
	// !!!HACK - the other piece of the Alyx crossbow bolt hack for Outland_10 (see ::BoltTouch() for more detail)
	if (FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
	{
		trace_t tr;
		UTIL_TraceLine(tr.endpos, vecSrc + vecAiming * 24.0f, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt != NULL && tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL)
		{
			// If Alyx is right in front of the player, make sure the bolt starts outside of the player's BBOX, or the bolt
			// will instantly collide with the player after the owner of the bolt is switched to Alyx in ::BoltTouch(). We 
			// avoid this altogether by making it impossible for the bolt to collide with the player.
			vecSrc += vecAiming * 24.0f;
		}
	}
#endif

	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate(tr.endpos, angAiming, pOwner, exploBolts);

	pBolt->exploBolts = exploBolts;

	pBolt->SetLocalOrigin(tr.endpos);
	pBolt->SetLocalVelocity(tr.endpos);

	if (pOwner->GetWaterLevel() == 3)
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_WATER_VELOCITY);
	}
	else
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}

	m_iClip1--;

	float X = -2.f;
	if (abs(pOwner->LocalEyeAngles().x) + abs(X) >= 89.0f)
	{
		X = X < 0 ? (89.0f - abs(pOwner->LocalEyeAngles().x)) * -1.f : 89.0f - abs(pOwner->LocalEyeAngles().x);
	}
	pOwner->ViewPunch(QAngle(X, 0, 0));

	WeaponSound(SINGLE);
	WeaponSound(SPECIAL2);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 200, 0.2);

	SendWeaponAnim(GetPrimaryAttackActivity());

	/*if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}*/

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;

	DoLoadEffect();
	SetChargerState(CHARGER_STATE_DISCHARGE);
}

void CWeaponCrossbow::Activate(void)
{
	BaseClass::Activate();

	SetSkin(BOLT_SKIN_GLOW);
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponCrossbow::Deploy(void)
{
	/*SetBodygroup(0,0);
	SetBodygroup(1, 0);
	SetBodygroup(2, 0);
	SetBodygroup(3, 0);
	SetBodygroup(4, 0);
	SetBodygroup(5, 0);
	SetBodygroup(6, 0);
	SetBodygroup(7, 0);
	SetBodygroup(8, 0);*/

	if (m_iClip1 <= 0)
	{
		return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix());
	}

	SetSkin(BOLT_SKIN_GLOW);

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponCrossbow::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopEffects();

	SetSkin(BOLT_SKIN_NORMAL);

	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return false;

	//pPlayer->SetFOV(this, 0, 0.1f);
	CSingleUserRecipientFilter filter(pPlayer);
	UserMessageBegin(filter, "ShowScope");
	WRITE_BYTE(0);
	MessageEnd();
	cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
	cvar->FindVar("oc_state_InSecondFire_crossbow")->SetValue(0);*/

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponCrossbow::ToggleZoom( void )
{
CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

if ( pPlayer == NULL )
return;

if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
return;

if (m_bInReload)
return;

if ( m_bInZoom )
{
if ( pPlayer->SetFOV( this, 0, 0.2f ) )
{
m_bInZoom = false;
// Send a message to hide the scope
CSingleUserRecipientFilter filter(pPlayer);
UserMessageBegin(filter, "ShowScope");
WRITE_BYTE(0);
MessageEnd();
cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
cvar->FindVar("oc_state_InSecondFire_crossbow")->SetValue(0);
}
}
else
{
if ((pPlayer->SetFOV(this, 20, 0.1f)) && (m_iClip1 != 0))
{
m_bInZoom = true;
// Send a message to Show the scope
CSingleUserRecipientFilter filter(pPlayer);
UserMessageBegin(filter, "ShowScope");
WRITE_BYTE(1);
MessageEnd();
cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
cvar->FindVar("oc_state_InSecondFire_crossbow")->SetValue(1);
}
}
}*/

#define	BOLT_TIP_ATTACHMENT	2

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::CreateChargerEffects(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (m_hChargerSprite != NULL)
		return;

	m_hChargerSprite = CSprite::SpriteCreate(CROSSBOW_GLOW_SPRITE, GetAbsOrigin(), false);

	if (m_hChargerSprite)
	{
		m_hChargerSprite->SetAttachment(pOwner->GetViewModel(), BOLT_TIP_ATTACHMENT);
		m_hChargerSprite->SetTransparency(kRenderTransAdd, 255, 128, 0, 255, kRenderFxNoDissipation);
		m_hChargerSprite->SetBrightness(0);
		m_hChargerSprite->SetScale(0.1f);
		m_hChargerSprite->TurnOff();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : skinNum - 
//-----------------------------------------------------------------------------
/*void CWeaponCrossbow::SetSkin(int skinNum)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	pViewModel->m_nSkin = skinNum;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::DoLoadEffect(void)
{
	SetSkin(BOLT_SKIN_GLOW);

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	CEffectData	data;

	data.m_nEntIndex = pViewModel->entindex();
	data.m_nAttachmentIndex = 1;

	DispatchEffect("CrossbowLoad", data);

	CSprite *pBlast = CSprite::SpriteCreate(CROSSBOW_GLOW_SPRITE2, GetAbsOrigin(), false);

	if (pBlast)
	{
		pBlast->SetAttachment(pOwner->GetViewModel(), 1);
		pBlast->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone);
		pBlast->SetBrightness(128);
		pBlast->SetScale(0.2f);
		pBlast->FadeOutFromSpawn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::SetChargerState(ChargerState_t state)
{
	// Make sure we're setup
	CreateChargerEffects();

	// Don't do this twice
	if (state == m_nChargeState)
		return;

	m_nChargeState = state;

	switch (m_nChargeState)
	{
	case CHARGER_STATE_START_LOAD:

		WeaponSound(SPECIAL1);

		// Shoot some sparks and draw a beam between the two outer points
		DoLoadEffect();

		break;

	case CHARGER_STATE_START_CHARGE:
	{
		if (m_hChargerSprite == NULL)
			break;

		m_hChargerSprite->SetBrightness(32, 0.5f);
		m_hChargerSprite->SetScale(0.025f, 0.5f);
		m_hChargerSprite->TurnOn();
	}

	break;

	case CHARGER_STATE_READY:
	{
		// Get fully charged
		if (m_hChargerSprite == NULL)
			break;

		m_hChargerSprite->SetBrightness(80, 1.0f);
		m_hChargerSprite->SetScale(0.1f, 0.5f);
		m_hChargerSprite->TurnOn();
	}

	break;

	case CHARGER_STATE_DISCHARGE:
	{
		SetSkin(BOLT_SKIN_NORMAL);

		if (m_hChargerSprite == NULL)
			break;

		m_hChargerSprite->SetBrightness(0);
		m_hChargerSprite->TurnOff();
	}

	break;

	case CHARGER_STATE_OFF:
	{
		SetSkin(BOLT_SKIN_NORMAL);

		if (m_hChargerSprite == NULL)
			break;

		m_hChargerSprite->SetBrightness(0);
		m_hChargerSprite->TurnOff();
	}
	break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_THROW:
		SetChargerState(CHARGER_STATE_START_LOAD);
		break;

	case EVENT_WEAPON_THROW2:
		SetChargerState(CHARGER_STATE_START_CHARGE);
		break;

	case EVENT_WEAPON_THROW3:
		SetChargerState(CHARGER_STATE_READY);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the desired activity for the weapon and its viewmodel counterpart
// Input  : iActivity - activity to play
//-----------------------------------------------------------------------------
bool CWeaponCrossbow::SendWeaponAnim(int iActivity)
{
	int newActivity = iActivity;

	// The last shot needs a non-loaded activity
	if ((newActivity == ACT_VM_IDLE) && (m_iClip1 <= 0))
	{
		newActivity = ACT_VM_FIDGET;
	}

	//For now, just set the ideal activity and be done with it
	return BaseClass::SendWeaponAnim(newActivity);
}

//-----------------------------------------------------------------------------
// Purpose: Stop all zooming and special effects on the viewmodel
//-----------------------------------------------------------------------------
void CWeaponCrossbow::StopEffects(void)
{
	// Stop zooming
	/*if ( m_bInZoom )
	{
	ToggleZoom();
	}*/

	// Turn off our sprites
	SetChargerState(CHARGER_STATE_OFF);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponCrossbow::Drop( const Vector &vecVelocity )
{
StopEffects();
BaseClass::Drop( vecVelocity );
}*/
