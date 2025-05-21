//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "weapon_citizenpackage.h"

//-----------------------------------------------------------------------------
// Purpose: Old Man Harpoon - Lost Coast.
//-----------------------------------------------------------------------------
class CWeaponOldManHarpoon : public CWeaponCitizenPackage
{
	DECLARE_CLASS( CWeaponOldManHarpoon, CWeaponCitizenPackage );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();	
	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST( CWeaponOldManHarpoon, DT_WeaponOldManHarpoon )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponOldManHarpoon )
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_oldmanharpoon, CWeaponOldManHarpoon );
PRECACHE_WEAPON_REGISTER( weapon_oldmanharpoon );

acttable_t	CWeaponOldManHarpoon::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SUITCASE,					false },
	{ ACT_WALK,						ACT_WALK_SUITCASE,					false },

	{ ACT_IDLE_RELAXED,				/*ACT_IDLE_SUITCASE,*/ ACT_IDLE,		false },	// BJ: Player animations added under
	{ ACT_RUN_RELAXED,				/*ACT_WALK_SUITCASE*/ ACT_RUN,		false },

	{ ACT_HL2MP_IDLE,               ACT_HL2MP_IDLE_SLAM,                  false },
    { ACT_HL2MP_RUN,                ACT_HL2MP_RUN_SLAM,                    false },

    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,       false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,       false },

    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,             false },
};
IMPLEMENT_ACTTABLE( CWeaponOldManHarpoon );
