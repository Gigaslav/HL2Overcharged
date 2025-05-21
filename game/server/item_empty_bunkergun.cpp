#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Small health kit. Heals the player when picked up.
//-----------------------------------------------------------------------------
class CE_BG : public CItem
{
public:
	DECLARE_CLASS(CE_BG, CItem);

	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(item_empty_bunkergun, CE_BG);
PRECACHE_REGISTER(item_empty_bunkergun);


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CE_BG::Spawn(void)
{
	Precache();
	SetModel("models/props_combine/w_bunker_gun01.mdl");
	if (cvar->FindVar("oc_particle_other_weapons")->GetInt())
	{
		DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, this, 1, false);
		DispatchParticleEffect("weapon_dust_stream", PATTACH_POINT_FOLLOW, this, 1, false);
	}
	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CE_BG::Precache(void)
{
	PrecacheModel("models/props_combine/w_bunker_gun01.mdl");

	PrecacheScriptSound("HealthKit.Touch");
}
