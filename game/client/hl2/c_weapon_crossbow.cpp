//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()

//
// Crossbow bolt
//

class C_CrossbowBolt : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_CrossbowBolt, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_CrossbowBolt( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

	//IMaterial* m_pWireframe;
private:

	C_CrossbowBolt( const C_CrossbowBolt & ); // not defined, not accessible

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_CrossbowBolt, DT_CrossbowBolt, CCrossbowBolt )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CrossbowBolt::C_CrossbowBolt( void )
{
	//m_pWireframe = materials->FindMaterial("shadertest/wireframevertexcolor", TEXTURE_GROUP_OTHER);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::OnDataChanged( DataUpdateType_t updateType )
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
int C_CrossbowBolt::DrawModel( int flags )
{
	// See if we're drawing the motion blur
	/*if ( flags & STUDIO_TRANSPARENCY )
	{
		float		color[3];
		IMaterial	*pBlurMaterial = materials->FindMaterial( "effects/muzzleflash1", NULL, false );

		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		if ( speed > 0 )
		{
			float	stepSize = MIN( ( speed * 0.5f ), 4.0f );

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
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}*/

	// Draw the normal portion
	return BaseClass::DrawModel( flags );


}

/*double noise2(double arg)
{
	return (arg + random->RandomFloat(-1.5f, 1.5f));
}
static void RandomizeNormal1(Vector &vec)
{
	vec.x = 2.0f * (noise2(vec.x) - 0.5f);
	vec.y = 2.0f * (noise2(vec.y) - 0.5f);
	vec.z = 2.0f * (noise2(vec.z) - 0.5f);
}*/


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::ClientThink( void )
{
	m_bUpdated = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CrosshairLoadCallback( const CEffectData &data )
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


	/*
	Vector origin = position;
	int stacks = 200;
	int slices = 200;
	float radius = 5000.0f;
	// this sucks and stuff
	float x = origin.x;
	float y = origin.y;
	float z = origin.z;
	float stackAngle, sliceAngle;
	int stack, slice;
	Vector v[4];
	float	sliced, stacked, sliced1, stacked1, stacks1;
	float	slicedsin, slicedcos, stackedsin, stackedcos;
	float	sliced1sin, sliced1cos, stacked1sin, stacked1cos;
	float	stacks1sin, stacks1cos;
	float	stacksin, stackcos;

	IMaterial *pMaterial = materials->FindMaterial("effects/splashwake4", 0);
	CMatRenderContextPtr pRenderContext(materials);
	IMesh *pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, pMaterial);
	CMeshBuilder meshBuilder;

	stackAngle = M_PI / (float)stacks;
	sliceAngle = 2.0 * M_PI / (float)slices;

	for (stack = 1; stack < stacks - 1; stack++)
	{
		for (slice = 0; slice < slices; slice++)
		{
			int i, j;

			sliced = sliceAngle * slice;
			stacked = stackAngle * stack;
			sliced1 = sliceAngle * (slice + 1);
			stacked1 = stackAngle * (stack + 1);

			SinCos(sliced, &slicedsin, &slicedcos);
			SinCos(stacked, &stackedsin, &stackedcos);
			SinCos(sliced1, &sliced1sin, &sliced1cos);
			SinCos(stacked1, &stacked1sin, &stacked1cos);

			v[0][0] = -slicedsin * stackedsin;
			v[0][1] = slicedcos * stackedsin;
			v[0][2] = stackedcos;

			v[1][0] = -sliced1sin * stackedsin;
			v[1][1] = sliced1cos * stackedsin;
			v[1][2] = stackedcos;

			v[2][0] = -sliced1sin * stacked1sin;
			v[2][1] = sliced1cos * stacked1sin;
			v[2][2] = stacked1cos;

			v[3][0] = -slicedsin * stacked1sin;
			v[3][1] = slicedcos * stacked1sin;
			v[3][2] = stacked1cos;

			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 3; j++)
				{
					v[i][j] *= radius;
				}
				v[i][0] += x;
				v[i][1] += y;
				v[i][2] += z;
			}

#if 1
			//			if( drawWireframe.value )
			if (1)
			{
				meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

				meshBuilder.Position3fv(v[0].Base());
				Vector normal;
				normal = v[0] - origin;
				VectorNormalize(normal);
				RandomizeNormal1(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[1].Base());
				normal = v[1] - origin;
				VectorNormalize(normal);
				RandomizeNormal1(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[2].Base());
				normal = v[2] - origin;
				VectorNormalize(normal);
				RandomizeNormal1(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[3].Base());
				normal = v[3] - origin;
				VectorNormalize(normal);
				RandomizeNormal1(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.End();
				pMesh->Draw();
			}
			else
			{
				//				DrawIndexedQuad( v, 0, 1, 2, 3 );
			}
#endif
		}
	}

	// do the caps
	for (slice = 0; slice < slices; slice++)
	{
		int i, j;

		sliced = sliceAngle * slice;
		stacked = stackAngle * stack;
		sliced1 = sliceAngle * (slice + 1);
		stacked1 = stackAngle * (stack + 1);
		stacks1 = stackAngle * (stacks - 1);

		SinCos(sliced, &slicedsin, &slicedcos);
		SinCos(stacked, &stackedsin, &stackedcos);
		SinCos(sliced1, &sliced1sin, &sliced1cos);
		SinCos(stacked1, &stacked1sin, &stacked1cos);
		SinCos(stackAngle, &stacksin, &stackcos);
		SinCos(stacks1, &stacks1sin, &stacks1cos);

		v[0][0] = 0.0f;
		v[0][1] = 0.0f;
		v[0][2] = 1.0f;

		v[1][0] = -sliced1sin * stacksin;
		v[1][1] = sliced1cos * stacksin;
		v[1][2] = stackcos;

		v[2][0] = -slicedsin * stacksin;
		v[2][1] = slicedcos * stacksin;
		v[2][2] = stackcos;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				v[i][j] *= radius;
			}
			v[i][0] += x;
			v[i][1] += y;
			v[i][2] += z;
		}

		meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 1);

		meshBuilder.Position3fv(v[0].Base());
		Vector normal;
		normal = v[0] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[1].Base());
		normal = v[1] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[2].Base());
		normal = v[2] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();

		v[0][0] = 0.0f;
		v[0][1] = 0.0f;
		v[0][2] = -1.0f;

		v[1][0] = -sliced1sin * stacks1sin;
		v[1][1] = sliced1cos * stacks1sin;
		v[1][2] = stacks1cos;

		v[2][0] = -slicedsin * stacks1sin;
		v[2][1] = slicedcos * stacks1sin;
		v[2][2] = stacks1cos;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				v[i][j] *= radius;
			}
			v[i][0] += x;
			v[i][1] += y;
			v[i][2] += z;
		}

		meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 1);

		meshBuilder.Position3fv(v[0].Base());
		normal = v[0] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[2].Base());
		normal = v[2] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[1].Base());
		normal = v[1] - origin;
		VectorNormalize(normal);
		RandomizeNormal1(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}*/
}

DECLARE_CLIENT_EFFECT( "CrossbowLoad", CrosshairLoadCallback );
