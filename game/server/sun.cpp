//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "sendproxy.h"
#include "sun_shared.h"
#include "map_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CSun : public CBaseEntity
{
public:
	DECLARE_CLASS( CSun, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CSun();

	virtual void	Activate();

	// Input handlers
	void InputTurnOn( inputdata_t &inputdata );
	void InputTurnOff( inputdata_t &inputdata );
	void InputSetColor( inputdata_t &inputdata );
	//void				Think(void);
	virtual int UpdateTransmitState();

public:
	CNetworkVector( m_vDirection );
	
	string_t	m_strMaterial;
	string_t	m_strOverlayMaterial;

	int		SunRaysIntesity;
	float	SunRaysPower;
	float SunRaysPowerOrigin;
	float SunRaysColorR;
	float SunRaysColorG;
	float SunRaysColorB;
	float SunRaysColorA;
	int		m_bUseAngles;
	float	m_flPitch;
	float	m_flYaw;
	bool disableSprite = false;
	CNetworkVar( int, m_nSize );		// Size of the main core image
	CNetworkVar( int, m_nOverlaySize ); // Size for the glow overlay
	CNetworkVar( color32, m_clrOverlay );
	CNetworkVar( bool, m_bOn );
	CNetworkVar( int, m_nMaterial );
	CNetworkVar( int, m_nOverlayMaterial );
	CNetworkVar( float, m_flHDRColorScale );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CSun, DT_Sun )
	SendPropInt( SENDINFO(m_clrRender), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt ),
	SendPropInt( SENDINFO(m_clrOverlay), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt ),
	SendPropVector( SENDINFO(m_vDirection), 0, SPROP_NORMAL ),
	SendPropInt( SENDINFO(m_bOn), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nSize), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nOverlaySize), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nMaterial), 32, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nOverlayMaterial), 32, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO_NAME( m_flHDRColorScale, HDRColorScale ), 0,	SPROP_NOSCALE,	0.0f,	100.0f ),
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS( env_sun, CSun );


BEGIN_DATADESC( CSun )

	DEFINE_FIELD( m_vDirection,		FIELD_VECTOR ),
	
	DEFINE_KEYFIELD( SunRaysIntesity, FIELD_INTEGER, "sunrays_intensity" ),
	DEFINE_KEYFIELD(SunRaysPower, FIELD_FLOAT, "sunrays_power"),
	DEFINE_KEYFIELD(SunRaysColorR, FIELD_FLOAT, "sunrays_color.r"),
	DEFINE_KEYFIELD(SunRaysColorG, FIELD_FLOAT, "sunrays_color.g"),
	DEFINE_KEYFIELD(SunRaysColorB, FIELD_FLOAT, "sunrays_color.b"),
	DEFINE_KEYFIELD(SunRaysColorA, FIELD_FLOAT, "sunrays_color.a"),
	DEFINE_KEYFIELD( m_bUseAngles, FIELD_INTEGER, "use_angles" ),
	DEFINE_KEYFIELD( m_flPitch, FIELD_FLOAT, "pitch" ),
	DEFINE_KEYFIELD( m_flYaw, FIELD_FLOAT, "angle" ),
	DEFINE_KEYFIELD( m_nSize, FIELD_INTEGER, "size" ),
	DEFINE_KEYFIELD( disableSprite, FIELD_BOOLEAN, "DisableSprite" ),
	DEFINE_KEYFIELD( m_clrOverlay, FIELD_COLOR32, "overlaycolor" ),
	DEFINE_KEYFIELD( m_nOverlaySize, FIELD_INTEGER, "overlaysize" ),
	DEFINE_KEYFIELD( m_strMaterial, FIELD_STRING, "material" ),
	DEFINE_KEYFIELD( m_strOverlayMaterial, FIELD_STRING, "overlaymaterial" ),
	
	// NOT SAVED
	// m_nOverlayMaterial
	// m_nMaterial

	DEFINE_FIELD( m_bOn, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_COLOR32, "SetColor", InputSetColor ),

	DEFINE_KEYFIELD( m_flHDRColorScale,		FIELD_FLOAT,	"HDRColorScale" ),
END_DATADESC()

CSun::CSun()
{
	m_vDirection.Init( 0, 0, 1 );
	
	m_bUseAngles = false;
	m_flPitch = 0;
	m_flYaw = 0;
	m_nSize = 16;

	m_bOn = true;

	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	m_strMaterial = NULL_STRING;
	m_strOverlayMaterial = NULL_STRING;
	m_nOverlaySize = -1;
}

