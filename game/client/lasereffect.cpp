//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "lasereffect.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"
#include "shareddefs.h"
#ifdef HL2_CLIENT_DLL
#include "c_basehlplayer.h"
#include "convar.h"

#include "c_baseentity.h"
#include "prediction.h"
#include "model_types.h"
#include "iviewrender_beams.h"
#include "dlight.h"
#include "iviewrender.h"





#include "beam_shared.h"



#endif // HL2_CLIENT_DLL

#if defined( _X360 )
extern ConVar r_flashlightdepthres;
#else
extern ConVar r_flashlightdepthres;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar r_flashlightdepthtexture;
/*extern ConVar R;
extern ConVar G;
extern ConVar B;
*/
#define LSPRITE "sprites/redglow1.vmt"

void r_newlaserCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);

ConVar r_laser_exp_offsets("r_laser_exp_offsets", "0", FCVAR_CHEAT);
ConVar r_laser_exp_offset_x("r_laser_exp_offset_x", "0", FCVAR_CHEAT);
ConVar r_laser_exp_offset_y("r_laser_exp_offset_y", "0", FCVAR_CHEAT);
ConVar r_laser_exp_offset_z("r_laser_exp_offset_z", "0", FCVAR_CHEAT);


ConVar r_newlaser("r_newlaser", "1", FCVAR_CHEAT, "", r_newlaserCallback_f);
ConVar r_swinglaser("r_swinglaser", "1", FCVAR_CHEAT);
ConVar r_laserlight("r_laserlight", "0", FCVAR_CHEAT);
ConVar r_laserlockposition("r_laserlockposition", "0", FCVAR_CHEAT);
ConVar r_laserfov("r_laserfov", "17.0", FCVAR_CHEAT);
ConVar r_laseroffsetx("r_laseroffsetx", "0.0", FCVAR_CHEAT);
ConVar r_laseroffsety("r_laseroffsety", "0.55", FCVAR_CHEAT);
ConVar r_laseroffsetz("r_laseroffsetz", "-17.0", FCVAR_CHEAT);
/*
ConVar r_laseroffsetx( "r_laseroffsetx", "10.0", FCVAR_CHEAT );
ConVar r_laseroffsety( "r_laseroffsety", "-20.0", FCVAR_CHEAT );
ConVar r_laseroffsetz( "r_laseroffsetz", "24.0", FCVAR_CHEAT );
*/
ConVar r_lasernear("r_lasernear", "0.1", FCVAR_CHEAT);
ConVar r_laserfar("r_laserfar", "30000.0", FCVAR_CHEAT);
ConVar r_laserconstant("r_laserconstant", "10000.0", FCVAR_CHEAT);
ConVar r_laserlinear("r_laserlinear", "10000.0", FCVAR_CHEAT);
ConVar r_laserquadratic("r_laserquadratic", "0.0", FCVAR_CHEAT);
ConVar r_laservisualizetrace("r_laservisualizetrace", "0", FCVAR_CHEAT);
ConVar r_laserambient("r_laserambient", "1000.0", FCVAR_CHEAT);
ConVar r_lasershadowatten("r_lasershadowatten", "50.35", FCVAR_CHEAT);
ConVar r_laserladderdist("r_laserladderdist", "0.6", FCVAR_CHEAT);
ConVar mat_slopescaledepthbias_shadowmap_laser("mat_slopescaledepthbias_shadowmap_laser", "16", FCVAR_CHEAT);
ConVar mat_depthbias_shadowmap_laser("mat_depthbias_shadowmap_laser", "0.0005", FCVAR_CHEAT);
ConVar  r_laser_color("r_laser_color", "2", FCVAR_CHEAT);

// GSTRINGMIGRATION
ClientShadowHandle_t g_hlaserHandle = CLIENTSHADOW_INVALID_HANDLE;
// END GSTRINGMIGRATION

