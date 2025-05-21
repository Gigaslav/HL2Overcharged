//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Combine Sniper Rifle - recreation from ep2
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"

#include "OverCharged/ShotgunBullet.h"//For laser

#include "weapon_combinesniper.h"//For laser
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
extern ConVar   weapon_laser_pointer;
#define	BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	LASER_SPRITE	"sprites/greenglow1.vmt"//"effects/greenglow1"//"sprites/redglow1.vmt"
//#define	LSPRITE	        "sprites/greenglow1.vmt"//"effects/greenglow1"
#define	LASER_SPRITE1	"effects/black2.vmt"
//-----------------------------------------------------------------------------
// CWeaponCombineSniper
//-----------------------------------------------------------------------------
#define	CS_FASTEST_REFIRE_TIME		0.15f
#define	CS_FASTEST_DRY_REFIRE_TIME	0.2f

#define	CS_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	CS_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//-----------------------------------------------------------------------------
// Discrete zoom levels for the scope.	// old 3x scope preset
//-----------------------------------------------------------------------------
static int g_nZoomFOV[] =
{
	20,
	5
};



LINK_ENTITY_TO_CLASS(weapon_combinesniper, CWeaponCombineSniper);
PRECACHE_WEAPON_REGISTER(weapon_combinesniper);

IMPLEMENT_SERVERCLASS_ST(CWeaponCombineSniper, DT_WeaponCombineSniper)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponCombineSniper)
	DEFINE_FIELD(durationTime, FIELD_FLOAT),
	DEFINE_FIELD(durationTime2, FIELD_FLOAT),
	DEFINE_FIELD(wasInReload, FIELD_BOOLEAN),
END_DATADESC()

short sHaloSprite2;	// BJ: Added laser npc

acttable_t CWeaponCombineSniper::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },	// Light Kill : MP animstate for singleplayer
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	//{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeaponCombineSniper);



