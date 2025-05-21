#ifndef	WEAPON_M4_H
#define	WEAPON_M4_H

#include "basehlcombatweapon.h"

class CWeaponM4 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponM4, CHLSelectFireMachineGun);

	CWeaponM4();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void);
	virtual const char		*GetWorldModel(void) const;
	virtual void SetWeaponModelIndex(const char *pName);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition(float flDot, float flDist);
	const WeaponProficiencyInfo_t *GetProficiencyValues();
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;

private:

	bool m_bCheckIvent;
	float m_flCheckTimer;
};


#endif