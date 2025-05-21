//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_PARSE_H
#define WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "OverCharged/WeaponsType.h"

class IFileSystem;

typedef unsigned short WEAPON_FILE_INFO_HANDLE;

// -----------------------------------------------------------
// Weapon sound types
// Used to play sounds defined in the weapon's classname.txt file
// This needs to match pWeaponSoundCategories in weapon_parse.cpp
// ------------------------------------------------------------
typedef enum {
	EMPTY,
	SINGLE,
	SINGLE_NPC,
	WPN_DOUBLE, // Can't be "DOUBLE" because windows.h uses it.
	DOUBLE_NPC,
	BURST,
	RELOAD,
	RELOAD_NPC,
	MELEE_MISS,
	MELEE_HIT,
	MELEE_HIT_WORLD,
	SPECIAL1,
	SPECIAL2,
	SPECIAL3,
	TAUNT,
	DEPLOY,
	LOW_AMMO,
	// Add new shoot sound types here

	NUM_SHOOT_SOUND_TYPES,
} WeaponSound_t;

int GetWeaponSoundFromString( const char *pszString );

#define MAX_SHOOT_SOUNDS	16			// Maximum number of shoot sounds per shoot type

#define MAX_WEAPON_STRING	80
#define MAX_WEAPON_PREFIX	16
#define MAX_WEAPON_AMMO_NAME		32

#define WEAPON_PRINTNAME_MISSING "!!! Missing printname on weapon"

class CHudTexture;
class KeyValues;

struct AnimationData
{
	bool	 ZeroIdleAnim;
	bool	 UseSpecialAnimations;
	bool	 UseIronSightAnimations;
	bool	 ReloadWaitWhileAnimation;
	bool	 UsePistolRefireChecker;
	bool	 UseHalfFullMagAnimations;
	Activity FirstDrawAnimation;
	Activity DryFireAnimation;
	Activity DrawAnimation;
	Activity DrawSilencedAnimation;
	Activity DrawEmptyAnimation;
	Activity DrawEmptySilencedAnimation;
	Activity CustomIdleAnimOnIronSightOrScope;
	Activity ThrowPullUp;
	Activity ThrowPullDown;
	Activity ThrowPrimary;
	Activity ThrowSecondary;
	Activity IronsightIdle;
	Activity IdleAlmostEmpty;
	Activity IdleFullEmpty;
	Activity IdleAnimation;
	Activity WallAlmostEmpty;
	Activity WallFullEmpty;
	Activity RunAlmostEmpty;
	Activity RunFullEmpty;
	Activity WalkAlmostEmpty;
	Activity WalkFullEmpty;
	Activity IronAlmostEmpty;
	Activity IronFullEmpty;
	Activity LoweredAlmostEmpty;
	Activity LoweredFullEmpty;
	Activity LoweredAnimation;
	Activity HolsterAlmostEmpty;
	Activity HolsterFullEmpty;
	Activity FirePrimaryTwoRounds;
	Activity FirePrimaryAlmostEmpty;
	Activity FirePrimaryCompleteEmpty;
	Activity FireIronsightedTwoRounds;
	Activity FireIronsightedAlmostEmpty;
	Activity FireIronsightedCompleteEmpty;
	Activity FireScopedTwoRounds;
	Activity FireScopedAlmostEmpty;
	Activity FireScopedCompleteEmpty;
	Activity FirePrimary1;
	Activity FirePrimary2;
	Activity FirePrimary3;
	Activity FirePrimary4;
	Activity FirePrimarySilenced1;
	Activity FirePrimarySilenced2;
	Activity FirePrimarySilenced3;
	Activity FirePrimarySilenced4;
	Activity FirePrimarySpecial;
	Activity FireIronsighted1;
	Activity FireIronsighted2;
	Activity FireIronsighted3;
	Activity FireIronsighted4;
	Activity FireIronsightedSilenced1;
	Activity FireIronsightedSilenced2;
	Activity FireIronsightedSilenced3;
	Activity FireIronsightedSilenced4;
	Activity FireScoped1;
	Activity FireScoped2;
	Activity FireScoped3;
	Activity FireScoped4;
	Activity FireScopedSilenced1;
	Activity FireScopedSilenced2;
	Activity FireScopedSilenced3;
	Activity FireScopedSilenced4;
	Activity FireSecondary1;
	Activity FireSecondary2;
	Activity FireSecondary3;
	Activity FireSecondary4;
	Activity FireSecondaryLast;
	Activity FireSecondaryCharging;
	Activity FireSecondaryIronsighted1;
	Activity FireSecondaryIronsighted2;
	Activity FireSecondaryIronsighted3;
	Activity FireSecondaryIronsighted4;
	Activity InspectAnimation;
	Activity InspectSilencedAnimation;
	Activity OverhaulAnimation;
	Activity ShotgunFireAnimation;
	Activity ShotgunPumpAnimation;
	Activity ShotgunDoublePumpAnimation;
	Activity ShotgunStartReloadAnimation;
	Activity ShotgunFillClipAnimation;
	Activity ShotgunStopReloadAnimation;
	Activity RunAnimation;
	Activity WalkAnimation;
	Activity WallAnimation;
	Activity ReloadMidEmptyAnimation;
	Activity ReloadFullEmptyAnimation;
	//Activity BMSEmptyReloadAnimation;
	Activity ReloadDefaultAnimation;
	Activity ReloadSilencedAnimation;
	Activity HolsterAnimation;
	Activity HolsterMode1Animation;
	Activity HolsterMode2Animation;
	Activity HolsterMode3Animation;
	Activity PumpAnimation;
	Activity MeleeIdle;
	Activity MeleeAttack1;
	Activity MeleeAttack2;
	Activity MeleeSwing1;
	Activity MeleeSwing2;
	Activity MeleeMiss1;
	Activity MeleeMiss2;

