//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_MUZZLEFLASH_EFFECT_H
#define C_MUZZLEFLASH_EFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_MuzzleflashEffect
{
public:

	C_MuzzleflashEffect(int MnEntIndex = 0);
	~C_MuzzleflashEffect();


	void SetupTexture(C_BasePlayer *pPlayer);

	virtual void UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, float lKoef);

	void TurnOnM();
	void TurnOffM();
	bool IsOnM(void) { return m_M; }

	bool m_M;

	ClientShadowHandle_t GetMHandle(void) { return m_MHandle; }
	void SetMHandle(ClientShadowHandle_t Handle) { m_MHandle = Handle; }

	bool Destroy(void) { return D; }
	bool D;

	float MaxTime = 0.0f;

protected:

	void LightOffM();
	void LightOffOldM();
	void LightOffNewM();

	//	void Tracer(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);

	void UpdateLightNewM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float lKoef);
	void UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance, float lKoef);

	//	bool m_M;

	int m_nEntIndexM;
	ClientShadowHandle_t m_MHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLightM;
	float m_flDistModM;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_MTexture;
	float TimeTick = 0.0f;
	//float SpeedOfTime = 0.0f;

	private:
		const char *nameL;
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
