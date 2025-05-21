//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "overcharged/OVR_VGUI/DataParser.h"
#include "vgui_controls/RadioButton.h"
#include "OVRPostProcessing.h"
#include "OVROptions.h"
using namespace vgui;



static ConVarList cvarPPList1[] =
{
	{ _CBOX, 0, 0, 1, "r_post_processing", "#GameUI_EnablePP" },
	{ _CBOX, 0, 0, 1, "r_post_processing_special", "r_post_processing" },
	{ _CBOX, 0, 0, 1, "r_post_draw_chromatic_abberations", "#GameUI_ChromaticAbberations" },
	{ _CBOX, 0, 0, 1, "r_post_draw_bloom", "#GameUI_Bloom" },
	{ _CBOX, 0, 0, 1, "r_post_draw_underwater", "#GameUI_UnderWater" },
	{ _SLIDER, 75, 175, 1, "r_post_draw_underwater_idelay", "#OverUI_UnderWaterDelayDesc" },
	{ _SLIDER, 0, 5, 1, "r_post_draw_underwater_ispeed", "#OverUI_UnderWaterSpeedDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_toon", "#GameUI_Toon" },
	{ _CBOX, 0, 0, 1, "r_post_draw_flare_naive", "#GameUI_FlareNaive" },
	{ _CBOX, 0, 0, 1, "r_post_draw_sunrays", "#GameUI_SunRays" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_sunrays_power", "#OverUI_SunRaysPowerDesc" },
	{ _SLIDER, 0, 10, 1, "r_post_draw_sunrays_amount", "#OverUI_SunRaysAmountDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_fxaa", "#GameUI_Fxaa" },
	{ _CBOX, 0, 0, 1, "r_post_draw_flare_anamorphic", "#GameUI_Anamorph" },
	{ _CBOX, 0, 0, 1, "r_post_draw_cinematic_overlay", "#GameUI_CC_colorful" },
	{ _CBOX, 0, 0, 1, "r_post_gears_bloom", "#GameUI_Bloom2" },
	{ _CBOX, 0, 0, 1, "r_post_draw_bokeh_dof", "#GameUI_BokehDOF" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ssao_cr", "#GameUI_SSAO" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ssao_cr_blur", "r_post_draw_ssao_cr" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ssao_cr_combined", "r_post_draw_ssao_cr" },
	{ _CBOX, 0, 0, 1, "oc_global_lightning_enabled", "#GameUI_GPL" },
	{ _SLIDER, 0, 10, 0.1f, "oc_global_lightning_shadow_filter_size", "#OverUI_shadowfiltersize" },
	{ _SLIDER, 0, 100, 1, "oc_global_lightning_update_freq", "#OverUI_suf" },
	{ _CBOX, 0, 0, 1, "r_shader_srgb", "#GameUI_Shader_SRGB" },
	{ _CBOX, 0, 0, 1, "r_drawdetailprops", "#GameUI_DrawDetailProps" },
	{ _CBOX, 0, 0, 1, "r_pixelfog", "#GameUI_AdditionalFog" },
	{ _SLIDER, 0, 500, 1, "r_maxmodeldecal", "#OverUI_MaxMDLDecalsDesc" },
	{ _SLIDER, 0, 15000, 1, "cl_detaildist", "#OverUI_MaxDistDetailRender" },
};

#define PPListSize1		(ARRAYSIZE(cvarPPList1))

static ConVarList cvarPPList2[] =
{

	{ _CBOX, 0, 0, 1, "r_post_draw_dragonborn", "#GameUI_CC_DragonBurn" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ironsight_dof", "#GameUI_IronSightDOF" },
	{ _CBOX, 0, 0, 1, "r_post_draw_natural", "#GameUI_CC_natural" },
	{ _CBOX, 0, 0, 1, "r_post_draw_hurt", "#GameUI_Hurt" },
	{ _CBOX, 0, 0, 1, "r_post_draw_pain", "#GameUI_Pain" },
	{ _SLIDER, 0, 3, 1, "r_post_draw_pain_ispeed", "#OverUI_PainSpeedDesc" },
	{ _SLIDER, 0, 5, 1, "r_post_draw_pain_idelay", "#OverUI_PainDelayDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_suit_regenerate", "#GameUI_EnergyRegen" },
	{ _SLIDER, 0, 35, 1, "r_post_draw_suit_regenerate_delay", "#OverUI_EnergyRegenDelay" },
	{ _SLIDER, 0, 1, 0.1f, "r_post_draw_suit_regenerate_grain_inensity", "#OverUI_EnergRegenGrainIntenst" },
	{ _SLIDER, 0, 1, 0.1f, "r_post_draw_suit_regenerate_inensity", "#OverUI_EnergRegenLightIntenst" },
	{ _CBOX, 0, 0, 1, "r_post_draw_health_regenerate", "#GameUI_HealthRegen" },
	{ _SLIDER, 0, 35, 1, "r_post_draw_health_delay", "#OverUI_HealthDelayDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_health", "#GameUI_Health" },
	{ _CBOX, 0, 0, 1, "r_post_draw_colorgrading", "#GameUI_CC_colorgrading" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ironsight_screenblur", "#GameUI_IronSightBlur" },
	{ _CBOX, 0, 0, 1, "r_post_draw_screen_dust", "#GameUI_ScreenDirt" },
	{ _CBOX, 0, 0, 1, "r_post_draw_desaturation", "#GameUI_Desaturation" },
	{ _CBOX, 0, 0, 1, "r_post_draw_filmgrain", "#GameUI_FilmGrain" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_filmgrain_grain_inensity", "#OverUI_FilmGrainIntensDesc" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_filmgrain_light_inensity", "#OverUI_FilmLightIntensDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_colorgrading_simple", "#GameUI_CC_colorgrading_simple" },
	{ _CBOX, 0, 0, 1, "r_post_draw_saturation", "#GameUI_CC_saturation" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_saturation_intensity", "#OverUI_SaturationIntensDesc" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_saturation_brightness", "#OverUI_SaturationBrightDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_bullettime", "#GameUI_BulletTime" },
	{ _CBOX, 0, 0, 1, "r_post_draw_InReload_dof", "#GameUI_ReloadBlur" },
	{ _CBOX, 0, 0, 1, "r_post_draw_InReload_dof2", "#GameUI_IronSightBlur3" },
	{ _CBOX, 0, 0, 1, "r_post_draw_ironsight_blur", "#GameUI_IronSightBlur1" },
};

#define PPListSize2		(ARRAYSIZE(cvarPPList2))

static ConVarList cvarPPList3[] =
{
	{ _CBOX, 0, 0, 1, "r_post_draw_battery", "#GameUI_Battery" },
	{ _SLIDER, 0, 35, 1, "r_post_draw_battery_delay", "#OverUI_BatteryDelayDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_health", "#GameUI_Health" },
	{ _SLIDER, 0, 35, 1, "r_post_draw_health_delay", "#OverUI_HealthDelayDesc" },
	{ _CBOX, 0, 0, 1, "r_post_draw_crossprocessing", "#GameUI_CC_crossprocessing" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_crossprocessing_saturation", "#OverUI_CPSaturDesc" },
	{ _SLIDER, 0, 75, 0.01f, "r_post_draw_crossprocessing_intensity", "#OverUI_CPIntensDesc" },
	{ _SLIDER, 0, 10, 0.1f, "r_post_draw_crossprocessing_contrast", "#OverUI_CPContrastDesc" },
	{ _SLIDER, 0, 5, 0.1f, "r_post_draw_crossprocessing_brightness", "#OverUI_CPBrightDesc" },
};

#define PPListSize3		(ARRAYSIZE(cvarPPList3))
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COVRPostProcessing::COVRPostProcessing(vgui::Panel *parent, int num) : BaseClass(parent, NULL)
{
	//COVROptions *pOptions = (COVROptions*)parent;

	switch (num)
	{
		case 0:
		{
			FillGrid(this, ppButtons, pPPSliders, pTexts, cvarPPList1, PPListSize1);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsPostProcessing_01.res");
		}
		break;
		case 1:
		{
			FillGrid(this, ppButtons, pPPSliders, pTexts, cvarPPList2, PPListSize2);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsPostProcessing_02.res");
		}
		break;
		case 2:
		{
			FillGrid(this, ppButtons, pPPSliders, pTexts, cvarPPList3, PPListSize3);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsPostProcessing_03.res");
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: resets controls
//-----------------------------------------------------------------------------
void COVRPostProcessing::OnResetData()
{

}

//-----------------------------------------------------------------------------
// Purpose: sets data based on control settings
//-----------------------------------------------------------------------------
void COVRPostProcessing::OnApplyChanges(int num)
{
	switch (num)
	{
	case 0:
		ApplyGrid(ppButtons, pPPSliders, pTexts, cvarPPList1, PPListSize1);
		break;
	case 1:
		ApplyGrid(ppButtons, pPPSliders, pTexts, cvarPPList2, PPListSize2);
		break;
	case 2:
		ApplyGrid(ppButtons, pPPSliders, pTexts, cvarPPList3, PPListSize3);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: enables apply button on radio buttons being pressed
//-----------------------------------------------------------------------------
void COVRPostProcessing::OnRadioButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}