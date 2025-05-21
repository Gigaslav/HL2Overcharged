//========= Copyright © 2008, Valve, All rights reserved. ============
//
// Purpose: Tau Cannon Super gun
//
//==================================================================================

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "basehlcombatweapon.h"
#include "decals.h"
#include "beam_shared.h"
#include "AmmoDef.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "soundenvelope.h"
#include "soundent.h"
#include "shake.h"
#include "explode.h"
#include "weapon_tau.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
ConVar   oc_weapon_tau_shake("oc_weapon_tau_shake", "0", FCVAR_ARCHIVE);
ConVar   oc_weapon_tau_animation("oc_weapon_tau_animation", "0", FCVAR_ARCHIVE);
ConVar	 oc_weapon_tau_beam_life("oc_weapon_tau_beam_life", "0", FCVAR_ARCHIVE);
ConVar	 oc_weapon_tau_afterfire_soundtime("oc_weapon_tau_afterfire_soundtime", "10", FCVAR_ARCHIVE);
//-----------------------------------------------------------------------------
// Declarations
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST( CWeaponTau, DT_WeaponTau )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_tau, CWeaponTau );
PRECACHE_WEAPON_REGISTER(weapon_tau);

acttable_t	CWeaponTau::m_acttable[] = 
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

IMPLEMENT_ACTTABLE( CWeaponTau );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponTau )

	DEFINE_FIELD( m_hViewModel,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextChargeTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flNextSpinTime, FIELD_TIME ),
	DEFINE_FIELD(m_flNextSoundTime, FIELD_TIME),
	DEFINE_FIELD( m_flChargeStartTime,	FIELD_TIME ),
	DEFINE_FIELD(m_flNextZapTime, FIELD_TIME),
	DEFINE_FIELD( m_bCharging,		FIELD_BOOLEAN ),
	DEFINE_FIELD( isCharging, FIELD_BOOLEAN ),
	DEFINE_FIELD(blockSoundThink, FIELD_BOOLEAN),
	DEFINE_FIELD( m_bChargeIndicated,	FIELD_BOOLEAN ),
	DEFINE_FIELD(minZap, FIELD_FLOAT),
	DEFINE_FIELD(maxZap, FIELD_FLOAT),

	DEFINE_SOUNDPATCH( m_sndCharge ),

END_DATADESC()



