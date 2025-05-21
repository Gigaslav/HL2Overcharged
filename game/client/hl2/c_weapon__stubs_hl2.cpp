//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS(cycler_weapon, WeaponCycler, C_BaseCombatWeapon);

//=========== HL2 DEFAULT ===========
STUB_WEAPON_CLASS(weapon_ar2, WeaponAR2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_frag, WeaponFrag, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_357, Weapon357, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon);
//STUB_WEAPON_CLASS(weapon_stunbaton, WeaponStunbaton, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage);
STUB_WEAPON_CLASS(weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon);

//=========== HL2 DEFAULT NPC ===========
STUB_WEAPON_CLASS(weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun);

//=========== CUSTOM Weapons based ===========
STUB_WEAPON_CLASS(weapon_ak47Custom1, WeaponAK47C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_ak47Custom2, WeaponAK47C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_ak47Custom3, WeaponAK47C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_sg552Custom1, WeaponSG552C1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_sg552Custom2, WeaponSG552C2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_sg552Custom3, WeaponSG552C3, C_HLMachineGun);

STUB_WEAPON_CLASS(weapon_ar3Custom1, WeaponAr3C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_ar3Custom2, WeaponAr3C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_ar3Custom3, WeaponAr3C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_airboatgunCustom1, WeaponAirboatGunC1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_airboatgunCustom2, WeaponAirboatGunC2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_airboatgunCustom3, WeaponAirboatGunC3, C_HLMachineGun);

STUB_WEAPON_CLASS(weapon_357Custom1, Weapon357C1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_357Custom2, Weapon357C2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_357Custom3, Weapon357C3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_alyxgunCustom1, WeaponAlyxGunC1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_alyxgunCustom2, WeaponAlyxGunC2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_alyxgunCustom3, WeaponAlyxGunC3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_annabelleCustom1, WeaponAnnabelleC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_annabelleCustom2, WeaponAnnabelleC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_annabelleCustom3, WeaponAnnabelleC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_ar2Custom1, WeaponAR2C1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_ar2Custom2, WeaponAR2C2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_ar2Custom3, WeaponAR2C3, C_HLMachineGun);

STUB_WEAPON_CLASS(weapon_crossbowCustom1, WeaponCrossbowC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crossbowCustom2, WeaponCrossbowC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crossbowCustom3, WeaponCrossbowC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_crowbarCustom1, WeaponCrowbarC1, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_crowbarCustom2, WeaponCrowbarC2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_crowbarCustom3, WeaponCrowbarC3, C_BaseHLBludgeonWeapon);

STUB_WEAPON_CLASS(weapon_fragCustom1, WeaponFragC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_fragCustom2, WeaponFragC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_fragCustom3, WeaponFragC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_pistolCustom1, WeaponPistolC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistolCustom2, WeaponPistolC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistolCustom3, WeaponPistolC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_rpgCustom1, WeaponRPGC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rpgCustom2, WeaponRPGC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rpgCustom3, WeaponRPGC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_shotgunCustom1, WeaponShotgunC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shotgunCustom2, WeaponShotgunC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shotgunCustom3, WeaponShotgunC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_smg1Custom1, WeaponSMG1C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg1Custom2, WeaponSMG1C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg1Custom3, WeaponSMG1C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_autoshotgunCustom1, WeaponAutoShotgunC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_autoshotgunCustom2, WeaponAutoShotgunC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_autoshotgunCustom3, WeaponAutoShotgunC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_berettaCustom1, WeaponBerettaC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_berettaCustom2, WeaponBerettaC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_berettaCustom3, WeaponBerettaC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_cguardCustom1, WeaponCGuardC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_cguardCustom2, WeaponCGuardC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_cguardCustom3, WeaponCGuardC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_combinesniperCustom1, WeaponCombineSniperC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_combinesniperCustom2, WeaponCombineSniperC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_combinesniperCustom3, WeaponCombineSniperC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_deagleCustom1, WeaponDeagleC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_deagleCustom2, WeaponDeagleC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_deagleCustom3, WeaponDeagleC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_displacerCustom1, WeapondisplacerC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_displacerCustom2, WeapondisplacerC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_displacerCustom3, WeapondisplacerC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_dualpistolsCustom1, WeaponDual_PistolsC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_dualpistolsCustom2, WeaponDual_PistolsC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_dualpistolsCustom3, WeaponDual_PistolsC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_egonCustom1, WeaponEgonC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_egonCustom2, WeaponEgonC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_egonCustom3, WeaponEgonC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_m4Custom1, WeaponM4C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_m4Custom2, WeaponM4C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_m4Custom3, WeaponM4C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_m16Custom1, WeaponM16C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_m16Custom2, WeaponM16C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_m16Custom3, WeaponM16C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_machinegunCustom1, WeaponMachinegunC1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_machinegunCustom2, WeaponMachinegunC2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_machinegunCustom3, WeaponMachinegunC3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_mp5Custom1, WeaponMp5C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_mp5Custom2, WeaponMp5C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_mp5Custom3, WeaponMp5C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_oicwCustom1, WeaponOICWC1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_oicwCustom2, WeaponOICWC2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_oicwCustom3, WeaponOICWC3, C_HLMachineGun);

