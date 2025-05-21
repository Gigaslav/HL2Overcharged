#if !defined( VIEWRENDER_H )
#define VIEWRENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "datamap.h"


//------------------------------------------------------------------------------
// Purpose : Sunlight shadow control entity
//------------------------------------------------------------------------------
class CGlobalLight : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CGlobalLight, CBaseEntity);

	CGlobalLight();
	virtual void	Activate();
	void EnableFromMapAdd();
	void Spawn(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	virtual bool GetKeyValue(const char *szKeyName, char *szValue, int iMaxLen);
	int  UpdateTransmitState();
	void  SetDirection(const QAngle &angle);
	// Inputs
	void	InputSetAngles(inputdata_t &inputdata);
	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);
	void	InputSetTexture(inputdata_t &inputdata);
	void	InputSetEnableShadows(inputdata_t &inputdata);
	void	InputSetLightColor(inputdata_t &inputdata);
#ifdef MAPBASE
	void	InputSetBrightness(inputdata_t &inputdata);
	void	InputSetBrightness(float value);
	void	InputSetColorTransitionTime(inputdata_t &inputdata);
	void	InputSetXOffset(inputdata_t &inputdata) { m_flEastOffset = inputdata.value.Float(); }
	void	InputSetYOffset(inputdata_t &inputdata) { m_flForwardOffset = inputdata.value.Float(); }
	void	InputSetOrthoSize(inputdata_t &inputdata) { m_flOrthoSize = inputdata.value.Float(); }
	void	InputSetDistance(inputdata_t &inputdata) { m_flSunDistance = inputdata.value.Float(); }
	void	InputSetFOV(inputdata_t &inputdata) { m_flFOV = inputdata.value.Float(); }
	void	InputSetNearZDistance(inputdata_t &inputdata) { m_flNearZ = inputdata.value.Float(); }
	void	InputSetNorthOffset(inputdata_t &inputdata) { m_flNorthOffset = inputdata.value.Float(); }
#endif

	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int	Restore(IRestore &restore);

	DECLARE_SERVERCLASS();

private:
	CNetworkVector(m_shadowDirection);
	Vector m_shadowDirectionInternal;
	CNetworkVar(bool, m_bEnabled);
	bool m_bStartDisabled;

	CNetworkString(m_TextureName, MAX_PATH);
#ifdef MAPBASE
	CNetworkVar(int, m_nSpotlightTextureFrame);
#endif
	CNetworkColor32(m_LightColor);
#ifdef MAPBASE
	CNetworkVar(float, m_flBrightnessScale);
#endif
	CNetworkVar(float, m_flColorTransitionTime);
	CNetworkVar(float, m_flSunDistance);
	CNetworkVar(float, m_flFOV);
	CNetworkVar(float, m_flNearZ);
	CNetworkVar(float, m_flNorthOffset);
	CNetworkVar(int, m_iShadowQuality);
#ifdef MAPBASE
	CNetworkVar(float, m_flEastOffset); // xoffset
	CNetworkVar(float, m_flForwardOffset); // yoffset
	CNetworkVar(float, m_flOrthoSize);
#endif
	CNetworkVar(bool, m_bEnableShadows);

};

#endif