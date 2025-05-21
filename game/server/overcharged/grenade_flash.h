//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_FLASH_H
#define GRENADE_FLASH_H
#pragma once

class CBaseGrenade;
struct edict_t;
#include "basegrenade_shared.h"

CBaseGrenade *Flashgrenade_Create( 
const Vector &position, 
const QAngle &angles, 
const Vector &velocity, 
const AngularImpulse &angVelocity, 
CBaseCombatCharacter *pOwner, 
float timer );
#endif