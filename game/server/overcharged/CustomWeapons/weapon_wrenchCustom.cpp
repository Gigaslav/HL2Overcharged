#include "cbase.h"
#include "weapon_wrenchCustom.h"

#include "tier0/memdbgon.h"


IMPLEMENT_SERVERCLASS_ST(CWeaponWrenchC1, DT_WeaponWrenchC1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_wrenchCustom1, CWeaponWrenchC1);
PRECACHE_WEAPON_REGISTER(weapon_wrenchCustom1);

BEGIN_DATADESC(CWeaponWrenchC1)

END_DATADESC()

acttable_t	CWeaponWrenchC1::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponWrenchC1);





IMPLEMENT_SERVERCLASS_ST(CWeaponWrenchC2, DT_WeaponWrenchC2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_wrenchCustom2, CWeaponWrenchC2);
PRECACHE_WEAPON_REGISTER(weapon_wrenchCustom2);

BEGIN_DATADESC(CWeaponWrenchC2)

END_DATADESC()

acttable_t	CWeaponWrenchC2::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponWrenchC2);




IMPLEMENT_SERVERCLASS_ST(CWeaponWrenchC3, DT_WeaponWrenchC3)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_wrenchCustom3, CWeaponWrenchC3);
PRECACHE_WEAPON_REGISTER(weapon_wrenchCustom3);

BEGIN_DATADESC(CWeaponWrenchC3)

END_DATADESC()

acttable_t	CWeaponWrenchC3::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },
    { ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },

	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,		false },
	{ ACT_WALK_RELAXED,				ACT_WALK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponWrenchC3);