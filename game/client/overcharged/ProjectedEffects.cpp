// Purpose: Client-side CBasePlayer.
//
//			- Manages the player's flashlight effect.
//
//===========================================================================//
#include "cbase.h"
#include "c_baseplayer.h"
#include "view_shared.h"
#include "view.h"
#include "const.h"
#include "input.h"
#include "view.h"
#include "hud_macros.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "iefx.h"
#include "dlight.h"
#include "baseviewmodel_shared.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar oc_camera_bobbing_border("oc_camera_bobbing_border", "0.1", FCVAR_ARCHIVE);
static ConVar oc_camera_bobbing_speed("oc_camera_bobbing_speed", "0.2", FCVAR_ARCHIVE);
static ConVar oc_weapon_free_aim_movemax_lasersight_X("oc_weapon_free_aim_movemax_lasersight_X", "0.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_free_aim_movemax_lasersight_Y("oc_weapon_free_aim_movemax_lasersight_Y", "0.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_free_aim_movemax_lasersight_angle("oc_weapon_free_aim_movemax_lasersight_angle", "0.0", FCVAR_ARCHIVE);

ConVar oc_test_adjust_weapon_laserbeam("oc_test_adjust_weapon_laserbeam", "0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_pos_x("oc_test_adjust_weapon_laserbeam_pos_x", "0.0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_pos_y("oc_test_adjust_weapon_laserbeam_pos_y", "0.0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_pos_z("oc_test_adjust_weapon_laserbeam_pos_z", "0.0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_ang_x("oc_test_adjust_weapon_laserbeam_ang_x", "0.0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_ang_y("oc_test_adjust_weapon_laserbeam_ang_y", "0.0", FCVAR_CHEAT);
ConVar oc_test_adjust_weapon_laserbeam_ang_z("oc_test_adjust_weapon_laserbeam_ang_z", "0.0", FCVAR_CHEAT);

static ConVar oc_weapon_dual_pistols_firstbarrel_tracer_y("oc_weapon_dual_pistols_firstbarrel_tracer_y", "43.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_dual_pistols_secondbarrel_tracer_y("oc_weapon_dual_pistols_secondbarrel_tracer_y", "-43.0", FCVAR_ARCHIVE);

static ConVar oc_weapon_free_aim_movemax_muzzleflash_X("oc_weapon_free_aim_movemax_muzzleflash_X", "0.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_free_aim_movemax_muzzleflash_Y("oc_weapon_free_aim_movemax_muzzleflash_Y", "0.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_free_aim_movemax_muzzleflash_angle("oc_weapon_free_aim_movemax_muzzleflash_angle", "0.0", FCVAR_ARCHIVE);

static ConVar oc_muzzleflash_adjust_right("oc_muzzleflash_adjust_right", "12.0", FCVAR_ARCHIVE);
static ConVar oc_muzzleflash_adjust_up("oc_muzzleflash_adjust_up", "-8.0", FCVAR_ARCHIVE);
static ConVar oc_muzzleflash_adjust_forward("oc_muzzleflash_adjust_forward", "10.0", FCVAR_ARCHIVE);
static ConVar oc_scopelight_attached("oc_scopelight_attached", "1", FCVAR_ARCHIVE);

static ConVar oc_laser_adjust_right("oc_laser_adjust_right", "12.0", FCVAR_ARCHIVE);
static ConVar oc_laser_adjust_up("oc_laser_adjust_up", "-8.0", FCVAR_ARCHIVE);
static ConVar oc_laser_adjust_forward("oc_laser_adjust_forward", "10.0", FCVAR_ARCHIVE);

#define FLASHLIGHT_DISTANCE		1000
#define MAX_BEAM_LENGTH 10000.f

float MuzzleFlashRoll(0.0f);

float TimeTick1 = 0.0f;
bool On = false;

extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);
extern void ScreenToWorld(int mousex, int mousey, float fov, const Vector& vecRenderOrigin, const QAngle& vecRenderAngles, Vector& vecPickingRay);


/*ConVar oc_player_cam_origin_x("oc_player_cam_origin_x", "0");
ConVar oc_player_cam_origin_y("oc_player_cam_origin_y", "0");
ConVar oc_player_cam_origin_z("oc_player_cam_origin_z", "0");*/

void LaserLightCallback(C_BaseCombatWeapon *pWeapon, trace_t &tr, int index)
{
	dlight_t *el = effects->CL_AllocDlight(index);//(LIGHT_INDEX_MUZZLEFLASH);//( index );
	el->origin = tr.endpos;
	el->color.r = pWeapon->GetWpnData().lightBeamColor.r;
	el->color.g = pWeapon->GetWpnData().lightBeamColor.g;
	el->color.b = pWeapon->GetWpnData().lightBeamColor.b;
	el->color.exponent = pWeapon->GetWpnData().lightBeamColor.a;
	el->radius = random->RandomInt(pWeapon->GetWpnData().lightMinRadius, pWeapon->GetWpnData().lightMaxRadius) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / pWeapon->GetWpnData().lightDecay;
	el->die = gpGlobals->curtime + pWeapon->GetWpnData().lightDie;
}

void C_BasePlayer::ClientThink()
{
	MuzzleFlashRoll = /*gpGlobals->curtime -*/ RandomFloat(-360.0f, 360.0f);

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	/*if (pPlayer)
	{	
		int attachmentIndex = LookupAttachment("muzzle");
		tempents->UpdateMuzzlePosition(GetRefEHandle(), attachmentIndex);
	}*/


	/*if (pPlayer != NULL && pPlayer->GetActiveWeapon() != NULL && pPlayer->GetViewModel() != NULL)
	{
		// Setup the center beam point
		iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
	}
	else
	{
		iLaserAttachment = -1;
	}*/

	if (pPlayer)
	{
		if (tempents)
		{
			int attachmentIndex = LookupAttachment("muzzle");
			tempents->UpdateMuzzlePosition(GetRefEHandle(), attachmentIndex);
		}

		/*if (pPlayer->mfEl)
		{
			dlight_t **Llights;
			int lights = effects->CL_GetActiveDLights(Llights);
			DevMsg("lights: %i\n", lights);
		}*/

		if (pPlayer->GetActiveWeapon())
		{
			int iLaserAttachment = 1;

			if (pPlayer->GetViewModel())
			{
				if (pPlayer->GetActiveWeapon()->GetWpnData().AllowMuzzleFlashDLight)
				{
					if (pPlayer->mfEl && pPlayer->mfEl->key == pPlayer->dlightKey)
					{
						Vector origin;
						pPlayer->GetViewModel()->GetBaseAnimating()->GetAttachment(pPlayer->GetActiveWeapon()->m_bSilenced ? "muzzle_sil" : pPlayer->GetActiveWeapon()->GetWpnData().iMuzzleFlashLightAttachment, origin);
						pPlayer->mfEl->origin = origin;
					}
				}

				//if (pPlayer->GetActiveWeapon()->GetWpnData().enableLaser)
				iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
			}
			else
			{
				iLaserAttachment = -1;
			}

			if ((/*IsCarriedByLocalPlayer() && */!IsEffectActive(EF_NODRAW)
				&& gpGlobals->frametime != 0.0f /*&& m_fNextZoom <= gpGlobals->curtime*/)
				&& pPlayer->GetActiveWeapon()->EnableLaser
				&& !pPlayer->GetActiveWeapon()->EnableLaserInterrupt)
			{
				Vector	vecOrigin, vecAngles;
				QAngle	angAngles;

				BeamInfo_t beamInfo;

				trace_t tr;

				bool wallBump = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0 : true;
				bool wallBump2 = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1 : false;

				if (!pPlayer->IsRunning() && !pPlayer->IsReloadingWeapon() && wallBump)
				{
					Vector vecOrigin2, vecOriginEnd2;
					QAngle angAngles2;
					//C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
					C_BaseViewModel *pBeamEnt = pPlayer->GetViewModel();
					iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
					pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
					::FormatViewModelAttachment(vecOrigin2, false);
					Vector	forward2, right2, Up2;
					if (oc_test_adjust_weapon_laserbeam.GetBool())
					{
						angAngles.x += oc_test_adjust_weapon_laserbeam_ang_x.GetFloat();
						angAngles.y += oc_test_adjust_weapon_laserbeam_ang_y.GetFloat();
						angAngles.z += oc_test_adjust_weapon_laserbeam_ang_z.GetFloat();
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
					//AngleVectors(angAngles, &vecForward, &vecRight, &vecUp);
					//pPlayer->GetViewModel()->GetVectors(&vecForward, &vecRight, &vecUp);
					//pPlayer->GetViewModel()->GetBaseAnimating()->GetVectors(&forward2, &right2, &Up2);
					//pPlayer->GetViewModel()->GetBaseAnimating()->GetVectors(&vecForward, &vecRight, &vecUp);

					Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);
					if (::input->CAM_IsFreeAiming())
					{
						beamInfo.m_vecEnd = vecOrigin2;
						beamInfo.m_pEndEnt = pBeamEnt;
						beamInfo.m_nEndAttachment = iLaserAttachment;
						vecOriginEnd2 = vecForward;
						if (oc_test_adjust_weapon_laserbeam.GetBool())
						{
							vecOriginEnd2 += vecForward + forward2 * oc_test_adjust_weapon_laserbeam_pos_x.GetFloat();
							vecOriginEnd2 += vecRight + right2 * oc_test_adjust_weapon_laserbeam_pos_y.GetFloat();
							vecOriginEnd2 += vecUp + Up2 * oc_test_adjust_weapon_laserbeam_pos_z.GetFloat();
						}
						else
						{
							vecOriginEnd2 += vecForward + forward2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.x;
							vecOriginEnd2 += vecRight + right2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.y;
							vecOriginEnd2 += vecUp + Up2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.z;
						}
						UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_BEAM_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
						beamInfo.m_vecStart = tr.endpos;
						beamInfo.m_pStartEnt = NULL;
						beamInfo.m_nStartAttachment = -1;
					}
					else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() || cvar->FindVar("oc_state_InSecondFire")->GetInt())
					{
						beamInfo.m_vecEnd = vecOrigin2;
						beamInfo.m_pEndEnt = pBeamEnt;
						beamInfo.m_nEndAttachment = iLaserAttachment;
						vecOriginEnd2 = vecForward;
						if (oc_test_adjust_weapon_laserbeam.GetBool())
						{
							vecOriginEnd2 += vecForward + forward2 * oc_test_adjust_weapon_laserbeam_pos_x.GetFloat();
							vecOriginEnd2 += vecRight + right2 * oc_test_adjust_weapon_laserbeam_pos_y.GetFloat();
							vecOriginEnd2 += vecUp + Up2 * oc_test_adjust_weapon_laserbeam_pos_z.GetFloat();
						}
						else
						{
							vecOriginEnd2 += vecForward + forward2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.x;
							vecOriginEnd2 += vecRight + right2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.y;
							vecOriginEnd2 += vecUp + Up2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.z;
						}
						UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_BEAM_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)
						beamInfo.m_vecStart = tr.endpos;

						beamInfo.m_pStartEnt = NULL;
						beamInfo.m_nStartAttachment = -1;
					}
					else
					{
						beamInfo.m_vecEnd = vecOrigin2;
						beamInfo.m_pEndEnt = pBeamEnt;
						beamInfo.m_nEndAttachment = iLaserAttachment;
						vecOriginEnd2 = vecForward + vecRight + vecUp;
						if (oc_test_adjust_weapon_laserbeam.GetBool())
						{
							vecOriginEnd2 += forward2 * oc_test_adjust_weapon_laserbeam_pos_x.GetFloat();
							vecOriginEnd2 += right2 * oc_test_adjust_weapon_laserbeam_pos_y.GetFloat();
							vecOriginEnd2 += Up2 * oc_test_adjust_weapon_laserbeam_pos_z.GetFloat();
						}
						else
						{
							vecOriginEnd2 += forward2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.x;
							vecOriginEnd2 += right2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.y;
							vecOriginEnd2 += Up2 * pPlayer->GetActiveWeapon()->GetWpnData().beamLaserPos.z;
						}
						UTIL_TraceLine(vecOrigin, vecOrigin + (vecOriginEnd2 * MAX_BEAM_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)

						beamInfo.m_vecStart = tr.endpos;


						//beamInfo.m_nStartAttachment = iLaserAttachment;
						//beamInfo.m_pStartEnt = pBeamEnt;


						beamInfo.m_pStartEnt = NULL;
						beamInfo.m_nStartAttachment = -1;
					}

					LaserLightCallback(pPlayer->GetActiveWeapon(), tr, entindex());

					/*dlight_t *el = effects->CL_AllocDlight(pBeamEnt->entindex());//(LIGHT_INDEX_MUZZLEFLASH);//( index );
					el->origin = tr.endpos;
					el->color.r = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.r;
					el->color.g = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.g;
					el->color.b = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.b;
					el->color.exponent = pPlayer->GetActiveWeapon()->GetWpnData().lightBeamColor.a;
					el->radius = random->RandomInt(pPlayer->GetActiveWeapon()->GetWpnData().lightMinRadius, pPlayer->GetActiveWeapon()->GetWpnData().lightMaxRadius);
					el->decay = el->radius / pPlayer->GetActiveWeapon()->GetWpnData().lightDecay;
					el->die = gpGlobals->curtime + pPlayer->GetActiveWeapon()->GetWpnData().lightDie;*/

					/*dlight_t *ml = effects->CL_AllocDlight(pBeamEnt->entindex());//( index );
					ml->origin = vecOrigin2;
					ml->color.r = el->color.r;
					ml->color.g = el->color.g;
					ml->color.b = el->color.b;
					ml->color.exponent = el->color.exponent;
					if (pPlayer->GetActiveWeapon()->GetWpnData().lightVMMinRadius && pPlayer->GetActiveWeapon()->GetWpnData().lightVMMaxRadius)
						ml->radius = random->RandomInt(pPlayer->GetActiveWeapon()->GetWpnData().lightVMMinRadius, pPlayer->GetActiveWeapon()->GetWpnData().lightVMMaxRadius);
					else
						ml->radius = el->radius;
					ml->decay = el->decay;
					ml->die = el->die;*/

				}
				else if (pPlayer->IsRunning() || pPlayer->IsReloadingWeapon() || wallBump2)
				{
					Vector vecOrigin3, vecAngles3;
					QAngle angAngles3;
					//C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
					C_BaseViewModel *pBeamEnt = pPlayer->GetViewModel();
					iLaserAttachment = pPlayer->GetViewModel()->LookupAttachment(pPlayer->GetActiveWeapon()->GetWpnData().beamAttachment);
					pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin3, angAngles3);
					::FormatViewModelAttachment(vecOrigin3, false);
					beamInfo.m_vecEnd = vecOrigin3;
					beamInfo.m_pEndEnt = NULL;
					beamInfo.m_nEndAttachment = -1;
					AngleVectors(angAngles3, &vecAngles3);
					UTIL_TraceLine(vecOrigin3, vecOrigin3 + (vecAngles3 * MAX_BEAM_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

					beamInfo.m_vecStart = vecOrigin3;
					beamInfo.m_pStartEnt = NULL;
					beamInfo.m_nStartAttachment = -1;
				}
				if (!FStrEq(pPlayer->GetActiveWeapon()->GetWpnData().beamMaterial, "None"))
					beamInfo.m_pszModelName = pPlayer->GetActiveWeapon()->GetWpnData().beamMaterial;//"effects/bluelaser1.vmt";
				else
					beamInfo.m_pszModelName = "effects/bluelaser1.vmt";

				if (!(tr.surface.flags & SURF_SKY))
				{
					if (!FStrEq(pPlayer->GetActiveWeapon()->GetWpnData().beamHaloMaterial, "None"))
						beamInfo.m_pszHaloName = pPlayer->GetActiveWeapon()->GetWpnData().beamHaloMaterial;
					else
						beamInfo.m_pszHaloName = "sprites/light_glow03.vmt";

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
			}
		}
	}

	BaseClass::ClientThink();
}

void C_BasePlayer::update()
{
//	UpdateMuzzleFlashLaser();
	//UpdateMuzzleFlashSMG();
//	UpdateMuzzleFlashAR2();

	UpdateLaser();



	if (this->GetActiveWeapon() && this->GetActiveWeapon()->GetWpnData().AllowEnablingLight)
	{
		UpdateScopeLight();
	}
	else
	{
		if (m_ScopeLight)
		{
			delete m_ScopeLight;
			m_ScopeLight = NULL;
		}
	}
	if (this->GetActiveWeapon() && this->GetActiveWeapon()->GetWpnData().AllowEnablingIronsightLight && cvar->FindVar("oc_state_IRsight_bytime")->GetInt())
		UpdateIronSightLight();






	// Check for muzzle flash and apply to view model
	/*C_BaseAnimating *ve = this;
	if (GetObserverMode() == OBS_MODE_IN_EYE)
	{
		ve = dynamic_cast< C_BaseAnimating* >(GetObserverTarget());
	}*/
}

void C_BasePlayer::UpdateMuzzleFlashLaser()
{
	if ((IsEffectActive(EF_ML)) || (IsEffectActive(EF_IMM)))
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
		if (!pPlayer)
		{
			if (m_ML)
			{
				delete m_ML;
				m_ML = NULL;
				RemoveEffects(EF_M);
			}
			else if (m_MLM)
			{
				delete m_MLM;
				m_MLM = NULL;
				RemoveEffects(EF_M);
			}
			return;
		}

		//DevMsg("1: %.2f \n", 111);
		if (!m_ML)
		{
			m_ML = new C_MuzzleflashLaser(index);

			if (!m_ML)
				return;
			m_ML->TurnOnML();
		}
		if (!m_MLM)//Middle Laser
		{
			m_MLM = new C_MuzzleflashLaserM(index);

			if (!m_MLM)
				return;
			m_MLM->TurnOnMLM();
		}


		if (cvar->FindVar("oc_muzzleflash_attached")->GetBool())
		{
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_ML->UpdateLightML(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
			m_MLM->UpdateLightMLM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);

		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos;
			vecPos = EyePosition();
			EyeVectors(&vecForward, &vecRight, &vecUp);

			float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();

			C_BaseViewModel  *pVm = pPlayer->GetViewModel();
			if (pVm->m_bExpSighted)
			{
				vecPos = EyePosition();
			}
			else
			{
				vecPos += vecForward * Flash_adjust_forward;//Добавление этих координат убирает отображение вьюмодели, почему-то, если в скриптах не указаны координаты ironsight и обычные.
				vecPos += vecRight * Flash_adjust_right;
				vecPos += vecUp * Flash_adjust_up;//Добавление этих координат убирает отображение вьюмодели, почему-то, если в скриптах не указаны координаты ironsight и обычные.
			}


			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = MuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);

			// Update the light with the new position and direction.
			m_ML->UpdateLightML(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);



			QAngle angLightDir;//Middle_Laser
			Vector vecLightOrigin;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);//1
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_MLM->UpdateLightMLM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);

		}

	}
	else if (IsEffectActive(EF_ML_LEFT))
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
		if (!pPlayer)
		{
			if (m_ML)
			{
				delete m_ML;
				m_ML = NULL;
				RemoveEffects(EF_M);
			}
			else if (m_MLM)
			{
				delete m_MLM;
				m_MLM = NULL;
				RemoveEffects(EF_M);
			}
			return;
		}

		if (!m_ML)
		{
			m_ML = new C_MuzzleflashLaser(index);

			if (!m_ML)
				return;
			m_ML->TurnOnML();
		}
		if (!m_MLM)//Middle_Laser
		{
			m_MLM = new C_MuzzleflashLaserM(index);

			if (!m_MLM)
				return;
			m_MLM->TurnOnMLM();
		}


		if (cvar->FindVar("oc_muzzleflash_attached")->GetBool())
		{
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(2, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_ML->UpdateLightML(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos;
			vecPos = EyePosition();
			EyeVectors(&vecForward, &vecRight, &vecUp);

			float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
			if (!pPlayer)
				return;
			C_BaseViewModel  *pVm = pPlayer->GetViewModel();
			if (pVm->m_bExpSighted)
			{
				vecPos = EyePosition();
			}
			else
			{
				vecPos += vecForward * Flash_adjust_forward;
				vecPos += vecRight * (-Flash_adjust_right);
				vecPos += vecUp * Flash_adjust_up;
			}


			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = MuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);

			// Update the light with the new position and direction.
			m_ML->UpdateLightML(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);


			QAngle angLightDir;//Middle_Laser
			Vector vecLightOrigin;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(2, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_MLM->UpdateLightMLM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}

	}
	else if (m_ML)
	{
		delete m_ML;
		m_ML = NULL;

		delete m_MLM;//Middle_Laser
		m_MLM = NULL;
	}
}

void C_BasePlayer::UpdateMuzzleFlashSMG()
{
	/*CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());

	if (!pPlayer || !pPlayer->GetActiveWeapon() || pPlayer->ReinitializeMuzzleFlash)
		return;

	float lkoef = 0.f;

	if (IsMuzzleFlashActive)
	{
		if (m_flNextMuzzleFlashTime < gpGlobals->curtime && !IsMuzzleFlashComplete)
		{
			m_flNextMuzzleFlashTime = gpGlobals->curtime + m_M->MaxTime;

			if (m_flNextMuzzleFlashTime > pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack)
				m_flNextMuzzleFlashTime = pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack;

			if (pPlayer->GetActiveWeapon()->m_flNextPrimaryAttack - gpGlobals->curtime < 0.005)
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
		if (oc_muzzleflash_attached.GetBool())
		{
			C_BasePlayer *pOwner = C_BasePlayer::GetLocalPlayer();
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			angLightDir.z = MuzzleFlashRoll;
			int iAttachment = pOwner->GetActiveWeapon()->GetWpnData().iAttachment;
			GetViewModel()->GetAttachment(iAttachment, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_M->UpdateLightM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos, Muzzle;
			vecPos = EyePosition();
			EyeVectors(&vecForward, &vecRight, &vecUp);

			float Flash_adjust_right = pPlayer->GetActiveWeapon()->GetWpnData().ParticleTracerPosition.x;//oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = pPlayer->GetActiveWeapon()->GetWpnData().ParticleTracerPosition.y;//oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = pPlayer->GetActiveWeapon()->GetWpnData().ParticleTracerPosition.z;//oc_muzzleflash_adjust_forward.GetFloat();


			QAngle angAngles2;
			C_BasePlayer *pOwner = C_BasePlayer::GetLocalPlayer();
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			int iAttachment = pOwner->GetActiveWeapon()->GetWpnData().iAttachment;
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

				if (FClassnameIs(pPlayer->GetActiveWeapon(), "weapon_dual_pistols"))
				{
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 0)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_firstbarrel_tracer_y")->GetFloat();;
					}
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 1)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_secondbarrel_tracer_y")->GetFloat();;
					}
				}
				else
					vecPos += vecRight * Flash_adjust_right;

				vecPos += vecUp * Flash_adjust_up;

			}
			if (pPlayer->GetActiveWeapon()->FlashType() == 1 && m_Ar2_MM)
			{
				if (pPlayer->GetActiveWeapon()->MuzzleFlashAttachment() == 0)
				{
					QAngle ang2;
					VectorAngles(vecForward, vecUp, ang2);
					ang2.z = MuzzleFlashRoll;
					AngleVectors(ang2, &vecForward, &vecRight, &vecUp);

					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
				}
				else
					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
			}

			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = MuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);

			m_M->UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE, lkoef);
		}
	}*/
	/*if (IsEffectActive(EF_M))
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.

		if (!pPlayer || !pPlayer->GetActiveWeapon())
		{
			if (m_M)
			{
				delete m_M;
				m_M = NULL;
				RemoveEffects(EF_M);
			}
			else if (m_Ar2_MM)
			{
				delete m_Ar2_MM;
				m_Ar2_MM = NULL;
				RemoveEffects(EF_M);
			}
			return;
		}

		if (pPlayer->GetActiveWeapon() && !m_M)
		{
			m_M = new C_MuzzleflashEffect(index);

			if (!m_M)
				return;
			m_M->TurnOnM();
		}

		if (pPlayer->GetActiveWeapon() && pPlayer->GetActiveWeapon()->FlashType() == 1)
		{
			if (!m_Ar2_MM)//Middle_Flash
			{
				m_Ar2_MM = new C_Ar2_Middle_MuzzleflashEffect(index);

				if (!m_Ar2_MM)
					return;
				m_Ar2_MM->Ar2_Middle_TurnOnM();
			}
		}


		if (oc_muzzleflash_attached.GetBool())
		{
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_M->UpdateLightM(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos, Muzzle;
			vecPos = EyePosition();
			EyeVectors(&vecForward, &vecRight, &vecUp);

			float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();


			QAngle angAngles2;
			C_BasePlayer *pOwner = C_BasePlayer::GetLocalPlayer();
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			int iAttachment = pOwner->GetActiveWeapon()->GetWpnData().iAttachment;
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

				if (FClassnameIs(pPlayer->GetActiveWeapon(), "weapon_dual_pistols"))
				{
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 0)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_firstbarrel_tracer_y")->GetFloat();;
					}
					if (cvar->FindVar("oc_use_second_ballel")->GetInt() == 1)
					{
						vecPos += vecRight * cvar->FindVar("oc_weapon_dual_pistols_secondbarrel_tracer_y")->GetFloat();;
					}
				}
				else
				vecPos += vecRight * Flash_adjust_right;

				vecPos += vecUp * Flash_adjust_up;

			}
			if (pPlayer->GetActiveWeapon()->FlashType() == 1)
			{
				if (pPlayer->GetActiveWeapon()->MuzzleFlashAttachment() == 0)
				{
					QAngle ang2;
					VectorAngles(vecForward, vecUp, ang2);
					ang2.z = MuzzleFlashRoll;
					AngleVectors(ang2, &vecForward, &vecRight, &vecUp);

					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
				}
				else
					m_Ar2_MM->Ar2_Middle_UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
			}

			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = MuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);
			// Update the light with the new position and direction.
			m_M->UpdateLightM(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}

	}
	else if (m_M)
	{
		delete m_M;
		m_M = NULL;
	}
	else if (m_Ar2_MM)
	{
		delete m_Ar2_MM;
		m_Ar2_MM = NULL;
	}*/

}

void C_BasePlayer::UpdateScopeLight()
{
	C_BaseViewModel *pViewModel = GetViewModel();
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pViewModel && pWeapon)
	{
		//pViewModel->m_bExpSighted = true;

		if (pViewModel->alreadyInIronSight || pViewModel->m_bExpScope)//(cvar->FindVar("oc_state_InSecondFire")->GetInt() == 1)
		{
			if (!m_ScopeLight)
			{
				float fov = pViewModel->m_bExpScope ? pWeapon->GetWpnData().lightFov : pWeapon->GetWpnData().ironlightFov;

				float lightLinear = pViewModel->m_bExpScope ? pWeapon->GetWpnData().lightLinear : pWeapon->GetWpnData().ironlightLinear;

				float lightConstant = pViewModel->m_bExpScope ? pWeapon->GetWpnData().lightConstant : pWeapon->GetWpnData().ironlightConstant;

				float lightFar = pViewModel->m_bExpScope ? pWeapon->GetWpnData().lightFar : pWeapon->GetWpnData().ironlightFar;

				m_ScopeLight = new C_ScopeLight(index, lightLinear, lightConstant, fov, lightFar);

				if (!m_ScopeLight)
					return;
				m_ScopeLight->TurnOnScopeLight();
			}

			if (oc_scopelight_attached.GetBool())
			{
				QAngle angLightDir;
				Vector vecLightOrigin, vecForward, vecRight, vecUp;
				angLightDir.z = MuzzleFlashRoll;
				GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);
				::FormatViewModelAttachment(vecLightOrigin, true);
				AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
				m_ScopeLight->UpdateLightScopeLight(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
			}
			else
			{
				Vector vecForward, vecRight, vecUp, vecPos;
				vecPos = EyePosition();
				EyeVectors(&vecForward, &vecRight, &vecUp);

				float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
				float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
				float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();
				CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
				if (!pPlayer)
					return;
				C_BaseViewModel  *pVm = pPlayer->GetViewModel();
				if (pVm->alreadyInIronSight)
				{
					vecPos = EyePosition();
				}
				else
				{
					vecPos += vecForward * Flash_adjust_forward;
					vecPos += vecRight * Flash_adjust_right;
					vecPos += vecUp * Flash_adjust_up;
				}

				QAngle ang;
				VectorAngles(vecForward, vecUp, ang);
				ang.z = MuzzleFlashRoll;
				AngleVectors(ang, &vecForward, &vecRight, &vecUp);

				// Update the light with the new position and direction.
				m_ScopeLight->UpdateLightScopeLight(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
			}

		}
		else if (!pViewModel->alreadyInIronSight && !pViewModel->m_bExpScope)//(cvar->FindVar("oc_state_InSecondFire")->GetInt() == 0)
		{
			delete m_ScopeLight;
			m_ScopeLight = NULL;
		}
	}
	else
	{
		if (m_ScopeLight != NULL)
		{
			delete m_ScopeLight;
			m_ScopeLight = NULL;
		}
	}
}

void C_BasePlayer::UpdateIronSightLight()
{
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		if (!m_ScopeLight)
		{
			m_ScopeLight = new C_ScopeLight(index, GetActiveWeapon()->GetWpnData().ironlightLinear, GetActiveWeapon()->GetWpnData().ironlightConstant, GetActiveWeapon()->GetWpnData().ironlightFov, GetActiveWeapon()->GetWpnData().ironlightFar);

			if (!m_ScopeLight)
				return;
			m_ScopeLight->TurnOnScopeLight();
		}

		if (oc_scopelight_attached.GetBool())
		{
			QAngle angLightDir;
			Vector vecLightOrigin, vecForward, vecRight, vecUp;
			angLightDir.z = MuzzleFlashRoll;
			GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);
			::FormatViewModelAttachment(vecLightOrigin, true);
			AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
			m_ScopeLight->UpdateLightScopeLight(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}
		else
		{
			Vector vecForward, vecRight, vecUp, vecPos;
			vecPos = EyePosition();
			EyeVectors(&vecForward, &vecRight, &vecUp);

			float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
			float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
			float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
			if (!pPlayer)
				return;
			C_BaseViewModel  *pVm = pPlayer->GetViewModel();
			if (pVm->m_bExpSighted)
			{
				vecPos = EyePosition();
			}
			else
			{
				vecPos += vecForward * Flash_adjust_forward;
				vecPos += vecRight * Flash_adjust_right;
				vecPos += vecUp * Flash_adjust_up;
			}

			QAngle ang;
			VectorAngles(vecForward, vecUp, ang);
			ang.z = MuzzleFlashRoll;
			AngleVectors(ang, &vecForward, &vecRight, &vecUp);

			// Update the light with the new position and direction.
			m_ScopeLight->UpdateLightScopeLight(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
		}

	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		delete m_ScopeLight;
		m_ScopeLight = NULL;
	}

}

void C_BasePlayer::UpdateMuzzleFlashAR2()
{

if (IsEffectActive(EF_ML))
{
	On = true;
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
	if (!pPlayer)
		return;
	if (!pPlayer->GetActiveWeapon()->FlashType())
		return;

	if (!m_ML)
	{
		m_ML = new C_MuzzleflashLaser(index);

		if (!m_ML)
			return;
		m_ML->TurnOnML();
	}



	if (cvar->FindVar("oc_muzzleflash_attached")->GetBool())
	{
		QAngle angLightDir;
		Vector vecLightOrigin, vecForward, vecRight, vecUp;
		angLightDir.z = MuzzleFlashRoll;
		GetViewModel()->GetAttachment(1, vecLightOrigin, angLightDir);
		::FormatViewModelAttachment(vecLightOrigin, true);
		AngleVectors(angLightDir, &vecForward, &vecRight, &vecUp);
		m_ML->UpdateLightML(vecLightOrigin, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);
	}
	else
	{
		Vector vecForward, vecRight, vecUp, vecPos, Muzzle;
		vecPos = EyePosition();
		EyeVectors(&vecForward, &vecRight, &vecUp);

		float Flash_adjust_right = oc_muzzleflash_adjust_right.GetFloat();
		float Flash_adjust_up = oc_muzzleflash_adjust_up.GetFloat();
		float Flash_adjust_forward = oc_muzzleflash_adjust_forward.GetFloat();

		QAngle angAngles2;
		C_BasePlayer *pOwner = C_BasePlayer::GetLocalPlayer();
		C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
		int iAttachment = pOwner->GetActiveWeapon()->GetWpnData().iAttachment;
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
			vecPos += vecRight * Flash_adjust_right;
			vecPos += vecUp * Flash_adjust_up;

		}


		QAngle ang;
		VectorAngles(vecForward, vecUp, ang);
		ang.z = MuzzleFlashRoll;
		AngleVectors(ang, &vecForward, &vecRight, &vecUp);

		// Update the light with the new position and direction.
		m_ML->UpdateLightML(vecPos, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE);


		float MaxTime = cvar->FindVar("oc_ProjectedMuzzleFlash_LifeTime")->GetFloat();
		float SpeedOfTime = cvar->FindVar("oc_ProjectedMuzzleFlash_SpeedOfTime")->GetFloat();
		if (TimeTick1 < MaxTime)
		{
			TimeTick1 = TimeTick1 + SpeedOfTime *gpGlobals->frametime;
		}
		else if (TimeTick1 >= MaxTime)
		{

			//TimeTick = 0.0f;
			pPlayer->RemoveEffects(EF_ML);
			On = false;
			TimeTick1 = 0.0f;
		}
		DevMsg("TimeTick1: %.2f \n", TimeTick1);
	}

}
else if (m_ML)
{
	TimeTick1 = 0.0f;
	delete m_ML;
	m_ML = NULL;
}


}

void C_BasePlayer::UpdateLaser()
{
	if (IsEffectActive(EF_LASER))
	{

		CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());//Этот участок отвечает за положение вспышки по центру экрана в режиме ironsight и без него.
		if (!pPlayer)
		{
			if (m_plaser)
			{
				delete m_plaser;
				m_plaser = NULL;
				RemoveEffects(EF_LASER);
			}
			return;
		}

		if (!m_plaser)
		{
			m_plaser = new CLaserEffect(index);
			if (!m_plaser)
				return;
			m_plaser->TurnOnLaser();
		}
		m_plaser->TurnOnLaser();

		QAngle angLightDir1;
		Vector vecLightOrigin1, vecForward1, vecRight1, vecUp1;

		GetViewModel()->GetAttachment(1, vecLightOrigin1, angLightDir1);
		::FormatViewModelAttachment(vecLightOrigin1, true);

		angLightDir1.z = angLightDir1.z + oc_weapon_free_aim_movemax_lasersight_angle.GetFloat();

		float laser_adjust_right = oc_laser_adjust_right.GetFloat();
		float laser_adjust_up = oc_laser_adjust_up.GetFloat();
		float laser_adjust_forward = oc_laser_adjust_forward.GetFloat();

		Vector			forward(1, 0, 0), up(0, 0, 1), right(0, 1, 0);
		AngleVectors(angLightDir1, &forward, &right, &up);
		vecLightOrigin1 += forward * laser_adjust_forward;
		vecLightOrigin1 += right * laser_adjust_right;
		vecLightOrigin1 += up * laser_adjust_up;

		if (::input->CAM_IsFreeAiming())
		{
			trace_t tr;
			Vector vecForward = vecLightOrigin1;
			Vector2D vec_AimPos = ::input->CAM_GetFreeAimCursor();

			int screen_x, screen_y;
			engine->GetScreenSize(screen_x, screen_y);
			screen_x *= vec_AimPos.x * oc_weapon_free_aim_movemax_lasersight_X.GetFloat() + 0.5f;
			screen_y *= vec_AimPos.y * oc_weapon_free_aim_movemax_lasersight_Y.GetFloat() + 0.5f;
			ScreenToWorld(screen_x, screen_y, pPlayer->GetFOV(), vecLightOrigin1, angLightDir1, vecForward);

			UTIL_TraceLine(vecLightOrigin1, vecLightOrigin1 + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);


			VectorAngles(vecForward, angLightDir1);

		}

		AngleVectors(angLightDir1, &vecForward1, &vecRight1, &vecUp1);
		m_plaser->UpdateLightLaser(vecLightOrigin1, vecForward1, vecRight1, vecUp1, FLASHLIGHT_DISTANCE * 2);

	}
	else if (m_plaser)
	{
		// Turned off the flashlight; delete it.
		delete m_plaser;
		m_plaser = NULL;
	}

}