/*class CParticle : public CBaseCombatCharacter
{
DECLARE_CLASS(CParticle, CBaseCombatCharacter);
//Class_T Classify(void) { return CLASS_NONE; }
public:
CParticle() { };
~CParticle() { };
void Precache(void);
void Spawn(void);
static CParticle		*DispatchImpactSound(const Vector &Start, const QAngle &Ang, CBaseViewModel *pentOwner = NULL, CBasePlayer *pentOwner2 = NULL);

/*protected:
DECLARE_DATADESC();
DECLARE_SERVERCLASS();*/
/*};
LINK_ENTITY_TO_CLASS(Particle, CParticle);


static ConVar oc_test_beam__x("oc_test_beam__x", "0.0", FCVAR_ARCHIVE);
static ConVar oc_test_beam__y("oc_test_beam__y", "0.0", FCVAR_ARCHIVE);
static ConVar oc_test_beam__z("oc_test_beam__z", "0.0", FCVAR_ARCHIVE);
CParticle *CParticle::DispatchImpactSound(const Vector &Start, const QAngle &Ang, CBaseViewModel *pentOwner, CBasePlayer *pentOwner2)
{
CParticle *Particle = (CParticle *)(CreateEntityByName("Particle"));
UTIL_SetOrigin(Particle, pentOwner2->GetLocalOrigin());
Particle->SetLocalOrigin(pentOwner2->GetLocalOrigin());
Particle->SetLocalAngles(pentOwner2->GetLocalAngles());
//Particle->Spawn();
Particle->SetOwnerEntity(pentOwner2);
Particle->SetParent(pentOwner2, 1);
//DispatchParticleEffect("laser_pointer", Start, Ang, pentOwner);


Vector Muzzle, forward, right, up, End, VecAng;
Muzzle = pentOwner2->GetLocalOrigin();
pentOwner2->EyeVectors(&forward, &right, &up);

Muzzle += forward * oc_test_beam__x.GetFloat();
Muzzle += right * oc_test_beam__y.GetFloat();
Muzzle += up * oc_test_beam__z.GetFloat();

End = Muzzle + (forward * MAX_TRACE_LENGTH);
trace_t tr;


UTIL_TraceLine(Muzzle, End, (MASK_SHOT & ~CONTENTS_WINDOW), NULL, COLLISION_GROUP_NONE, &tr);
debugoverlay->AddLineOverlay(Start, tr.endpos, 0, 255, 0, false, 130.2);

//VectorSubtract(tr.endpos, tr.plane.normal, VecAng);
QAngle vecAngles;
VectorAngles(End, vecAngles);
DispatchParticleEffect("sniper_laser", Start, vecAngles, Particle);
PATTACH_POINT_FOLLOW
//Particle->Spawn();

return Particle;
}
void CParticle::Precache(void)
{
PrecacheScriptSound("Weapon_Gluon.Hit");
}
void CParticle::Spawn(void)
{
Precache();

SetMoveType(MOVETYPE_NONE);
UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
SetSolid(SOLID_NONE);



//EmitSound("Weapon_Gluon.Hit");

UTIL_Remove(this);

}*/

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponCombineSniper::CWeaponCombineSniper(void)
{
	/*m_flSoonestPrimaryAttack = gpGlobals->curtime;
	pressed = false;
	m_flAccuracyPenalty = 0.0f;*/
	m_bReloadsSingly = false;
	/*m_bInZoom = false;*/

	//=======BriJee: NPC allow shoot.
	m_fMinRange1 = 24;
	m_fMinRange2 = 24;
	m_fMaxRange1 = 8000;
	m_fMaxRange2 = 8000;
	//Once = false;
	wasInReload = false;
	durationTime = 0;
	durationTime2 = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCombineSniper::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel("sprites/greenglow1.vmt");
	PrecacheModel(LASER_SPRITE);
	PrecacheModel(LASER_SPRITE1);
	PrecacheModel(BEAM_SPRITE);
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_CombineSniper.Pump");
	PrecacheScriptSound("Weapon_CombineSniper.PumpFail");
	PrecacheModel("effects/bluelaser1.vmt");
	PrecacheModel("sprites/light_glow03.vmt");
	//PrecacheScriptSound("Weapon_357.Draw");
}
/*Activity CWeaponCombineSniper::GetDrawActivity(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return (Activity)0;
	}


	if (pPlayer->GetActiveWeapon())
	{
		durationTime = GetSoundDuration("Weapon_CombineSniper.Pump", STRING(pPlayer->GetActiveWeapon()->GetModelName()));
		durationTime2 = GetSoundDuration("Weapon_CombineSniper.PumpFail", STRING(pPlayer->GetActiveWeapon()->GetModelName()));
	}
	//EmitSound("Weapon_357.Draw");
	return BaseClass::GetDrawActivity();
}*/

void CWeaponCombineSniper::Pump(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	if (m_iClip1 <= 0)
	{
		//EmitSound("Weapon_CombineSniper.PumpFail");
		WeaponSound(SPECIAL2);
	}
	else
	{
		//EmitSound("Weapon_CombineSniper.Pump");
		WeaponSound(SPECIAL1);
	}

	m_bNeedPump = false;
	//WeaponSound(SPECIAL1);

	//SendWeaponAnim(ACT_SHOTGUN_PUMP);
	/*if (m_iClip1 <= 0)
	{
		//->FindVar("oc_enable_cam_attachment")->SetValue(1);
		pOwner->m_flNextAttack = gpGlobals->curtime + durationTime2;
		m_flNextPrimaryAttack = gpGlobals->curtime + durationTime2;
	}
	else
	{
		//->FindVar("oc_enable_cam_attachment")->SetValue(1);
		pOwner->m_flNextAttack = gpGlobals->curtime + durationTime;
		m_flNextPrimaryAttack = gpGlobals->curtime + durationTime;
	}*/
}

void CWeaponCombineSniper::PrimaryAttack()
{
	m_bNeedPump = true;

	durationTime = gpGlobals->curtime + GetFireRate()*0.3f;

	BaseClass::PrimaryAttack();
}
void CWeaponCombineSniper::ItemPostFrame(void)
{
	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (m_bInReload && m_iClip1 < 1)
		wasInReload = true;
	if (m_iClip1 == GetMaxClip1() && wasInReload && IsViewModelSequenceFinished())
	{
		Pump();
		wasInReload = false;
	}

	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime+(1.1f)))
	{
		Pump();
	}*/

	if (m_bNeedPump && gpGlobals->curtime > durationTime && m_iClip1 > 0)
	{
		Pump();
	}

	BaseClass::ItemPostFrame();
}