STUB_WEAPON_CLASS(weapon_glockCustom1, WeaponGlockC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_glockCustom2, WeaponGlockC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_glockCustom3, WeaponGlockC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_iceaxeCustom1, WeaponIceaxeC1, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_iceaxeCustom2, WeaponIceaxeC2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_iceaxeCustom3, WeaponIceaxeC3, C_BaseHLBludgeonWeapon);

STUB_WEAPON_CLASS(weapon_gaussCustom1, WeaponGaussC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_gaussCustom2, WeaponGaussC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_gaussCustom3, WeaponGaussC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_grenadelauncherCustom1, WeaponGLC1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_grenadelauncherCustom2, WeaponGLC2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_grenadelauncherCustom3, WeaponGLC3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_immolatorCustom1, WeaponImmolatorC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_immolatorCustom2, WeaponImmolatorC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_immolatorCustom3, WeaponImmolatorC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_ionrifleCustom1, WeaponIonrifleC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_ionrifleCustom2, WeaponIonrifleC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_ionrifleCustom3, WeaponIonrifleC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_laserCustom1, WeaponLaserC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_laserCustom2, WeaponLaserC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_laserCustom3, WeaponLaserC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_knifeCustom1, WeaponKnifeC1, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_knifeCustom2, WeaponKnifeC2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_knifeCustom3, WeaponKnifeC3, C_BaseHLBludgeonWeapon);

STUB_WEAPON_CLASS(weapon_shockCustom1, WeaponShockRifleC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shockCustom2, WeaponShockRifleC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shockCustom3, WeaponShockRifleC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_smg2Custom1, WeaponSMG2C1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg2Custom2, WeaponSMG2C2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg2Custom3, WeaponSMG2C3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_sniperCustom1, WeaponSniperC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sniperCustom2, WeaponSniperC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sniperCustom3, WeaponSniperC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_sporeCustom1, WeaponSporeLauncherC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sporeCustom2, WeaponSporeLauncherC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sporeCustom3, WeaponSporeLauncherC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_stunbatonCustom1, WeaponStunBatonC1, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_stunbatonCustom2, WeaponStunBatonC2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_stunbatonCustom3, WeaponStunBatonC3, C_BaseHLBludgeonWeapon);

STUB_WEAPON_CLASS(weapon_tauCustom1, WeaponTauC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_tauCustom2, WeaponTauC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_tauCustom3, WeaponTauC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_uspCustom1, WeaponUSPC1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_uspCustom2, WeaponUSPC2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_uspCustom3, WeaponUSPC3, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_vectorCustom1, WeaponVectorC1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_vectorCustom2, WeaponVectorC2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_vectorCustom3, WeaponVectorC3, C_HLSelectFireMachineGun);

STUB_WEAPON_CLASS(weapon_wrenchCustom1, WeaponWrenchC1, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_wrenchCustom2, WeaponWrenchC2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_wrenchCustom3, WeaponWrenchC3, C_BaseHLBludgeonWeapon);

STUB_WEAPON_CLASS(weapon_m40a1Custom1, WeaponSniperM40A1C1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_m40a1Custom2, WeaponSniperM40A1C2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_m40a1Custom3, WeaponSniperM40A1C3, C_BaseHLCombatWeapon);

//=========== OVERCHARGED based ===========