	Activity WeaponSwitchAnim;
	Activity SwingAnim;
	//Activity WeaponSwitchAnim2;
	//Activity WeaponSwitchAnim3;
};
//-----------------------------------------------------------------------------
// Purpose: Contains the data read from the weapon's script file. 
// It's cached so we only read each weapon's script file once.
// Each game provides a CreateWeaponInfo function so it can have game-specific
// data (like CS move speeds) in the weapon script.
//-----------------------------------------------------------------------------
class FileWeaponInfo_t
{
public:

	FileWeaponInfo_t();
	
	// Each game can override this to get whatever values it wants from the script.
	AnimationData SetumAnimations(KeyValues *kv = NULL);
	virtual void Parse( KeyValues *pKeyValuesData, const char *szWeaponName );
	void ParseCustomData(KeyValues *pKeyValuesData, const char *szWeaponName);
	
public:	
	bool	m_bPickHint;
	char	hintText[MAX_WEAPON_STRING];

	bool	m_bUsePrimaryAmmoAsSecondary;
	bool	m_bLowAmmoNotify;
	bool	m_bCanBePickedUpByNPCs;

	char MuzzleAttachment[MAX_WEAPON_STRING];
	char MuzzleAttachment2[MAX_WEAPON_STRING];
	char MuzzleAttachmentSilencer[MAX_WEAPON_STRING];
	char MuzzleAttachmentNPC[MAX_WEAPON_STRING];
	char MuzzleAttachmentNPC2[MAX_WEAPON_STRING];
	float viewbob_model_scale;



	AnimationData animData[4];
	AnimationData animDataTP[4];

	bool		ViewModelShake;
	Activity MeleeStunbattonParticleEffects;


	int viewmodel_fov;
	int AllowShellEjectSmokeFlash;

	bool AttachmentApply; //ADDED
	CUtlVector<int> AttachmentsArray;
	CUtlVector<int> BonesArray;
	CUtlVector<float> Scales;

	int AttachmentBonesArray[MAX_WEAPON_STRING]; //ADDED
	int AttachmentBonesArraySize;
	bool Randomize; //ADDED
	float AttachmentScalesArray[MAX_WEAPON_STRING]; //ADDED
	int AttachmentScalesArraySize;
	//char muzzle_particle[MAX_WEAPON_STRING];

	bool useHl2KickSystem;
	float easyDampen; //EASY_DAMPEN
	float maxVerticalKick; //MAX_VERTICAL_KICK
	float slideLimit; //SLIDE_LIMIT

