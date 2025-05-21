//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 0 );
	}
}

DECLARE_CLIENT_EFFECT( "ShellEject", ShellEjectCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RifleShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 1 );
	}
}

DECLARE_CLIENT_EFFECT( "RifleShellEject", RifleShellEjectCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShotgunShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 2 );
	}
}

DECLARE_CLIENT_EFFECT( "ShotgunShellEject", ShotgunShellEjectCallback );

void Ar2ShellEjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 3);
	}
}

DECLARE_CLIENT_EFFECT("Ar2ShellEject", Ar2ShellEjectCallback);


///////////////////OverCharged MAGAZINES///////////////////////////
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SMG_MAG_EjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrassMag(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 3);
	}
}

DECLARE_CLIENT_EFFECT("SMG_MAG_Eject", SMG_MAG_EjectCallback);

void Rifle_MAG_EjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrassMag(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 5);
	}
}

DECLARE_CLIENT_EFFECT("Rifle_MAG_Eject", Rifle_MAG_EjectCallback);

void Pistol_MAG_EjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrassMag(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 4);
	}
}

DECLARE_CLIENT_EFFECT("Pistol_MAG_Eject", Pistol_MAG_EjectCallback);