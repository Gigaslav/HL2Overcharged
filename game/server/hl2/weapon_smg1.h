#ifndef	WEAPON_SMG1_H
#define	WEAPON_SMG1_H

#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
//-----------------------------------------------------------------------------
// CWeaponSMG1
//-----------------------------------------------------------------------------

class CWeaponSMG1 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponSMG1, CHLSelectFireMachineGun);

	CWeaponSMG1();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	SecondaryAttack(void);
	virtual void Equip(CBaseCombatCharacter *pOwner);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void	StartLaserEffects(void);
	void	StopLaserEffects(void);
	const WeaponProficiencyInfo_t *GetProficiencyValues();
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:
	CHandle<CBeam>		LaserBeam;
	CHandle<CSprite>	LaserMuzzleSprite;
	CHandle<CSprite>	LSprite;

	CHandle<CSprite>	MSprite;
	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

#endif