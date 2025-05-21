//========= Copyright © 2021, Overcharged.  =================================//
//
// Purpose: Cremator cut enemy remake
//
// $NoKeywords: $
//=============================================================================//
#pragma warning(disable:4706)
#include "cbase.h"
#include "npc_cremator.h"
#include "weapon_physcannon.h"

#define SF_CREMATOR_NO_GRENADES					1<<23
#define SF_CREMATOR_NO_FUEL_SPILLING			1<<24 
#define SF_CREMATOR_NO_PATROL_BEHAVIOUR			1<<25
#define CREMATOR_SKIN_ALERT				0 // yellow eyes
#define CREMATOR_SKIN_CALM				1 // blue eyes
#define CREMATOR_SKIN_ANGRY				2 // red eyes
#define CREMATOR_SKIN_DEAD				3 // black eyes
#define CREMATOR_IMMOLATOR_RANGE		370
#define	CREMATOR_AE_IMMO_START			( 6 )
#define	CREMATOR_AE_IMMO_PARTICLE		( 7 )
#define	CREMATOR_AE_IMMO_PARTICLEOFF	( 8 )
#define CREMATOR_AE_THROWGRENADE		( 9 )
#define CREMATOR_AE_SPECIAL_START		( 10 )
#define CREMATOR_AE_SPECIAL_MIDDLE		( 11 ) // bad name?
#define CREMATOR_AE_SPECIAL_END			( 12 )
#define CREMATOR_AE_RELOAD				( 15 )
#define CREMATOR_AE_FLLEFT				( 98 )
#define CREMATOR_AE_FLRIGHT				( 99 )
#define	CREMATOR_BEAM_ATTACH			1
#define CREMATOR_BURN_TIME				20
#define CREMATOR_DETECT_CORPSE_RANGE	300.0

enum
{
	SCHED_CREMATOR_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE + 100,
	SCHED_CREMATOR_RANGE_ATTACK2,
	SCHED_CREMATOR_CHASE_ENEMY,
	SCHED_CREMATOR_PATROL,
	SCHED_CREMATOR_INVESTIGATE_CORPSE,
};
enum
{
	TASK_CREMATOR_RANGE_ATTACK1 = LAST_SHARED_TASK + 1,
	TASK_CREMATOR_RANGE_ATTACK2,
	TASK_CREMATOR_RELOAD,
	TASK_CREMATOR_INVESTIGATE_CORPSE,
	TASK_CREMATOR_BURN_CORPSE,
};
enum
{
	COND_CREMATOR_OUT_OF_AMMO = LAST_SHARED_CONDITION + 1,
	COND_CREMATOR_ENEMY_WITH_HIGHER_PRIORITY,
	COND_CREMATOR_FOUND_CORPSE, // Cremator was patrolling the streets and found a corpse
};

CSound *pInterestSound;
CBaseEntity *pCorpse;
CBaseEntity *pCorpseLast;
CBaseEntity *pEnemy;

LINK_ENTITY_TO_CLASS(npc_cremator, CNPC_Cremator);

ConVar	sk_cremator_health("sk_cremator_health", "180");
ConVar	sk_cremator_firedamage("sk_cremator_firedamage", "0");
ConVar	sk_cremator_radiusdamage("sk_cremator_radiusdamage", "0");
ConVar  sk_cremator_corpse_search_radius("sk_cremator_corpse_search_radius", "1520");
ConVar  sk_cremator_attackplayeronsight("sk_cremator_attackplayeronsight", "0");

Activity ACT_FIRESPREAD;
Activity ACT_FIREIDLE;
Activity ACT_CREMATOR_ARM;
Activity ACT_CREMATOR_DISARM;
Activity ACT_CREMATOR_RELOAD;

BEGIN_DATADESC(CNPC_Cremator)
DEFINE_FIELD(m_bIsFiring, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextRangeAttack1Time, FIELD_TIME),
DEFINE_FIELD(distance, FIELD_FLOAT),
DEFINE_FIELD(DoFireUp, FIELD_BOOLEAN),
DEFINE_FIELD(m_breath, FIELD_BOOLEAN),
DEFINE_FIELD(m_iAmmo, FIELD_INTEGER),
DEFINE_FIELD(m_bHeadshot, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsPlayerEnemy, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsNPCEnemy, FIELD_BOOLEAN),
DEFINE_FIELD(m_bPlayAngrySound, FIELD_BOOLEAN),
END_DATADESC();

CNPC_Cremator::CNPC_Cremator(void)
{
	m_bIsBodygrouped = true;
	m_iBodyGroup = 1;
	m_iBodyGroupValue = 1;
}

void CNPC_Cremator::Precache()
{
	PrecacheModel("models/Cremator_over.mdl");
	//PrecacheModel("models/Cremator_headprop.mdl");
	PrecacheModel(GUNMODEL);

	PrecacheParticleSystem("vapor_ray");

	PrecacheScriptSound("NPC_Cremator.NPCAlert");
	PrecacheScriptSound("NPC_Cremator.PlayerAlert");
	PrecacheScriptSound("NPC_Cremator.BreathingAmb");
	PrecacheScriptSound("NPC_Cremator.AngryAmb");
	PrecacheScriptSound("NPC_Cremator.DeathAmb");
	PrecacheScriptSound("Weapon_Immolator.Single");
	PrecacheScriptSound("Weapon_Immolator.Stop");

	PrecacheMaterial(CREMATOR_BEAM_SPRITE);
	PrecacheParticleSystem("vapor_ray");
	PrecacheParticleSystem("immolator_normal");
	PrecacheParticleSystem("immolator_sparks01");
	PrecacheParticleSystem("flamethrower");
	PrecacheParticleSystem("flamethrower_orange");
	PrecacheParticleSystem("weapon_muzzle_smoke");
	beamIndex = PrecacheModel("sprites/bluelaser1.vmt");

	PrecacheScriptSound("NPC_Cremator.NPCAlert");
	PrecacheScriptSound("NPC_Cremator.PlayerAlert");
	PrecacheScriptSound("NPC_Cremator.BreathingAmb");
	PrecacheScriptSound("NPC_Cremator.AngryAmb");
	PrecacheScriptSound("NPC_Cremator.DeathAmb");
	PrecacheScriptSound("Weapon_Immolator.Single");
	PrecacheScriptSound("Weapon_Immolator.Stop");
	PrecacheScriptSound("Weapon_Immolator.Start");
	enginesound->PrecacheSound("npc/cremator/amb_loop.wav");
	enginesound->PrecacheSound("npc/cremator/amb1.wav");
	enginesound->PrecacheSound("npc/cremator/amb2.wav");
	enginesound->PrecacheSound("npc/cremator/amb3.wav");
	enginesound->PrecacheSound("npc/cremator/crem_die.wav");
	enginesound->PrecacheSound("npc/cremator/alert_object.wav");
	enginesound->PrecacheSound("npc/cremator/alert_player.wav");
	PrecacheScriptSound("NPC_Cremator.FootstepLeft");
	PrecacheScriptSound("NPC_Cremator.Breath");
	PrecacheScriptSound("NPC_Cremator.Mad");
	PrecacheScriptSound("NPC_Cremator.FindPlayer");
	PrecacheScriptSound("NPC_Cremator.FootstepRight");
	PrecacheScriptSound("Weapon_Immolator.Single");
	PrecacheScriptSound("Weapon_Immolator.Stop");

	CreateSounds();
	CreateBreathSound();
	BaseClass::Precache();
}
void CNPC_Cremator::Spawn(void)
{
	Precache();
	SetModel("models/Cremator_over.mdl");
	SetHullType(HULL_MEDIUM_TALL);
	SetHullSizeNormal();

	SetBodygroup(1, 0); // the gun
	//SetBodygroup(2, 0); // the head	// over: always enabled in remaster

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	m_bloodColor = BLOOD_COLOR_MECH; // TODO: basically turns blood into sparks. Need something more special.
	m_iHealth = sk_cremator_health.GetFloat();
	m_flFieldOfView = VIEW_FIELD_WIDE;

	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_DOORS_GROUP); // TODO: cremator thus can open doors but he is too tall to fit normal doors?

	NPCInit();
	distance = 128.0f;
	SetDistLook(1280);
	m_flNextRangeAttack1Time = 0.f;
	pCorpse = NULL;
	pCorpseLast = NULL;
	pEnemy = NULL;
	m_bIsFiring = false;
	m_breath = false;
	m_bIsNPCEnemy = true;
	//StartBreathSound();
	CPASAttenuationFilter filter(this, 4.f);
	enginesound->EmitSound(filter, entindex(), CHAN_VOICE, "npc/cremator/amb_loop.wav", 0.2f, ATTN_NORM);
	//m_flNextFlinchTime = gpGlobals->curtime + SequenceDuration();
	oc_ragdoll_dissolved = false;
	Q_snprintf(pCorpseName, sizeof(pCorpseName), "prop_ragdoll");
}

