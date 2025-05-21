//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "fx.h"
#include "decals.h"
#include "fx_quad.h"
#include "fx_sparks.h"
#include "dlight.h"
#include "iefx.h"
#include "tier0/vprof.h"
#include "iviewrender_beams.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Handle jeep impacts
//-----------------------------------------------------------------------------
void ImpactJeepCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 2 );
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "ImpactJeep", ImpactJeepCallback );

#include "engine/ivdebugoverlay.h"
//-----------------------------------------------------------------------------
// Purpose: Handle gauss impacts
//-----------------------------------------------------------------------------
void ImpactGaussCallback( const CEffectData &data )
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		//PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 2 );
		QAngle AngDir;
		VectorAngles(vecStart, AngDir);
		//debugoverlay->AddLineOverlay(vecOrigin, vecStart, 0, 255, 0, false, 10);
		//debugoverlay->AddLineOverlay(vecOrigin, -data.m_vNormal, 0, 255, 0, false, 10);
		//VectorAngles(vecShotDir, AngDir);
		DispatchParticleEffect("impact_metal_sparks_gauss", vecOrigin, AngDir);//OverCharged
	}

	PlayImpactSound( pEntity, tr, vecStart, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "ImpactGauss", ImpactGaussCallback );

void ImpactGaussChargedCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		//PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 2 );
		QAngle AngDir;
		VectorAngles(vecStart, AngDir);
		//debugoverlay->AddLineOverlay(vecOrigin, vecStart, 0, 255, 0, false, 10);
		//debugoverlay->AddLineOverlay(vecOrigin, -data.m_vNormal, 0, 255, 0, false, 10);
		//VectorAngles(vecShotDir, AngDir);
		DispatchParticleEffect("gauss_penetration_glow", vecOrigin, AngDir);//OverCharged
	}

	PlayImpactSound(pEntity, tr, vecStart, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("ImpactGaussCharged", ImpactGaussChargedCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void ImpactCallback( const CEffectData &data )
{
	VPROF_BUDGET( "ImpactCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0 );
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "Impact", ImpactCallback );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&normal - 
//			scale - 
//-----------------------------------------------------------------------------
void FX_AirboatGunImpact( const Vector &origin, const Vector &normal, float scale )
{
#ifdef _XBOX

	Vector offset = origin + ( normal * 1.0f );

	CSmartPtr<CTrailParticles> sparkEmitter = CTrailParticles::Create( "FX_MetalSpark 1" );

	if ( sparkEmitter == NULL )
		return;

	//Setup our information
	sparkEmitter->SetSortOrigin( offset );
	sparkEmitter->SetFlag( bitsPARTICLE_TRAIL_VELOCITY_DAMPEN );
	sparkEmitter->SetVelocityDampen( 8.0f );
	sparkEmitter->SetGravity( 800.0f );
	sparkEmitter->SetCollisionDamped( 0.25f );
	sparkEmitter->GetBinding().SetBBox( offset - Vector( 32, 32, 32 ), offset + Vector( 32, 32, 32 ) );

	int	numSparks = random->RandomInt( 4, 8 );

	TrailParticle	*pParticle;
	PMaterialHandle	hMaterial = sparkEmitter->GetPMaterial( "effects/spark" );
	Vector			dir;

	float	length	= 0.1f;

	//Dump out sparks
	for ( int i = 0; i < numSparks; i++ )
	{
		pParticle = (TrailParticle *) sparkEmitter->AddParticle( sizeof(TrailParticle), hMaterial, offset );

		if ( pParticle == NULL )
			return;

		pParticle->m_flLifetime	= 0.0f;
		pParticle->m_flDieTime	= random->RandomFloat( 0.05f, 0.1f );

		float	spreadOfs = random->RandomFloat( 0.0f, 2.0f );

		dir[0] = normal[0] + random->RandomFloat( -(0.5f*spreadOfs), (0.5f*spreadOfs) );
		dir[1] = normal[1] + random->RandomFloat( -(0.5f*spreadOfs), (0.5f*spreadOfs) );
		dir[2] = normal[2] + random->RandomFloat( -(0.5f*spreadOfs), (0.5f*spreadOfs) );

		VectorNormalize( dir );

		pParticle->m_flWidth		= random->RandomFloat( 1.0f, 4.0f );
		pParticle->m_flLength		= random->RandomFloat( length*0.25f, length );

		pParticle->m_vecVelocity	= dir * random->RandomFloat( (128.0f*(2.0f-spreadOfs)), (512.0f*(2.0f-spreadOfs)) );

		Color32Init( pParticle->m_color, 255, 255, 255, 255 );
	}

#else

	// Normal metal spark
	FX_MetalSpark( origin, normal, normal, (int) scale );

#endif // _XBOX

	// Add a quad to highlite the hit point
	FX_AddQuad( origin, 
				normal, 
				random->RandomFloat( 16, 32 ),
				random->RandomFloat( 32, 48 ),
				0.75f, 
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ), 
				0,
				Vector( 1.0f, 1.0f, 1.0f ), 
				0.05f, 
				"effects/combinemuzzle2_nocull",
				(FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA) );
}

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts from the airboat gun shooting (cheaper versions)
//-----------------------------------------------------------------------------
void ImpactAirboatGunCallback( const CEffectData &data )
{
	VPROF_BUDGET( "ImpactAirboatGunCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

	dlight_t *el = effects->CL_AllocDlight(LIGHT_INDEX_MUZZLEFLASH + data.entindex());

	el->origin = data.m_vOrigin;
	el->m_Direction = data.m_vNormal;
	el->color.r = 0;
	el->color.g = random->RandomInt(95, 125);
	el->color.b = random->RandomInt(155, 195);
	el->color.exponent = 1;

	el->radius = random->RandomInt(90, 130);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.1f;

#if !defined( _XBOX )
	// If we hit, perform our custom effects and play the sound. Don't create decals
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS ) )
	{
		FX_AirboatGunImpact( vecOrigin, tr.plane.normal, 2 );
	}
#else
	FX_AirboatGunImpact( vecOrigin, tr.plane.normal, 1 );
#endif
}

DECLARE_CLIENT_EFFECT( "AirboatGunImpact", ImpactAirboatGunCallback );

void LaserImpactCallback(const CEffectData &data)
{
	VPROF_BUDGET("LaserImpactCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		tr.fraction = 1.0;
		//PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

#if !defined( _XBOX )
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS ) )
	{
		FX_LaserSpark(vecOrigin, tr.plane.normal, tr.plane.normal, 2);
	}
#endif
}
DECLARE_CLIENT_EFFECT("LaserSparksImpact", LaserImpactCallback);

