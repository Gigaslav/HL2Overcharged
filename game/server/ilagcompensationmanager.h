//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ILAGCOMPENSATIONMANAGER_H
#define ILAGCOMPENSATIONMANAGER_H
#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;
class CUserCmd;

//-----------------------------------------------------------------------------
// Purpose: This is also an IServerSystem
//-----------------------------------------------------------------------------
abstract_class ILagCompensationManager
{
public:
	// Called during player movement to set up/restore after lag compensation
	virtual void	StartLagCompensation( CBasePlayer *player, CUserCmd *cmd ) = 0;
	virtual void	FinishLagCompensation( CBasePlayer *player ) = 0;
};
int GetTargetTick(CBasePlayer *player = NULL, CUserCmd *cmd = NULL);
extern ILagCompensationManager *lagcompensation;

#endif // ILAGCOMPENSATIONMANAGER_H
