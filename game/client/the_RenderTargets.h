#ifndef TNERENDERTARGETS_H_
#define TNERENDERTARGETS_H_
#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render   targets

// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CTNERenderTargets : public CBaseClientRenderTargets
{
	// no networked vars 
	DECLARE_CLASS_GAMEROOT(CTNERenderTargets, CBaseClientRenderTargets);
public:
	virtual void InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture* CreateScopeTexture(IMaterialSystem* pMaterialSystem);

private:
	CTextureReference		m_ScopeTexture;
};

extern CTNERenderTargets* TNERenderTargets;





/*class CTNERenderTargetsCamo : public CBaseClientRenderTargets
{
	// no networked vars 
	DECLARE_CLASS_GAMEROOT(CTNERenderTargetsCamo, CBaseClientRenderTargets);
public:
	virtual void InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture* CreateCamoTexture(IMaterialSystem* pMaterialSystem);

private:
	CTextureReference		m_CamoTexture;
};

extern CTNERenderTargetsCamo* TNERenderTargetsCamo;*/
#endif //TNERENDERTARGETS_H_