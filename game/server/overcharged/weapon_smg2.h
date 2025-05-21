#ifndef	WEAPON_SMG2_H
#define	WEAPON_SMG2_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponSMG2
//-----------------------------------------------------------------------------

class CWeaponSMG2 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG2, CHLSelectFireMachineGun);

	CWeaponSMG2();
	~CWeaponSMG2();

	DECLARE_SERVERCLASS();

	void			Precache(void);
	virtual void	Equip(CBaseCombatCharacter *pOwner);
	virtual void	SecondaryAttack(void) {};
	int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int				WeaponRangeAttack1Condition(float flDot, float flDist);
	int				WeaponRangeAttack2Condition(float flDot, float flDist);
	void			FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void			Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void			Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	float m_flTooCloseTimer;

	DECLARE_ACTTABLE();
private:
	int Shoot;
	float   MuzzleFlashTime;//—чЄтчик дл€ вспышки
};

#endif