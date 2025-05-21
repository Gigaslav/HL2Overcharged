#ifndef	WEAPONDISPLACER_H
#define	WEAPONDISPLACER_H
#include "cbase.h"
#include "basehlcombatweapon.h"
//enum DISPL_FIRESTATE { FIRE_NONE, FIRE_STARTUP, FIRE_END };

//-----------------------------------------------------------------------------
// CWeapondisplacer
//-----------------------------------------------------------------------------

class CWeapondisplacer : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:

	DECLARE_CLASS(CWeapondisplacer, CBaseHLCombatWeapon);

	CWeapondisplacer(void);

	DECLARE_SERVERCLASS();

protected:

	virtual void			Precache(void);
	virtual void			ItemPostFrame(void);
	virtual void			PrimaryAttack(void);
	virtual void			SecondaryAttack(void);
	virtual void			DelayedAttack(void);
	virtual void			FireTheBall(void);

	string_t				m_iLandmark;
	//DISPL_FIRESTATE			m_fireState;

protected:

	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	bool					m_bPrimary;
	bool					m_bSecondary;
	bool					isFiring;
	DECLARE_ACTTABLE();
};

#endif