void LaserSparksMuzzleCallback(const CEffectData &data)
{
	VPROF_BUDGET("LaserSparksMuzzleCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		tr.fraction = 1.0;
		//PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

#if !defined( _XBOX )
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS ) )
	{
		FX_LaserSparkMuzzle(vecOrigin, vecShotDir, 2);
	}
#endif
}
DECLARE_CLIENT_EFFECT("LaserSparksMuzzle", LaserSparksMuzzleCallback);

void FX_BloodDripsImpact(const Vector &origin, const Vector &normal, float scale)
{
#ifdef _XBOX

	Vector offset = origin + (normal * 1.0f);

	CSmartPtr<CTrailParticles> sparkEmitter = CTrailParticles::Create("FX_MetalSpark 1");

	if (sparkEmitter == NULL)
		return;

	//Setup our information
	sparkEmitter->SetSortOrigin(offset);
	sparkEmitter->SetFlag(bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);
	sparkEmitter->SetVelocityDampen(8.0f);
	sparkEmitter->SetGravity(800.0f);
	sparkEmitter->SetCollisionDamped(0.25f);
	sparkEmitter->GetBinding().SetBBox(offset - Vector(32, 32, 32), offset + Vector(32, 32, 32));

	int	numSparks = random->RandomInt(4, 8);

	TrailParticle	*pParticle;
	PMaterialHandle	hMaterial = sparkEmitter->GetPMaterial("effects/spark");
	Vector			dir;

	float	length = 0.1f;

	//Dump out sparks
	for (int i = 0; i < numSparks; i++)
	{
		pParticle = (TrailParticle *)sparkEmitter->AddParticle(sizeof(TrailParticle), hMaterial, offset);

		if (pParticle == NULL)
			return;

		pParticle->m_flLifetime = 0.0f;
		pParticle->m_flDieTime = random->RandomFloat(0.05f, 0.1f);

		float	spreadOfs = random->RandomFloat(0.0f, 2.0f);

		dir[0] = normal[0] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));
		dir[1] = normal[1] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));
		dir[2] = normal[2] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));

		VectorNormalize(dir);

		pParticle->m_flWidth = random->RandomFloat(1.0f, 4.0f);
		pParticle->m_flLength = random->RandomFloat(length*0.25f, length);

		pParticle->m_vecVelocity = dir * random->RandomFloat((128.0f*(2.0f - spreadOfs)), (512.0f*(2.0f - spreadOfs)));

		Color32Init(pParticle->m_color, 255, 255, 255, 255);
	}

