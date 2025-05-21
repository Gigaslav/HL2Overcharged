#ifndef	WEAPONCGUARD_H
#define	WEAPONCGUARD_H
#include "cbase.h"
#include "AI_BaseNPC.h"
#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "beam_flags.h"
#include "te_effect_dispatch.h"
#include "energy_wave.h"
#include "te_particlesystem.h"

class CWeaponCGuard : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCGuard, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

	CWeaponCGuard(void);

protected:
	void				FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	virtual void		Precache(void);
	virtual void		PrimaryAttack(void);
	virtual void		DelayedFire(void);
	virtual void		ItemPostFrame(void);
	virtual void		AlertTargets(void);
	virtual void		UpdateLasers(void);
	virtual void		Fire(const Vector &vecOrigSrc, const Vector &vecDir);
	virtual void		UpdateEffect(const Vector &startPoint, const Vector &endPoint);
	virtual void		CreateEffect(void);
	virtual void		DestroyEffect(void);
	virtual bool		Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual int			CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual bool		ShouldDrawWaterImpacts(const trace_t &shot_trace);

	float				m_flChargeTime;
	bool				Destroy;
	bool				m_bFired;
	bool				InHolster;
	int					m_beamIndex;
	int					m_haloIndex;

private:
	//CCguard_Sphere		*Sphere;
	CHandle<CBeam>		_hNoise;
};

#endif