ConVar sk_plr_dmg_tau("sk_plr_dmg_tau", "0");
ConVar sk_plr_max_dmg_tau("sk_plr_max_dmg_tau", "0");
/*extern*/ //ConVar sk_plr_dmg_gauss;	// sdk 2013 bug?
/*extern*/ //ConVar sk_plr_max_dmg_gauss;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponTau::CWeaponTau( void )
{
	m_flNextSoundTime = 0;
	m_hViewModel 		= NULL;
	m_flNextChargeTime	= 0;
	m_flNextSpinTime = 0;
	m_flChargeStartTime	= 0;
	m_flNextZapTime = 0;
	m_sndCharge		= NULL;
	m_bCharging		= false;
	isCharging = false;
	blockSoundThink = false;
	m_bChargeIndicated	= false;
	m_bReloadsSingly	= false;
	minZap = 0.f;
	maxZap = 0.f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTau::Precache( void )
{
	enginesound->PrecacheSound( "Weapon_Gauss.ChargeLoop" );
	PrecacheParticleSystem("gauss_sparks01");
	PrecacheParticleSystem("gauss_normal");
	PrecacheParticleSystem("gauss_balls01");
	PrecacheParticleSystem("gauss_charge");
	PrecacheParticleSystem("gauss_normal2");
	PrecacheParticleSystem("gauss_zap");
	PrecacheParticleSystem("gauss_penetration_glow");
	PrecacheScriptSound("Weapon_gauss.Draw");
	PrecacheScriptSound("Weapon_gauss.After");
	PrecacheScriptSound("Weapon_gauss.Double");
	PrecacheScriptSound("Weapon_gauss.Special3");
	PrecacheScriptSound("Weapon_gauss.Common");
//	PrecacheScriptSound("Weapon_gauss.Recharge");
	BaseClass::Precache();
}

Activity CWeaponTau::GetDrawActivity(void)
{
	return BaseClass::GetDrawActivity();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTau::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTau::Fire(bool secondary, float damage)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( !pOwner ){ return; }


//	m_flChargeTime2 = 0;

	m_bCharging = false;
	isCharging = m_bCharging;
	m_bInReload = false;

	m_flNextSoundTime = gpGlobals->curtime;// +random->RandomFloat(0.05f, 0.02f);
	blockSoundThink = true;

	if ( m_hViewModel == NULL )
	{
		CBaseViewModel *vm = pOwner->GetViewModel();

		if ( vm )
		{
			m_hViewModel.Set( vm );
		}
	}

	float flDamage = secondary ? damage : sk_plr_dmg_tau.GetFloat();

	Vector		vecSrc = pOwner->Weapon_ShootPosition();
	Vector		aimDir = pOwner->GetAutoaimVector(0);
	Vector		vecDest = vecSrc + aimDir *MAX_TRACE_LENGTH;
	bool		fFirstBeam = true;
	bool		fHasPunched = false;
	float		flMaxFrac = 1.0;
	int			nMaxHits = secondary ? 1 : 2;
	trace_t		trEnd;

	if (secondary)
		trEnd.startpos = vecSrc;

	CTakeDamageInfo dmgInfo(this, pOwner, flDamage, DMG_GAUSS);

	CTakeDamageInfo dmgInfo2(this, pOwner, flDamage, DMG_BULLET);

	while (flDamage > 10 && nMaxHits > 0)
	{
		trace_t	tr;

		nMaxHits--;

		UTIL_TraceLine(vecSrc, vecDest, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

		DrawBeam(tr.startpos, tr.endpos, 1, fFirstBeam, secondary);

		UTIL_DecalTrace(&tr, "FadingScorch"); //Гарь от луча

		if (secondary)
			trEnd.endpos = tr.endpos;
		else
			trEnd = tr;

		if (tr.allsolid)
		{
			break;
		}

		CBaseEntity *pHit = tr.m_pEnt;

		if (pHit == NULL)
		{
			break;
		}

		if (fFirstBeam)
		{
			fFirstBeam = false;
		}

		float n;

		n = -DotProduct(tr.plane.normal, aimDir);

		if (pHit)
		{
			if (pHit->m_takedamage != DAMAGE_NO)
			{
				ClearMultiDamage();
				
				Vector Dir = /*secondary ? aimDir*-1 : */aimDir;
				//DevMsg("Dir: %.2f, %.2f, %.2f\n", Dir.x, Dir.y, Dir.z);

				if (pHit != NULL && pHit->IsNPC())
				{
					CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, Dir, tr.endpos);
					pHit->DispatchTraceAttack(dmgInfo, Dir, &tr);
				}
				else if (pHit != NULL && !pHit->IsNPC())
				{
					CalculateBulletDamageForce(&dmgInfo2, m_iPrimaryAmmoType, Dir, tr.endpos);
					pHit->DispatchTraceAttack(dmgInfo2, Dir, &tr);
				}
			}

			ApplyMultiDamage();

			//if (n >= 0.5)
			{
				if (secondary)
					UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGaussCharged");
				else
					UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

				CPVSFilter filter(tr.endpos);

				te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
			}

			//ShouldDrawWaterImpacts(tr);

			//CPVSFilter filter(tr.endpos);

			//te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);

			if (!secondary && !pHit->IsBSPModel())
			{
				fHasPunched = true;

				break;
			}
			
		}

		//if (secondary)
		{
			//ApplyMultiDamage();

			//UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

			ShouldDrawWaterImpacts(tr);

		}

		if (pHit && pHit->IsBSPModel() && (pHit->m_takedamage == DAMAGE_NO))
		{


			if (n < 0.5) // 60 degrees
			{
				Vector r;

				r = 2.0 * tr.plane.normal * n + aimDir;
				flMaxFrac = flMaxFrac - tr.fraction;
				aimDir = r;
				vecSrc = tr.endpos;
				vecDest = vecSrc + aimDir * MAX_TRACE_LENGTH;

				// explode a bit
				RadiusDamage(CTakeDamageInfo(this, pOwner, flDamage * n, (DMG_BLAST | DMG_GAUSS)), tr.endpos, flDamage * n * 2.5, CLASS_NONE, pOwner);

				if (n == 0)
					n = 0.1;

				flDamage = flDamage * (1 - n);
			}
			else
			{
				if (!secondary)
					fHasPunched = true;

				if (fHasPunched)
				{
					break;
				}

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (secondary)
				{
					trace_t beam_tr;

					UTIL_TraceLine(tr.endpos + aimDir * 8, vecDest, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &beam_tr);

					if (!beam_tr.allsolid)
					{
						// trace backwards to find exit point
						UTIL_TraceLine(beam_tr.endpos, tr.endpos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &beam_tr);

						float n = (beam_tr.endpos - tr.endpos).Length()/100;

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;

							flDamage -= n;

							float flDamageRadius;

							if (g_pGameRules->IsMultiplayer())
							{
								flDamageRadius = flDamage * 1.75;  // Old code == 2.5
							}
							else
							{
								flDamageRadius = flDamage * 2.5;
							}

							RadiusDamage(CTakeDamageInfo(this, pOwner, flDamage, (DMG_BLAST | DMG_GAUSS)), beam_tr.endpos - aimDir * 2, flDamageRadius, CLASS_NONE, pOwner);

							vecSrc = beam_tr.endpos + aimDir;
						}
						else
						{
							fHasPunched = true;

							break;
						}
					}
					else
					{
						flDamage = 0;
					}
				}
				else
				{
					flDamage = 0;
				}
			}
		}
		else
		{
			vecSrc = tr.endpos + aimDir;
		}
	}

	if (secondary)
	{
		CEffectData data;
		data.m_vOrigin = trEnd.endpos;
		data.m_nEntIndex = pOwner->entindex();
		DispatchEffect("GaussLightCharged", data);
	}
	else
	{
		CEffectData data;
		data.m_vOrigin = trEnd.endpos;
		data.m_nEntIndex = pOwner->entindex();
		data.m_vStart.x = GetWpnData().iMuzzleFlashLightR;
		data.m_vStart.y = GetWpnData().iMuzzleFlashLightG;
		data.m_vStart.z = GetWpnData().iMuzzleFlashLightB;
		DispatchEffect("GaussLight", data);
	}

	/*if (secondary)
		UTIL_ImpactTrace(&trEnd, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGaussCharged");
	else
		UTIL_ImpactTrace(&trEnd, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

	CPVSFilter filter(trEnd.endpos);

	te->GaussExplosion(filter, 0.0f, trEnd.endpos, trEnd.plane.normal, 0);*/

	/*if (!secondary)
	{
		UTIL_ImpactTrace(&trEnd, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

		ShouldDrawWaterImpacts(trEnd);

		CPVSFilter filter(trEnd.endpos);

		te->GaussExplosion(filter, 0.0f, trEnd.endpos, trEnd.plane.normal, 0);
	}*/

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	AddViewKick();

	pOwner->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

}

