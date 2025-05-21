#ifndef	WEAPONDISPLACER_H
#define	WEAPONDISPLACER_H
#include "cbase.h"
#include "basehlcombatweapon.h"

class CWeaponMachineGun : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponMachineGun, CHLSelectFireMachineGun);

	CWeaponMachineGun();

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

protected:

	virtual void		Precache(void);
	virtual void		SecondaryAttack(void) { return; }
	virtual void		ItemPostFrame(void);
	virtual void		FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	virtual void		Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	virtual void		Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual void		Equip(CBaseCombatCharacter *pOwner);
	virtual int			CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual int			WeaponRangeAttack2Condition(float flDot, float flDist);
	virtual const		WeaponProficiencyInfo_t *GetProficiencyValues();

	Vector				m_vecTossVelocity;
	float				m_flNextGrenadeCheck;
	float				BG;
};
#endif