//Melee based
STUB_WEAPON_CLASS(weapon_barnacle, WeaponBarnacle, C_BaseHLCombatWeapon);				// OP4 Hand Barnacle remake
STUB_WEAPON_CLASS(weapon_wrench, WeaponWrench, C_BaseHLBludgeonWeapon);					// OP4 Wrench remake
STUB_WEAPON_CLASS(weapon_thumper, WeaponThumper, C_BaseHLCombatWeapon);					// portable thumper										// RE ACTIVATED. MOSTLY GAMESCRIPT WEAPONS
STUB_WEAPON_CLASS(weapon_grapple, WeaponGrapple, C_BaseHLCombatWeapon);					// Grapple Hook. test weapon
STUB_WEAPON_CLASS(weapon_flamethrower, Weaponflamethrower, C_HLSelectFireMachineGun);	// Flame Thrower
STUB_WEAPON_CLASS(weapon_knife, WeaponKnife, C_BaseHLBludgeonWeapon);					// CS knife remake
STUB_WEAPON_CLASS(weapon_iceaxe, WeaponIceaxe, C_BaseHLBludgeonWeapon);					// Beta ice axe

//Explosives
STUB_WEAPON_CLASS(weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_hopmine, Weapon_HopMine, C_BaseHLCombatWeapon);				// combine hopmine player weapon	// custom
STUB_WEAPON_CLASS(weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon);					// hopwire from ep1 unused beta
STUB_WEAPON_CLASS(weapon_smokegrenade, WeaponSmokeGrenade, C_BaseHLCombatWeapon);		// smoke grenade. hurt enemy, don't hurt player.
STUB_WEAPON_CLASS(weapon_flashbang, WeaponFlashbang, C_BaseHLCombatWeapon);				// flash grenade. most like cs 1.6
STUB_WEAPON_CLASS(weapon_tripwire, Weapon_Tripwire, C_BaseHLCombatWeapon);				// 2019 tripwire hl2beta added
STUB_WEAPON_CLASS(weapon_molotov, WeaponMolotov, C_BaseHLCombatWeapon);					// Beta molotov

//Energy based
STUB_WEAPON_CLASS(weapon_airboatgun, WeaponAirboatGun, C_HLMachineGun);					// Airboat Gun
STUB_WEAPON_CLASS(weapon_ar3, WeaponAr3, C_HLMachineGun);								// Bunker Gun
STUB_WEAPON_CLASS(weapon_particlerifle, WeaponParticleRifle, C_BaseHLCombatWeapon);		// Unused
STUB_WEAPON_CLASS(weapon_immolator, WeaponImmolator, C_BaseHLCombatWeapon);				// Cremator's gun
STUB_WEAPON_CLASS(weapon_laser, WeaponLaser, C_BaseHLCombatWeapon);						// Black Mesa experimental laser gun	// Custom
STUB_WEAPON_CLASS(weapon_combinesniper, WeaponCombineSniper, C_BaseHLCombatWeapon);		// Combine sniper rifle from EP2 remake
STUB_WEAPON_CLASS(weapon_ionrifle, WeaponIonRifle, C_BaseHLCombatWeapon);				// Ion Rifle from EP2 Remake, based on CGUARD
STUB_WEAPON_CLASS(weapon_egon, WeaponEgon, C_BaseHLCombatWeapon);						// HL1 EGON port
STUB_WEAPON_CLASS(weapon_gauss_overhaul, WeaponTaussOver, C_BaseHLCombatWeapon);		// HL1 additional gauss
STUB_WEAPON_CLASS(weapon_cguard, WeaponCguard, C_BaseHLCombatWeapon);					// Combine guard's gun
STUB_WEAPON_CLASS(weapon_gauss, WeaponGauss, C_BaseHLCombatWeapon);						// HL1 gauss
STUB_WEAPON_CLASS(weapon_tau, WeaponTau, C_BaseHLCombatWeapon);							// HL2 gauss
STUB_WEAPON_CLASS(weapon_displacer, Weapondisplacer, C_BaseHLCombatWeapon);				// HL1 op4 displacer

