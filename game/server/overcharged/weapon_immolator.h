#ifndef	WEAPONIMMOLATOR_H
#define	WEAPONIMMOLATOR_H
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "beam_flags.h"
#include "sprite.h"

class CWeaponImmolator : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponImmolator, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();

	CWeaponImmolator();

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

protected:

	virtual Activity		GetPrimaryAttackActivity(void);
	virtual void			Precache(void);
	virtual void			UseAmmo(int count);
	virtual void			PrimaryAttack(void);
	virtual void			ApplyDamage(const Vector &vecDir, trace_t &tr, CBasePlayer *pPlayer);
	virtual void			AmmoUsing();
	virtual void			ItemPostFrame(void);
	virtual void			Charge(void);
	virtual bool			WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions);
	virtual int				WeaponRangeAttack1Condition(float flDot, float flDist);
	virtual bool			Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual void			Fire(const Vector &vecOrigSrc, const Vector &vecDir, CBasePlayer *pPlayer);
	virtual void			UpdateBeam(const Vector &startPoint, const Vector &endPoint, CBasePlayer *pPlayer);
	virtual void			CreateBeam(CBasePlayer *pPlayer);
	virtual void			DestroyBeam(void);
	virtual bool			ShouldDrawWaterImpacts(const trace_t &shot_trace);

	float					m_flBurnRadius;
	float					m_flTimeLastUpdatedRadius;
	Vector					m_vecLaserTarget;
	float					m_bPlayingHitWall;
	float					m_bPlayingHitFlesh;

	float					m_flChargeTime;//Время зарядки
	float					m_flAmmoTime;
	float					m_flDmgTime;

private:

	int						m_beamIndex;
	bool					m_bFirstPlay;

	CHandle<CSprite>		m_SSprite;
	CHandle<CBeam>			m_hCurveBeam;
	CHandle<CBeam>			m_hCurveBeam2;
	CHandle<CBeam>			m_hBeam;
};

#endif