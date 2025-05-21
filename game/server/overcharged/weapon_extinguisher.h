#ifndef	WEAPON_EXT_H
#define	WEAPON_EXT_H

#include "basehlcombatweapon.h"
#include "extinguisherjet.h"

class CWeaponExtinguisher : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponExtinguisher, CHLSelectFireMachineGun);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponExtinguisher();

	void	Spawn(void);
	void	Precache(void);

	void	ItemPostFrame(void);
	void	Event_Killed(const CTakeDamageInfo &info);
	void	Equip(CBaseCombatCharacter *pOwner);
	void	Drop(const Vector &vecVelocity);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	virtual void SetPickupTouch(void)
	{
		// BJ, MikeD: No auto pick up
		if (cvar->FindVar("oc_weapon_extinguisher_pickup")->GetInt()) BaseClass::SetPickupTouch();
	}

protected:

	void	StartJet(void);
	void	StopJet(void);
	bool	m_bFireSoundOn;	//test

	CExtinguisherJet	*m_pJet;
};

#endif