//-----------------------------------------------------------------------------
// Purpose: Charged fire
//-----------------------------------------------------------------------------
void CWeaponTau::ChargedFire( void )
{

	//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	//if ( !pOwner ){ return; }

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	pPlayer->DoMuzzleFlash();


	m_flNextSoundTime = gpGlobals->curtime + random->RandomFloat(oc_weapon_tau_afterfire_soundtime.GetFloat() * 0.9f, oc_weapon_tau_afterfire_soundtime.GetFloat() * 1.1f);//random->RandomFloat(0.5f, 1.2f);//gpGlobals->curtime + cvar->FindVar("oc_weapon_gauss_afterfire_soundtime")->GetFloat();// +RandomInt(-5, 5);

	blockSoundThink = true;

	WeaponSound( WPN_DOUBLE );

	if (pPlayer->GetActiveWeapon())
	{
		if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].FireIronsighted1);
		}
		else
		{
			if (oc_weapon_tau_animation.GetInt() == 0)
			{
				SendWeaponAnim(GetSecondaryAttackActivity());
			}
			if (oc_weapon_tau_animation.GetInt() == 1)
			{
				SendWeaponAnim(GetWpnData().animData[m_bFireMode].FirePrimary1);
			}
			if (oc_weapon_tau_animation.GetInt() == 2)
			{
				int i = RandomInt(1, 2);
				if (i == 1)
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].FirePrimary1);
				}
				if (i == 2)
				{
					SendWeaponAnim(GetSecondaryAttackActivity());
				}
			}
		}
	}

	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

	pPlayer->SetAnimation(PLAYER_ATTACK1);
	StopChargeSound();

	m_bCharging = false;
	isCharging = m_bCharging;
	m_bInReload = false;
	m_bChargeIndicated = false;
	m_flNextZapTime = 0.f;

	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.2f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	// Calc final damage
	float flChargeAmount = (gpGlobals->curtime - m_flChargeStartTime) / MAX_TAU_CHARGE_TIME;

	if (flChargeAmount > 1.0f){ flChargeAmount = 1.0f; }

	float flDamage = sk_plr_dmg_tau.GetFloat() + ((sk_plr_max_dmg_tau.GetFloat() - sk_plr_dmg_tau.GetFloat()) * flChargeAmount);

	Fire(true, flDamage);

