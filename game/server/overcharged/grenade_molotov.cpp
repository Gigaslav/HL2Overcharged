//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Flaming bottle thrown from the hand
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "ammodef.h"
#include "gamerules.h"
#include "grenade_molotov.h"
#include "weapon_brickbat.h"
#include "soundent.h"
#include "decals.h"
#include "fire.h"
#include "shake.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "BloodDrips.h"
#include "EntityFlame.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern short	g_sModelIndexFireball;

//ConVar sk_plr_dmg_molotov("sk_plr_dmg_molotov", "0");
//ConVar sk_npc_dmg_molotov("sk_npc_dmg_molotov", "0");

//extern ConVar    sk_plr_dmg_molotov;	// OLD 
//extern ConVar    sk_npc_dmg_molotov;
ConVar		sk_molotov_radius			( "sk_molotov_radius","0");
ConVar		oc_weapon_molotov_model		( "oc_weapon_molotov_model", "models/weapons/w_grenade.mdl", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Molotov w_ model" );

#define MOLOTOV_EXPLOSION_VOLUME	1024
#define MOLOTOV_MODEL oc_weapon_molotov_model.GetString()

BEGIN_DATADESC( CGrenade_Molotov )

	DEFINE_FIELD( m_pFireTrail, FIELD_CLASSPTR ),

	// Function Pointers
	DEFINE_ENTITYFUNC( MolotovTouch ),
	DEFINE_THINKFUNC( MolotovThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( grenade_molotov, CGrenade_Molotov );

void CGrenade_Molotov::Spawn( void )
{
	Precache();

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	SetSolid( SOLID_BBOX ); 
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	RemoveEffects( EF_NOINTERP );

	PrecacheModel(GetThrower()->GetActiveWeapon()->GetWpnData().szWorldModel);
	SetModel(GetThrower()->GetActiveWeapon()->GetWpnData().szWorldModel);//"models/weapons/w_molotov.mdl");//MOLOTOV_MODEL

	UTIL_SetSize(this, Vector( -6, -6, -2), Vector(6, 6, 2));

	SetTouch( &CGrenade_Molotov::MolotovTouch );
	SetThink( &CGrenade_Molotov::MolotovThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage		= cvar->FindVar("sk_plr_dmg_molotov")->GetFloat(); //sk_plr_dmg_molotov.GetFloat();
	m_DmgRadius		= sk_molotov_radius.GetFloat();

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetGravity( 1.0 );
	SetFriction( 0.8 );  // Give a little bounce so can flatten
	SetSequence( 1 );

	//DispatchParticleEffect("weapon_molotov_flame_world", PATTACH_ABSORIGIN_FOLLOW, this);
	DispatchParticleEffect("weapon_molotov_flame_world", PATTACH_POINT_FOLLOW, this, 1, false);
	/*CEntityFlame *pFlame = CEntityFlame::Create(this, false);
	if (pFlame != NULL)
	{
		pFlame->SetLifetime(5);
	}*/
	/*m_pFireTrail = SmokeTrail::CreateSmokeTrail();

	if( m_pFireTrail )
	{
		m_pFireTrail->m_SpawnRate			= 48;
		m_pFireTrail->m_ParticleLifetime	= 1.0f;
		
		m_pFireTrail->m_StartColor.Init( 0.2f, 0.2f, 0.2f );
		m_pFireTrail->m_EndColor.Init( 0.0, 0.0, 0.0 );
		
		m_pFireTrail->m_StartSize	= 8;
		m_pFireTrail->m_EndSize		= 32;
		m_pFireTrail->m_SpawnRadius	= 4;
		m_pFireTrail->m_MinSpeed	= 8;
		m_pFireTrail->m_MaxSpeed	= 16;
		m_pFireTrail->m_Opacity		= 0.25f;

		m_pFireTrail->SetLifetime( 20.0f );
		m_pFireTrail->FollowEntity( this, "0" );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CGrenade_Molotov::MolotovTouch( CBaseEntity *pOther )
{
	// Don't hit triggers or water		// L1ght 15 : Damn that's should fix mistakes.
	Assert( pOther );
	if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) )
		return;

	if (pOther && (!(pOther->GetFlags() & FL_ONFIRE)))
	{
		CEntityFlame *pFlame = CEntityFlame::Create(pOther);
		if (pFlame)
		{
			SetEffectEntity(NULL);
			pFlame->SetAbsOrigin(GetAbsOrigin());
			pFlame->AttachToEntity(pOther);
			pFlame->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
			//pFlame->AddEffects(EF_BRIGHTLIGHT); // create light from the fire
			pFlame->SetLifetime(20.0); // burn for 20 seconds

			pOther->AddFlag(FL_ONFIRE);
			pOther->SetEffectEntity(pFlame);
			pOther->SetRenderColor(50, 50, 50);
			m_OnIgnite.FireOutput(pOther, pFlame);
			/*pOther->SetOwnerEntity(pFlame); // HACKHACK - we're marking this corpse so that it won't be picked again in the future.

			pOther->SetEFlags(FL_ONFIRE);
			pOther->AddFlag(FL_ONFIRE);
			pOther->AddEFlags(FL_ONFIRE);
			pOther->AddSpawnFlags(FL_ONFIRE);*/
			//DevMsg("This corpse has been handled. Moving on\n");
		}
	}
	Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CGrenade_Molotov::Detonate( void ) 
{
	SetModelName( NULL_STRING );		//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );	// intangible

	m_takedamage = DAMAGE_NO;

	/*
	trace_t trace;
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, -128 ),  MASK_SOLID_BRUSHONLY, 
		this, COLLISION_GROUP_NONE, &trace);

	// Pull out of the wall a bit
	if ( trace.fraction != 1.0 )
	{
		SetLocalOrigin( trace.endpos + (trace.plane.normal * (m_flDamage - 24) * 0.6) );
	}

	int contents = UTIL_PointContents ( GetAbsOrigin() );
	
	if ( (contents & MASK_WATER) )
	{
		UTIL_Remove( this );
		return;
	}

	EmitSound( "Grenade_Molotov.Detonate");

// Start some fires
	int i;
	QAngle vecTraceAngles;
	Vector vecTraceDir;
	trace_t firetrace;

	for( i = 0 ; i < 16 ; i++ )
	{
		// build a little ray
		vecTraceAngles[PITCH]	= random->RandomFloat(45, 135);
		vecTraceAngles[YAW]		= random->RandomFloat(0, 360);
		vecTraceAngles[ROLL]	= 0.0f;

		AngleVectors( vecTraceAngles, &vecTraceDir );

		Vector vecStart, vecEnd;

		vecStart = GetAbsOrigin() + ( trace.plane.normal * 128 );
		vecEnd = vecStart + vecTraceDir * 512;

		UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace );

		Vector	ofsDir = ( firetrace.endpos - GetAbsOrigin() );
		float	offset = VectorNormalize( ofsDir );

		if ( offset > 128 )
			offset = 128;

		//Get our scale based on distance
		float scale	 = 0.1f + ( 0.75f * ( 1.0f - ( offset / 128.0f ) ) );
		float growth = 0.1f + ( 0.75f * ( offset / 128.0f ) );

		if( firetrace.fraction != 1.0 )
		{
			FireSystem_StartFire( firetrace.endpos, scale, growth, 30.0f, (SF_FIRE_START_ON|SF_FIRE_SMOKELESS|SF_FIRE_NO_GLOW), (CBaseEntity*) this, FIRE_NATURAL );
		}
	}
	
	CPASFilter filter2( trace.endpos );
	*/

	CBaseEntity *pOwner;
	pOwner = GetOwnerEntity();
	SetOwnerEntity( NULL ); // can't traceline attack owner if this is set

	//UTIL_DecalTrace( &trace, "Scorch" );

	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

	//RadiusDamage( CTakeDamageInfo( this, pOwner, m_flDamage, DMG_BLAST ), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime + 0.2 );

	if ( m_pFireTrail )
	{
		UTIL_Remove( m_pFireTrail );
	}

	UTIL_Remove(this);





	FireSystem_StartFire(GetAbsOrigin(), 1 * random->RandomFloat(1, 50), 0.2f, random->RandomInt(10, 30), (SF_FIRE_START_ON | SF_FIRE_SMOKELESS), (CBaseEntity*) this, FIRE_PLASMA);//firetrace.endpos

	int MaxDrips = RandomInt(34, 52);
	Vector	vecToss, vecTraceDir;
	//if (info.GetDamageType() & (DMG_BULLET | DMG_SHOCK))
	{
		for (int i = 0; i < MaxDrips; i++)
		{
			vecTraceDir = WorldSpaceCenter();
			//if (LastHitGroup() == HITGROUP_CHEST)
			{
				vecTraceDir.x += random->RandomFloat(-0.2, 0.2);
				vecTraceDir.y += random->RandomFloat(-0.2, 0.2);
				vecTraceDir.z += random->RandomFloat(-0.2, 0.2);
			}
			/*if (LastHitGroup() == HITGROUP_HEAD)
			{
			vecTraceDir.x += random->RandomFloat(0.1, 0.4);
			vecTraceDir.y += random->RandomFloat(0.1, 0.4);
			vecTraceDir.z += random->RandomFloat(12.1, 12.4);
			}
			if (LastHitGroup() == HITGROUP_LEFTARM)
			{
			vecTraceDir.x += random->RandomFloat(0.1, 0.2);
			vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
			vecTraceDir.z += random->RandomFloat(0.1, 0.2);
			}
			if (LastHitGroup() == HITGROUP_RIGHTARM)
			{
			vecTraceDir.x += random->RandomFloat(0.1, 0.2);
			vecTraceDir.y += random->RandomFloat(12.1, 12.4);
			vecTraceDir.z += random->RandomFloat(0.1, 0.2);
			}
			if (LastHitGroup() == HITGROUP_LEFTLEG)
			{
			vecTraceDir.x += random->RandomFloat(0.1, 0.2);
			vecTraceDir.y += random->RandomFloat(-12.1, -12.4);
			vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
			}
			if (LastHitGroup() == HITGROUP_RIGHTLEG)
			{
			vecTraceDir.x += random->RandomFloat(0.1, 0.2);
			vecTraceDir.y += random->RandomFloat(12.1, 12.4);
			vecTraceDir.z += random->RandomFloat(-20.1, -20.2);
			}*/

			// Set the velocity
			Vector vecVelocity;
			AngularImpulse angImpulse;

			QAngle angles;
			angles.x = random->RandomFloat(-70, 20);
			angles.y = random->RandomFloat(0, 360);
			angles.z = 0.0f;
			AngleVectors(angles, &vecVelocity);

			vecVelocity *= random->RandomFloat(150, 300);
			vecVelocity += GetAbsVelocity();

			angImpulse = RandomAngularImpulse(-180, 180);



			CMolotovEmbers *pGrenade = (CMolotovEmbers*)CreateEntityByName("molotov_embers");
			//pGrenade->SetAbsOrigin(vecTraceDir);
			pGrenade->SetLocalOrigin(vecTraceDir);
			pGrenade->SetAbsAngles(RandomAngle(0, 360));
			DispatchSpawn(pGrenade);
			//pGrenade->SetThrower(this);
			//pGrenade->SetOwnerEntity(this);
			pGrenade->SetAbsVelocity(vecVelocity);
		}
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CGrenade_Molotov::MolotovThink( void )
{
	// See if I can lose my owner (has dropper moved out of way?)
	// Want do this so owner can throw the brickbat
	if (GetOwnerEntity())
	{
		trace_t tr;
		Vector	vUpABit = GetAbsOrigin();
		vUpABit.z += 5.0;

		CBaseEntity* saveOwner	= GetOwnerEntity();
		SetOwnerEntity( NULL );
		UTIL_TraceEntity( this, GetAbsOrigin(), vUpABit, MASK_SOLID, &tr );
		if ( tr.startsolid || tr.fraction != 1.0 )
		{
			SetOwnerEntity( saveOwner );
		}
	}
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CGrenade_Molotov::Precache( void )
{
	PrecacheModel(MOLOTOV_MODEL); //"models/weapons/w_bb_bottle.mdl");
	UTIL_PrecacheOther("_firesmoke");
	PrecacheScriptSound( "Grenade_Molotov.Detonate" );
	PrecacheParticleSystem("weapon_molotov_flame_world");
	BaseClass::Precache();
}











//ConVar    sk_BloodDrips_dmg("sk_BloodDrips_dmg", "20", FCVAR_NONE, "Total damage done by an individual antlion worker loogie.");
ConVar	  sk_molotov_embers_radius("sk_molotov_embers_radius", "40", FCVAR_NONE, "Radius of effect for an antlion worker spit grenade.");
ConVar	  sk_molotov_embers_ratio("sk_molotov_embers_ratio", "0.3", FCVAR_NONE, "Percentage of an antlion worker's spit damage done as poison (which regenerates)");

LINK_ENTITY_TO_CLASS(molotov_embers, CMolotovEmbers);

BEGIN_DATADESC(CMolotovEmbers)

DEFINE_FIELD(m_bPlaySound, FIELD_BOOLEAN),

// Function pointers
DEFINE_ENTITYFUNC(BloodDripsTouch),

END_DATADESC()

CMolotovEmbers::CMolotovEmbers(void) : m_bPlaySound(true), m_pHissSound(NULL)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMolotovEmbers::Spawn(void)
{
	Precache();
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolidFlags(FSOLID_NOT_STANDABLE);
	Vector SizeMin(0.2, 0.2, 0.2), SizeMax(1, 1, 1);

	//SetModel("models/weapons/flare.mdl");
	SetModel("models/ballsphere.mdl");
	SetModelScale(0);//0.01f, 0.0f);
	UTIL_SetSize(this, SizeMin, SizeMax);
	SetModelScale(0.2f);
	SetUse(&CBaseGrenade::DetonateUse);
	SetTouch(&CMolotovEmbers::BloodDripsTouch);
	SetNextThink(gpGlobals->curtime + 0.1f);

	//m_flDamage = sk_BloodDrips_dmg.GetFloat();
	m_DmgRadius = sk_molotov_embers_radius.GetFloat();
	m_takedamage = DAMAGE_NO;
	m_iHealth = 1;

	SetGravity(UTIL_ScaleForGravity(EMBERS_GRAVITY));
	SetFriction(0.8f);

	SetCollisionGroup(HL2COLLISION_GROUP_SPIT);

	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	// We're self-illuminating, so we don't take or give shadows
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	CEntityFlame *pFlame = CEntityFlame::Create(this, false);
	if (pFlame != NULL)
	{
		pFlame->SetLifetime(5);
	}

	DispatchParticleEffect("weapon_molotov_flame_world", PATTACH_ABSORIGIN_FOLLOW, this);

}


void CMolotovEmbers::SetBloodDripsSize(int nSize)
{
	/*switch (nSize)
	{
	case REDSPIT_LARGE:
	{
		m_bPlaySound = true;
		SetModel("models/spitball_large.mdl");
		break;
	}
	case REDSPIT_MEDIUM:
	{
		m_bPlaySound = true;
		//m_flDamage *= 0.5f;
		SetModel("models/spitball_medium.mdl");
		break;
	}*/
	//case REDSPIT_SMALL:
	{
		m_bPlaySound = false;
		//m_flDamage *= 0.25f;
		SetModel("models/spitball_small.mdl");
		//break;
	}
	//}
}

void CMolotovEmbers::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Handle spitting
//-----------------------------------------------------------------------------
void CMolotovEmbers::BloodDripsTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
		return;

	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();


	if (pOther == this->GetParent())
		return;

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ((pTrace->contents & CONTENTS_WATER) != 0);
	//	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if (bHitWater)
	{
		// Splash!
		CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector(0, 0, 1);
		data.m_flScale = 8.0f;

		DispatchEffect("watersplash", data);
	}

	StopParticleEffects(this);

	if (pOther && !(pOther->GetFlags() & FL_ONFIRE))
	{
		CEntityFlame *pFlame = CEntityFlame::Create(pOther);
		if (pFlame)
		{
			SetEffectEntity(NULL);
			pFlame->SetAbsOrigin(GetAbsOrigin());
			pFlame->AttachToEntity(pOther);
			pFlame->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
			//pFlame->AddEffects(EF_BRIGHTLIGHT); // create light from the fire
			pFlame->SetLifetime(20.0); // burn for 20 seconds

			//pOther->AddSpawnFlags(FL_ONFIRE);
			pOther->AddFlag(FL_ONFIRE);

			pOther->SetEffectEntity(pFlame);
			pOther->SetRenderColor(50, 50, 50);
			m_OnIgnite.FireOutput(pOther, pFlame);

			//pOther->SetOwnerEntity(pFlame); // HACKHACK - we're marking this corpse so that it won't be picked again in the future.
			//DevMsg("This corpse has been handled. Moving on\n");
		}
	}
	//pOther->SetEFlags(FL_ONFIRE);
	//pOther->AddFlag(FL_ONFIRE);
	//pOther->AddSpawnFlags(FL_ONFIRE);

	Detonate();
}

void CMolotovEmbers::Detonate(void)
{

	trace_t trace;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin()/* + Vector(0, 0, -128)*/, MASK_SOLID_BRUSHONLY,
		this, COLLISION_GROUP_NONE, &trace);

	// Pull out of the wall a bit
	if (trace.fraction != 1.0)
	{
		//SetLocalOrigin(trace.endpos + (trace.plane.normal * (m_flDamage - 24) * 0.6));
	}

	int contents = UTIL_PointContents(GetAbsOrigin());

	if ((contents & MASK_WATER))
	{
		UTIL_Remove(this);
		return;
	}

	//EmitSound("Grenade_Molotov.Detonate");


	QAngle vecTraceAngles;
	Vector vecTraceDir;
	trace_t firetrace;

	for (int i = 0; i < random->RandomInt(20,52); i++)
	{
		// build a little ray
		//vecTraceAngles[PITCH] = random->RandomFloat(45, 135);
		//vecTraceAngles[YAW] = random->RandomFloat(0, 360);

		vecTraceAngles[PITCH] = 0;
		vecTraceAngles[YAW] = 0;
		vecTraceAngles[ROLL] = random->RandomFloat(-360, 360);

		AngleVectors(vecTraceAngles, &vecTraceDir);

		Vector vecStart, vecEnd;

		vecStart = GetAbsOrigin() + (trace.plane.normal * 128);

		vecEnd = vecStart + vecTraceDir;// *random->RandomFloat(2.f, 15.f);

		UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace);

		Vector	ofsDir = (firetrace.endpos - GetAbsOrigin());
		float	offset = VectorNormalize(ofsDir);

		if (offset > 128)
			offset = 128;

		//Get our scale based on distance
		float scale = 0.1f + (0.75f * (1.0f - (offset / 128.0f)));
		//float growth = 0.1f + (0.75f * (offset / 128.0f));

		if (firetrace.fraction != 1.0)
		{
			FireSystem_StartFire(firetrace.endpos, scale*random->RandomFloat(10, 70), 0.2f, random->RandomFloat(7, 30), (SF_FIRE_START_ON | SF_FIRE_SMOKELESS ), (CBaseEntity*) this, FIRE_NATURAL);//firetrace.endpos
		}
	}
	CPASFilter filter2(trace.endpos);
	UTIL_DecalTrace( &trace, "Scorch" );










	m_takedamage = DAMAGE_NO;

	EmitSound("BloodDrips.Hit");

	// Stop our hissing sound
	if (m_pHissSound != NULL)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pHissSound);
		m_pHissSound = NULL;
	}

	if (m_hSpitEffect)
	{
		UTIL_Remove(m_hSpitEffect);
	}

	StopParticleEffects(this);
	UTIL_Remove(this);
}

