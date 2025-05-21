#ifndef	WEAPON_OICW_H
#define	WEAPON_OICW_H

#include "grenade_oicw.h"
#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeaponOICW
//-----------------------------------------------------------------------------

class CWeaponOICW : public CHLMachineGun
{
public:
	DECLARE_CLASS(CWeaponOICW, CHLMachineGun);

	CWeaponOICW();

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

	void	Precache(void);

	virtual void	SecondaryAttack(void);

	virtual void Equip(CBaseCombatCharacter *pOwner);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;

private:
	bool				m_bMustReload;
};

#endif