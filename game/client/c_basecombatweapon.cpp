//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side implementation of CBaseCombatWeapon.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "history_resource.h"
#include "iclientmode.h"
#include "iinput.h"
#include "weapon_selection.h"
#include "hud_crosshair.h"
#include "engine/ivmodelinfo.h"
#include "tier0/vprof.h"
#include "hltvcamera.h"
#include "tier1/KeyValues.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "iviewrender_beams.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar oc_muzzleflash_attached("oc_muzzleflash_attached", "0", FCVAR_ARCHIVE);

//-----------------------------------------------------------------------------
// Purpose: Gets the local client's active weapon, if any.
//-----------------------------------------------------------------------------
C_BaseCombatWeapon *GetActiveWeapon( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if ( !player )
		return NULL;

	return player->GetActiveWeapon();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::SetDormant( bool bDormant )
{
	// If I'm going from active to dormant and I'm carried by another player, holster me.
	if ( !IsDormant() && bDormant && GetOwner() && !IsCarriedByLocalPlayer() )
	{
		Holster( NULL );
	}

	BaseClass::SetDormant( bDormant );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit(state);

	if (state == SHOULDTRANSMIT_END)
	{
		if (m_iState == WEAPON_IS_ACTIVE)
		{
			m_iState = WEAPON_IS_CARRIED_BY_PLAYER;
		}
	}
	else if( state == SHOULDTRANSMIT_START )
	{
		if( m_iState == WEAPON_IS_CARRIED_BY_PLAYER )
		{
			if( GetOwner() && GetOwner()->GetActiveWeapon() == this )
			{
				// Restore the Activeness of the weapon if we client-twiddled it off in the first case above.
				m_iState = WEAPON_IS_ACTIVE;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: To wrap PORTAL mod specific functionality into one place
//-----------------------------------------------------------------------------
static inline bool ShouldDrawLocalPlayerViewModel( void )
{
#if defined( PORTAL )
	return false;
#else
	return !C_BasePlayer::ShouldDrawLocalPlayer();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::OnRestore()
{
	BaseClass::OnRestore();

	// if the player is holding this weapon, 
	// mark it as just restored so it won't show as a new pickup
	if (GetOwner() == C_BasePlayer::GetLocalPlayer())
	{
		m_bJustRestored = true;
	}
}

int C_BaseCombatWeapon::GetWorldModelIndex( void )
{
	if ( GameRules() )
	{
		const char *pBaseName = modelinfo->GetModelName( modelinfo->GetModel( m_iWorldModelIndex ) );
		const char *pTranslatedName = GameRules()->TranslateEffectForVisionFilter( "weapons", pBaseName );

		if ( pTranslatedName != pBaseName )
		{
			return modelinfo->GetModelIndex( pTranslatedName );
		}
	}

	return m_iWorldModelIndex;
}

//extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged(updateType);

	CHandle< C_BaseCombatWeapon > handle = this;

	// If it's being carried by the *local* player, on the first update,
	// find the registered weapon for this ID

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatCharacter *pOwner = GetOwner();

	// check if weapon is carried by local player
	bool bIsLocalPlayer = pPlayer && pPlayer == pOwner;
	if ( bIsLocalPlayer && ShouldDrawLocalPlayerViewModel() )		// TODO: figure out the purpose of the ShouldDrawLocalPlayer() test.
	{
		// If I was just picked up, or created & immediately carried, add myself to this client's list of weapons
		if ( (m_iState != WEAPON_NOT_CARRIED ) && (m_iOldState == WEAPON_NOT_CARRIED) )
		{
			// Tell the HUD this weapon's been picked up
			if ( ShouldDrawPickup() )
			{
				CBaseHudWeaponSelection *pHudSelection = GetHudWeaponSelection();
				if ( pHudSelection )
				{
					pHudSelection->OnWeaponPickup( this );
				}

				pPlayer->EmitSound( "Player.PickupWeapon" );
			}
		}
	}
	else // weapon carried by other player or not at all
	{
		int overrideModelIndex = CalcOverrideModelIndex();
		if( overrideModelIndex != -1 && overrideModelIndex != GetModelIndex() )
		{
			SetModelIndex( overrideModelIndex );
		}
	}

	UpdateVisibility();

	m_iOldState = m_iState;

	m_bJustRestored = false;

	/*if (m_bEnableMainLaser)
	{
		int iLaserAttachment = 1;
		if (pPlayer != NULL && pPlayer->GetActiveWeapon() != NULL && pPlayer->GetViewModel() != NULL)
		{
			iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
		}
		else
		{
			iLaserAttachment = -1;
		}

		BeamInfo_t beamInfo;
		trace_t tr;
		Vector vecOrigin2, vecOriginEnd2;
		QAngle angAngles2;
		QAngle	angAngles;
		C_BaseViewModel *pBeamEnt = pPlayer->GetViewModel();
		iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
		pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
		::FormatViewModelAttachment(vecOrigin2, false);
		Vector	forward2, right2, Up2;
		if (cvar->FindVar("oc_test_adjust_weapon_laserbeam")->GetBool())
		{
			angAngles.x += cvar->FindVar("oc_test_adjust_weapon_laserbeam_ang_x")->GetFloat();
				angAngles.y += cvar->FindVar("oc_test_adjust_weapon_laserbeam_ang_y")->GetFloat();
			angAngles.z += cvar->FindVar("oc_test_adjust_weapon_laserbeam_ang_z")->GetFloat();
		}
		else
		{
			angAngles.x += pPlayer->GetActiveWeapon()->GetWpnData().beamLaserAng.x;
			angAngles.y += pPlayer->GetActiveWeapon()->GetWpnData().beamLaserAng.y;
			angAngles.z += pPlayer->GetActiveWeapon()->GetWpnData().beamLaserAng.z;
		}
		AngleVectors(angAngles, &forward2, &right2, &Up2);
		Vector vecOrigin, vecForward, vecRight, vecUp;
		pPlayer->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);
		Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

		beamInfo.m_vecEnd = vecOrigin2;
		beamInfo.m_pEndEnt = pBeamEnt;
		beamInfo.m_nEndAttachment = iLaserAttachment;
		vecOriginEnd2 = vecForward + vecRight + vecUp;
		if (cvar->FindVar("oc_test_adjust_weapon_laserbeam")->GetBool())
		{
			vecOriginEnd2 += forward2 * cvar->FindVar("oc_test_adjust_weapon_laserbeam_pos_x")->GetFloat();
			vecOriginEnd2 += right2 * cvar->FindVar("oc_test_adjust_weapon_laserbeam_pos_y")->GetFloat();
				vecOriginEnd2 += Up2 * cvar->FindVar("oc_test_adjust_weapon_laserbeam_pos_z")->GetFloat();
		}
		else
		{
			vecOriginEnd2 += forward2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.x;
			vecOriginEnd2 += right2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.y;
			vecOriginEnd2 += Up2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.z;
		}
		UTIL_TraceLine(vecOrigin, vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)

		beamInfo.m_vecStart = tr.endpos;


		//beamInfo.m_nStartAttachment = iLaserAttachment;
		//beamInfo.m_pStartEnt = pBeamEnt;


		beamInfo.m_pStartEnt = NULL;
		beamInfo.m_nStartAttachment = -1;

		if (V_strcmp(pPlayer->GetActiveWeapon()->GetWpnData().beamMaterial, "None") != 0)
			beamInfo.m_pszModelName = pPlayer->GetActiveWeapon()->GetWpnData().beamMaterial;//"effects/bluelaser1.vmt";
		if (!(tr.surface.flags & SURF_SKY))
		{
			if (V_strcmp(pPlayer->GetActiveWeapon()->GetWpnData().beamHaloMaterial, "None") != 0)
				beamInfo.m_pszHaloName = pPlayer->GetActiveWeapon()->GetWpnData().beamHaloMaterial;

			beamInfo.m_flHaloScale = pPlayer->GetActiveWeapon()->GetWpnData().beamHaloWidth;
		}
		beamInfo.m_flLife = 0.001f;
		if (cvar->FindVar("oc_state_InSecondFire")->GetInt())
			beamInfo.m_flWidth = pPlayer->GetActiveWeapon()->m_bZoomLevel * 0.01f; //random->RandomFloat( 1.0f, 2.0f );
		else
			beamInfo.m_flWidth = pPlayer->GetActiveWeapon()->GetWpnData().beamWidth;

		if (pPlayer->GetActiveWeapon()->GetWpnData().beamLength != -1.f && pPlayer->GetActiveWeapon()->GetWpnData().beamLength > 0)
			beamInfo.m_flFadeLength = pPlayer->GetActiveWeapon()->GetWpnData().beamLength;

		beamInfo.m_flEndWidth = pPlayer->GetActiveWeapon()->GetWpnData().beamEndWidth;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 0.0f; //random->RandomFloat( 16, 32 );
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 0.1f;
		beamInfo.m_flRed = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.r;
		beamInfo.m_flGreen = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.g;
		beamInfo.m_flBlue = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.b;
		beamInfo.m_nSegments = 0;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = 0;
		beams->CreateBeamEntPoint(beamInfo);
	}*/

	/*if (this && m_iMuzzleFlashState == 1)
	{
		if (!m_M)
			SetMuzzleFlashActive(pPlayer, this);
		else
			SetMuzzleFlashDeactive(this);
	}
	else if (this && m_iMuzzleFlashState == 2)
	{
		if (m_M || m_Ar2_MM)
			SetMuzzleFlashDeactive(this);
	}
	if (this)
		UpdateMuzzleFlash(pPlayer, this);*/


	if (GetOwner() && GetOwner()->IsNPC())
		return;

	if (GetWpnData().AllowProjectedMuzzleFlashes)
	{
		if (IsMuzzleFlashActive)
			UpdateMuzzleFlash(pPlayer, this);
	}
	else
	{
		if (IsMuzzleFlashActive || m_M || m_Ar2_MM)
		{
			SetMuzzleFlashDeactive(this);
		}
	}
}

#include "c_te_effect_dispatch.h"
void SetMuzzleFlashActiveCallBack(const CEffectData &data)
{
	C_BaseCombatWeapon *pWeap = dynamic_cast<C_BaseCombatWeapon *>(ClientEntityList().GetBaseEntity(data.entindex()));

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer->GetActiveWeapon() && pWeap && (char*)pPlayer->GetActiveWeapon() != (char*)pWeap)
		return;

	if (pWeap && pPlayer)
		pWeap->SetMuzzleFlashActive(pPlayer, pWeap);
}
DECLARE_CLIENT_EFFECT("SetMuzzleFlashActive", SetMuzzleFlashActiveCallBack);

void SetMuzzleFlashDeActiveCallBack(const CEffectData &data)
{
	C_BaseCombatWeapon *pWeap = dynamic_cast<C_BaseCombatWeapon *>(ClientEntityList().GetBaseEntity(data.entindex()));

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer->GetActiveWeapon() && pWeap && (char*)pPlayer->GetActiveWeapon() != (char*)pWeap)
		return;

	if (pWeap && pPlayer)
		pWeap->SetMuzzleFlashActive(pPlayer, pWeap);
}
DECLARE_CLIENT_EFFECT("SetMuzzleFlashDeActive", SetMuzzleFlashDeActiveCallBack);

/*void C_BaseCombatWeapon::SetMuzzleFlashActiveInternalCallBack(C_BasePlayer *pPlayer, C_BaseCombatWeapon *pWeap)
{
	if (pPlayer->GetActiveWeapon() && pWeap && (char*)pPlayer->GetActiveWeapon() != (char*)pWeap)
		return;

	if (pWeap && pPlayer)
		pWeap->SetMuzzleFlashActive(pPlayer, pWeap);
}
void C_BaseCombatWeapon::SetMuzzleFlashDeactiveInternalCallBack(C_BasePlayer *pPlayer, C_BaseCombatWeapon *pWeap)
{
	if (pPlayer->GetActiveWeapon() && pWeap && (char*)pPlayer->GetActiveWeapon() != (char*)pWeap)
		return;

	if (pWeap && pPlayer)
		pWeap->SetMuzzleFlashActive(pPlayer, pWeap);
}*/

void C_BaseCombatWeapon::SetMuzzleFlashActive(C_BasePlayer *pPlayer, C_BaseCombatWeapon *pCurWeapon)
{
	if (GetOwner())
	{
		if (GetOwner()->IsNPC())
			return;
	}

	if (!GetActiveWeapon())
	{
		if (pCurWeapon)
			SetMuzzleFlashDeactive(pCurWeapon);

		return;
	}


	pCurWeapon->m_iMuzzleFlashState = 1;

	if (!pCurWeapon->GetWpnData().AllowProjectedMuzzleFlashes)
	{
		SetMuzzleFlashDeactive(pCurWeapon);
		pCurWeapon->ReinitializeMuzzleFlash = false;
		return;
	}

	if (!pCurWeapon->m_M)
	{
		pCurWeapon->m_M = new C_MuzzleflashEffect(pPlayer->index);
		pCurWeapon->m_M->TurnOnM();
		pCurWeapon->IsMuzzleFlashActive = false;
		pCurWeapon->IsMuzzleFlashComplete = false;
	}

	if (pCurWeapon->FlashType() == 1)
	{
		if (!pCurWeapon->m_Ar2_MM)
		{
			pCurWeapon->m_Ar2_MM = new C_Ar2_Middle_MuzzleflashEffect(pPlayer->index);
			pCurWeapon->m_Ar2_MM->Ar2_Middle_TurnOnM();
			pCurWeapon->IsMuzzleFlashActive = false;
			pCurWeapon->IsMuzzleFlashComplete = false;
		}
	}

	pCurWeapon->ReinitializeMuzzleFlash = false;
}

void C_BaseCombatWeapon::SetMuzzleFlashDeactive(C_BaseCombatWeapon *pCurWeapon)
{
	if (GetOwner())
	{
		if (GetOwner()->IsNPC())
			return;
	}

	pCurWeapon->m_iMuzzleFlashState = 0;

	if (pCurWeapon->m_M)
	{
		delete pCurWeapon->m_M;
		pCurWeapon->m_M = NULL;
	}
	if (pCurWeapon->m_Ar2_MM)
	{
		delete pCurWeapon->m_Ar2_MM;
		pCurWeapon->m_Ar2_MM = NULL;
	}

	pCurWeapon->IsMuzzleFlashActive = false;
	pCurWeapon->IsMuzzleFlashComplete = false;
	pCurWeapon->ReinitializeMuzzleFlash = true;
}

#define FLASHLIGHT_DISTANCE		1000
extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);
void C_BaseCombatWeapon::UpdateMuzzleFlash(C_BasePlayer *pPlayer, C_BaseCombatWeapon *pCurWeapon)
{
	if (GetOwner())
	{
		if (GetOwner()->IsNPC())
			return;
	}

	float lkoef = 0.f;

	m_flMuzzleFlashRoll = /*gpGlobals->curtime -*/ RandomFloat(-360.0f, 360.0f);

	if (IsMuzzleFlashActive && m_M)
	{
		if (m_flNextMuzzleFlashTime < gpGlobals->curtime && !IsMuzzleFlashComplete)
		{
			m_flNextMuzzleFlashTime = gpGlobals->curtime + m_M->MaxTime;

			if (m_flNextMuzzleFlashTime > pCurWeapon->m_flNextPrimaryAttack)
				m_flNextMuzzleFlashTime = pCurWeapon->m_flNextPrimaryAttack;

			if (pCurWeapon->m_flNextPrimaryAttack - gpGlobals->curtime < 0.005)
				m_flNextMuzzleFlashTime = gpGlobals->curtime + m_M->MaxTime;

			IsMuzzleFlashComplete = true;
		}

		lkoef = m_flNextMuzzleFlashTime - gpGlobals->curtime;

		if (m_flNextMuzzleFlashTime < gpGlobals->curtime)
		{
			lkoef = 0.f;
			m_flNextMuzzleFlashTime = gpGlobals->curtime;
			IsMuzzleFlashActive = false;
			IsMuzzleFlashComplete = false;
		}
	}

	if (m_M)
	{
		if (true)//(oc_muzzleflash_attached.GetBool())
		{
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			pPlayer->EyeVectors(&vecForward, &vecRight, &vecUp);

			Vector vecForwardStatic, vecRightStatic, vecUpStatic;
			pPlayer->EyeVectors(&vecForwardStatic, &vecRightStatic, &vecUpStatic);

			int iAttachment = pCurWeapon->LookupAttachment(pCurWeapon->GetWpnData().iFlashFirstAttachment);

			if (FClassnameIs(pCurWeapon, "weapon_dual_pistols"))
			{
				if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 0)
				{
					iAttachment = pCurWeapon->LookupAttachment(pCurWeapon->GetWpnData().iFlashSecondAttachment);
				}
				if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 1)
				{
					iAttachment = pCurWeapon->LookupAttachment(pCurWeapon->GetWpnData().iFlashFirstAttachment);
				}
			}

			pPlayer->GetViewModel()->GetAttachment(iAttachment, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			AngleVectors(angLightDir, &vecForwardStatic, &vecRightStatic, &vecUpStatic);

			QAngle angLightDirStatic = angLightDir;
			VectorAngles(vecForward, vecUp, angLightDir);
			angLightDir.z = m_flMuzzleFlashRoll;
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
		
			angLightDirStatic.z = 0.f;
			AngleVectors(angLightDirStatic, &vecForwardStatic, &vecRightStatic, &vecUpStatic);

			if (pCurWeapon->GetWpnData().iFlashRollPrimary)
				m_M->UpdateLightM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
			else
				m_M->UpdateLightM(vecLightOrigin, vecForwardStatic, vecRightStatic, vecUpStatic, FLASHLIGHT_DISTANCE, lkoef);

			if (pCurWeapon->FlashType() == 1 && m_Ar2_MM)
			{
				if (pCurWeapon->GetWpnData().iFlashRollSecondary)
					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
				else
					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecLightOrigin, vecForwardStatic, vecRightStatic, vecUpStatic, FLASHLIGHT_DISTANCE, lkoef);
			}
		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos, Muzzle;
			vecPos = EyePosition();
			pPlayer->EyeVectors(&vecForward, &vecRight, &vecUp);

			/*float Flash_adjust_right = pCurWeapon->GetWpnData().ParticleTracerPosition.x;
			float Flash_adjust_up = pCurWeapon->GetWpnData().ParticleTracerPosition.y;
			float Flash_adjust_forward = pCurWeapon->GetWpnData().ParticleTracerPosition.z;*/

			float Flash_adjust_right = pCurWeapon->GetWpnData().iFlashXPos;//oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = pCurWeapon->GetWpnData().iFlashYPos;//oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = pCurWeapon->GetWpnData().iFlashZPos;//oc_muzzleflash_adjust_forward.GetFloat();

			/*if (pCurWeapon->GetWpnData().iFlashXPos > 0)
				Flash_adjust_forward += pCurWeapon->GetWpnData().iFlashXPos;
			if (pCurWeapon->GetWpnData().iFlashYPos > 0)
				Flash_adjust_forward += pCurWeapon->GetWpnData().iFlashYPos;
			if (pCurWeapon->GetWpnData().iFlashZPos > 0)
				Flash_adjust_forward += pCurWeapon->GetWpnData().iFlashZPos;*/

			QAngle angAngles2;
			C_BaseViewModel *pBeamEnt = pPlayer->GetViewModel();
			int iAttachment = pCurWeapon->GetWpnData().iAttachment;
			pBeamEnt->GetAttachment(iAttachment, Muzzle, angAngles2);
			::FormatViewModelAttachment(Muzzle, false);


			C_BaseViewModel  *pVm = pPlayer->GetViewModel();
			if (pVm->m_bExpSighted)
			{
				vecPos = EyePosition();
			}
			else
			{
				vecPos = Muzzle;
				vecPos += vecForward * Flash_adjust_forward;

				if (FClassnameIs(pCurWeapon, "weapon_dual_pistols"))
				{
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 0)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_firstbarrel_tracer_y")->GetFloat();
					}
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 1)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_secondbarrel_tracer_y")->GetFloat();
					}
				}
				else
					vecPos += vecRight * Flash_adjust_right;

				vecPos += vecUp * Flash_adjust_up;

			}
			if (pCurWeapon->FlashType() == 1 && m_Ar2_MM)
			{
				if (pCurWeapon->GetWpnData().iFlashRollSecondary)
				{
					QAngle ang2;
					VectorAngles(vecForward, vecUp, ang2);
					ang2.z = m_flMuzzleFlashRoll;
					AngleVectors(ang2, &vecForward, &vecRight, &vecUp);

					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
				}
				else
					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
			}

			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = m_flMuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);

			m_M->UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
		}
	}
}
//-----------------------------------------------------------------------------
// Is anyone carrying it?
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::IsBeingCarried() const
{
	return ( m_hOwner.Get() != NULL );
}

