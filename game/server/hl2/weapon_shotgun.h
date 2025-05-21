#ifndef	WEAPON_SHOTGUN_H
#define	WEAPON_SHOTGUN_H

#include "basehlcombatweapon_shared.h"

//-----------------------------------------------------------------------------
// CWeaponShotgun
//-----------------------------------------------------------------------------

class CWeaponShotgun : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponShotgun, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

	~CWeaponShotgun(){};

private:
	bool wasAnimated;
	bool ChangeOnce;
	bool    PumpAfterTwoBarrels;
	bool	m_bNeedPump;		// When emptied completely
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
	bool	m_bDelayedFire2;	// Fire secondary when finished reloading
	float   MuzzleFlashTime;//—чЄтчик дл€ вспышки
	FireBulletsInfo_t info;

public:
	void	Precache(void);
	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual float			GetMinRestTime();
	virtual float			GetMaxRestTime();
	void DryFire(void);
	bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

	CWeaponShotgun(void);
};

#endif