//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "OVRGameplay.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "vgui_controls/RadioButton.h"
#include "overcharged/OVR_VGUI/DataParser.h"
#include "OVROptions.h"
using namespace vgui;

static ConVarList cvarGPList1[] =
{
	{ _CBOX, 0, 0, 1, "oc_ally_allow_friendlyfire", "#GameUI_FriendlyFire" },
	{ _CBOX, 0, 0, 1, "oc_ragdoll_enable_blooddrips", "#OverUI_RagdollBloodDrips" },
	{ _CBOX, 0, 0, 1, "oc_ragdoll_enable_bloodstream", "#OverUI_RagdollBloodStream" },
	{ _SLIDER, 0, 128, 1, "oc_blood_drip_maxcount", "#OverUI_BloodDripMaxCount" },
	{ _SLIDER, 25, 255, 1, "oc_sporegenerator_projectile_impulse", "#OverUI_SGImpulse" },
	{ _CBOX, 0, 0, 1, "npc_citizen_auto_player_squad", "#OverUI_AutoSquadJoin" },
	{ _CBOX, 0, 0, 1, "cl_righthand", "#OverUI_DifferentHandsDesc" },
	{ _CBOX, 0, 0, 1, "oc_playerview_draw_body", "#OverUI_RenderBodyDeath" },
	{ _SLIDER, 0, 10, 0.01f, "oc_hydra_runtime", "#OverUI_HydraRunTimeDesc" },
	{ _CBOX, 0, 0, 1, "oc_npc_zombies_dont_less_headcrab", "#GameUI_ZombiesHeadcrab" },
	{ _CBOX, 0, 0, 1, "sk_voltigore_allow_extra_electric_throw", "#OverUI_VoltExtraThrow" },
	{ _CBOX, 0, 0, 1, "oc_player_death_sound", "#OverUI_Playerdeathsound" },
	{ _TEXT, 0, 0, 1, "oc_playermodel_nosuit", "#OverUI_PlayerModelWithoutSuitDesc" },
	{ _TEXT, 0, 0, 1, "oc_playermodel_suit", "#OverUI_PlayerModelWithSuitDesc" },
	{ _CBOX, 0, 0, 1, "oc_ragdoll_show_blood", "#OverUI_RagdollBlood" },
	{ _CBOX, 0, 0, 1, "oc_npc_allow_jumping", "#OverUI_NPCJump" },
};

#define GPListSize1	(ARRAYSIZE(cvarGPList1))

static ConVarList cvarGPList2[] =
{
	{ _SLIDER, 5, 13, 0.1f, "oc_state_slomo_drain_rate", "#OverUI_SlowmotionDrain" },
	{ _CBOX, 0, 0, 1, "cl_drawhud", "#OverUI_HudVisibility" },
	{ _CBOX, 0, 0, 1, "crosshair", "#OverUI_CrosshairVisibility" },
	{ _CBOX, 0, 0, 1, "hud_drawhistory_time", "#OverUI_PickupHistoryTimeDesc" },
	{ _CBOX, 0, 0, 1, "hud_showemptyweaponslots", "#OverUI_ShowEmptyWeaponSlots" },
	{ _TEXT, 0, 0, 1, "oc_mapadd_preset", "#OverUI_mapaddpresetlist" },
	{ _SLIDER, 25, 100, 1, "oc_mapadd_randomspawn_safe_distance", "#OverUI_mapaddRSRadiusDesc" },
	{ _TEXT, 0, 0, 1, "oc_npc_assassin_model", "#OverUI_assassinfmodel" },
	{ _SLIDER, 0, 13, 0.1f, "r_post_draw_nightvision_minlighting", "#OverUI_ShowEmptyWeaponSlots" },
	{ _CBOX, 0, 0, 1, "oc_weapon_disable_dissolve", "#OverUI_Disabledissolve" },
	{ _SLIDER, 0, 1, 1, "oc_playerview_attach_deathcam_", "#OverUI_AttachDeathcam" },

};

#define GPListSize2	(ARRAYSIZE(cvarGPList2))