	float ironOrScopeKoefficient;
	int DoRandomPunch;
	int AutoReduce;
	float ViewPunch_X;
	float ViewPunch_Y;
	float ViewPunch_Z;

	int DoRandomKick;
	float ViewKick_X;
	float ViewKick_Y;
	float ViewKick_Z;

	float recoilVelocity;
	//Координаты настройки актуального положения оружия в руках
	Vector m_expOffsetTP; //ADDED
	QAngle m_expOriOffsetTP; //ADDED

	Vector m_expOffset; //ADDED
	QAngle m_expOriOffset; //ADDED

	Vector PositionWall; //ADDED
	QAngle AngleWall; //ADDED

	Vector PositionFwd; //ADDED
	QAngle AngleFwd; //ADDED
	Vector PositionBwd; //ADDED
	QAngle AngleBwd; //ADDED

	Vector PositionLeft; //ADDED
	QAngle AngleLeft; //ADDED
	Vector PositionRight; //ADDED
	QAngle AngleRight; //ADDED

	float DistanceToWall; //ADDED

	Vector LaserSightPosition; //ADDED
	Vector ProjectilePosition; //ADDED

	Vector ParticleTracerPosition; //ADDED
	Vector OriginTracerPosition; //ADDED
	Vector ScopeParticleTracerPosition; //ADDED
	Vector IrSightParticleTracerPosition; //ADDED
	Vector ScopeOriginTracerPosition;
	Vector IrSightOriginTracerPosition;

	Vector m_expOffset2; //ADDED//OverCharged
	QAngle m_expOriOffset2; //ADDEDOverCharged

	//Координаты настройки Scope положения оружия в руках
	Vector m_expOffsetScope; //ADDED//OverCharged
	QAngle m_expOriOffsetScope; //ADDEDOverCharged

	Vector RT_Offset; //ADDED//OverCharged
	Vector RT_OffsetIron; //ADDED//OverCharged
	QAngle RT_Angle;
	QAngle RT_AngleIron;

	bool   enableLaser;
	bool   useAsSecondaryAttack;
	char  beamAttachment[MAX_WEAPON_STRING];
	color32_s lightBeamColor;
	float lightMinRadius;
	float beamWidth;
	float beamEndWidth;
	float lightMaxRadius;
	float lightDecay;
	float lightDie;
	char  beamMaterial[MAX_WEAPON_STRING];
	char  beamHaloMaterial[MAX_WEAPON_STRING];
	float beamHaloWidth;
	float beamHaloViewModelWidth;
	float lightVMMaxRadius;
	float lightVMMinRadius;
	Vector beamLaserPos; //ADDED//OverCharged
	QAngle beamLaserAng; //ADDEDOverCharged
	char  beamOnSound[MAX_WEAPON_STRING];
	char  beamOffSound[MAX_WEAPON_STRING];
	float beamLength;
	float	flIronsightFOVOffset;//ADDED

	int AllowIronSight;
	int  AllowFreeAim;
	int m_iPlayerDamage;
	int ViewmodelRollFix;
	Vector Mag_Offset;
	Vector Mag_Velocity;
	Vector Mag_AngularVelocity;
	QAngle Mag_Angle;
	char   Mag_Attachment[MAX_WEAPON_STRING];

	char  magHitSound[MAX_WEAPON_STRING];
	bool	m_bHasMag;
	char	iMagName[MAX_WEAPON_STRING];
	
	float fireRate;
	float meleeRange;
	float meleeHull;
	//int	  WeaponModes[3];
	//Vector	  WeaponModes;
	CUtlVector<int> WeaponModes;
	int	  WeaponDefaultMode;
	float SwingHitTime;
	float SwingDamage;
	bool m_bFiresUnderwater;
	bool m_bAltFiresUnderwater;
	int	  NumShots;
	bool  WeaponUsingSilencer;
	bool  allowAmmoRegen;
	float ammoEmptyDelay;
	float ammoRegenDelay;
	float ammoRemoveDelay;
	char  silencerBodyGroup[MAX_WEAPON_STRING];
	int	  silencerBGEnableValue;
	int	  silencerBGDisableValue;

	int	  npcMinBurst;
	int   npcMaxBurst;
	//char  WeaponType[MAX_WEAPON_STRING];

