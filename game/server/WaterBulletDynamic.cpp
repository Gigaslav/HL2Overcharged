#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon_shared.h"
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
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "particle_parse.h"
#include "grenade_spit.h"
#include "particle_system.h"
#include "effect_dispatch_data.h"
#include "WaterBulletDynamic.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS(water_bullet_dynamic, CWaterBulletDynamic);

BEGIN_DATADESC(CWaterBulletDynamic)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_Spite, FIELD_EHANDLE),
DEFINE_FIELD(m_Sprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWaterBulletDynamic, DT_WaterBulletDynamic)
SendPropInt(SENDINFO(underWater), 1, SPROP_UNSIGNED),
END_SEND_TABLE()

CWaterBulletDynamic *CWaterBulletDynamic::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBaseEntity *pentOwner)
{
	// Create a new entity with CShockRifleProjectile private data
	CWaterBulletDynamic *pWaterBulletDynamic = (CWaterBulletDynamic *)CreateEntityByName("water_bullet_dynamic");
	UTIL_SetOrigin(pWaterBulletDynamic, vecOrigin);
	pWaterBulletDynamic->SetAbsAngles(angAngles);
	pWaterBulletDynamic->Spawn();
	pWaterBulletDynamic->SetOwnerEntity(pentOwner);

	pWaterBulletDynamic->m_iDamage = iDamage;

	return pWaterBulletDynamic;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWaterBulletDynamic::~CWaterBulletDynamic(void)
{
	if (m_Spite)
	{
		UTIL_Remove(m_Spite);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CWaterBulletDynamic::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*unsigned int CWaterBulletDynamic::PhysicsSolidMaskForEntity() const
{
	//return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_DETAIL);
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWaterBulletDynamic::CreateSprites(void)
{


	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWaterBulletDynamic::Spawn(void)
{
	Precache();
	//m_pIgnoreList = new CTraceFilterSimpleList(COLLISION_GROUP_NONE);
	//m_pIgnoreList->AddEntityToIgnore(this);
	SetModel("models/weapons/w_bullet.mdl");
	
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_CUSTOM);
	SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS | COLLISION_GROUP_PLAYER);
	//SetModelScale(0.1f, 0.0f);
	//AddEffects( EF_NODRAW );	// BriJee OVR : Little trick, nodraw model
	//AddEffects( EF_NOSHADOW );
	SetGravity(200);
	SetTouch(&CWaterBulletDynamic::BoltTouch);

	SetThink(&CWaterBulletDynamic::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.02f);
	underWater = true;

	AbsAngles = GetAbsAngles();
}


void CWaterBulletDynamic::Precache(void)
{
	PrecacheModel("models/weapons/w_bullet.mdl");
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
void CWaterBulletDynamic::BoltTouch(CBaseEntity *pOther)
{
	//if (pOther == this)
		//SetSolid(SOLID_BBOX);

	if (GetOwnerEntity() && (pOther == GetOwnerEntity()))
		return;

	if (AllocPooledString(pOther->GetClassname()) == AllocPooledString("water_bullet_dynamic"))
		return;

	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	trace_t	tr2;
	Vector vForward;
	AngleVectors(GetAbsAngles(), &vForward);
	VectorNormalize(vForward);
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2);


	//tr2 = BaseClass::GetTouchTrace();
	UTIL_ImpactTrace(&tr2, GetDamageType);
	//UTIL_DecalTrace(&tr2, "RedGlowFade");

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);
		flActualForce *= GetBulletSpeedRatio;

		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_NEVERGIB );
			//	dmgInfo(this, GetOwnerEntity(), flActualDamage, DMG_BULLET);
			//dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateBulletDamageForce(&info, AmmoType, m_vecDirShooting, tr.endpos);
			//dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(info, m_vecDirShooting, &tr);
		}
		else
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			//CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), flActualDamage, DMG_BULLET);
			CalculateBulletDamageForce(&info, AmmoType, m_vecDirShooting, tr.endpos);
			//dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(info, m_vecDirShooting, &tr);
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		SetAbsVelocity(Vector(0, 0, 0));

		//EmitSound("Weapon_Shock.Impact");

		//CEffectData data;

		/*data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);*/
		UTIL_ImpactTrace(&tr, GetDamageType);

		SetTouch(NULL);
		SetThink(NULL);
		UTIL_Remove(m_Sprite);
		UTIL_Remove(this);
	}
	else
	{
		//	tr;
		//tr = BaseClass::GetTouchTrace();
		//UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, 0 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

		/*Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);*/
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();
		UTIL_ImpactTrace(&tr, GetDamageType);
		//EmitSound("Weapon_Shock.Impact");

		/*CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;
		g_pEffects->Sparks(data.m_vOrigin);*/
		//UTIL_DecalTrace( &tr, "extinguish" );	// BriJee OVR : Draw the decal
		//UTIL_DecalTrace(&tr, "SmallScorch"); //"Scorch" );
		//UTIL_DecalTrace(&tr, "RedGlowFade");
		//UTIL_ImpactTrace( &tr, DMG_SLASH );
		UTIL_Remove(m_Sprite);
		UTIL_Remove(this);
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWaterBulletDynamic::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.02f);

	/*if (GetAbsAngles() != AbsAngles)
		SetSolid(SOLID_BBOX);*/
	/*if (GetAbsVelocity().x <= 10 && GetAbsVelocity().y <= 10 && GetAbsVelocity().z <= 10)
		UTIL_Remove(this);*/

	if (!underWater)
		return;

	if (GetWaterLevel() != 3)
	{
		SetGravity(600);
		DispatchParticleEffect("shell_exhaust_smoke", PATTACH_ABSORIGIN_FOLLOW, this);
		underWater = false;
	}
		//return;
	//DispatchParticleEffect("Shockrifle_sparks", PATTACH_ABSORIGIN, NULL);
	//UTIL_Remove(m_Sprite);
	//UTIL_Remove(this);	// BriJee OVR : Little trick to get entity killed in water

}
