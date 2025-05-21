//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		The class from which all bludgeon melee
//				weapons are derived. 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "basehlcombatweapon.h"
#include "basecombatweapon_shared.h"

#ifndef BASEBLUDGEONWEAPON_H
#define BASEBLUDGEONWEAPON_H

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CBaseHLBludgeonWeapon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CBaseHLBludgeonWeapon, CBaseHLCombatWeapon );
public:
	CBaseHLBludgeonWeapon();

	DECLARE_SERVERCLASS();

	virtual	void	Spawn( void );
	virtual	void	Precache( void );
	
	//Attack functions
	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	virtual void	AddViewKick(void);
	virtual void	ItemPostFrame( void );

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity	GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

	virtual	float	GetFireRate( void )								
	{	
		return BaseClass::GetFireRate();
	}

	float					GetMeleeRange()					{ return GetWpnData().meleeRange; }

	virtual const Vector&	GetMeleeHullMin()
	{
		static Vector hullMin = Vector(GetWpnData().meleeHull*-1, GetWpnData().meleeHull*-1, GetWpnData().meleeHull*-1);
		return hullMin;
	}

	virtual const Vector	&GetMeleeHullMax()
	{
		static Vector hullMax = Vector(GetWpnData().meleeHull, GetWpnData().meleeHull, GetWpnData().meleeHull);
		return hullMax;
	}

	virtual float	GetRange(void)									{ return GetWpnData().meleeRange; }
	virtual float   GetHull(void)									{	return GetWpnData().meleeHull; }
	virtual	float	GetDamageForActivity( Activity hitActivity )	{	return	1.0f;	}

	virtual int		CapabilitiesGet( void );
	virtual	int		WeaponMeleeAttack1Condition( float flDot, float flDist );

protected:
	virtual	void	ImpactEffect( trace_t &trace );

private:
	bool			ChangeOnce = true;
	bool			ImpactWater( const Vector &start, const Vector &end );
	void			Swing( int bIsSecondary );
private:
	void			Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary );
	Activity		ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
};

#endif