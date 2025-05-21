#ifndef WEAPON_INSTANCE_H
#define WEAPON_INSTANCE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
//Combat projectile base weapons
#include "basehlcombatweapon.h"
#include "weapon_rpg.h"
#include "weapon_ar2.h"
#include "weapon_machinegun.h"
#include "weapon_ak47.h"
#include "weapon_flaregun.h"
//Combat melee base weapons
#include "weapon_wrench.h"
//#include "weapon_iceaxe.h"
//#include "weapon_knife.h"
#include "weapon_shovel.h"
#include "weapon_stunbaton.h"
//Combat throwing base weapons
//#include "weapon_slam.h"
#include "weapon_tripwire.h"
#include "weapon_molotov.h"
//Combat energy base weapons
#include "weapon_cguard.h"
#include "weapon_displacer.h"
#include "weapon_gauss.h"
#include "weapon_egon.h"
#include "weapon_immolator.h"
#include "weapon_laser.h"
//Combat unique base weapons
#include "weapon_barnacle.h"
#include "weapon_grapple.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//TODO: fill with base classes
class CWeaponInstanceBase ://WIP
	public CWeaponMachineGun,
	public CWeaponLaser,
	public CWeaponImmolator,
	public CWeaponGrapple,
	public CWeaponEgon,
	//public CWeapondisplacer,
	public CWeaponCGuard,
	public CWeaponbarnacle,
	public CWeaponRPG,
	public CWeaponAR2,
	public CWeaponTauCannon,
	public CWeaponAK47,
	public CFlaregun,
	public CWeaponWrench,
	//public CWeaponIceaxe,
	//public CWeaponKnife,
	public CWeaponshovel,
	public CWeaponStunBaton,
	//public CWeapon_SLAM,
	public CWeapon_Tripwire,
	public CWeaponMolotov
{
	//DECLARE_DATADESC();
protected:
	/*DECLARE_CLASS(CWeaponInstanceBase,
	CWeaponMachineGun,
		CWeaponLaser,
		CWeaponImmolator,
		CWeaponGrapple,
		CWeaponEgon,
		//CWeapondisplacer,
		CWeaponCGuard,
		CWeaponbarnacle,
		CWeaponRPG,
		CWeaponAR2,
		CWeaponTauCannon,
		CWeaponAK47,
		CFlaregun,
		CWeaponWrench,
		//CWeaponIceaxe,
		//CWeaponKnife,
		CWeaponshovel,
		CWeaponStunBaton,
		//CWeapon_SLAM,
		CWeapon_Tripwire,
		CWeaponMolotov);*/

	CWeaponInstanceBase()
	{
		//TODO: parse script dictionary to stateMachine
	};

	virtual void SelectState(int state)
	{
		//TODO: here we have to map our state to weapon's script name
	}

	//TODO: Here we need to call base class method assigned by state machine
	virtual void Precache(void)
	{
		switch (stateMachine)
		{
		case 0:
			//BaseClass::Precache();
			break;
		case 1:
			CWeaponRPG::Precache();
			break;
		case 2:
			CWeaponAR2::Precache();
			break;
		case 3:
			CWeaponTauCannon::Precache();
			break;
		case 4:
			CWeaponAK47::Precache();
			break;
		case 5:
			CFlaregun::Precache();
			break;
		case 6:
			CWeaponWrench::Precache();
			break;
		case 7:
			//CWeaponIceaxe::Precache();
			break;
		case 8:
			//CWeaponKnife::Precache();
			break;
		case 9:
			CWeaponshovel::Precache();
			break;
		case 10:
			CWeaponStunBaton::Precache();
			break;
		case 11:
			//CWeapon_SLAM::Precache();
			break;
		case 12:
			CWeapon_Tripwire::Precache();
			break;
		case 13:
			CWeaponMolotov::Precache();
			break;
		default:
			//BaseClass::Precache();
			break;
		}
	};

	virtual const char *GetWorldModel(void) const
	{
		const char *WorldModelName = "";//CHLSelectFireMachineGun::GetWorldModel();

		switch (stateMachine)
		{
		case 0:
			//WorldModelName = CHLSelectFireMachineGun::GetWorldModel();
			break;
		case 1:
			WorldModelName = CWeaponRPG::GetWorldModel();
			break;
		case 2:
			WorldModelName = CWeaponAR2::GetWorldModel();
			break;
		case 3:
			WorldModelName = CWeaponTauCannon::GetWorldModel();
			break;
		case 4:
			WorldModelName = CWeaponAK47::GetWorldModel();
			break;
		case 5:
			WorldModelName = CFlaregun::GetWorldModel();
			break;
		case 6:
			WorldModelName = CWeaponWrench::GetWorldModel();
			break;
		case 7:
			//WorldModelName = CWeaponIceaxe::GetWorldModel();
			break;
		case 8:
			//WorldModelName = CWeaponKnife::GetWorldModel();
			break;
		case 9:
			WorldModelName = CWeaponshovel::GetWorldModel();
			break;
		case 10:
			WorldModelName = CWeaponStunBaton::GetWorldModel();
			break;
		case 11:
			//WorldModelName = CWeapon_SLAM::GetWorldModel();
			break;
		case 12:
			WorldModelName = CWeapon_Tripwire::GetWorldModel();
			break;
		case 13:
			WorldModelName = CWeaponMolotov::GetWorldModel();
			break;
		default:
			//WorldModelName = CHLSelectFireMachineGun::GetWorldModel();
			break;
		}
		return WorldModelName;
	};

public:

	//DECLARE_SERVERCLASS();
protected:
	int stateMachine;
};



#endif