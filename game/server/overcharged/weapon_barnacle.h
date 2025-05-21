//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the hand barnacle weapon.
//			
//	
//	
//
//
//=============================================================================//

#ifndef WEAPON_barnacle_H
#define WEAPON_barnacle_H
 
#ifdef _WIN32
#pragma once
#endif

//#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "basehlcombatweapon.h"                
#include "sprite.h"                            
#include "props.h"                             
 
#ifndef CLIENT_DLL
    #include "rope.h"
    #include "props.h"
#endif
 
#include "rope_shared.h"

#include "te_effect_dispatch.h"
#include "beam_shared.h"
 
#ifndef CLIENT_DLL
 
class CWeaponbarnacle;
 
//-----------------------------------------------------------------------------
// barnacle Hook
//-----------------------------------------------------------------------------
class CbarnacleHook : public CBaseCombatCharacter
{
    DECLARE_CLASS( CbarnacleHook, CBaseCombatCharacter );
 
public:
    CbarnacleHook() { };
    ~CbarnacleHook();
 
    Class_T Classify( void ) { return CLASS_NONE; }
 
public:
    void Spawn( void );
    void Precache( void );
    void FlyThink( void );
    void HookedThink( void );
    void HookTouch( CBaseEntity *pOther );
    bool CreateVPhysics( void );
    unsigned int PhysicsSolidMaskForEntity() const;
    static CbarnacleHook *HookCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner = NULL );
 
protected:
 
    DECLARE_DATADESC();
 
private:
  
    CHandle<CWeaponbarnacle>    m_hOwner;
    CHandle<CBasePlayer>        m_hPlayer;
    CHandle<CDynamicProp>       m_hBolt;
    IPhysicsSpring              *m_pSpring;
    float                       m_fSpringLength;
    bool                        m_bPlayerWasStanding;
};
 
#endif
 
//-----------------------------------------------------------------------------
// CWeaponbarnacle
//-----------------------------------------------------------------------------
 
#ifdef CLIENT_DLL
#define CWeaponbarnacle C_Weaponbarnacle
#endif

//class CWeaponbarnacle : public CBaseHL2MPCombatWeapon       
class CWeaponbarnacle : public CBaseHLCombatWeapon            
{                                                            
//  DECLARE_CLASS( CWeaponbarnacle, CBaseHL2MPCombatWeapon ); 
    DECLARE_CLASS( CWeaponbarnacle, CBaseHLCombatWeapon );    
public:
 
    CWeaponbarnacle( void );
 
    virtual void    Precache( void );
    virtual void    PrimaryAttack( void );
    virtual void    SecondaryAttack( void );
    virtual bool    Deploy( void );
    bool            CanHolster( void );
    virtual bool    Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
    void            Drop( const Vector &vecVelocity );
    virtual bool    Reload( void );
    virtual void    ItemPostFrame( void );
    virtual void    ItemBusyFrame( void );
 
    void            NotifyHookDied( void );
 
    bool            HasAnyAmmo( void );
 
    CBaseEntity     *GetHook( void ) { return m_hHook; }

	//bool ToggleHook( void );

	void   	DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	//void	DoImpactEffect( trace_t &tr, int nDamageType );

	//bool                        m_bHook;
 
    //DECLARE_NETWORKCLASS(); 
    //DECLARE_PREDICTABLE();
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
 
private:

    void    FireHook( void );
 
#ifndef CLIENT_DLL
 
    DECLARE_ACTTABLE();
#endif
 
private:

#ifndef CLIENT_DLL
	CHandle<CBeam>        pBeam;
	//CHandle<CSprite>	m_pLightGlow;
#endif
 
    CNetworkVar( bool,    m_bInZoom );
    CNetworkVar( bool,    m_bMustReload );
 
    CNetworkHandle( CBaseEntity, m_hHook );
 
    CWeaponbarnacle( const CWeaponbarnacle & );

	CNetworkVar( int, m_nBulletType );
};
 
 
 
#endif // WEAPON_barnacle_H