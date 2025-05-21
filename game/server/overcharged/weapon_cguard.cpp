//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:		Combine guard gun, strider destroyer
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "grenade_ar2.h"
#include "soundent.h"
#include "explode.h"
#include "shake.h"
#include "ndebugoverlay.h"
#include "particle_parse.h"
#include "in_buttons.h"

#include "weapon_cguard.h"
#include "tier0/memdbgon.h"


extern ConVar hl2_walkspeed;
extern ConVar hl2_normspeed;
extern ConVar hl2_sprintspeed;

class CTEConcussiveExplosion : public CTEParticleSystem
{
public:
	DECLARE_CLASS(CTEConcussiveExplosion, CTEParticleSystem);
	DECLARE_SERVERCLASS();

	CTEConcussiveExplosion(const char *name);
	virtual	~CTEConcussiveExplosion(void);

	virtual	void Create(IRecipientFilter& filter, float delay = 0.0f);

	CNetworkVector(m_vecNormal);
	CNetworkVar(float, m_flScale);
	CNetworkVar(int, m_nRadius);
	CNetworkVar(int, m_nMagnitude);

};

IMPLEMENT_SERVERCLASS_ST(CTEConcussiveExplosion, DT_TEConcussiveExplosion)
SendPropVector(SENDINFO(m_vecNormal), -1, SPROP_COORD),
SendPropFloat(SENDINFO(m_flScale), 0, SPROP_NOSCALE),
SendPropInt(SENDINFO(m_nRadius), 32, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_nMagnitude), 32, SPROP_UNSIGNED),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTEConcussiveExplosion::CTEConcussiveExplosion(const char *name) : BaseClass(name)
{
	m_nRadius = 0;
	m_nMagnitude = 0;
	m_flScale = 0.0f;

	m_vecNormal.Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTEConcussiveExplosion::~CTEConcussiveExplosion(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : msg_dest - 
//			delay - 
//			origin - 
//			*recipient - 
//-----------------------------------------------------------------------------
void CTEConcussiveExplosion::Create(IRecipientFilter& filter, float delay)
{
	engine->PlaybackTempEntity(filter, delay, (void *)this, GetServerClass()->m_pTable, GetServerClass()->m_ClassID);
}

// Singleton to fire TEExplosion objects
static CTEConcussiveExplosion g_TEConcussiveExplosion("ConcussiveExplosion");

void TE_ConcussiveExplosion(IRecipientFilter& filter, float delay,
	const Vector* pos, float scale, int radius, int magnitude, const Vector* normal)
{
	g_TEConcussiveExplosion.m_vecOrigin = *pos;
	g_TEConcussiveExplosion.m_flScale = scale;
	g_TEConcussiveExplosion.m_nRadius = radius;
	g_TEConcussiveExplosion.m_nMagnitude = magnitude;

	if (normal)
		g_TEConcussiveExplosion.m_vecNormal = *normal;
	else
		g_TEConcussiveExplosion.m_vecNormal = Vector(0, 0, 1);

	// Send it over the wire
	g_TEConcussiveExplosion.Create(filter, delay);
}

//Temp ent for the blast

class CConcussiveBlast : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CConcussiveBlast, CBaseEntity);

	int		m_spriteTexture;

	CConcussiveBlast(void) {}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------
	void Precache(void)
	{
		m_spriteTexture = engine->PrecacheModel("sprites/lgtning.vmt");

		BaseClass::Precache();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------

	void Explode(float magnitude)
	{
		//Create a concussive explosion
		CPASFilter filter(GetAbsOrigin());

		Vector vecForward;
		AngleVectors(GetAbsAngles(), &vecForward);
		TE_ConcussiveExplosion(filter, 0.0,
			&GetAbsOrigin(),//position
			1.0f,	//scale
			256 * magnitude,	//radius
			175 * magnitude,	//magnitude
			&vecForward);	//normal

		int	colorRamp = random->RandomInt(128, 255);

		//Shockring
		CBroadcastRecipientFilter filter2;
		te->BeamRingPoint(filter2, 0,
			GetAbsOrigin(),	//origin
			16,			//start radius
			300 * magnitude,		//end radius
			m_spriteTexture, //texture
			0,			//halo index
			0,			//start frame
			2,			//framerate
			0.3f,		//life
			128,		//width
			16,			//spread
			0,			//amplitude
			colorRamp,	//r
			colorRamp,	//g
			255,		//g
			24,			//a
			128			//speed
			);

		//Do the radius damage
		RadiusDamage(CTakeDamageInfo(this, this, 125 * magnitude, DMG_BLAST), GetAbsOrigin(), 256 * magnitude, CLASS_NONE, NULL);

		UTIL_Remove(this);
		
	}
};

LINK_ENTITY_TO_CLASS(concussiveblast, CConcussiveBlast);

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CConcussiveBlast)

