#include "cbase.h"
#include "viewrender.h"
#include "view_scene.h"
#include "screen_overlay_multiple.h"//OverCharged
#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "rendertexture.h"
#include "ivieweffects.h"
#include "ClientEffectPrecacheSystem.h"
#include "view.h"
#include "shadereditor/shadereditorsystem.h"
#include "shadereditor/ivshadereditor.h"
#include "basecombatweapon_shared.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
//=================================================================================
#define SCREENBLUR_EDITOR_NAME "ppe_gaussian_blur"//Определяем что за шейдер
#define OVERLAY_EDITOR_NAME "ppe_dragonborn"
#define SSAO_EDITOR_NAME "ppe_ssao"
#define SUNRAYS_EDITOR_NAME "ppe_sunrays"
#define SUNRAYS2_EDITOR_NAME "ppe_sunrays2"
#define SUNRAYS3_EDITOR_NAME "ppe_sunrays3"
#define FLARE_A_EDITOR_NAME "ppe_flare_anamorphic"
//=================================================================================
//Тут очень много душераздирающего кода////////////////////////////////////////////
//=================================================================================
ConVar r_post_processing("r_post_processing", "0", FCVAR_ARCHIVE);//OverCharged
//=================================================================================
//extern ConVar drawDOFInReload;
//=================================================================================
ConVar r_post_draw_ironsight_screenblur("r_post_draw_ironsight_screenblur", "0", FCVAR_ARCHIVE);//Читы на включение шейдеров
ConVar r_post_draw_dragonborn("r_post_draw_dragonborn", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_chromatic_abberations("r_post_draw_chromatic_abberations", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_ironsight_dof("r_post_draw_ironsight_dof", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_natural("r_post_draw_natural", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_bloom("r_post_draw_bloom", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_hurt("r_post_draw_hurt", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_pain("r_post_draw_pain", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_pain_ispeed("r_post_draw_pain_ispeed", "0.8", FCVAR_ARCHIVE);
ConVar r_post_draw_pain_idelay("r_post_draw_pain_idelay", "1.3", FCVAR_ARCHIVE);
ConVar r_post_draw_underwater("r_post_draw_underwater", "0", FCVAR_ARCHIVE);
//ConVar r_post_draw_underwater_ispeed("r_post_draw_underwater_ispeed", "75", FCVAR_ARCHIVE);
//ConVar r_post_draw_underwater_idelay("r_post_draw_underwater_idelay", "100", FCVAR_ARCHIVE);
ConVar r_post_draw_suit_regenerate("r_post_draw_suit_regenerate", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_health_regenerate("r_post_draw_health_regenerate", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_suit_regenerate_delay("r_post_draw_suit_regenerate_delay", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_health_regenerate_inensity("r_post_draw_health_regenerate_inensity", "0.15", FCVAR_ARCHIVE);
ConVar r_post_draw_suit_regenerate_grain_inensity("r_post_draw_suit_regenerate_grain_inensity", "0.5", FCVAR_ARCHIVE);
ConVar r_post_draw_suit_regenerate_inensity("r_post_draw_suit_regenerate_inensity", "0.15", FCVAR_ARCHIVE);
ConVar r_post_draw_battery("r_post_draw_battery", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_battery_delay("r_post_draw_battery_delay", "20", FCVAR_ARCHIVE);
ConVar r_post_draw_health("r_post_draw_health", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_health_delay("r_post_draw_health_delay", "15", FCVAR_ARCHIVE);
ConVar r_post_draw_toon("r_post_draw_toon", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_colorgrading("r_post_draw_colorgrading", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_flare_naive("r_post_draw_flare_naive", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_flare_naive_eyes("r_post_draw_flare_naive_eyes", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_flare_naive_eyes2("r_post_draw_flare_naive_eyes2", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_sunrays("r_post_draw_sunrays", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_sunrays_colorR("r_post_draw_sunrays_color.r", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_sunrays_colorG("r_post_draw_sunrays_color.g", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_sunrays_colorB("r_post_draw_sunrays_color.b", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_sunrays_colorA("r_post_draw_sunrays_color.a", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_sunrays_power("r_post_draw_sunrays_power", "0.04", FCVAR_ARCHIVE);

ConVar r_post_draw_sunrays_amount("r_post_draw_sunrays_amount", "1", FCVAR_ARCHIVE);

ConVar r_post_draw_screen_dust("r_post_draw_screen_dust", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_fxaa("r_post_draw_fxaa", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_ssao("r_post_draw_ssao", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_flare_anamorphic("r_post_draw_flare_anamorphic", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_desaturation("r_post_draw_desaturation", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_filmgrain("r_post_draw_filmgrain", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_filmgrain_grain_inensity("r_post_draw_filmgrain_grain_inensity", "0.15", FCVAR_ARCHIVE);
ConVar r_post_draw_filmgrain_light_inensity("r_post_draw_filmgrain_light_inensity", "0.8", FCVAR_ARCHIVE);

ConVar r_post_draw_colorgrading_simple("r_post_draw_colorgrading_simple", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_crossprocessing("r_post_draw_crossprocessing", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_crossprocessing_saturation("r_post_draw_crossprocessing_saturation", "0.8", FCVAR_ARCHIVE);
ConVar r_post_draw_crossprocessing_contrast("r_post_draw_crossprocessing_contrast", "1.0", FCVAR_ARCHIVE);
ConVar r_post_draw_crossprocessing_brightness("r_post_draw_crossprocessing_brightness", "0.0", FCVAR_ARCHIVE);
ConVar r_post_draw_crossprocessing_intensity("r_post_draw_crossprocessing_intensity", "0.2", FCVAR_ARCHIVE);

ConVar r_post_draw_cinematic_overlay("r_post_draw_cinematic_overlay", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_saturation("r_post_draw_saturation", "0", FCVAR_ARCHIVE);

ConVar r_post_gears_bloom("r_post_gears_bloom", "0", FCVAR_ARCHIVE);

ConVar r_post_draw_saturation_brightness("r_post_draw_saturation_brightness", "0.0", FCVAR_ARCHIVE);
ConVar r_post_draw_saturation_intensity("r_post_draw_saturation_intensity", "0.02", FCVAR_ARCHIVE);
ConVar r_post_draw_bullettime("r_post_draw_bullettime", "0", FCVAR_ARCHIVE);
//=================================================================================

//=================================================================================
class C_ViewRender_Overwrite : public CViewRender {
	DECLARE_CLASS(C_ViewRender_Overwrite, CViewRender);
public:
	C_ViewRender_Overwrite::C_ViewRender_Overwrite();

	virtual void RenderView(const CViewSetup &view, int nClearFlags, int whatToDraw);

	void	Post_Processing_Effects(int x, int y, int width, int height);//OverCharged
	void	DrawSunRays(int x, int y, int width, int height);//OverCharged
	void	DrawScreenGaussianBlur(int x, int y, int width, int height);//OverCharged
	void	DrawScreenGaussianBlurSlomo(int x, int y, int width, int height);//OverCharged
//	void	DrawScreenGaussianBlurReload(int x, int y, int width, int height);//OverCharged
	void	DrawSSAO(int x, int y, int width, int height);//OverCharged
	void	DrawFlareAnamorphic(int x, int y, int width, int height);//OverCharged
	void	DrawScope(const CViewSetup &cameraView);//OverCharged

public:
	float indicate = r_post_draw_pain_idelay.GetFloat();
	float indicate2 = r_post_draw_pain_idelay.GetFloat();
	float indicateU = 0;//r_post_draw_underwater_idelay.GetFloat();
	float indicateE = 0.0f;//r_post_draw_suit_regenerate_delay.GetFloat();
	float indicateH = 0.2f;//r_post_draw_suit_regenerate_delay.GetFloat();
	float indicateHE = 0.0f;//r_post_draw_suit_regenerate_delay.GetFloat();
	float Tick = 0.0f;
	float TickU = 0.0f;
	float Health;
	bool underwater = false;
};
//=================================================================================
//=================================================================================
static C_ViewRender_Overwrite g_ViewRender;
C_ViewRender_Overwrite::C_ViewRender_Overwrite() {
	if (!view) {
		view = (IViewRender *)&g_ViewRender;
	}
}

void C_ViewRender_Overwrite::RenderView(const CViewSetup &view, int nClearFlags, int whatToDraw)
{
	BaseClass::RenderView(view, nClearFlags, whatToDraw);

	if (!shaderEdit)
	{
		if (cvar->FindVar("r_post_processing")->GetInt())
			cvar->FindVar("r_post_processing")->SetValue(0);

		if (cvar->FindVar("r_post_processing_special")->GetInt())
			cvar->FindVar("r_post_processing_special")->SetValue(0);

		return;
	}
	/////////////////OverCharged POSTPROCESSING EFFECTS/////////////////////////////////////

	DrawOverlaysForMode(CScreenoverlayMulti::RENDERMODE_POST_HDR,
		view.x, view.y, view.width, view.height);

	Post_Processing_Effects(view.x, view.y, view.width, view.height);


	////////////////////////////////////////////////////////////////////////////////////////
}
//=================================================================================


//=================================================================================
/////////////////////////////////////////////////POSTPROCESS EFFECTS////////////////////////////////////////////////////



CLIENTEFFECT_REGISTER_BEGIN(PrecachePostProcessingMaterials)
//postprocessing materials
CLIENTEFFECT_MATERIAL("effects/shaders/blurx")
CLIENTEFFECT_MATERIAL("effects/shaders/blury")
CLIENTEFFECT_MATERIAL("effects/shaders/sunshaft_base")
CLIENTEFFECT_MATERIAL("effects/shaders/sunshaft_final")
CLIENTEFFECT_MATERIAL("effects/shaders/dust")
CLIENTEFFECT_MATERIAL("effects/shaders/cross_processing_new")
CLIENTEFFECT_MATERIAL("effects/shaders/cross_processing")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bokeh")
CLIENTEFFECT_MATERIAL("effects/shaders/colorgrading_simple")
CLIENTEFFECT_MATERIAL("effects/shaders/scope_sniper")
CLIENTEFFECT_MATERIAL("effects/shaders/scope_crossbow")
CLIENTEFFECT_MATERIAL("effects/shaders/scope_oicw")
CLIENTEFFECT_MATERIAL("effects/shaders/scope_sg552")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_cinematic_overlay")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_cinematic_overlay_saturation")
CLIENTEFFECT_MATERIAL("effects/shaders/postprocess_filmgrain")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_dragonborn")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_chromatic_abberations")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_desaturation")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_nightvision")
CLIENTEFFECT_MATERIAL("effects/shaders/anamorphic_bloom")
CLIENTEFFECT_MATERIAL("effects/shaders/glass_effect_01")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_fxaa")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_gears_bloom")
CLIENTEFFECT_MATERIAL("shadereditor/_rt_framebuffer_prev")
CLIENTEFFECT_MATERIAL("shadereditor/_rt_ppe")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays_texture")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bokeh_for_ssao2")
CLIENTEFFECT_MATERIAL("effects/shaders/Effects from CITY 17/anamorphic_bloom")
CLIENTEFFECT_MATERIAL("effects/shaders/Effects from CITY 17/anamorphic_bloom_final")

CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/blurx")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/blury")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_base")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_final")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_debug")
CLIENTEFFECT_MATERIAL("effects/shaders/downsample_4")

CLIENTEFFECT_MATERIAL("effects/shaders/postproc_natural")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bloom")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_colorgrading")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_flare_naive")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_flare_naive_eyes")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_flare_naive_eyes2")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_hurt")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_pain")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_wet")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_energy")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_energy_grain")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_suit_regenerate")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_battery")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_health")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_health_regenerate")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_toon")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bullettime")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bullettime2")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_sunrays")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_sunrays2")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_bokeh_for_ssao2")
//SSAO
CLIENTEFFECT_MATERIAL("effects/shaders/ssao/ssao")//OverCharged
CLIENTEFFECT_MATERIAL("effects/shaders/ssao/ssaoblur")//OverCharged
CLIENTEFFECT_MATERIAL("effects/shaders/ssao/ssao_combine")//OverCharged

CLIENTEFFECT_MATERIAL("texture_samples/noise_3d")
CLIENTEFFECT_MATERIAL("texture_samples/noise_2d")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_0")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_0_normal")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_1")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_2")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_3")
CLIENTEFFECT_MATERIAL("texture_samples/fx_clouds_4")

CLIENTEFFECT_MATERIAL("effects/shaders/postproc_deferred_lightning")

CLIENTEFFECT_REGISTER_END_CONDITIONAL(engine->GetDXSupportLevel() >= 90)


static IMaterialVar *GetPPEMaterialVar(const char *pszPPEName, const char *pszNode, const char *pszVar)
{
	if (!g_ShaderEditorSystem->IsReady())
	{
		Assert(0);
		return NULL;
	}

	const int iPPEIndex = shaderEdit->GetPPEIndex(pszPPEName);
	IMaterial *pMat = shaderEdit->GetPPEMaterial(iPPEIndex, pszNode);

	if (IsErrorMaterial(pMat))
	{
		Assert(0);
		return NULL;
	}

	IMaterialVar *pVarMutable = pMat->FindVar(pszVar, NULL, false);

	Assert(pVarMutable);

	return pVarMutable;
}
static void SetRenderTargetAndViewPort(ITexture *rt)
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0, 0, rt->GetActualWidth(), rt->GetActualHeight());
}

void C_ViewRender_Overwrite::Post_Processing_Effects(int x, int y, int width, int height)
{
	ConVarRef NV("oc_player_allow_nightvision");
	if (NV.GetBool())
	{
		CBasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

		if (pPlayer)
		{
			if (pPlayer->m_bNightvisionEnabled)
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_nightvision", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);
			}
		}
	}

	if (!r_post_processing.GetInt())
	{
		if (cvar->FindVar("r_post_draw_sunrays")->GetInt())
		{
			int Name = shaderEdit->GetPPEIndex("ppe_sunrays");
			int Name2 = shaderEdit->GetPPEIndex("ppe_sunrays2");
			int Name3 = shaderEdit->GetPPEIndex("ppe_sunrays3");

			shaderEdit->SetPPEEnabled(Name, false);
			shaderEdit->SetPPEEnabled(Name2, false);
			shaderEdit->SetPPEEnabled(Name3, false);

			cvar->FindVar("r_post_draw_sunrays")->SetValue(0);
		}
		return;
	}

	/*if (r_post_draw_ironsight_dof.GetInt())
	{
		if (((cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1) && !reload))
		{
			IMaterial *pMaterialDOF = materials->FindMaterial("effects/shaders/postproc_bokeh", TEXTURE_GROUP_CLIENT_EFFECTS);
			DrawScreenEffectMaterial(pMaterialDOF, x, y, width, height);
		}
		if ((cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0) || reload)
		{

		}
	}*/

	if (r_post_draw_toon.GetInt())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_toon", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
	}

	if (r_post_draw_screen_dust.GetInt() && engine->MapHasHDRLighting())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/dust", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
	}

	if (r_post_draw_hurt.GetInt())
	{
		CBasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

		if (pPlayer)
		{
			if ((pPlayer->GetHealth() <= 30) && (pPlayer->GetHealth() > 27))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.05f);
			}
			if ((pPlayer->GetHealth() <= 27) && (pPlayer->GetHealth() > 24))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.07f);
			}
			if ((pPlayer->GetHealth() <= 24) && (pPlayer->GetHealth() > 21))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.1f);
			}
			if ((pPlayer->GetHealth() <= 21) && (pPlayer->GetHealth() > 17))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.13f);
			}
			if ((pPlayer->GetHealth() <= 17) && (pPlayer->GetHealth() > 13))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.18f);
			}
			if ((pPlayer->GetHealth() <= 13) && (pPlayer->GetHealth() > 9))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.25f);
			}
			if ((pPlayer->GetHealth() <= 9) && (pPlayer->GetHealth() > 5))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.35f);
			}
			if ((pPlayer->GetHealth() <= 5) && (pPlayer->GetHealth() > 0))
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.47f);
			}
			if (pPlayer->GetHealth() <= 0)
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_hurt", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(0.55f);
			}
		}
	}

	if (r_post_draw_pain.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->doDmgBulletEffect)
			{
				indicate = indicate - r_post_draw_pain_ispeed.GetFloat() * gpGlobals->frametime;

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_pain", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(indicate);
			}
			if (pPlayer->doDmgShockEffect)
			{
				indicate2 = indicate2 - r_post_draw_pain_ispeed.GetFloat() * gpGlobals->frametime;

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_pain", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue((indicate)*(-1.0f));
			}
			if ((indicate <= 0.0f) || (indicate2 <= 0.0f))
			{
				indicate = r_post_draw_pain_idelay.GetFloat();
				indicate2 = r_post_draw_pain_idelay.GetFloat();
				pPlayer->doDmgBulletEffect = false;
				pPlayer->doDmgShockEffect = false;
			}
		}
	}

	if (r_post_draw_underwater.GetInt())
	{
		CBasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

		if (pPlayer)
		{
			if (pPlayer->GetWaterLevel() == 3)
			{
				if (indicateU < 1.0f)
				{
					indicateU = indicateU + 1.f/*r_post_draw_underwater_ispeed.GetFloat()*/ * gpGlobals->frametime;
				}

				indicateU = Clamp(indicateU, 0.f, 1.f);

				//indicateU = 1.f;//r_post_draw_underwater_idelay.GetFloat();
				underwater = true;

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_wet", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);
				/*IMaterial *pMaterial2 = materials->FindMaterial("effects/shaders/downsample_4", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial2, x, y, width, height);
				DrawScreenEffectMaterial(pMaterial2, x, y, width, height);
				DrawScreenEffectMaterial(pMaterial2, x, y, width, height);
				DrawScreenEffectMaterial(pMaterial2, x, y, width, height);
				DrawScreenEffectMaterial(pMaterial2, x, y, width, height);*/

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(indicateU);
			}
			if ((pPlayer->GetWaterLevel() != 3) && (underwater))
			{
				if (indicateU > 0.0f)
				{
					indicateU = indicateU - 1.f/*r_post_draw_underwater_ispeed.GetFloat()*/ * gpGlobals->frametime;
				}
				if (indicateU <= 0.0f)
				{
					indicateU = 0.0f;
					underwater = false;
				}

				indicateU = Clamp(indicateU, 0.f, 1.f);

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_wet", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(indicateU);

			}
		}
	}

	if (r_post_draw_suit_regenerate.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->doRechargeEffect)
			{
				//Не удалять!
				/*IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_energy", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);*/

				//Не удалять2!
				/*IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_battery", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				IMaterialVar *var2;
				IMaterialVar *var3;
				IMaterialVar *var4;
				var = pMaterial->FindVar("$texture_1", NULL);
				var2 = pMaterial->FindVar("$texture_2", NULL);
				var3 = pMaterial->FindVar("$texture_3", NULL);
				var4 = pMaterial->FindVar("$texture_4", NULL);
				ITexture *texture = materials->FindTexture("texture_samples/fx_clouds_4", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture2 = materials->FindTexture("texture_samples/noise_3d", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture3 = materials->FindTexture("texture_samples/fx_clouds_1", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture4 = materials->FindTexture("texture_samples/noise_3d", TEXTURE_GROUP_CLIENT_EFFECTS);
				var->SetTextureValue(texture);
				var2->SetTextureValue(texture2);
				var3->SetTextureValue(texture3);
				var4->SetTextureValue(texture4);*/

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_suit_regenerate", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);
				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(r_post_draw_suit_regenerate_inensity.GetFloat());

			}
		}
	}

	if (r_post_draw_health_regenerate.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->doHealthRegenEffect)
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_health_regenerate", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);
				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(r_post_draw_health_regenerate_inensity.GetFloat());
			}
		}
	}

	if (r_post_draw_battery.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->doBatteryEffect)
			{
				indicateE = indicateE++;

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_battery", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				IMaterialVar *var;
				IMaterialVar *var2;
				IMaterialVar *var3;
				IMaterialVar *var4;
				var = pMaterial->FindVar("$texture_1", NULL);
				var2 = pMaterial->FindVar("$texture_2", NULL);
				var3 = pMaterial->FindVar("$texture_3", NULL);
				var4 = pMaterial->FindVar("$texture_4", NULL);
				ITexture *texture = materials->FindTexture("texture_samples/fx_clouds_4", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture2 = materials->FindTexture("texture_samples/noise_3d", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture3 = materials->FindTexture("texture_samples/fx_clouds_1", TEXTURE_GROUP_CLIENT_EFFECTS);
				ITexture *texture4 = materials->FindTexture("texture_samples/noise_3d", TEXTURE_GROUP_CLIENT_EFFECTS);
				var->SetTextureValue(texture);
				var2->SetTextureValue(texture2);
				var3->SetTextureValue(texture3);
				var4->SetTextureValue(texture4);

				if (indicateE >= r_post_draw_battery_delay.GetFloat())
				{
					indicateE = 0;
					pPlayer->doBatteryEffect = false;
				}
			}
		}
	}

	if (r_post_draw_health.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->doHealthEffect)
			{
				indicateH = indicateH - 0.01f;

				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_health_regenerate", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);
				IMaterialVar *var;
				var = pMaterial->FindVar("$MUTABLE_01", NULL);
				var->SetFloatValue(indicateH);

				if (indicateH <= 0.0f)//if (indicateH >= r_post_draw_health_delay.GetFloat())
				{
					pPlayer->doHealthEffect = false;
					indicateH = 0.2;
				}
			}
		}
	}

	if (r_post_draw_bullettime.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			if (pPlayer->GetSlowMoIsEnabled())//if (cvar->FindVar("oc_state_slomo")->GetInt() == 1)
			{
				IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_bullettime2", TEXTURE_GROUP_CLIENT_EFFECTS);
				DrawScreenEffectMaterial(pMaterial, x, y, width, height);

				DrawScreenGaussianBlurSlomo(x, y, width, height);
			}
		}
	}

	if (r_post_draw_filmgrain.GetInt())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postprocess_filmgrain", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filterW;
		IMaterialVar *var;
		var = pMaterial->FindVar("$MUTABLE_01", NULL);
		var->SetFloatValue(r_post_draw_filmgrain_grain_inensity.GetFloat());
		IMaterialVar *var2;
		var2 = pMaterial->FindVar("$MUTABLE_02", NULL);
		var2->SetFloatValue(r_post_draw_filmgrain_light_inensity.GetFloat());
	}

	if (r_post_draw_colorgrading_simple.GetInt())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/colorgrading_simple", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_natural.GetInt())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_natural", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_bloom.GetInt() && engine->MapHasHDRLighting())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_bloom", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_colorgrading.GetInt())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_colorgrading", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_flare_naive.GetInt() && engine->MapHasHDRLighting())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_flare_naive", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_flare_naive_eyes.GetInt() && engine->MapHasHDRLighting())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_flare_naive_eyes", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_flare_naive_eyes2.GetInt() && engine->MapHasHDRLighting())
	{
		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/postproc_flare_naive_eyes2", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
	}

	if (r_post_draw_crossprocessing.GetInt())
	{
		IMaterialVar *var;
		IMaterial *pMaterial2 = materials->FindMaterial("effects/shaders/cross_processing", TEXTURE_GROUP_CLIENT_EFFECTS);

		var = pMaterial2->FindVar("$MUTABLE_01", NULL);
		var->SetFloatValue(r_post_draw_crossprocessing_saturation.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_02", NULL);
		var->SetFloatValue(r_post_draw_crossprocessing_contrast.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_03", NULL);
		var->SetFloatValue(r_post_draw_crossprocessing_brightness.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_04", NULL);
		var->SetFloatValue(r_post_draw_crossprocessing_intensity.GetFloat());

		DrawScreenEffectMaterial(pMaterial2, x, y, width, height);

	}

	if (r_post_draw_desaturation.GetInt() == 1)
	{
		IMaterial *pMaterial3 = materials->FindMaterial("effects/shaders/postproc_desaturation", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial3, x, y, width, height);
	}

	if (r_post_draw_cinematic_overlay.GetInt())
	{
		IMaterial *pMaterial4 = materials->FindMaterial("effects/shaders/postproc_cinematic_overlay", TEXTURE_GROUP_CLIENT_EFFECTS, false);
		DrawScreenEffectMaterial(pMaterial4, x, y, width, height);
	}

	if (r_post_draw_saturation.GetInt())
	{
		IMaterial *pMaterial4 = materials->FindMaterial("effects/shaders/postproc_cinematic_overlay_saturation", TEXTURE_GROUP_CLIENT_EFFECTS, false);

		IMaterialVar *var;

		var = pMaterial4->FindVar("$MUTABLE_01", NULL);
		var->SetFloatValue(r_post_draw_saturation_brightness.GetFloat());
		var = pMaterial4->FindVar("$MUTABLE_02", NULL);
		var->SetFloatValue(r_post_draw_saturation_intensity.GetFloat());

		DrawScreenEffectMaterial(pMaterial4, x, y, width, height);
		CLocalPlayerFilter filterT;
	}

	if (r_post_gears_bloom.GetInt() && engine->MapHasHDRLighting())
	{
		//		DrawDragonBorn(x, y, width, height);
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_gears_bloom", TEXTURE_GROUP_CLIENT_EFFECTS);

		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->PushRenderTargetAndViewport();

		ITexture *dest_rt0 = materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);


		pRenderContext->SetRenderTarget(dest_rt0);
		pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

		pRenderContext->PopRenderTargetAndViewport();

		CMatRenderContextPtr renderContext(materials);

		renderContext->OverrideDepthEnable(true, false);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filter;
		renderContext->OverrideDepthEnable(false, false);
	}

	if (r_post_draw_ssao.GetInt())
	{
		DrawSSAO(x, y, width, height);
	}

	if (r_post_draw_flare_anamorphic.GetInt() && engine->MapHasHDRLighting())
	{
		//int flare = shaderEdit->GetPPEIndex("ppe_flare_anamorphic");
		//shaderEdit->SetPPEEnabled(flare, true);
		DrawFlareAnamorphic(x, y, width, height);
	}

	int Name = shaderEdit->GetPPEIndex("ppe_sunrays");
	int Name2 = shaderEdit->GetPPEIndex("ppe_sunrays2");
	int Name3 = shaderEdit->GetPPEIndex("ppe_sunrays3");
	if (cvar->FindVar("r_post_processing")->GetInt() == 0)
	{
		shaderEdit->SetPPEEnabled(Name, false);
		shaderEdit->SetPPEEnabled(Name2, false);
		shaderEdit->SetPPEEnabled(Name3, false);
	}
	else
	{
		float rayPower = Clamp(r_post_draw_sunrays_power.GetFloat(), 0.f, 1.2f);

		if (cvar->FindVar("mat_queue_mode")->GetInt() != 0)
		{
			if (cvar->FindVar("r_post_draw_sunrays")->GetInt())
				cvar->FindVar("r_post_draw_sunrays")->SetValue(0);
		}

		if (r_post_draw_sunrays.GetInt() == 1)
		{
			shaderEdit->SetPPEEnabled(Name, true);
			IMaterial *pMaterialSun = shaderEdit->GetPPEMaterial(Name, "Sunrays calc");
			IMaterialVar *var;
			var = pMaterialSun->FindVar("$MUTABLE_01", NULL);
			var->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var2;//Power
			var2 = pMaterialSun->FindVar("$MUTABLE_02", NULL);
			var2->SetFloatValue(rayPower);
		}
		else if (r_post_draw_sunrays.GetInt() == 0)
		{
			shaderEdit->SetPPEEnabled(Name, false);
			shaderEdit->SetPPEEnabled(Name2, false);
			shaderEdit->SetPPEEnabled(Name3, false);

			IMaterial *pMaterialSun = shaderEdit->GetPPEMaterial(Name, "Sunrays calc");
			IMaterialVar *var;
			var = pMaterialSun->FindVar("$MUTABLE_01", NULL);
			var->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun2 = shaderEdit->GetPPEMaterial(Name2, "Sunrays calc");
			IMaterialVar *var2;
			var2 = pMaterialSun2->FindVar("$MUTABLE_01", NULL);
			var2->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun3 = shaderEdit->GetPPEMaterial(Name3, "Sunrays calc");
			IMaterialVar *var3;
			var3 = pMaterialSun3->FindVar("$MUTABLE_01", NULL);
			var3->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var4;//Power
			var4 = pMaterialSun->FindVar("$MUTABLE_02", NULL);
			var4->SetFloatValue(rayPower);

			IMaterialVar *var5;//Power
			var5 = pMaterialSun2->FindVar("$MUTABLE_02", NULL);
			var5->SetFloatValue(rayPower);

			IMaterialVar *var6;//Power
			var6 = pMaterialSun3->FindVar("$MUTABLE_02", NULL);
			var6->SetFloatValue(rayPower);
		}
		if ((r_post_draw_sunrays_amount.GetInt() == 0) && (r_post_draw_sunrays.GetInt() == 1))
		{
			shaderEdit->SetPPEEnabled(Name3, false);
			shaderEdit->SetPPEEnabled(Name2, false);

			IMaterial *pMaterialSun3 = shaderEdit->GetPPEMaterial(Name3, "Sunrays calc");
			IMaterialVar *var;
			var = pMaterialSun3->FindVar("$MUTABLE_01", NULL);
			var->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun2 = shaderEdit->GetPPEMaterial(Name2, "Sunrays calc");
			IMaterialVar *var2;
			var2 = pMaterialSun2->FindVar("$MUTABLE_01", NULL);
			var2->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var3;//Power
			var3 = pMaterialSun3->FindVar("$MUTABLE_02", NULL);
			var3->SetFloatValue(rayPower);

			IMaterialVar *var4;//Power
			var4 = pMaterialSun2->FindVar("$MUTABLE_02", NULL);
			var4->SetFloatValue(rayPower);
		}
		if ((r_post_draw_sunrays_amount.GetInt() == 1) && (r_post_draw_sunrays.GetInt() == 1))
		{
			shaderEdit->SetPPEEnabled(Name3, false);
			shaderEdit->SetPPEEnabled(Name2, true);
			shaderEdit->SetPPEEnabled(Name, false);


			IMaterial *pMaterialSun3 = shaderEdit->GetPPEMaterial(Name3, "Sunrays calc");
			IMaterialVar *var3;
			var3 = pMaterialSun3->FindVar("$MUTABLE_01", NULL);
			var3->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun2 = shaderEdit->GetPPEMaterial(Name2, "Sunrays calc");
			IMaterialVar *var2;
			var2 = pMaterialSun2->FindVar("$MUTABLE_01", NULL);
			var2->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun = shaderEdit->GetPPEMaterial(Name, "Sunrays calc");
			IMaterialVar *var;
			var = pMaterialSun->FindVar("$MUTABLE_01", NULL);
			var->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var4;//Power
			var4 = pMaterialSun->FindVar("$MUTABLE_02", NULL);
			var4->SetFloatValue(rayPower);

			IMaterialVar *var5;//Power
			var5 = pMaterialSun2->FindVar("$MUTABLE_02", NULL);
			var5->SetFloatValue(rayPower);

			IMaterialVar *var6;//Power
			var6 = pMaterialSun3->FindVar("$MUTABLE_02", NULL);
			var6->SetFloatValue(rayPower);
		}
		if ((r_post_draw_sunrays_amount.GetInt() == 2) && (r_post_draw_sunrays.GetInt() == 1))
		{
			shaderEdit->SetPPEEnabled(Name3, true);
			shaderEdit->SetPPEEnabled(Name2, false);
			shaderEdit->SetPPEEnabled(Name, false);

			IMaterial *pMaterialSun3 = shaderEdit->GetPPEMaterial(Name3, "Sunrays calc");
			IMaterialVar *var3;
			var3 = pMaterialSun3->FindVar("$MUTABLE_01", NULL);
			var3->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun2 = shaderEdit->GetPPEMaterial(Name2, "Sunrays calc");
			IMaterialVar *var2;
			var2 = pMaterialSun2->FindVar("$MUTABLE_01", NULL);
			var2->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun = shaderEdit->GetPPEMaterial(Name, "Sunrays calc");
			IMaterialVar *var;
			var = pMaterialSun->FindVar("$MUTABLE_01", NULL);
			var->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var4;//Power
			var4 = pMaterialSun->FindVar("$MUTABLE_02", NULL);
			var4->SetFloatValue(rayPower);

			IMaterialVar *var5;//Power
			var5 = pMaterialSun2->FindVar("$MUTABLE_02", NULL);
			var5->SetFloatValue(rayPower);

			IMaterialVar *var6;//Power
			var6 = pMaterialSun3->FindVar("$MUTABLE_02", NULL);
			var6->SetFloatValue(rayPower);
		}
		if ((r_post_draw_sunrays_amount.GetInt() >= 3) && (r_post_draw_sunrays.GetInt() == 1))
		{
			shaderEdit->SetPPEEnabled(Name3, false);
			shaderEdit->SetPPEEnabled(Name2, false);

			IMaterial *pMaterialSun3 = shaderEdit->GetPPEMaterial(Name3, "Sunrays calc");
			IMaterialVar *var3;
			var3 = pMaterialSun3->FindVar("$MUTABLE_01", NULL);
			var3->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterial *pMaterialSun2 = shaderEdit->GetPPEMaterial(Name2, "Sunrays calc");
			IMaterialVar *var2;
			var2 = pMaterialSun2->FindVar("$MUTABLE_01", NULL);
			var2->SetVecValue(r_post_draw_sunrays_colorR.GetFloat(), r_post_draw_sunrays_colorG.GetFloat(), r_post_draw_sunrays_colorB.GetFloat(), r_post_draw_sunrays_colorA.GetFloat());

			IMaterialVar *var4;//Power
			var4 = pMaterialSun2->FindVar("$MUTABLE_02", NULL);
			var4->SetFloatValue(rayPower);

			IMaterialVar *var5;//Power
			var5 = pMaterialSun3->FindVar("$MUTABLE_02", NULL);
			var5->SetFloatValue(rayPower);
		}
	}

	if (r_post_draw_ironsight_screenblur.GetInt())
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
		{
			C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
			if (pWeapon && pWeapon->IsIronSighted())
				DrawScreenGaussianBlur(x, y, width, height);
		}
	}

	if (r_post_draw_dragonborn.GetInt())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_dragonborn", TEXTURE_GROUP_CLIENT_EFFECTS);

		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->PushRenderTargetAndViewport();

		ITexture *dest_rt0 = materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);

		pRenderContext->SetRenderTarget(dest_rt0);
		pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

		pRenderContext->PopRenderTargetAndViewport();

		CMatRenderContextPtr renderContext(materials);

		renderContext->OverrideDepthEnable(true, false);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filter;
		renderContext->OverrideDepthEnable(false, false);
	}
	if (!r_post_draw_dragonborn.GetInt())
	{

	}

	if (r_post_draw_chromatic_abberations.GetInt())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_chromatic_abberations", TEXTURE_GROUP_CLIENT_EFFECTS);

		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->PushRenderTargetAndViewport();

		ITexture *dest_rt0 = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);

		pRenderContext->SetRenderTarget(dest_rt0);
		pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

		pRenderContext->PopRenderTargetAndViewport();

		CMatRenderContextPtr renderContext(materials);

		renderContext->OverrideDepthEnable(true, false);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filter;
		renderContext->OverrideDepthEnable(false, false);
	}
	if (!r_post_draw_chromatic_abberations.GetInt())
	{

	}

	if (r_post_draw_fxaa.GetInt())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_fxaa", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
	}
	if (!r_post_draw_fxaa.GetInt())
	{

	}
}

