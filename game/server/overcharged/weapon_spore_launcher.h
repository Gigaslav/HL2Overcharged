#ifndef	WEAPON_SPORE_LAUNC_H
#define	WEAPON_SPORE_LAUNC_H

#include "basehlcombatweapon_shared.h"
#include "grenade_spit.h"
#include "grenade_strooper_instant.h"

//-----------------------------------------------------------------------------
// CWeaponSporeLauncher
//-----------------------------------------------------------------------------
class CWeaponSporeLauncher : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponSporeLauncher, CBaseHLCombatWeapon);
public:

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CWeaponSporeLauncher(void);
	virtual void PrimaryAttackShotgun(void);
	virtual void SecondaryAttackShotgun(void);
	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

private:
	void	FireBolt(void);
	void	FireBoltBounced(void);
	bool m_bNeedPump;
	bool m_bDelayedFire;
	FireBulletsInfo_t info;
	bool ChangeOnce;

protected:
	int	m_nShotsFired;

};

#endif