#else

	// Normal metal spark
	FX_BloodDrips(origin, normal, normal, (int)scale);

#endif // _XBOX

	// Add a quad to highlite the hit point
	FX_AddQuad(origin,
		normal,
		random->RandomFloat(16, 32),
		random->RandomFloat(32, 48),
		0.75f,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		0.05f,
		"effects/combinemuzzle2_nocull",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
}

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts from the airboat gun shooting (cheaper versions)
//-----------------------------------------------------------------------------
void BloodDripsImpactCallback(const CEffectData &data)
{
	VPROF_BUDGET("BloodDripsImpactCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		//PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}


#if !defined( _XBOX )
	// If we hit, perform our custom effects and play the sound. Don't create decals
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS))
	{
		FX_BloodDripsImpact(vecOrigin, tr.plane.normal, 2);
	}
#else
	FX_BloodDripsImpact(vecOrigin, tr.plane.normal, 1);
#endif
}

DECLARE_CLIENT_EFFECT("BloodDripsImpact", BloodDripsImpactCallback);

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts from the helicopter shooting (cheaper versions)
//-----------------------------------------------------------------------------
void ImpactHelicopterCallback( const CEffectData &data )
{
	VPROF_BUDGET( "ImpactHelicopterCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
		return;
	}

	// If we hit, perform our custom effects and play the sound. Don't create decals
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS ) )
	{
		FX_AirboatGunImpact( vecOrigin, tr.plane.normal, IsXbox() ? 1 : 2 );

		// Only do metal + computer custom effects
		if ( (iMaterial == CHAR_TEX_METAL) || (iMaterial == CHAR_TEX_COMPUTER) )
		{
			PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, 1.0, FLAGS_CUSTIOM_EFFECTS_NOFLECKS );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "HelicopterImpact", ImpactHelicopterCallback );

void GaussLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("GaussLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(/*LIGHT_INDEX_MUZZLEFLASH + */data.entindex());

	el->origin = data.m_vOrigin;
	//el->m_Direction = data.m_vNormal;
	el->color.r = data.m_vStart.x;
	el->color.g = data.m_vStart.y;
	el->color.b = data.m_vStart.z;
	el->color.exponent = 5;

	el->radius = random->RandomInt(40, 60) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.2f;

}

DECLARE_CLIENT_EFFECT("GaussLight", GaussLightCallback);