#if 0

	/*DispatchParticleEffect("gauss_muzzle_flash", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged
	DispatchParticleEffect("gauss_normal2", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);//OverCharged
	DispatchParticleEffect("weapon_muzzle_smoke2", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);
	DispatchParticleEffect("gauss_smoke", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), 1, false);*/

#endif

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat(-4.0f, -8.0f);
	viewPunch.y = random->RandomFloat(-0.25f, 0.25f);
	viewPunch.z = 0;

	pPlayer->ViewPunch(viewPunch);

	Vector	recoilForce = pPlayer->GetAbsVelocity() - pPlayer->GetAutoaimVector(0) * (flDamage * 1.0f);
	recoilForce[2] += 15.0f;
	pPlayer->SetAbsVelocity(recoilForce);

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

}

//-----------------------------------------------------------------------------
// Purpose: Draw Beam
//-----------------------------------------------------------------------------
void CWeaponTau::DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle, bool secondary)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	float life = oc_weapon_tau_beam_life.GetFloat();
	//Check to store off our view model index
	if ( m_hViewModel == NULL )// Приводит к вылету игры(жалуется на player.cpp(489-я строка) return m_hViewModel[index]
	{
		CBaseViewModel *vm = pOwner->GetViewModel();
		if (!vm)
		return;

		if ( vm )
		{
			m_hViewModel.Set( vm );
		}

	}

	color32 beamColor;
	beamColor.a = 255;
	beamColor.r = 255;
	beamColor.g = secondary ? 255 : 145 + random->RandomInt(-16, 16);
	beamColor.b = secondary ? 255 : 0;

	CBeam *pBeam = CBeam::BeamCreate( TAU_BEAM_SPRITE, width );
	
	if ( useMuzzle )
	{
		pBeam->PointEntInit( endPos, m_hViewModel );
		pBeam->SetEndAttachment( 1 );
	}
	else
	{
		pBeam->SetStartPos( startPos );
		pBeam->SetEndPos( endPos );
	}

	float widthKoef = secondary ? 4 : 1;
	pBeam->SetBrightness(beamColor.a);
	pBeam->SetColor(beamColor.r, beamColor.g, beamColor.b);
	pBeam->SetWidth(width * widthKoef);
	pBeam->SetEndWidth(width * widthKoef);
	pBeam->RelinkBeam();
	pBeam->LiveForTime(life);

//	pBeam->LiveForTime( 0.025f );

	// Some sparks
	for ( int i = 0; i < 3; i++ )
	{
		pBeam = CBeam::BeamCreate( TAU_BEAM_SPRITE, (width/2.0f) + i );
		
		if ( useMuzzle )
		{
			pBeam->PointEntInit( endPos, m_hViewModel );
			pBeam->SetEndAttachment( 1 );
		}
		else
		{
			pBeam->SetStartPos( startPos );
			pBeam->SetEndPos( endPos );
		}
		
		pBeam->SetBrightness( random->RandomInt( 155, 255 ) );
		pBeam->SetColor( 255, 255, 150+random->RandomInt( 0, 64 ) );
		pBeam->RelinkBeam();
		pBeam->LiveForTime(life);
		pBeam->SetNoise( 1.6f * i );
		pBeam->SetEndWidth( 0.04f );
	}

	//QAngle pAngles;
	//VectorAngles(endPos, pAngles);
	//DispatchParticleEffect("Gauss_impact_round_sparks", endPos, pAngles);

	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	Vector	vForward, vRight, vUp, vThrowPos;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	vThrowPos = pPlayer->EyePosition();
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		vThrowPos += vForward * 2.0f;
		vThrowPos += vRight * 0.1f;// *1.0f;
		vThrowPos += vUp * -3.4f;
		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &vThrowPos, 140, 140, 0, 2, 120, 0.09, 0);
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		vThrowPos += vForward * 2.0f;
		vThrowPos += vRight * 7.4f;
		vThrowPos += vUp * -3.4f;
		CPASFilter filter(GetAbsOrigin());
		te->DynamicLight(filter, 0.0, &vThrowPos, 140, 140, 0, 1, 120, 0.09, 0);
	}*/

	Vector	recoilForce = pOwner->GetAbsVelocity() - pOwner->GetAutoaimVector(0) * (1.0f);
	recoilForce[2] += 10.0f;
	pOwner->SetAbsVelocity(recoilForce);

	QAngle punch;
	punch.Init(SharedRandomFloat("gaussX", -5, 5), SharedRandomFloat("gaussY", -5, 5), 0);

}

