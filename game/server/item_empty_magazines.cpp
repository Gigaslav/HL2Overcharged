#include "cbase.h"
#include "overcharged/item_empty_magazines.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS(item_empty_mag, CE_MAG);
PRECACHE_REGISTER(item_empty_mag);


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CE_MAG::Spawn(void)
{
	Precache();
	//SetModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");

	//DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, this, 1, false);
	//DispatchParticleEffect("weapon_dust_stream", PATTACH_POINT_FOLLOW, this, 1, false);

	//DispatchParticleEffect("weapon_muzzle_smoke2", GetAbsOrigin(), GetAbsAngles(), this);
	//DispatchParticleEffect("weapon_dust_stream", GetAbsOrigin(), GetAbsAngles(), this);
	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CE_MAG::Precache(void)
{
	//PrecacheModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");

	//PrecacheScriptSound("HealthKit.Touch");

	PrecacheScriptSound("Magazines.Impact");
}


void CE_MAG::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	// filter out ragdoll props hitting other parts of itself too often
	// UNDONE: Store a sound time for this entity (not just this pair of objects)
	// and filter repeats on that?
	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	// Don't make sounds / effects if neither entity is MOVETYPE_VPHYSICS.  The game
	// physics should have done so.
	if (GetMoveType() != MOVETYPE_VPHYSICS && pHitEntity->GetMoveType() != MOVETYPE_VPHYSICS)
		return;

	if (pEvent->deltaCollisionTime < 0.5 && (pHitEntity == this))
		return;

	// don't make noise for hidden/invisible/sky materials
	surfacedata_t *phit = physprops->GetSurfaceData(pEvent->surfaceProps[otherIndex]);
	const surfacedata_t *pprops = physprops->GetSurfaceData(pEvent->surfaceProps[index]);
	if (phit->game.material == 'X' || pprops->game.material == 'X')
		return;

	//int soundIndex = SoundIndex != 0 ? SoundIndex : pEvent->surfaceProps[index];

	if (pHitEntity == this)
	{
		//PhysCollisionSound(this, pEvent->pObjects[index], CHAN_BODY, pEvent->surfaceProps[index], pEvent->surfaceProps[otherIndex], pEvent->deltaCollisionTime, pEvent->collisionSpeed);
	}
	else
	{
		//PhysCollisionSound(this, pEvent->pObjects[index], CHAN_STATIC, soundIndex/*75*/, pEvent->surfaceProps[otherIndex]/*30*/, pEvent->deltaCollisionTime, pEvent->collisionSpeed);
	}

	if (pEvent->deltaCollisionTime >= 0.05f && pEvent->collisionSpeed >= 70.0f)
	{
		float volume = pEvent->collisionSpeed * pEvent->collisionSpeed * (1.0f / (320.0f*320.0f));	// max volume at 320 in/s
		if (volume > 1.0f)
			volume = 1.0f;

		const char *impact = STRING(hitSound);//"Magazines.Impact";//GetShootSound(sound_type);

		EmitSound_t params;
		params.m_pSoundName = impact;
		params.m_flSoundTime = 0.f;
		params.m_pOrigin = &GetAbsOrigin();
		float duration = 0.f;
		params.m_pflSoundDuration = &duration;
		params.m_bWarnOnDirectWaveReference = true;

		//ModifyEmitSoundParams(const_cast< EmitSound_t& >(params));

		CPASAttenuationFilter filter(this);

		EmitSound(filter, entindex(), params);
	}

	PhysCollisionScreenShake(pEvent, index);

#if HL2_EPISODIC
	// episodic does something different for when advisor shields are struck
	if (phit->game.material == 'Z' || pprops->game.material == 'Z')
	{
		PhysCollisionWarpEffect(pEvent, phit);
	}
	else
	{
		PhysCollisionDust(pEvent, phit);
	}
#else
	PhysCollisionDust(pEvent, phit);
#endif
}