Disposition_t CNPC_Cremator::IRelationType(CBaseEntity *pTarget)
{
	Disposition_t disp = BaseClass::IRelationType(pTarget);
	
	if (pTarget == NULL)
		return disp;

	if (pTarget->Classify() == CLASS_COMBINE)
		return D_NU;

	if (pTarget->IsNPC() && (pTarget->Classify() == CLASS_NONE || pTarget->Classify() == CLASS_BULLSEYE))
	{
		return D_NU;
	}
	if (pTarget->Classify() == CLASS_PLAYER || pTarget->Classify() == CLASS_PLAYER_ALLY || pTarget->Classify() == CLASS_PLAYER_ALLY_VITAL)
	{
		if (sk_cremator_attackplayeronsight.GetBool())
		{
			return D_HT;
		}
		else
		{
			return m_bIsPlayerEnemy ? D_HT : D_NU;
		}
	}
	else if (pTarget->IsNPC()
		&& pTarget->Classify() != this->Classify()
		&& pTarget->Classify() != CLASS_COMBINE
		&& pTarget->Classify() != CLASS_COMBINE_HUNTER
		&& pTarget->Classify() != CLASS_COMBINE_GUNSHIP
		&& pTarget->Classify() != CLASS_MANHACK
		&& pTarget->Classify() != CLASS_METROPOLICE
		&& pTarget->Classify() != CLASS_SCANNER)
	{
		return m_bIsNPCEnemy ? D_HT : D_NU;
	}


	/*if (pTarget->IsNPC() && (pTarget->Classify() == CLASS_HEADCRAB
		|| pTarget->Classify() == CLASS_HOUNDEYE
		|| pTarget->Classify() == CLASS_ZOMBIE
		|| pTarget->Classify() == CLASS_ANTLION
		|| pTarget->Classify() == CLASS_BARNACLE
		|| pTarget->Classify() == CLASS_VORTIGAUNT))
	{
		return D_HT;
	}*/

	return disp;
}
#if 0
void CNPC_Cremator::OnListened(void)
{
	AISoundIter_t iter;

	CSound *pCurrentSound = GetSenses()->GetFirstHeardSound(&iter);

	static int ConditionsToClear[] =
	{
		COND_CREMATOR_DETECT_INTEREST,
		COND_CREMATOR_DETECT_NEW_INTEREST,
	};

	ClearConditions(ConditionsToClear, ARRAYSIZE(ConditionsToClear));

	while (pCurrentSound)
	{
		if (!pCurrentSound->FIsSound())
		{
			if (pCurrentSound->m_iType & SOUND_CARCASS | SOUND_MEAT)
			{
				pInterestSound = pCurrentSound;
				Msg("Cremator smells a carcass\n");
				SetCondition(COND_CREMATOR_FOUND_CORPSE);
			}
		}

		pCurrentSound = GetSenses()->GetNextHeardSound(&iter);
	}

	BaseClass::OnListened();
}
#endif
void CNPC_Cremator::SelectSkin(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		m_nSkin = CREMATOR_SKIN_ANGRY;

		break;
	}
	case NPC_STATE_ALERT:
	{
		m_nSkin = CREMATOR_SKIN_ALERT;
		break;
	}
	case NPC_STATE_IDLE:
	{
		m_nSkin = CREMATOR_SKIN_CALM;
		break;
	}
	case NPC_STATE_DEAD:
	{
		m_nSkin = CREMATOR_SKIN_DEAD;
		break;
	}
	default:
		m_nSkin = CREMATOR_SKIN_CALM;
		break;
	}
}
float CNPC_Cremator::MaxYawSpeed(void)
{
	float flYS = 0;

	switch (GetActivity())
	{
	case	ACT_WALK_HURT:		flYS = 30;	break;
	case	ACT_RUN:			flYS = 90;	break;
	case	ACT_IDLE:			flYS = 90;	break;
	case	ACT_RANGE_ATTACK1:	flYS = 30;	break;
	default:
		flYS = 90;
		break;
	}
	return flYS;
}
void CNPC_Cremator::AlertSound(void)
{
	switch (random->RandomInt(0, 1))
	{
	case 0:
		EmitSound("NPC_Cremator.NPCAlert");
		break;
	case 1:
		EmitSound("NPC_Cremator.PlayerAlert");
		break;
	}
}
void CNPC_Cremator::IdleSound(void)
{
	int randSay = random->RandomInt(0, 2);
	if (randSay == 2)
	{
		if (m_bPlayAngrySound)
		{
			EmitSound("NPC_Cremator.AngryAmb");
		}
		else
		{
			EmitSound("NPC_Cremator.BreathingAmb");
		}
	}
	EmitSound("NPC_Cremator.ClothAmb");
}
void CNPC_Cremator::DeathSound(const CTakeDamageInfo &info)
{
	//EmitSound("NPC_Cremator.DeathAmb");
	CPASAttenuationFilter filter(this);
	int iPitch = random->RandomInt(90, 105);
	enginesound->EmitSound(filter, entindex(), CHAN_VOICE, "npc/cremator/crem_die.wav", 1, ATTN_NORM, 0, iPitch);
}

int CNPC_Cremator::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	if (info.GetAttacker())
	{
		if (info.GetAttacker()->IsPlayer())
		{
			m_bIsPlayerEnemy = true;
			m_bPlayAngrySound = true;
		}
		else if (info.GetAttacker()->IsNPC() && info.GetAttacker()->Classify() != Classify())
		{
			m_bIsNPCEnemy = true;
			m_bPlayAngrySound = true;
		}
	}

	/*if (GetActivity() == ACT_WALK && IsMoving() && m_iHealth < (sk_cremator_health.GetFloat() * 0.5))
	SetActivity(ACT_WALK_HURT);*/

	if (info.GetDamage() >= 20.f)//(infoCopy.GetDamageType() & DMG_BUCKSHOT)
	{
		//infoCopy.ScaleDamage(0.625);
		EmitSound("NPC_Cremator.Mad");
		SetActivity(ACT_SMALL_FLINCH);
	}

	return BaseClass::OnTakeDamage_Alive(info);
}