static ConVarList cvarGPList3[] =
{
	{ _CBOX, 0, 0, 1, "oc_player_flashlight_onweapon", "#OverUI_FlashLightAttach" },
	{ _CBOX, 0, 0, 1, "oc_weapons_enable_dynamic_bullets", "#OverUI_DynamicBullets" },
	{ _CBOX, 0, 0, 1, "oc_weapons_enable_dynamic_bullets_penetration", "#OverUI_DynamicBulletsPenetration" },
	{ _SLIDER, 0, 10, 1, "oc_ragdoll_serverside_collision_group", "#OverUI_ServersideRagdollsCollisionGroup" },
	{ _SLIDER, 2, 5000, 0.01f, "oc_ragdoll_serverside_dissolve_impulse", "#OverUI_ssdissolve" },
	{ _SLIDER, 0, 12, 1, "oc_ragdoll_death_weapon_holding", "#OverUI_DeadweaponRagdollsHolding" },
	{ _CBOX, 0, 0, 1, "oc_ragdoll_death_weapon_firing", "#OverUI_DeadweaponRagdollsFiring" },
	{ _CBOX, 0, 0, 1, "oc_ragdoll_serverside", "#OverUI_ServersideRagdolls" },
	{ _CBOX, 0, 0, 1, "oc_player_bloodoverlay", "#OverUI_Bloodoverlay" },
	{ _SLIDER, 2, 50, 1, "oc_ragdoll_death_weapon_holding_time", "#OverUI_deathweaponholdingtime" },
	{ _CBOX, 0, 0, 1, "npc_citizen_auto_player_squad_allow_use", "#OverUI_UseSquadJoin" },
	{ _CBOX, 0, 0, 1, "oc_weapons_allow_wall_bump", "#OverUI_weaponwallbump" },
	{ _CBOX, 0, 0, 1, "oc_weapons_allow_sprint_lower", "#OverUI_weaponsprintlower" },
	{ _SLIDER, 0, 2, 1, "oc_viewmodel_lag_type", "#OverUI_vmlagtype012" },
	{ _SLIDER, 0, 2, 1, "oc_viewmodel_move_wall_type", "#OverUI_VMWType" },
	{ _SLIDER, 10, 180, 1, "oc_player_bloodoverlay_lifetime", "#OverUI_Bloodoverlaylifetime" },
	{ _SLIDER, 2, 30, 0.1f, "oc_player_garbage_drop_delay", "#OverUI_Garbagedropdelayy" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_drop_used_perks", "#GameUI_DropPerks" },
};

#define GPListSize3	(ARRAYSIZE(cvarGPList3))

static ConVarList cvarGPList4[] =
{
	{ _SLIDER, 0, 50, 0.1f, "oc_viewbob_walk_scale", "#OverUI_ViewBobWalkScale" },
	{ _SLIDER, 0, 50, 0.1f, "oc_viewbob_walk_dist", "#OverUI_ViewBobWalkDist" },
	{ _CBOX, 0, 0, 1, "oc_viewmodel_bob_enable", "#OverUI_Viewmdlbob" },
	{ _CBOX, 0, 0, 1, "cl_viewbob_enabled", "#OverUI_ViewBob" },
	{ _SLIDER, 1, 30, 0.1f, "oc_viewbob_speed", "#OverUI_ViewBobRunSpeed" },
	{ _SLIDER, 1, 25, 0.1f, "oc_viewbob_speed_walk", "#OverUI_ViewBobWalkSpeed" },
	{ _SLIDER, 0, 10, 1, "oc_viewbob_vol_walk", "#OverUI_ViewBobWalkVolume" },
	{ _SLIDER, 0, 10, 1, "oc_viewbob_vol", "#OverUI_ViewBobRunVolume" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_fast_gren_throw", "#OverUI_FastThrow" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_kick", "#OverUI_AllowKick" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_kick_doors", "#OverUI_AllowKick" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_swing", "#OverUI_AllowSwing" },
	{ _CBOX, 0, 0, 1, "oc_player_camera_landing_bob", "#OverUI_Landingbob" },
	{ _SLIDER, 0, 40, 0.1f, "oc_player_animated_camera_scale", "#OverUI_animcamerscale" },
	{ _CBOX, 0, 0, 1, "oc_player_allow_animated_camera", "#OverUI_Allowanimcam" },
};

#define GPListSize4	(ARRAYSIZE(cvarGPList4))

COVRGameplay::COVRGameplay(vgui::Panel *parent, int num) : BaseClass(parent, NULL)
{
	//COVROptions *pOptions = (COVROptions*)parent;

	switch (num)
	{
		case 0:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList1, GPListSize1);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGameplay_01.res");
		}
		break;
		case 1:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList2, GPListSize2);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGameplay_02.res");
		}
		break;
		case 2:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList3, GPListSize3);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGameplay_03.res");
		}
		break;
		case 3:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList4, GPListSize4);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGameplay_04.res");
		}
		break;
	}
}

void COVRGameplay::OnResetData()
{

}

void COVRGameplay::OnApplyChanges(int num)
{
	switch (num)
	{
	case 0:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList1, GPListSize1);
		break;
	case 1:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList2, GPListSize2);
		break;
	case 2:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList3, GPListSize3);
		break;
	case 3:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList4, GPListSize4);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: enables apply button on radio buttons being pressed
//-----------------------------------------------------------------------------
void COVRGameplay::OnRadioButtonChecked()
{
	//PostActionSignal(new KeyValues("ApplyButtonEnable"));
}
