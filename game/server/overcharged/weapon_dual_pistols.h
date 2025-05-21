#ifndef	WEAPON_DPS_H
#define	WEAPON_DPS_H

#include "basehlcombatweapon.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "beam_shared.h"//For Laser
#include "Sprite.h"//For laser
//-----------------------------------------------------------------------------
// CWeaponPistol
//-----------------------------------------------------------------------------

class CWeaponDual_Pistols : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponDual_Pistols, CBaseHLCombatWeapon);

	CWeaponDual_Pistols(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	Activity	GetDrawActivity(void);
	void	PrimaryAttack(void);
	void    Drop(const Vector &vecVelocity);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual bool Reload(void);

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	float   MuzzleFlashTime;
	int		m_nNumShotsFired;
	int Shoot2;
	bool    bFlip;

protected:
	CHandle<CBeam>		LaserBeam;
	CHandle<CSprite>	LaserMuzzleSprite;
	CHandle<CSprite>	LSprite;
};


#endif