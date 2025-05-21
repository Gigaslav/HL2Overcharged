//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef IN_BUTTONS_H
#define IN_BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_FORWARD		(1 << 3)
#define IN_BACK			(1 << 4)
#define IN_USE			(1 << 5)
#define IN_CANCEL		(1 << 6)
#define IN_LEFT			(1 << 7)
#define IN_RIGHT		(1 << 8)
#define IN_MOVELEFT		(1 << 9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_ATTACK2		(1 << 11)
#define IN_DROP			(1 << 12)
#define IN_RELOAD		(1 << 13)
#define IN_KICK			(1 << 14)
#define IN_LASERSWITCH	(1 << 15)
#define IN_SCORE		(1 << 16)   // Used by client.dll for when scoreboard is held down
#define IN_SPEED		(1 << 17)	// Player is holding the speed key
#define IN_WALK			(1 << 18)	// Player holding walk key
#define IN_ZOOM			(1 << 19)	// Zoom key for HUD zoom
#define IN_WEAPON1		(1 << 20)	// weapon defines these bits
#define IN_WEAPON2		(1 << 21)	// weapon defines these bits
#define IN_SWITCHMODE	(1 << 22)
#define IN_GRENADESWITCH	(1 << 23)	// grenade 1
#define IN_SWITCHSILENCER	(1 << 24)	// grenade 2
#define	IN_ATTACK3		(1 << 25)
#define IN_LEANLEFT		(1 << 26)   // BriJee OVR: Player Lean left
#define IN_LEANRIGHT	(1 << 27)   // BriJee OVR: Player Lean right
#define IN_SWING		(1 << 28)	// BriJee OVR: Weapon swing
#define IN_THROWGRENADE (1 << 29)	// BriJee OVR: Quick grenade throw
#define IN_AIMMODE		(1 << 30)	// BriJee OVR: Aim mode (Fistperson Ironsight, Thirdperson over the shoulder)
#define IN_FIREMODE		(1 << 31)	// BriJee OVR: Custom firemodes for guns

//#define IN_SCROLLUP		(31 >> 0)
//#define IN_SCROLLDOWN	(31 >> 1)
#endif // IN_BUTTONS_H
