#ifndef	WEAPON_DEAGLE_H
#define	WEAPON_DEAGLE_H

#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "te_effect_dispatch.h"

class CLaserDot2 : public CSprite
{
	DECLARE_CLASS(CLaserDot2, CSprite);
public:

	CLaserDot2(void);
	~CLaserDot2(void);

	static CLaserDot2 *Create(const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true);

	void	SetTargetEntity(CBaseEntity *pTarget) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity(void) { return m_hTargetEnt; }

	void	SetLaserPosition(const Vector &origin, const Vector &normal);
	Vector	GetChasePosition();
	void	TurnOn(void);
	void	TurnOff(void);
	bool	IsOn() const { return m_bIsOn; }

	void	Toggle(void);

	void	LaserThink(void);

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void	MakeInvisible(void);

protected:
	Vector				m_vecSurfaceNormal;
	EHANDLE				m_hTargetEnt;
	bool				m_bVisibleLaserDot;
	bool				m_bIsOn;

	DECLARE_DATADESC();
public:
	CLaserDot2			*m_pNext;
};
//=============================================================================
// Deagle
//=============================================================================

class CWeaponDeagle : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponDeagle, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
public:

	CWeaponDeagle(void);
	~CWeaponDeagle(void);

	void	Precache(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void	ItemPostFrame(void);
	bool	Deploy(void);
	virtual void Drop(const Vector &vecVelocity);
	void	Activate(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void	StartLaserEffects(void);
	void	StopLaserEffects(void);
	void	CreateLaserPointer(void);
	void	UpdateLaserPosition(Vector vecMuzzlePos = vec3_origin, Vector vecEndPos = vec3_origin);
	void	UpdateLaserEffects(void);

	void	StartGuiding(void);
	void	StopGuiding(void);
	bool	IsGuiding(void);
	void	SuppressGuiding(bool state = true);
	Vector	GetLaserPosition(void);

	float	WeaponAutoAimScale()	{ return 0.6f; }

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual bool Reload(void);

protected:
	bool				m_bInitialStateUpdate;
	bool				m_bGuiding;
	bool				m_bHideGuiding;		//User to override the player's wish to guide under certain circumstances
	Vector				m_vecNPCLaserDot2;
	CHandle<CLaserDot2>	m_hLaserDot;
	CHandle<CSprite>	m_hLaserMuzzleSprite;
	CHandle<CBeam>		m_hLaserBeam;
	int		m_nNumShotsFired;
	float	m_flAccuracyPenalty;


	DECLARE_DATADESC();

	DECLARE_ACTTABLE();
};

#endif