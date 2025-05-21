//========= Copyright 1996-2010, Valve Corporation, All rights reserved. ============//
//
// Purpose: global dynamic light. Ported from Insolence's port of Alien Swarm's env_global_light.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "OverCharged/RTT/env_global_light.h"
#include "datamap.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------



LINK_ENTITY_TO_CLASS(global_lightning, CGlobalLight);

BEGIN_DATADESC(CGlobalLight)

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "enabled"),
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_AUTO_ARRAY_KEYFIELD(m_TextureName, FIELD_CHARACTER, "texturename"),
#ifdef MAPBASE
DEFINE_KEYFIELD(m_nSpotlightTextureFrame, FIELD_INTEGER, "textureframe"),
#endif
DEFINE_KEYFIELD(m_flSunDistance, FIELD_FLOAT, "distance"),
DEFINE_KEYFIELD(m_flFOV, FIELD_FLOAT, "fov"),
DEFINE_KEYFIELD(m_flNearZ, FIELD_FLOAT, "nearz"),
DEFINE_KEYFIELD(m_flNorthOffset, FIELD_FLOAT, "northoffset"),
#ifdef MAPBASE
DEFINE_KEYFIELD(m_flEastOffset, FIELD_FLOAT, "eastoffset"),
DEFINE_KEYFIELD(m_flForwardOffset, FIELD_FLOAT, "forwardoffset"),
DEFINE_KEYFIELD(m_flOrthoSize, FIELD_FLOAT, "orthosize"),
#endif
DEFINE_KEYFIELD(m_bEnableShadows, FIELD_BOOLEAN, "enableshadows"),
DEFINE_FIELD(m_LightColor, FIELD_COLOR32),
DEFINE_FIELD(m_shadowDirection, FIELD_VECTOR),
#ifdef MAPBASE
DEFINE_KEYFIELD(m_flBrightnessScale, FIELD_FLOAT, "brightnessscale"),
#endif
DEFINE_KEYFIELD(m_flColorTransitionTime, FIELD_FLOAT, "colortransitiontime"),

// Inputs
#ifdef MAPBASE
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetXOffset", InputSetXOffset),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetYOffset", InputSetYOffset),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetOrthoSize", InputSetOrthoSize),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetDistance", InputSetDistance),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetFOV", InputSetFOV),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetNearZDistance", InputSetNearZDistance),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetNorthOffset", InputSetNorthOffset),
#else
DEFINE_INPUT( m_flSunDistance,		FIELD_FLOAT, "SetDistance" ),
DEFINE_INPUT( m_flFOV,				FIELD_FLOAT, "SetFOV" ),
DEFINE_INPUT( m_flNearZ,			FIELD_FLOAT, "SetNearZDistance" ),
DEFINE_INPUT( m_flNorthOffset,			FIELD_FLOAT, "SetNorthOffset" ),
#endif

DEFINE_INPUTFUNC(FIELD_COLOR32, "LightColor", InputSetLightColor),
DEFINE_INPUTFUNC(FIELD_STRING, "SetAngles", InputSetAngles),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_STRING, "SetTexture", InputSetTexture),
//DEFINE_INPUTFUNC(FIELD_BOOLEAN, "EnableShadows", InputSetEnableShadows),
#ifdef MAPBASE
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetBrightness", InputSetBrightness),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetColorTransitionTime", InputSetColorTransitionTime),
#endif

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST_NOBASE(CGlobalLight, DT_GlobalLight)
SendPropVector(SENDINFO(m_shadowDirection), -1, SPROP_NOSCALE),
SendPropBool(SENDINFO(m_bEnabled)),
SendPropString(SENDINFO(m_TextureName)),
#ifdef MAPBASE
SendPropInt(SENDINFO(m_nSpotlightTextureFrame)),
#endif
/*SendPropInt(SENDINFO (m_LightColor ),	32, SPROP_UNSIGNED, SendProxy_Color32ToInt32 ),*/
SendPropInt(SENDINFO(m_LightColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt),
#ifdef MAPBASE
SendPropFloat(SENDINFO(m_flBrightnessScale)),
#endif
SendPropFloat(SENDINFO(m_flColorTransitionTime)),
SendPropFloat(SENDINFO(m_flSunDistance), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flFOV), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flNearZ), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flNorthOffset), 0, SPROP_NOSCALE),
#ifdef MAPBASE
SendPropFloat(SENDINFO(m_flEastOffset), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flForwardOffset), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flOrthoSize), 0, SPROP_NOSCALE),
#endif
SendPropBool(SENDINFO(m_bEnableShadows)),
END_SEND_TABLE()

