#ifndef	WEAPON_HORNETGUN_H
#define	WEAPON_HORNETGUN_H

#include "basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"
#include "projectile_hornet.h"

//-----------------------------------------------------------------------------
// CWeaponHornetGun
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponHornetGun C_WeaponHornetGun
#endif

class CWeaponHornetGun : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponHornetGun, CBaseHLCombatWeapon);
public:

	CWeaponHornetGun(void);
	~CWeaponHornetGun(void);
	void	Spawn(void);

	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	void			SecondaryAttack(void);
	virtual bool	Reload(void);
	bool	ReloadOrSwitchWeapons(void) { return false; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:

	void	FireBolt(void);

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

	int		m_iFirePhase;
	bool				m_bInZoom;
	bool				m_bMustReload;

	bool bFirstPickUp;
};

#endif