void CNPC_Cremator::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	CTakeDamageInfo infoCopy = info;

	if (ptr->hitgroup == HITGROUP_HEAD)
	{
		m_bHeadshot = true;
	}

	if (infoCopy.GetDamageType() & DMG_BUCKSHOT)
	{
		infoCopy.ScaleDamage(0.625);
	}

	if (infoCopy.GetDamageType() & DMG_DISSOLVE)
	{
		infoCopy.ScaleDamage(0.05);
	}
	/*if (GetActivity() == ACT_WALK && IsMoving() && m_iHealth < (sk_cremator_health.GetFloat() * 0.5))
	SetActivity(ACT_WALK_HURT);*/

	/*if (infoCopy.GetDamage() >= 20.f)//(infoCopy.GetDamageType() & DMG_BUCKSHOT)
	{
	//infoCopy.ScaleDamage(0.625);
	SetActivity(ACT_SMALL_FLINCH);
	}*/

	BaseClass::TraceAttack(infoCopy, vecDir, ptr, pAccumulator);
}
void CNPC_Cremator::Event_Killed(const CTakeDamageInfo &info)
{
	DeathSound(info);
	DestroyEffect();
	DoFireUp = false;
	StopFiring();
	StopBreathSound();
	this->StopLoopingSounds();

	Vector Gun;
	QAngle Ang;
	GetAttachment(IMMOLATOR_ATTACHMENT, Gun);
	//VectorAngles(Gun, Ang);
	DropGun(Gun, this->GetAbsAngles());

	if (PlayerHasMegaPhysCannon())
	{
		StopParticleEffects(this);
		m_nSkin = CREMATOR_SKIN_DEAD; // turn the eyes black
		SetBodygroup(1, 1); // turn the gun off
		BaseClass::Event_Killed(info);
		return;
	}

	// over: NO head drop in overcharged
	/*
	if (m_bHeadshot && ((info.GetAmmoType() == GetAmmoDef()->Index(".50BMG")) // sniper ammo
		|| (info.GetAmmoType() == GetAmmoDef()->Index("Buckshot")) // shotgun ammo
		|| (info.GetAmmoType() == GetAmmoDef()->Index("Gauss")) // gauss ammo
		|| (info.GetAmmoType() == GetAmmoDef()->Index("XBowBolt")) // crossbow ammo
		|| (info.GetAmmoType() == GetAmmoDef()->Index("357")))) // revolver ammo
	{
		SetBodygroup(2, 1); // turn the head off
		Vector vecDamageDir = info.GetDamageForce();
		VectorNormalize(vecDamageDir);
		DropHead(50, vecDamageDir); // spawn headprop
	}

	else if (info.GetDamageType() == DMG_BLAST) // blown up
	{
		SetBodygroup(2, 1);
		DropHead(300, Vector(0, 0, 1)); // spawn headprop
	}
	*/

	StopParticleEffects(this);
	m_nSkin = CREMATOR_SKIN_DEAD; // turn the eyes black
	SetBodygroup(1, 1); // turn the gun off

	BaseClass::Event_Killed(info);
}

// Not needed anymore
/*
const char *CNPC_Cremator::GetHeadpropModel(void)
{
	return "models/cremator_headprop.mdl";
}
void CNPC_Cremator::DropHead(int iVelocity, Vector &vecVelocity)
{
	DestroyEffect();
	DoFireUp = false;
	StopFiring();
	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName("prop_physics"));
	pGib->SetModel(GetHeadpropModel());
	pGib->SetAbsOrigin(EyePosition());
	pGib->SetAbsAngles(EyeAngles());
	pGib->SetMoveType(MOVETYPE_VPHYSICS);
	pGib->SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);
	pGib->SetOwnerEntity(this);
	pGib->Spawn();
	pGib->SetAbsVelocity(vecVelocity * (iVelocity + RandomFloat(-10, 10)));
}
*/

void CNPC_Cremator::DropGun(const Vector &vecVelocity, const QAngle &angles)
{
	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName("weapon_immolator"));
	//pGib->SetModel(GUNMODEL);	// already in weapon_immolator script
	pGib->SetAbsOrigin(vecVelocity);//EyePosition()
	pGib->SetAbsAngles(angles);//EyeAngles()
	pGib->SetMoveType(MOVETYPE_VPHYSICS);
	pGib->SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS);
	pGib->SetOwnerEntity(this);
	pGib->Spawn();
	//pGib->SetAbsVelocity(vecVelocity * (iVelocity + RandomFloat(-10, 10)));
	DestroyEffect();
	DoFireUp = false;
	StopFiring();

	/*
	float flRandomVel = random->RandomFloat( -10, 10 );
	pGib->GetMassCenter( &vecDir );
	vecDir *= (iVelocity + flRandomVel) / 15;
	vecDir.z += 30.0f;
	AngularImpulse angImpulse = RandomAngularImpulse( -500, 500 );
	IPhysicsObject *pObj = pGib->VPhysicsGetObject();
	if ( pObj != NULL )
	{
	pObj->AddVelocity( &vecDir, &angImpulse );
	}
	*/
}
void CNPC_Cremator::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case CREMATOR_AE_FLLEFT:
	{
		LeftFootHit(pEvent->eventtime);
	}
	break;
	case CREMATOR_AE_FLRIGHT:
	{
		RightFootHit(pEvent->eventtime);
	}
	break;
	case CREMATOR_AE_IMMO_START: // for combat
	{
		if (m_flNextRangeAttack1Time < gpGlobals->curtime)
		{
			pEnemy = GetEnemyCombatCharacterPointer();
			Assert(pEnemy != NULL);
			if (pEnemy && (pEnemy->IsPlayer() || pEnemy->IsNPC()))
			{
				DispatchSpray(pEnemy);

			}
			else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) <= distance)
			{
				pCorpse = dynamic_cast<CBaseEntity*>(MyCombatCharacterPointer());
				DispatchSpray(pCorpse->GetAbsOrigin());
			}
			else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) > distance)
			{
				DestroyEffect();
				DoFireUp = false;
				StopFiring();
				GetNavigator()->SetGoal(corpseCoord);
				GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
			}
			else if ((pCorpse == NULL && pEnemy == NULL))
			{
				pCorpse = NULL;
				DestroyEffect();
				DoFireUp = false;
				StopFiring();
				if ((GetActivity() == ACT_FIREIDLE || GetActivity() == ACT_RANGE_ATTACK1))
				{
					SetActivity(ACT_IDLE);
					ClearCondition(COND_CREMATOR_FOUND_CORPSE);
				}
				distance = 128.0f;
				Q_snprintf(pCorpseName, sizeof(pCorpseName), "prop_ragdoll");
			}
		}
	}
	break;
	case CREMATOR_AE_IMMO_PARTICLE:
	{
		/*pEnemy = GetEnemyCombatCharacterPointer();
		Assert(pEnemy != NULL);
		if (pEnemy && (pEnemy->IsPlayer() || pEnemy->IsNPC()))
		{
			DispatchSpray(pEnemy);

		}
		else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) <= distance)
		{
			pCorpse = dynamic_cast<CBaseEntity*>(MyCombatCharacterPointer());
			DispatchSpray(pCorpse->GetAbsOrigin());
		}
		else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) > distance)
		{
			DestroyEffect();
			DoFireUp = false;
			StopFiring();
			GetNavigator()->SetGoal(corpseCoord);
			GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
		}
		else if ((pCorpse == NULL && pEnemy == NULL))
		{
			pCorpse = NULL;
			DestroyEffect();
			DoFireUp = false;
			StopFiring();
			if ((GetActivity() == ACT_FIREIDLE || GetActivity() == ACT_RANGE_ATTACK1))
			{
				SetActivity(ACT_IDLE);
				ClearCondition(COND_CREMATOR_FOUND_CORPSE);
			}
			distance = 128.0f;
			Q_snprintf(pCorpseName, sizeof(pCorpseName), "prop_ragdoll");
		}*/
	}
	break;
	case CREMATOR_AE_IMMO_PARTICLEOFF:
	{
		StopFiring();
		DoFireUp = false;
		StopParticleEffects(this);
		StopSound("Weapon_Immolator.Single");
		//EmitSound("Weapon_Immolator.Stop");
	}
	break;
	case CREMATOR_AE_RELOAD:
	{
		StopFiring();
		ClearCondition(COND_CREMATOR_OUT_OF_AMMO);

		// Put your own ints here. This defines for how long a cremator would be able to fire at an enemy
		// Cremator gets shorter bursts on lower difficulty. On Hard, it can continously fire 60 attack cycles (1 ammo per cycle)
		if (g_pGameRules->IsSkillLevel(SKILL_EASY)) { m_iAmmo += 15; }
		else if (g_pGameRules->IsSkillLevel(SKILL_MEDIUM)) { m_iAmmo += 25; }
		else if (g_pGameRules->IsSkillLevel(SKILL_HARD)) { m_iAmmo += 60; }
	}
	break;
	case CREMATOR_AE_SPECIAL_START: // for corpse removal routine
	{
		m_flNextRangeAttack1Time = gpGlobals->curtime + 1.5f;//SequenceDuration();
		EmitSound("Weapon_Immolator.Start");
		DispatchParticleEffect("Immolator_line01", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);
		DoFireUp = false;
	}
	break;
	case CREMATOR_AE_SPECIAL_MIDDLE:
	{
		DoFireUp = true;
		DispatchParticleEffect("Immolator_explodeMain", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);
		StartFiring();
	}
	break;
	case CREMATOR_AE_SPECIAL_END:
	{
		StopFiring();
		StopParticleEffects(this);
		StopSound("Weapon_Immolator.Single");
		//EmitSound("Weapon_Immolator.Stop");
	}
	break;
