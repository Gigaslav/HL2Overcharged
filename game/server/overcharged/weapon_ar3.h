#ifndef	WEAPONAR3_H
#define	WEAPONAR3_H

#include "basehlcombatweapon.h"

class CWeaponAr3 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAr3, CHLSelectFireMachineGun);

	CWeaponAr3();

	DECLARE_SERVERCLASS();

	void	Precache(void);

	virtual void	SecondaryAttack(void) {};
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void			FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void			Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void			Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:
	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;

};

#endif