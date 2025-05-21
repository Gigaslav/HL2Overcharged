#ifndef	WEAPON_M16_H
#define	WEAPON_M16_H

#include "basehlcombatweapon.h"

class CWeaponM16 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM16, CHLSelectFireMachineGun);

	CWeaponM16();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void);
	virtual void Equip(CBaseCombatCharacter *pOwner);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);

	void	StartLaserEffects(void);
	void	StopLaserEffects(void);
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