//	DEFINE_FIELD( CConcussiveBlast, m_spriteTexture,	FIELD_INTEGER ),

END_DATADESC()



IMPLEMENT_SERVERCLASS_ST(CWeaponCGuard, DT_WeaponCGuard)
//SendPropBool(SENDINFO(EnableSphere)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_cguard, CWeaponCGuard);
PRECACHE_WEAPON_REGISTER(weapon_cguard);


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CWeaponCGuard)

	DEFINE_FIELD(m_flChargeTime, FIELD_TIME),
	DEFINE_FIELD(m_bFired, FIELD_BOOLEAN),
	DEFINE_FIELD(Destroy, FIELD_BOOLEAN),
	DEFINE_FIELD(_hNoise, FIELD_EHANDLE),
	//DEFINE_FIELD(Sphere, FIELD_CLASSPTR),

END_DATADESC()

//-----------------------------------------------------------------------------
// Maps base activities to weapons-specific ones so our characters do the right things.
//-----------------------------------------------------------------------------
acttable_t CWeaponCGuard::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },

	{ ACT_IDLE, ACT_IDLE_SMG1, true },

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

	/*{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SNIPER_RIFLE, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, false },*/
};

IMPLEMENT_ACTTABLE(CWeaponCGuard);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCGuard::CWeaponCGuard(void)
{
	m_flNextPrimaryAttack = gpGlobals->curtime;
	m_flChargeTime = gpGlobals->curtime;
	m_bFired = false;
	Destroy = false;
	InHolster = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::Precache(void)
{
	UTIL_PrecacheOther("concussiveblast");
	m_beamIndex = engine->PrecacheModel("sprites/bluelaser1.vmt");
	m_haloIndex = engine->PrecacheModel("sprites/blueshaft1.vmt");
	PrecacheScriptSound("Weapon_CGuard.Draw1");
	PrecacheScriptSound("Weapon_CombineGuard.Single");
	PrecacheScriptSound("Weapon_CGuard.Draw2");
	PrecacheParticleSystem("ion_laser");
	PrecacheParticleSystem("ion_laser_dot");
	PrecacheParticleSystem("ion_laser_dot2");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::AlertTargets(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	// Fire the bullets
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	Vector	impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	trace_t	tr;

	UTIL_TraceLine(vecSrc, impactPoint, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if ((vecSrc - tr.endpos).Length() > 1024)
		return;

	CSoundEnt::InsertSound(SOUND_DANGER, tr.endpos, 128, 0.5f);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::UpdateLasers(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if ((m_flChargeTime < gpGlobals->curtime) || (!m_bFired))
		return;

	//Vector	start, end, v_forward, v_right, v_up;

	if (pPlayer->GetWaterLevel() != 3)
	{
		/*CEffectData	fireData;
		fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
		fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
		fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
		fireData.m_nAttachmentIndex = 21;
		DispatchEffect("FireEvent", fireData);*/
	}

	if ((pPlayer->m_nButtons & IN_ATTACK2))
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		//DestroyEffect();
		StopSound("Weapon_CombineGuard.Single");
		return;
	}

	/*Vector vecAiming = pPlayer->GetAutoaimVector(0);
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Fire(vecSrc, vecAiming);

	pPlayer->GetVectors(&v_forward, &v_right, &v_up);

	//Get the position of the laser
	start = pPlayer->Weapon_ShootPosition();

	start += (v_forward * 8.0f) + (v_right * 3.0f) + (v_up * -2.0f);

	end = start + (v_forward * MAX_TRACE_LENGTH);

	float	angleOffset = (1.0f - (m_flChargeTime - gpGlobals->curtime)) / 1.0f;
	//float	angleOffset = ((cvar->FindVar("oc_weapon_cguard_charge_time")->GetInt()-11) - (m_flChargeTime - gpGlobals->curtime)) / (cvar->FindVar("oc_weapon_cguard_charge_time")->GetInt()-11);
	Vector	offset[4];

	offset[0] = Vector(0.0f, 0.5f, -0.5f);
	offset[1] = Vector(0.0f, 0.5f, 0.5f);
	offset[2] = Vector(0.0f, -0.5f, -0.5f);
	offset[3] = Vector(0.0f, -0.5f, 0.5f);

	QAngle  v_ang;
	Vector	v_dir;

	angleOffset *= 2.0f;

	if (angleOffset > 1.0f)
		angleOffset = 1.0f;

	for (int i = 0; i < 4; i++)
	{
		Vector	ofs = start + (v_forward * offset[i][0]) + (v_right * offset[i][1]) + (v_up * offset[i][2]);

		float hScale = (offset[i][1] <= 0.0f) ? 1.0f : -1.0f;
		float vScale = (offset[i][2] <= 0.0f) ? 1.0f : -1.0f;

		VectorAngles(v_forward, v_ang);
		v_ang[PITCH] = UTIL_AngleMod(v_ang[PITCH] + ((1.0f - angleOffset) * 15.0f * vScale));
		v_ang[YAW] = UTIL_AngleMod(v_ang[YAW] + ((1.0f - angleOffset) * 15.0f * hScale));

		AngleVectors(v_ang, &v_dir);

		trace_t	tr;
		UTIL_TraceLine(ofs, ofs + (v_dir * MAX_TRACE_LENGTH), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		//UTIL_Beam(ofs, tr.endpos, m_beamIndex, 0, 0, 0.4f, 0.1f, 1, 0, 1, 0, 255, 255, 255, 32, 100);

		//UTIL_Beam(ofs, tr.endpos, m_haloIndex, 0, 0, 0.4f, 0.1f, 1, 0, 1, 16, 255, 255, 255, 8, 100);
		Vector Muzzle;
		Muzzle.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();//pViewModel->tracerOrig_X;
		Muzzle.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();//pViewModel->tracerOrig_Y;
		Muzzle.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();//pViewModel->tracerOrig_Z;

		//DispatchParticleEffect("Weapon_cguard_Cannon", Muzzle, tr.endpos, v_ang, pPlayer->GetViewModel());
		//int iEntIndex = pPlayer->GetViewModel()->entindex();
		//int	startAttachment = LookupAttachment("muzzle");
		//UTIL_ParticleTracer("ion_laser", Muzzle, tr.endpos, i, startAttachment, 0);//OverCharged
		

		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &tr.endpos, 0, 0, 255, 0, 200, 0.01, 0);
	}


	Vector	vForward, vRight, vUp, vThrowPos;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();

	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		vThrowPos += vForward * 22.0f;
		vThrowPos += vRight * 0.7f;// *1.0f;
		vThrowPos += vUp * -3.4f;
		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &vThrowPos, 0, 70, 255, 2, 100, 0.03, 0);
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		vThrowPos += vForward * 22.0f;
		vThrowPos += vRight * 5.4f;
		vThrowPos += vUp * -3.4f;
		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &vThrowPos, 0, 70, 255, 2, 100, 0.03, 0);
	}*/
}

void CWeaponCGuard::Fire(const Vector &vecOrigSrc, const Vector &vecDir)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	Vector vecDest = vecOrigSrc + (vecDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine(vecOrigSrc, vecDest, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	//  UTIL_Smoke(tr.endpos, random->RandomInt(5, 10), 10);//ƒым от луча
	//	UTIL_DecalTrace(&tr, "RedGlowFade");
	//	UTIL_DecalTrace( &tr, "FadingScorch" ); //√арь от луча

	if (tr.allsolid)
		return;

	CBaseEntity *pEntity = tr.m_pEnt;
	if (pEntity == NULL)
		return;

	Vector vecUp, vecRight;
	QAngle angDir;

	VectorAngles(vecDir, angDir);
	AngleVectors(angDir, NULL, &vecRight, &vecUp);

	Vector tmpSrc = vecOrigSrc + (vecUp * -8) + (vecRight * 3);
	//UpdateEffect(tmpSrc, tr.endpos);

	//DispatchParticleEffect("CombineGuard_Attack_Laser", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", false);
	//	DispatchParticleEffect("ball", tmpSrc, tr.endpos, vec3_angle, NULL);
	//		UpdateEffect( tr.endpos );
	// UTIL_ImpactTrace( &tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss" ); // след на стене		
}

void CWeaponCGuard::UpdateEffect(const Vector &startPoint, const Vector &endPoint)
{
	/*CreateEffect();
	if (_hNoise)
	{
		_hNoise->SetStartPos(endPoint);
	}*/
}

void CWeaponCGuard::CreateEffect(void)
{
	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	DestroyEffect();

	_hNoise = CBeam::BeamCreate(BEAM_SPRITE, 4.0);
	_hNoise->PointEntInit(GetAbsOrigin(), this);
	_hNoise->SetEndAttachment(1);
	_hNoise->AddSpawnFlags(SF_BEAM_TEMPORARY);
	_hNoise->SetOwnerEntity(pPlayer);
	_hNoise->SetBrightness(2255);
	_hNoise->SetColor(0, 0, 255);*/

}



void CWeaponCGuard::DestroyEffect(void)
{
	/*if (_hNoise)
	{
		UTIL_Remove(_hNoise);
		_hNoise = NULL;
	}*/
}

#define CGUARD_MSG_SHOT_START	2
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	if (m_flChargeTime >= gpGlobals->curtime || m_bFired)
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	/*if (pPlayer->GetWaterLevel() != 3)
	{
		CEffectData	fireData;
		fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
		fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
		fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
		fireData.m_nAttachmentIndex = 21;
		DispatchEffect("FireEvent", fireData);
	}*/

	//AlertTargets();

	WeaponSound(SPECIAL1);

	//UTIL_ScreenShake( GetAbsOrigin(), 10.0f, 100.0f, 2.0f, 128, SHAKE_START, false );

	m_flChargeTime = gpGlobals->curtime + (cvar->FindVar("oc_weapon_cguard_charge_time")->GetFloat());
	m_bFired = true;

	SendWeaponAnim(GetPrimaryAttackActivity());

	m_flNextPrimaryAttack = m_flChargeTime;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	DispatchParticleEffect(STRING(GetWpnData().iMuzzleFlashDelayed), PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), GetWpnData().iAttachment, false);
	
	//DispatchParticleEffect("charge", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);

	/*Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;		// BriJee OVR: Projectile start position
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();
	vThrowPos += vForward * 12.0f;
	vThrowPos += vRight * 7.8f;
	vThrowPos += vUp * -1.8f;

	QAngle angAiming;
	VectorAngles(vThrowPos, angAiming);
	Vector MuzzleS;
	MuzzleS.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
	MuzzleS.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
	MuzzleS.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();
	QAngle MuzzleAngle;
	VectorAngles(MuzzleS, MuzzleAngle);
	MuzzleAngle.x = MuzzleAngle.x - 90.0f;

	Sphere = CCguard_Sphere::SphereCreate(MuzzleS, MuzzleAngle, 0, pPlayer->GetViewModel());
	Sphere->SetModelScale(0.0f);*/

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;


	UpdateLasers();

	if ((m_flChargeTime < gpGlobals->curtime) && (m_bFired == true))
	{
		DelayedFire();
	}
}

void CWeaponCGuard::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}
void CWeaponCGuard::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	//WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, pOperator->GetAttackSpread(this), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);

	m_iClip1 = m_iClip1 - 1;
}
//-----------------------------------------------------------------------------
// Purpose: Create a concussive blast entity and detonate it
//-----------------------------------------------------------------------------
void CreateConcussiveBlast(const Vector &origin, const Vector &surfaceNormal, CBaseEntity *pOwner, float magnitude)
{
	QAngle angles;
	VectorAngles(surfaceNormal, angles);
	CConcussiveBlast *pBlast = (CConcussiveBlast *)CBaseEntity::Create("concussiveblast", origin, angles, pOwner);

	if (pBlast)
	{
		pBlast->Explode(magnitude);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCGuard::DelayedFire(void)
{
	if (m_flChargeTime >= gpGlobals->curtime)
		return;

	if (!m_bFired)
		return;


	DestroyEffect();

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	// Abort here to handle burst and auto fire modes
	if ((GetMaxClip1() != -1 && m_iClip1 == 0) || (GetMaxClip1() == -1 && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	//if (GetSequence() != SelectWeightedSequence(ACT_VM_PRIMARYATTACK))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		m_iClip1 = m_iClip1 - 1;
	}

	// Fire the bullets
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	//Factor in the view kick
	AddViewKick();

	Vector	impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceHull(vecSrc, impactPoint, Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
	ShouldDrawWaterImpacts(tr);
	CreateConcussiveBlast(tr.endpos, tr.plane.normal, this, 1.0);

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector Muzzle;
	Muzzle = GetClientMuzzleVector();
	//Muzzle.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();//pViewModel->tracerOrig_X;
	//Muzzle.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();//pViewModel->tracerOrig_Y;
	//Muzzle.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();//pViewModel->tracerOrig_Z;

	if (pPlayer->GetWaterLevel() != 3)
	{
		CEffectData	fireData;
		fireData.m_nEntIndex = pPlayer->GetViewModel()->entindex();
		fireData.m_vOrigin = pPlayer->GetViewModel()->GetLocalOrigin();
		fireData.m_vAngles = pPlayer->GetViewModel()->GetLocalAngles();
		fireData.m_nAttachmentIndex = 21;
		DispatchEffect("FireEvent", fireData);
	}

	Vector	vFirePos, vForward1, vRight1, vUp1;
	pPlayer->GetVectors(&vForward1, &vRight1, &vUp1);
	vFirePos = pPlayer->Weapon_ShootPosition();
	vFirePos += (vForward1 * 8.0f) + (vRight1 * 6.0f) + (vUp1 * -2.8f);
	//DispatchParticleEffect("CombineGuard_Attack_Laser", Muzzle, tr.endpos, vec3_angle, NULL);
	//DispatchParticleEffect("Weapon_Combine_cguard_Cannon", Muzzle, tr.endpos, vec3_angle, NULL);
	//DispatchParticleEffect("weapon_tracer_cguard", Muzzle, tr.endpos, vec3_angle, NULL);
	DispatchParticleEffect("Weapon_Combine_Ion_Cannon", Muzzle, tr.endpos, vec3_angle, NULL);
	DispatchParticleEffect("CombineGuard_Outer_plr", Muzzle, vec3_angle);
	DispatchParticleEffect("CombineGuard_Outer", tr.endpos, vec3_angle);
	DispatchParticleEffect("CombineGuard_Outer_impact", tr.endpos, vec3_angle);
	DispatchParticleEffect("weapon_muzzle_smoke_cguard", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);//true
	//DispatchParticleEffect("weapon_muzzle_smoke_cguard1", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);

	m_bFired = false;
}

bool CWeaponCGuard::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return false;

	m_flChargeTime = 0;
	m_bFired = false;
	InHolster = true;

	StopWeaponSound(SINGLE);

	StopWeaponSound(SPECIAL1);

	Destroy = true;
	DestroyEffect();

	return BaseClass::Holster(pSwitchingTo);

}

//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponCGuard::ShouldDrawWaterImpacts(const trace_t &shot_trace)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (WaterContents(shot_trace.startpos))
		return false;

	// We must end inside of water
	if (!WaterContents(shot_trace.endpos))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(shot_trace.startpos, shot_trace.endpos, (CONTENTS_WATER | CONTENTS_SLIME), UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &waterTrace);


	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(2.0, 4.0f);	// Water effect scale

		// See if we hit slime
		if (FBitSet(waterTrace.contents, CONTENTS_SLIME))
		{
			FSetBit(data.m_fFlags, FX_WATER_IN_SLIME);
		}

		CPASFilter filter(data.m_vOrigin);
		te->DispatchEffect(filter, 0.0, data.m_vOrigin, "watersplash", data);
	}
	return true;
}




