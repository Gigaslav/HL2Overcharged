#ifndef	WEAPON_M40A1_H
#define	WEAPON_M40A1_H

#include "basehlcombatweapon.h"


class CWeaponSniperM40A1 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponSniperM40A1, CBaseHLCombatWeapon);
public:

	CWeaponSniperM40A1(void);
	void	Precache(void);
	/*void	ItemPostFrame(void);
	void	Pump(void);
	void	PrimaryAttack(void);*/
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	float	WeaponAutoAimScale()	{ return 0.6f; }
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

private:
	float durationTime;
	float durationTime2;
	bool	wasInReload;

public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

};
#endif