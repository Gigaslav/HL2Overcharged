#ifndef	WEAPON_IONRIFLE_H
#define	WEAPON_IONRIFLE_H

#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "energy_wave.h"
#include "explode.h"
#include "grenade_ar2.h"
#include "te_particlesystem.h"

//Ion explosion entity

class CTE_IonBang : public CTEParticleSystem
{
public:
	DECLARE_CLASS(CTE_IonBang, CTEParticleSystem);
	DECLARE_SERVERCLASS();

	CTE_IonBang(const char *name);
	virtual	~CTE_IonBang(void);

	CNetworkVector(m_vecNormal);
	CNetworkVar(float, m_flScale);
	CNetworkVar(int, m_nRadius);
	CNetworkVar(int, m_nMagnitude);
};

// Singleton to fire TEExplosion objects
static CTE_IonBang g_TEIonExplosion("IonExplosion");

//Temp ent for the blast

class C_IonBlast : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(C_IonBlast, CBaseEntity);

	int		m_spriteTexture;

	C_IonBlast(void) {}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------
	void Precache(void)
	{
		m_spriteTexture = PrecacheModel("sprites/lgtning.vmt"); //sprites/lgtning.vmt
		PrecacheParticleSystem("hunter_projectile_explosion_1"); // precache explosion effect

		BaseClass::Precache();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------

	void Explode(float magnitude);
};



class CWeaponIonRifle : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponIonRifle, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

	CWeaponIonRifle(void);
	~CWeaponIonRifle(void);

	void Precache(void);
	void PrimaryAttack(void);

	//void DelayedFire( void );
	//void ItemPostFrame( void );
	//void AlertTargets( void );
	void UpdateLasers(void);

	virtual bool Reload(void);
	//void	StartLaserEffects(void);
	//void	StopLaserEffects(void);
	//bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	bool	ShouldDrawWaterImpacts(const trace_t &shot_trace);

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	DECLARE_ACTTABLE();
	//private:
	//bool pressed;
protected:
	//float	m_flChargeTime;
	//bool	m_bFired;
	//bool StartEffects;
	//CNetworkVar(bool, EnableLaser);
	int		m_beamIndex;
	int		m_haloIndex;
	bool DoOnce;
	//CHandle<CSprite>	m_hLaserMuzzleSprite;
	//CHandle<CBeam>		m_hLaserBeam;
};

#endif