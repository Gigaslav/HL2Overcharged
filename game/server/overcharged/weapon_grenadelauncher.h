#ifndef	WEAPON_GLAUNCHER_H
#define	WEAPON_GLAUNCHER_H

#include "basehlcombatweapon.h"
#include "basegrenade_shared.h"

class CWeaponGrenadelauncher : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGrenadelauncher, CHLSelectFireMachineGun);

	CWeaponGrenadelauncher();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void) { return; }
	void	PrimaryAttack(void);
	virtual void Equip(CBaseCombatCharacter *pOwner);
	bool	Reload(void);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition();
	const   WeaponProficiencyInfo_t *GetProficiencyValues();
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

#endif