	WeaponType weaponType;

	char  WeaponSwitchingMode1Sound[MAX_WEAPON_STRING];
	char  WeaponSwitchingMode2Sound[MAX_WEAPON_STRING];
	char  WeaponSwitchingMode3Sound[MAX_WEAPON_STRING];

	float mode_fire_rate_single;
	float mode_fire_rate_automatic_startup_delay;
	float mode_fire_rate_automatic;
	bool  AllowShotgunSecondaryAttack;
	float mode_fire_rate_burst;
	float mode_fire_rate_burst_refire_time;
	int   mode_fire_burst_count;
	float mode_accuracy_maximum_penalty_time;
	float mode_fastest_refire_time;
	float mode_accuracy_shot_penalty_time;
	float mode_fastest_dry_refire_time;
	

	bool  PlayerDisableSprint;
	float PlayerSpeedWalkKoefficient;
	float PlayerSpeedRunKoefficient;
	float PlayerSpeedCrouchKoefficient;

	bool					bParsedScript;
	bool					bLoadedHudElements;

// SHARED
	char					szClassName[MAX_WEAPON_STRING];
	char					szPrintName[MAX_WEAPON_STRING];			// Name for showing in HUD, etc.

	char					szViewModel[MAX_WEAPON_STRING];			// View model of this weapon
	char					szAdditionalViewModel[MAX_WEAPON_STRING];			// View model of this weapon
	char					szWorldModel[MAX_WEAPON_STRING];		// Model of this weapon seen carried by the player
	char					szAnimationPrefix[MAX_WEAPON_STRING];//MAX_WEAPON_PREFIX	// Prefix of the animations that should be used by the player carrying this weapon

	char					szSilentModel[MAX_WEAPON_STRING];		// BriJee OVR: Silent w_ model.

	//char                 iMuzzleFlashType[512];
	//char					iMuzzleFlashType[MAX_WEAPON_STRING];
	string_t				iWeaponZoomUpSound;

	int						indexP;
	const char				*precacheParticles;


	float					ScopeSightSpread;
	float					irSightSpread;
	float					standingSpread;
	float					crouchSpread;
	float					walkingSpread;
	float					runningSpread;

	float					standingSpreadNpc;
	float					crouchSpreadNpc;
	float					walkingSpreadNpc;
	float					runningSpreadNpc;

	char					iWeaponShellAttachment[MAX_WEAPON_STRING];
	string_t				iWeaponShellModel;
	string_t				iWeaponShellBounceSound;
	string_t				iWeaponShellHitSound;
	string_t				iWeaponShellParticle;
	string_t				iWeaponShellExhaustAttachment;
	string_t				iWeaponShellExhaustParticle;
	string_t				iWeaponShellExhaustAttachmentDual1;
	string_t				iWeaponShellExhaustAttachmentDual2;
	bool					BulletOverride;
	float					bulletPenetrationLength;
	float					BulletImpulseForce;
	float					BulletSpeed;
	int						indexT;
	int						externalIndexT;
	string_t				iMuzzleTracerType[MAX_WEAPON_STRING];

	bool					TracersCombined;
	char					iOldMuzzleTracerType[MAX_WEAPON_STRING];
	float					iOldMuzzleTracerLengthMin;
	float					iOldMuzzleTracerLengthMax;
	float					iOldMuzzleTracerScaleMin;
	float					iOldMuzzleTracerScaleMax;
	float					iOldMuzzleTracerVelocityMin;
	float					iOldMuzzleTracerVelocityMax;
	bool					iOldMuzzleTracerWhiz;
	int						iAttachment;
	string_t				iAttachment_second;
	string_t				iAttachment_sil;

	int						indexTN;
	int						externalIndexTN;
	string_t				iMuzzleTracerNpcType[MAX_WEAPON_STRING];

	bool					TracersCombinedNpc;
	char					iOldMuzzleTracerTypeNPC[MAX_WEAPON_STRING];
	float					iOldMuzzleTracerLengthMinNPC;
	float					iOldMuzzleTracerLengthMaxNPC;
	float					iOldMuzzleTracerScaleMinNPC;
	float					iOldMuzzleTracerScaleMaxNPC;
	float					iOldMuzzleTracerVelocityMinNPC;
	float					iOldMuzzleTracerVelocityMaxNPC;
	bool					iOldMuzzleTracerWhizNPC;

