//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "ClientEffectPrecacheSystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"

#include "dlight.h"
#include "r_efx.h"

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectGreenBall )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()

//
// GreenBall Projectile
//

class C_GreenBallProjectile : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_GreenBallProjectile, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_GreenBallProjectile( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

private:

	C_GreenBallProjectile( const C_GreenBallProjectile & ); // not defined, not accessible

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_GreenBallProjectile, DT_GreenBallProjectile, CGreenBallProjectile )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_GreenBallProjectile::C_GreenBallProjectile( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_GreenBallProjectile::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_GreenBallProjectile::DrawModel( int flags )
{
	// See if we're drawing the motion blur
	if ( flags & STUDIO_TRANSPARENCY )
	{
		float		color[3];
		IMaterial	*pBlurMaterial = materials->FindMaterial( "effects/muzzleflash1", NULL, false );

		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		if ( speed > 0 )
		{
			float	stepSize = min( ( speed * 0.5f ), 4.0f );

			Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
			Vector	spawnStep = -vecDir * stepSize;

			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->Bind( pBlurMaterial );

			float	alpha;

			// Draw the motion blurred trail
			for ( int i = 0; i < 20; i++ )
			{
				spawnPos += spawnStep;

				alpha = RemapValClamped( i, 5, 11, 0.25f, 0.05f );

				color[0] = color[1] = color[2] = alpha;

				DrawHalo( pBlurMaterial, spawnPos, 3.0f, color );
			}

			{
				dlight_t *dl = effects->CL_AllocDlight(index);	// BriJee : dlight
				dl->origin = spawnPos;	//effect_origin;
				dl->color.r = 0;
				dl->color.g = 255;
				dl->color.b = 0;
				dl->color.exponent = 5;
				dl->radius = 150.0f * (engine->MapHasHDRLighting() ? 1 : 0.3);
				dl->die = gpGlobals->curtime + 0.1f; //0.2f;
				dl->decay = 512.0f;//0.05f; //0.0f;
			}
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}

	// Draw the normal portion
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_GreenBallProjectile::ClientThink( void )
{
	m_bUpdated = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
/*void CrosshairLoadCallbackGreenBall( const CEffectData &data )
{
	IClientRenderable *pRenderable = data.GetRenderable( );
	if ( !pRenderable )
		return;
	
	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if ( pRenderable->GetAttachment( data.m_nAttachmentIndex, position, angles ) )
	{
		FX_ElectricSpark( position, 1.0f, 1.0f, NULL );
	}
}*/

//DECLARE_CLIENT_EFFECT( "CrossbowLoad", CrosshairLoadCallbackGreenBall );