//-----------------------------------------------------------------------------
// Purpose: Primary Attack
//-----------------------------------------------------------------------------
void CWeaponTau::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( !pOwner ){ return; }

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	//WeaponSound( SINGLE );
	//WeaponSound( SPECIAL2 );
	//EmitSound("Weapon_gauss.Common");
	
	pOwner->DoMuzzleFlash();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

	RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

	Fire();

	/*DispatchParticleEffect("gauss_normal", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
	DispatchParticleEffect("gauss_sparks01", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
	DispatchParticleEffect("muzzle_sparks01", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
	DispatchParticleEffect("gauss_zap01", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged*/

	/*CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	pPlayer->AddEffects(EF_M);*/

	if (!m_bIsFiring)
		m_bIsFiring = true;

	BaseClass::PrimaryAttack();
	//PrepareHitmarker();

	if (pOwner->GetActiveWeapon())
	{
		if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
		{
			SendWeaponAnim(GetWpnData().animData[m_bFireMode].FireIronsighted1);
		}
		else
		{
			if (oc_weapon_tau_animation.GetInt() == 0)
			{
				SendWeaponAnim(GetSecondaryAttackActivity());
			}
			if (oc_weapon_tau_animation.GetInt() == 1)
			{
				SendWeaponAnim(GetWpnData().animData[m_bFireMode].FirePrimary1);
			}
			if (oc_weapon_tau_animation.GetInt() == 2)
			{
				int i = RandomInt(1, 2);
				if (i == 1)
				{
					SendWeaponAnim(GetWpnData().animData[m_bFireMode].FirePrimary1);
				}
				if (i == 2)
				{
					SendWeaponAnim(GetSecondaryAttackActivity());
				}
			}
		}
	}
	pOwner->SetAnimation(PLAYER_ATTACK1);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTau::IncreaseCharge( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( m_flNextChargeTime > gpGlobals->curtime || !pOwner ){ return; }

	//m_flChargeTime2 = 0;
//	if (!delay2)
//	DispatchParticleEffect("gauss_charge", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged


	if ((gpGlobals->curtime - m_flChargeStartTime) < (0.1f))
	{
		if (m_bChargeIndicated == false)
		{
			DispatchParticleEffect(STRING(GetWpnData().iMuzzleFlashDelayed), PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), GetWpnData().iAttachment, false);
			//DispatchParticleEffect("gauss_charge", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
		}
	}

	/*if (!m_bChargeIndicated)
		m_flAmmoRemoveDelay = TAU_CHARGE_TIME;
	else
		m_flAmmoRemoveDelay = -1;*/

	if ((gpGlobals->curtime - m_flChargeStartTime) > MAX_TAU_CHARGE_TIME)
	{
		if (m_bChargeIndicated == false)
		{
			//WeaponSound(SPECIAL2);
			m_bChargeIndicated = true;
		}

		if ((gpGlobals->curtime - m_flChargeStartTime) > ((DANGER_TAU_CHARGE_TIME)*(0.6f)))
		{
			//WeaponSound(SPECIAL3);
			EmitSound("Weapon_Gauss.Special3");
			//DispatchParticleEffect("gauss_tik", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
	    }

		if ( ( gpGlobals->curtime - m_flChargeStartTime ) > DANGER_TAU_CHARGE_TIME )
		{
			// Damage the player (Overcharge)
			//WeaponSound( SPECIAL2 );
			pOwner->TakeDamage(CTakeDamageInfo(this, this, 25, DMG_GAUSS | DMG_CRUSH));
			//DispatchParticleEffect("gauss_zap02", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged

			//DispatchParticleEffect("gauss_tik", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), 1, false);//OverCharged
			color32 gaussDamage = {255,128,0,128};
			UTIL_ScreenFade( pOwner, gaussDamage, 0.2f, 0.2f, FFADE_IN );
			m_flNextChargeTime = gpGlobals->curtime + random->RandomFloat( 0.5f, 2.5f );
		}


		return;
	}

	RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

	if (minZap > 0.15f)
		minZap -= 0.05f;
	if (minZap > 0.6f)
		maxZap -= 0.4f;

	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		ChargedFire();
		return;
	}

	m_flNextChargeTime = gpGlobals->curtime + TAU_CHARGE_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: Secondary Attack