float GetSceneFadeScalar()
{
	byte color[4];
	bool blend;
	vieweffects->GetFadeParams(&color[0], &color[1], &color[2], &color[3], &blend);

	return 1.0f - (color[3] / 255.0f);
}
void C_ViewRender_Overwrite::DrawSunRays(int x, int y, int width, int height)
{

}

void C_ViewRender_Overwrite::DrawScreenGaussianBlur(int x, int y, int width, int height)
{
	static const int iScreenBlur = shaderEdit->GetPPEIndex(SCREENBLUR_EDITOR_NAME);
	if (iScreenBlur < 0)
		return;

	if (!r_post_draw_ironsight_screenblur.GetBool())
		return;

	/*if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
		return;*/

	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "downsample", "$MUTABLE_01", pVar_ScreenBlur_Strength);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blur x", "$MUTABLE_02", pVar_ScreenBlur_x);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blur y", "$MUTABLE_03", pVar_ScreenBlur_y);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blend base textures", "$MUTABLE_04", pVar_ScreenBlur_blend);

	if (pVar_ScreenBlur_Strength == NULL)
	{
		Assert(0);
		return;
	}

	if (pVar_ScreenBlur_x == NULL)
	{
		Assert(0);
		return;
	}
	if (pVar_ScreenBlur_y == NULL)
	{
		Assert(0);
		return;
	}
	if (pVar_ScreenBlur_blend == NULL)
	{
		Assert(0);
		return;
	}


	bool g_pPPCtrl = r_post_draw_ironsight_screenblur.GetInt();
	float intensity = g_pPPCtrl ? 30.0f : 0;
	if (intensity <= 0.001f)
		return;

	pVar_ScreenBlur_Strength->SetFloatValue(clamp(intensity, 0, 1));
	CMatRenderContextPtr renderContext(materials);
	renderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreenBlur, x, y, width, height);
	renderContext->OverrideDepthEnable(false, false);
}
void C_ViewRender_Overwrite::DrawScreenGaussianBlurSlomo(int x, int y, int width, int height)
{
	static const int iScreenBlur = shaderEdit->GetPPEIndex(SCREENBLUR_EDITOR_NAME);
	if (iScreenBlur < 0)
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		if (!pPlayer->GetSlowMoIsEnabled())//if (cvar->FindVar("oc_state_slomo")->GetInt() == 0)
			return;
	}

	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "downsample", "$MUTABLE_01", pVar_ScreenBlur_Strength);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blur x", "$MUTABLE_02", pVar_ScreenBlur_x);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blur y", "$MUTABLE_03", pVar_ScreenBlur_y);
	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blend base textures", "$MUTABLE_04", pVar_ScreenBlur_blend);

	if (pVar_ScreenBlur_Strength == NULL)
	{
		Assert(0);
		return;
	}

	if (pVar_ScreenBlur_x == NULL)
	{
		Assert(0);
		return;
	}
	if (pVar_ScreenBlur_y == NULL)
	{
		Assert(0);
		return;
	}
	if (pVar_ScreenBlur_blend == NULL)
	{
		Assert(0);
		return;
	}

	float intensity = 30.0f;
	if (intensity <= 0.001f)
		return;

	pVar_ScreenBlur_Strength->SetFloatValue(clamp(intensity, 0, 1));
	CMatRenderContextPtr renderContext(materials);
	renderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreenBlur, x, y, width, height);
	renderContext->OverrideDepthEnable(false, false);
}

