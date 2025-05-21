#ifndef	WEAPON_ANNABELLE_H
#define	WEAPON_ANNABELLE_H

#include "basehlcombatweapon_shared.h"

//-----------------------------------------------------------------------------
// CWeaponAnnabelle
//-----------------------------------------------------------------------------
class CWeaponAnnabelle : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAnnabelle, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();
	~CWeaponAnnabelle(){};

private:
	bool	m_bNeedPump;		// When emptied completely
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
	bool	m_bDelayedFire2;	// Fire secondary when finished reloading

public:
	void	Precache(void);
	void	PrimaryAttack(void);

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void ItemHolsterFrame(void);
	void SecondaryAttackShotgun(void)
	{
		return;
	}
	void CheckHolsterReload(void);
	void Pump(void);
	virtual float			GetMinRestTime() { return 0.3; }
	virtual float			GetMaxRestTime() { return 0.6; }
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

public:
	DECLARE_ACTTABLE();

	CWeaponAnnabelle(void);
};

#endif