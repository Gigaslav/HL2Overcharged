#ifndef	WEAPON_VECTOR_H
#define	WEAPON_VECTOR_H

#include "basehlcombatweapon.h"

class CWeaponVector : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponVector, CHLSelectFireMachineGun);

	CWeaponVector();

	DECLARE_SERVERCLASS();

	void	SecondaryAttack(void) {};
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	const	WeaponProficiencyInfo_t *GetProficiencyValues();
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

};

#endif