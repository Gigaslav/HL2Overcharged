#ifndef	WEAPON_CSNIPER_H
#define	WEAPON_CSNIPER_H

#include "basehlcombatweapon.h"
#include "IEffects.h"//дым от ствола
#include "beam_shared.h"//For Laser
#include "Sprite.h"//For laser

class CWeaponCombineSniper : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponCombineSniper, CBaseHLCombatWeapon);
public:

	CWeaponCombineSniper(void);
	void	Precache(void);
	void	ItemPostFrame(void);
	void	Pump(void);
	void	PrimaryAttack(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void	Operator_FrameUpdate(CBaseCombatCharacter  *pOperator);	// BJ: Added laser npc
	float	WeaponAutoAimScale()	{ return 0.6f; }
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

private:

#ifndef CLIENT_DLL
	CHandle<CBeam>        pBeam;
#endif

private:
	float durationTime;
	float durationTime2;
	bool	wasInReload;

public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

};

#endif