//-----------------------------------------------------------------------------
// Is the carrier alive?
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::IsCarrierAlive() const
{
	if ( !m_hOwner.Get() )
		return false;

	return m_hOwner.Get()->GetHealth() > 0;
}

//-----------------------------------------------------------------------------
// Should this object cast shadows?
//-----------------------------------------------------------------------------
ShadowType_t C_BaseCombatWeapon::ShadowCastType()
{
	if ( IsEffectActive( /*EF_NODRAW |*/ EF_NOSHADOW ) )
		return SHADOWS_NONE;

	if (GetOwner() && GetOwner()->IsPlayer())
	{
		if (cvar->FindVar("oc_player_draw_body")->GetInt() && !cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			return SHADOWS_NONE;
	}

	if (!IsBeingCarried())
		return SHADOWS_RENDER_TO_TEXTURE;

	if (IsCarriedByLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer())
		return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE;
}

//-----------------------------------------------------------------------------
// Purpose: This weapon is the active weapon, and it should now draw anything
//			it wants to. This gets called every frame.
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::Redraw()
{
	if ( g_pClientMode->ShouldDrawCrosshair() )
	{
		DrawCrosshair();
	}

	// ammo drawing has been moved into hud_ammo.cpp
}

//-----------------------------------------------------------------------------
// Purpose: Draw the weapon's crosshair
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::DrawCrosshair()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	Color clr = gHUD.m_clrNormal;
/*

	// TEST: if the thing under your crosshair is on a different team, light the crosshair with a different color.
	Vector vShootPos, vShootAngles;
	GetShootPosition( vShootPos, vShootAngles );

	Vector vForward;
	AngleVectors( vShootAngles, &vForward );
	
	
	// Change the color depending on if we're looking at a friend or an enemy.
	CPartitionFilterListMask filter( PARTITION_ALL_CLIENT_EDICTS );	
	trace_t tr;
	traceline->TraceLine( vShootPos, vShootPos + vForward * 10000, COLLISION_GROUP_NONE, MASK_SHOT, &tr, true, ~0, &filter );

	if ( tr.index != 0 && tr.index != INVALID_CLIENTENTITY_HANDLE )
	{
		C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( tr.index );
		if ( pEnt )
		{
			if ( pEnt->GetTeamNumber() != player->GetTeamNumber() )
			{
				g = b = 0;
			}
		}
	}		 
*/

	CHudCrosshair *crosshair = GET_HUDELEMENT( CHudCrosshair );
	if ( !crosshair )
		return;

	// Find out if this weapon's auto-aimed onto a target
	bool bOnTarget = ( m_iState == WEAPON_IS_ONTARGET );
	
	if ( player->GetFOV() >= 90 )
	{ 
		// normal crosshairs
		if ( bOnTarget && GetWpnData().iconAutoaim )
		{
			clr[3] = 255;

			crosshair->SetCrosshair( GetWpnData().iconAutoaim, clr );
		}
		else if ( GetWpnData().iconCrosshair )
		{
			clr[3] = 255;
			crosshair->SetCrosshair( GetWpnData().iconCrosshair, clr );
		}
		else
		{
			crosshair->ResetCrosshair();
		}
	}
	else
	{ 
		Color white( 255, 255, 255, 255 );

		// zoomed crosshairs
		if (bOnTarget && GetWpnData().iconZoomedAutoaim)
			crosshair->SetCrosshair(GetWpnData().iconZoomedAutoaim, white);
		else if ( GetWpnData().iconZoomedCrosshair )
			crosshair->SetCrosshair( GetWpnData().iconZoomedCrosshair, white );
		else
			crosshair->ResetCrosshair();
	}
}

