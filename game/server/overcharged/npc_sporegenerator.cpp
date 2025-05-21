//========= Copyright © 2019 Overcharged ============//
//
// Purpose: op4 spore generation entity
//
//
//	Основная логика воссоздана, что осталось:
//	1)Сделать условие если у игрока полный боезапас спор то возвращаем (return) функцию touch чтобы модель споры не реагировала
//	2)При попадании в модель споры чтобы спавнилась реальная ентитя споры и вылетала от аттачмента 'spore' вперёд от модели
//	при этом вызывая стандартную функцию touch (она уже уберёт бодигруппу и проиграет анимацию)
//	3)Вписать новый аммотип в функции touch()
//	
//	Все заметки помечены тегом OVER
//
//  ATTACHMENT 'spore'
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
#include "soundenvelope.h"
#include "hl2_gamerules.h"
#include "ammodef.h"
#include "npc_playercompanion.h"
#include "tier0/memdbgon.h"
#include "grenade_bullsquid.h"
#include "grenade_frag.h"
#include "grenade_strooper_instant.h"
ConVar oc_sporegenerator_regenerate_delay("oc_sporegenerator_regenerate_delay", "0.5", FCVAR_REPLICATED, "Spore generator regenerate delay.");
ConVar oc_sporegenerator_projectile_impulse("oc_sporegenerator_projectile_impulse", "20.0", FCVAR_ARCHIVE);

class CSporeGenerator : public CAI_BaseNPC//CNPC_PlayerCompanion//CNPC_PlayerCompanion
{
	DECLARE_CLASS(CSporeGenerator, CAI_BaseNPC);

public:

	DECLARE_DATADESC();

	virtual Class_T Classify();

	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity *pOther);
	void		Think(void);
	void		SpitOut(void);
	virtual int			OnTakeDamage(const CTakeDamageInfo &info);
	void		SporePick(void);
	void		SporeReGenerate(void);
	void		MyTouch(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void		TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	float		m_flDmgTime;
	float		Time;
	int			bodygroup1;
	int			activity;
	CSporeLauncherBouncedProjectile *Sphere; // Null
};

LINK_ENTITY_TO_CLASS(npc_sporegenerator, CSporeGenerator);		// BriJee: Spore generator link

BEGIN_DATADESC(CSporeGenerator)
DEFINE_FIELD(m_flDmgTime, FIELD_FLOAT),
DEFINE_FIELD(bodygroup1, FIELD_INTEGER),
DEFINE_FIELD(activity, FIELD_INTEGER),
DEFINE_FIELD(Time, FIELD_FLOAT),
DEFINE_THINKFUNC(Think),
DEFINE_USEFUNC(MyTouch),
DEFINE_ENTITYFUNC(Touch),
END_DATADESC()

Class_T	CSporeGenerator::Classify()
{
	return CLASS_NONE;
}

void CSporeGenerator::Spawn(void)
{
	Precache();

	//======= BriJee: Different models & anims
	if (random->RandomInt(0, 3) == 3)
	{
		SetModel("models/RaceX/spore_generator_up.mdl");
	}
	else
	{
		SetModel("models/RaceX/spore_generator.mdl");
	}
	//=====end

	SetBodygroup(1, 1);	// spore bodygroup enable by default
	bodygroup1 = GetBodygroup(1);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);	// full physics model. old _BBOX
	//AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);		// BriJee: unused no collision
	Time = cvar->FindVar("oc_sporegenerator_regenerate_delay")->GetFloat();
	SetHullType(HULL_TINY);
	SetHullSizeSmall();

	SetBloodColor(BLOOD_COLOR_YELLOW);

	NPCInit();
	m_takedamage = DAMAGE_EVENTS_ONLY;
	
	SetUse(&CSporeGenerator::MyTouch);
	SetTouch(&CSporeGenerator::Touch);
	SetThink(&CSporeGenerator::Think);
	UTIL_SetSize(this, Vector(-10, -10, -10), Vector(10, 10, 10));
	SetActivity(ACT_IDLE);
	activity = ACT_IDLE;
	SetNextThink(gpGlobals->curtime + 0.1);
	SetCycle(random->RandomFloat(0, 1));
}

