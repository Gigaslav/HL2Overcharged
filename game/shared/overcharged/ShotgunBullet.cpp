//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ShotgunBullet.h"
#include "soundent.h"
#include "decals.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "BaseTypes.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_shotgun_bullet_dmg("sk_shotgun_bullet_dmg", "20", FCVAR_NONE, "Total damage done by an individual antlion worker loogie.");
ConVar    oc_ShotgunBulletSpeed("oc_ShotgunBulletSpeed", "3000", FCVAR_NONE, "Total damage done by an individual antlion worker loogie.");

LINK_ENTITY_TO_CLASS(shotgun_bullet, CShotgunBullet);

BEGIN_DATADESC(CShotgunBullet)
// Function Pointers
DEFINE_FIELD(m_Speed, FIELD_FLOAT),
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
DEFINE_FIELD(m_pGlowSprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

//IMPLEMENT_SERVERCLASS_ST(CShotgunBullet, DT_ShotgunBullet)
//END_SEND_TABLE()

CShotgunBullet *CShotgunBullet::BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner)
{

	// Create a new entity with CCrossbowBolt private data
	CShotgunBullet *pBolt = (CShotgunBullet *)CreateEntityByName("shotgun_bullet");

	UTIL_SetOrigin(pBolt, vecOrigin);
	pBolt->SetAbsAngles(angAngles);
	pBolt->Spawn();
	pBolt->SetOwnerEntity(pentOwner);
	return pBolt;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CShotgunBullet::~CShotgunBullet(void)
{
	if (m_pGlowSprite)
	{
		UTIL_Remove(m_pGlowSprite);
	}
	cvar->FindVar("FireAgain")->SetValue(1);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*bool CShotgunBullet::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*unsigned int CShotgunBullet::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CShotgunBullet::CreateSprites(void)
{
	/*QAngle Ang;
	VectorAngles(GetLocalOrigin(), Ang);
	DispatchParticleEffect("Bullet_glow", GetLocalOrigin(), Ang, NULL);*/
	// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate("sprites/light_glow02_noz.vmt", GetLocalOrigin(), false);

	if (m_pGlowSprite != NULL)
	{
		m_pGlowSprite->FollowEntity(this);
		m_pGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation);
		m_pGlowSprite->SetScale(0.01f);
		m_pGlowSprite->TurnOff();
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShotgunBullet::Spawn(void)
{
	Precache();
	//AddEffects(EF_NODRAW);
	SetModel("models/airboatgun.mdl");
	SetMoveType(MOVETYPE_FLY);
	//UTIL_SetSize(this, -Vector(0.3f, 0.3f, 0.3f), Vector(0.3f, 0.3f, 0.3f));
	SetSolid(SOLID_CUSTOM);
	//SetGravity(0.05f);

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch(&CShotgunBullet::BoltTouch);

	SetThink(&CShotgunBullet::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.01f);

	//CreateSprites();
	m_Speed = oc_ShotgunBulletSpeed.GetFloat();

	DispatchParticleEffect("Bullet_exhaust", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма
	
	// Make us glow until we've hit the wall
	//m_nSkin = BULLET_SKIN_GLOW;
}


void CShotgunBullet::Precache(void)
{
	//PrecacheModel(BULLET_MODEL);

	// This is used by C_TEStickyBolt, despte being different from above!!!
	PrecacheModel("models/airboatgun.mdl");
	PrecacheParticleSystem("Bullet_exhaust");
	PrecacheParticleSystem("Bullet_glow");
	PrecacheModel("sprites/light_glow02_noz.vmt");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CShotgunBullet::BoltTouch(CBaseEntity *pOther)
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
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_shotgun_bullet_dmg.GetFloat(), DMG_BULLET | DMG_NEVERGIB);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

			CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
			if (pPlayer)
			{
				//gamestats->Event_WeaponHit(pPlayer, true, "weapon_crossbow", dmgInfo);
			}

		}
		else
		{
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_shotgun_bullet_dmg.GetFloat(), DMG_BULLET | DMG_NEVERGIB);
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
		//EmitSound("Weapon_Crossbow.BoltHitBody");

		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr2);

		if (tr2.fraction != 1.0f)
		{
			//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
			//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			/*if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE))
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;

				DispatchEffect("BoltImpact", data);
			}*/
		}

		SetTouch(NULL);
		SetThink(NULL);

		if (!g_pGameRules->IsMultiplayer())
		{

			/*CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
			//if (pPlayer)
			{
				FireBulletsInfo_t info;
				info.m_vecSrc = pPlayer->Weapon_ShootPosition();

				info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
				Vector vForward, vRight, vUp;
				pPlayer->EyeVectors(&vForward, &vRight, &vUp);
				if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
				{
					info.m_vecSrc += vForward * 0.0f;
					info.m_vecSrc += vRight * 5.5f;
					info.m_vecSrc += vUp * -3.4f;
				}
				else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
				{
					info.m_vecSrc += vForward * 0.0f;
					info.m_vecSrc += vRight * 0.0f;
					info.m_vecSrc += vUp * -3.4f;
				}
				// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
				// especially if the weapon we're firing has a really fast rate of fire.
				info.m_iShots = 0;
				float fireRate = pPlayer->GetActiveWeapon()->GetFireRate();
				while (pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack <= gpGlobals->curtime)
				{
					// MUST call sound before removing a round from the clip of a CMachineGun
					pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack = pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack + fireRate;
					info.m_iShots++;
					if (!fireRate)
						break;
				}

				// Make sure we don't fire more than the amount in the clip
				if (pPlayer->GetActiveWeapon()->UsesClipsForAmmo1())
				{
					info.m_iShots = MIN(info.m_iShots, pPlayer->GetActiveWeapon()->m_iClip1);
					pPlayer->GetActiveWeapon()->m_iClip1 -= info.m_iShots;
				}
				else
				{
					info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(pPlayer->GetActiveWeapon()->m_iPrimaryAmmoType));
					pPlayer->RemoveAmmo(info.m_iShots, pPlayer->GetActiveWeapon()->m_iPrimaryAmmoType);
				}

				info.m_flDistance = MAX_TRACE_LENGTH;
				info.m_iAmmoType = pPlayer->GetActiveWeapon()->m_iPrimaryAmmoType;
				info.m_iTracerFreq = 1;

#if !defined( CLIENT_DLL )
				// Fire the bullets
				info.m_vecSpread = pPlayer->GetAttackSpread(pPlayer->GetActiveWeapon());
#else
				//!!!HACKHACK - what does the client want this function for? 
				info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

				pPlayer->FireBullets(info);
				debugoverlay->AddLineOverlay(info.m_vecSrc, (info.m_vecDirShooting + pPlayer->GetActiveWeapon()->GetBulletSpread()) * 300000, 0, 255, 0, false, 10);
			}*/

			cvar->FindVar("FireAgain")->SetValue(1);
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
			//EmitSound("Weapon_Crossbow.BoltHitWorld");
			DispatchParticleEffect("weapon_dust_stream", PATTACH_ABSORIGIN_FOLLOW, this);
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			// See if we should reflect off this surface
			float hitDot = DotProduct(tr.plane.normal, -vecDir);
			int Posibility = RandomInt(1, 10);
			if ((hitDot < 0.5f) && (speed > 100) && (Posibility == 5))
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
				SetThink(&CShotgunBullet::SUB_Remove);
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

				//DispatchEffect("BoltImpact", data);

				UTIL_ImpactTrace(&tr, DMG_BULLET);

				AddEffects(EF_NODRAW);
				SetTouch(NULL);
				SetThink(&CShotgunBullet::SUB_Remove);
				SetNextThink(gpGlobals->curtime + 2.0f);

				if (m_pGlowSprite != NULL)
				{
					m_pGlowSprite->TurnOn();
					m_pGlowSprite->FadeAndDie(3.0f);
				}
			}

			int g_pef = RandomInt(1,3);
			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER)
			{
				if (g_pef == 1)
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



			cvar->FindVar("FireAgain")->SetValue(1);
			UTIL_Remove(this);
		}
	}

	if (g_pGameRules->IsMultiplayer())
	{
		//		SetThink( &CCrossbowBolt::ExplodeThink );
		//		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CShotgunBullet::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.01f);

	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 200, 255, 255, 3, 80, 0.03, 0);//OverCharged
	// Make danger sounds out in front of me, to scare snipers back into their hole
	CSoundEnt::InsertSound(SOUND_DANGER_SNIPERONLY, GetAbsOrigin() + GetAbsVelocity() * 0.2, 120.0f, 0.5f, this, SOUNDENT_CHANNEL_REPEATED_DANGER);

	if (GetWaterLevel() == 0)
		return;
	cvar->FindVar("FireAgain")->SetValue(0);
	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.01f, GetAbsOrigin(), 5);
	//UTIL_Remove(this);
}