void CSun::Activate()
{
	BaseClass::Activate();
	//SetThink(&CSun::Think);
	//SetNextThink(gpGlobals->curtime + 0.1f);
	// Find our target.
	if (disableSprite)
		m_bOn = !disableSprite;


	if ( m_bUseAngles )
	{
		//if (SunRaysPower > 0)
		SetupLightNormalFromProps(GetAbsAngles(), m_flYaw, m_flPitch, m_vDirection.GetForModify());
		//else
			//SetupLightNormalFromProps( GetAbsAngles(), m_flYaw, m_flPitch, m_vDirection.GetForModify() );
		m_vDirection = -m_vDirection.Get();
	}
	else
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( 0, m_target );
		if( pEnt )
		{
			Vector vDirection = GetAbsOrigin() - pEnt->GetAbsOrigin();
			VectorNormalize( vDirection );
			m_vDirection = vDirection;
		}
	}

	DevMsg("GetAbsAngles().x %2f !\n", GetAbsAngles().x);
	DevMsg("GetAbsAngles().y %2f !\n", GetAbsAngles().y);
	DevMsg("GetAbsAngles().z %2f !\n", GetAbsAngles().z);

	cvar->FindVar("r_post_draw_sunrays_amount")->SetValue(SunRaysIntesity);

	SunRaysPowerOrigin = cvar->FindVar("r_post_draw_sunrays_power")->GetFloat();
	if (SunRaysPower != 0.0f && SunRaysPower != NULL)
		cvar->FindVar("r_post_draw_sunrays_power")->SetValue(SunRaysPower);
	else
		cvar->FindVar("r_post_draw_sunrays_power")->SetValue(SunRaysPowerOrigin);

	cvar->FindVar("r_post_draw_sunrays_color.r")->SetValue(SunRaysColorR);
	cvar->FindVar("r_post_draw_sunrays_color.g")->SetValue(SunRaysColorG);
	cvar->FindVar("r_post_draw_sunrays_color.b")->SetValue(SunRaysColorB);
	cvar->FindVar("r_post_draw_sunrays_color.a")->SetValue(SunRaysColorA);

	// Default behavior
	if ( m_nOverlaySize == -1 )
	{
		m_nOverlaySize = m_nSize;
	}

	// Cache off our image indices
	if ( m_strMaterial == NULL_STRING )
	{
		m_strMaterial = AllocPooledString( "sprites/light_glow02_add_noz.vmt" );
	}
	else 
	{
		const char *pExtension = V_GetFileExtension( STRING( m_strMaterial ) );
		if ( !pExtension )
		{
			char szFixedString[MAX_PATH];
			V_strncpy( szFixedString, STRING( m_strMaterial ), sizeof( szFixedString ) );
			V_strncat( szFixedString, ".vmt", sizeof( szFixedString ) );
			m_strMaterial = AllocPooledString( szFixedString );
		}
	}

	if ( m_strOverlayMaterial == NULL_STRING )
	{
		m_strOverlayMaterial = AllocPooledString( "sprites/light_glow02_add_noz.vmt" );
	}
	else 
	{
		const char *pExtension = V_GetFileExtension( STRING( m_strOverlayMaterial ) );
		if ( !pExtension )
		{
			char szFixedString[MAX_PATH];
			V_strncpy( szFixedString, STRING( m_strOverlayMaterial ), sizeof( szFixedString ) );
			V_strncat( szFixedString, ".vmt", sizeof( szFixedString ) );
			m_strOverlayMaterial = AllocPooledString( szFixedString );
		}
	}

	m_nMaterial = PrecacheModel( STRING( m_strMaterial ) );
	m_nOverlayMaterial = PrecacheModel( STRING( m_strOverlayMaterial ) );
}

/*void CSun::Think(void)
{
	DevMsg("Think...\n");
	m_bUseAngles = true;
	m_flYaw++;
	m_flPitch++;
	m_vDirection.Init(m_flYaw, m_flPitch, 1);
	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.05f);
}*/
void CSun::InputTurnOn( inputdata_t &inputdata )
{
	if( !m_bOn )
	{
		m_bOn = true;
	}
}

void CSun::InputTurnOff( inputdata_t &inputdata )
{
	if ( m_bOn )
	{
		m_bOn = false;
	}
}

void CSun::InputSetColor( inputdata_t &inputdata )
{
	m_clrRender = inputdata.value.Color32();
}

int CSun::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}


