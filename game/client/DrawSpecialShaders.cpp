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
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
ConVar r_post_draw_bokeh_dof("r_post_draw_bokeh_dof", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_InReload_dof("r_post_draw_InReload_dof", "0", FCVAR_ARCHIVE);
ConVar r_post_draw_ironsight_blur("r_post_draw_ironsight_blur", "0", FCVAR_ARCHIVE);


ConVar r_post_draw_deferred("r_post_draw_deferred", "0", FCVAR_ARCHIVE);
void CViewRender::DrawDOF(int x, int y, int width, int height)
{
	if (r_post_draw_bokeh_dof.GetInt())
	{
		IMaterial *pMaterial3 = materials->FindMaterial("effects/shaders/postproc_bokeh", TEXTURE_GROUP_OTHER);
			DrawScreenEffectMaterial(pMaterial3, x, y, width, height);
	}
	if (!(r_post_draw_bokeh_dof.GetInt()))
	{

	}

	if (r_post_draw_deferred.GetInt())
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		IMaterial *pMaterial = materials->FindMaterial("effects/shaders/postproc_deferred_lightning", TEXTURE_GROUP_OTHER);
		IMaterialVar *var;
		var = pMaterial->FindVar("$MUTABLE_01", NULL);

		Vector vs_pos = pPlayer->EyePosition();

		var->SetVecValue(vs_pos.x, vs_pos.y, vs_pos.z);

		DrawScreenEffectMaterial(pMaterial, x, y, width, height);
	}
}

void CViewRender::DrawIRDOF(int x, int y, int width, int height)
{

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

		bool inReload = pWeapon != NULL ? pWeapon->IsInReload() && r_post_draw_InReload_dof.GetInt() : false;

		if (pPlayer->IsInSelectionSlowMo() | inReload)
		{
			IMaterial *pMaterial4 = materials->FindMaterial("effects/shaders/downsample_4", TEXTURE_GROUP_OTHER);
			DrawScreenEffectMaterial(pMaterial4, x, y, width, height);
		}
	}

}


/*float GetSceneFadeScalar()
{
	byte color[4];
	bool blend;
	vieweffects->GetFadeParams(&color[0], &color[1], &color[2], &color[3], &blend);

	return 1.0f - (color[3] / 255.0f);
}
void CViewRender::DrawSunRays(int x, int y, int width, int height)
{
	if (!r_post_draw_sunrays.GetInt())
		return;
	else
	{
		if (!r_post_draw_sunrays_amount.GetFloat())
		{
			static const int iScreenSunRays = shaderEdit->GetPPEIndex(SUNRAYS_EDITOR_NAME);
			if (iScreenSunRays < 0)
				return;
			DEFINE_SHADEREDITOR_MATERIALVAR(SUNRAYS_EDITOR_NAME, "sunrays calc", "$MUTABLE_01", sunrays_calc);

			sunrays_calc->SetVecValue(1.0f * GetSceneFadeScalar(), 0, 0, 0);

			CMatRenderContextPtr pRenderContext(materials);

			pRenderContext->PushRenderTargetAndViewport();

			ITexture *dest_rt0 = materials->FindTexture("_rt_SEdit_Skymask", TEXTURE_GROUP_RENDER_TARGET);//"_rt_SmallFB0"

			//SetRenderTargetAndViewPort(dest_rt0);
			pRenderContext->SetRenderTarget(dest_rt0);
			pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

			//SetRenderTargetAndViewPort(dest_rt0);

			pRenderContext->PopRenderTargetAndViewport();

			Assert(sunrays_calc);

			if (sunrays_calc == NULL)
			{
				Assert(0);
				return;
			}

			pRenderContext->OverrideDepthEnable(true, true);
			shaderEdit->DrawPPEOnDemand(iScreenSunRays, x, y, width, height);
			pRenderContext->OverrideDepthEnable(false, false);



		}
		else if (r_post_draw_sunrays_amount.GetFloat() == 1)
		{
			static const int iScreenSunRays2 = shaderEdit->GetPPEIndex(SUNRAYS2_EDITOR_NAME);
			if (iScreenSunRays2 < 0)
				return;
			DEFINE_SHADEREDITOR_MATERIALVAR(SUNRAYS_EDITOR_NAME, "sunrays calc", "$MUTABLE_01", sunrays_calc);

			sunrays_calc->SetVecValue(1.0f * GetSceneFadeScalar(), 0, 0, 0);

			CMatRenderContextPtr pRenderContext(materials);

			pRenderContext->PushRenderTargetAndViewport();

			ITexture *dest_rt0 = materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);

			//SetRenderTargetAndViewPort(dest_rt0);
			pRenderContext->SetRenderTarget(dest_rt0);
			pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

			//SetRenderTargetAndViewPort(dest_rt0);

			pRenderContext->PopRenderTargetAndViewport();

			Assert(sunrays_calc);

			if (sunrays_calc == NULL)
			{
				Assert(0);
				return;
			}

			pRenderContext->OverrideDepthEnable(true, false);
			shaderEdit->DrawPPEOnDemand(iScreenSunRays2, x, y, width, height);
			pRenderContext->OverrideDepthEnable(false, false);
		}
		else if (r_post_draw_sunrays_amount.GetFloat() == 2)
		{
			static const int iScreenSunRays3 = shaderEdit->GetPPEIndex(SUNRAYS3_EDITOR_NAME);
			if (iScreenSunRays3 < 0)
				return;
			DEFINE_SHADEREDITOR_MATERIALVAR(SUNRAYS_EDITOR_NAME, "sunrays calc", "$MUTABLE_01", sunrays_calc);

			sunrays_calc->SetVecValue(1.0f * GetSceneFadeScalar(), 0, 0, 0);

			CMatRenderContextPtr pRenderContext(materials);

			pRenderContext->PushRenderTargetAndViewport();

			ITexture *dest_rt0 = materials->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);

			//SetRenderTargetAndViewPort(dest_rt0);
			pRenderContext->SetRenderTarget(dest_rt0);
			pRenderContext->Viewport(0, 0, dest_rt0->GetActualWidth(), dest_rt0->GetActualHeight());

			//SetRenderTargetAndViewPort(dest_rt0);

			pRenderContext->PopRenderTargetAndViewport();

			Assert(sunrays_calc);

			if (sunrays_calc == NULL)
			{
				Assert(0);
				return;
			}

			pRenderContext->OverrideDepthEnable(true, false);
			shaderEdit->DrawPPEOnDemand(iScreenSunRays3, x, y, width, height);
			pRenderContext->OverrideDepthEnable(false, false);
		}


		if (!r_post_draw_sunrays.GetBool())
			return;


	}
}*/