//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_SCOPELIGHT_H
#define C_SCOPELIGHT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;


class C_ScopeLight
{
public:

	C_ScopeLight(int ScopeLightnEntIndex = 0, float _lightLinear = 0, float constLight = 0, float _lightFov = 0, float _lightFar = 0);
	~C_ScopeLight();




	virtual void UpdateLightScopeLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
	void TurnOnScopeLight();
	void TurnOffScopeLight();
	bool IsOnScopeLight(void) { return m_ScopeLight; }

	bool m_ScopeLight;
	float lightConstant;
	float lightFov;
	float lightFar;
	float lightLinear;

	ClientShadowHandle_t GetScopeLightHandle(void) { return m_ScopeLightHandle; }
	void SetScopeLightHandle(ClientShadowHandle_t Handle) { m_ScopeLightHandle = Handle; }

protected:

	void LightOffScopeLight();
	void LightOffOldScopeLight();
	void LightOffNewScopeLight();

	//	void Tracer(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);

	void UpdateLightNewScopeLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
	void UpdateLightOldScopeLight(const Vector &vecPos, const Vector &vecDir, int nDistance);

	//	bool m_M;

	int m_nEntIndexScopeLight;
	ClientShadowHandle_t m_ScopeLightHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLightScopeLight;
	float m_flDistModScopeLight;
	float mFov = 0.0f;
	// Texture for flashlight
	CTextureReference m_ScopeLightTexture;

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
