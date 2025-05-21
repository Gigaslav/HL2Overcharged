#ifndef	WEAPON_HOPMINE_H
#define	WEAPON_HOPMINE_H

#include "basegrenade_shared.h"
#include "basecombatweapon_shared.h"
#include "basehlcombatweapon.h"

class CWeapon_HopMine : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeapon_HopMine, CBaseCombatWeapon);

	DECLARE_SERVERCLASS();

	void				Spawn(void);
	void				Precache(void);
	void				Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void				PrimaryAttack(void);
	void				SecondaryAttack(void);
	void				ItemPostFrame(void);
	void				SatchelThrow(bool m_bPrimary = true);
	void				DecrementAmmo(CBaseCombatCharacter *pOwner);
	void				Weapon_Switch(void);
	bool				Deploy(void);

	CWeapon_HopMine();

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

private:
	int m_iAttackState;
};

#endif