#include "cbase.h"
#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "rendertexture.h"
#include "view_scene.h"
#include "viewrender.h"
#include "ivieweffects.h"
#include "ClientEffectPrecacheSystem.h"
#include "view.h"
#include "shadereditor/shadereditorsystem.h"
#include "shadereditor/ivshadereditor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SCREENBLUR_EDITOR_NAME "ppe_gaussian_blur"
#define OVERLAY_EDITOR_NAME "ppe_dragonborn"

ConVar r_post_drawscreenblur("r_post_drawscreenblur", "0", FCVAR_CHEAT);
ConVar r_post_dragonborn("r_post_dragonborn", "0", FCVAR_CHEAT);

ConVar r_post_glass_effect("r_post_glass_effect", "0", FCVAR_CHEAT);


ConVar r_post_screen_dust("r_post_screen_dust", "0", FCVAR_CHEAT);
ConVar r_post_bokeh_dof("r_post_bokeh_dof", "0", FCVAR_CHEAT);




ConVar r_post_desaturation("r_post_desaturation", "0", FCVAR_CHEAT);
ConVar r_post_filmgrain("r_post_filmgrain", "0", FCVAR_CHEAT);
ConVar r_post_flare_anamorphic("r_post_flare_anamorphic", "0", FCVAR_CHEAT);
ConVar r_post_flare_anamorphic_strenth("r_post_flare_anamorphic_strenth", "0", FCVAR_CHEAT);

ConVar r_post_colorgrading_simple("r_post_colorgrading_simple", "0", FCVAR_CHEAT);

ConVar r_post_cinematic("r_post_cinematic", "0", FCVAR_CHEAT);
ConVar r_post_cinematic_saturation("r_post_cinematic_saturation", "0.8", FCVAR_CHEAT);
ConVar r_post_cinematic_contrast("r_post_cinematic_contrast", "1.0", FCVAR_CHEAT);
ConVar r_post_cinematic_brightness("r_post_cinematic_brightness", "0.0", FCVAR_CHEAT);
ConVar r_post_cinematic_intensity("r_post_cinematic_intensity", "0.2", FCVAR_CHEAT);

ConVar r_post_cinematic_overlay("r_post_cinematic_overlay", "0", FCVAR_CHEAT);

ConVar r_post_gears_bloom("r_post_gears_bloom", "0", FCVAR_CHEAT);

ConVar r_post_sunshaft("r_post_sunshaft", "0", FCVAR_CHEAT);
ConVar r_post_sunshaft_debug("r_post_sunshaft_debug", "0", FCVAR_CHEAT);

ConVar r_post_sunshaft_blur("r_post_sunshaft_blur", "1", FCVAR_ARCHIVE);
ConVar r_post_sunshaft_blur_amount("r_post_sunshaft_blur_amount", "0.5", FCVAR_CHEAT);


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
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_cinematic_overlay")
CLIENTEFFECT_MATERIAL("effects/shaders/postprocess_filmgrain")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_dragonborn")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_desaturation")
CLIENTEFFECT_MATERIAL("effects/shaders/anamorphic_bloom")
CLIENTEFFECT_MATERIAL("effects/shaders/glass_effect_01")
CLIENTEFFECT_MATERIAL("effects/shaders/postproc_gears_bloom")
CLIENTEFFECT_MATERIAL("shadereditor/_rt_framebuffer_prev")
CLIENTEFFECT_MATERIAL("shadereditor/_rt_ppe")


CLIENTEFFECT_MATERIAL("effects/shaders/Effects from CITY 17/anamorphic_bloom")
CLIENTEFFECT_MATERIAL("effects/shaders/Effects from CITY 17/anamorphic_bloom_final")

CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/blurx")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/blury")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_base")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_final")
CLIENTEFFECT_MATERIAL("effects/shaders/sunrays/sunshaft_debug")

CLIENTEFFECT_REGISTER_END_CONDITIONAL(engine->GetDXSupportLevel() >= 90)



inline bool ShouldDrawCommon()
{
	if (!r_post_drawscreenblur.GetBool())
		return false;

	return g_ShaderEditorSystem->IsReady() && engine->IsInGame() && !engine->IsPaused();
}



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


