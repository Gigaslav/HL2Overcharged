#ifndef	WEAPON_SHOCKRIFLE_H
#define	WEAPON_SHOCKRIFLE_H

#include "basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"
#include "beam_shared.h"

//-----------------------------------------------------------------------------
// CWeaponShockRifle
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponShockRifle C_WeaponShockRifle
#endif

class CWeaponShockRifle : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponShockRifle, CBaseHLCombatWeapon);

public:
	CWeaponShockRifle(void);
	~CWeaponShockRifle(void);

	void			Spawn(void);
	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	void			SecondaryAttack(void) { return; }
	void            Drop(const Vector &vecVelocity);
	virtual bool	Reload(void);
	void			StartLaserEffects(void);
	void			StopLaserEffects(void);
	void			Event_Killed(const CTakeDamageInfo &info);
	bool			ReloadOrSwitchWeapons(void) { return false; }
	virtual void	ItemPostFrame(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	void			FireBolt(void);

	// Various states for the crossbow's charger
	enum ChargerState_t
	{
		CHARGER_STATE_START_LOAD,
		CHARGER_STATE_START_CHARGE,
		CHARGER_STATE_READY,
		CHARGER_STATE_ShockRifleHARGE,
		CHARGER_STATE_OFF,
	};

	DECLARE_ACTTABLE();

protected:
	CHandle<CBeam>		m_hLaserBeam1;
	CHandle<CBeam>		m_hLaserBeam2;
	CHandle<CBeam>		m_hLaserBeam3;

private:
	bool				m_bInZoom;
	bool				m_bMustReload;
	bool bFirstPickUp;
};

#endif