void C_ViewRender_Overwrite::DrawSSAO(int x, int y, int width, int height)
{
	static const int iScreenSSAO = shaderEdit->GetPPEIndex(SSAO_EDITOR_NAME);
	if (iScreenSSAO < 0)
		return;

	if (!r_post_draw_ssao.GetBool())
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(SSAO_EDITOR_NAME, "ssao calc", "$MUTABLE_01", ssao_calc);
	DEFINE_SHADEREDITOR_MATERIALVAR(SSAO_EDITOR_NAME, "ssao fxaa", "$MUTABLE_02", ssao_fxaa);
	DEFINE_SHADEREDITOR_MATERIALVAR(SSAO_EDITOR_NAME, "combine from 1", "$MUTABLE_03", ssao_combine);


	if (ssao_calc == NULL)
	{
		Assert(0);
		return;
	}

	if (ssao_fxaa == NULL)
	{
		Assert(0);
		return;
	}
	if (ssao_combine == NULL)
	{
		Assert(0);
		return;
	}

	CMatRenderContextPtr renderContext(materials);
	renderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreenSSAO, x, y, width, height);
	renderContext->OverrideDepthEnable(false, false);
}
void C_ViewRender_Overwrite::DrawFlareAnamorphic(int x, int y, int width, int height)
{
	static const int iScreenFlare_A = shaderEdit->GetPPEIndex(FLARE_A_EDITOR_NAME);
	if (iScreenFlare_A < 0)
		return;

	if (!r_post_draw_flare_anamorphic.GetBool())
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(FLARE_A_EDITOR_NAME, "postproc_flare_anamorphic", "$MUTABLE_01", postproc_flare_anamorphic);
	DEFINE_SHADEREDITOR_MATERIALVAR(FLARE_A_EDITOR_NAME, "post_flare_combine", "$MUTABLE_02", post_flare_combine);
	CMatRenderContextPtr pRenderContext(materials);

	pRenderContext->PushRenderTargetAndViewport();

	ITexture *dest_rt0 = materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);

	SetRenderTargetAndViewPort(dest_rt0);
	pRenderContext->SetRenderTarget(dest_rt0);
	pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

	SetRenderTargetAndViewPort(dest_rt0);

	ITexture *dest_rt1 = materials->FindTexture("_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET);

	SetRenderTargetAndViewPort(dest_rt1);
	pRenderContext->SetRenderTarget(dest_rt1);
	pRenderContext->Viewport(0, 0, dest_rt1->GetActualWidth(), dest_rt1->GetActualHeight());

	SetRenderTargetAndViewPort(dest_rt1);

	pRenderContext->PopRenderTargetAndViewport();

	Assert(postproc_flare_anamorphic);

	if (postproc_flare_anamorphic == NULL)
	{
		Assert(0);
		return;
	}
	Assert(post_flare_combine);

	if (post_flare_combine == NULL)
	{
		Assert(0);
		return;
	}
	pRenderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreenFlare_A, x, y, width, height);
	pRenderContext->OverrideDepthEnable(false, false);
}