#if 0
	case CREMATOR_AE_THROWGRENADE:
	{
		//	DevMsg( "Throwing incendiary grenade!\n" );
		ThrowIncendiaryGrenade();

		if (g_pGameRules->IsSkillLevel(SKILL_EASY))
		{
			m_flNextRangeAttack2Time = gpGlobals->curtime + random->RandomFloat(15.0f, 30.0f);
		}
		else if (g_pGameRules->IsSkillLevel(SKILL_HARD))
		{
			m_flNextRangeAttack2Time = gpGlobals->curtime + random->RandomFloat(5.0f, 10.0f);
		}
		else
		{
			m_flNextRangeAttack2Time = gpGlobals->curtime + random->RandomFloat(10.0f, 20.0f);
		}
	}
	break;
#endif
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}
Vector CNPC_Cremator::LeftFootHit(float eventtime)
{
	Vector footPosition;

	GetAttachment("footleft", footPosition);
	//CPASAttenuationFilter filter(this);
	//EmitSound(filter, entindex(), "NPC_Cremator.FootstepLeft", &footPosition, eventtime);

	FootstepEffect(footPosition);
	return footPosition;
}
Vector CNPC_Cremator::RightFootHit(float eventtime)
{
	Vector footPosition;

	GetAttachment("footright", footPosition);
	//CPASAttenuationFilter filter(this);
	//EmitSound(filter, entindex(), "NPC_Cremator.FootstepRight", &footPosition, eventtime);

	FootstepEffect(footPosition);
	return footPosition;
}
void CNPC_Cremator::FootstepEffect(const Vector &origin)
{
	trace_t tr;
	AI_TraceLine(origin, origin - Vector(0, 0, 0), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	float yaw = random->RandomInt(0, 0);
	for (int i = 0; i < 2; i++)
	{
		if (UTIL_PointContents(tr.endpos + Vector(0, 0, 1)) & MASK_WATER)
		{
			float flWaterZ = UTIL_FindWaterSurface(tr.endpos, tr.endpos.z, tr.endpos.z + 100.0f);

			CEffectData	data;
			data.m_fFlags = 0;
			data.m_vOrigin = tr.endpos;
			data.m_vOrigin.z = flWaterZ;
			data.m_vNormal = Vector(0, 0, 1);
			data.m_flScale = random->RandomFloat(10.0, 14.0);

			DispatchEffect("watersplash", data);
		}
		else
		{
			Vector dir = UTIL_YawToVector(yaw + i * 180) * 10;
			VectorNormalize(dir);
			dir.z = 0.25;
			VectorNormalize(dir);
			g_pEffects->Dust(tr.endpos, dir, 12, 50);
		}
	}
}

void CNPC_Cremator::RunAI(void)
{

	/*if (pCorpse != NULL)
	{
		SearchForCorpses();
	}

	if (!HasCondition(COND_CREMATOR_FOUND_CORPSE))
		SearchForCorpses(); // FIXME: is it the best place for it?

	if (GetActivity() == ACT_WALK && IsMoving() && m_iHealth < (sk_cremator_health.GetFloat() * 0.5))
		SetActivity(ACT_WALK_HURT);

	if (HasCondition(COND_CREMATOR_FOUND_CORPSE) && GetActivity() == ACT_IDLE && pCorpse != NULL && pEnemy == NULL && !this->IsMoving() )
	{
		GetNavigator()->SetGoal(corpseCoord);

	}

	if (this->IsCurSchedule(SCHED_CREMATOR_INVESTIGATE_CORPSE) && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) <= distance)
	{
		GetNavigator()->StopMoving();
		GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
		distance *= 3;
	}
	if ((GetActivity() == ACT_FIREIDLE || GetActivity() == ACT_RANGE_ATTACK1) && DoFireUp)
	{
		pEnemy = GetEnemyCombatCharacterPointer();
		Assert(pEnemy != NULL);
		if (pEnemy && (pEnemy->IsPlayer() || pEnemy->IsNPC()))
		{
			DispatchSpray(pEnemy);

		}
		else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) <= distance)
		{			
			pCorpse = dynamic_cast<CBaseEntity*>(MyCombatCharacterPointer());
			DispatchSpray(pCorpse->GetAbsOrigin());
		}
		else if (pEnemy == NULL && pCorpse != NULL && GetAbsOrigin().DistTo(corpseCoord) > distance)
		{
			DestroyEffect();
			DoFireUp = false;
			StopFiring();
			GetNavigator()->SetGoal(corpseCoord);
			GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
		}
		else if ((pCorpse == NULL && pEnemy == NULL))
		{
			pCorpse = NULL;
			DestroyEffect();
			DoFireUp = false;
			StopFiring();
			if ((GetActivity() == ACT_FIREIDLE || GetActivity() == ACT_RANGE_ATTACK1))
			{
				SetActivity(ACT_IDLE);
				ClearCondition(COND_CREMATOR_FOUND_CORPSE);
			}
			distance = 128.0f;
			Q_snprintf(pCorpseName, sizeof(pCorpseName), "prop_ragdoll");
		}

		SetNextThink(gpGlobals->curtime + 0.05);
	}
	else
	{
		distance = 128.0f;
		StopFiring();
		DoFireUp = false;

		if (m_pBeam1)
		{
			UTIL_Remove(m_pBeam1);
			m_pBeam1 = NULL;
		}
		if (m_pBeam2)
		{
			UTIL_Remove(m_pBeam2);
			m_pBeam2 = NULL;
		}
		if (m_pBeam3)
		{
			UTIL_Remove(m_pBeam3);
			m_pBeam3 = NULL;
		}

		SetNextThink(gpGlobals->curtime + 0.05);
	}*/
	SetNextThink(gpGlobals->curtime + 0.05);
	BaseClass::RunAI();
}
void CNPC_Cremator::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_CREMATOR_INVESTIGATE_CORPSE:
	{
		AssertMsg(pCorpse != NULL, "The corpse the cremator was after, it's gone!\n");

		if (pCorpse != NULL)
		{
			GetNavigator()->SetGoal(pCorpse->WorldSpaceCenter());
			GetMotor()->SetIdealYawToTargetAndUpdate(corpseCoord);
			if (IsUnreachable(pCorpse))
			{
				pCorpse->SetOwnerEntity(this);	//HACKHACK: set the owner to prevent this unreachable corpse from being detected again. 

				pCorpse = NULL;						//forget about this corpse.
				ClearCondition(COND_CREMATOR_FOUND_CORPSE);
				TaskFail(FAIL_NO_ROUTE);
			}
			TaskComplete();
		}
		SetNextThink(gpGlobals->curtime + 0.01);
		break;
	}
	case TASK_CREMATOR_BURN_CORPSE:
	{
		if (!pCorpse)
		{
			TaskFail(FAIL_NO_TARGET);
			ClearCondition(COND_CREMATOR_FOUND_CORPSE);
		}
		else
		{
			GetMotor()->SetIdealYawToTarget(pCorpse->GetAbsOrigin(), 0, 0);
			SetActivity(ACT_RANGE_ATTACK1);
		}
		break;
	}
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}
void CNPC_Cremator::RunTask(const Task_t *pTask)
{
	//DevMsg("pTask->iTask: %i \n", (int)pTask->iTask);

	switch (pTask->iTask)
	{
	case TASK_CREMATOR_INVESTIGATE_CORPSE: // FIXME: that never runs!
	{
		if (pCorpse != NULL && EnemyDistance(pCorpse) <= 15.f)
		{
			GetNavigator()->StopMoving();

			TaskFail(FAIL_NO_TARGET);
		}
		SetNextThink(gpGlobals->curtime + 0.01);
		break;
	}
	case TASK_CREMATOR_BURN_CORPSE:
	{
		if (IsActivityFinished())
		{
			TaskComplete();
			ClearCondition(COND_CREMATOR_FOUND_CORPSE);

		}
		break;
	}
	case TASK_FACE_ENEMY:
	{
		if (pEnemy != NULL)
		{
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTargetAndUpdate(flEnemyLKP);
		}
		break;
	}

	case TASK_CREMATOR_RANGE_ATTACK1:
	{
		Assert(GetEnemy() != NULL);
		SetActivity(ACT_RANGE_ATTACK1);

		if (pEnemy != NULL)
		{
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTargetAndUpdate(flEnemyLKP);
		}

		if (m_iAmmo < 1 && IsActivityFinished())
		{
			SetCondition(COND_CREMATOR_OUT_OF_AMMO);
			StopParticleEffects(this);
			StopSound("Weapon_Immolator.Single");

			SetActivity(ACT_CREMATOR_RELOAD);
			TaskComplete();
			SetNextThink(gpGlobals->curtime + 0.1f);
		}
		break;
	}
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}
int CNPC_Cremator::RangeAttack1Conditions(float flDot, float flDist)
{
	if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	else if (flDist > CREMATOR_IMMOLATOR_RANGE - 100) // create a buffer between us and the target
	{
		//DestroyEffect();
		StopFiring();
		StopSound(entindex(), "Weapon_Immolator.Start");
		return COND_TOO_FAR_TO_ATTACK;
	}
	
	return COND_CAN_RANGE_ATTACK1;
}


NPC_STATE CNPC_Cremator::SelectIdealState(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		// COMBAT goes to ALERT upon death of enemy
		if (GetEnemy() == NULL)
		{
			//DestroyEffect();
			m_bPlayAngrySound = false;
			m_nSkin = CREMATOR_SKIN_CALM;
			return NPC_STATE_IDLE;
		}
		break;
	}
	case NPC_STATE_IDLE:
	{
		if (HasCondition(COND_CREMATOR_FOUND_CORPSE))
		{
			return NPC_STATE_ALERT;
		}
		break;
	}
	}
	return BaseClass::SelectIdealState();
}

