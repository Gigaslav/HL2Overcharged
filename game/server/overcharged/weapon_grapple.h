#ifndef WEAPON_GRAPPLE_H
#define WEAPON_GRAPPLE_H
 
#ifdef _WIN32
#pragma once
#endif
 
//#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "basehlcombatweapon.h"
 
#ifndef CLIENT_DLL
	#include "rope.h"
	#include "props.h"
#endif
 
#include "rope_shared.h"

// Light Kill : Forgotten uncludes...
#include "Sprite.h"
#include "SpriteTrail.h"

#ifndef CLIENT_DLL
 
class CWeaponGrapple;
 
//-----------------------------------------------------------------------------
// Grapple Hook
//-----------------------------------------------------------------------------
class CGrappleHook : public CBaseCombatCharacter
{
	DECLARE_CLASS( CGrappleHook, CBaseCombatCharacter );
 
public:
	CGrappleHook() { };
	~CGrappleHook();
 
	Class_T Classify( void ) { return CLASS_NONE; }
 
public:
	void Spawn( void );
	void Precache( void );
	void FlyThink( void );
	void HookedThink( void );
	void HookTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CGrappleHook *HookCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner = NULL );
 
protected:
 
	DECLARE_DATADESC();
 
private:
 
	void UpdatePlayerConstraint( void );
 
	CHandle<CWeaponGrapple>		m_hOwner;
	CHandle<CBasePlayer>		m_hPlayer;
	CHandle<CDynamicProp>		m_hBolt;
	IPhysicsSpring				*m_pSpring;
	float						m_fSpringLength;
	bool						m_bPlayerWasStanding;
};
 
#endif
 
//-----------------------------------------------------------------------------
// CWeaponGrapple
//-----------------------------------------------------------------------------
 
#ifdef CLIENT_DLL
#define CWeaponGrapple C_WeaponGrapple
#endif
 
class CWeaponGrapple : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponGrapple, CBaseHLCombatWeapon );
public:
 
	CWeaponGrapple( void );
 
	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual bool	Deploy( void );
	bool			CanHolster( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	void			Drop( const Vector &vecVelocity );
	virtual bool	Reload( void );
	virtual void	ItemPostFrame( void );
	virtual void	ItemBusyFrame( void );
	virtual bool	SendWeaponAnim( int iActivity );
 
	void			NotifyHookDied( void );
 
	bool			HasAnyAmmo( void );
 
	CBaseEntity		*GetHook( void ) { return m_hHook; }
 
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif
 
	//DECLARE_NETWORKCLASS(); 
	//DECLARE_PREDICTABLE();
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
 
private:
 
	void	SetSkin( int skinNum );
	void	CheckZoomToggle( void );
	void	FireHook( void );
	void	ToggleZoom( void );
 
	// Various states for the crossbow's charger
	enum ChargerState_t
	{
		CHARGER_STATE_START_LOAD,
		CHARGER_STATE_START_CHARGE,
		CHARGER_STATE_READY,
		CHARGER_STATE_DISCHARGE,
		CHARGER_STATE_OFF,
	};
 
	void	CreateChargerEffects( void );
	void	SetChargerState( ChargerState_t state );
	void	DoLoadEffect( void );
 
#ifndef CLIENT_DLL
	bool	CreateRope( void );
 
	DECLARE_ACTTABLE();
#endif
 
private:
 
	// Charger effects
	ChargerState_t		m_nChargeState;
 
#ifndef CLIENT_DLL
	CHandle<CSprite>	m_hChargerSprite;
	CHandle<CRopeKeyframe>		m_hRope;
#endif
 
	//CNetworkVar( bool,	m_bInZoom );
	//CNetworkVar( bool,	m_bMustReload );

	bool				m_bInZoom;
	bool				m_bMustReload;
 
	//CNetworkHandle( CBaseEntity, m_hHook );
	CHandle<CBaseEntity>			m_hHook;
 
	CWeaponGrapple( const CWeaponGrapple & );
};
 
 
 
#endif // WEAPON_GRAPPLE_H