//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: When you take damage or you damage someone and blood splats on 
//          you then you'll draw that blood on your hands/weapon. (overlay)
//
//=============================================================================//

#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "baseviewmodel_shared.h"
#include "c_baseplayer.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_WeaponBloodTextureProxy : public IMaterialProxy
{
public:
	C_WeaponBloodTextureProxy();
	virtual ~C_WeaponBloodTextureProxy();

	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	C_BaseEntity *BindArgToEntity(void *pArg);
	virtual void OnBind(void *pC_BaseEntity);
	virtual void Release() { delete this; }
	IMaterial *GetMaterial();

private:
	IMaterialVar *m_pBlendFactor;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_WeaponBloodTextureProxy::C_WeaponBloodTextureProxy()
{
	m_pBlendFactor = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_WeaponBloodTextureProxy::~C_WeaponBloodTextureProxy()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_WeaponBloodTextureProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool found;

	m_pBlendFactor = pMaterial->FindVar("$detailblendfactor", &found, false);
	if (!found)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseEntity *C_WeaponBloodTextureProxy::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_WeaponBloodTextureProxy::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_BaseViewModel *pViewModel = dynamic_cast<C_BaseViewModel *>(pEntity);
	if (pViewModel)
	{
		C_BasePlayer *pOwner = ToBasePlayer(pViewModel->GetOwner());
		if (pOwner && pOwner->GetActiveWeapon())
		{
			IMaterialVar *m_pBlendFactorFrame;
			m_pBlendFactorFrame = GetMaterial()->FindVar("$detailframe", NULL);
			m_pBlendFactorFrame->SetFloatValue(pOwner->GetActiveWeapon()->m_iWeaponBloodOverlayDetailFrame);
		
			float alpha = 0.f;

			alpha = (pOwner->m_flNextBloodDryDisappear - gpGlobals->curtime) * 0.1f;

			alpha = Clamp(alpha, 0.f, 1.f);

			//DevMsg("Weapons alpha: %.2f \n", alpha * (float)pOwner->GetActiveWeapon()->m_bShouldDrawWeaponBloodOverlay);

			m_pBlendFactor->SetFloatValue(alpha * (float)pOwner->GetActiveWeapon()->m_bShouldDrawWeaponBloodOverlay);
		}
	}
	else
		m_pBlendFactor->SetFloatValue(0.f);

	if (ToolsEnabled())
		ToolFramework_RecordMaterialParams(GetMaterial());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IMaterial *C_WeaponBloodTextureProxy::GetMaterial()
{
	if (m_pBlendFactor == NULL)
		return NULL;

	return m_pBlendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE(C_WeaponBloodTextureProxy, IMaterialProxy, "BloodyTexture" IMATERIAL_PROXY_INTERFACE_VERSION);








class C_HandsBloodTextureProxy : public IMaterialProxy
{
public:
	C_HandsBloodTextureProxy();
	virtual ~C_HandsBloodTextureProxy();

	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	C_BaseEntity *BindArgToEntity(void *pArg);
	virtual void OnBind(void *pC_BaseEntity);
	virtual void Release() { delete this; }
	IMaterial *GetMaterial();

private:
	IMaterialVar *m_pBlendFactor;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_HandsBloodTextureProxy::C_HandsBloodTextureProxy()
{
	m_pBlendFactor = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_HandsBloodTextureProxy::~C_HandsBloodTextureProxy()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_HandsBloodTextureProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool found;

	m_pBlendFactor = pMaterial->FindVar("$detailblendfactor", &found, false);
	if (!found)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseEntity *C_HandsBloodTextureProxy::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HandsBloodTextureProxy::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_BaseViewModel *pViewModel = dynamic_cast<C_BaseViewModel *>(pEntity);
	if (pViewModel)
	{
		C_BasePlayer *pOwner = ToBasePlayer(pViewModel->GetOwner());
		if (pOwner)
		{
			IMaterialVar *m_pBlendFactorFrame;
			m_pBlendFactorFrame = GetMaterial()->FindVar("$detailframe", NULL);
			m_pBlendFactorFrame->SetFloatValue(pOwner->m_iBloodOverlayDetailFrame);

			float alpha = 0.f;

			alpha = (pOwner->m_flNextBloodDryDisappear - gpGlobals->curtime) * 0.1f;

			alpha = Clamp(alpha, 0.f, 1.f);

			//DevMsg("Hands alpha: %.2f \n", alpha);

			m_pBlendFactor->SetFloatValue(alpha);
		}
	}
	else
		m_pBlendFactor->SetFloatValue(0.f);

	if (ToolsEnabled())
		ToolFramework_RecordMaterialParams(GetMaterial());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IMaterial *C_HandsBloodTextureProxy::GetMaterial()
{
	if (m_pBlendFactor == NULL)
		return NULL;

	return m_pBlendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE(C_HandsBloodTextureProxy, IMaterialProxy, "BloodyHandsTexture" IMATERIAL_PROXY_INTERFACE_VERSION);