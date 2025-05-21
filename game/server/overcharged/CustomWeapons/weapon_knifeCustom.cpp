#include "cbase.h"
#include "weapon_knifeCustom.h"

#include "tier0/memdbgon.h"
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERVERCLASS_ST(CWeaponKnifeC1, DT_WeaponKnifeC1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_knifeCustom1, CWeaponKnifeC1);
PRECACHE_WEAPON_REGISTER(weapon_knifeCustom1);

BEGIN_DATADESC(CWeaponKnifeC1)
END_DATADESC()

acttable_t	CWeaponKnifeC1::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },							// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },		// END

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};
IMPLEMENT_ACTTABLE(CWeaponKnifeC1);
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERVERCLASS_ST(CWeaponKnifeC2, DT_WeaponKnifeC2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_knifeCustom2, CWeaponKnifeC2);
PRECACHE_WEAPON_REGISTER(weapon_knifeCustom2);

BEGIN_DATADESC(CWeaponKnifeC2)
END_DATADESC()

acttable_t	CWeaponKnifeC2::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },							// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },		// END

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};
IMPLEMENT_ACTTABLE(CWeaponKnifeC2);
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERVERCLASS_ST(CWeaponKnifeC3, DT_WeaponKnifeC3)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_knifeCustom3, CWeaponKnifeC3);
PRECACHE_WEAPON_REGISTER(weapon_knifeCustom3);

BEGIN_DATADESC(CWeaponKnifeC3)
END_DATADESC()

acttable_t	CWeaponKnifeC3::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },							// Light Kill : MP animstate for singleplayer
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },		// END

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};
IMPLEMENT_ACTTABLE(CWeaponKnifeC3);
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////