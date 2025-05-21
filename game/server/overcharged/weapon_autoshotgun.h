#ifndef	WEAPON_AUTOSHOTGUN_H
#define	WEAPON_AUTOSHOTGUN_H

#include "basehlcombatweapon_shared.h"

class CWeaponAutoShotgun : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponAutoShotgun, CBaseHLCombatWeapon);
public:

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CWeaponAutoShotgun(void);

	void SecondaryAttack(void) { return; }

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

private:
	bool m_bNeedPump;
	bool m_bDelayedFire;
	FireBulletsInfo_t info;
protected:

	int	m_nShotsFired;
};

#endif