bool CWeaponCombineSniper::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool rRet;
	pSwitchingTo = NULL;
	rRet = BaseClass::Holster(pSwitchingTo);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	StopParticleEffects(this);
	StopParticleEffects(pPlayer);

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponCombineSniper::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	WeaponSound(SINGLE_NPC);
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	//pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCombineSniper::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCombineSniper::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
	{
		ShellOut(pEvent);
	}
	break;
	case EVENT_WEAPON_THROW3:
	{
		/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		StopEffects();
		if (pPlayer->SetFOV(this, 20, 0.2f))
		{
			m_bInZoom = false;
			// Send a message to hide the scope
			pPlayer->SetFOV(this, 0, 0.1f);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
		}*/

	}
	break;
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;
	//=======BriJee: Player thirdperson animations & NPC anims, allow shoot.
	case 4://EVENT_WEAPON_PISTOL_FIRE
	{
		DevMsg("Npc Shot \n");
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
		//=======BriJee: END. break added.
	}
}

// BJ: Added laser npc
void CWeaponCombineSniper::Operator_FrameUpdate(CBaseCombatCharacter *pOperator)
{

	if (GetOwnerEntity()->IsNPC())
	{
		Vector vecShootOrigin, vecShootDir, vecTarget, vecSrc, vecAiming;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		/*GetVectors( &vecAiming, NULL, NULL );
		vecSrc = WorldSpaceCenter() + vecAiming * 64;

		//vecShootDir = npc->GetActualShootPosition(vecSrc);

		vecTarget = npc->GetEnemy()->BodyTarget( vecSrc );*/

		/*Vector vecTarget = npc->GetActualShootPosition( shootOrigin );

		Vector vecAiming = npc->GetActualShootPosition();//GetAutoaimVector( AUTOAIM_2DEGREES );
		Vector	AimEnd	= vecSrc + ( vecAiming * MAX_TRACE_LENGTH );*/

		CBaseEntity *pEnemy = npc->GetEnemy();



		if (pEnemy)
		{

			vecTarget = pEnemy->WorldSpaceCenter();

			trace_t tr;
			AI_TraceLine(vecShootOrigin, vecTarget, MASK_SHOT , this, COLLISION_GROUP_NONE, &tr);

			if ((npc->FVisible(pEnemy)) && (npc->GetActiveWeapon()->m_iClip1 > 0) && (vecShootOrigin.DistTo(vecTarget) > 230) && (vecShootOrigin.DistTo(vecTarget) < 4500) && (!npc->IsMoving()) && (!npc->IsOnFire()) && (!npc->HasCondition(COND_LOW_PRIMARY_AMMO)))
			{
				//Msg("I SEE YOU \n");

				int LasAttachTP = LookupAttachment("laser");
				int MuzAttachTP = LookupAttachment("muzzle");

				pBeam = CBeam::BeamCreate("effects/bluelaser1.vmt", 1.0f);
				pBeam->SetColor(0, 100, 255);

				//pBeam->SetStartPos(vecTarget);	// end
				//pBeam->PointEntInit(vecTarget, this);	// end (MUST BE THE SAME) , Start point

				pBeam->SetStartPos(tr.startpos);	// end
				pBeam->PointEntInit(tr.startpos, this);	// end (MUST BE THE SAME) , Start point

				if (LasAttachTP)
					pBeam->SetEndAttachment(LasAttachTP /*LookupAttachment("muzzle")*/);	// blank?
				else if (MuzAttachTP)
					pBeam->SetEndAttachment(MuzAttachTP);

				//pBeam->SetStartPos( startPos );
				//pBeam->SetEndPos( endPos );

				pBeam->SetBrightness(255);
				pBeam->SetNoise(0);
				pBeam->SetWidth(1.0f);
				pBeam->SetEndWidth(0);
				pBeam->SetScrollRate(0);
				pBeam->SetFadeLength(0);
				pBeam->SetHaloTexture(sHaloSprite2);
				pBeam->SetHaloScale(4.0f);

				pBeam->LiveForTime(0.1f);
			}
		}
	}

	BaseClass::Operator_FrameUpdate(pOperator);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponCombineSniper::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;
	if (m_bInReload && !m_bInZoom)
		return;

	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		return;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
			// Send a message to hide the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(0);
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			m_bInZoom = true;
			// Send a message to Show the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
			cvar->FindVar("oc_state_InSecondFire")->SetValue(1);
			cvar->FindVar("oc_state_InSecondFire_357")->SetValue(1);
		}
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: Stop all zooming and special effects on the viewmodel
//-----------------------------------------------------------------------------
/*void CWeaponCombineSniper::StopEffects(void)
{
	// Stop zooming
	if (m_bInZoom)
	{
		ToggleZoom();
	}

}*/

