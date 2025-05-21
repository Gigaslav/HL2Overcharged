//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_MUZZLEFLASH_LASER_H
#define C_MUZZLEFLASH_LASER_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_MuzzleflashLaser
{
public:

	C_MuzzleflashLaser(int MLIndex = 0);
	~C_MuzzleflashLaser();

	virtual void UpdateLightML(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void TurnOnML();
	void TurnOffML();
	bool IsOnML(void) { return m_ML; }

	bool m_ML;
	bool Switcher;

	ClientShadowHandle_t GetMHandle(void) { return m_MLHandle; }
	void SetMHandle(ClientShadowHandle_t Handle) { m_MLHandle = Handle; }

protected:

	void LightOffML();
	void LightOffOldML();
	void LightOffNewML();

	void UpdateLightNewML(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void UpdateLightOldML(const Vector &vecPos, const Vector &vecDir, int nDistance);

	//	bool m_M;

	int IndexML;
	ClientShadowHandle_t m_MLHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLightML;
	float m_flDistModML;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_MLTexture;

};

#endif // FLASHLIGHTEFFECT_H