void CViewRender::Post_Processing_Effects(int x, int y, int width, int height)
{





	if (r_post_screen_dust.GetInt() == 1)
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/dust", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filterW;
	}
	if (r_post_screen_dust.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null); //убирает значение оверлея экрана 
		CLocalPlayerFilter filterW;
	}




	if (r_post_filmgrain.GetInt() == 1)
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postprocess_filmgrain", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filterW;
	}
	if (r_post_filmgrain.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filterW;
	}





	if (r_post_colorgrading_simple.GetInt() == 1)
	{

		IMaterial *pMaterial1 = materials->FindMaterial("effects/shaders/colorgrading_simple", TEXTURE_GROUP_CLIENT_EFFECTS);

		DrawScreenEffectMaterial(pMaterial1, x, y, width, height);
		CLocalPlayerFilter filterQ;
	}
	if (r_post_colorgrading_simple.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filterQ;
	}




	if (r_post_cinematic.GetInt() == 1)
	{
		IMaterialVar *var;
		IMaterial *pMaterial2 = materials->FindMaterial("effects/shaders/cross_processing", TEXTURE_GROUP_CLIENT_EFFECTS);

		var = pMaterial2->FindVar("$MUTABLE_01", NULL);
		var->SetFloatValue(r_post_cinematic_saturation.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_02", NULL);
		var->SetFloatValue(r_post_cinematic_contrast.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_03", NULL);
		var->SetFloatValue(r_post_cinematic_brightness.GetFloat());
		var = pMaterial2->FindVar("$MUTABLE_04", NULL);
		var->SetFloatValue(r_post_cinematic_intensity.GetFloat());


		DrawScreenEffectMaterial(pMaterial2, x, y, width, height);
		CLocalPlayerFilter filter;
	}
	if (r_post_cinematic.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filter;
	}



	if (r_post_bokeh_dof.GetInt() == 1)
	{
		IMaterial *pMaterial3 = materials->FindMaterial("effects/shaders/postproc_bokeh", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial3, x, y, width, height);
		//view->SetScreenOverlayMaterial( pMaterial3 );
		CLocalPlayerFilter filter;
	}
	if (r_post_bokeh_dof.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filter;
	}





	if (r_post_desaturation.GetInt() == 1)
	{
		IMaterial *pMaterial3 = materials->FindMaterial("effects/shaders/postproc_desaturation", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial3, x, y, width, height);
		//view->SetScreenOverlayMaterial( pMaterial3 );
		CLocalPlayerFilter filter;
	}
	if (r_post_desaturation.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filter;
	}







	if (r_post_cinematic_overlay.GetInt() == 1)
	{
		IMaterial *pMaterial4 = materials->FindMaterial("effects/shaders/postproc_cinematic_overlay", TEXTURE_GROUP_CLIENT_EFFECTS, false);
		DrawScreenEffectMaterial(pMaterial4, x, y, width, height);
		CLocalPlayerFilter filterT;
	}
	if (r_post_cinematic_overlay.GetInt() == 0)
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filterT;
	}









	if (r_post_drawscreenblur.GetInt() )
	{
		DrawScreenGaussianBlur(x, y, width, height);
	}



	if (r_post_dragonborn.GetInt())
	{
//		DrawDragonBorn(x, y, width, height);
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
	if (!r_post_dragonborn.GetInt())
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filter;
	}




	if (r_post_glass_effect.GetInt())
	{
		//		DrawDragonBorn(x, y, width, height);
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/glass_effect_01", TEXTURE_GROUP_CLIENT_EFFECTS);
		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
		CLocalPlayerFilter filter;
	}
	if (!r_post_glass_effect.GetInt())
	{
		view->SetScreenOverlayMaterial(null);
		CLocalPlayerFilter filter;
	}


}


void CViewRender::DrawScreenGaussianBlur(int x, int y, int width, int height)
{
	/*	if (!ShouldDrawCommon())
			return;*/

	static const int iScreenBlur = shaderEdit->GetPPEIndex(SCREENBLUR_EDITOR_NAME);
	if (iScreenBlur < 0)
		return;

	if (!r_post_drawscreenblur.GetBool())
		return;

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


	bool g_pPPCtrl = r_post_drawscreenblur.GetInt();
	float intensity = g_pPPCtrl ? 30.0f : 0;
	if (intensity <= 0.001f)
		return;

		pVar_ScreenBlur_Strength->SetFloatValue(clamp(intensity, 0, 1));
		CMatRenderContextPtr renderContext(materials);
		renderContext->OverrideDepthEnable(true, false);
		shaderEdit->DrawPPEOnDemand(iScreenBlur, x, y, width, height);
		renderContext->OverrideDepthEnable(false, false);
}

void CViewRender::DrawDragonBorn(int x, int y, int width, int height)
{
	/*	if (!ShouldDrawCommon())
	return;*/
	/*
	static const int iScreen = shaderEdit->GetPPEIndex(OVERLAY_EDITOR_NAME);
	if (iScreen < 0)
		return;

	if (!r_post_dragonborn.GetBool())
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(OVERLAY_EDITOR_NAME, "dragonborn", "$MUTABLE_02", pVar_dragonborn);
//	DEFINE_SHADEREDITOR_MATERIALVAR(OVERLAY_EDITOR_NAME, "fbquarter_0", "$MUTABLE_01", pVar_fbquarter_0);
//	DEFINE_SHADEREDITOR_MATERIALVAR(OVERLAY_EDITOR_NAME, "postproc_dragonborn", "$MUTABLE_01", pVar_postproc_dragonborn);


	Assert(pVar_dragonborn);
	if (pVar_dragonborn == NULL)
	{
		Assert(0);
		return;
	}

/*	if (pVar_postproc_dragonborn == NULL)
	{
		Assert(0);
		return;
		}*//*

//	ITexture *pTexture = GetFullFrameFrameBufferTexture(0);
//	CMatRenderContextPtr renderContext(materials);

	bool g_pPPCtrl = r_post_dragonborn.GetInt();
	float intensity = g_pPPCtrl ? 15.0f : 0;
	if (intensity <= 0.001f)
		return;

//	pVar_dragonborn->SetFloatValue(clamp(intensity, 0.1f, 15.0f));
//	pVar_postproc_dragonborn->SetFloatValue(clamp(intensity, 1.1f, 3.0f));

	CMatRenderContextPtr renderContext(materials);
	renderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreen, x, y, width, height);
	renderContext->OverrideDepthEnable(false, false);*/
}