int CNPC_Cremator::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_RANGE_ATTACK1:
	{
		return SCHED_CREMATOR_RANGE_ATTACK1;
		break;
	}
	case SCHED_RANGE_ATTACK2:
	{
		//DestroyEffect();
		return SCHED_CREMATOR_RANGE_ATTACK2;
		break;
	}
	case SCHED_MOVE_TO_WEAPON_RANGE:
	{
		if (m_pBeam1 || m_pBeam2 || m_pBeam3)
		{
			StopFiring();
			StopParticleEffects(this);
		}
		return SCHED_CREMATOR_CHASE_ENEMY;
		break;
	}
	}
	return BaseClass::TranslateSchedule(scheduleType);
}

//=========================================================
// SetState
//=========================================================
void CNPC_Cremator::SetState(NPC_STATE State)
{
	BaseClass::SetState(State);
}

int CNPC_Cremator::SelectSchedule(void)
{
	SelectSkin();
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
	{
		if (m_pBeam1 || m_pBeam2 || m_pBeam3)
		{
			StopFiring();
			StopParticleEffects(this);
		}

		if (!HasSpawnFlags(SF_CREMATOR_NO_PATROL_BEHAVIOUR))
			return SCHED_CREMATOR_PATROL;
	}
	case NPC_STATE_ALERT:
	{
		if (HasCondition(COND_CREMATOR_FOUND_CORPSE) && !HasCondition(COND_LIGHT_DAMAGE | COND_HEAVY_DAMAGE) && GetEnemy() == NULL)
		{
			if (m_pBeam1 || m_pBeam2 || m_pBeam3)
			{
				StopFiring();
				StopParticleEffects(this);
			}

			return SCHED_CREMATOR_INVESTIGATE_CORPSE;
		}
		else
		{
			if (m_pBeam1 || m_pBeam2 || m_pBeam3)
			{
				StopFiring();
				StopParticleEffects(this);
			}
		}
		if (!HasSpawnFlags(SF_CREMATOR_NO_PATROL_BEHAVIOUR))
			return SCHED_CREMATOR_PATROL;
	}
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			return SCHED_CREMATOR_RANGE_ATTACK1;//SCHED_RANGE_ATTACK1;
		}
		else
		{
			
			if (m_pBeam1 || m_pBeam2 || m_pBeam3)
			{
				StopFiring();
				StopParticleEffects(this);
			}
			return SCHED_CREMATOR_CHASE_ENEMY;
		}
#if 0
		if (HasCondition(COND_CAN_RANGE_ATTACK2))
		{
			return SCHED_CREMATOR_RANGE_ATTACK2;
		}
		if (HasCondition(COND_ENEMY_UNREACHABLE))
		{
			return SCHED_CREMATOR_RANGE_ATTACK2;
		}
#endif
		if (HasCondition(COND_ENEMY_DEAD))
		{
			m_bPlayAngrySound = false;
			if (m_pBeam1 || m_pBeam2 || m_pBeam3)
			{
				StopFiring();
				StopParticleEffects(this);
			}
			m_nSkin = CREMATOR_SKIN_CALM;
			return BaseClass::SelectSchedule();
		}
	}
	}
	return BaseClass::SelectSchedule();
}
void CNPC_Cremator::OnScheduleChange(void)
{
	SelectSkin();
	StopParticleEffects(this);
	//StopSound("Weapon_Immolator.Single");
	BaseClass::OnScheduleChange();
}
void CNPC_Cremator::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();
#if 0
	//if (pInterestSound && pInterestSound->m_iType & GetSoundInterests())
	{
		DevMsg("Think... Searching corpses");
		if (!HasCondition(COND_CREMATOR_FOUND_CORPSE))
			SetCondition(COND_CREMATOR_FOUND_CORPSE);
	}
#endif

	switch (m_NPCState)
	{
	case NPC_STATE_ALERT:
	{
		if (HasCondition(COND_CREMATOR_FOUND_CORPSE))
		{
			if (HasCondition(COND_LIGHT_DAMAGE | COND_HEAVY_DAMAGE | COND_REPEATED_DAMAGE))
			{
				ClearCondition(COND_CREMATOR_FOUND_CORPSE); // stop caring about stinks if we've been hit

				if (m_pBeam1 || m_pBeam2 || m_pBeam3)
				{
					StopFiring();
					StopParticleEffects(this);
				}
			}
		}
		break;
	}
	}
}


void CNPC_Cremator::SearchForCorpses(void)
{	
	DevMsg("pCorpseName: %s \n", pCorpseName);
	pCorpse = gEntList.FindEntityGenericWithin(this, pCorpseName, GetAbsOrigin(), sk_cremator_corpse_search_radius.GetFloat());//prop_ragdoll

	pCorpse = gEntList.FindEntityGenericNearest(pCorpseName, GetAbsOrigin(), sk_cremator_corpse_search_radius.GetFloat());
	
	if (pCorpse != NULL && pEnemy == NULL)
		EmitSound("NPC_Cremator.FindPlayer");

	if (pCorpse)
	{
		//char tempName[150];
		Q_snprintf(pCorpseName, sizeof(pCorpseName), "prop_ragdollCrem%d", this->entindex());
		//pCorpseName = tempName;
		pCorpse->SetClassname(pCorpseName);

		corpseCoord.x = pCorpse->GetAbsOrigin().x;
		corpseCoord.y = pCorpse->GetAbsOrigin().y;
		corpseCoord.z = pCorpse->GetAbsOrigin().z;

		SetCondition(COND_CREMATOR_FOUND_CORPSE);
	}
}
void CNPC_Cremator::IncinerateCorpse(CBaseEntity *pTarget)
{
	if (pTarget)
	{
		CEntityFlame *pFlame = CEntityFlame::Create(this);

		if (pFlame)
		{
			SetEffectEntity(NULL);
			pFlame->SetAbsOrigin(pTarget->GetAbsOrigin());
			pFlame->AttachToEntity(pTarget);
			pFlame->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
			pFlame->AddEffects(EF_BRIGHTLIGHT); // create light from the fire
			pFlame->SetLifetime(20.0); // burn for 20 seconds

			pTarget->AddFlag(FL_ONFIRE);
			pTarget->SetEffectEntity(pFlame);
			pTarget->SetRenderColor(50, 50, 50);

			pTarget->SetOwnerEntity(this); // HACKHACK - we're marking this corpse so that it won't be picked again in the future.
			//DevMsg("This corpse has been handled. Moving on\n");
		}
	}
}