//-----------------------------------------------------------------------------
void CWeaponTau::SecondaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( !pOwner || pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ){ return; }

	if ( pOwner->GetWaterLevel() == 3 )
	{
		EmitSound( "Weapon_Gauss.Zap1" );
		SendWeaponAnim( ACT_VM_IDLE );
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		return;
	}

	if (oc_weapon_tau_shake.GetBool())
	{
		QAngle angles = pOwner->GetLocalAngles();
		pOwner->ViewPunchReset();
		angles.x += random->RandomFloat(-0.1, 0.1);
		angles.y += random->RandomFloat(-0.1, 0.1);
		pOwner->SnapEyeAngles(angles);
	}

	if ( !m_bCharging )
	{
		m_bWeaponBlockWall = true;
		//SendWeaponAnim(ACT_VM_PULLBACK);

		//if (m_flNextSpinTime < gpGlobals->curtime)
		{
			SendWeaponAnim(ACT_GAUSS_SPINUP);

			m_flNextSpinTime = gpGlobals->curtime + GetViewModelSequenceDuration() - 0.15f;
		}

		m_flNextPrimaryAttack = m_flNextSpinTime + 0.2f;
		
		if ( !m_sndCharge )
		{
			CPASAttenuationFilter filter( this );
			m_sndCharge	= (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "Weapon_Gauss.ChargeLoop", ATTN_NORM );
		}

		if ( m_sndCharge != NULL )
		{
			(CSoundEnvelopeController::GetController()).Play( m_sndCharge, 1.0f, 50 );
			(CSoundEnvelopeController::GetController()).SoundChangePitch( m_sndCharge, 250, 3.0f );
		}

		m_flChargeStartTime = gpGlobals->curtime;
		m_bCharging = true;
		isCharging = true;
		//m_bInReload = true;
		m_bChargeIndicated = false;
		minZap = 0.7f;
		maxZap = 1.5f;
		m_flNextZapTime = gpGlobals->curtime + random->RandomFloat(minZap, maxZap);

		RemoveAmmo(GetPrimaryAmmoType(), 1); //pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	if (!m_bIsFiring)
		m_bIsFiring = true;

//	QAngle punch;
//	punch.Init(SharedRandomFloat("gaussX", -10, 10), SharedRandomFloat("gaussY", -10, 10), 0);

	int pitch = (gpGlobals->curtime - m_flChargeStartTime) * (150 / GetFullChargeTime()) + 100;
	if (pitch > 250){ pitch = 250; }
	if (m_sndCharge != NULL)
	{
		(CSoundEnvelopeController::GetController()).SoundChangePitch(m_sndCharge, pitch, 0);
	}

	IncreaseCharge();

	//if (gpGlobals->curtime > m_flNextZapTime)
		//RandomZap();
}