void CMolotovEmbers::InitHissSound(void)
{
	if (m_bPlaySound == false)
		return;

	//CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if (m_pHissSound == NULL)
	{
		/*CPASAttenuationFilter filter(this);
		m_pHissSound = controller.SoundCreate(filter, entindex(), "NPC_Antlion.PoisonBall");
		controller.Play(m_pHissSound, 1.0f, 100);*/
	}
}

void CMolotovEmbers::Think(void)
{
	InitHissSound();
	if (m_pHissSound == NULL)
		return;
	SetModelScale(0.01f, 0.0f);
	// Add a doppler effect to the balls as they travel
	CBaseEntity *pPlayer = AI_GetSinglePlayer();
	if (pPlayer != NULL)
	{
		Vector dir;
		VectorSubtract(pPlayer->GetAbsOrigin(), GetAbsOrigin(), dir);
		VectorNormalize(dir);

		float velReceiver = DotProduct(pPlayer->GetAbsVelocity(), dir);
		float velTransmitter = -DotProduct(GetAbsVelocity(), dir);

		// speed of sound == 13049in/s
		int iPitch = 100 * ((1 - velReceiver / 13049) / (1 + velTransmitter / 13049));

		// clamp pitch shifts
		if (iPitch > 250)
		{
			iPitch = 250;
		}
		if (iPitch < 50)
		{
			iPitch = 50;
		}

		// Set the pitch we've calculated
		//CSoundEnvelopeController::GetController().SoundChangePitch(m_pHissSound, iPitch, 0.1f);
	}
	DispatchParticleEffect("blood_spit_trail", PATTACH_ABSORIGIN_FOLLOW, this);
	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.05f);
}

void CMolotovEmbers::Precache(void)
{
	PrecacheModel("models/props_hive/larval_essence.mdl");

	PrecacheModel("models/spitball_large.mdl");
	PrecacheModel("models/spitball_medium.mdl");
	PrecacheModel("models/spitball_small.mdl");
	PrecacheModel("models/weapons/flare.mdl");
	PrecacheScriptSound("BloodDrips.Hit");
	PrecacheModel("models/ballsphere.mdl");
	PrecacheParticleSystem("weapon_molotov_flame_world");
	PrecacheParticleSystem("grenade_spit");
	PrecacheParticleSystem("grenade_spit_trail");
	PrecacheParticleSystem("blood_spit_trail");
}