void CNPC_Cremator::DispatchSpray(CBaseEntity *pEntity)
{

	/*CBaseEntity *pVictum = GetEnemy();
	if (pVictum)
		DevMsg("pVictum: %i \n", (int)pVictum->Classify());*/

	Vector vecSrc, vecAim, vecAimCorpse;
	trace_t tr;
	//DevMsg("DispatchSpray \n");

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	vecSrc = GetAbsOrigin() + up * 36;
	vecAim = GetShootEnemyDir(vecSrc);

	if (pEntity != NULL && (!pEntity->IsPlayer() && !pEntity->IsNPC()))
		vecAimCorpse = GetShootEnemyDir(pEntity->GetAbsOrigin());

	//float deflection = 0.01;
	//vecAim = vecAim + 1 * right * random->RandomFloat(0, deflection) + up * random->RandomFloat(-deflection, deflection);
	//StartFiring();

	Vector vAttachPos;
	GetAttachment(IMMOLATOR_ATTACHMENT, vAttachPos);
	Vector End = vAttachPos + (up + forward * MAX_TRACE_LENGTH);
	vecAim = GetShootEnemyDir(vAttachPos, false);
	//forward.x = pEntity->GetAbsOrigin().x;
	//forward.z = pEntity->GetAbsOrigin().z;
	//up.z = pEntity->GetAbsOrigin().z;
	if ((pEntity != NULL) && (pEntity->IsPlayer()))
	{
		//UTIL_TraceLine(vecSrc, vecSrc + vecAim * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		//AI_TraceLine(vAttachPos, right + up + forward * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		//AI_TraceLine(vAttachPos, pEntity->GetAbsOrigin() + forward * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		AI_TraceLine(vAttachPos, vecSrc + vecAim * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		//debugoverlay->AddLineOverlay(vAttachPos, End, 0, 255, 0, false, 10);
	}
	else if ((pEntity != NULL) && (pEntity->IsNPC()))
	{
		//AI_TraceLine(vecSrc, up + forward * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		AI_TraceLine(vAttachPos, vecSrc + vecAim * MAX_TRACE_LENGTH, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
	}

	pEntity = tr.m_pEnt;

	if (pEntity != NULL && m_takedamage)
	{
		ClearMultiDamage();
		CTakeDamageInfo firedamage(this, this, sk_cremator_firedamage.GetFloat(), DMG_DISSOLVE);
		CTakeDamageInfo radiusdamage(this, this, sk_cremator_radiusdamage.GetFloat(), DMG_DISSOLVE);
		CalculateMeleeDamageForce(&firedamage, vecAim, tr.endpos);
		RadiusDamage(CTakeDamageInfo(this, this, 2, DMG_DISSOLVE), // AOE; this stuff makes cremators absurdly powerfull sometimes btw
			tr.endpos,
			64.0f,
			CLASS_NONE,
			NULL);

		pEntity->DispatchTraceAttack((firedamage), vecAim, &tr);


		DestroyEffect();


		//debugoverlay->AddLineOverlay(vAttachPos, vAttachPos + forward * MAX_TRACE_LENGTH, 0, 255, 0, false, 10);
		if (m_pBeam1 == NULL)
		{
			m_pBeam1 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.5);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam1->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam1->PointEntInit(tr.endpos, this);

			//m_pBeam1->PointEntInit(End, this);
			m_pBeam1->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam1->SetBrightness(99000);
			m_pBeam1->SetColor(0, 255, 0);
			m_pBeam1->AddSpawnFlags(SF_SPRITE_TEMPORARY);
		}
		if (m_pBeam2 == NULL)
		{
			m_pBeam2 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.0);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam2->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam2->PointEntInit(tr.endpos, this);

			m_pBeam2->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam2->AddSpawnFlags(SF_BEAM_TEMPORARY);
			m_pBeam2->SetBrightness(2255);
			m_pBeam2->SetColor(0, 255, 0);
			m_pBeam2->SetNoise(RandomFloat(0.5f, 2.0f));
		}
		if (m_pBeam3 == NULL)
		{
			m_pBeam3 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.0);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam3->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam3->PointEntInit(tr.endpos, this);

			m_pBeam3->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam3->AddSpawnFlags(SF_BEAM_TEMPORARY);
			m_pBeam3->SetBrightness(2255);
			m_pBeam3->SetColor(0, 255, 0);
			m_pBeam3->SetNoise(RandomFloat(0.5f, 2.0f));
		}

		DispatchParticleEffect("immolator_normal", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);
		DispatchParticleEffect("immolator_sparks01", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);



		SetNextThink(gpGlobals->curtime + 0.001);

		//if (pEntity != NULL && (pEntity->IsPlayer() || pEntity->IsNPC()))
		UpdateEffect(vAttachPos, tr.endpos);//tr.endpos
		//else
		//UpdateEffect(vAttachPos, pEntity->WorldSpaceCenter());//tr.endpos


		if (tr.DidHit())
		{
			int beams;
			CPASFilter filter(GetAbsOrigin());
			te->DynamicLight(filter, 0.0, &tr.endpos, 0, 255, 0, 0, 340, 0.02, 0);
			for (beams = 0; beams < 5; beams++)
			{
				//		Vector vecDest;
				Vector vecDest;
				// Random unit vector
				vecDest.x = random->RandomFloat(-1, 1);
				vecDest.y = random->RandomFloat(-1, 1);
				vecDest.z = random->RandomFloat(-1, 1);

				// Push out to radius dist.
				vecDest = tr.endpos + vecDest * 1.1*55.0f;


				UTIL_Beam(tr.endpos,
					vecDest,
					beamIndex,
					0,		//halo index
					0,		//frame start
					2.0f,	//framerate
					0.15f,	//life
					1.5,		// width
					1.15,	// endwidth
					0.75,	// fadelength,
					25,		// noise

					0,		// red
					255,	// green
					0,		// blue,

					128, // bright
					100  // speed
					);
			}
		}
	}
	m_iAmmo--;
}
void CNPC_Cremator::UpdateEffect(const Vector &startPoint, const Vector &endPoint)
{
	if (m_pBeam1)
	{
		m_pBeam1->SetAbsOrigin(endPoint);
	}
	if (m_pBeam2)
	{
		m_pBeam2->SetAbsOrigin(endPoint);
	}
	if (m_pBeam3)
	{
		m_pBeam3->SetAbsOrigin(endPoint);
	}
}