//-----------------------------------------------------------------------------
// Purpose: This weapon is the active weapon, and the viewmodel for it was just drawn.
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::ViewModelDrawn( C_BaseViewModel *pViewModel )
{
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this client's carrying this weapon
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::IsCarriedByLocalPlayer( void )
{
	if ( !GetOwner() )
		return false;

	return ( GetOwner() == C_BasePlayer::GetLocalPlayer() );
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if this client is carrying this weapon and is
//			using the view models
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::ShouldDrawUsingViewModel( void )
{
	return IsCarriedByLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer();
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if this weapon is the local client's currently wielded weapon
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::IsActiveByLocalPlayer( void )
{
	if ( IsCarriedByLocalPlayer() )
	{
		return (m_iState == WEAPON_IS_ACTIVE);
	}

	return false;
}

bool C_BaseCombatWeapon::GetShootPosition( Vector &vOrigin, QAngle &vAngles )
{
	// Get the entity because the weapon doesn't have the right angles.
	C_BaseCombatCharacter *pEnt = ToBaseCombatCharacter( GetOwner() );
	if ( pEnt )
	{
		if ( pEnt == C_BasePlayer::GetLocalPlayer() )
		{
			vAngles = pEnt->EyeAngles();
		}
		else
		{
			vAngles = pEnt->GetRenderAngles();	
		}
	}
	else
	{
		vAngles.Init();
	}

	QAngle vDummy;
	if ( IsActiveByLocalPlayer() && ShouldDrawLocalPlayerViewModel() )
	{
		C_BasePlayer *player = ToBasePlayer( pEnt );
		C_BaseViewModel *vm = player ? player->GetViewModel( 0 ) : NULL;
		if ( vm )
		{
			int iAttachment = vm->LookupAttachment( "muzzle" );
			if ( vm->GetAttachment( iAttachment, vOrigin, vDummy ) )
			{
				return true;
			}
		}
	}
	else
	{
		// Thirdperson
		int iAttachment = LookupAttachment( "muzzle" );
		if ( GetAttachment( iAttachment, vOrigin, vDummy ) )
		{
			return true;
		}
	}

	vOrigin = GetRenderOrigin();
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::ShouldDraw( void )
{
	if ( m_iWorldModelIndex == 0 )
		return false;

	// FIXME: All weapons with owners are set to transmit in CBaseCombatWeapon::UpdateTransmitState,
	// even if they have EF_NODRAW set, so we have to check this here. Ideally they would never
	// transmit except for the weapons owned by the local player.
	if ( IsEffectActive( EF_NODRAW ) )
		return false;

	C_BaseCombatCharacter *pOwner = GetOwner();

	// weapon has no owner, always draw it
	if ( !pOwner )
		return true;

	if (pOwner->IsPlayer())
	{
		if (cvar->FindVar("oc_player_draw_body")->GetInt() && !cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			return false;
	}

	bool bIsActive = ( m_iState == WEAPON_IS_ACTIVE );

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	 // carried by local player?
	if ( pOwner == pLocalPlayer )
	{
		// Only ever show the active weapon
		if ( !bIsActive )
			return false;

		if ( !pOwner->ShouldDraw() )
		{
			// Our owner is invisible.
			// This also tests whether the player is zoomed in, in which case you don't want to draw the weapon.
			return false;
		}

		// 3rd person mode?
		if ( !ShouldDrawLocalPlayerViewModel() )
			return true;

		// don't draw active weapon if not in some kind of 3rd person mode, the viewmodel will do that
		return false;
	}

	// If it's a player, then only show active weapons
	if ( pOwner->IsPlayer() )
	{
		// Show it if it's active...
		return bIsActive;
	}

	// FIXME: We may want to only show active weapons on NPCs
	// These are carried by AIs; always show them
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if a weapon-pickup icon should be displayed when this weapon is received
//-----------------------------------------------------------------------------
bool C_BaseCombatWeapon::ShouldDrawPickup( void )
{
	if ( GetWeaponFlags() & ITEM_FLAG_NOITEMPICKUP )
		return false;

	if ( m_bJustRestored )
		return false;

	return true;
}
		   
//-----------------------------------------------------------------------------
// Purpose: Render the weapon. Draw the Viewmodel if the weapon's being carried
//			by this player, otherwise draw the worldmodel.
//-----------------------------------------------------------------------------
int C_BaseCombatWeapon::DrawModel( int flags )
{
	VPROF_BUDGET( "C_BaseCombatWeapon::DrawModel", VPROF_BUDGETGROUP_MODEL_RENDERING );
	if ( !m_bReadyToDraw )
		return 0;

	if ( !IsVisible() )
		return 0;

	if (GetOwner() && GetOwner()->IsPlayer())
	{
		if (cvar->FindVar("oc_player_draw_body")->GetInt() && !cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			return 0;
	}

	// check if local player chases owner of this weapon in first person
	C_BasePlayer *localplayer = C_BasePlayer::GetLocalPlayer();

	if ( localplayer && localplayer->IsObserver() && GetOwner() )
	{
		// don't draw weapon if chasing this guy as spectator
		// we don't check that in ShouldDraw() since this may change
		// without notification 
		
		if ( localplayer->GetObserverMode() == OBS_MODE_IN_EYE &&
			 localplayer->GetObserverTarget() == GetOwner() ) 
			return false;
	}

	return BaseClass::DrawModel( flags );
}


//-----------------------------------------------------------------------------
// Allows the client-side entity to override what the network tells it to use for
// a model. This is used for third person mode, specifically in HL2 where the
// the weapon timings are on the view model and not the world model. That means the
// server needs to use the view model, but the client wants to use the world model.
//-----------------------------------------------------------------------------
int C_BaseCombatWeapon::CalcOverrideModelIndex() 
{ 
	C_BasePlayer *localplayer = C_BasePlayer::GetLocalPlayer();
	if ( localplayer && 
		localplayer == GetOwner() &&
		ShouldDrawLocalPlayerViewModel() )
	{
		return BaseClass::CalcOverrideModelIndex();
	}
	else
	{
		return GetWorldModelIndex();
	}
}


//-----------------------------------------------------------------------------
// tool recording
//-----------------------------------------------------------------------------
void C_BaseCombatWeapon::GetToolRecordingState( KeyValues *msg )
{
	if ( !ToolsEnabled() )
		return;

	int nModelIndex = GetModelIndex();
	int nWorldModelIndex = GetWorldModelIndex();
	if ( nModelIndex != nWorldModelIndex )
	{
		SetModelIndex( nWorldModelIndex );
	}

	BaseClass::GetToolRecordingState( msg );

	if ( m_iState == WEAPON_NOT_CARRIED )
	{
		BaseEntityRecordingState_t *pBaseEntity = (BaseEntityRecordingState_t*)msg->GetPtr( "baseentity" );
		pBaseEntity->m_nOwner = -1;
	}
	else
	{
		msg->SetInt( "worldmodel", 1 );
		if ( m_iState == WEAPON_IS_ACTIVE )
		{
			BaseEntityRecordingState_t *pBaseEntity = (BaseEntityRecordingState_t*)msg->GetPtr( "baseentity" );
			pBaseEntity->m_bVisible = true;
		}
	}

	if ( nModelIndex != nWorldModelIndex )
	{
		SetModelIndex( nModelIndex );
	}
}
