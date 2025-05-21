#include "basehlcombatweapon.h"
#include "gamestats.h"

#ifndef WEAPON_FRAG
#define WEAPON_FRAG

#define GRENADE_TIMER	3.0f //Seconds

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches
//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFrag : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponFrag, CBaseHLCombatWeapon);
public:
	DECLARE_SERVERCLASS();

public:
	//CWeaponFrag();


	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	DecrementAmmo(CBaseCombatCharacter *pOwner);
	void	ItemPostFrame(void);

	bool	Deploy(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	bool	Reload(void);

	bool	ShouldDisplayHUDHint() { return true; }

	void	CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade

protected:
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual void	Precache(void);
	virtual void	ThrowGrenade(CBasePlayer *pPlayer);
	virtual void	RollGrenade(CBasePlayer *pPlayer);
	virtual void	LobGrenade(CBasePlayer *pPlayer);
private:

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye

	int		m_AttackPaused;
	bool	m_fDrawbackFinished;

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};

#endif