void CNPC_Cremator::DestroyEffect(void)
{
	//StopSound("Weapon_Immolator.Single");
	//EmitSound("Weapon_Immolator.Stop");

	if (m_pBeam1)
	{
		UTIL_Remove(m_pBeam1);
		m_pBeam1 = NULL;
	}
	if (m_pBeam2)
	{
		UTIL_Remove(m_pBeam2);
		m_pBeam2 = NULL;
	}
	if (m_pBeam3)
	{
		UTIL_Remove(m_pBeam3);
		m_pBeam3 = NULL;
	}
}
void CNPC_Cremator::DispatchSpray(const Vector &endPoint)
{
	/*if (pCorpse != pCorpseLast)
	{
	DestroyEffect();
	return;
	}*/
	Vector vecSrc, vecZero, vecAim, vecAimCorpse;
	trace_t tr;
	//DevMsg("DispatchSpray \n");

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);
	vecZero = pCorpse->WorldSpaceCenter();
	vecSrc = GetAbsOrigin();// +up * 36;
	vecAim = GetShootEnemyDir(vecSrc);

	if (pCorpse != NULL)
		vecAimCorpse = GetShootEnemyDir(pCorpse->WorldSpaceCenter());

	//float deflection = 0.01;
	//vecAim = vecAim + 1 * right * random->RandomFloat(0, deflection) + up * random->RandomFloat(-deflection, deflection);
	//DevMsg("pCorpse->GetAbsOrigin().x: %.2f \n", pCorpse->GetAbsOrigin().x);
	//DevMsg("pCorpse->GetAbsOrigin()y: %.2f \n", pCorpse->GetAbsOrigin().y);
	//DevMsg("pCorpse->GetAbsOrigin()z: %.2f \n", pCorpse->GetAbsOrigin().z);
	//StartFiring();

	Vector vAttachPos, Dist;
	QAngle Ang;
	GetAttachment(IMMOLATOR_ATTACHMENT, vAttachPos);
	Vector End = (vAttachPos + up + right + forward) * MAX_TRACE_LENGTH;
	VectorAngles(End, Ang);
	Ang.x += -90;

	Vector X, Y, Z;
	AngleVectors(Ang, &X, &Y, &Z);
	Vector Complete = X + Y + Z;

	DevMsg("Firing in corpse \n");


	Vector corpse;
	corpse.x = corpseCoord.x;//cvar->FindVar("oc_ragdoll_worldpos_x")->GetFloat();
	corpse.y = corpseCoord.y;//cvar->FindVar("oc_ragdoll_worldpos_y")->GetFloat();
	corpse.z = corpseCoord.z;//cvar->FindVar("oc_ragdoll_worldpos_z")->GetFloat();
	GetMotor()->SetIdealYawToTargetAndUpdate(corpse);

	//UTIL_TraceLine(vecSrc, vecSrc + vecAim * 521, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	//UTIL_TraceLine(vecSrc, vecSrc + vecAim * GetAbsOrigin().DistTo(pCorpse->WorldSpaceCenter()) * 2.7f, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	AI_TraceLine(vAttachPos, corpse, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	//UTIL_TraceLine(vecSrc, u, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	//pCorpse = tr.m_pEnt;

	if (pCorpse != NULL && m_takedamage)
	{

		ClearMultiDamage();
		CTakeDamageInfo firedamage(this, this, sk_cremator_firedamage.GetFloat(), DMG_DISSOLVE);
		CTakeDamageInfo radiusdamage(this, this, sk_cremator_radiusdamage.GetFloat(), DMG_DISSOLVE);
		CalculateMeleeDamageForce(&firedamage, vecAim, tr.endpos);
		RadiusDamage(CTakeDamageInfo(this, this, 2, DMG_DISSOLVE), // AOE; this stuff makes cremators absurdly powerfull sometimes btw
			tr.endpos,
			64.0f,
			CLASS_NONE,
			NULL);

		pCorpse->DispatchTraceAttack((firedamage), vecAim, &tr);


		DestroyEffect();


		//debugoverlay->AddLineOverlay(vAttachPos, vAttachPos + forward * MAX_TRACE_LENGTH, 0, 255, 0, false, 10);
		if (m_pBeam1 == NULL)
		{
			m_pBeam1 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.5);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam1->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam1->PointEntInit(tr.endpos, this);

			//m_pBeam1->PointEntInit(End, this);
			m_pBeam1->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam1->SetBrightness(99000);
			m_pBeam1->SetColor(0, 255, 0);
			m_pBeam1->AddSpawnFlags(SF_SPRITE_TEMPORARY);
		}
		if (m_pBeam2 == NULL)
		{
			m_pBeam2 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.0);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam2->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam2->PointEntInit(tr.endpos, this);

			m_pBeam2->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam2->AddSpawnFlags(SF_BEAM_TEMPORARY);
			m_pBeam2->SetBrightness(2255);
			m_pBeam2->SetColor(0, 255, 0);
			m_pBeam2->SetNoise(RandomFloat(0.5f, 2.0f));
		}
		if (m_pBeam3 == NULL)
		{
			m_pBeam3 = CBeam::BeamCreate(CREMATOR_BEAM_SPRITE, 1.0);

			//if (pEntity->IsPlayer() || pEntity->IsNPC())
			m_pBeam3->PointEntInit(vAttachPos + vecAim * MAX_TRACE_LENGTH, this);
			//else
			//m_pBeam3->PointEntInit(tr.endpos, this);

			m_pBeam3->SetEndAttachment(IMMOLATOR_ATTACHMENT);
			m_pBeam3->AddSpawnFlags(SF_BEAM_TEMPORARY);
			m_pBeam3->SetBrightness(2255);
			m_pBeam3->SetColor(0, 255, 0);
			m_pBeam3->SetNoise(RandomFloat(0.5f, 2.0f));
		}

		DispatchParticleEffect("immolator_normal", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);
		DispatchParticleEffect("immolator_sparks01", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);



		SetNextThink(gpGlobals->curtime + 0.001);

		//if (pEntity != NULL && (pEntity->IsPlayer() || pEntity->IsNPC()))
		UpdateEffect(vAttachPos, tr.endpos);//tr.endpos
		//else
		//UpdateEffect(vAttachPos, pEntity->WorldSpaceCenter());//tr.endpos


		//if (tr.DidHit())
		{
			int beams;
			CPASFilter filter(GetAbsOrigin());
			te->DynamicLight(filter, 0.0, &tr.endpos, 0, 255, 0, 0, 340, 0.02, 0);
			for (beams = 0; beams < 5; beams++)
			{
				//		Vector vecDest;
				Vector vecDest;
				// Random unit vector
				vecDest.x = random->RandomFloat(-1, 1);
				vecDest.y = random->RandomFloat(-1, 1);
				vecDest.z = random->RandomFloat(-1, 1);

				// Push out to radius dist.
				vecDest = tr.endpos + vecDest * 1.1*55.0f;


				UTIL_Beam(tr.endpos,
					vecDest,
					beamIndex,
					0,		//halo index
					0,		//frame start
					2.0f,	//framerate
					0.15f,	//life
					2,		// width
					1.25,	// endwidth
					0.75,	// fadelength,
					15,		// noise

					0,		// red
					255,	// green
					0,		// blue,

					128, // bright
					100  // speed
					);
			}
		}
	}
	m_iAmmo--;
}
/*Activity CNPC_Cremator::TranslateActivity(Activity activity) //OLD
{
Assert(activity != ACT_INVALID);

switch (activity)
{
case ACT_RUN:
{
return (Activity)ACT_WALK;
}
}

return activity;
}*/
Activity CNPC_Cremator::TranslateActivity(Activity activity)
{
	Assert(activity != ACT_INVALID);

	switch (activity)
	{
	case ACT_RUN:
	{
		DestroyEffect();
		DoFireUp = false;
		if (m_iHealth < (sk_cremator_health.GetFloat() * 0.5))
		{
			return (Activity)ACT_WALK_HURT;
		}
		else
			return (Activity)ACT_WALK;
	}
	}
	/*if (this->GetDamage() > 10.f)
	{
	return ACT_SMALL_FLINCH;
	}*/

	return activity;
}

Activity CNPC_Cremator::NPC_TranslateActivity(Activity activity)
{
	float fiftyprecent = 0.5 * sk_cremator_health.GetInt();

	if ((activity == ACT_WALK || activity == ACT_RUN) && (GetHealth() < fiftyprecent))
	{
		return ACT_WALK_HURT;
	}
	/*if (this->GetDamage() > 10.f)
	{
	return ACT_SMALL_FLINCH;
	}*/

	return activity;
}

int CNPC_Cremator::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_BULLET_IMPACT |
		SOUND_CARCASS |
		SOUND_MEAT |
		SOUND_GARBAGE |
		SOUND_PLAYER |
		SOUND_MOVE_AWAY;
}

//-----------------------------------------------------------------------------
// Create/destroy looping sounds 
//-----------------------------------------------------------------------------
void CNPC_Cremator::CreateSounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);
	if (!m_pGunFiringSound)
	{
		m_pGunFiringSound = controller.SoundCreate(filter, entindex(), "Weapon_Immolator.Single");
		controller.Play(m_pGunFiringSound, 0, 100);
	}
}

void CNPC_Cremator::DestroySounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy(m_pGunFiringSound);
	m_pGunFiringSound = NULL;
}


