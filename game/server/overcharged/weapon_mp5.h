#ifndef	WEAPON_MP5_H
#define	WEAPON_MP5_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponMP5
//-----------------------------------------------------------------------------

class CWeaponmp5 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponmp5, CHLSelectFireMachineGun);

	CWeaponmp5();

	DECLARE_SERVERCLASS();

	virtual void	Equip(CBaseCombatCharacter *pOwner);
	bool			Reload(void);

	virtual void	SecondaryAttack(void) {};
	int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int				WeaponRangeAttack2Condition(float flDot, float flDist);

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

#endif