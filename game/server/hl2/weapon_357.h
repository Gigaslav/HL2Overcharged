#ifndef	WEAPON_357_H
#define	WEAPON_357_H

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// CWeapon357
//-----------------------------------------------------------------------------
class CWeapon357 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeapon357, CBaseHLCombatWeapon);
public:

	CWeapon357(void);
	void			Precache(void);
	virtual void	ItemBusyFrame(void);
	virtual bool	IsWeaponZoomed() { return m_bInZoom; }
	virtual void	Drop(const Vector &vecVelocity);
	bool			Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

#ifndef CLIENT_DLL
	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

	float	WeaponAutoAimScale()	{ return 0.6f; }
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

private:
	void	CheckZoomToggle(void);
	bool	m_bInZoom;
	void	ToggleZoom(void);
	void	StopEffects(void);
	int		m_nNumShotsFired;
	float	m_flAccuracyPenalty;
	bool	DoOnceMPD;

public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

};

#endif