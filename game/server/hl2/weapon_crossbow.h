#ifndef	WEAPON_CROSSBOW_H
#define	WEAPON_CROSSBOW_H

#include "basehlcombatweapon_shared.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"

//-----------------------------------------------------------------------------
// CWeaponCrossbow
//-----------------------------------------------------------------------------

class CWeaponCrossbow : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponCrossbow, CBaseHLCombatWeapon);
public:

	CWeaponCrossbow(void);

	void			Activate(void);
	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);
	virtual bool	Deploy(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual bool	Reload(void);
	virtual void	ItemPostFrame(void);
	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual bool	SendWeaponAnim(int iActivity);

											//Twilight request to disable this feature
	bool			ShouldDisplayHUDHint()  { return false; }//{ return true; }


	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();	//added

private:

	void	StopEffects(void);
	void	FireBolt(void);

	// Various states for the crossbow's charger
	enum ChargerState_t
	{
		CHARGER_STATE_START_LOAD,
		CHARGER_STATE_START_CHARGE,
		CHARGER_STATE_READY,
		CHARGER_STATE_DISCHARGE,
		CHARGER_STATE_OFF,
	};

	void	CreateChargerEffects(void);
	void	SetChargerState(ChargerState_t state);
	void	DoLoadEffect(void);

private:
	bool Once;
	// Charger effects
	ChargerState_t		m_nChargeState;
	CHandle<CSprite>	m_hChargerSprite;
	bool				m_bMustReload;
	bool				exploBolts;
};

#endif