//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef LIGHTS_H
#define LIGHTS_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CLight : public CPointEntity
{
public:
	DECLARE_CLASS(CLight, CPointEntity);

	bool	KeyValue(const char *szKeyName, const char *szValue);
	void	Spawn(void);
	void	FadeThink(void);
	void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void	TurnOn(void);
	void	TurnOff(void);
	void	Toggle(void);

	// Input handlers
	void	InputSetPattern(inputdata_t &inputdata);
	void	InputFadeToPattern(inputdata_t &inputdata);

	void	InputToggle(inputdata_t &inputdata);
	void	InputTurnOn(inputdata_t &inputdata);
	void	InputTurnOff(inputdata_t &inputdata);

	DECLARE_DATADESC();

private:
	int		m_iStyle;
	int		m_iDefaultStyle;
	string_t m_iszPattern;
	char	m_iCurrentFade;
	char	m_iTargetFade;
};

#ifdef DEFERRED
#define EnvLightBase CServerOnlyPointEntity
#else
#define EnvLightBase CBaseEntity
#endif

class CEnvLight : public EnvLightBase
{
public:
	DECLARE_CLASS(CEnvLight, EnvLightBase);
#ifndef DEFERRED
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CEnvLight();
#endif

	virtual bool KeyValue(const char *szKeyName, const char *szValue);

#ifdef DEFERRED
	void	Activate(void);

private:
	float m_vecLight[4];
	float m_vecAmbientLight[4];
	float m_fLightPitch;
#else
	virtual void Spawn();

	virtual int ObjectCaps()
	{
		return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	}

	virtual int UpdateTransmitState()
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	CNetworkVar(bool, m_bCascadedShadowMappingEnabled);
	CNetworkVar(bool, csmEnableLightColor);
	CNetworkVar(bool, csmEnableAmbientLightColor);
	CNetworkVar(int, cmsFov);
	CNetworkColor32(csmLightColor);
	CNetworkColor32(csmAmbientColor);
private:
	CNetworkQAngle(m_angSunAngles);
	CNetworkVector(m_vecLight);
	CNetworkVector(m_vecAmbient);
	bool m_bHasHDRLightSet;
	bool m_bHasHDRAmbientSet;
#endif
};

#endif // LIGHTS_H