//-----------------------------------------------------------------------------
// Stop Firing
//-----------------------------------------------------------------------------
void CNPC_Cremator::StartFiring()
{
	if (!m_bIsFiring)
	{
		pController = &CSoundEnvelopeController::GetController();

		m_bIsFiring = true;
	}
	float flVolume = pController->SoundGetVolume(m_pGunFiringSound);
	pController->SoundChangeVolume(m_pGunFiringSound, 1.4f, 0.1f * (1.0f - flVolume));
	//int iPitch = 100;
	pController->SoundChangePitch(m_pGunFiringSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);
}

void CNPC_Cremator::StopFiring()
{
	DestroyEffect();
	DoFireUp = false;

	if (m_bIsFiring)
	{
		pController = &CSoundEnvelopeController::GetController();
		float flVolume = pController->SoundGetVolume(m_pGunFiringSound);
		pController->SoundChangeVolume(m_pGunFiringSound, 0.0f, 0.1f * flVolume);
		pController->SoundGetPitch(m_pGunFiringSound);
		//int iPitch = 100;
		pController->SoundChangePitch(m_pGunFiringSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);
		StopSound("Weapon_Immolator.Start");
		EmitSound("Weapon_Immolator.Stop");
		m_bIsFiring = false;
		if (!this)
			return;
		//		StopParticleEffects(pOwner->GetViewModel());

		DispatchParticleEffect("weapon_muzzle_smoke_immolator", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, true);
		DispatchParticleEffect("weapon_muzzle_smoke_immolator1", PATTACH_POINT_FOLLOW, this, IMMOLATOR_ATTACHMENT, false);
	}
}

void CNPC_Cremator::CreateBreathSound()
{
	return;
	CSoundEnvelopeController &pBreath = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);
	if (!m_pBreathSound)
	{
		m_pBreathSound = pBreath.SoundCreate(filter, entindex(), "NPC_Cremator.Breath");
		pBreath.Play(m_pBreathSound, 0, 100);
	}
}

void CNPC_Cremator::DestroyBreathSound()
{
	return;
	CSoundEnvelopeController &pBreath = CSoundEnvelopeController::GetController();

	pBreath.SoundDestroy(m_pBreathSound);
	m_pBreathSound = NULL;
}


//-----------------------------------------------------------------------------
// Stop Firing
//-----------------------------------------------------------------------------
void CNPC_Cremator::StartBreathSound()
{
	return;
	if (!m_breath)
	{
		pBreath = &CSoundEnvelopeController::GetController();

		m_breath = true;
	}
	float flVolume = pBreath->SoundGetVolume(m_pBreathSound);
	pBreath->SoundChangeVolume(m_pBreathSound, 1.4f, 0.1f * (1.0f - flVolume));

	pBreath->SoundChangePitch(m_pBreathSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);
}

void CNPC_Cremator::StopBreathSound()
{
	return;
	if (m_breath)
	{
		pBreath = &CSoundEnvelopeController::GetController();
		float flVolume = pBreath->SoundGetVolume(m_pBreathSound);
		pBreath->SoundChangeVolume(m_pBreathSound, 0.0f, 0.1f * flVolume);
		pBreath->SoundGetPitch(m_pBreathSound);
		//int iPitch = 100;
		pBreath->SoundChangePitch(m_pBreathSound, 100 * cvar->FindVar("host_timescale")->GetFloat(), 1.0f);

		m_breath = false;
		if (!this)
			return;

	}
}

AI_BEGIN_CUSTOM_NPC(npc_cremator, CNPC_Cremator)

DECLARE_ACTIVITY(ACT_FIRESPREAD)
DECLARE_ACTIVITY(ACT_FIREIDLE)
DECLARE_ACTIVITY(ACT_CREMATOR_ARM)
DECLARE_ACTIVITY(ACT_CREMATOR_DISARM)
DECLARE_ACTIVITY(ACT_CREMATOR_RELOAD)

DECLARE_CONDITION(COND_CREMATOR_OUT_OF_AMMO)
DECLARE_CONDITION(COND_CREMATOR_ENEMY_WITH_HIGHER_PRIORITY)
DECLARE_CONDITION(COND_CREMATOR_FOUND_CORPSE)

DECLARE_TASK(TASK_CREMATOR_RANGE_ATTACK1)
DECLARE_TASK(TASK_CREMATOR_RELOAD)
DECLARE_TASK(TASK_CREMATOR_INVESTIGATE_CORPSE)
DECLARE_TASK(TASK_CREMATOR_BURN_CORPSE)

DEFINE_SCHEDULE(
	SCHED_CREMATOR_CHASE_ENEMY,
	"	Tasks"
	"	TASK_GET_CHASE_PATH_TO_ENEMY 300"//1024
	"	TASK_SET_TOLERANCE_DISTANCE 24"//250
	"	TASK_WALK_PATH 0"
	"	TASK_WAIT_FOR_MOVEMENT 0"
	"	TASK_FACE_ENEMY 0"
	""
	"	Interrupts"
	"	COND_CAN_RANGE_ATTACK1"
	"	COND_ENEMY_DEAD"
	"	COND_LOST_ENEMY"
	"	COND_CREMATOR_ENEMY_WITH_HIGHER_PRIORITY"
	)

	DEFINE_SCHEDULE(
	SCHED_CREMATOR_RANGE_ATTACK1,
	"	Tasks"
	"	TASK_STOP_MOVING 0"
	//"	TASK_FACE_ENEMY 0"
	//"	TASK_ANNOUNCE_ATTACK 1"
	"	TASK_PLAY_SEQUENCE ACTIVITY:ACT_CREMATOR_ARM"
	"	TASK_CREMATOR_RANGE_ATTACK1 0"
	"	TASK_PLAY_SEQUENCE ACTIVITY:ACT_CREMATOR_RELOAD"
	""
	"	Interrupts"
	"	COND_HEAVY_DAMAGE"
	"	COND_REPEATED_DAMAGE"
	"	COND_ENEMY_DEAD"
	"	COND_TOO_FAR_TO_ATTACK"
	"	COND_CREMATOR_ENEMY_WITH_HIGHER_PRIORITY"
	)

	DEFINE_SCHEDULE(
	SCHED_CREMATOR_PATROL,
	"	Tasks"
	"	TASK_STOP_MOVING 0"
	"	TASK_GET_PATH_TO_RANDOM_NODE 1024"
	"	TASK_WALK_PATH 0"
	"	TASK_WAIT_FOR_MOVEMENT 0"
	"	TASK_WAIT 5"
	"	TASK_SET_SCHEDULE SCHEDULE:SCHED_CREMATOR_PATROL"
	""
	"	Interrupts"
	"	COND_CREMATOR_FOUND_CORPSE"
	"	COND_NEW_ENEMY"
	"	COND_LIGHT_DAMAGE"
	"	COND_HEAVY_DAMAGE"
	)
	DEFINE_SCHEDULE(
	SCHED_CREMATOR_INVESTIGATE_CORPSE, // we're here because we have COND_CREMATOR_FOUND_CORPSE.
	"	Tasks"
	"	TASK_WAIT_FOR_MOVEMENT 0"
	"	TASK_SOUND_WAKE 0" // Play the alert sound
	"	TASK_CREMATOR_INVESTIGATE_CORPSE 64" // analogous to TASK_GET_PATH_TO_BESTSCENT
	//"	TASK_WALK_PATH 0"
	"	TASK_WAIT_FOR_MOVEMENT 0"
	"	TASK_CREMATOR_BURN_CORPSE 0" // Play the firespread animation
	"	TASK_WAIT 3"
	//"	TASK_SET_FAIL_SCHEDULE SCHEDULE:SCHED_CREMATOR_INVESTIGATE_CORPSE"
	"	TASK_SET_SCHEDULE SCHEDULE:SCHED_CREMATOR_PATROL" // resume patroling
	""
	"	Interrupts"
	"	COND_NEW_ENEMY"
	"	COND_SEE_ENEMY"
	"	COND_LIGHT_DAMAGE"
	"	COND_HEAVY_DAMAGE"
	)
	AI_END_CUSTOM_NPC()