static CGlobalLight *g_GlobalLightning = NULL;

void UpdateGlobalLight();
void UpdateGlobalLightShadows();
void UpdateGlobalLightFreq();
void UpdateGlobalLightFov();
void UpdateGlobalLightNearZ();
void UpdateGlobalLightOrthoSize();
void UpdateGlobalLightDist();
void UpdateGlobalLightBrightness();
void UpdateGlobalLightOffsets();
void UpdateGlobalLightAngles();

ConVar oc_global_lightning_enabled("oc_global_lightning_enabled", "1", FCVAR_ARCHIVE, "", (FnChangeCallback_t)UpdateGlobalLight);
ConVar oc_global_lightning_update_freq("oc_global_lightning_update_freq", "0", FCVAR_ARCHIVE, "", (FnChangeCallback_t)UpdateGlobalLightFreq);
ConVar oc_global_lightning_shadows("oc_global_lightning_shadows", "1", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightShadows);
ConVar oc_global_lightning_shadows_quality("oc_global_lightning_shadows_quality", "1", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightShadows);
ConVar oc_global_lightning_fov("oc_global_lightning_fov", "180", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightFov);
ConVar oc_global_lightning_brightness("oc_global_lightning_brightness", "2", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightBrightness);
ConVar oc_global_lightning_nearz("oc_global_lightning_nearz", "10", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightNearZ);
ConVar oc_global_lightning_northoffset("oc_global_lightning_northoffset", "0", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightOffsets);
ConVar oc_global_lightning_eastoffset("oc_global_lightning_eastoffset", "0", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightOffsets);
ConVar oc_global_lightning_forwardoffset("oc_global_lightning_forwardoffset", "0", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightOffsets);
ConVar oc_global_lightning_orthosize("oc_global_lightning_orthosize", "1000", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightOrthoSize);
ConVar oc_global_lightning_spawnangles("oc_global_lightning_spawnangles", "0 0 90", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightAngles);
ConVar oc_global_lightning_sun_distance("oc_global_lightning_sun_distance", "10000", FCVAR_NONE, "", (FnChangeCallback_t)UpdateGlobalLightDist);

CGlobalLight* GetGlobalLightEntity();

CGlobalLight* GetGlobalLightEntity()
{
	return g_GlobalLightning;
}

void UpdateGlobalLight()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		//cvar->FindVar("r_shadows_gamecontrol")->SetValue(oc_global_lightning_enabled.GetBool() ? 0 : -1);

		glEnt->KeyValue("enabled", oc_global_lightning_enabled.GetString());
	}
}

void UpdateGlobalLightShadows()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("enableshadows", oc_global_lightning_shadows.GetString());

		glEnt->KeyValue("shadows_quality", oc_global_lightning_shadows_quality.GetString());
	}
}

void UpdateGlobalLightFreq()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("update_freq", oc_global_lightning_update_freq.GetString());
	}
}

void UpdateGlobalLightAngles()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("angles", oc_global_lightning_spawnangles.GetString());
	}
}

void UpdateGlobalLightBrightness()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->InputSetBrightness(oc_global_lightning_brightness.GetFloat());
	}
}

void UpdateGlobalLightDist()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("distance", oc_global_lightning_sun_distance.GetString());

		/*glEnt->KeyValue("fov", oc_global_lightning_fov.GetString());

		glEnt->KeyValue("nearz", oc_global_lightning_nearz.GetString());

		glEnt->KeyValue("orthosize", oc_global_lightning_orthosize.GetString());*/
	}
}

void UpdateGlobalLightFov()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("fov", oc_global_lightning_fov.GetString());
	}
}

void UpdateGlobalLightNearZ()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("nearz", oc_global_lightning_nearz.GetString());
	}
}

void UpdateGlobalLightOrthoSize()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("orthosize", oc_global_lightning_orthosize.GetString());
	}
}

void UpdateGlobalLightOffsets()
{
	CGlobalLight *glEnt = GetGlobalLightEntity();
	if (glEnt)
	{
		glEnt->KeyValue("northoffset", oc_global_lightning_northoffset.GetString());

		glEnt->KeyValue("eastoffset", oc_global_lightning_eastoffset.GetString());

		glEnt->KeyValue("forwardoffset", oc_global_lightning_forwardoffset.GetString());
	}
}

