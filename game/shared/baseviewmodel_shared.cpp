//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "baseviewmodel_shared.h"
#include "datacache/imdlcache.h"
//#include "basecombatweapon_shared.h"
#include "in_buttons.h"//


#if defined( CLIENT_DLL )
#include "iprediction.h"
#include "prediction.h"
#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"
// cin: 070105 - ironsight mode changes
#include "convar.h"
#include "c_baseplayer.h"
#include "input.h"//
//#include "iinput.h"//
#include "view.h"
#include "c_cam_attachment.h"

#include "c_te_legacytempents.h"
#else
#include "vguiscreen.h"
#endif

#if defined( CLIENT_DLL ) && defined( SIXENSE )
#include "sixense/in_sixense.h"
#include "sixense/sixense_convars_extern.h"
#endif

#ifdef SIXENSE
extern ConVar in_forceuser;
#include "iclientmode.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VIEWMODEL_ANIMATION_PARITY_BITS 3

#define SCREEN_OVERLAY_MATERIAL "vgui/screens/vgui_overlay"




#if defined( CLIENT_DLL )

ConVar oc_viewbob_move_speed("oc_viewbob_move_speed", "0.01", FCVAR_ARCHIVE);

void AdjustWpnTestOffset(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetScope(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetMove(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetWall(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetRT(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetRTIron(ConVar *pConVar, char *pszString);
void AdjustWpnTestOffsetIronSight(ConVar *pConVar, char *pszString);

ConVar   oc_adjust_weapon_rtscope_offset("oc_adjust_weapon_rtscope_offset", "0", 0, "Tests weapon offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetRT));
ConVar   oc_adjust_weapon_irrtscope_offset("oc_adjust_weapon_irrtscope_offset", "0", 0, "Tests weapon offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetRTIron));

ConVar   oc_adjust_weapon_rtscope_offset_x("oc_adjust_weapon_rtscope_offset_x", "0");
ConVar   oc_adjust_weapon_rtscope_offset_y("oc_adjust_weapon_rtscope_offset_y", "0");
ConVar   oc_adjust_weapon_rtscope_offset_z("oc_adjust_weapon_rtscope_offset_z", "0");
ConVar   oc_adjust_weapon_irrtscope_offset_x("oc_adjust_weapon_irrtscope_offset_x", "0");
ConVar   oc_adjust_weapon_irrtscope_offset_y("oc_adjust_weapon_irrtscope_offset_y", "0");
ConVar   oc_adjust_weapon_irrtscope_offset_z("oc_adjust_weapon_irrtscope_offset_z", "0");

ConVar   oc_adjust_weapon_offset("oc_adjust_weapon_offset", "0", FCVAR_CHEAT, "Standart weapon offsets", ((FnChangeCallback_t)AdjustWpnTestOffset));
ConVar   oc_adjust_weapon_offset_x("oc_adjust_weapon_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_offset_y("oc_adjust_weapon_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_offset_z("oc_adjust_weapon_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_offset_angle_x("oc_adjust_weapon_offset_angle_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_offset_angle_y("oc_adjust_weapon_offset_angle_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_offset_angle_z("oc_adjust_weapon_offset_angle_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_weapon_ironsight_offset("oc_adjust_weapon_ironsight_offset", "0", FCVAR_CHEAT, "Tests weapon offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetIronSight));
ConVar   oc_adjust_weapon_ironsight_offset_x("oc_adjust_weapon_ironsight_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_ironsight_offset_y("oc_adjust_weapon_ironsight_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_ironsight_offset_z("oc_adjust_weapon_ironsight_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_ironsight_offset_angle_x("oc_adjust_weapon_ironsight_offset_angle_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_ironsight_offset_angle_y("oc_adjust_weapon_ironsight_offset_angle_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_ironsight_offset_angle_z("oc_adjust_weapon_ironsight_offset_angle_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_weapon_scope_offset("oc_adjust_weapon_scope_offset", "0", 0, "Tests weapon offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetScope));
ConVar   oc_adjust_weapon_scope_offset_x("oc_adjust_weapon_scope_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_scope_offset_y("oc_adjust_weapon_scope_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_scope_offset_z("oc_adjust_weapon_scope_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_scope_offset_angle_x("oc_adjust_weapon_scope_offset_angle_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_scope_offset_angle_y("oc_adjust_weapon_scope_offset_angle_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_weapon_scope_offset_angle_z("oc_adjust_weapon_scope_offset_angle_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_wall_offset("oc_adjust_wall_offset", "0", FCVAR_CHEAT, "Tests weapon wall offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetWall));
ConVar   oc_adjust_wall_offset_x("oc_adjust_wall_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_wall_offset_y("oc_adjust_wall_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_wall_offset_z("oc_adjust_wall_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_wall_offset_angle_x("oc_adjust_wall_offset_angle_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_wall_offset_angle_y("oc_adjust_wall_offset_angle_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_wall_offset_angle_z("oc_adjust_wall_offset_angle_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_wall_distance_to("oc_adjust_wall_distance_to", "0", FCVAR_CHEAT);

ConVar   oc_adjust_moveForward_offset("oc_adjust_moveForward_offset", "0", 0, "Tests weapon run offsets", ((FnChangeCallback_t)AdjustWpnTestOffsetMove));
ConVar   oc_adjust_moveForward_offset_x("oc_adjust_moveForward_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveForward_offset_y("oc_adjust_moveForward_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveForward_offset_z("oc_adjust_moveForward_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveForward_offset_angle_x("oc_adjust_moveForward_offset_angle_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveForward_offset_angle_y("oc_adjust_moveForward_offset_angle_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveForward_offset_angle_z("oc_adjust_moveForward_offset_angle_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_moveBackward_offset_x("oc_adjust_moveBackward_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveBackward_offset_y("oc_adjust_moveBackward_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveBackward_offset_z("oc_adjust_moveBackward_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveBackward_ori_offset_x("oc_adjust_moveBackward_ori_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveBackward_ori_offset_y("oc_adjust_moveBackward_ori_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveBackward_ori_offset_z("oc_adjust_moveBackward_ori_offset_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_moveLeft_offset_x("oc_adjust_moveLeft_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveLeft_offset_y("oc_adjust_moveLeft_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveLeft_offset_z("oc_adjust_moveLeft_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveLeft_ori_offset_x("oc_adjust_moveLeft_ori_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveLeft_ori_offset_y("oc_adjust_moveLeft_ori_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveLeft_ori_offset_z("oc_adjust_moveLeft_ori_offset_z", "0", FCVAR_CHEAT);

ConVar   oc_adjust_moveRight_offset_x("oc_adjust_moveRight_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveRight_offset_y("oc_adjust_moveRight_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveRight_offset_z("oc_adjust_moveRight_offset_z", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveRight_ori_offset_x("oc_adjust_moveRight_ori_offset_x", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveRight_ori_offset_y("oc_adjust_moveRight_ori_offset_y", "0", FCVAR_CHEAT);
ConVar   oc_adjust_moveRight_ori_offset_z("oc_adjust_moveRight_ori_offset_z", "0", FCVAR_CHEAT);



ConVar oc_viewmodel_lag_type("oc_viewmodel_lag_type", "0", FCVAR_ARCHIVE);
ConVar oc_viewmodel_bob_enable("oc_viewmodel_bob_enable", "0", FCVAR_ARCHIVE);

ConVar oc_viewmodel_movement_type("oc_viewmodel_movement_type", "0", FCVAR_ARCHIVE);

ConVar oc_viewmodel_move_fwd_bwd_change_speed_type0("oc_viewmodel_move_fwd_bwd_change_speed_type0", "0.051", FCVAR_ARCHIVE);
ConVar oc_viewmodel_move_fwd_bwd_change_speed_type1("oc_viewmodel_move_fwd_bwd_change_speed_type1", "0.0079", FCVAR_ARCHIVE);
ConVar oc_viewmodel_move_wall_change_speed("oc_viewmodel_move_wall_change_speed", "0.06255", FCVAR_ARCHIVE);
ConVar oc_viewmodel_ironsight_move_speed("oc_viewmodel_ironsight_move_speed", "0.08", FCVAR_ARCHIVE);
ConVar oc_viewmodel_secondfire_move_speed("oc_viewmodel_secondfire_move_speed", "0.5", FCVAR_ARCHIVE);
ConVar oc_viewmodel_move_wall_type("oc_viewmodel_move_wall_type", "1", FCVAR_ARCHIVE);

ConVar oc_viewmodel_lag_change_speed("oc_viewmodel_lag_change_speed", "1.5", FCVAR_ARCHIVE);

ConVar oc_viewmodel_dynamic_FOV("oc_viewmodel_dynamic_FOV", "0");
ConVar oc_viewmodel_dynamic_FOV_sighted("oc_viewmodel_dynamic_FOV_sighted", "0");
ConVar oc_viewmodel_dynamic_FOV_unsighted("oc_viewmodel_dynamic_FOV_unsighted", "0");


ConVar oc_weapon_free_aim_movemax_viewmodel_change_time("oc_weapon_free_aim_movemax_viewmodel_change_time", "1.02", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_koef("oc_weapon_free_aim_movemax_viewmodel_koef", "0.02", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_tracers_koef("oc_weapon_free_aim_movemax_viewmodel_tracers_koef", "0.06", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_y("oc_weapon_free_aim_movemax_viewmodel_y", "-0.23", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_x("oc_weapon_free_aim_movemax_viewmodel_x", "-0.23", FCVAR_ARCHIVE);

ConVar oc_weapon_free_aim_movemax_viewmodel_tracers_y("oc_weapon_free_aim_movemax_viewmodel_tracers_y", "0.7", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_tracers_x("oc_weapon_free_aim_movemax_viewmodel_tracers_x", "0.3", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax_viewmodel_tracers_fov("oc_weapon_free_aim_movemax_viewmodel_tracers_fov", "0.3", FCVAR_ARCHIVE);


ConVar oc_weapon_free_aim_up_roll_range("oc_weapon_free_aim_up_roll_range", "0.7", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_down_roll_range("oc_weapon_free_aim_down_roll_range", "0.3", FCVAR_ARCHIVE);

void AdjustWpnTestOffset(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			oc_adjust_wall_distance_to.SetValue(pWeapon->GetWpnData().DistanceToWall);

			if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			{
				oc_adjust_weapon_offset_x.SetValue(pWeapon->GetWpnData().m_expOffsetTP.x);
				oc_adjust_weapon_offset_y.SetValue(pWeapon->GetWpnData().m_expOffsetTP.y);
				oc_adjust_weapon_offset_z.SetValue(pWeapon->GetWpnData().m_expOffsetTP.z);

				oc_adjust_weapon_offset_angle_x.SetValue(pWeapon->GetWpnData().m_expOriOffsetTP.x);
				oc_adjust_weapon_offset_angle_y.SetValue(pWeapon->GetWpnData().m_expOriOffsetTP.y);
				oc_adjust_weapon_offset_angle_z.SetValue(pWeapon->GetWpnData().m_expOriOffsetTP.z);
			}
			else
			{
				oc_adjust_weapon_offset_x.SetValue(pWeapon->GetWpnData().m_expOffset.x);
				oc_adjust_weapon_offset_y.SetValue(pWeapon->GetWpnData().m_expOffset.y);
				oc_adjust_weapon_offset_z.SetValue(pWeapon->GetWpnData().m_expOffset.z);

				oc_adjust_weapon_offset_angle_x.SetValue(pWeapon->GetWpnData().m_expOriOffset.x);
				oc_adjust_weapon_offset_angle_y.SetValue(pWeapon->GetWpnData().m_expOriOffset.y);
				oc_adjust_weapon_offset_angle_z.SetValue(pWeapon->GetWpnData().m_expOriOffset.z);
			}

			//Для настройки айронсайта
			oc_adjust_weapon_ironsight_offset_x.SetValue(pWeapon->GetWpnData().m_expOffset2.x);
			oc_adjust_weapon_ironsight_offset_y.SetValue(pWeapon->GetWpnData().m_expOffset2.y);
			oc_adjust_weapon_ironsight_offset_z.SetValue(pWeapon->GetWpnData().m_expOffset2.z);

			oc_adjust_weapon_ironsight_offset_angle_x.SetValue(pWeapon->GetWpnData().m_expOriOffset2.x);
			oc_adjust_weapon_ironsight_offset_angle_y.SetValue(pWeapon->GetWpnData().m_expOriOffset2.y);
			oc_adjust_weapon_ironsight_offset_angle_z.SetValue(pWeapon->GetWpnData().m_expOriOffset2.z);

			oc_adjust_wall_offset_x.SetValue(pWeapon->GetWpnData().PositionWall.x);
			oc_adjust_wall_offset_y.SetValue(pWeapon->GetWpnData().PositionWall.y);
			oc_adjust_wall_offset_z.SetValue(pWeapon->GetWpnData().PositionWall.z);

			oc_adjust_wall_offset_angle_x.SetValue(pWeapon->GetWpnData().AngleWall.x);
			oc_adjust_wall_offset_angle_y.SetValue(pWeapon->GetWpnData().AngleWall.y);
			oc_adjust_wall_offset_angle_z.SetValue(pWeapon->GetWpnData().AngleWall.z);

		}
	}
}

void AdjustWpnTestOffsetIronSight(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			oc_adjust_weapon_ironsight_offset_x.SetValue(pWeapon->GetWpnData().m_expOffset2.x);
			oc_adjust_weapon_ironsight_offset_y.SetValue(pWeapon->GetWpnData().m_expOffset2.y);
			oc_adjust_weapon_ironsight_offset_z.SetValue(pWeapon->GetWpnData().m_expOffset2.z);

			oc_adjust_weapon_ironsight_offset_angle_x.SetValue(pWeapon->GetWpnData().m_expOriOffset2.x);
			oc_adjust_weapon_ironsight_offset_angle_y.SetValue(pWeapon->GetWpnData().m_expOriOffset2.y);
			oc_adjust_weapon_ironsight_offset_angle_z.SetValue(pWeapon->GetWpnData().m_expOriOffset2.z);
		}
	}
}


void AdjustWpnTestOffsetScope(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			//Для настройки Scope
			oc_adjust_weapon_scope_offset_x.SetValue(pWeapon->GetWpnData().m_expOffsetScope.x);
			oc_adjust_weapon_scope_offset_y.SetValue(pWeapon->GetWpnData().m_expOffsetScope.y);
			oc_adjust_weapon_scope_offset_z.SetValue(pWeapon->GetWpnData().m_expOffsetScope.z);

			oc_adjust_weapon_scope_offset_angle_x.SetValue(pWeapon->GetWpnData().m_expOriOffsetScope.x);
			oc_adjust_weapon_scope_offset_angle_y.SetValue(pWeapon->GetWpnData().m_expOriOffsetScope.y);
			oc_adjust_weapon_scope_offset_angle_z.SetValue(pWeapon->GetWpnData().m_expOriOffsetScope.z);
		}
	}
}

void AdjustWpnTestOffsetMove(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			oc_adjust_moveForward_offset_x.SetValue(pWeapon->GetWpnData().PositionFwd.x);
			oc_adjust_moveForward_offset_y.SetValue(pWeapon->GetWpnData().PositionFwd.y);
			oc_adjust_moveForward_offset_z.SetValue(pWeapon->GetWpnData().PositionFwd.z);

			oc_adjust_moveForward_offset_angle_x.SetValue(pWeapon->GetWpnData().AngleFwd.x);
			oc_adjust_moveForward_offset_angle_y.SetValue(pWeapon->GetWpnData().AngleFwd.y);
			oc_adjust_moveForward_offset_angle_z.SetValue(pWeapon->GetWpnData().AngleFwd.z);
		}
	}
}
void AdjustWpnTestOffsetWall(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			oc_adjust_wall_offset_x.SetValue(0);
			oc_adjust_wall_offset_y.SetValue(0);
			oc_adjust_wall_offset_z.SetValue(0);

			oc_adjust_wall_offset_angle_x.SetValue(0);
			oc_adjust_wall_offset_angle_y.SetValue(0);
			oc_adjust_wall_offset_angle_z.SetValue(0);
		}
	}
}

void AdjustWpnTestOffsetRT(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			//Для настройки RenderTargets
			oc_adjust_weapon_rtscope_offset_x.SetValue(pWeapon->GetWpnData().RT_Offset.x);
			oc_adjust_weapon_rtscope_offset_y.SetValue(pWeapon->GetWpnData().RT_Offset.y);
			oc_adjust_weapon_rtscope_offset_z.SetValue(pWeapon->GetWpnData().RT_Offset.z);
		}
	}
}
void AdjustWpnTestOffsetRTIron(ConVar *pConVar, char *pszString)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (pPlayer)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			//Для настройки RenderTargets
			oc_adjust_weapon_irrtscope_offset_x.SetValue(pWeapon->GetWpnData().RT_OffsetIron.x);
			oc_adjust_weapon_irrtscope_offset_y.SetValue(pWeapon->GetWpnData().RT_OffsetIron.y);
			oc_adjust_weapon_irrtscope_offset_z.SetValue(pWeapon->GetWpnData().RT_OffsetIron.z);
		}
	}
}

void CalcExpWpnOffsets(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	// this is a simple test mode to help determine the proper values
	// to place in the weapon script

	CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
	if (pWeapon)
	{
		if (oc_adjust_weapon_offset.GetBool())
		{
			ang.x += oc_adjust_weapon_offset_angle_x.GetFloat();
			ang.y += oc_adjust_weapon_offset_angle_y.GetFloat();
			ang.z += oc_adjust_weapon_offset_angle_z.GetFloat();
			offset.Init(oc_adjust_weapon_offset_x.GetFloat(),
				oc_adjust_weapon_offset_y.GetFloat(),
				oc_adjust_weapon_offset_z.GetFloat());
		}
		else
		{
			if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
			{
				ang += pWeapon->GetWpnData().m_expOriOffsetTP;
				offset = pWeapon->GetWpnData().m_expOffsetTP;
			}
			else
			{
				ang += pWeapon->GetWpnData().m_expOriOffset;
				offset = pWeapon->GetWpnData().m_expOffset;
			}
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}

void CalcExpWpnOffsets2(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;


	if (oc_adjust_weapon_ironsight_offset.GetBool())
    {
		ang.x += oc_adjust_weapon_ironsight_offset_angle_x.GetFloat();
		ang.y += oc_adjust_weapon_ironsight_offset_angle_y.GetFloat();
		ang.z += oc_adjust_weapon_ironsight_offset_angle_z.GetFloat();
		offset.Init(oc_adjust_weapon_ironsight_offset_x.GetFloat(),
			oc_adjust_weapon_ironsight_offset_y.GetFloat(),
			oc_adjust_weapon_ironsight_offset_z.GetFloat());
	}
	else
	{
	      CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
		  if (pWeapon)
	      {
	    	ang += pWeapon->GetWpnData().m_expOriOffset2;
	    	offset = pWeapon->GetWpnData().m_expOffset2;
	      }
    }

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}

void CalcExpWpnOffsetsScope(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;


	if (oc_adjust_weapon_scope_offset.GetBool())
	{
		ang.x += oc_adjust_weapon_scope_offset_angle_x.GetFloat();
		ang.y += oc_adjust_weapon_scope_offset_angle_y.GetFloat();
		ang.z += oc_adjust_weapon_scope_offset_angle_z.GetFloat();
		offset.Init(oc_adjust_weapon_scope_offset_x.GetFloat(),
			oc_adjust_weapon_scope_offset_y.GetFloat(),
			oc_adjust_weapon_scope_offset_z.GetFloat());
	}
	else
	{
		CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
		if (pWeapon)
		{
			ang += pWeapon->GetWpnData().m_expOriOffsetScope;
			offset = pWeapon->GetWpnData().m_expOffsetScope;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}

void CalcExpWpnMoveWallOffsets(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
	if (pWeapon)
	{
		if (oc_adjust_wall_offset.GetBool())
		{
			ang.x += oc_adjust_wall_offset_angle_x.GetFloat();
			ang.y += oc_adjust_wall_offset_angle_y.GetFloat();
			ang.z += oc_adjust_wall_offset_angle_z.GetFloat();
			offset.Init(oc_adjust_wall_offset_x.GetFloat(),
				oc_adjust_wall_offset_y.GetFloat(),
				oc_adjust_wall_offset_z.GetFloat());
		}
		else
		{
			ang += pWeapon->GetWpnData().AngleWall;
			offset = pWeapon->GetWpnData().PositionWall;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}

void CalcExpWpnMoveFwdOffsets(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
	if (pWeapon)
	{
		if (oc_adjust_moveForward_offset.GetBool())
		{
			ang.x += oc_adjust_moveForward_offset_angle_x.GetFloat();
			ang.y += oc_adjust_moveForward_offset_angle_y.GetFloat();
			ang.z += oc_adjust_moveForward_offset_angle_z.GetFloat();

			offset.Init(oc_adjust_moveForward_offset_x.GetFloat(),
				oc_adjust_moveForward_offset_y.GetFloat(),
				oc_adjust_moveForward_offset_z.GetFloat());
		}
		else
		{
			ang += pWeapon->GetWpnData().AngleFwd;
			offset = pWeapon->GetWpnData().PositionFwd;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}

void CalcExpWpnOffsetsRT(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	// this is a simple test mode to help determine the proper values
	// to place in the weapon script
	if (oc_adjust_weapon_rtscope_offset.GetBool())
	{
		offset.Init(oc_adjust_weapon_rtscope_offset_x.GetFloat(),
			oc_adjust_weapon_rtscope_offset_y.GetFloat(),
			oc_adjust_weapon_rtscope_offset_z.GetFloat());
	}
	else
	{
		CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
		if (pWeapon)
		{
				offset = pWeapon->GetWpnData().RT_Offset;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}
void CalcExpWpnOffsetsRTIron(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	// this is a simple test mode to help determine the proper values
	// to place in the weapon script
	if (oc_adjust_weapon_irrtscope_offset.GetBool())
	{
		offset.Init(oc_adjust_weapon_irrtscope_offset_x.GetFloat(),
			oc_adjust_weapon_irrtscope_offset_y.GetFloat(),
			oc_adjust_weapon_irrtscope_offset_z.GetFloat());
	}
	else
	{
		CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
		if (pWeapon)
		{
			offset = pWeapon->GetWpnData().RT_OffsetIron;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}



/*ConVar   oc_adjust_leg_offset("oc_adjust_leg_offset", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_x("oc_adjust_leg_offset_x", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_y("oc_adjust_leg_offset_y", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_z("oc_adjust_leg_offset_z", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_angle_x("oc_adjust_leg_offset_angle_x", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_angle_y("oc_adjust_leg_offset_angle_y", "0", FCVAR_ARCHIVE);
ConVar   oc_adjust_leg_offset_angle_z("oc_adjust_leg_offset_angle_z", "0", FCVAR_ARCHIVE);*/
void CalcLegOffsets(CBasePlayer *owner, Vector &pos, QAngle &ang)
{
	Vector   forward, right, up, offset;

	float x = 3.f;
	float y = 2.f;
	float z = -5.f;

	float x_ang = 0.f;
	float y_ang = 0.f;
	float z_ang = 0.f;

	{
		/*CBaseCombatWeapon *pWeapon = owner->GetActiveWeapon();
		if (pWeapon)*/
		{
			ang += QAngle(x_ang, y_ang, z_ang);

			offset.x = x;
			offset.y = y;
			offset.z = z;
		}
	}

	// get eye direction angles
	AngleVectors(ang, &forward, &right, &up);

	// apply the offsets
	pos += forward   * offset.x;
	pos += right     * offset.y;
	pos += up        * offset.z;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseViewModel::CBaseViewModel()
{
#if defined( CLIENT_DLL )
	// NOTE: We do this here because the color is never transmitted for the view model.
	m_nOldAnimationParity = 0;
	m_EntClientFlags |= ENTCLIENTFLAG_ALWAYS_INTERPOLATE;

#endif
	SetRenderColor( 255, 255, 255, 255 );

	// View model of this weapon
	m_sVMName			= NULL_STRING;		
	// Prefix of the animations that should be used by the player carrying this weapon
	m_sAnimationPrefix	= NULL_STRING;

	m_nViewModelIndex	= 0;

	m_nAnimationParity	= 0;
#if defined( CLIENT_DLL )

	fov1 = oc_viewmodel_dynamic_FOV_unsighted.GetInt();
	fov2 = oc_viewmodel_dynamic_FOV_sighted.GetInt();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseViewModel::~CBaseViewModel()
{
}

void CBaseViewModel::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();

	DestroyControlPanels();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::Spawn(void)
{
	Precache();
	SetSize(Vector(-8, -4, -2), Vector(8, 4, 2));
	SetSolid(SOLID_NONE);

#ifdef CLIENT_DLL
	lagScale = 1.f;
	m_bExpSighted = false;
	alreadyInIronSight = false;
	m_bExpScope = false;
	m_bIsLeg = false;
	IsNearWall = false;
	delta = 0.f;
	wasIronSighted = false;
	toggled_do_once = false;
	m_expFactor    = 0.0f;
	m_expFactor2 = 0.0f;
	m_flNextLagScale = gpGlobals->curtime;
	lagScale = 1.0f;
#else

#endif

}


#if defined ( CSTRIKE_DLL ) && !defined ( CLIENT_DLL )
#define VGUI_CONTROL_PANELS
#endif

#if defined ( TF_DLL )
#define VGUI_CONTROL_PANELS
#endif

#ifdef INVASION_DLL
#define VGUI_CONTROL_PANELS
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetControlPanelsActive( bool bState )
{
#if defined( VGUI_CONTROL_PANELS )
	// Activate control panel screens
	for ( int i = m_hScreens.Count(); --i >= 0; )
	{
		if (m_hScreens[i].Get())
		{
			m_hScreens[i]->SetActive( bState );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// This is called by the base object when it's time to spawn the control panels
//-----------------------------------------------------------------------------
void CBaseViewModel::SpawnControlPanels()
{
#if defined( VGUI_CONTROL_PANELS )
	char buf[64];

	// Destroy existing panels
	DestroyControlPanels();

	CBaseCombatWeapon *weapon = m_hWeapon.Get();

	if ( weapon == NULL )
	{
		return;
	}

	MDLCACHE_CRITICAL_SECTION();

	// FIXME: Deal with dynamically resizing control panels?

	// If we're attached to an entity, spawn control panels on it instead of use
	CBaseAnimating *pEntityToSpawnOn = this;
	char *pOrgLL = "controlpanel%d_ll";
	char *pOrgUR = "controlpanel%d_ur";
	char *pAttachmentNameLL = pOrgLL;
	char *pAttachmentNameUR = pOrgUR;
	/*
	if ( IsBuiltOnAttachment() )
	{
		pEntityToSpawnOn = dynamic_cast<CBaseAnimating*>((CBaseEntity*)m_hBuiltOnEntity.Get());
		if ( pEntityToSpawnOn )
		{
			char sBuildPointLL[64];
			char sBuildPointUR[64];
			Q_snprintf( sBuildPointLL, sizeof( sBuildPointLL ), "bp%d_controlpanel%%d_ll", m_iBuiltOnPoint );
			Q_snprintf( sBuildPointUR, sizeof( sBuildPointUR ), "bp%d_controlpanel%%d_ur", m_iBuiltOnPoint );
			pAttachmentNameLL = sBuildPointLL;
			pAttachmentNameUR = sBuildPointUR;
		}
		else
		{
			pEntityToSpawnOn = this;
		}
	}
	*/

	Assert( pEntityToSpawnOn );

	// Lookup the attachment point...
	int nPanel;
	for ( nPanel = 0; true; ++nPanel )
	{
		Q_snprintf( buf, sizeof( buf ), pAttachmentNameLL, nPanel );
		int nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
		if (nLLAttachmentIndex <= 0)
		{
			// Try and use my panels then
			pEntityToSpawnOn = this;
			Q_snprintf( buf, sizeof( buf ), pOrgLL, nPanel );
			nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
			if (nLLAttachmentIndex <= 0)
				return;
		}

		Q_snprintf( buf, sizeof( buf ), pAttachmentNameUR, nPanel );
		int nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
		if (nURAttachmentIndex <= 0)
		{
			// Try and use my panels then
			Q_snprintf( buf, sizeof( buf ), pOrgUR, nPanel );
			nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(buf);
			if (nURAttachmentIndex <= 0)
				return;
		}

		const char *pScreenName;
		weapon->GetControlPanelInfo( nPanel, pScreenName );
		if (!pScreenName)
			continue;

		const char *pScreenClassname;
		weapon->GetControlPanelClassName( nPanel, pScreenClassname );
		if ( !pScreenClassname )
			continue;

		// Compute the screen size from the attachment points...
		matrix3x4_t	panelToWorld;
		pEntityToSpawnOn->GetAttachment( nLLAttachmentIndex, panelToWorld );

		matrix3x4_t	worldToPanel;
		MatrixInvert( panelToWorld, worldToPanel );

		// Now get the lower right position + transform into panel space
		Vector lr, lrlocal;
		pEntityToSpawnOn->GetAttachment( nURAttachmentIndex, panelToWorld );
		MatrixGetColumn( panelToWorld, 3, lr );
		VectorTransform( lr, worldToPanel, lrlocal );

		float flWidth = lrlocal.x;
		float flHeight = lrlocal.y;

		CVGuiScreen *pScreen = CreateVGuiScreen( pScreenClassname, pScreenName, pEntityToSpawnOn, this, nLLAttachmentIndex );
		pScreen->ChangeTeam( GetTeamNumber() );
		pScreen->SetActualSize( flWidth, flHeight );
		pScreen->SetActive( false );
		pScreen->MakeVisibleOnlyToTeammates( false );
	
#ifdef INVASION_DLL
		pScreen->SetOverlayMaterial( SCREEN_OVERLAY_MATERIAL );
#endif
		pScreen->SetAttachedToViewModel( true );
		int nScreen = m_hScreens.AddToTail( );
		m_hScreens[nScreen].Set( pScreen );
	}
#endif
}

void CBaseViewModel::DestroyControlPanels()
{
#if defined( VGUI_CONTROL_PANELS )
	// Kill the control panels
	int i;
	for ( i = m_hScreens.Count(); --i >= 0; )
	{
		DestroyVGuiScreen( m_hScreens[i].Get() );
	}
	m_hScreens.RemoveAll();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetOwner( CBaseEntity *pEntity )
{
	m_hOwner = pEntity;
#if !defined( CLIENT_DLL )
	// Make sure we're linked into hierarchy
	//SetParent( pEntity );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetIndex( int nIndex )
{
	m_nViewModelIndex = nIndex;
	Assert( m_nViewModelIndex < (1 << VIEWMODEL_INDEX_BITS) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseViewModel::ViewModelIndex( ) const
{
	return m_nViewModelIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Pass our visibility on to our child screens
//-----------------------------------------------------------------------------
void CBaseViewModel::AddEffects( int nEffects )
{
	if ( nEffects & EF_NODRAW )
	{
		SetControlPanelsActive( false );
	}

	BaseClass::AddEffects( nEffects );
}

//-----------------------------------------------------------------------------
// Purpose: Pass our visibility on to our child screens
//-----------------------------------------------------------------------------
void CBaseViewModel::RemoveEffects( int nEffects )
{
	if ( nEffects & EF_NODRAW )
	{
		SetControlPanelsActive( true );
	}

	BaseClass::RemoveEffects( nEffects );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *modelname - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SetWeaponModel(const char *modelname, CBaseCombatWeapon *weapon)
{
	m_hWeapon = weapon;

#if defined( CLIENT_DLL )
	if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
	{
		PrecacheModel(cvar->FindVar("oc_playermodel_suit")->GetString());
		SetModel(cvar->FindVar("oc_playermodel_suit")->GetString());

		int bg_rl = FindBodygroupByName("right_leg");
		int bg_ll = FindBodygroupByName("left_leg");
		int bg_t = FindBodygroupByName("torso");
		int bg_h = FindBodygroupByName("head");
		SetBodygroup(bg_rl, 1);
		SetBodygroup(bg_ll, 1);
		SetBodygroup(bg_t, 1);
		SetBodygroup(bg_h, 1);
	}
	else
	{
		PrecacheModel(modelname);
		SetModel(modelname);
	}
#else
	string_t str;
	if ( modelname != NULL )
	{
		str = MAKE_STRING( modelname );
	}
	else
	{
		str = NULL_STRING;
	}

	if ( str != m_sVMName )
	{
		// Msg( "SetWeaponModel %s at %f\n", modelname, gpGlobals->curtime );
		m_sVMName = str;

		if (cvar->FindVar("oc_player_draw_body")->GetInt() && cvar->FindVar("oc_player_true_firstperson_vm")->GetInt())
		{
			PrecacheModel(cvar->FindVar("oc_playermodel_suit")->GetString());
			SetModel(cvar->FindVar("oc_playermodel_suit")->GetString());

			int bg_rh = FindBodygroupByName("right_hand");
			int bg_lh = FindBodygroupByName("left_hand");

			SetBodygroup(bg_rh, 0);
			SetBodygroup(bg_lh, 0);

			int bg_rl = FindBodygroupByName("right_leg");
			int bg_ll = FindBodygroupByName("left_leg");
			int bg_t = FindBodygroupByName("torso");
			int bg_h = FindBodygroupByName("head");
			SetBodygroup(bg_rl, 1);
			SetBodygroup(bg_ll, 1);
			SetBodygroup(bg_t, 1);
			SetBodygroup(bg_h, 1);
		}
		else
		{
			PrecacheModel(m_sVMName.ToCStr());
			SetModel(STRING(m_sVMName));
		}
		// Create any vgui control panels associated with the weapon
		SpawnControlPanels();

		bool showControlPanels = weapon && weapon->ShouldShowControlPanels();
		SetControlPanelsActive( showControlPanels );
	}
#endif
}

CStudioHdr *CBaseViewModel::OnNewModel()
{
	//SetBodygroup(0, 0);

	return BaseClass::OnNewModel();
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatWeapon
//-----------------------------------------------------------------------------
CBaseCombatWeapon *CBaseViewModel::GetOwningWeapon( void )
{
	return m_hWeapon.Get();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : sequence - 
//-----------------------------------------------------------------------------
void CBaseViewModel::SendViewModelMatchingSequence( int sequence )
{
	// since all we do is send a sequence number down to the client, 
	// set this here so other weapons code knows which sequence is playing.

	SetSequence(sequence);

	m_nAnimationParity = (m_nAnimationParity + 1) & ((1 << VIEWMODEL_ANIMATION_PARITY_BITS) - 1);

#if defined( CLIENT_DLL )
	m_nOldAnimationParity = m_nAnimationParity;

	// Force frame interpolation to start at exactly frame zero
	m_flAnimTime = gpGlobals->curtime;
#else
	CBaseCombatWeapon *weapon = m_hWeapon.Get();
	bool showControlPanels = weapon && weapon->ShouldShowControlPanels();
	SetControlPanelsActive( showControlPanels );
#endif

	// Restart animation at frame 0
	SetCycle(0);
	ResetSequenceInfo();

}

#if defined( CLIENT_DLL )
#include "ivieweffects.h"
#endif


extern void ScreenToWorld(int mousex, int mousey, float fov, const Vector& vecRenderOrigin, const QAngle& vecRenderAngles, Vector& vecPickingRay);


float yy = 0;
bool F = false;
bool D = false;
float alpha = 0.0f;
float Wfwd = 0.0f;//FORWARD
float fwd = 0.0f;//FORWARD
float deltaFwd = 0.0f;
float fwd0 = 0.0f;
float bwd0 = 0.0f;//BACKWARD

void CBaseViewModel::CalcViewModelView(CBasePlayer *owner, const Vector& eyePosition, const QAngle& eyeAngles)
{

#if defined( CLIENT_DLL )

	QAngle vmangoriginal = eyeAngles;
	QAngle vmangles = eyeAngles;
	Vector vmorigin = eyePosition;

	if (m_bIsLeg)
	{
		CalcLegOffsets(owner, vmorigin, vmangles);
		SetLocalOrigin(vmorigin);
		SetLocalAngles(vmangles);
		return;
	}

	Vector RTOrigin = eyePosition;
	Vector RTIronOrigin = eyePosition;
	QAngle vmanglesW = eyeAngles;
	Vector vmoriginW = eyePosition;
	QAngle vmanglesM = eyeAngles;
	Vector vmoriginM = eyePosition;

	QAngle vmangles2 = eyeAngles;
	Vector vmorigin2 = eyePosition;
	QAngle vmanglesScope = eyeAngles;
	Vector vmoriginScope = eyePosition;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer == NULL)
		return;

	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	//Allow weapon lagging
	C_BaseCombatWeapon *pWeapon_ = m_hWeapon.Get();

	if (pWeapon != NULL)
	{
		m_bExpSighted = (bool)pWeapon->IsIronSighted();
	}

	if (::input->CAM_IsFreeAiming())
	{
		QAngle Ang, vmangles2;

		trace_t tr;
		const Vector eyePos = pPlayer->EyePosition();
		Vector vecForward = MainViewForward();//vmorigin;
		Vector2D vec_AimPos = ::input->CAM_GetFreeAimCursor();

		int screen_x, screen_y;
		engine->GetScreenSize(screen_x, screen_y);
		screen_x *= vec_AimPos.x * oc_weapon_free_aim_movemax_viewmodel_y.GetFloat() + 0.5f;
		screen_y *= vec_AimPos.y * oc_weapon_free_aim_movemax_viewmodel_x.GetFloat() + 0.5f;
		ScreenToWorld(screen_x, screen_y, pPlayer->GetFOV(), MainViewOrigin(), vmangles, vecForward);
		
		UTIL_TraceLine(MainViewOrigin(), MainViewOrigin() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		VectorAngles(vecForward, vmangles);
	}

	int speed = 0;

	Vector velocity;
	if (pPlayer->GetFlags() & FL_ONGROUND)
	{
		pPlayer->EstimateAbsVelocity(velocity);
		speed = velocity.NormalizeInPlace();///Normalized
	}
	else
	{
		speed = cvar->FindVar("hl2_normspeed")->GetInt();

	}
	cvar->FindVar("oc_state_player_velocity")->SetValue(speed);

	bool wallBump = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt();
	bool wallBump2 = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? /*cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0*/ !IsNearWall : true;
	bool wallBump3 = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? /*cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1*/ IsNearWall : false;

	bool sprintLower = cvar->FindVar("oc_weapons_allow_sprint_lower")->GetInt() ? true : false;

	if (pWeapon != NULL)
	{
		if (pWeapon->letViewModelBob && 
			!m_bExpSighted && 
			wallBump2 &&
			!m_bExpScope)//cvar->FindVar("oc_state_InSecondFire")->GetInt() == 0)
		{
			if (lagScale <= 1.f)
			{
				/*if (m_flNextLagScale < gpGlobals->curtime)
				{
					m_flNextLagScale = gpGlobals->curtime + 1.f;
				}
				if (gpGlobals->curtime < m_flNextLagScale && lagScale <= 1.f)
				{
					lagScale = 1.01f-(m_flNextLagScale - gpGlobals->curtime);
				}*/
				lagScale += oc_viewmodel_lag_change_speed.GetFloat() * gpGlobals->frametime;
			}
		}
		else
		{
			if (lagScale > 0)
			{
				lagScale -= oc_viewmodel_lag_change_speed.GetFloat() * gpGlobals->frametime;
			}
			/*if (m_flNextLagScale < gpGlobals->curtime && lagScale >= 1.f)
			{
				m_flNextLagScale = gpGlobals->curtime + 1.f;
			}
			if (gpGlobals->curtime < m_flNextLagScale && lagScale >= 0.f)
			{
				lagScale = m_flNextLagScale - gpGlobals->curtime;
			}*/
		}

		if (pWeapon->m_bWeaponBlockWall)
		{
			wallBump = wallBump2 = wallBump3 = false;
		}
		//DevMsg("lagScale: %.2f \n", lagScale);

/*#if defined( CLIENT_DLL )
		if (!prediction->InPrediction())
#endif*/
		//{

		CalcViewModelLag(vmorigin, vmangles, vmangoriginal, oc_viewmodel_lag_type.GetInt(), lagScale);
		CalcViewModelLag(vmoriginM, vmangles, vmangoriginal, oc_viewmodel_lag_type.GetInt(), lagScale);

		//}

		if (oc_viewmodel_bob_enable.GetInt())
		{
			if (pWeapon->GetWpnData().ViewModelShake)
			{
				pWeapon->AddViewmodelBob(this, vmorigin, vmangles, lagScale);
				pWeapon->AddViewmodelBob(this, vmoriginM, vmanglesM, lagScale);
			}
		}
		
	}

	CalcExpWpnOffsets(owner, vmorigin, vmangles);
	CalcExpWpnOffsetsRT(owner, RTOrigin, vmangles);
	CalcExpWpnOffsetsRTIron(owner, RTIronOrigin, vmangles2);

	if (wallBump3)
	{
		CalcExpWpnMoveWallOffsets(owner, vmoriginW, vmanglesW);
	}

	CalcExpWpnMoveFwdOffsets(owner, vmoriginM, vmanglesM);

	CalcExpWpnOffsets2(owner, vmorigin2, vmangles2);
	CalcExpWpnOffsetsScope(owner, vmoriginScope, vmangles2);

	Vector difPos(vmorigin - eyePosition);
	Vector RTdifPos(RTOrigin - eyePosition);
	Vector RTIdifPos(RTIronOrigin - eyePosition);
	Vector difPosW(vmoriginW - eyePosition);
	Vector difPosM(vmoriginM - eyePosition);
	Vector difPos2(vmorigin2 - eyePosition);
	Vector difPosScope(vmoriginScope - eyePosition);

	if (pWeapon != NULL)
	{
		/*trace_t tr, m_trHit;
		Vector vecDir, vecStop, vecSrc, vForward, vRight, vUp;
		pPlayer->EyeVectors(&vForward, &vRight, &vUp);
		QAngle Angle = pPlayer->EyeAngles();

		Vector res = pPlayer->EyePosition();
		res += vForward;
		res += vRight;
		res += vUp;
		vecSrc = res;
		AngleVectors(Angle, &vecDir);
		vecStop = vecSrc + vecDir * MAX_TRACE_LENGTH;
		UTIL_TraceLine(vecSrc, vecStop, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

		float Dist = pWeapon_->GetWpnData().DistanceToWall;*/

		//if (tr.m_pEnt != NULL ? !FClassnameIs(tr.m_pEnt, "physics_prop_ragdoll") && !FClassnameIs(tr.m_pEnt, "prop_ragdoll") && !FClassnameIs(tr.m_pEnt, "prop_ragdoll_attached") : true)
		{
			if (wallBump)
			{
				if ((oc_viewmodel_move_wall_type.GetInt() == 1)
					&& !pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC() && !(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						F = true;
						D = false;
						alpha = oc_viewmodel_move_wall_change_speed.GetFloat() * gpGlobals->frametime;
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{
						D = true;
						F = false;
						alpha = oc_viewmodel_move_wall_change_speed.GetFloat() * gpGlobals->frametime;
					}

					if ((!D) && (!F))
					{
						alpha = 0.0f;
					}
					else if ((D) && (F))
					{
						alpha = 0.0f;
					}

					//Vector Pos = m_bExpSighted ? difPos2 : difPos;
					if ((!D) && (F))
					{
						if ((yy < 1.0f) && ((!D) && (F)))
							Wfwd = Wfwd + alpha;

						yy = (yy < 1.0f - alpha) ? Wfwd : 1.0f;



						difPos = Lerp(yy, difPos, difPosW);
						vmangles = Lerp(yy, vmangles, vmanglesW);

						vmorigin = eyePosition + difPos;

						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}

					if ((D) && (!F))
					{

						if ((yy > 0.0f) && ((D) && (!F)))
							Wfwd = Wfwd - alpha;
						yy = (yy > 0.0f + alpha) ? Wfwd : 0.0f;

						difPos = Lerp(yy, difPos, difPosW);
						vmangles = Lerp(yy, vmangles, vmanglesW);
						vmorigin = eyePosition + difPos;

						if (yy == 0.0f)
						{
							IsNearWall = false;

							cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
						}
					}

				}
				else if ((oc_viewmodel_move_wall_type.GetInt() == 1) &&
					!pPlayer->IsReloadingWeapon() &&
					pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC()) && (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{
						IsNearWall = false;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
					}
				}



				if ((oc_viewmodel_move_wall_type.GetInt() == 0)
					&& !pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					trace_t tr, m_trHit;
					Vector vecDir, vecStop, vecSrc, vForward, vRight, vUp;
					pPlayer->EyeVectors(&vForward, &vRight, &vUp);
					QAngle Angle = pPlayer->EyeAngles();

					Vector res = pPlayer->EyePosition();
					res += vForward;
					res += vRight;
					res += vUp;
					vecSrc = res;
					AngleVectors(Angle, &vecDir);
					vecStop = vecSrc + vecDir * MAX_TRACE_LENGTH;
					UTIL_TraceLine(vecSrc, vecStop, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

					float Dist = pWeapon_->GetWpnData().DistanceToWall;

					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC()) && (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						//alpha = (1 / (1 / (tr.endpos.DistTo(vecSrc)))) * (tr.endpos.DistTo(vecSrc));


						alpha = abs(((tr.endpos.DistTo(vecSrc)) / (tr.endpos.DistTo(vecSrc))) - ((1 / (tr.endpos.DistTo(vecSrc))*((Dist * 5) / (tr.endpos.DistTo(vecSrc))))));

						//alpha = abs(((pWeapon->distanceToWall) / (pWeapon->distanceToWall)) - ((1 / (pWeapon->distanceToWall)*((Dist * 5) / (pWeapon->distanceToWall)))));

						//DevMsg("Alpha: %.2f \n", alpha);
						Clamp(alpha, 0.0f, 1.0f);

						difPos = Lerp(alpha, difPosW, difPos);
						vmangles = Lerp(alpha, vmanglesW, vmangles);

						vmorigin = eyePosition + difPos;

						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{

						alpha = 0.0f;


						difPos = Lerp(alpha, difPos, difPosW);
						vmangles = Lerp(alpha, vmangles, vmanglesW);

						vmorigin = eyePosition + difPos;

						IsNearWall = false;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
					}
				}
				else if ((oc_viewmodel_move_wall_type.GetInt() == 0) &&
					!pPlayer->IsReloadingWeapon() &&
					pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC()) && (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{
						IsNearWall = false;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
					}
				}



				if ((oc_viewmodel_move_wall_type.GetInt() == 2)
					&& !pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					trace_t tr, m_trHit;
					Vector vecDir, vecStop, vecSrc, vForward, vRight, vUp;
					pPlayer->EyeVectors(&vForward, &vRight, &vUp);
					QAngle Angle = pPlayer->EyeAngles();

					Vector res = pPlayer->EyePosition();
					res += vForward;
					res += vRight;
					res += vUp;
					vecSrc = res;
					AngleVectors(Angle, &vecDir);
					vecStop = vecSrc + vecDir * MAX_TRACE_LENGTH;
					UTIL_TraceLine(vecSrc, vecStop, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

					float Dist = pWeapon_->GetWpnData().DistanceToWall;

					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC()) && (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						alpha = abs(((tr.endpos.DistTo(vecSrc)) / (tr.endpos.DistTo(vecSrc))) - ((1 / (tr.endpos.DistTo(vecSrc))*((Dist * 5) / (tr.endpos.DistTo(vecSrc))))));

						//alpha = abs(((pWeapon->distanceToWall) / (pWeapon->distanceToWall)) - ((1 / (pWeapon->distanceToWall)*((Dist * 5) / (pWeapon->distanceToWall)))));

						Clamp(alpha, 0.0f, 1.0f);

						difPos = Lerp(alpha, difPosW, difPos);
						vmangles = Lerp(alpha, vmanglesW, vmangles);

						vmorigin = eyePosition + difPos;

						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{
						alpha = 0.0f;

						difPos = Lerp(alpha, difPos, difPosW);
						vmangles = Lerp(alpha, vmangles, vmanglesW);

						vmorigin = eyePosition + difPos;

						IsNearWall = false;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
					}
				}
				else if ((oc_viewmodel_move_wall_type.GetInt() == 2) &&
					!pPlayer->IsReloadingWeapon() &&
					pWeapon_->GetWpnData().animData[pWeapon_->m_bFireMode].WallAnimation)
				{
					//if ((tr.endpos.DistTo(vecSrc) < Dist) && (!tr.m_pEnt->IsNPC()) && (!(tr.surface.flags & (SURF_TRIGGER | SURF_WARP | SURF_HITBOX))))
					if (pWeapon->clipNearWall)
					{
						IsNearWall = true;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(1);
					}
					//if (tr.endpos.DistTo(vecSrc) >= Dist)
					else
					{
						IsNearWall = false;

						cvar->FindVar("oc_state_near_wall_standing")->SetValue(0);
					}
				}
			}
		}

		if (m_bExpSighted)
		{		
			wasIronSighted = true;

			if (delta < 1.0f)
			{
				delta = Clamp((delta + (oc_viewmodel_ironsight_move_speed.GetFloat() * gpGlobals->frametime)), 0.f, 1.f);
			}
			else
			{
				delta = 1.f;
				alreadyInIronSight = true;
			}

			m_expFactor2 = delta;

			difPos = Lerp(m_expFactor2, difPos, difPos2);
			RTdifPos = Lerp(m_expFactor2, RTdifPos, RTIdifPos);
			vmangles = Lerp(m_expFactor2, vmangles, vmangles2);

			vmorigin = eyePosition + difPos;
			RTOrigin = eyePosition + RTdifPos;
		}

		if (m_bExpScope)
		{
			wasIronSighted = false;

			if (delta < 1.0f)
				delta = Clamp((float)(delta + (oc_viewmodel_secondfire_move_speed.GetFloat() * gpGlobals->frametime * 2.5)), 0.f, 1.f);
			else
				delta = 1.f;

			m_expFactor2 = delta;

			difPos = Lerp(m_expFactor2, difPos, difPosScope);
			RTdifPos = Lerp(m_expFactor2, RTdifPos, RTIdifPos);
			vmangles = Lerp(m_expFactor2, vmangles, vmangles2);

			vmorigin = eyePosition + difPos;
			RTOrigin = eyePosition + RTdifPos;
		}

		if (!m_bExpSighted && !m_bExpScope)
		{
			cvar->FindVar("oc_state_IRsight_bytime")->SetValue(0);

			if (wasIronSighted)
			{
				if (delta > 0.0f)
					delta = delta - (oc_viewmodel_ironsight_move_speed.GetFloat() * gpGlobals->frametime);
				else
				{
					delta = 0.f;
				}

				alreadyInIronSight = false;

				m_expFactor2 = delta;

				difPos = Lerp(m_expFactor2, difPos, difPos2);

				RTdifPos = Lerp(m_expFactor2, RTdifPos, RTIdifPos);
				vmangles = Lerp(m_expFactor2, vmangles, vmangles2);
				vmorigin = eyePosition + difPos;
				RTOrigin = eyePosition + RTdifPos;

				if (delta <= 0.0f)
				{		
					cvar->FindVar("oc_state_IRsight_on")->SetValue(0);
				}
			}
			else
			{
				if (delta > 0.0f)
					delta = delta - (oc_viewmodel_secondfire_move_speed.GetFloat() * gpGlobals->frametime * 3);
				else
					delta = 0.f;

				m_expFactor2 = delta;

				if (delta <= 0.0f)
				{
					cvar->FindVar("oc_state_IRsight_on")->SetValue(0);
				}

				vmorigin = eyePosition + difPos;
				RTOrigin = eyePosition + RTdifPos;
			}
		}

		if (sprintLower && !m_bExpScope && !pWeapon->IsInReload() )//&& !m_bExpSighted)
		{
			/*if (speed >= cvar->FindVar("hl2_sprintspeed")->GetInt() - 10)
			{
				cvar->FindVar("oc_state_is_running")->SetValue(1);
			}
			if (speed < cvar->FindVar("hl2_sprintspeed")->GetInt() - 10)
			{
				cvar->FindVar("oc_state_is_running")->SetValue(0);
			}*/
	
			if (oc_viewmodel_movement_type.GetInt() == 1)
			{
				if ((speed >= cvar->FindVar("hl2_sprintspeed")->GetInt() - 10))
				{
					if (deltaFwd < 1.0f)
						fwd = fwd + (oc_viewmodel_move_fwd_bwd_change_speed_type0.GetFloat() * gpGlobals->frametime);
					else
						fwd = 1.0f;

					deltaFwd = (deltaFwd < 1.0f - (oc_viewmodel_move_fwd_bwd_change_speed_type0.GetFloat() * gpGlobals->frametime)) ? fwd : 1.0f;

					difPos = Lerp(deltaFwd, difPos, difPosM);
					vmangles = Lerp(deltaFwd, vmangles, vmanglesM);

					cvar->FindVar("oc_state_is_runningLAG")->SetValue(1);
					vmorigin = eyePosition + difPos;

					/*if (deltaFwd > 0.0f)
						cvar->FindVar("oc_state_is_running")->SetValue(1);*/
				}
				//Original
				else if ((speed < cvar->FindVar("hl2_sprintspeed")->GetInt() - 10))
				{
					if (deltaFwd > 0.0f)
						fwd = fwd - (oc_viewmodel_move_fwd_bwd_change_speed_type0.GetFloat() * gpGlobals->frametime);

					deltaFwd = (deltaFwd > 0.0f + (oc_viewmodel_move_fwd_bwd_change_speed_type0.GetFloat() * gpGlobals->frametime)) ? fwd : 0.0f;

					if ((fwd0 != 2) && (fwd0 != 3))
					{
						difPos = Lerp(deltaFwd, difPos, difPosM);
						vmangles = Lerp(deltaFwd, vmangles, vmanglesM);
					}
					if (fwd0 == 2)
					{
						difPos = Lerp(deltaFwd, difPos, difPosM);
						vmangles = Lerp(deltaFwd, vmangles, vmanglesM);
					}
					if (fwd0 == 3)
					{
						difPos = Lerp(deltaFwd, difPos, difPosM);
						vmangles = Lerp(deltaFwd, vmangles, vmanglesM);
					}
					cvar->FindVar("oc_state_is_runningLAG")->SetValue(0);
					vmorigin = eyePosition + difPos;

					if (deltaFwd == 0.0f)
					{
						fwd0 = 0;
						//cvar->FindVar("oc_state_is_running")->SetValue(0);
					}
				}			
			}
		}
		/*else
		{
			if (cvar->FindVar("oc_state_is_running")->GetInt())
				cvar->FindVar("oc_state_is_running")->SetValue(0);
		}*/
	}

	//DevMsg("vmangles yaw: %.2f\n vmangoriginal yaw: %.2f\n", vmangles[YAW], vmangoriginal[YAW]);

	SetLocalOrigin(vmorigin);
	SetLocalAngles(vmangles);

	//engine->ServerCmd(VarArgs("MuzzleAngles %f %f %f\n", vmangles.x, vmangles.y, vmangles.z), false);
	cvar->FindVar("oc_muzzle_angle_x")->SetValue(vmangles.x);
	cvar->FindVar("oc_muzzle_angle_y")->SetValue(vmangles.y);
	cvar->FindVar("oc_muzzle_angle_z")->SetValue(vmangles.z);


	cvar->FindVar("oc_RT_vector_x")->SetValue(RTOrigin.x);
	cvar->FindVar("oc_RT_vector_y")->SetValue(RTOrigin.y);
	cvar->FindVar("oc_RT_vector_z")->SetValue(RTOrigin.z);

#endif

}

ConVar oc_viewbob_vectorma_flspeed("oc_viewbob_vectorma_flspeed", "5.5", FCVAR_REPLICATED);
ConVar oc_viewbob_maxviewmodellag("oc_viewbob_maxviewmodellag", "3.0", FCVAR_REPLICATED);
ConVar oc_viewbob_scale("oc_viewbob_scale", "-0.017", FCVAR_REPLICATED);//-0.052
ConVar oc_viewbob_dest("oc_viewbob_dest", "1.3", FCVAR_REPLICATED);//2.5
ConVar oc_viewbob_scale2("oc_viewbob_scale2", "-0.052", FCVAR_REPLICATED);//-0.052
ConVar oc_viewbob_dest2("oc_viewbob_dest2", "1.3", FCVAR_REPLICATED);//2.5
ConVar oc_test_y("oc_test_y", "200.0", FCVAR_REPLICATED);

void CBaseViewModel::CalcViewModelLag(Vector& origin, QAngle& angles, QAngle& original_angles, int state, float lagScale)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	//bool wallBump = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0 : true;

	//DevMsg("mouseX: %i\n mouseY: %i\n", input->mouseX, input->mouseY);
	//DevMsg("GetLookSpring: %.2f\n Joystick_GetSide: %.2f\n", input->GetLookSpring(), input->Joystick_GetSide());

/*#if defined( CLIENT_DLL )
	DevMsg("mouseX: %.2f\n mouseY: %.2f\n", input->mouseX, input->mouseY);
#endif*/

	float lag = lagScale;

	lag = Clamp(lag, 0.02f, 1.0f);

	QAngle AnglesEyes = pPlayer->EyeAngles();
	Vector vOriginalOrigin = origin, origin2;
	QAngle vOriginalAngles = angles;
	Vector	forward;
	AngleVectors(angles, &forward, NULL, NULL);

	float g_fMaxViewModelLag = oc_viewbob_maxviewmodellag.GetFloat();
	float scale = oc_viewbob_scale.GetFloat();
	float scale2 = oc_viewbob_scale2.GetFloat();
	float dest = oc_viewbob_dest.GetFloat();
	float dest2 = oc_viewbob_dest2.GetFloat();

	Vector vDifference;

	float scaleConst = 5.0f;
	float diff = -1.0f;
	scaleConst = scaleConst*lag;
	diff = diff*lag;
	scale2 = scale2*lag;
	scale = scale*lag;
	dest2 = dest2*lag;
	dest = dest*lag;

	float dif = abs(AnglesEyes.x) * 0.01f;
	//DevMsg("dif: %.2f\n", dif);
	float destDif = 1.0f - dif;
	//DevMsg("dif dest: %.2f\n", destDif);

	dest2 *= destDif;
	if (!(abs(AnglesEyes.x) <= 86.0f))
	{
		dest2 *= 0.05f;
		//VectorMA(m_vecLastFacing, 0.f, vDifference*dest2, m_vecLastFacing);
		scale2 *= dest2;
		vDifference *= dest2;
	}

	if (state == 0)//Default
	{
		if (gpGlobals->frametime != 0.0f)
		{
			VectorSubtract(forward, m_vecLastFacing, vDifference);

			float flSpeed = (oc_viewbob_vectorma_flspeed.GetFloat());

			float flDiff = vDifference.Length();
			if ((flDiff > g_fMaxViewModelLag) && (g_fMaxViewModelLag > 0.0f))
			{
				float flScale = abs(flDiff / g_fMaxViewModelLag);
				flSpeed *= flScale;
			}

			scale2 = oc_viewbob_scale2.GetFloat();
			// FIXME:  Needs to be predictable?
			VectorMA(m_vecLastFacing, flSpeed * gpGlobals->frametime, vDifference, m_vecLastFacing);
			// Make sure it doesn't grow out of control!!!
			VectorNormalize(m_vecLastFacing);

			VectorMA(origin, scaleConst, vDifference * diff, origin);

			AngleVectors(angles, &origin2, NULL, NULL);
		}
	}
	else if (state == 1)
	{
		if (gpGlobals->frametime != 0.0f)
		{
			VectorSubtract(forward, m_vecLastFacing, vDifference);

			float flSpeed = (oc_viewbob_vectorma_flspeed.GetFloat());

			float flDiff = vDifference.Length();
			if ((flDiff > g_fMaxViewModelLag) && (g_fMaxViewModelLag > 0.0f))
			{
				float flScale = abs(flDiff / g_fMaxViewModelLag);
				flSpeed *= flScale;
			}

			if (!((AnglesEyes.x <= 88.0f) && (AnglesEyes.x >= -88.0f)))
			{
				flDiff = 0.f;
				flSpeed = 0.f;
			}

			scale2 = oc_viewbob_scale2.GetFloat();

			VectorMA(m_vecLastFacing, flSpeed * gpGlobals->frametime, vDifference, m_vecLastFacing);

			VectorNormalize(m_vecLastFacing);

			VectorMA(origin, -scale2, vDifference * dest2, origin);

			//if (((AnglesEyes.x <= 88.0f) && (AnglesEyes.x >= -88.0f)))
			{
				AngleVectors(angles, &origin2, NULL, NULL);
				VectorMA(origin2, -scale2, vDifference * dest2, origin2);
				VectorAngles(origin2, angles);

				/*QAngle angles2 = angles;

				AngleVectors(angles2, &origin2, NULL, &origin2);*/

				/*angles.x = angles2.x;
				angles.y = angles2.y;
				angles.z = angles2.z + 180;*/

				/*VectorMA(origin2, -scale2, vDifference * dest2, origin2);

				VectorAngles(origin2, angles);

				angles.x += 180;*/
			}
		}
	}
	else if (state == 2)
	{
		if (gpGlobals->frametime != 0.0f)
		{
			VectorSubtract(forward, m_vecLastFacing, vDifference);

			float flSpeed = (oc_viewbob_vectorma_flspeed.GetFloat());

			float flDiff = vDifference.Length();
			if ((flDiff > g_fMaxViewModelLag) && (g_fMaxViewModelLag > 0.0f))
			{
				float flScale = abs(flDiff / g_fMaxViewModelLag);
				flSpeed *= flScale;
			}

			if (!((AnglesEyes.x <= 88.0f) && (AnglesEyes.x >= -88.0f)))
			{
				flDiff = 0.f;
				flSpeed = 0.f;
			}

			scale = oc_viewbob_scale.GetFloat();

			VectorMA(m_vecLastFacing, flSpeed * gpGlobals->frametime, vDifference, m_vecLastFacing);

			VectorNormalize(m_vecLastFacing);

			VectorMA(origin, scale2, vDifference * dest2, origin);

			//if (((AnglesEyes.x <= 88.0f) && (AnglesEyes.x >= -88.0f)))
			{
				AngleVectors(angles, &origin2, NULL, NULL);

				VectorMA(origin2, scale2, vDifference * dest2, origin2);

				VectorAngles(origin2, angles);
			}
		}
	}

	Vector right, up;
	AngleVectors(original_angles, &forward, &right, &up);

	float pitch = original_angles[PITCH];
	if (pitch > 180.0f)
		pitch -= 360.0f;
	else if (pitch < -180.0f)
		pitch += 360.0f;

	if (g_fMaxViewModelLag == 0.0f)
	{
		origin = vOriginalOrigin;
		angles = vOriginalAngles;
	}

	VectorMA(origin, -pitch * 0.001f, forward, origin);
	//VectorMA(origin, -pitch * 0.001f, right, origin);

	if (cvar->FindVar("oc_player_draw_body")->GetInt())
		VectorMA(origin, pitch * 0.025f, up, origin);
	else
		VectorMA(origin, -pitch * 0.001f, up, origin);

}


//-----------------------------------------------------------------------------
// Stub to keep networking consistent for DEM files
//-----------------------------------------------------------------------------
#if defined( CLIENT_DLL )
  extern void RecvProxy_EffectFlags( const CRecvProxyData *pData, void *pStruct, void *pOut );
  void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut );
#endif

//-----------------------------------------------------------------------------
// Purpose: Resets anim cycle when the server changes the weapon on us
//-----------------------------------------------------------------------------
#if defined( CLIENT_DLL )
static void RecvProxy_Weapon( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *pViewModel = ((CBaseViewModel*)pStruct);
	CBaseCombatWeapon *pOldWeapon = pViewModel->GetOwningWeapon();

	// Chain through to the default recieve proxy ...
	RecvProxy_IntToEHandle( pData, pStruct, pOut );

	// ... and reset our cycle index if the server is switching weapons on us
	CBaseCombatWeapon *pNewWeapon = pViewModel->GetOwningWeapon();
	if ( pNewWeapon != pOldWeapon )
	{
		// Restart animation at frame 0
		pViewModel->SetCycle( 0 );
		pViewModel->m_flAnimTime = gpGlobals->curtime;
	}
}
#endif


LINK_ENTITY_TO_CLASS( viewmodel, CBaseViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( BaseViewModel, DT_BaseViewModel )

BEGIN_NETWORK_TABLE_NOBASE(CBaseViewModel, DT_BaseViewModel)
#if !defined( CLIENT_DLL )
	SendPropModelIndex(SENDINFO(m_nModelIndex)),
	SendPropInt		(SENDINFO(m_nBody), 8),
	SendPropInt		(SENDINFO(m_nSkin), 10),
	SendPropInt		(SENDINFO(m_nSequence),	8, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_nViewModelIndex), VIEWMODEL_INDEX_BITS, SPROP_UNSIGNED),
	SendPropFloat	(SENDINFO(m_flPlaybackRate),	8,	SPROP_ROUNDUP,	-4.0,	12.0f),
	SendPropInt		(SENDINFO(m_fEffects),		10, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_nAnimationParity), 3, SPROP_UNSIGNED ),
	SendPropEHandle (SENDINFO(m_hWeapon)),
	SendPropEHandle (SENDINFO(m_hOwner)),

	SendPropInt( SENDINFO( m_nNewSequenceParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nResetEventsParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nMuzzleFlashParity ), EF_MUZZLEFLASH_BITS, SPROP_UNSIGNED ),

	//SendPropFloat	(SENDINFO(tracerOrig_X)),
	//SendPropFloat	(SENDINFO(tracerOrig_Y)),
	//SendPropFloat	(SENDINFO(tracerOrig_Z)),
	SendPropBool(SENDINFO(m_bExpSighted)),
	SendPropBool(SENDINFO(m_bExpScope)),
	SendPropBool(SENDINFO(m_bIsLeg)),
	SendPropBool(SENDINFO(m_bDefRenderGroup)),
	SendPropBool(SENDINFO(IsNearWall)),

#if !defined( INVASION_DLL ) && !defined( INVASION_CLIENT_DLL )
	SendPropArray	(SendPropFloat(SENDINFO_ARRAY(m_flPoseParameter),	8, 0, 0.0f, 1.0f), m_flPoseParameter),
#endif
#else
	RecvPropInt		(RECVINFO(m_nModelIndex)),
	RecvPropInt		(RECVINFO(m_nSkin)),
	RecvPropInt		(RECVINFO(m_nBody)),
	RecvPropInt		(RECVINFO(m_nSequence), 0, RecvProxy_SequenceNum ),
	RecvPropInt		(RECVINFO(m_nViewModelIndex)),
	RecvPropFloat	(RECVINFO(m_flPlaybackRate)),
	RecvPropInt		(RECVINFO(m_fEffects), 0, RecvProxy_EffectFlags ),
	RecvPropInt		(RECVINFO(m_nAnimationParity)),
	RecvPropEHandle (RECVINFO(m_hWeapon), RecvProxy_Weapon ),
	RecvPropEHandle (RECVINFO(m_hOwner)),

	RecvPropInt( RECVINFO( m_nNewSequenceParity )),
	RecvPropInt( RECVINFO( m_nResetEventsParity )),
	RecvPropInt( RECVINFO( m_nMuzzleFlashParity )),

	//RecvPropFloat(RECVINFO(tracerOrig_X)),
	//RecvPropFloat(RECVINFO(tracerOrig_Y)),
	//RecvPropFloat(RECVINFO(tracerOrig_Z)),
	RecvPropBool(RECVINFO(m_bExpSighted)),
	RecvPropBool(RECVINFO(m_bExpScope)),
	RecvPropBool(RECVINFO(m_bIsLeg)),
	RecvPropBool(RECVINFO(m_bDefRenderGroup)),
	RecvPropBool(RECVINFO(IsNearWall)),

#if !defined( INVASION_DLL ) && !defined( INVASION_CLIENT_DLL )
	RecvPropArray(RecvPropFloat(RECVINFO(m_flPoseParameter[0]) ), m_flPoseParameter ),
#endif
#endif

END_NETWORK_TABLE()

/*#ifndef CLIENT_DLL
BEGIN_PREDICTION_DATA(CBaseViewModel)
	DEFINE_FIELD(m_bExpSighted, FIELD_BOOLEAN)
END_PREDICTION_DATA()
#endif*/

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CBaseViewModel )

	// Networked
	DEFINE_PRED_FIELD( m_nModelIndex, FIELD_SHORT, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_nSkin, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nBody, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.125f ),
	DEFINE_PRED_FIELD( m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_OVERRIDE ),
	DEFINE_PRED_FIELD( m_nAnimationParity, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hWeapon, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flAnimTime, FIELD_FLOAT, 0 ),

	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT ),
	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_PRED_FIELD(m_bExpSighted, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bExpScope, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bIsLeg, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(IsNearWall, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_PRIVATE | FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),

	//DEFINE_PRED_FIELD(tracerOrig_X, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	//DEFINE_PRED_FIELD(tracerOrig_Y, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	//DEFINE_PRED_FIELD(tracerOrig_Z, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *model = (CBaseViewModel *)pStruct;
	if (pData->m_Value.m_Int != model->GetSequence())
	{
		MDLCACHE_CRITICAL_SECTION();

		model->SetSequence(pData->m_Value.m_Int);
		model->m_flAnimTime = gpGlobals->curtime;
		model->SetCycle(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBaseViewModel::LookupAttachment( const char *pAttachmentName )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->LookupAttachment( pAttachmentName );

	return BaseClass::LookupAttachment( pAttachmentName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, matrix3x4_t &matrix )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, matrix );

	return BaseClass::GetAttachment( number, matrix );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, Vector &origin )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, origin );

	return BaseClass::GetAttachment( number, origin );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachment( number, origin, angles );

	return BaseClass::GetAttachment( number, origin, angles );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseViewModel::GetAttachmentVelocity( int number, Vector &originVel, Quaternion &angleVel )
{
	if ( m_hWeapon.Get() && m_hWeapon.Get()->WantsToOverrideViewmodelAttachments() )
		return m_hWeapon.Get()->GetAttachmentVelocity( number, originVel, angleVel );

	return BaseClass::GetAttachmentVelocity( number, originVel, angleVel );
}

#endif
