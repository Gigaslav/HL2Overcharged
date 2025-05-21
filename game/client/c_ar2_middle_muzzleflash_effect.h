//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_AR2_MIDDLE_MUZZLEFLASH_EFFECT_H
#define C_AR2_MIDDLE_MUZZLEFLASH_EFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_Ar2_Middle_MuzzleflashEffect
{
public:

	C_Ar2_Middle_MuzzleflashEffect(int AR2_MnEntIndex = 0);
	~C_Ar2_Middle_MuzzleflashEffect();


	void SetupTexture(C_BasePlayer *pPlayer);

	virtual void Ar2_Middle_UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, float lKoef);
	void Ar2_Middle_TurnOnM();
	void Ar2_Middle_TurnOffM();
	bool Ar2_Middle_IsOnM(void) { return m_Ar2_MM; }

	bool m_Ar2_MM;

	ClientShadowHandle_t GetAr2MMHandle(void) { return m_Ar2_MMHandle; }
	void SetAr2MMHandle(ClientShadowHandle_t Handle) { m_Ar2_MMHandle = Handle; }

protected:

	void Ar2_Middle_LightOffM();
	void Ar2_Middle_LightOffOldM();
	void Ar2_Middle_LightOffNewM();

	void Ar2_Middle_UpdateLightNewM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float lKoef);
	void Ar2_Middle_UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance, float lKoef);

	//	bool m_M;

	int m_Ar2_MnEntIndexM;
	ClientShadowHandle_t m_Ar2_MMHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_Ar2_MpPointLightM;
	float m_Ar2_MflDistModM;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_Ar2_MMTexture;
	float TimeTick = 0.0f;
	float MaxTime = 0.0f;
	//float SpeedOfTime = 0.0f;

	private:
		const char *nameL;



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