void GaussLightChargedCallback(const CEffectData &data)
{
	VPROF_BUDGET("GaussLightChargedCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(/*LIGHT_INDEX_MUZZLEFLASH + */data.entindex());

	el->origin = data.m_vOrigin;
	//el->m_Direction = data.m_vNormal;
	el->color.r = 255;
	el->color.g = 255;
	el->color.b = 255;
	el->color.exponent = 5;

	el->radius = random->RandomInt(60, 80) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.2f;

}

DECLARE_CLIENT_EFFECT("GaussLightCharged", GaussLightChargedCallback);

void GaussWeaponLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("GaussWeaponLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	IClientRenderable *pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		dlight_t *el = effects->CL_AllocDlight(data.entindex());

		el->origin = position;

		el->color.r = data.m_vStart.x;
		el->color.g = data.m_vStart.y;
		el->color.b = data.m_vStart.z;
		el->color.exponent = 5;

		el->radius = random->RandomInt(70, 100) * (engine->MapHasHDRLighting() ? 1 : 0.3);
		el->decay = el->radius / 0.1f;
		el->die = gpGlobals->curtime + 0.2f;


		/*BeamInfo_t beamInfo;

		//beamInfo.m_vecStart = vEnd;
		beamInfo.m_vecStart = position;
		//beamInfo.m_pEndEnt = pRenderable;
		beamInfo.m_nStartAttachment = data.m_nAttachmentIndex;

		Vector	offset = RandomVector(-12, 12);

		offset += Vector(2, 2, 2);
		beamInfo.m_vecEnd = position + offset;

		beamInfo.m_nType = TE_BEAMTESLA;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.05f;
		beamInfo.m_flWidth = random->RandomFloat(1.5f, 3.0f);
		beamInfo.m_flEndWidth = 0;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = random->RandomFloat(1, 2);
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 1.0f;
		beamInfo.m_flRed = 255;
		beamInfo.m_flGreen = random->RandomInt(140, 140);
		beamInfo.m_flBlue = random->RandomInt(0, 5);
		beamInfo.m_nSegments = 14;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE | FBEAM_SHADEOUT);

		beams->CreateBeamPoints(beamInfo);*/
	}
}

DECLARE_CLIENT_EFFECT("GaussWeaponLight", GaussWeaponLightCallback);

void ExplosionLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("ExplosionLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(data.entindex());

	el->origin = data.m_vOrigin;
	//el->m_Direction = data.m_vNormal;
	el->color.r = 255;
	el->color.g = random->RandomInt(200, 255);
	el->color.b = random->RandomInt(0, 5);
	el->color.exponent = 6;

	el->radius = random->RandomInt(280, 340) * (engine->MapHasHDRLighting() ? 1 : 0.5);
	el->decay = el->radius / 0.8f;
	el->die = gpGlobals->curtime + 3.1f;

}

DECLARE_CLIENT_EFFECT("ExplosionLight", ExplosionLightCallback);

void EgonWeaponLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("EgonWeaponLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(data.entindex());

	el->origin = data.m_vOrigin;

	el->color.r = data.m_vStart.x;
	el->color.g = data.m_vStart.y;
	el->color.b = data.m_vStart.z;
	el->color.exponent = 5;

	el->radius = random->RandomInt(50, 80) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.2f;	
}

DECLARE_CLIENT_EFFECT("EgonWeaponLight", EgonWeaponLightCallback);

void ImmolatorWeaponLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("ImmolatorWeaponLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(data.entindex());

	el->origin = data.m_vOrigin;

	el->color.r = data.m_vStart.x;
	el->color.g = data.m_vStart.y;
	el->color.b = data.m_vStart.z;
	el->color.exponent = 5;

	el->radius = random->RandomInt(70, 100) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.2f;
}

DECLARE_CLIENT_EFFECT("ImmolatorWeaponLight", ImmolatorWeaponLightCallback);

void LaserWeaponLightCallback(const CEffectData &data)
{
	VPROF_BUDGET("LaserWeaponLightCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	dlight_t *el = effects->CL_AllocDlight(data.entindex());

	el->origin = data.m_vOrigin;

	el->color.r = data.m_vStart.x;
	el->color.g = data.m_vStart.y;
	el->color.b = data.m_vStart.z;
	el->color.exponent = 0;

	el->radius = random->RandomInt(85, 125) * (engine->MapHasHDRLighting() ? 1 : 0.3);
	el->decay = el->radius / 0.1f;
	el->die = gpGlobals->curtime + 0.2f;
}

DECLARE_CLIENT_EFFECT("LaserWeaponLight", LaserWeaponLightCallback);