	int						index;
	int						externalIndex;
	string_t				iMuzzleFlashType[MAX_WEAPON_STRING];

	string_t				iMuzzleFlashUnderWaterType;

	int						indexS;
	int						externalIndexS;
	string_t				iMuzzleFlashTypeS[MAX_WEAPON_STRING];

	string_t				iMuzzleFlashDelayed;
	char					iSecondaryAttachment[MAX_WEAPON_STRING];
	//string_t				iMuzzleFlashTransparentType;
	string_t				iMuzzleFlashShortBurst;
	string_t				iMuzzleSmoke;
	int						iMuzzleSmokeNumShots;
	int						oldFlashType;
	bool					oldFlashTypeTP;

	int						indexNpc;
	int						externalIndexNpc;
	string_t				iMuzzleFlashTypeNpc[MAX_WEAPON_STRING];
	int						oldFlashTypeNPC;

	string_t				iMuzzleFlashUnderWaterTypeNpc;

	//bool					allowMuzzleLight = false;
	int						indexLight;
	int						externalIndexLight;
	string_t				iMuzzleLightType[MAX_WEAPON_STRING];

	int						indexSLight;
	int						externalIndexSLight;
	string_t				iMuzzleSecondaryLightType[MAX_WEAPON_STRING];
	//char					iMuzzleFlashTypeNpc[MAX_WEAPON_STRING];

	bool					AllowMuzzleFlashDLight;
	bool					DLightDisableForPlayer;
	float					iMuzzleFlashLightRadius;
	float					iMuzzleFlashLightDecayTime;
	float					iMuzzleFlashLightLifeTime;
	int						iMuzzleFlashLightR;
	int						iMuzzleFlashLightG;
	int						iMuzzleFlashLightB;
	bool					iMuzzleFlashLightNoModelIllumination;
	int						iMuzzleFlashLightExponent;
	string_t				iMuzzleFlashLightAttachment;

	bool					AllowMuzzleFlashDLightNpc;
	float					iMuzzleFlashLightRadiusNpc;
	float					iMuzzleFlashLightDecayTimeNpc;
	float					iMuzzleFlashLightLifeTimeNpc;
	int						iMuzzleFlashLightRNpc;
	int						iMuzzleFlashLightGNpc;
	int						iMuzzleFlashLightBNpc;
	bool					iMuzzleFlashLightNoModelIlluminationNpc;
	int						iMuzzleFlashLightExponentNpc;
	string_t				iMuzzleFlashLightAttachmentNpc;


	int						iSlot;									// inventory slot.
	bool					m_bReservedSlot;
	int						iPosition;								// position in the inventory slot.
	int						iMaxClip1;								// max primary clip size (-1 if no clip)
	int						iMaxClip2;								// max secondary clip size (-1 if no clip)
	int						iDefaultClip1;							// amount of primary ammo in the gun when it's created
	int						iDefaultClip2;							// amount of secondary ammo in the gun when it's created

	bool					allowTwoDScope;
	string_t				twoDScopeTexture;

	int						minScopeFov;
	int						maxScopeFov;
	int						rtFov;
	char					rtAttachment[MAX_WEAPON_STRING];
	color24					rtColor;
	int						AllowEnablingLight;
	int						AllowEnablingIronsightLight;
	float					lightConstant;
	float					lightLinear;
	float					lightFov;
	float					lightFar;
	float					ironlightConstant;
	float					ironlightLinear;
	float					ironlightFov;
	float					ironlightFar;

	bool					AllowProjectedMuzzleFlashes;
	float					iFlashLifeTime;//OverCharged
	int					    iFlashType;//OverCharged
	float					iFlashXPos;
	float					iFlashYPos;
	float					iFlashZPos;
	char					iFlashFirstAttachment[MAX_WEAPON_STRING];
	char					iFlashSecondAttachment[MAX_WEAPON_STRING];
	bool					iFlashRollPrimary;
	bool					iFlashRollSecondary;


//	int					    iFlashLightType;//OverCharged
//	int					    iFlashLightTypeNPC;//OverCharged
//	int					    iTracerType;//OverCharged

