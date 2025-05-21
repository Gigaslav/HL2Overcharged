//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Xen light plant.
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "ai_task.h"
#include "npcevent.h"
#include "bitstring.h"
#include "eventqueue.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "npcevent.h"
#include "game.h"
#include "Sprite.h"
#include "soundenvelope.h"
#include "hl2_gamerules.h"

#include "tier0/memdbgon.h"


#define XEN_PLANT_GLOW_SPRITE		"sprites/glow06.vmt" //08 06 04 //yellowglow1 //"sprites/flare1.vmt"
#define XEN_PLANT_HIDE_TIME			5

class CXenLightPlant : public CAI_BaseNPC
{
	DECLARE_CLASS(CXenLightPlant, CAI_BaseNPC);

public:

	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity *pOther);
	void		Think(void);

	void		LightOn(void);
	void		LightOff(void);

	float		m_flDmgTime;

	DECLARE_DATADESC();

private:
	CSprite		*m_pGlow;
};

LINK_ENTITY_TO_CLASS(npc_xenlight, CXenLightPlant);		// BriJee : Name of xen light entity

BEGIN_DATADESC(CXenLightPlant)
DEFINE_FIELD(m_pGlow, FIELD_CLASSPTR),
DEFINE_FIELD(m_flDmgTime, FIELD_FLOAT),
DEFINE_THINKFUNC(Think),
DEFINE_ENTITYFUNC(Touch),
END_DATADESC()

void CXenLightPlant::Spawn(void)
{
	Precache();

	SetModel("models/xen_light.mdl");	// BriJee : New model name

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);

	//SetHullType(HULL_TINY);
	//SetHullSizeSmall();

	UTIL_SetSize(this, Vector(-80, -80, 0), Vector(80, 80, 32));
	SetActivity(ACT_IDLE);
	SetNextThink(gpGlobals->curtime + 0.1);
	SetCycle(random->RandomFloat(0, 1));

	SetThink(&CXenLightPlant::Think);

	SetTouch(&CXenLightPlant::Touch);

	m_pGlow = CSprite::SpriteCreate(XEN_PLANT_GLOW_SPRITE, GetLocalOrigin() + Vector(0, 0, (WorldAlignMins().z + WorldAlignMaxs().z)*0.5), FALSE);
	m_pGlow->SetTransparency(kRenderGlow, GetRenderColor().r, GetRenderColor().g, GetRenderColor().b, GetRenderColor().a, m_nRenderFX);
	m_pGlow->SetAttachment(this, 1);

	if (m_pGlow)
		m_pGlow->AddEffects(EF_BRIGHTLIGHT);	// BriJee : Force 'light' around at spawn
}


void CXenLightPlant::Precache(void)
{
	PrecacheModel("models/xen_light.mdl");
	PrecacheModel(XEN_PLANT_GLOW_SPRITE);
}


void CXenLightPlant::Think(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	switch (GetActivity())
	{
	case ACT_CROUCH:
		if (IsSequenceFinished())
		{
			SetActivity(ACT_CROUCHIDLE);
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if (gpGlobals->curtime > m_flDmgTime)
		{
			SetActivity(ACT_STAND);
			LightOn();
		}
		break;

	case ACT_STAND:
		if (IsSequenceFinished())
			SetActivity(ACT_IDLE);
		break;

	case ACT_IDLE:
		break;
	case ACT_RESET:
		if (IsSequenceFinished())
			SetActivity(ACT_IDLE);
	default:
		break;
	}
}


void CXenLightPlant::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		m_flDmgTime = gpGlobals->curtime + XEN_PLANT_HIDE_TIME;
		if (GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND)
		{
			SetActivity(ACT_CROUCH);
		}
	}
}


void CXenLightPlant::LightOn(void)
{
	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOn", Value, 0, this, this);

	//============= LIGHT TEST
	/*//Vector vLightPos;
	//vThrowPos = this->EyePosition();

	//vLightPos = GetAbsOrigin(); //GetAttachment(this, 1);
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 100, &GetAbsOrigin(), 140, 140, 0, 2, 150, 1000, 0);*/

	//===================END

	if (m_pGlow)
		m_pGlow->RemoveEffects(EF_NODRAW);

	if (m_pGlow)
		m_pGlow->AddEffects(EF_BRIGHTLIGHT);		// BriJee : LOL that was SIMPLE. checked everything that was broke, now it works :3
}


void CXenLightPlant::LightOff(void)
{
	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOff", Value, 0, this, this);

	//============= LIGHT TEST
	/*Vector vLightPos;
	//vThrowPos = this->EyePosition();

	vLightPos = GetAbsOrigin(); //GetAttachment(this, 1);
	CPASFilter filter(GetAbsOrigin());
	te->DynamicLight(filter, 0.0, &vLightPos, 140, 140, 0, 2, 60, 0.08, 0);*/

	//===================END

	if (m_pGlow)
		m_pGlow->AddEffects(EF_NODRAW);

	if (m_pGlow)
		m_pGlow->RemoveEffects(EF_BRIGHTLIGHT);
}
