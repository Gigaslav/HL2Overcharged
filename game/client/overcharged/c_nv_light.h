//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_NVLIGHT_H
#define C_NVLIGHT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_NVLight
{
public:

	C_NVLight(int nEntIndex = 0);
	virtual ~C_NVLight();

	virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void TurnOn();
	void TurnOff();
	bool IsOn(void) { return m_bIsOn; }

	ClientShadowHandle_t GetFlashlightHandle(void) { return m_FlashlightHandle; }
	void SetFlashlightHandle(ClientShadowHandle_t Handle) { m_FlashlightHandle = Handle; }

protected:

	void LightOff();
	void LightOffOld();
	void LightOffNew();

	void UpdateLightNew(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void UpdateLightOld(const Vector &vecPos, const Vector &vecDir, int nDistance);

	bool m_bIsOn;
	int m_nEntIndex;
	ClientShadowHandle_t m_FlashlightHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLight;
	float m_flDistMod;

	// Texture for flashlight
	CTextureReference m_FlashlightTexture;
};

#endif // FLASHLIGHTEFFECT_H
