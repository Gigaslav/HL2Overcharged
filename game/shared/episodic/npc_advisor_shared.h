//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared data between client and server side npc_advisor classes.
//
// Catchphrase: "It's advising us!!!"
//
//=============================================================================//

#ifndef NPC_ADVISOR_SHARED_H
#define NPC_ADVISOR_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#ifndef CLIENT_DLL
#include "ai_default.h"
#endif
// Set this to 0 to disable the advisor's special AI behavior (all that object chucking), 
// which we did in Ep2 to make him a scripted creature.

#define NPC_ADVISOR_HAS_BEHAVIOR 0
#define NPC_ADVISOR_COMBAT_HAS_BEHAVIOR 1
//#if NPC_ADVISOR_HAS_BEHAVIOR
// Message ID constants used for communciation between client and server.
enum 
{
	ADVISOR_MSG_START_BEAM = 10,
	ADVISOR_MSG_STOP_BEAM,
	ADVISOR_MSG_STOP_ALL_BEAMS,
	ADVISOR_MSG_START_ELIGHT,
	ADVISOR_MSG_STOP_ELIGHT,
};
//#endif
#ifndef CLIENT_DLL
//
// Custom schedules.
//
enum
{
	SCHED_ADVISOR_COMBAT = LAST_SHARED_SCHEDULE,
	SCHED_ADVISOR_IDLE_STAND,
	SCHED_ADVISOR_TOSS_PLAYER
};


//
// Custom tasks.
//
enum
{
	TASK_ADVISOR_FIND_OBJECTS = LAST_SHARED_TASK,
	TASK_ADVISOR_LEVITATE_OBJECTS,
	TASK_ADVISOR_STAGE_OBJECTS,
	TASK_ADVISOR_BARRAGE_OBJECTS,

	TASK_ADVISOR_PIN_PLAYER,
};

//
// Custom conditions.
//
enum
{
	COND_ADVISOR_PHASE_INTERRUPT = LAST_SHARED_CONDITION,
};
#endif

#endif // NPC_ADVISOR_SHARED_H