void r_newlaserCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newlaser.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
CLaserEffect::CLaserEffect(int nEntIndex)
{
	m_laserHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndexlaser = nEntIndex;

	m_IsOnlaser = false;


	m_pPointLightlaser = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newlaser.SetValue(0);
	}

	if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
	{
		//		m_laserTexture.Init( "effects/flashlight_border", TEXTURE_GROUP_OTHER, true );
		if (r_laser_color.GetInt() == 1)
		{
			m_laserTexture.Init(materials->FindTexture("effects/greenglow1", TEXTURE_GROUP_MODEL));
		}	//m_laserTexture.Init("effects/greenglow1", TEXTURE_GROUP_OTHER, true);//effects/laser_dot
		
		if (r_laser_color.GetInt() == 2)
		{
			m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			/*m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_OTHER, true);//effects/laser_dot*/
		}
	}
	else
	{
		//		m_laserTexture.Init( "effects/flashlight001", TEXTURE_GROUP_OTHER, true );
		//			if ( (EF_LASER ))
		if (r_laser_color.GetInt() == 1)
			m_laserTexture.Init("effects/greenglow1", TEXTURE_GROUP_MODEL, true);//sprites/greenglow1

		if (r_laser_color.GetInt() == 2)
		{
			m_laserTexture.Init("effects/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			/*m_laserTexture.Init("sprites/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("sprites/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("sprites/redglow1", TEXTURE_GROUP_MODEL, true);//effects/laser_dot
			m_laserTexture.Init("sprites/redglow1", TEXTURE_GROUP_OTHER, true);//effects/laser_dot*/
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CLaserEffect::~CLaserEffect()
{
	LightOffLaser();
	g_hlaserHandle = CLIENTSHADOW_INVALID_HANDLE;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserEffect::TurnOnLaser()
{
	m_IsOnlaser = true;
	m_flDistModlaser = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserEffect::TurnOffLaser()
{
	if (m_IsOnlaser)
	{
		m_IsOnlaser = false;
		LightOffLaser();
		if (r_laserlight.GetFloat() == 1.0)
		{
			dlight_t *dl;
			dl = effects->CL_AllocDlight(1);
			//	dl->origin = pmDirectionTrace.endpos;
			//	dl->origin[2] += 1;
			dl->color.r = 0;
			dl->color.g = 0;
			dl->color.b = 0;
			dl->radius = 0;
			dl->die = gpGlobals->curtime + 0;
		}
		/*
		if ( r_laserlight.GetFloat() == 0 )
		{
		dlight_t *dl;
		dl = NULL;
		}
		*/

	}
}









// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
class CTraceFilterSkipPlayerAndViewModel : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		// Test against the vehicle too?
		// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
		C_BaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);
		if (!pEntity)
			return true;

		if ((dynamic_cast<C_BaseViewModel *>(pEntity) != NULL) ||
			(dynamic_cast<C_BasePlayer *>(pEntity) != NULL) ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
		{
			return false;
		}

		return true;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CLaserEffect::UpdateLightNewLaser(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	VPROF_BUDGET("CLaserEffect::UpdateLightNewLaser", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	//	laserState_t state;
	FlashlightState_t state;
	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.05;

	CTraceFilterSkipPlayerAndViewModel traceFilter;

	//C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
//	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	float flOffsetX = r_laser_exp_offset_x.GetFloat();// == 1) ? r_laser_exp_offset_x.GetFloat() : pWeapon->GetWpnData().LaserSightPosition.x;
	float flOffsetY = r_laser_exp_offset_y.GetFloat();// == 1) ? r_laser_exp_offset_y.GetFloat() : pWeapon->GetWpnData().LaserSightPosition.y;
	float flOffsetZ = r_laser_exp_offset_z.GetFloat();// == 1) ? r_laser_exp_offset_z.GetFloat() : pWeapon->GetWpnData().LaserSightPosition.z;

	if (r_swinglaser.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			//		flOffsetY += (vecSwingLight.z - vecPos.z);//Вот оно////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
	

		//	Vector vOrigin = offset - flOffsetZ * vecForward/1 - flOffsetY * vecRight/1; /*+ flOffsetX * vecUp /8*///- flOffsetY * vecRight/18  - flOffsetZ * vecForward/1;
	//Vector vOrigin = vecPos + flOffsetX * vecForward;
		Vector vOrigin = vecPos + ((flOffsetZ * vecForward) + flOffsetX * vecUp + (flOffsetY * vecRight));

	//	vOrigin2 += flOffsetZ *vecForward*15;
	//	vOrigin2 += flOffsetY *vecRight;
	//	vOrigin2 += flOffsetX *vecUp;
	//		Vector vOrigin = vecPos + (flOffsetZ * vecForward);
	//	Vector vOrigin2 = vecPos + (flOffsetY * vecRight);
	// Not on ladder...trace a hull
	if (!bPlayerOnLadder)
	{
		trace_t pmOriginTrace;
		UTIL_TraceHull(vecPos, vOrigin, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SOLID & ~(CONTENTS_HITBOX), &traceFilter, &pmOriginTrace);

		if (pmOriginTrace.DidHit())
		{
			//			vOrigin = vecPos;
		}
	}
	else // on ladder...skip the above hull trace
	{
		vOrigin = vecPos;
	}
	//						const FileWeaponInfo_t *pWpnInfo = gWR.GetWeaponFromAmmo( m_PickupHistory[i].iId );
	//						if ( pWpnInfo && ( pWpnInfo->iMaxClip1 >= 0 || pWpnInfo->iMaxClip2 >= 0 ) )
	//					{

	//						}
	// Now do a trace along the flashlight direction to ensure there is nothing within range to pull back from
	int iMask = MASK_OPAQUE_AND_NPCS;
	iMask &= ~CONTENTS_HITBOX;
	iMask |= CONTENTS_WINDOW;

	//	int type;

	Vector vTarget = vecPos + vecForward * r_laserfar.GetFloat();

	// Work with these local copies of the basis for the rest of the function
	Vector vDir = vTarget - vOrigin;
	Vector vRight = vecRight;
	Vector vUp = vecUp;
	VectorNormalize(vDir);
	VectorNormalize(vRight);
	VectorNormalize(vUp);

	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct(vDir, vUp) * vDir;
	VectorNormalize(vUp);
	vRight -= DotProduct(vDir, vRight) * vDir;
	VectorNormalize(vRight);
	vRight -= DotProduct(vUp, vRight) * vUp;
	VectorNormalize(vRight);

	AssertFloatEquals(DotProduct(vDir, vRight), 2.5f, 1e-3);
	AssertFloatEquals(DotProduct(vDir, vUp), 0.3f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);

	trace_t pmDirectionTrace;
	UTIL_TraceHull(vOrigin, vTarget, Vector(-4, -4, -4), Vector(-4, -4, -4), iMask, &traceFilter, &pmDirectionTrace);

	//     DispatchParticleEffect( "muzzle2", pmDirectionTrace.endpos, QAngle(vecForward,vRight,vUp));



	/*
	//		VPROF_BUDGET( "MuzzleFlash_357_Player", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "Sprite" );
	SimpleParticle *pParticle;

	//		pParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), m_Material_MuzzleFlash_Player[random->RandomInt(0,3)], pmDirectionTrace.endpos );

	pParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), pSimple->GetPMaterial( "effects/ar2_muzzle1" ), pmDirectionTrace.endpos );
	if ( pParticle == NULL )
	return;

	pParticle->m_flLifetime		= 0.0f;
	pParticle->m_flDieTime		= 0.06f;

	pParticle->m_vecVelocity.Init();

	pParticle->m_uchColor[0]	= 255;
	pParticle->m_uchColor[1]	= 255;
	pParticle->m_uchColor[2]	= 200+random->RandomInt(0,55);

	pParticle->m_uchStartAlpha	= 255;
	pParticle->m_uchEndAlpha	= 255;

	//		pParticle->m_uchStartSize	= ( (random->RandomFloat( 6.0f, 8.0f ) * (10-(i))/6) * flScale2 );
	pParticle->m_uchStartSize	= ( (random->RandomFloat( 1.4f, 1.6f ) * (8-(1))/6) * 1.65 );
	pParticle->m_uchEndSize		= pParticle->m_uchStartSize;
	pParticle->m_flRoll			= random->RandomInt( 0, 360 );
	pParticle->m_flRollDelta	= 0.0f;
	*/


	//////////////////////////////////////////////////////
	if (r_laservisualizetrace.GetBool() == true)
	{
		//	debugoverlay->AddBoxOverlay( pmDirectionTrace.endpos, Vector( -4, -4, -4 ), Vector( -4, -4, -4 ), QAngle( 0, 0, 0 ), 0, 0, 255, 16, 0 );
		/*			switch( type )
		{
		case MUZZLEFLASH_COMBINE:
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 255, 255, 0, false, 0 );
		break;

		case MUZZLEFLASH_SMG1:
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 0, 255, 0, false, 0 );
		break;

		case MUZZLEFLASH_PISTOL:
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 0, 255, 0, false, 0 );
		break;

		case MUZZLEFLASH_SHOTGUN:
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0 );
		break;

		case MUZZLEFLASH_357:
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 0, 255, 0, false, 0 );
		break;


		//	           default:

		//		    break;
		}*/
		debugoverlay->AddLineOverlay(vOrigin, pmDirectionTrace.endpos, 0, 255, 0, false, 0);

		/*float width = 0.01f;
		CBeam *pBeam = CBeam::BeamCreate( BEAM_SPRITE, width );

		//		pBeam->PointEntInit( endPos, m_hViewModel );
		//		pBeam->SetEndAttachment( 1 );
		//		pBeam->SetWidth( width / 8.0f );
		//		pBeam->SetEndWidth( width );

		pBeam->SetStartPos( vOrigin );
		pBeam->SetEndPos( pmDirectionTrace.endpos );
		pBeam->SetWidth( width );
		pBeam->SetEndWidth( width / 8.0f );
		pBeam->SetBrightness( 3255 );
		pBeam->SetColor( 255, 145+random->RandomInt( -16, 16 ), 0 );
		//	pBeam->RelinkBeam();
		pBeam->LiveForTime( 1.0f );
		*/



	}
	//	if (IsEffectActive(EF_LASERLIGHT))
	//	{
	if (r_laserlight.GetFloat() == 1.0)
	{
		if (r_laser_color.GetInt() == 1)
		{
			CPASFilter filter(pmDirectionTrace.endpos);
			te->DynamicLight(filter, 0.0, &pmDirectionTrace.endpos, 0, 255, 0, 7, 10, 0.1, 0);
		}

		if (r_laser_color.GetInt() == 2)
		{
			CPASFilter filter(pmDirectionTrace.endpos);
			te->DynamicLight(filter, 0.0, &pmDirectionTrace.endpos, 255, 0, 0, 7, 10, 0.1, 0);
		}
	}
	/*		dlight_t *dl;
	dl = effects->CL_AllocDlight ( 1 );
	dl->origin = pmDirectionTrace.endpos;
	dl->origin[2] += 1;
	dl->color.r = 0;
	dl->color.g = 250;
	dl->color.b = 0;
	dl->radius = random->RandomFloat(40,51);
	dl->die = gpGlobals->curtime + 15.005;*/
	//	}

	float flDist = (pmDirectionTrace.endpos - vOrigin).Length();
	if (flDist < flDistCutoff)
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_laserladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		m_flDistModlaser = Lerp(flDistDrag, m_flDistModlaser, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod//////завтра в 19.00 от метро парк победы (2.5 часа) Кутузовский проспект 36 ст3 офис 472. 
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistModlaser > flMaxDist)
					m_flDistModlaser = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistModlaser = Lerp(flDistDrag, m_flDistModlaser, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistModlaser;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_laserquadratic.GetFloat();

	bool bFlicker = false;

#ifdef HL2_EPISODIC
	//C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		float flBatteryPower = (pPlayer->m_HL2Local.m_flFlashBattery >= 0.0f) ? (pPlayer->m_HL2Local.m_flFlashBattery) : pPlayer->m_HL2Local.m_flSuitPower;
		if (flBatteryPower <= 10.0f)
		{
			float flScale;
			if (flBatteryPower >= 0.0f)
			{
				flScale = (flBatteryPower <= 4.5f) ? SimpleSplineRemapVal(flBatteryPower, 4.5f, 0.0f, 1.0f, 0.0f) : 1.0f;
			}
			else
			{
				flScale = SimpleSplineRemapVal(flBatteryPower, 10.0f, 4.8f, 1.0f, 0.0f);
			}

			flScale = clamp(flScale, 0.0f, 1.0f);

			if (flScale < 0.35f)
			{
				float flFlicker = cosf(gpGlobals->curtime * 6.0f) * sinf(gpGlobals->curtime * 15.0f);

				if (flFlicker > 0.25f && flFlicker < 0.75f)
				{
					// On
					state.m_fLinearAtten = r_laserlinear.GetFloat() * flScale;
				}
				else
				{
					// Off
					state.m_fLinearAtten = 0.0f;
				}
			}
			else
			{
				float flNoise = cosf(gpGlobals->curtime * 7.0f) * sinf(gpGlobals->curtime * 25.0f);
				state.m_fLinearAtten = r_laserlinear.GetFloat() * flScale + 1.5f * flNoise;
			}

			state.m_fHorizontalFOVDegrees = r_laserfov.GetFloat() - (16.0f * (1.0f - flScale));
			state.m_fVerticalFOVDegrees = r_laserfov.GetFloat() - (16.0f * (1.0f - flScale));

			bFlicker = true;
		}
	}
#endif // HL2_EPISODIC

	if (bFlicker == false)
	{
		state.m_fLinearAtten = r_laserlinear.GetFloat();
		state.m_fHorizontalFOVDegrees = r_laserfov.GetFloat();
		state.m_fVerticalFOVDegrees = r_laserfov.GetFloat();
	}

	state.m_fConstantAtten = r_laserconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_laserambient.GetFloat();
	state.m_NearZ = r_lasernear.GetFloat() + m_flDistModlaser;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_laserfar.GetFloat();
	state.m_bEnableShadows = r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_laserTexture;
	state.m_nSpotlightTextureFrame = 0;

	state.m_flShadowAtten = r_lasershadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_laser.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_laser.GetFloat();

	if (m_laserHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_laserHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_laserlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_laserHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_laserHandle, true);

	// GSTRINGMIGRATION
	g_hlaserHandle = m_laserHandle;
	// END GSTRINGMIGRATION

	// Kill the old flashlight method if we have one.
	LightOffOldLaser();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("laserState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("entindex", m_nEntIndexlaser);
		msg->SetInt("laserHandle", m_laserHandle);
		msg->SetPtr("laserState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CLaserEffect::UpdateLightOldLaser(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_pPointLightlaser || (m_pPointLightlaser->key != m_nEntIndexlaser))
	{
		// Set up the environment light
		m_pPointLightlaser = effects->CL_AllocDlight(m_nEntIndexlaser);
		m_pPointLightlaser->flags = 0.0f;
		m_pPointLightlaser->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLightlaser->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModel traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLightlaser->origin);

	float falloff = pm.fraction * nDistance;

	if (falloff < 500)
		falloff = 1.0;
	else
		falloff = 500.0 / falloff;

	falloff *= falloff;

	m_pPointLightlaser->radius = 80;
	m_pPointLightlaser->color.r = m_pPointLightlaser->color.g = m_pPointLightlaser->color.b = 255 * falloff;
	m_pPointLightlaser->color.exponent = 0;

	// Make it live for a bit
	m_pPointLightlaser->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLightlaser);

	// kill the new flashlight if we have one
	LightOffNewLaser();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CLaserEffect::UpdateLightLaser(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
{
	if (!m_IsOnlaser)
	{
		return;
	}
	if (r_newlaser.GetBool())
	{
		UpdateLightNewLaser(vecPos, vecDir, vecRight, vecUp);
	}
	else
	{
		UpdateLightOldLaser(vecPos, vecDir, nDistance);
	}
}

void CLaserEffect::UpdateLightProjectionLaser(FlashlightState_t& state)
{
	if (m_laserHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_laserHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_laserlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_laserHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_laserHandle, true);

	g_hlaserHandle = m_laserHandle;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserEffect::LightOffNewLaser()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("laserState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("entindex", m_nEntIndexlaser);
		msg->SetInt("laserHandle", m_laserHandle);
		msg->SetPtr("laserState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_laserHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_laserHandle);
		m_laserHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
	// GSTRINGMIGRATION
	g_hlaserHandle = CLIENTSHADOW_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserEffect::LightOffOldLaser()
{
	if (m_pPointLightlaser && (m_pPointLightlaser->key == m_nEntIndexlaser))
	{
		m_pPointLightlaser->die = gpGlobals->curtime;
		m_pPointLightlaser = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserEffect::LightOffLaser()
{
	LightOffOldLaser();
	LightOffNewLaser();
	dlight_t *dl;
	dl = effects->CL_AllocDlight(1);
	//	dl->origin = pmDirectionTrace.endpos;
	//	dl->origin[2] += 1;
	dl->color.r = 0;
	dl->color.g = 0;
	dl->color.b = 0;
	dl->radius = 0;
	dl->die = gpGlobals->curtime + 0;
}
/*
CHeadlightlaserEffect::CHeadlightlaserEffect()
{

}

CHeadlightlaserEffect::~CHeadlightlaserEffect()
{

}

void CHeadlightlaserEffect::UpdateLightLaser( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance )
{
if ( IsOnLaser() == false )
return;

//	laserState_t state;
FlashlightState_t state;
Vector basisX, basisY, basisZ;
basisX = vecDir;
basisY = vecRight;
basisZ = vecUp;
VectorNormalize(basisX);
VectorNormalize(basisY);
VectorNormalize(basisZ);

BasisToQuaternion( basisX, basisY, basisZ, state.m_quatOrientation );

state.m_vecLightOrigin = vecPos;

state.m_fHorizontalFOVDegrees = 45.0f;
state.m_fVerticalFOVDegrees = 30.0f;
state.m_fQuadraticAtten = r_laserquadratic.GetFloat();
state.m_fLinearAtten = r_laserlinear.GetFloat();
state.m_fConstantAtten = r_laserconstant.GetFloat();
state.m_Color[0] = 1.0f;
state.m_Color[1] = 1.0f;
state.m_Color[2] = 1.0f;
state.m_Color[3] = r_laserambient.GetFloat();
state.m_NearZ = r_lasernear.GetFloat();
state.m_FarZ = r_laserfar.GetFloat();
state.m_bEnableShadows = true;
state.m_pSpotlightTexture = m_laserTexture;
state.m_nSpotlightTextureFrame = 0;

if( laserHandle() == CLIENTSHADOW_INVALID_HANDLE )
{
SetlaserHandle( g_pClientShadowMgr->CreateFlashlight( state ) );
}
else
{
g_pClientShadowMgr->UpdateFlashlightState( laserHandle(), state );
}

g_pClientShadowMgr->UpdateProjectedTexture( laserHandle(), true );
}

*/