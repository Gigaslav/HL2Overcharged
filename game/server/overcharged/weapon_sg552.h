#ifndef	WEAPONSG552_H
#define	WEAPONSG552_H

#include "basecombatweapon.h"
#include "basehlcombatweapon.h"

class CWeaponSG552 : public CHLMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSG552, CHLMachineGun);

	CWeaponSG552();

	DECLARE_SERVERCLASS();

	virtual void	Precache(void);
	virtual int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void			FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void			Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();
};

#endif