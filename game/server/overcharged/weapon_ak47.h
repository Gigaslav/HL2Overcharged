#ifndef	WEAPONAK47_H
#define	WEAPONAK47_H

#include "basehlcombatweapon.h"

class CWeaponAK47 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:

	DECLARE_CLASS(CWeaponAK47, CHLSelectFireMachineGun);

	CWeaponAK47();

	DECLARE_SERVERCLASS();

protected:

	virtual int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual int				WeaponRangeAttack2Condition(float flDot, float flDist);
	const					WeaponProficiencyInfo_t *GetProficiencyValues();
	virtual void			SecondaryAttack(void) override;
	virtual void			Equip(CBaseCombatCharacter *pOwner);
	virtual void			FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	virtual void			Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	virtual void			Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

#endif