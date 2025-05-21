//========= Copyright © 2018, Overcharged =======================================//
//
// Purpose: Custom ammo for new guns.
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// DEFAULT CODE START
//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int ITEM_GiveAmmoCustom(CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n", pszAmmoName);
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = MAX(1.0f, flCount);

	return pPlayer->GiveAmmo(flCount, iAmmoType, bSuppressSound);
}

// DEFAULT CODE END

//	===========
// BriJee: CUSTOM AMMO ENTs
//	===========

// Gauss
#define AMMO_GAUSS_GIVE		50
#define AMMO_GAUSS_MODEL	"models/items/ammo_tau.mdl"

class CItem_GaussAmmo : public CItem
{
public:
	DECLARE_CLASS( CItem_GaussAmmo, CItem );

	void Spawn( void )
	{ 
		Precache();
		SetModel( AMMO_GAUSS_MODEL );
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		engine->PrecacheModel( AMMO_GAUSS_MODEL );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmoCustom(pPlayer, AMMO_GAUSS_GIVE, "GaussEnergy"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};


LINK_ENTITY_TO_CLASS( item_ammo_gauss, CItem_GaussAmmo ); 

class CItem_UraniumRounds : public CItem
{
public:
	DECLARE_CLASS(CItem_UraniumRounds, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/ammo_gauss.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/ammo_gauss.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_GAUSS_BOX, "Uranium"))	// replace ar2 later
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_uranium, CItem_UraniumRounds);

//=== Basic Rifle 7.62 (AK, SG, etc)
class CItem_RifleRounds : public CItem
{
public:
	DECLARE_CLASS(CItem_RifleRounds, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/ammo_rifle.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/ammo_rifle.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_RIFLE, "OICW"))	// replace ar2 later
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_rifle, CItem_RifleRounds);

class CItem_Ak47Rounds : public CItem
{
public:
	DECLARE_CLASS(CItem_Ak47Rounds, CItem);

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/ammo_rifle.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/ammo_rifle.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_AK47, "Ak47"))	// replace ar2 later
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_ak47, CItem_Ak47Rounds);

//=== M249
class CItem_M249Box : public CItem
{
public:
	DECLARE_CLASS(CItem_M249Box, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/MAGS/w_mach_m249_mag.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/MAGS/w_mach_m249_mag.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_M249, "M249"))	// replace ar2 later
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_m249, CItem_M249Box);

class CItem_SniperBox : public CItem
{
public:
	DECLARE_CLASS(CItem_SniperBox, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/boxsniperrounds.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/boxsniperrounds.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_SNIPER, "SniperRound"))	// replace ar2 later
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_sniper, CItem_SniperBox);

class CItem_CombineSniperBox : public CItem
{
public:
	DECLARE_CLASS(CItem_CombineSniperBox, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/boxsniperrounds.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/boxsniperrounds.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_COMBINE_SNIPER, "CombineSniperRound"))	// replace ar2 later
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_combine_sniper, CItem_CombineSniperBox);
//=== Basic Pistol 1
class CItem_BoxPisRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxPisRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/ammo_altpistol.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/ammo_altpistol.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_PISTOL, "Pistol"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_altpistol, CItem_BoxPisRounds);

class CItem_AlyxGunRounds : public CItem
{
public:
	DECLARE_CLASS(CItem_AlyxGunRounds, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/MAGS/w_pist_tec9_mag.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/MAGS/w_pist_tec9_mag.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, SIZE_AMMO_PISTOL, "AlyxGun"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_alyxgun, CItem_AlyxGunRounds);

//=== Ion Rifle
class CItem_BoxIonRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxIonRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/combine_rifle_cartridge01.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/combine_rifle_cartridge01.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmoCustom(pPlayer, /*SIZE_AMMO_357*/ 1, "LaserPistol"))	// Temp 357
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_ion, CItem_BoxIonRounds);

class CItem_HopMine : public CItem
{
public:
	DECLARE_CLASS(CItem_HopMine, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/props_combine/combine_mine01.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/props_combine/combine_mine01.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 1, "HopMine"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_hopmine, CItem_HopMine);

class CItem_Slam : public CItem
{
public:
	DECLARE_CLASS(CItem_Slam, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/overcharged/world/w_slam.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/overcharged/world/w_slam.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 1, "Slam"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_slam, CItem_Slam);

class CItem_HopWire : public CItem
{
public:
	DECLARE_CLASS(CItem_HopWire, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/overcharged/world/w_hopwire_blue.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/overcharged/world/w_hopwire_blue.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 1, "HopWire"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_hopwire, CItem_HopWire);


class CItem_TripWire : public CItem
{
public:
	DECLARE_CLASS(CItem_TripWire, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/overcharged/world/w_tripwire_white.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/overcharged/world/w_tripwire_white.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 1, "TripWire"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_tripwire, CItem_TripWire);

class CItem_Extinguisher : public CItem
{
public:
	DECLARE_CLASS(CItem_Extinguisher, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/overcharged/world/w_fire_extinguisher.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/overcharged/world/w_fire_extinguisher.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 100, "Extinguisher"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_extinguisher, CItem_Extinguisher);


// Snark
#define AMMO_GRSNARK_GIVE		5
#define AMMO_GRSNARK_MODEL	"models/items/ammo_snark.mdl"

class CItem_SnarkGreenAmmo : public CItem
{
public:
	DECLARE_CLASS(CItem_SnarkGreenAmmo, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_GRSNARK_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		engine->PrecacheModel(AMMO_GRSNARK_MODEL);
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, AMMO_GRSNARK_GIVE, "Gravity"))	//temp BJ
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_snark, CItem_SnarkGreenAmmo);

// Immolator
#define AMMO_IMMOAMMO_MODEL	"items/ammo_immolator.mdl"

class CItem_ImmolatorAmmo : public CItem
{
public:
	DECLARE_CLASS(CItem_ImmolatorAmmo, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_IMMOAMMO_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		engine->PrecacheModel(AMMO_IMMOAMMO_MODEL);
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmoCustom(pPlayer, 120, "Uranium"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_immolator, CItem_ImmolatorAmmo);