int CGlobalLight::Restore(IRestore &restore)
{
	int status = BaseClass::Restore(restore);
	if (!status)
		return 0;

	//UpdateGlobalLight();

	// success
	return 1;
}

CGlobalLight::CGlobalLight()
{
#if defined( _X360 )
	Q_strcpy( m_TextureName.GetForModify(), "effects/flashlight_border" );
#else
	Q_strcpy( m_TextureName.GetForModify(), "effects/flashlight001" );
#endif
	if (m_LightColor.GetR() == 0 && m_LightColor.GetG() == 0 && m_LightColor.GetB() == 0 && m_LightColor.GetA() == 0)
	{
#ifdef MAPBASE
		m_LightColor.Init(255, 255, 255, 255);
#else
		m_LightColor.Init( 255, 255, 255, 1 );
#endif
	}
	m_flColorTransitionTime = 0.5f;

	m_nSpotlightTextureFrame = 0;
	/*m_flSunDistance = 10000.0f;
	m_flFOV = 5.0f;
	m_bEnableShadows = false;
#ifdef MAPBASE
	m_nSpotlightTextureFrame = 0;
	m_flBrightnessScale = 1.0f;
	m_flOrthoSize = 1000.0f;
#endif*/

	//cvar->FindVar("r_simpleshadows_disabled")->SetValue(0);
	//m_bEnabled = m_bEnableShadows = true;

	m_nSpotlightTextureFrame = oc_global_lightning_update_freq.GetInt();

	m_iShadowQuality = oc_global_lightning_shadows_quality.GetInt();

	m_flFOV = oc_global_lightning_fov.GetInt();


	m_flNearZ = oc_global_lightning_nearz.GetInt();

	m_flForwardOffset = oc_global_lightning_forwardoffset.GetInt();

	m_flSunDistance = oc_global_lightning_sun_distance.GetInt();
	m_flOrthoSize = oc_global_lightning_orthosize.GetInt();

	g_GlobalLightning = this;
	
	m_bEnableShadows = true;
}

void CGlobalLight::Activate()
{
	BaseClass::Activate();
}

void CGlobalLight::EnableFromMapAdd()
{
	UpdateGlobalLight();

	UpdateGlobalLightShadows();
}

//------------------------------------------------------------------------------
// Purpose : Send even though we don't have a model
//------------------------------------------------------------------------------
int CGlobalLight::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CGlobalLight::SetDirection(const QAngle &angle)
{
	Vector vForward;
	AngleVectors(angle, &vForward);
	m_shadowDirection = vForward;
}

