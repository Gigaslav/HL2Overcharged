//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_MUZZLEFLASH_LASER_MIDDLE_H
#define C_MUZZLEFLASH_LASER_MIDDLE_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_MuzzleflashLaserM
{
public:

	C_MuzzleflashLaserM(int MLMIndex = 0);
	~C_MuzzleflashLaserM();

	virtual void UpdateLightMLM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void TurnOnMLM();
	void TurnOffMLM();
	bool IsOnMLM(void) { return m_MLM; }

	bool m_MLM;
	bool Switcher;

	ClientShadowHandle_t GetMHandle(void) { return m_MLMHandle; }
	void SetMHandle(ClientShadowHandle_t Handle) { m_MLMHandle = Handle; }

protected:

	void LightOffMLM();
	void LightOffOldMLM();
	void LightOffNewMLM();

	void UpdateLightNewMLM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void UpdateLightOldMLM(const Vector &vecPos, const Vector &vecDir, int nDistance);

	//	bool m_M;

	int IndexMLM;
	ClientShadowHandle_t m_MLMHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLightMLM;
	float m_flDistModMLM;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_MLMTexture;

};

#endif // FLASHLIGHTEFFECT_H
