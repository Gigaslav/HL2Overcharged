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
class CEPerks : public CItem
{
public:
	DECLARE_CLASS(CEPerks, CItem);

	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(item_empty_perk, CEPerks);
PRECACHE_REGISTER(item_empty_perk);


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEPerks::Spawn(void)
{
	Precache();
	//SetModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");

	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEPerks::Precache(void)
{
	//PrecacheModel("models/weapons/MAGS/w_smg_ump45_mag.mdl");

	//PrecacheScriptSound("HealthKit.Touch");
}