	int						iWeight;								// this value used to determine this weapon's importance in autoselection.
	int						iRumbleEffect;							// Which rumble effect to use when fired? (xbox)
	bool					bAutoSwitchTo;							// whether this weapon should be considered for autoswitching to
	bool					bAutoSwitchFrom;						// whether this weapon can be autoswitched away from when picking up another weapon or ammo
	int						iFlags;									// miscellaneous weapon flags
	char					szAmmo1[MAX_WEAPON_AMMO_NAME];			// "primary" ammo type
	char					szAmmo2[MAX_WEAPON_AMMO_NAME];			// "secondary" ammo type

	// Sound blocks
	char					aShootSounds[NUM_SHOOT_SOUND_TYPES][MAX_WEAPON_STRING];	
	bool					allowLoopSound;
	bool					allowLoopSilencerSound;
	char					ShootLoopingSound[MAX_WEAPON_STRING];
	char					ShootLoopingSilencerSound[MAX_WEAPON_STRING];
	char					EndLoopingSound[MAX_WEAPON_STRING];
	char					EndLoopingSilencerSound[MAX_WEAPON_STRING];
	float					LoopSoundBurstPitchMultiplier;
	float					LoopSoundSlowMoPitchDivider;
	float					LoopSilencerSoundSlowMoPitchDivider;

	int						iAmmoType;
	int						iAmmo2Type;
	bool					m_bMeleeWeapon;		// Melee weapons can always "fire" regardless of ammo.

	// This tells if the weapon was built right-handed (defaults to true).
	// This helps cl_righthand make the decision about whether to flip the model or not.
	bool					m_bBuiltRightHanded;
	bool					m_bAllowFlipping;	// False to disallow flipping the model, regardless of whether
												// it is built left or right handed.

// CLIENT DLL
	// Sprite data, read from the data file
	int						iSpriteCount;
	CHudTexture						*iconActive;
	CHudTexture	 					*iconInactive;
	CHudTexture 					*iconAmmo;
	CHudTexture 					*iconAmmo2;
	CHudTexture 					*iconCrosshair;
	CHudTexture 					*iconAutoaim;
	CHudTexture 					*iconZoomedCrosshair;
	CHudTexture 					*iconZoomedAutoaim;
	CHudTexture						*iconSmall;

// TF2 specific
	bool					bShowUsageHint;							// if true, then when you receive the weapon, show a hint about it

// SERVER DLL
	/////////////////////////////////////////OverCharged//////////////////////////////////////////////////////////
	char				szCameraAttachmentName[32];				// defaults to 'muzzle'
	char				szCameraBoneName[32];					// not used by default, overwrites attachment

	float				flCameraMovementScale;
	float				flCameraMovementReferenceCycle;
	QAngle				angCameraMovementOrientation;
	QAngle				angCameraMovementOffset;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

// The weapon parse function
bool ReadWeaponDataFromFileForSlot( IFileSystem* filesystem, const char *szWeaponName, 
	WEAPON_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey = NULL );

// If weapon info has been loaded for the specified class name, this returns it.
WEAPON_FILE_INFO_HANDLE LookupWeaponInfoSlot( const char *name );

FileWeaponInfo_t *GetFileWeaponInfoFromHandle( WEAPON_FILE_INFO_HANDLE handle );
WEAPON_FILE_INFO_HANDLE GetInvalidWeaponInfoHandle( void );
void PrecacheFileWeaponInfoDatabase( IFileSystem *filesystem, const unsigned char *pICEKey );

// Read a possibly-encrypted KeyValues file in. 
// If pICEKey is NULL, then it appends .txt to the filename and loads it as an unencrypted file.
// If pICEKey is non-NULL, then it appends .ctx to the filename and loads it as an encrypted file.
//
// (This should be moved into a more appropriate place).
//
KeyValues* ReadEncryptedKVFile( IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey, bool bForceReadEncryptedFile = false );


// Each game implements this. It can return a derived class and override Parse() if it wants.
extern FileWeaponInfo_t* CreateWeaponInfo();


#endif // WEAPON_PARSE_H
