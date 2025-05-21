//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEPLAYER_SHARED_H
#define BASEPLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// PlayerUse defines
#define	PLAYER_USE_RADIUS	80.f
#define CONE_45_DEGREES		0.707f
#define CONE_15_DEGREES		0.9659258f
#define CONE_90_DEGREES		0

#define TRAIN_ACTIVE	0x80 
#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

// entity messages
#define PLAY_PLAYER_JINGLE	1
#define UPDATE_PLAYER_RADAR	2

#define DEATH_ANIMATION_TIME	3.0f

#define PLAYER_MSG_DO_RECHARGE		4
#define PLAYER_MSG_STOP_RECHARGE	5
#define PLAYER_MSG_DO_HEALTH_REGEN	6
#define PLAYER_MSG_STOP_HEALTH_REGEN	7
#define PLAYER_MSG_DO_DMG_SHOCK		8
#define PLAYER_MSG_DO_DMG_BULLET	9
#define PLAYER_MSG_DO_BATTERY		10
#define PLAYER_MSG_DO_HEALTH		11

#ifdef MAPBASE
#define SF_PLAYER_SUPPRESS_FIRING	(1 << 18)
#endif

typedef struct 
{
	Vector		m_vecAutoAimDir;		// The direction autoaim wishes to point.
	Vector		m_vecAutoAimPoint;		// The point (world space) that autoaim is aiming at.
	EHANDLE		m_hAutoAimEntity;		// The entity that autoaim is aiming at.
	bool		m_bAutoAimAssisting;	// If this is true, autoaim is aiming at the target. If false, the player is naturally aiming.
	bool		m_bOnTargetNatural;		
	float		m_fScale;
	float		m_fMaxDist;
} autoaim_params_t;

enum stepsoundtimes_t
{
	STEPSOUNDTIME_NORMAL = 0,
	STEPSOUNDTIME_ON_LADDER,
	STEPSOUNDTIME_WATER_KNEE,
	STEPSOUNDTIME_WATER_FOOT,
};

void CopySoundNameWithModifierToken( char *pchDest, const char *pchSource, int nMaxLenInChars, const char *pchToken );

// Shared header file for players
#if defined( CLIENT_DLL )
#define CBasePlayer C_BasePlayer
#include "c_baseplayer.h"
#else
#include "player.h"
#endif

#endif // BASEPLAYER_SHARED_H