/*void CWeaponCombineSniper::Zoom3x(void)		// old css, hl2 beta zoom
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();	// L1ght 15 : Check to avoid problems

	if (pWeapon == NULL || (cvar->FindVar("oc_state_IRsight_on")->GetInt())) //pWeapon->IsIronsighted() )
		return;

#ifndef CLIENT_DLL
	if (m_nZoomLevel >= sizeof(g_nZoomFOV) / sizeof(g_nZoomFOV[0]))
	{
		if (pPlayer->SetFOV(this, 0))
		{
			pPlayer->ShowViewModel(true);

			// Zoom out to the default zoom level
			WeaponSound(SPECIAL2);
			m_nZoomLevel = 0;

			// Light Kill : Scope test
			if (cvar->FindVar("oc_weapon_sniper_drawscope")->GetInt() == 1)
			{
				CombineSniperUndrawScope();
			}

		}
	}
	else
	{
		if (pPlayer->SetFOV(this, g_nZoomFOV[m_nZoomLevel]))
		{
			if (m_nZoomLevel == 0)
			{
				pPlayer->ShowViewModel(false);
			}

			// Light Kill : Scope test
			if (cvar->FindVar("oc_weapon_sniper_drawscope")->GetInt() == 1)
			{
				CombineSniperDrawScope();
			}

			WeaponSound(SPECIAL1);

			m_nZoomLevel++;
		}
	}
#endif

	m_fNextZoom = gpGlobals->curtime + 0.2;//CombineSniper_ZOOM_RATE;
}

void CWeaponCombineSniper::CombineSniperDrawScope(void)	// draw 2d scope
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter filter(pPlayer);
	UserMessageBegin(filter, "ShowScope");
	WRITE_BYTE(1);
	MessageEnd();
	//cvar->FindVar("crosshair")->SetValue(0);
	cvar->FindVar("oc_state_IRsight_on")->SetValue(1);
	cvar->FindVar("oc_state_InSecondFire")->SetValue(1);

	color32 ScopeEffect = { 0, 0, 0, 128 }; // Light Kill : Some "flash" effects
	UTIL_ScreenFade(pPlayer, ScopeEffect, 0.5, 0, FFADE_IN);
}
void CWeaponCombineSniper::CombineSniperUndrawScope(void)		// disable 2d scope
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	CSingleUserRecipientFilter filter(pPlayer);
	UserMessageBegin(filter, "ShowScope");
	WRITE_BYTE(0);
	MessageEnd();
	//cvar->FindVar("crosshair")->SetValue(1);
	cvar->FindVar("oc_state_IRsight_on")->SetValue(0);
	cvar->FindVar("oc_state_InSecondFire")->SetValue(0);
}

void CWeaponCombineSniper::Drop(const Vector &vecVelocity)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer != NULL)
	{
		if (m_nZoomLevel != 0)
		{
			if (pPlayer->SetFOV(this, 0))
			{
				pPlayer->ShowViewModel(true);
				m_nZoomLevel = 0;
			}

			// Light Kill : Scope test
			if (cvar->FindVar("oc_weapon_sniper_drawscope")->GetInt() == 1)
			{
				CombineSniperUndrawScope();
			}
		}
	}

	//LaserOff();

	StopEffects();	// added 

	BaseClass::Drop(vecVelocity);
}*/