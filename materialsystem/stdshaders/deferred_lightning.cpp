#include "BaseVSShader.h"
#include "deferred_lightning_vs30.inc"
#include "deferred_lightning_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS(deffered_lightning, "Deffered shading", SHADER_NOT_EDITABLE)
BEGIN_SHADER_PARAMS
SHADER_PARAM(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	SET_FLAGS2(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE);
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	if (params[BASETEXTURE]->IsDefined())
	{
		LoadTexture(BASETEXTURE);
	}
}

SHADER_DRAW
{
	SHADOW_STATE
	{
		pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

		pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);

		bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
		pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, bForceSRGBReadAndWrite);
		pShaderShadow->EnableSRGBWrite(bForceSRGBReadAndWrite);

		DECLARE_STATIC_VERTEX_SHADER(deferred_lightning_vs30);
		SET_STATIC_VERTEX_SHADER(deferred_lightning_vs30);

		DECLARE_STATIC_PIXEL_SHADER(deferred_lightning_ps30);
		SET_STATIC_PIXEL_SHADER(deferred_lightning_ps30);
	}

		DYNAMIC_STATE
	{
		BindTexture(SHADER_SAMPLER0, BASETEXTURE, -1);

		ITexture *src_texture = params[BASETEXTURE]->GetTextureValue();

		int width = src_texture->GetActualWidth();
		int height = src_texture->GetActualHeight();

		float g_TexelSize[2] = { 1.0f / float(width), 1.0f / float(height) };

		pShaderAPI->SetPixelShaderConstant(0, g_TexelSize);

		DECLARE_DYNAMIC_VERTEX_SHADER(deferred_lightning_vs30);
		SET_DYNAMIC_VERTEX_SHADER(deferred_lightning_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(deferred_lightning_ps30);
		SET_DYNAMIC_PIXEL_SHADER(deferred_lightning_ps30);

		Vector cam = Vector(100, 245, 244);
		//pShaderAPI->SetVertexShaderConstant()
		pShaderAPI->SetPixelShaderConstant(1, cam.Base());
	}
	Draw();
}
END_SHADER