void CSporeGenerator::Precache(void)
{
	PrecacheModel("models/RaceX/spore_generator.mdl");
	PrecacheModel("models/RaceX/spore_generator_up.mdl");
	PrecacheScriptSound("SporeGenerator.PickSpore");
}
void CSporeGenerator::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{

	/*if (GetActivity() == ACT_IDLE && GetBodygroup(1) == 1)
		SpitOut();*/
	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

int CSporeGenerator::OnTakeDamage(const CTakeDamageInfo &info)
{
	if (GetActivity() == ACT_IDLE && GetBodygroup(1) == 1)
		SpitOut();

	return 0;
}

void CSporeGenerator::Think(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	/*if (GetActivity() == 0)//LOAD game fix
	{
		SetActivity((Activity)activity);
	}
	if (GetActivity() == ACT_CROUCH)
	{
		if (IsSequenceFinished())
		{
			activity = ACT_CROUCHIDLE;
			SetActivity((Activity)activity);
			SporeReGenerate();

			m_flDmgTime = gpGlobals->curtime + (SequenceDuration()*Time);
		}
	}
	else if (GetActivity() == ACT_CROUCHIDLE)
	{
		if (m_flDmgTime <= gpGlobals->curtime)//(gpGlobals->curtime > m_flDmgTime)
		{
			SporePick();
		}
	}
	else if (GetActivity() == ACT_STAND)
	{
		if (IsSequenceFinished())
		{
			activity = ACT_IDLE;
			SetActivity((Activity)activity);
		}
	}*/

	switch (GetActivity())
	{
	case ACT_RESET:
		{
			SetActivity((Activity)activity);
		}
		break;
	case ACT_CROUCH:
		if (IsSequenceFinished())
		{
			activity = ACT_CROUCHIDLE;
			SetActivity((Activity)activity);
			SporeReGenerate();

			m_flDmgTime = gpGlobals->curtime + (SequenceDuration()*Time);
		}
		break;
	case ACT_CROUCHIDLE:
		{
			if (m_flDmgTime <= gpGlobals->curtime)//(gpGlobals->curtime > m_flDmgTime)
			{
				SporePick();
			}
		}
		break;
	case ACT_STAND:
		if (IsSequenceFinished())
		{
			activity = ACT_IDLE;
			SetActivity((Activity)activity);
		}
		break;

	default:
		break;
	}
}
void CSporeGenerator::MyTouch(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Touch(pActivator);
}

void CSporeGenerator::Touch(CBaseEntity *pOther)
{
	// OVER Если у игрока полный боезапас спор (проверяем по аммотипу) то return; 
	// трогать ентитю при этом не нужно чтобы не вызывать последующие функции
	//DevMsg("Player stand on spore \n");
	if (pOther->IsPlayer())
	{
		if (pOther->MyCombatCharacterPointer()->GetAmmoCount("SporeAcid") == GetAmmoDef()->MaxCarry(GetAmmoDef()->Index("SporeAcid")))
			return;

		//m_flDmgTime = gpGlobals->curtime + cvar->FindVar("sk_sporegenerator_regenerate_delay")->GetInt();								// BriJee: 7 seconds to refill spore
		if (GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND)
		{
			bodygroup1 = 0;
			SetBodygroup(1, bodygroup1);	//disable spore bg

			activity = ACT_CROUCH;
			SetActivity((Activity)activity);

			pOther->MyCombatCharacterPointer()->GiveAmmo(1, "SporeAcid", false);
			// OVER Тут даём игроку 1 снаряд споры, вместо "Pistol" вписать новое название
			//CBasePlayer *pPlayer;
			//pPlayer = (CBasePlayer *)pOther;
			//pPlayer->GiveAmmo(1, "Pistol", false);	// 1 спора, "аммотип споры", иконка справа при подборе
		}
		return;
	}

}

void CSporeGenerator::SpitOut(void)
{
	bodygroup1 = 0;
	SetBodygroup(1, bodygroup1);	//disable spore bg

	activity = ACT_CROUCH;
	SetActivity((Activity)activity);

	Vector start, out;
	QAngle ang;
	GetAttachment("spore", start, ang);

	Vector start2 = GetAbsOrigin();
	start2.x += 200;

	/*Sphere = CSporeLauncherBouncedProjectile::SporeCreate(GetAbsOrigin(), ang, 0, this, start);	
	Sphere->SetModelScale(1);*/

	CBaseEntity *pGren = NULL;
	pGren = CBaseEntity::Create("spore_launcher_bounced_projectile", start, ang); // Creates
	pGren->PrecacheModel("models/spitball_medium.mdl");
	pGren->SetModel("models/spitball_medium.mdl");
	pGren->SetMoveType(MOVETYPE_VPHYSICS);

	Vector res, vel;

	/*if (abs(start.x) > abs(GetAbsOrigin().x))
	{
		if (start.x < 0 && GetAbsOrigin().x > 0)
			res.x = start.x - GetAbsOrigin().x;
		else if (start.x > 0 && GetAbsOrigin().x < 0)
			res.x = start.x - GetAbsOrigin().x;
		else if (start.x > 0 && GetAbsOrigin().x > 0)
			res.x = start.x - GetAbsOrigin().x;
		else if (start.x == 0 || GetAbsOrigin().x == 0)
			res.x = start.x - GetAbsOrigin().x;
		else if (start.x == 0 && GetAbsOrigin().x == 0)
			res.x = start.x - GetAbsOrigin().x;
	}
	else if (start.x < GetAbsOrigin().x)
	{

	}*/


	res.x = start.x - GetAbsOrigin().x;
	res.y = start.y - GetAbsOrigin().y;
	res.z = start.z - GetAbsOrigin().z;

	/*res.x = start.x > GetAbsOrigin().x ? start.x - GetAbsOrigin().x : GetAbsOrigin().x - start.x;
	res.y = start.y > GetAbsOrigin().y ? start.y - GetAbsOrigin().y : GetAbsOrigin().y - start.y;
	res.z = start.z > GetAbsOrigin().z ? start.z - GetAbsOrigin().z : GetAbsOrigin().z - start.z;*/

	vel = res * cvar->FindVar("oc_sporegenerator_projectile_impulse")->GetFloat();

	pGren->SetAbsVelocity(vel);
	pGren->SetDamage(cvar->FindVar("sk_npc_dmg_spore_acid_bounce")->GetFloat());
	pGren->Spawn();
}
void CSporeGenerator::SporePick(void)
{
	bodygroup1 = 1;
	SetBodygroup(1, bodygroup1);	//enable spore bg

	activity = ACT_STAND;
	SetActivity((Activity)activity);

	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOn", Value, 0, this, this);
}

void CSporeGenerator::SporeReGenerate(void)
{
	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOff", Value, 0, this, this);

	CPASAttenuationFilter filter(this);							// BriJee: Pick up spore sound	// closed // after pick up
	EmitSound(filter, entindex(), "SporeGenerator.PickSpore");
}
