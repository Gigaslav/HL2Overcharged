//========= Copyright © 2002-2008, Lolmen & Valve, All rights reserved. ============
//
// Purpose: Tau Cannon Super gun
//
//==================================================================================

#include "basehlcombatweapon.h"

#ifndef WEAPON_GAUSS_H
#define WEAPON_GAUSS_H
#ifdef _WIN32
#pragma once
#endif

#include "te_particlesystem.h"
#include "effect_dispatch_data.h"

#define GAUSS_BEAM_SPRITE		"effects/gauss/beam.vmt"

#define	GAUSS_CHARGE_TIME		0.2f
#define	MAX_GAUSS_CHARGE		16
#define	MAX_GAUSS_CHARGE_TIME		3
#define	DANGER_GAUSS_CHARGE_TIME	10


//=============================================================================
// Tau cannon
//=============================================================================

class CWeaponGauss : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGauss, CBaseHLCombatWeapon);

	CWeaponGauss(void);

	DECLARE_SERVERCLASS();

	void	StartFire(void);

protected:

	virtual Activity	GetDrawActivity(void);
	virtual void	Spawn(void);
	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);

	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual void	Drop(const Vector &vecVelocity);
	virtual void	ItemPostFrame(void);
	virtual void	MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType) {};
	//float	GetFireRate( void ) { return 0.2f; }

	virtual void	Fire(bool secondary = false, float damage = 0.f);
	virtual void	FirePenetrated(const trace_t &tr, bool secondary = false, float damage = 0.f);
	virtual void	ChargedFire(void);
	virtual float	GetFullChargeTime(void);

	virtual void	StopChargeSound(void);
	virtual void	RandomZap(WeaponSound_t soundType = BURST);
	virtual void	DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle = false, bool secondary = false);
	virtual void	IncreaseCharge(void);
	virtual bool	ShouldDrawWaterImpacts(const trace_t &shot_trace);
//	float	m_flChargeTime2;//Время зарядки
private:
	EHANDLE			m_hViewModel;
	float			m_flNextChargeTime;
	float			m_flNextSpinTime;
	float			m_flNextZapTime;
	float			minZap;
	float			maxZap;

	CSoundPatch		*m_sndCharge;

	float			m_flChargeStartTime;
	bool			m_bCharging;
	bool			m_bChargeIndicated;
	bool			isCharging;
	bool			blockSoundThink;
//	bool		delay2;
	float			m_flNextSoundTime;//Счётчик для вспышки

	FireBulletsInfo_t info;
	DECLARE_ACTTABLE();
};

#endif // WEAPON_GAUSS_H