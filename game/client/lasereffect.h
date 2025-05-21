//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef LASEREFFECT_H
#define LASEREFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class CLaserEffect
{
public:

	CLaserEffect(int nEntIndex = 0);
	~CLaserEffect();

	virtual void UpdateLightLaser(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void TurnOnLaser();
	void TurnOffLaser();
	bool IsOnLaser(void) { return m_IsOnlaser; }



	ClientShadowHandle_t laserHandle(void) { return m_laserHandle; }
	void SetlaserHandle(ClientShadowHandle_t Handle) { m_laserHandle = Handle; }

protected:

	void LightOffLaser();
	void LightOffOldLaser();
	void LightOffNewLaser();
	virtual void UpdateLightProjectionLaser(FlashlightState_t &state);
	void UpdateLightNewLaser(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void UpdateLightOldLaser(const Vector &vecPos, const Vector &vecDir, int nDistance);

	bool m_IsOnlaser;

	int m_nEntIndexlaser;
	//	ClientShadowHandle_t m_FlashlightHandle;
	ClientShadowHandle_t m_laserHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLightlaser;
	float m_flDistModlaser;

	// Texture for flashlight
	//	CTextureReference m_FlashlightTexture;
	//For laser dot
	CTextureReference m_laserTexture;
};
/*
class CHeadlightlaserEffect : public CLaserEffect
{
public:

CHeadlightlaserEffect();
~CHeadlightlaserEffect();

virtual void UpdateLightLaser(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
};
*/


#endif // FLASHLIGHTEFFECT_H