bool CGlobalLight::KeyValue( const char *szKeyName, const char *szValue )
{
#ifdef MAPBASE
	if ( FStrEq( szKeyName, "lightcolor" ) || FStrEq( szKeyName, "color" ) )
#else
	if ( FStrEq( szKeyName, "color" ) )
#endif
	{
		float tmp[4];
		UTIL_StringToFloatArray( tmp, 4, szValue );
		
		m_LightColor.SetR( tmp[0] );
		m_LightColor.SetG( tmp[1] );
		m_LightColor.SetB( tmp[2] );
		m_LightColor.SetA( tmp[3] );
	}
	else if (FStrEq(szKeyName, "enabled"))
	{
		int ival = Q_atoi(szValue);
		m_bEnabled = ival > 0;
		cvar->FindVar("r_shadows_gamecontrol")->SetValue(m_bEnabled ? 0 : -1);
	}
	else if (FStrEq(szKeyName, "enableshadows"))
	{
		int ival = Q_atoi(szValue);
		m_bEnableShadows = ival > 0;
	}
	else if ( FStrEq( szKeyName, "angles" ) )
	{
		QAngle angles;
		UTIL_StringToVector( angles.Base(), szValue );
		if (angles == vec3_angle)
		{
			angles.Init( 80, 30, 0 );
		}
		Vector vForward;
		AngleVectors( angles, &vForward );
		m_shadowDirectionInternal = vForward;
		m_shadowDirection = m_shadowDirectionInternal;
		return true;
	}
	else if (FStrEq(szKeyName, "orthosize"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flOrthoSize = Clamp((float)tmp[0], 2000.f, 50000.f);//oc_global_lightning_orthosize.GetInt();
	}
	else if (FStrEq(szKeyName, "distance"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flSunDistance = tmp[0];//oc_global_lightning_sun_distance.GetInt();
	}
	else if (FStrEq(szKeyName, "fov"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flFOV = tmp[0];//oc_global_lightning_fov.GetInt();
	}
	else if (FStrEq(szKeyName, "nearz"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flNearZ = tmp[0];//oc_global_lightning_nearz.GetInt();
	}
	else if (FStrEq(szKeyName, "forwardoffset"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flForwardOffset = tmp[0];//oc_global_lightning_forwardoffset.GetInt();
	}
	else if (FStrEq(szKeyName, "northoffset"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flNorthOffset = tmp[0];//oc_global_lightning_northoffset.GetInt();
	}
	else if (FStrEq(szKeyName, "eastoffset"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_flEastOffset = tmp[0];//oc_global_lightning_eastoffset.GetInt();
	}
	else if (FStrEq(szKeyName, "shadows_quality"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_iShadowQuality = tmp[0];//oc_global_lightning_shadows_quality.GetInt();
	}
	else if (FStrEq(szKeyName, "update_freq"))
	{
		int tmp[1];
		UTIL_StringToIntArray(tmp, 1, szValue);
		m_nSpotlightTextureFrame = tmp[0];
	}
	/*else if (FStrEq(szKeyName, "enableshadows"))
	{
		switch (szValue[0])
		{
		case '1':
			cvar->FindVar("r_draw_simpleshadows")->SetValue(1);
			break;
		case '0':
			cvar->FindVar("r_draw_simpleshadows")->SetValue(0);
			break;
		}
	}*/
	else if ( FStrEq( szKeyName, "texturename" ) )
	{
#if defined( _X360 )
		if ( Q_strcmp( szValue, "effects/flashlight001" ) == 0 )
		{
			// Use this as the default for Xbox
			Q_strcpy( m_TextureName.GetForModify(), "effects/flashlight_border" );
		}
		else
		{
			Q_strcpy( m_TextureName.GetForModify(), szValue );
		}
#else
		Q_strcpy( m_TextureName.GetForModify(), szValue );
#endif
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

bool CGlobalLight::GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen )
{
	if ( FStrEq( szKeyName, "color" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%d %d %d %d", m_LightColor.GetR(), m_LightColor.GetG(), m_LightColor.GetB(), m_LightColor.GetA() );
		return true;
	}
	else if ( FStrEq( szKeyName, "texturename" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%s", m_TextureName.Get() );
		return true;
	}
	return BaseClass::GetKeyValue( szKeyName, szValue, iMaxLen );
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CGlobalLight::Spawn( void )
{
	Precache();
	SetSolid( SOLID_NONE );

	if( m_bStartDisabled )
	{
		m_bEnabled = false;
	}
	else
	{
		m_bEnabled = true;
	}
}

//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CGlobalLight::InputSetAngles( inputdata_t &inputdata )
{
	const char *pAngles = inputdata.value.String();

	QAngle angles;
	UTIL_StringToVector( angles.Base(), pAngles );

	Vector vTemp;
	AngleVectors( angles, &vTemp );
	m_shadowDirection = vTemp;
}

//------------------------------------------------------------------------------
// Purpose : Input handlers
//------------------------------------------------------------------------------
void CGlobalLight::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;
}

void CGlobalLight::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
}

void CGlobalLight::InputSetTexture( inputdata_t &inputdata )
{
	Q_strcpy( m_TextureName.GetForModify(), inputdata.value.String() );
}

void CGlobalLight::InputSetEnableShadows( inputdata_t &inputdata )
{
	m_bEnableShadows = inputdata.value.Bool();
}

void CGlobalLight::InputSetLightColor( inputdata_t &inputdata )
{
	m_LightColor = inputdata.value.Color32();
} 

#ifdef MAPBASE
void CGlobalLight::InputSetBrightness( inputdata_t &inputdata )
{
	m_flBrightnessScale = inputdata.value.Float();
}

void CGlobalLight::InputSetBrightness(float value)
{
	m_flBrightnessScale = value;
}

void CGlobalLight::InputSetColorTransitionTime( inputdata_t &inputdata )
{
	m_flColorTransitionTime = inputdata.value.Float();
}
#endif