void CWeaponTau::RandomZap(WeaponSound_t soundType)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	/*if (pOwner->GetViewModel())//This is only for HL1 variant
	{
		int attachment = pOwner->GetViewModel()->LookupAttachment("spinner");
		DispatchParticleEffect("gauss_zap", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), attachment, false);

		CEffectData data;

		data.m_nEntIndex = pOwner->GetViewModel()->entindex();

		data.m_nAttachmentIndex = attachment;		

		data.m_vStart.x = GetWpnData().iMuzzleFlashLightR;
		data.m_vStart.y = GetWpnData().iMuzzleFlashLightG;
		data.m_vStart.z = GetWpnData().iMuzzleFlashLightB;

		DispatchEffect("GaussWeaponLight", data);
	}*/

	//EmitSound("Weapon_MegaPhysCannon.ChargeZap");
	WeaponSound(soundType);

	m_flNextZapTime = gpGlobals->curtime + random->RandomFloat(minZap, maxZap);
}
//-----------------------------------------------------------------------------
// Purpose: Item Post Frame
//-----------------------------------------------------------------------------
void CWeaponTau::ItemPostFrame(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (isCharging && m_flNextSpinTime < gpGlobals->curtime)
	{
		//SendWeaponAnim(ACT_VM_PULLBACK);
		SendWeaponAnim(ACT_GAUSS_SPINCYCLE);

		m_flNextSpinTime = gpGlobals->curtime + GetViewModelSequenceDuration() - 0.15f;
		m_flNextPrimaryAttack = m_flNextSpinTime + 0.2f;
	}

	if (m_bCharging)
	{
		if (&CSoundEnvelopeController::GetController() != NULL)
		{
			if (pPlayer->GetSlowMoIsEnabled())
			{
				CSoundEnvelopeController::GetController().SoundChangePitch(m_sndCharge, 0.3f, -0.01f);
			}
			else
			{
				CSoundEnvelopeController::GetController().SoundChangePitch(m_sndCharge, 1.0f, -0.01f);
			}
		}
	}

	if (pPlayer->m_afButtonReleased & IN_ATTACK2)
	{
		if (m_bCharging){ ChargedFire(); }
	}
	

	if (blockSoundThink && gpGlobals->curtime > m_flNextSoundTime)
	{
		int rndm = random->RandomInt(0, 7);

		if (rndm > 0)
			RandomZap(SPECIAL2);

		blockSoundThink = false;
	}

	if (m_bIsFiring && gpGlobals->curtime > m_flTimeWeaponIdle)
	{
		/*m_bWeaponBlockWall =*/ m_bIsFiring = false;
		m_bWeaponBlockWall = false;
	}
	//else
		/*m_bWeaponBlockWall = true;*/

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Stop Charge Sound
//-----------------------------------------------------------------------------
void CWeaponTau::StopChargeSound( void )
{
	if ( m_sndCharge != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndCharge, 0.1f );

		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		if (pOwner->GetViewModel())
			StopParticleEffects(pOwner->GetViewModel());

		StopSound("Weapon_Gauss.Special3");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Holster
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponTau::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bWeaponBlockWall = false;
	StopChargeSound();
	m_bCharging = false;
	m_bIsFiring = false;
	isCharging = false;
	m_bInReload = false;
	m_bChargeIndicated = false;
	blockSoundThink = false;
	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponTau::Drop( const Vector &vecVelocity )
{
	m_bWeaponBlockWall = false;
	StopChargeSound();
	m_bCharging = false;
	isCharging = false;
	m_bIsFiring = false;
	m_bInReload = false;
	m_bChargeIndicated = false;
	blockSoundThink = false;
	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------
// Purpose: Full charge time
//-----------------------------------------------
float CWeaponTau::GetFullChargeTime( void )
{
	if ( g_pGameRules->IsMultiplayer() )
	{
		return 1.5;
	}
	else
	{
		return 2.5;
	}
}
//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponTau::ShouldDrawWaterImpacts( const trace_t &shot_trace )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if ( WaterContents( shot_trace.startpos ) )
		return false;

	// We must end inside of water
	if ( !WaterContents( shot_trace.endpos ) )
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( shot_trace.startpos, shot_trace.endpos, (CONTENTS_WATER|CONTENTS_SLIME), UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &waterTrace );


	if ( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(2.0,4.0f);	// Water effect scale

		// See if we hit slime
		if ( FBitSet( waterTrace.contents, CONTENTS_SLIME ) )
		{
			FSetBit( data.m_fFlags, FX_WATER_IN_SLIME );
		}
		
		CPASFilter filter( data.m_vOrigin ); 
		te->DispatchEffect( filter, 0.0, data.m_vOrigin, "watersplash", data );	
	}
	return true;
}

/*void CWeaponTau::MakeTracer(const Vector &startPos, const trace_t &tr, int iTracerType)
{

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	if (pPlayer)
	{
		if (pPlayer != NULL)
		{
			pPlayer->AddEffects(EF_M);
		}
	}

	vThrowPos = pPlayer->EyePosition();

	vThrowPos += vForward * 30.0f;
	vThrowPos += vRight * 6.9f;
	vThrowPos += vUp * -0.3f;


	//	UTIL_ParticleTracer("weapon_tracer_ar2", vThrowPos, tr.endpos, 1, TRACER_DONT_USE_ATTACHMENT, true ); 
	UTIL_Tracer(vThrowPos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 7000, false, "RevTracer");
	UTIL_Tracer(vThrowPos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 5000, true, "TrailTracer");

	DevMsg("1: %.2f \n", 1);
}*/