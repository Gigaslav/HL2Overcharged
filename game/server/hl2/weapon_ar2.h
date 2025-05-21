//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONAR2_H
#define	WEAPONAR2_H

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"
#include "ammodef.h"
#include "sprite.h"
#include "beam_shared.h"
//#include "Sprite.h"
#include "basecombatweapon_shared.h"
#define FLARE_SPRITE		"sprites/redglow1.vmt"
#define	BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	LASER_SPRITE	"sprites/greenglow1.vmt"
#define	LSPRITE	        "sprites/greenglow1.vmt"//"effects/greenglow1"
#define	LASER_SPRITE1	"sprites/redglow1.vmt"
class CWeaponAR2 : public CHLMachineGun
{
public:
	DECLARE_CLASS(CWeaponAR2, CHLMachineGun);

	CWeaponAR2();

	DECLARE_SERVERCLASS();

protected:
	Activity		GetDrawActivity(void);
	Activity		GetHolsterActivity(void);
	virtual void	ItemPostFrame(void);
	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);
	virtual void	DelayedAttack(void);
	const char		*GetTracerType(void) { return "AR2Tracer"; }
	virtual void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	virtual void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	virtual void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual bool	CanHolster(void);
	virtual int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	const WeaponProficiencyInfo_t *GetProficiencyValues();

private:
	void					SetRoundsVisibility();
	void					SetRoundVisibility(animevent_t *pEvent, int iRound);
	float					m_flDelayedFire;
	bool					m_bShotDelayed;

	int						m_iFirstRound;
	int						m_iSecondRound;
	int						m_iThirdRound;

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


#endif	//WEAPONAR2_H