//Projectile based
STUB_WEAPON_CLASS(weapon_grenadelauncher, WeaponGrenadelauncher, C_HLSelectFireMachineGun);	// smg1 grenade on main attack.
STUB_WEAPON_CLASS(weapon_ShockRifle, WeaponShockRifle, C_BaseHLCombatWeapon);			// OP4 Shock Rifle remake
STUB_WEAPON_CLASS(weapon_spore_launcher, WeaponSporeLauncher, C_BaseHLCombatWeapon);	// OP4 Spore Launcher
STUB_WEAPON_CLASS(weapon_HornetGun, WeaponHornetGun, C_BaseHLCombatWeapon);				// HL1 hornet gun
STUB_WEAPON_CLASS(weapon_flaregun, Flaregun, C_BaseHLCombatWeapon);						// Beta flare gun

//Bullet based
STUB_WEAPON_CLASS(weapon_dual_pistols, WeaponDual_Pistols, C_BaseHLCombatWeapon);		// Ovr dual pistols
STUB_WEAPON_CLASS(weapon_sniper, WeaponSniper, C_BaseHLCombatWeapon);					// Beta sniper rifle
STUB_WEAPON_CLASS(weapon_glock, WeaponGlock, C_BaseHLCombatWeapon);						// pistol with 3rd firemode on alt attack
STUB_WEAPON_CLASS(weapon_autoshotgun, WeaponAutoShotgun, C_BaseHLCombatWeapon);			// automatic shotgun
STUB_WEAPON_CLASS(weapon_ak47, WeaponAK47, C_HLSelectFireMachineGun);					// adjusted smg1, 2 fire modes
STUB_WEAPON_CLASS(weapon_mp5, WeaponMP5, C_HLSelectFireMachineGun);						// SMG2 Remake? But better
STUB_WEAPON_CLASS(weapon_deagle, WeaponDeagle, C_BaseHLCombatWeapon);					// OP4 Deagle remake
STUB_WEAPON_CLASS(weapon_machinegun, WeaponMachineGun, C_HLSelectFireMachineGun);		// M249 from CS remake
STUB_WEAPON_CLASS(weapon_beretta, WeaponBeretta, C_BaseHLCombatWeapon);					// beretta from HL1 remake
STUB_WEAPON_CLASS(weapon_oicw, WeaponOICW, C_HLMachineGun);								// HL2 Beta OICW remake
STUB_WEAPON_CLASS(weapon_smg2, WeaponSMG2, C_HLSelectFireMachineGun);					// Beta smg2
STUB_WEAPON_CLASS(weapon_usp, WeaponUSP, C_BaseHLCombatWeapon);							// silenced pistol. stealth weapon
STUB_WEAPON_CLASS(weapon_m4, WeaponM4, C_HLSelectFireMachineGun);						// silenced rifle. stealth weapon
STUB_WEAPON_CLASS(weapon_vector, WeaponVector, C_HLSelectFireMachineGun);				// silenced smg. stealth weapon
STUB_WEAPON_CLASS(weapon_sg552, WeaponSG552, C_HLMachineGun);							// sg550 CS remake
STUB_WEAPON_CLASS(weapon_sniper_m40a1, WeaponSniperM40A1, C_BaseHLCombatWeapon);		// OP4 sniper remake
STUB_WEAPON_CLASS(weapon_m16, WeaponM16, C_HLSelectFireMachineGun);						// HL1 hecu m16 remake

//Other types	
STUB_WEAPON_CLASS(weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage);	// 2021 LOST COAST harpoon added
STUB_WEAPON_CLASS(weapon_objective, WeaponObjective, C_BaseHLCombatWeapon);				// Objective weapon. For script moments like place c4 on the wall.		// RE ACTIVATED. MOSTLY GAMESCRIPT WEAPONS
STUB_WEAPON_CLASS(weapon_teleport, WeaponTeleport, C_BaseHLCombatWeapon);				// Portable teleport weapon												// RE ACTIVATED. MOSTLY GAMESCRIPT WEAPONS
STUB_WEAPON_CLASS(weapon_adrenaline, WeaponAdrenaline, C_BaseHLCombatWeapon);			// Portable healthkit
STUB_WEAPON_CLASS(weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon);			// Beta binoculars
STUB_WEAPON_CLASS(weapon_extinguisher, WeaponExtinguisher, C_HLSelectFireMachineGun);	// Updated fire extinguisher
STUB_WEAPON_CLASS(weapon_snark, WeaponSnark, C_BaseHLCombatWeapon);						// HL1 snark monster
//======================================================================================
