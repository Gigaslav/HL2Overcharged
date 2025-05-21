#ifndef	WEAPON_FGUN_H
#define	WEAPON_FGUN_H

#include "basehlcombatweapon.h"
#include "overcharged/env_flare.h"

// Custom derived class for flare gun projectiles
class CFlareGunProjectile : public CFlare
{
public:
	DECLARE_CLASS(CFlareGunProjectile, CFlare);
	static CFlareGunProjectile *Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
	void	IgniteOtherIfAllowed(CBaseEntity *pOther);
	void	FlareGunProjectileTouch(CBaseEntity *pOther);
	void	FlareGunProjectileBurnTouch(CBaseEntity *pOther);
};

class CFlaregun/*Custom*/ : public CBaseHLCombatWeapon //CFlaregun
{
public:
	//DECLARE_CLASS(CFlaregun/*Custom*/, CBaseHLCombatWeapon); //CFlaregun);
	DECLARE_CLASS(CFlaregun, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();

	//virtual bool	Reload(void); 

	void Precache(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);

	const WeaponProficiencyInfo_t *GetProficiencyValues();	// BriJee Weapon priority

	DECLARE_ACTTABLE();	// BriJee Player anims.
};

#endif