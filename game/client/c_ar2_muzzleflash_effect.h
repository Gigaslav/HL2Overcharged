//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_AR2_MUZZLEFLASH_EFFECT_H
#define C_AR2_MUZZLEFLASH_EFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_Ar2_MuzzleflashEffect
{
public:

	C_Ar2_MuzzleflashEffect(int AR2_nEntIndex = 0);
	~C_Ar2_MuzzleflashEffect();




	virtual void Ar2_UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void Ar2_TurnOnM();
	void Ar2_TurnOffM();
	bool Ar2_IsOnM(void) { return m_Ar2_M; }

	bool m_Ar2_M;

	ClientShadowHandle_t GetAr2MHandle(void) { return m_Ar2_MHandle; }
	void SetAr2MHandle(ClientShadowHandle_t Handle) { m_Ar2_MHandle = Handle; }

protected:

	void Ar2_LightOffM();
	void Ar2_LightOffOldM();
	void Ar2_LightOffNewM();

	void Ar2_UpdateLightNewM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void Ar2_UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance);

	//	bool m_M;

	int m_Ar2_nEntIndexM;
	ClientShadowHandle_t m_Ar2_MHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_Ar2_pPointLightM;
	float m_Ar2_flDistModM;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_Ar2_MTexture;

	//private:
	//		float m_flMuzzleFlashRoll;



	//		CTextureReference m_MTexture;
};
/*
class CHeadlightEffect : public CFlashlightEffect
{
public:

CHeadlightEffect();
~CHeadlightEffect();

virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
};

*/

#endif // FLASHLIGHTEFFECT_H
