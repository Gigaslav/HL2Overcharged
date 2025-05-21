#ifndef	WEAPON_SNARK_H
#define	WEAPON_SNARK_H

#include "basehlcombatweapon.h"

#define SNARK_DRAW_SOUND	"WeaponSnark.Draw"
#define SNARK_THROW_SOUND	"WeaponSnark.Throw"

//-----------------------------------------------------------------------------
// CWeaponSnark
//-----------------------------------------------------------------------------

class CWeaponSnark : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponSnark, CBaseHLCombatWeapon);
public:

	CWeaponSnark(void);

	void	Precache(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	WeaponIdle(void);
	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	DECLARE_ACTTABLE();			// BJ: MP animstate
	bool	m_bThrowState;
};

#endif